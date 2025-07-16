local vsl = pd.Class:new():register("l.vsl")

-- ──────────────────────────────────────────
function vsl:initialize(_, args)
	self.inlets = 1
	self.outlets = 1
	self.need_update_args = false
	if args ~= nil and #args > 0 then
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
		self.width = 18
		self.height = 153
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
		self.need_update_args = true
	end

	self:set_size(self.width, self.height)
	local initial_value = self.init or self.default_value or 0
	self.pos = self:value_to_pos(initial_value)

	return true
end

-- ─────────────────────────────────────
function vsl:update_args()
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
function vsl:value_to_pos(val)
	local _, h = self:get_size()
	local range = self.top - self.bottom
	local rel = (val - self.bottom) / range
	return math.floor((1 - rel) * (h - 6)) -- inverted, top = max
end

-- ──────────────────────────────────────────
function vsl:pos_to_value(pos)
	local _, h = self:get_size()
	local range = self.top - self.bottom
	local rel = 1 - (pos / (h - 6))
	return self.bottom + rel * range
end

-- ──────────────────────────────────────────
function vsl:clamp_pos(y)
	local _, h = self:get_size()
	return math.max(3, math.min(y, h - 6))
end

-- ──────────────────────────────────────────
function vsl:mouse_down(x, y)
	self.pos = self:clamp_pos(y)
	local val = self:pos_to_value(self.pos)
	self:outlet(1, "float", { val })
	self:repaint(2)
end

-- ──────────────────────────────────────────
function vsl:mouse_up(x, y)
	self.pos = self:clamp_pos(y)
	local val = self:pos_to_value(self.pos)
	self:outlet(1, "float", { val })
	self:repaint(2)
end

-- ──────────────────────────────────────────
function vsl:mouse_drag(x, y)
	self.pos = self:clamp_pos(y)
	local val = self:pos_to_value(self.pos)
	self:outlet(1, "float", { val })
	self:repaint(2)
end

-- ──────────────────────────────────────────
function vsl:hex_to_rgb(hex)
	hex = hex:gsub("#", "")
	return {
		tonumber(hex:sub(1, 2), 16),
		tonumber(hex:sub(3, 4), 16),
		tonumber(hex:sub(5, 6), 16),
	}
end

-- ──────────────────────────────────────────
function vsl:paint(g)
	if self.need_update_args then
		self.need_update_args = false
		self:update_args()
	end
	local width, height = self:get_size()
	g:set_color(table.unpack(self:hex_to_rgb(self.bg_color)))
	g:fill_all()

	g:set_color(table.unpack(self:hex_to_rgb(self.fg_color)))
	g:stroke_rect(0, 0, width, height, 1)
end

-- ──────────────────────────────────────────
function vsl:paint_layer_2(g)
	local width, _ = self:get_size()
	g:set_color(table.unpack(self:hex_to_rgb(self.fg_color)))
	g:fill_rect(1, self.pos, width - 2, 3)
end

-- ──────────────────────────────────────────
function vsl:in_1_reload()
	self:dofilex(self._scriptname)
	self:initialize()
end
