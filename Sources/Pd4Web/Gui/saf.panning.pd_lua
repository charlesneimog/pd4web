local panning = pd.Class:new():register("saf.panning")

-- ─────────────────────────────────────
function panning:initialize(_, args)
	self.inlets = 1
	self.outlets = 1
	self.repaint_sources = false
	self.selected = false
	self.plan_size = 200
	self.fig_size = 200
	self.sources_size = 3
	self.margin = 5
	self.xzview = false

	-- Define colors with appropriate RGB values
	self.colors = {
		background1 = { 19, 47, 80 },
		background2 = { 27, 55, 87 },
		lines = { 46, 73, 102 },
		text = { 127, 145, 162 },
		sources = { 255, 0, 0 },
		source_text = { 230, 230, 240 },
	}

	for i, arg in ipairs(args) do
		if arg == "-plan_size" then
			self.plan_size = type(args[i + 1]) == "number" and args[i + 1] or 200
			self.fig_size = self.plan_size
		elseif arg == "-sources_size" then
			self.sources_size = type(args[i + 1]) == "number" and args[i + 1] or 5
		elseif arg == "-margin" then
			self.margin = type(args[i + 1]) == "number" and args[i + 1] or 5
		elseif arg == "-xzview" then
			local xzview = type(args[i + 1]) == "number" and args[i + 1] or 0
			if xzview == 1 then
				self.xzview = true
			end
		end
	end

	self.sources = {}
	self:set_size(self.fig_size, self.plan_size)
	for i = 1, self.sources_size do
		self.sources[i] = self:create_newsource(i)
	end

	return true
end

-- ─────────────────────────────────────
function panning:create_newsource(i)
	local center_x, center_y = self:get_size() / 2, self:get_size() / 2
	local margin = self.margin
	local max_radius = (self.plan_size / 2) - margin
	local angle_step = (math.pi * 2) / self.sources_size
	local angle = (i - 1) * angle_step
	local distance = max_radius * 0.9

	local x = center_x + math.cos(angle) * distance
	local y = center_y + math.sin(angle) * distance
	local z = center_x

	return {
		i = i,
		x = x,
		y = y,
		z = z,
		size = 8,
		color = self.colors.sources,
		fill = false,
		selected = false,
	}
end

-- ──────────────────────────────────────────
function panning:update_args()
	local args = {
		"-plan_size",
		self.plan_size,
	}
	table.insert(args, "-sources_size")
	table.insert(args, self.sources_size)
	table.insert(args, "-fig_size")
	table.insert(args, self.fig_size)

	if self.xzview == 1 then
		table.insert(args, "-xzview")
		table.insert(args, self.xzview)
	end

	self:set_args(args)
end

--╭─────────────────────────────────────╮
--│               METHODS               │
--╰─────────────────────────────────────╯
function panning:in_1_reload()
	self:dofilex(self._scriptname)
	self:initialize("", {})
	self:repaint()
end

-- ─────────────────────────────────────
function panning:in_1_xzview(args)
	if args[1] == 1 then
		self.xzview = true
		self.fig_size = self.plan_size * 2
		self:set_size(self.fig_size, self.plan_size)
		self:update_args()
	else
		self.xzview = false
		self.fig_size = self.fig_size / 2
		self:set_size(self.plan_size, self.fig_size)
	end
end
-- ─────────────────────────────────────
function panning:in_1_source(args)
	local index = args[1]
	local azi_deg = args[2]
	local ele_deg = args[3]
	local dis = 0.8

	if #args >= 4 then
		dis = args[4]
	end

	if index > self.sources_size then
		self.sources_size = index
		self:in_1_sources({ index })
	end

	self:outlet(1, "source", { index, azi_deg, ele_deg })
	local adjusted_radius = (self.plan_size / 2) - self.margin
	local azi_rad = math.rad(azi_deg)
	local ele_rad = math.rad(ele_deg)

	-- Flip X so +90° is left, -90° is right
	local x = -math.cos(ele_rad) * math.sin(azi_rad) * adjusted_radius
	local y = -math.cos(ele_rad) * math.cos(azi_rad) * adjusted_radius
	local z = math.sin(ele_rad) * adjusted_radius

	for _, source in pairs(self.sources) do
		if source.i == index then
			source.x = x + self.plan_size / 2
			source.y = y + self.plan_size / 2
			source.z = z + self.plan_size / 2
		end
	end

	self:update_args()
	self:repaint(2)
end

-- ─────────────────────────────────────
function panning:in_1_size(args)
	local old_size = self.plan_size

	self:set_size(args[1], args[1])
	self.plan_size = args[1]
	local relation = self.plan_size / old_size
	for _, source in pairs(self.sources) do
		source.x = source.x * relation
		source.y = source.y * relation
	end

	self:update_args()
	self:repaint(1)
	self:repaint(2)
	self:repaint(3)
end

-- ─────────────────────────────────────
function panning:in_1_sources(args)
	local num_circles = args[1]
	local center_x, center_y = self:get_size() / 2, self:get_size() / 2
	local angle_step = (math.pi * 2) / num_circles -- Espaçamento angular
	self.sources_size = args[1]

	self.sources = {}

	local margin = 10 -- Same margin used before
	local max_radius = (math.min(center_x, center_y) / 2) - margin
	local distance = max_radius * 0.9 -- Keep sources slightly inside the inner circle

	for i = 1, num_circles do
		local angle = i * angle_step -- Progressively increase the angle for each circle
		self.sources[i] = self:create_newsource(i)
		self.sources[i].x = center_x + math.cos(angle) * distance
		self.sources[i].y = center_y + math.sin(angle) * distance
	end

	self:repaint(2)
	self:outlet(1, "set", { "num_sources", args[1] })
end

--╭─────────────────────────────────────╮
--│                MOUSE                │
--╰─────────────────────────────────────╯
function panning:mouse_drag(x, y)
	local size_x, size_y = self:get_size()
	local cx, cy = size_x / 2, size_y / 2

	-- Ignore drags outside the margin
	if x < 5 or x > (size_x - 5) or y < 5 or y > (size_y - 5) then
		return
	end

	for i, source in pairs(self.sources) do
		if source.selected then
			source.x = x
			source.y = y
			source.fill = true

			-- Convert screen position to centered coordinates
			local dx = x - cx
			local dy = y - cy

			-- Apply inverse of the flipped X axis
			local azi = math.atan(-dx, -dy) -- Invert X and Y to match ambisonic
			local r = math.sqrt(dx * dx + dy * dy)
			local ele = math.atan(0, r) -- Still flat, set z=0 for now

			local azi_degrees = math.deg(azi)
			local ele_degrees = math.deg(ele)

			self:outlet(1, "source", { i, azi_degrees, ele_degrees })
		else
			source.fill = false
		end
	end

	self:repaint(3)
end

-- ─────────────────────────────────────
function panning:mouse_down(x, y)
	local already_selected = false
	for i, source in pairs(self.sources) do
		local cx = source.x
		local cy = source.y
		local radius = source.size / 2
		local dx = x - cx
		local dy = y - cy
		if (dx * dx + dy * dy) <= (radius * radius) then
			self.sources[i].x = x
			self.sources[i].y = y
			self.sources[i].fill = true
			if not already_selected then
				self.sources[i].selected = true
				already_selected = true
			else
				self.sources[i].selected = false
			end
		else
			self.sources[i].fill = false
			self.sources[i].selected = false
		end
	end

	self:repaint(2)
	self:repaint(3)
end

-- ─────────────────────────────────────
function panning:mouse_up(_, _)
	for i, _ in pairs(self.sources) do
		self.sources[i].fill = false
		self.sources[i].selected = false
	end
	self:repaint(2)
	self:repaint(3)
end

--╭─────────────────────────────────────╮
--│                PAINT                │
--╰─────────────────────────────────────╯
function panning:paint(g)
	local size_x, size_y = self.plan_size, self.plan_size
	if not self.colors then
		return
	end

	-- Use colors from self.colors
	g:set_color(table.unpack(self.colors.background1))
	g:fill_all()
	g:set_color(table.unpack(self.colors.background2))
	g:fill_ellipse(self.margin, self.margin, size_x - 2 * self.margin, size_y - 2 * self.margin)

	-- Lines
	g:set_color(table.unpack(self.colors.lines))
	local center = size_x / 2

	-- Adjusted vertical and horizontal lines
	g:draw_line(center, self.margin, center, size_y - self.margin, 1)
	g:draw_line(self.margin, center, size_x - self.margin, center, 1)
	--
	-- Lines from center to border (radial lines)
	local base_radius = (math.min(size_x, size_y) / 2) - self.margin -- Radius of the circle
	for angle = 0, 2 * math.pi, math.pi / 8 do -- Change the angle increment for more/less lines
		local x_end = center + math.cos(angle) * base_radius
		local y_end = center + math.sin(angle) * base_radius
		g:draw_line(center, center, x_end, y_end, 1) -- Line from center to border
	end

	-- Ellipse 1
	local base_size = (math.min(size_x, size_y) / 2) - self.margin
	for i = 0, 3 do
		local scale = math.log(i + 1) / math.log(6)
		local radius_x = base_size * (1 - scale)
		local radius_y = base_size * (1 - scale)
		g:stroke_ellipse(center - radius_x, center - radius_y, radius_x * 2, radius_y * 2, 1)
	end

	-- -- Text
	local text_x, text_y = 1, 1
	g:set_color(table.unpack(self.colors.text))
	g:draw_text("xy view", text_x, text_y, 50, 1)

	if self.xzview == 0 then
		return
	end

	--╭─────────────────────────────────────╮
	--│              WORLD TWO              │
	--╰─────────────────────────────────────╯
	if self.xzview then
		g:set_color(table.unpack(self.colors.background2))
		g:set_color(table.unpack(self.colors.background2))
		-- Draw the circle (ellipse) at position (self.plan_size + self.margin, self.margin)
		g:fill_ellipse(
			self.plan_size + self.margin, -- X position of the ellipse's top-left corner
			self.margin, -- Y position of the ellipse's top-left corner
			size_x - 2 * self.margin, -- Width of the ellipse
			size_y - 2 * self.margin -- Height of the ellipse
		)
		g:set_color(table.unpack(self.colors.lines))

		-- Calculate the TRUE center of the circle (ellipse)
		local ellipse_x = self.plan_size + self.margin
		local ellipse_y = self.margin
		local ellipse_width = size_x - 2 * self.margin
		local ellipse_height = size_y - 2 * self.margin

		-- True center coordinates of the ellipse/circle
		local center_x = ellipse_x + ellipse_width / 2 -- Critical fix: X center
		local center_y = ellipse_y + ellipse_height / 2 -- Critical fix: Y center
		local radius = ellipse_width / 2 -- Assume circle (width = height)

		local vertical_lines = 8
		for i = 1, vertical_lines do
			-- Vertical position of the chord (evenly spaced from bottom to top)
			local y_line = center_y - radius + (i * (2 * radius / vertical_lines))

			-- Horizontal offset (chord width) at this y_line
			local x_offset = math.sqrt(radius ^ 2 - (y_line - center_y) ^ 2)

			-- Draw the chord (no extra offsets needed; center_x is already correct)
			g:draw_line(
				center_x - x_offset, -- Left edge of chord
				y_line, -- Y position
				center_x + x_offset, -- Right edge of chord
				y_line, -- Y position
				1 -- Line thickness/color
			)
		end

		for i = 3, 7 do
			local circle_width = math.log(i) * (self.plan_size - self.margin)
			g:stroke_ellipse(
				self.plan_size + self.margin + (circle_width / 2),
				self.margin,
				(size_x - 2 * self.margin) - circle_width,
				size_y - 2 * self.margin,
				1
			)
		end

		-- Text
		text_x, text_y = 2 + self.plan_size, 1
		g:set_color(table.unpack(self.colors.text))
		g:draw_text("not finished yet", text_x, text_y, 50, 1)

		g:draw_line(self.plan_size, 0, self.plan_size, self.plan_size, 1)
	end
end

-- -- ─────────────────────────────────────
function panning:paint_layer_2(g)
	for i, source in pairs(self.sources) do
		if not self.sources[i].selected then
			-- Using the center of the circle, not the bottom-left corner
			local x = source.x
			local y = source.y
			local z = source.z
			local size = source.size

			-- Adjusting the drawing to make the ellipse centered at (x, y)
			g:set_color(table.unpack(source.color))
			g:stroke_ellipse(x - (size / 2), y - (size / 2), size, size, 1)

			if self.xzview then
				g:set_color(table.unpack(source.color))
				g:stroke_ellipse(x - (size / 2) + self.plan_size, z - (size / 2), size, size, 1)
			end

			local scale_factor = 0.7
			g:scale(scale_factor, scale_factor)
			local text_x, text_y = x - (size / 3), y - (size / 3)
			g:set_color(table.unpack(self.colors.source_text))
			g:draw_text(tostring(i), (text_x + 1) / scale_factor, (text_y - 1) / scale_factor, 20, 3)
			g:reset_transform()
		end
	end
end

-- -- ─────────────────────────────────────
function panning:paint_layer_3(g)
	for i, source in pairs(self.sources) do
		if self.sources[i].selected then
			local x = source.x - (source.size / 2)
			local y = source.y - (source.size / 2)
			local size = source.size
			g:set_color(table.unpack(source.color))
			g:fill_ellipse(x, y, size, size)
			g:stroke_ellipse(x, y, size, size, 1)

			-- Source index text
			local text_x, text_y = x - (size / 1.5), y - (size / 2)
			g:set_color(table.unpack(self.colors.source_text)) -- Use source_text color
			g:draw_text(tostring(i), text_x, text_y, 10, 3)

			-- Coordinate text
			text_x, text_y = x + (size / 1), y + (size / 2)
			local scale_factor = 0.7
			g:scale(scale_factor, scale_factor)
			g:set_color(table.unpack(self.colors.source_text)) -- Use source_text color
			g:draw_text(
				tostring(math.floor(x)) .. " " .. tostring(math.floor(y)),
				text_x / scale_factor,
				text_y / scale_factor,
				40,
				1
			)
			g:reset_transform()

			-- new
			if self.xzview then
				local z = source.z - (source.size / 2)
				g:set_color(table.unpack(source.color))
				g:fill_ellipse(x + self.plan_size, z, size, size)
				g:stroke_ellipse(x + self.plan_size, z, size, size, 1)
			end

			-- Source index text
			-- local text_x, text_y = x - (size / 1.5), y - (size / 2)
			-- g:set_color(table.unpack(self.colors.source_text)) -- Use source_text color
			-- g:draw_text(tostring(i), text_x, text_y, 10, 3)
			--
			-- -- Coordinate text
			-- text_x, text_y = x + (size / 1), y + (size / 2)
			-- local scale_factor = 0.7
			-- g:scale(scale_factor, scale_factor)
			-- g:set_color(table.unpack(self.colors.source_text)) -- Use source_text color
			-- g:draw_text(
			-- 	tostring(math.floor(x)) .. " " .. tostring(math.floor(y)),
			-- 	text_x / scale_factor,
			-- 	text_y / scale_factor,
			-- 	40,
			-- 	1
			-- )
			-- g:reset_transform()
		end
	end
end
