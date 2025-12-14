local tgl = pd.Class:new():register("l.tgl")

-- ──────────────────────────────────────────
function tgl:initialize(_, args)
	self.inlets = 1
	self.outlets = 1
	self.need_update_args = false
	self.on = false

	if args ~= nil and #args > 0 then
		self.size = args[1]
		self.init = args[2]
		self.send = args[3]
		self.receive = args[4]
		self.label = args[5]
		self.x_off = args[6]
		self.y_off = args[7]
		self.font = args[8]
		self.fontsize = args[9]
		self.bg_color = args[10]
		self.fg_color = args[11]
		self.label_color = args[12]
		self.init_value = args[13] -- initial value
		self.default_value = args[14] -- value for "on" state
	else
		self.size = 18
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
		self.init_value = 0
		self.default_value = 1
		self.need_update_args = true
	end

	if self.receive ~= "empty" then
		self.receiver = pd.Receive:new():register(self, self.receive, "_receiver")
	end

	self:set_size(self.size, self.size)
	return true
end

-- ─────────────────────────────────────
function tgl:_receiver(sel, atoms)
	if sel == "float" then
		if atoms[1] == 0 then
			self.on = false
		else
			self.on = true
		end
		self:repaint()
		self:outlet(1, "float", { atoms[1] })
	end
end

-- ──────────────────────────────────────────
function tgl:postinitialize()
	if self.receive ~= "empty" then
		self.receiver = pd.Receive:new():register(self, "list", "received")
	end
	if self.init then
		self:outlet(1, "float", { self.init_value })
	end
end

-- ──────────────────────────────────────────
function tgl:received(sel, atoms)
	for k, v in ipairs(atoms) do
		pd.post(v)
	end
end

-- ──────────────────────────────────────────
function tgl:update_args()
	local args = {
		self.size,
		self.init,
		self.send,
		self.receive,
		self.label,
		self.x_off,
		self.y_off,
		self.font,
		self.fontsize,
		self.bg_color,
		self.fg_color,
		self.label_color,
		self.init_value,
		self.default_value,
	}

	self:set_args(args)
end

-- ──────────────────────────────────────────
function tgl:mouse_down(x, y)
	self.on = not self.on
	if self.on then
		self:outlet(1, "float", { self.default_value })
		if self.init then
			self.init_value = self.default_value
		end
	else
		self:outlet(1, "float", { 0 })
		if self.init then
			self.init_value = 0
		end
	end

	if self.send ~= "empty" then
		pd.send(self.send, "float", self.on and self.default_value or 0)
	end

	self:repaint()
end

-- ──────────────────────────────────────────
function tgl:hex_to_rgb(hex)
	hex = hex:gsub("#", "")
	return {
		tonumber(hex:sub(1, 2), 16),
		tonumber(hex:sub(3, 4), 16),
		tonumber(hex:sub(5, 6), 16),
	}
end

-- ──────────────────────────────────────────
function tgl:paint(g)
	if self.need_update_args then
		self.need_update_args = false
		self:update_args()
	end

	local width, height = self:get_size()
	g:set_color(table.unpack(self:hex_to_rgb(self.bg_color)))
	g:fill_all()

	-- fg
	g:set_color(table.unpack(self:hex_to_rgb(self.fg_color)))
	g:stroke_rect(0, 0, width, height, 1)

	if self.on then
		g:set_color(table.unpack(self:hex_to_rgb(self.fg_color)))
		local w = width
		local line_width = 1
		if w >= 30 then
			line_width = 2
		end
		if w >= 60 then
			line_width = 3
		end

		g:draw_line(2, 2, width - 2, height - 2, line_width)
		g:draw_line(width - 2, 2, 2, height - 2, line_width)
		g:draw_line(2, 2, width - 2, height - 2, line_width)
		g:draw_line(width - 2, 2, 2, height - 2, line_width)
	end
end

--    class_addmethod(toggle_class, (t_method)toggle_nonzero, gensym("nonzero"), A_FLOAT, 0);
--    class_addmethod(toggle_class, (t_method)iemgui_zoom, gensym("zoom"),

-- ──────────────────────────────────────────
function tgl:in_1_init(args)
	self.init = args[1]
	self:update_args()
	self:repaint()
end

-- ──────────────────────────────────────────
function tgl:in_1_send(args)
	self.send = args[1]
	self:update_args()
	self:repaint()
end

-- ──────────────────────────────────────────
function tgl:in_1_receive(args)
	self.receive = args[1]
	self:update_args()
	self:repaint()
end

-- ──────────────────────────────────────────
function tgl:in_1_size(args)
	self.size = args[1]
	self:update_args()
	self:set_size(args[1], args[1])
	self:repaint()
end

-- ──────────────────────────────────────────
function tgl:in_1_set(args)
	if args[1] == 0 then
		self.on = false
	else
		self.on = true
	end
	self:repaint()
end

-- ──────────────────────────────────────────
function tgl:in_1_float(args)
	if args == 0 then
		self.on = false
	else
		self.on = true
	end
	self:repaint()
	self:outlet(1, "float", { args })
end

-- ──────────────────────────────────────────
function tgl:in_1_list(args)
	if args == 0 then
		self.on = false
	else
		self.on = true
	end
	self:repaint()
	self:outlet(1, "float", { args[1]})
end
-- ──────────────────────────────────────────
function tgl:in_1_bang(_)
	self:mouse_down(0, 0)
end

-- ──────────────────────────────────────────
function tgl:in_1_color(args)
	self.bg_color = args[1]
	self.fg_color = args[2]
	self.label_color = args[3]
end

-- ──────────────────────────────────────────
function tgl:in_1_reload()
	self:dofilex(self._scriptname)
	self:initialize()
end
