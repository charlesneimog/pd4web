local bng = pd.Class:new():register("l.bng")

-- ──────────────────────────────────────────
function bng:initialize(_, args)
	self.inlets = 1
	self.outlets = 1
	self.clock = pd.Clock:new():register(self, "blink")
	self.blink_delay = 0
	self.filled = false
	self.need_update_args = false
	if args ~= nil and #args > 0 then
		self.size = args[1]
		self.blink_max = args[2]
		self.interrupt = args[3]
		self.init = args[4]
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
	else
		self.size = 18
		self.blink_max = 250
		self.interrupt = 50
		self.init = 0
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
		self.need_update_args = true
	end

	self:set_size(self.size, self.size)
	return true
end

-- ──────────────────────────────────────────
function bng:update_args()
	local args = {}

	table.insert(args, self.size) -- Ex: 18
	table.insert(args, self.blink_max) -- Ex: 250
	table.insert(args, self.interrupt) -- Ex: 50
	table.insert(args, self.init) -- Ex: 0
	table.insert(args, self.send) -- Ex: "empty"
	table.insert(args, self.receive) -- Ex: "empty"
	table.insert(args, self.label) -- Ex: "empty"
	table.insert(args, self.x_off) -- Ex: 0
	table.insert(args, self.y_off) -- Ex: -9
	table.insert(args, self.font) -- Ex: 0
	table.insert(args, self.fontsize) -- Ex: 10
	table.insert(args, self.bg_color) -- Ex: "#fcfcfc"
	table.insert(args, self.fg_color) -- Ex: "#000000"
	table.insert(args, self.label_color) -- Ex: "#000000"

	self:set_args(args)
end

-- ──────────────────────────────────────────
function bng:mouse_down(x, y)
	self.value = not self.value
	self.filled = true
	self:repaint()
	self.clock:delay(1)
	self.blink_delay = 0
	self:outlet(1, "bang", {})
end

-- ──────────────────────────────────────────
function bng:hex_to_rgb(hex)
	hex = hex:gsub("#", "")
	return {
		tonumber(hex:sub(1, 2), 16),
		tonumber(hex:sub(3, 4), 16),
		tonumber(hex:sub(5, 6), 16),
	}
end

-- ──────────────────────────────────────────
function bng:blink(x, y)
	self.blink_delay = self.blink_delay + 1
	if self.blink_delay < self.blink_max then
		self.clock:delay(1)
	else
		self.filled = false
		self:repaint()
	end
end

-- ──────────────────────────────────────────
function bng:in_1(_)
    self.value = not self.value
	self.filled = true
	self:repaint()
	self.clock:delay(1)
	self.blink_delay = 0
	self:outlet(1, "bang", {"bang"})
end

-- ──────────────────────────────────────────
function bng:paint(g)
	if self.need_update_args then
		self.need_update_args = false
		self:update_args()
	end

	local width, height = self:get_size()

	-- bg
	g:set_color(table.unpack(self:hex_to_rgb(self.bg_color)))
	g:fill_all()

	-- fg
	g:set_color(table.unpack(self:hex_to_rgb(self.fg_color)))

	g:stroke_rect(0, 0, width, height, 1)
	g:stroke_ellipse(1, 1, width - 2, height - 2, 1)
	if self.filled then
		g:fill_ellipse(1, 1, width - 2, height - 2)
	end
end

-- ──────────────────────────────────────────
function bng:in_1_reload()
	self:dofilex(self._scriptname)
	self:initialize()
end
