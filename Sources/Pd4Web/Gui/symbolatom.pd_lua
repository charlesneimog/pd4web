local symbolatom = pd.Class:new():register("symbolatom")

-- ─────────────────────────────────────
function symbolatom:initialize(_, args)
	self.inlets = 1
	self.outlets = 1
	self.args = args
	self.width = 5
	self.labelpos = 4
	self.idontknow = 0
	self.label = "-"
	self.receive = "-"
	self.send = "-"
	self.fontsize = 12

	self.select = false
	self.needtoreset = false
	self.symbol = ""

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
function symbolatom:keyreceiver(_, atoms)
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
	elseif key_num == 46 or key_num == 44 then
		key = "." -- dot
	else
		if key_num < 32 or key_num > 126 then
			return
		end
		key = string.char(key_num)
	end

	if self.needtoreset then
		self.symbol = ""
		self.needtoreset = false
	end

	if key == "Enter" then
		self.select = false
		local val = tonumber(self.symbol) or 0
		if self.send ~= "-" then
			pd.send(self.send, "symbol", { val })
		end
		self:outlet(1, "symbol", { val })
	elseif key == "Backspace" or key == "Delete" then
		self.symbol = self.symbol:sub(1, -2)
	else
		self.symbol = self.symbol .. key
	end
	self:repaint()
end

-- ─────────────────────────────────────
function symbolatom:mouse_down(_, _)
	self.select = true
	self.needtoreset = true
	self:repaint()
end

-- ─────────────────────────────────────
function symbolatom:in_1_bang(_)
	self:outlet(1, "symbol", { self.symbol })
	if self.send ~= "-" then
		self:outlet(1, "symbol", { self.symbol })
	end
end

-- ─────────────────────────────────────
function symbolatom:in_1_float(args)
	self.symbol = "float"
	self:outlet(1, "symbol", { args })
	if self.send ~= "-" then
		pd.send(self.send, "symbol", args[1])
	end
	self:repaint()
end

-- ─────────────────────────────────────
function symbolatom:in_1_list(args)
	if tonumber(args[1]) then
		self.symbol = "float"
	else
		self.symbol = tostring(args[1])
	end

	self:outlet(1, "symbol", { self.symbol })
	if self.send ~= "-" then
		pd.send(self.send, "symbol", self.symbol)
	end
	self:repaint()
end

-- ─────────────────────────────────────
function symbolatom:in_1_symbol(args)
	self.symbol = args
	self:outlet(1, "symbol", { args })
	if self.send ~= "-" then
		pd.send(self.send, "symbol", args)
	end
	self:repaint()
end

-- ─────────────────────────────────────
function symbolatom:paint(g)
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

	g:draw_text(self.symbol, self.padding / 2, self.padding, self.fontsize * self.width, self.fontsize)
end

-- ─────────────────────────────────────
function symbolatom:in_1_reload()
	self:dofilex(self._scriptname)
	self.keysreceiver:destruct()
	self:initialize("floatatom", self.args)
end
