local show = pd.Class:new():register("show~")

function show:initialize(sel, args)
  self.inlets = {SIGNAL}
  self.outlets = {SIGNAL, DATA}
  self.graphWidth = 152
  self.graphHeight = 0
  self.interval = 1
  self.graphColors = {}
  self.needsRepaintBackground = true
  self.needsRepaintLegend = true
  self.scale = nil
  self.continuousInterval = 1
  self.zoomMode = "normal"
  self.paused = false
  self.hoverResizeHandle = false
  self.resizing = false
  self.resizeStart = nil
  for i, arg in ipairs(args) do
    if arg == "-scale" then
      self.scale = type(args[i+1]) == "number" and args[i+1] or 1
    elseif arg == "-width" then
      self.graphWidth = math.max(math.floor(args[i+1] or 152), 96)
    elseif arg == "-height" then
      self.graphHeight = math.max(math.floor(args[i+1] or 0), 140)
    elseif arg == "-interval" then
      self.interval = math.abs(args[i+1] or 1)
    end
  end
  self:reset()
  return true
end

function show:postinitialize()
  self.clock = pd.Clock:new():register(self, "tick")
  self.clock:delay(0)
end

function show:tick()
  if self.needsRepaintBackground then
    self:repaint(1)
    self.needsRepaintBackground = false
  end
  self:repaint(2)  -- Always repaint the graphs (layer 2)
  if self.needsRepaintLegend then
    self:repaint(3)
    self.needsRepaintLegend = false
  end
  self.clock:delay(self.frameDelay)
end

function show:reset()
  self.sampleIndex = 1
  self.sampleOffset = 0
  self.bufferIndex = 1
  self.colors = {
    graphGradients = {
      hue =        {-25, 275},
      saturation = {85, 95},
      brightness = {80, 90},
    },
    background = {248, 248, 248},
    areaHover =  {200, 200, 200},
    area =       {230, 230, 230},
    valueHover = {148, 148, 148},
    value =      {200, 200, 200},
    text =       {  0,   0,   0},
  }
  self.sigs = {}
  self.avgs = {}
  self.rms = {}
  self.inchans = 0
  self.frameDelay = 20
  self.strokeWidth = 1
  self.valWidth = 48
  self.sigHeight = 16
  self.hover = 0
  self.max = 1
  self.maxVal = 1
  self.width = self.graphWidth + self.valWidth
  self.height = self.graphHeight
  self.dragStart = nil
  self.dragStartInterval = nil
  self.hoverGraph = false
  self.visibleMax = 0
  self.visibleMaxUpdated = false
  self:update_layout()
end

function show:in_1_reset()
  self.reset()
end

function show:get_channel_from_point(x, y)
  if self:point_in_rect(x, y, self.channelRect) then
    return math.floor(y / self.sigHeight) + 1
  end
  return 0
end

function show:update_layout()
  -- Only use graphHeight if explicitly set (greater than 0), otherwise calculate based on channels
  if self.graphHeight > 0 then
    self.height = self.graphHeight
  else
    -- Ensure we have at least one channel worth of height
    local effectiveChannels = math.max(1, self.inchans or 1)
    self.height = effectiveChannels * self.sigHeight
  end
  -- Maintain minimum height requirement
  self.height = math.max(140, self.height)
  self.graphRect = {x = 0, y = 0, width = self.graphWidth, height = self.height}
  self.channelRect = {x = self.graphWidth, y = 0, width = self.valWidth, height = self.height}
  self:set_size(self.graphWidth + self.valWidth, self.height)
end

function show:update_args()
  local args = {}
  
  if self.graphHeight > 0 then
    table.insert(args, "-height")
    table.insert(args, self.graphHeight)
  end
  if self.graphWidth > 0 then
    table.insert(args, "-width")
    table.insert(args, self.graphWidth)
  end
  if self.scale then
    table.insert(args, "-scale")
    table.insert(args, self.scale)
  end
  if self.interval ~= 1 then
    table.insert(args, "-interval")
    table.insert(args, self.interval)
  end
  self:set_args(args)
end

function show:in_1_width(x)
  self.graphWidth = math.max(math.floor(x[1] or 152), 96)
  self:update_args()
  for i=1, self.inchans do
    self.sigs[i] = {}
  end
  self.needsRepaintBackground = true
  self.needsRepaintLegend = true
  self:update_layout()
end

function show:in_1_height(x)
  self.graphHeight = x[1] or 0
  self:update_args()
  self.needsRepaintBackground = true
  self.needsRepaintLegend = true
  self:update_layout()
end

function show:mouse_move(x, y)
  local oldHover = self.hover
  if self:point_in_rect(x, y, self.channelRect) then
    self.hover = math.max(0, math.floor((y - self.channelRect.y) / self.sigHeight) + 1)
    if self.hover > self.inchans then self.hover = 0 end
  else
    self.hover = 0
  end

  local oldHoverGraph = self.hoverGraph
  self.hoverGraph = self:point_in_rect(x, y, self.graphRect)

  if oldHover ~= self.hover or oldHoverGraph ~= self.hoverGraph then
    self.needsRepaintLegend = true
  end

  -- Check for resize handle hover (bottom-right 12x12 area)
  local handleSize = 12
  local wasHover = self.hoverResizeHandle
  self.hoverResizeHandle = (x >= self.graphWidth + self.valWidth - handleSize and y >= self.height - handleSize)
  if wasHover ~= self.hoverResizeHandle then
    self.needsRepaintLegend = true
  end
end

function show:mouse_drag(x, y)
  if self.resizing and self.resizeStart then
    local dx = x - self.resizeStart.x
    local dy = y - self.resizeStart.y
    local newWidth = self.resizeStart.width + dx
    local newHeight = self.resizeStart.height + dy
    self:in_1_width({newWidth})
    self:in_1_height({newHeight})
    return
  end
  if self.dragStart then
    local dy = y - self.dragStart.y
    local scaleFactor = 0.01
    local newInterval = self.dragStartInterval * math.exp(dy * scaleFactor)
    
    -- Apply constraints based on zoom mode
    if self.zoomMode == "above" then
      newInterval = math.max(1.0, newInterval)
    elseif self.zoomMode == "below" then
      newInterval = math.min(1.0, newInterval)
    end
    
    self.interval = math.max(0.01, newInterval)
    self.needsRepaintLegend = true
  end
end

function show:mouse_down(x, y)
  local handleSize = 12
  if x >= self.graphWidth + self.valWidth - handleSize and y >= self.height - handleSize then
    self.resizing = true
    self.resizeStart = {x = x, y = y, width = self.graphWidth, height = self.height}
    return
  end
  self.dragStart = {x = x, y = y}
  self.dragStartInterval = self.interval
  
  -- Determine zoom mode when starting drag
  if math.abs(self.interval - 1.0) < 0.001 then
    self.zoomMode = "boundary"
  else
    self.zoomMode = self.interval > 1.0 and "above" or "below"
  end
end

function show:mouse_up(x, y)
  if self.resizing then
    self.resizing = false
    self.resizeStart = nil
    return
  end
  self.dragStart = nil
  self.dragStartInterval = nil
  -- Don't reset zoomMode here, it persists until next mouse_down
end

function show:point_in_rect(x, y, rect)
  return x >= rect.x and x <= rect.x + rect.width and
         y >= rect.y and y <= rect.y + rect.height
end

function show:in_1_interval(x)
  self.interval = math.max(0.01, x[1] or 1)
  self:update_args()
  self.needsRepaintLegend = true
end

function show:in_1_scale(x)
  self.scale = x[1]
  self:update_args()
end

function show:in_1_framedelay(x)
  self.frameDelay = x[1] or 20
end

function show:in_1_reset()
  self.reset()
end

function show:in_1_bang()
  local output = {}
  for i = 1, self.inchans do
    output[i] = self.sigs[i][(self.bufferIndex - 2) % self.graphWidth + 1]
  end
  self:outlet(2, "list", output)
end

function show:getrange(maxValue)
  local baseValues = {1, 2, 5, 10}
  
  -- Find the appropriate power of 10
  local power = math.max(0, math.floor(math.log(maxValue, 10)))
  local scale = 10^power
  
  -- Normalize the maxValue to between 0 and 10
  local normalizedValue = maxValue / scale
  
  -- Find the first base value that's greater than or equal to the normalized value
  for _, base in ipairs(baseValues) do
    if base >= normalizedValue then
      return base * scale
    end
  end
  
  -- If we get here, normalizedValue must be 10, so we return the next scale up
  return baseValues[1] * (scale * 10)
end

function show:perform(in1)
  -- Skip signal processing if paused
  if self.paused then
    return in1
  end

  for c=1,self.inchans do
    for s=1,self.blocksize do
      local sample = in1[s + self.blocksize * (c-1)] or 0
      self.maxVal = math.max(math.abs(sample), self.maxVal)
      self.avgs[c] = self.avgs[c] * 0.9996 + sample * 0.0004
      self.rms[c] = self.rms[c] * 0.9996 + (sample * sample) * 0.0004
    end
  end
  
  while self.sampleIndex < self.blocksize do  -- Simplified condition
    for i=1,self.inchans do
      local baseIndex = self.blocksize * (i-1)
      
      if self.interval < 1 or math.abs(self.interval - 1) < 0.001 then
        -- Write the sample directly, no interpolation for zoom in and exactly 1:1
        self.sigs[i][self.bufferIndex] = in1[self.sampleIndex + baseIndex + 1] or 0
      else
        -- Keep interpolation for zoomed out mode only
        local exactPos = self.sampleIndex + self.sampleOffset
        local sampleIndex = math.floor(exactPos)
        local fraction = math.min(1.0, exactPos - sampleIndex)
        local sample1 = in1[sampleIndex + baseIndex + 1] or 0
        local sample2 = in1[math.min(sampleIndex + 2, self.blocksize) + baseIndex] or sample1
        self.sigs[i][self.bufferIndex] = sample1 * (1 - fraction) + sample2 * fraction
      end
    end
    
    self.bufferIndex = self.bufferIndex % self.graphWidth + 1
    
    if self.interval < 1 then
      -- For zoomed in mode, just increment by 1
      self.sampleIndex = self.sampleIndex + 1
    else
      -- For zoomed out mode, keep fractional stepping
      self.sampleIndex = self.sampleIndex + math.floor(self.interval)
      self.sampleOffset = self.sampleOffset + (self.interval % 1)
      if self.sampleOffset >= 1 then
        self.sampleIndex = self.sampleIndex + math.floor(self.sampleOffset)
        self.sampleOffset = self.sampleOffset % 1
      end
    end
  end
    
  -- Reset for next block
  self.sampleIndex = self.sampleIndex - self.blocksize
  
  -- Gradual decay of maxVal
  self.maxVal = self.maxVal * 0.99

  -- Use manual scale if set, otherwise use the visibleMax calculated during drawing
  local targetMax = self.scale or self:getrange(math.max(self.visibleMax, 0.000001))
  
  -- Smooth transition of max value
  local transitionSpeed = 0.02
  self.max = self.max + (targetMax - self.max) * transitionSpeed

  if self.max ~= targetMax then
    self.needsRepaintLegend = true
  end

  return in1
end

-- Background
function show:paint(g)
  g:set_color(table.unpack(self.colors.background))
  g:fill_all()
end

-- Graphs
function show:paint_layer_2(g)
  -- Reset the visibleMaxUpdated flag at the start of each frame
  self.visibleMaxUpdated = false
  
  g:set_color(table.unpack(self.colors.area))
  g:draw_line(0, self.height/2, self.graphWidth - 1, self.height/2, 1)
  -- Graphs, RMS charts, and avg values

  if self.hover == 0 then
    -- No channel highlighted: draw in reverse order
    for idx = #self.sigs, 1, -1 do
      self:draw_channel(g, idx, false)
    end
  else
    -- Channel highlighted: draw non-highlighted channels first, then the highlighted one
    for idx = 1, #self.sigs do
      if idx ~= self.hover then
        self:draw_channel(g, idx, false)
      end
    end
    if self.hover <= #self.sigs then
      self:draw_channel(g, self.hover, true)
    end
  end
end

-- Foreground elements (typo and hover)
function show:paint_layer_3(g)
  -- Legend: channel if hovered, and scale
  g:set_color(table.unpack(self.colors.text))
  g:draw_text(string.format("1px = %.2fsp", self.interval), 3, self.height-13, 100, 10)
  g:draw_text(string.format("% 8.2f", self.max), self.graphWidth-50, 3, 50, 10)
  g:draw_text(string.format("% 8.2f", -self.max), self.graphWidth-50, self.height-13, 50, 10)

  -- Draw hovered channel number
  if self.hover > 0 and self.hover <= #self.sigs then
    g:set_color(table.unpack(self.graphColors[self.hover] or {0, 0, 0}))
    g:draw_text(string.format("ch %d", self.hover), 3, 3, 64, 10)
  end

  -- Draw resize handle
  local baseSize = 4
  local handleSize = self.hoverResizeHandle and 12 or baseSize
  local x0 = self.graphWidth + self.valWidth - handleSize
  local y0 = self.height - handleSize
  g:set_color(1)
  g:fill_rect(x0 - (self.hoverResizeHandle and 0 or 2), 
              y0 - (self.hoverResizeHandle and 0 or 4), 
              handleSize, handleSize)
end

function show:draw_channel(g, idx, isHovered)
  local sig = self.sigs[idx]
  if not sig then return end

  local color = self.graphColors[idx] or {255, 255, 255}  -- Default to white if color is not set
  local graphColor = (self.hover == 0 or isHovered) and color or self.colors.area
  
  -- Calculate actual RMS value and scale it according to self.max
  local rmsValue = math.sqrt(self.rms[idx] or 0)
  local scaledRMS = rmsValue / self.max
  
  -- RMS bar
  g:set_color(table.unpack(isHovered and self.colors.valueHover or self.colors.value))
  g:fill_rect(self.graphWidth, self.sigHeight * (idx-1) + 1, 
              self.valWidth * math.min(1, scaledRMS), 
              self.sigHeight-1)
  
  -- Graph line
  g:set_color(table.unpack(graphColor))
  
  -- Reset visibleMax at the start of new frame
  if not self.visibleMaxUpdated and not self.scale then
    self.visibleMax = 0
    self.visibleMaxUpdated = true
  end
  
  local function scaleY(value)
    -- Track maximum value while scaling (only if not using manual scale)
    if not self.scale then
      self.visibleMax = math.max(self.visibleMax, math.abs(value))
    end
    return (value / self.max * -0.5 + 0.5) * (self.height - 2) + 1
  end

  local x0 = (self.bufferIndex - 1) % self.graphWidth

  if self.interval >= 1 or math.abs(self.interval - 1) < 0.001 then
    -- Draw one sample per pixel, left to right (handles both 1:1 and zoomed out)
    local y0 = scaleY(sig[x0 + 1] or 0)
    p = Path(0, y0)
    for x = 1, self.graphWidth - 1 do
      local bufferX = (x0 + x) % self.graphWidth + 1
      local y = scaleY(sig[bufferX] or 0)
      p:line_to(x, y)
    end
  else
    -- Zoomed-in drawing: work backwards from most recent sample
    local lastX = self.graphWidth - 1
    local lastY = scaleY(sig[x0] or 0)
    p = Path(lastX, lastY)
    
    -- Calculate how many points we can draw within graphWidth
    local maxPoints = math.ceil(self.graphWidth * self.interval)
    
    -- Draw lines between actual samples at their exact x positions, working backwards
    for i = 1, maxPoints do
      local bufferX = (x0 - i + self.graphWidth - 1) % self.graphWidth + 1
      local sampleX = lastX - (i * (1/self.interval))
      if sampleX < 0 then break end
      
      local y = scaleY(sig[bufferX] or 0)
      p:line_to(sampleX, y)
    end
  end
  
  g:stroke_path(p, self.strokeWidth)
  
  -- Display just the average value
  g:set_color(table.unpack((self.hover == 0 or isHovered) and color or self.colors.text))
  g:draw_text(string.format("% 7.2f", self.avgs[idx] or 0), self.graphWidth + 4, idx * self.sigHeight - 13, self.valWidth, 10)
end

function show:dsp(samplerate, blocksize, inchans)
  -- Store old channel count to detect changes
  local oldChannels = self.inchans
  
  -- Update basic parameters
  self.blocksize = blocksize
  self.inchans = inchans[1]
  self:signal_setmultiout(1, self.inchans)

  -- Reset signal buffers
  self.sigs = {}
  self.rms = {}
  self.avgs = {}
  
  -- Generate new colors for channels
  self.graphColors = self:generate_colors(self.inchans)
 
  -- Initialize buffers for each channel
  for c = 1, self.inchans do
    self.rms[c] = 0
    self.avgs[c] = 0
    self.sigs[c] = {}
  end
  
  -- Reset hover state when channel count changes
  if oldChannels ~= self.inchans then
    self.hover = 0
    -- Force layout update when channel count changes
    self.needsRepaintBackground = true
    self.needsRepaintLegend = true
    self:update_layout()
  end
end

function show:hsv_to_rgb(h, s, v)
  h = h % 360  -- Ensure h is in the range 0-359
  s = s / 100  -- Convert s to 0-1 range
  v = v / 100  -- Convert v to 0-1 range

  local c = v * s
  local x = c * (1 - math.abs((h / 60) % 2 - 1))
  local m = v - c

  local r, g, b
  if h < 60 then
    r, g, b = c, x, 0
  elseif h < 120 then
    r, g, b = x, c, 0
  elseif h < 180 then
    r, g, b = 0, c, x
  elseif h < 240 then
    r, g, b = 0, x, c
  elseif h < 300 then
    r, g, b = x, 0, c
  else
    r, g, b = c, 0, x
  end

  -- Scale to 0-255 range and round to nearest integer
  return math.floor((r + m) * 255 + 0.5), 
         math.floor((g + m) * 255 + 0.5), 
         math.floor((b + m) * 255 + 0.5)
end

function show:generate_colors(count)
  local colors = {}
  local hue_start, hue_end = table.unpack(self.colors.graphGradients.hue)
  local sat_start, sat_end = table.unpack(self.colors.graphGradients.saturation)
  local bright_start, bright_end = table.unpack(self.colors.graphGradients.brightness)

  for i = 1, count do
    local hue = hue_start + (hue_end - hue_start) * ((i - 1) / math.max(1, count - 1))
    local saturation = sat_start + (sat_end - sat_start) * ((i - 1) / math.max(1, count - 1))
    local brightness = bright_start + (bright_end - bright_start) * ((i - 1) / math.max(1, count - 1))
    local r, g, b = self:hsv_to_rgb(hue, saturation, brightness)
    table.insert(colors, {r, g, b})
  end

  return colors
end

function show:in_1_pause(x)
  local wasPaused = self.paused
  self.paused = (x[1] or 0) > 0
end
