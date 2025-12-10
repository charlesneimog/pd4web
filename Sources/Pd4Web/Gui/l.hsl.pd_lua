local hsl = pd.Class:new():register("l.hsl")

-- ──────────────────────────────────────────
function hsl:initialize(_, args)
	self.inlets = 1
	self.outlets = 1
	self.pos = 20

	if args ~= nil and #args > 0 then
		-- 248 29 0 0.99 0 0 empty empty empty -2 -8 0 10 #ffffff #000000 #373737 0 1
		self.width = args[1]
		self.height = args[2]
		self.bottom = args[3]
		self.top = args[4]
		self.log = args[5]
		self.init = args[6]
		self.send = args[7]
		self.receive = args[8]
		self.label = args[9]
		self.x_off = args[10]
		self.y_off = args[11]
		self.font = args[12]
		self.fontsize = args[13]
		self.bg_color = args[14]
		self.fg_color = args[15]
		self.label_color = args[16]
		self.default_value = args[17]
		self.steady_on_click = args[18]
	else
		self.width = 153
		self.height = 18
		self.bottom = 0
		self.top = 127
		self.log = 0
		self.init = 0
		self.send = "empty"
		self.receive = "empty"
		self.label = "empty"
		self.x_off = -2
		self.y_off = -9
		self.font = 0
		self.fontsize = 10
		self.bg_color = "#fcfcfc"
		self.fg_color = "#000000"
		self.label_color = "#000000"
		self.default_value = 0
		self.steady_on_click = 1
	end

	self:set_size(self.width, self.height)
	local initial_value = self.init or self.default_value or 0
	self.pos = self:value_to_pos(initial_value)

	return true
end

-- ──────────────────────────────────────────
function hsl:value_to_pos(val)
	local w, _ = self:get_size()
	local range = self.top - self.bottom
	local rel = (val - self.bottom) / range
	return math.floor(rel * (w - 6)) -- esquerda = min, direita = max
end

-- ─────────────────────────────────────
function hsl:finalize()
	local args = {}
	table.insert(args, self.width)
	table.insert(args, self.height)
	table.insert(args, self.bottom)
	table.insert(args, self.top)
	table.insert(args, self.log)
	table.insert(args, self.init)
	table.insert(args, self.send)
	table.insert(args, self.receive)
	table.insert(args, self.label)
	table.insert(args, self.x_off)
	table.insert(args, self.y_off)
	table.insert(args, self.font)
	table.insert(args, self.fontsize)
	table.insert(args, self.bg_color)
	table.insert(args, self.fg_color)
	table.insert(args, self.label_color)
	table.insert(args, self.default_value)
	table.insert(args, self.steady_on_click)
	self:set_args(args)
end

-- ──────────────────────────────────────────
function hsl:pos_to_value(pos)
	local w, _ = self:get_size()
	local range = self.top - self.bottom
	local rel = pos / (w - 6)
	return self.bottom + rel * range
end

-- ──────────────────────────────────────────
function hsl:clamp_pos(x)
	local w, _ = self:get_size()
	return math.max(3, math.min(x, w - 6))
end

-- ──────────────────────────────────────────
function hsl:mouse_down(x, _)
	self.pos = self:clamp_pos(x)
	self:repaint(2)
	self:outlet(1, "float", { self:pos_to_value(self.pos) })
end

-- ──────────────────────────────────────
function hsl:mouse_up(x, _)
	self.pos = self:clamp_pos(x)
	self:repaint(2)
	self:outlet(1, "float", { self:pos_to_value(self.pos) })
end

-- ──────────────────────────────────────
function hsl:mouse_drag(x, _)
	self.pos = self:clamp_pos(x)
	self:repaint(2)
	self:outlet(1, "float", { self:pos_to_value(self.pos) })
end

-- ──────────────────────────────────────
function hsl:hex_to_rgb(hex)
	local first = string.sub(hex, 1, 1)
	if first ~= "#" then
		self:error("Hex color must start with #")
		return { 0, 0, 0 }
	end

	hex = hex:gsub("#", "")
	return {
		tonumber(hex:sub(1, 2), 16),
		tonumber(hex:sub(3, 4), 16),
		tonumber(hex:sub(5, 6), 16),
	}
end

-- ──────────────────────────────────────────
function hsl:paint(g)
	local width, height = self:get_size()
	g:set_color(table.unpack(self:hex_to_rgb(self.bg_color)))

	g:fill_all()

	g:set_color(table.unpack(self:hex_to_rgb(self.fg_color)))
	g:stroke_rect(0, 0, width, height, 1)
end

-- ──────────────────────────────────────────
function hsl:paint_layer_2(g)
	local _, height = self:get_size()
	g:set_color(table.unpack(self:hex_to_rgb(self.fg_color)))
	g:fill_rect(self.pos, 1, 3, height - 2)
end

-- ──────────────────────────────────────────
function hsl:in_1_reload()
	self:dofilex(self._scriptname)
	self:initialize()
end

