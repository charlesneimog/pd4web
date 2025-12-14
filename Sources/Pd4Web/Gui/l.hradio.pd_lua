local hradio = pd.Class:new():register("l.hradio")

-- ──────────────────────────────────────────
function hradio:initialize(_, args)
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

	self:set_size(self.size * self.number, self.size)
	self.selected = self.init or self.default_value or 0

	return true
end

-- ──────────────────────────────────────────
function hradio:postinitialize()
	if self.init == 1 then
		self:outlet(1, "float", { self.default_value })
		if self.send ~= "empty" then
			pd.send(self.send, "float", { self.default_value })
		end
	end

	self.recv = nil
	if self.receive ~= "empty" then
		self.recv = pd.Receive:new():register(self, self.receive, "receive_callback")
	end
end

-- ──────────────────────────────────────────
function hradio:receive_callback(sel, atoms)
	if sel == "float" then
		local f = atoms[1]
		self.selected = f
		self:outlet(1, "float", { f })
		pd.send(self.send, "float", { f })
		self:repaint(2)
	end
end

-- ──────────────────────────────────────────
function hradio:mouse_down(x, y)
	local pos = math.floor(x / self.size)
	self.selected = pos
	self:outlet(1, "float", { pos })
	if self.send ~= "empty" then
		pd.send(self.send, "float", { self.selected })
	end
	self:repaint(2)
end

-- ──────────────────────────────────────────
function hradio:in_1_send(args)
	self.send = args[1]
end

-- ──────────────────────────────────────────
function hradio:in_1_receive(args)
	self.receive = args[1]
	if self.receive ~= "empty" then
		self.recv = pd.Receive:new():register(self, self.receive, "receive_callback")
	end
end

-- ──────────────────────────────────────────
function hradio:in_1_color(args)
	if args[1][1] == "#" or args[2][1] == "#" or args[3][1] == "#" then
		self:error("There is at least one invalid color")
		return
	end

	self.bg_color = args[1]
	self.fg_color = args[2]
	self.laber_color = args[3]
	self:repaint()
end

-- ──────────────────────────────────────────
function hradio:in_1_size(args)
	self.size = args[1]
	self:set_size(self.size, self.size * self.number)
	self:repaint()
end

-- ──────────────────────────────────────────
function hradio:in_1_bang()
	self:outlet("1", "float", { self.selected })
	if self.send ~= "empty" then
		pd.send(self.send, "float", { self.selected })
	end
end

-- ──────────────────────────────────────────
function hradio:in_1_float(args)
	local pos = args
	self.selected = pos
	if self.send ~= "empty" then
		pd.send(self.send, "float", { self.selected })
	end
	self:outlet(1, "float", { self.selected })
	self:repaint(2)
end

-- ──────────────────────────────────────────
function hradio:in_1_list(args)
	local pos = args[1]
	self.selected = pos
	if self.send ~= "empty" then
		pd.send(self.send, "float", { self.selected })
	end
	self:outlet(1, "float", { self.selected })
	self:repaint(2)
end

-- ──────────────────────────────────────────
function hradio:hex_to_rgb(hex)
	hex = hex:gsub("#", "")
	return {
		tonumber(hex:sub(1, 2), 16),
		tonumber(hex:sub(3, 4), 16),
		tonumber(hex:sub(5, 6), 16),
	}
end

-- ──────────────────────────────────────────
function hradio:paint(g)
	local width, height = self:get_size()
	g:set_color(table.unpack(self:hex_to_rgb(self.bg_color)))
	g:fill_all()

	g:set_color(table.unpack(self:hex_to_rgb(self.fg_color)))
	for i = 0, self.number - 1 do
		g:stroke_rect(i * self.size, 0, self.size, self.size, 1)
	end
end

-- ──────────────────────────────────────────
function hradio:paint_layer_2(g)
	g:fill_rect(4 + (self.selected * self.size), 4, self.size - 8, self.size - 8, 1)
end

-- ──────────────────────────────────────────
function hradio:in_1_reload()
	self:dofilex(self._scriptname)
	self:initialize()
end
