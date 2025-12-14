local listatom = pd.Class:new():register("listatom")

-- ─────────────────────────────────────
function listatom:initialize(name, args)
	self.inlets = 1
	self.outlets = 1
	self.args = args
	self.width = 20
	self.labelpos = 4
	self.idontknow = 0
	self.label = "-"
	self.receive = "-"
	self.send = "-"
	self.fontsize = 12

	self.atoms = {}

	self.select = false
	self.needtoreset = false
	self.atom = ""

	if #args > 0 then
		self.width = args[1]
		self.draglo = args[2]
		self.draghi = args[3]
		self.wherelabel = args[4]
		self.label = args[5]
		self.receive = args[6]
		self.sende = args[7]
		self.fontsize = args[8]
		if self.fontsize == 0 then
			self.fontsize = 12
		end
	end

	self.keysreceiver = pd.Receive:new():register(self, "#key", "keyreceiver")
	self.padding = 2
	self:set_size(self.width * self.fontsize * 0.65, self.fontsize + (2 * self.padding))
	return true
end

-- ─────────────────────────────────────
function listatom:keyreceiver(sel, atoms)
	if not self.select then
		return
	end
	local key_num = atoms[1]
	-- ignore key-up events
	if key_num == 0 then
		return
	end
	local key
	if key_num == 13 or key_num == 10 then
		key = "Enter"
	elseif key_num == 8 then
		key = "Backspace"
	elseif key_num == 46 then
		key = "Delete"
	elseif key_num == 32 then
		key = "Space"
	else
		if key_num < 32 or key_num > 126 then
			return
		end
		key = string.char(key_num)
	end

	if self.needtoreset then
		self.atom = ""
		self.atoms = {}
		self.needtoreset = false
	end

	if key == "Enter" then
		self.select = false
		if self.send ~= "-" then
			pd.send(self.send, "list", self.atoms)
		end
		self:outlet(1, "list", self.atoms)
	elseif key == "Space" then
		self.atoms[#self.atoms + 1] = self.atom
		self.atom = ""
	elseif key == "Backspace" or key == "Delete" then
		self.atom = self.atom:sub(1, -2)
	else
		self.atom = self.atom .. key
	end
	self:repaint()
end

-- ─────────────────────────────────────
function listatom:mouse_down(x, y)
	self.select = true
	self.needtoreset = true
	self:repaint()
end

-- ─────────────────────────────────────
function listatom:in_1_bang(_)
	if self.send ~= "-" then
		pd.send(self.send, "list", self.atoms)
	end
	self:outlet(1, "list", self.atoms)
	self:repaint()
end

-- ─────────────────────────────────────
function listatom:in_1_float(args)
	self.atoms = { args }
	if self.send ~= "-" then
		pd.send(self.send, "list", self.atoms)
	end
	self:outlet(1, "list", self.atoms)
	self:repaint()
end

-- ─────────────────────────────────────
function listatom:in_1_list(args)
	self.atoms = args
	if self.send ~= "-" then
		pd.send(self.send, "list", args)
	end
	self:outlet(1, "list", args)
	self:repaint()
end

-- ─────────────────────────────────────
function listatom:in_1_symbol(args)
	self.atoms = { args }
	if self.send ~= "-" then
		pd.send(self.send, "list", self.atoms)
	end
	self:outlet(1, "list", self.atoms)
	self:repaint()
end

-- ─────────────────────────────────────
function listatom:paint(g)
	local w, h = self:get_size()
	g:set_color(0)
	g:fill_all()

	g:set_color(1)
	g:stroke_rect(0, 0, w, h, 1)

	if self.select then
		g:set_color(255, 0, 0)
	else
		g:set_color(1)
	end

	local str = ""
	for k, v in pairs(self.atoms) do
		local atom_str = tostring(v)
		local int_part, frac_part = atom_str:match("^(%-?%d+)%.?(%d*)$")
		if frac_part ~= nil and frac_part == "0" then
			atom_str = int_part
		end
		str = str .. atom_str .. " "
	end

	if #self.atoms == 0 and #self.atom ~= 0 then
		str = self.atom
	elseif #self.atoms ~= 0 and #self.atom == 0 then
		str = str
	else
		str = str .. self.atom
	end

	g:draw_text(str, self.padding / 2, self.padding, self.fontsize * self.width, self.fontsize)
end

-- ─────────────────────────────────────
function listatom:in_1_reload()
	self:dofilex(self._scriptname)
	self.keysreceiver:destruct()
	self:initialize("floatatom", self.args)
end
