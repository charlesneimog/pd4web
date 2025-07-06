local vradio = pd.Class:new():register("l.vradio")

-- ──────────────────────────────────────────
function vradio:initialize(_, args)
	self.inlets = 1
	self.outlets = 1
	if args ~= nil and #args > 0 then
		self.size = args[1]
		self.new_old = args[2]
		self.init = args[3]
		self.number = args[4]
		self.send = args[5]
		self.receive = args[6]
		self.label = args[7]
		self.x_off = args[8]
		self.y_off = args[9]
		self.font = args[10]
		self.fontsize = args[11]
		self.bg_color = args[12]
		self.fg_color = args[13]
		self.label_color = args[14]
		self.default_value = args[15]
	else
		self.size = 18
		self.new_old = 1
		self.init = 0
		self.number = 8
		self.send = "empty"
		self.receive = "empty"
		self.label = "empty"
		self.x_off = 0
		self.y_off = -9
		self.font = 0
		self.fontsize = 10
		self.bg_color = "#fcfcfc"
		self.fg_color = "#000000"
		self.label_color = "#000000"
		self.default_value = 0
	end

	self:set_size(self.size, self.size * self.number)
	self.selected = 0

	return true
end

-- ──────────────────────────────────────────
function vradio:mouse_down(x, y)
	local pos = math.floor(y / self.size)
	self.selected = pos
	self:outlet(1, "float", { pos })
	self:repaint(2)
end

-- ──────────────────────────────────────────
function vradio:hex_to_rgb(hex)
	hex = hex:gsub("#", "")
	return {
		tonumber(hex:sub(1, 2), 16),
		tonumber(hex:sub(3, 4), 16),
		tonumber(hex:sub(5, 6), 16),
	}
end

-- ──────────────────────────────────────────
function vradio:paint(g)
	local width, height = self:get_size()
	g:set_color(table.unpack(self:hex_to_rgb(self.bg_color)))
	g:fill_all()

	g:set_color(table.unpack(self:hex_to_rgb(self.fg_color)))
	local pos = 0
	for i = 1, self.number do
		g:stroke_rect(0, 0 + pos, self.size, self.size, 1)
		pos = pos + self.size
	end
end

-- ──────────────────────────────────────────
function vradio:paint_layer_2(g)
	g:fill_rect(4, 4 + (self.selected * self.size), self.size - 8, self.size - 8, 1)
end

-- ──────────────────────────────────────────
function vradio:in_1_reload()
	self:dofilex(self._scriptname)
	self:initialize()
end
