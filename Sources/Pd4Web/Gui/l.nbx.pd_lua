local nbx = pd.Class:new():register("l.nbx")

-- ──────────────────────────────────────────
function nbx:initialize(_, args)
	self.inlets = 1
	self.outlets = 1
	self.select = false
	self.number = "0"
	self.needclear = false
	self.recv = nil

	-- defaults
	self.digs_len = 5
	self.height = 14
	self.lower = -1e+37
	self.upper = 1e+37
	self.liner = 0
	self.init = 0
	self.send = "empty"
	self.receive = "empty"
	self.label = ""
	self.x_off = 0
	self.y_off = -8
	self.font_style = 0
	self.font = "sys_font"
	self.fontsize = 12
	self.bg_color = "#ffffff"
	self.fg_color = "#000000"
	self.label_color = "#000000"
	self.default_value = 0
	self.steady_on_click = false
	self.log_height = 256

	if args and #args >= 17 then
		self.digs_len = args[1] or self.digs_len -- argv[0]
		self.height = args[2] or self.height -- argv[1]
		self.lower = args[3] or self.lower -- argv[2]
		self.upper = args[4] or self.upper -- argv[3]
		self.liner = (args[5] ~= 0) and 1 or 0 -- argv[4]
		self.init = args[6] or self.init -- argv[6]
		self.send = args[7] or self.send -- argv[7]
		self.receive = args[8] or self.receive -- argv[8]
		self.label = args[9] or self.label -- argv[9]
		self.x_off = args[10] or self.x_off -- argv[10]
		self.y_off = args[11] or self.y_off -- argv[11]
		self.font_style = args[12] or self.font_style -- argv[12]
		self.fontsize = args[13] or self.fontsize -- argv[13]
		self.bg_color = args[14] or self.bg_color -- argv[14]
		self.fg_color = args[15] or self.fg_color -- argv[15]
		self.label_color = args[16] or self.label_color -- argv[16]
		self.default_value = tonumber(args[17]) or self.default_value -- argv[17]
		if #args >= 18 then
			self.steady_on_click = (args[19] ~= 0)
		end
	end

	-- receptor
	if self.receive ~= "empty" then
		self.recv = pd.Receive:new():register(self, self.receive, "receive_value")
	end

	self.keyrecv = pd.Receive:new():register(self, "#key", "keydown")

	-- enviar valor inicial
	if self.send ~= "empty" then
		pd.send(self.send, "float", { self.default_value })
	end

	self.number = tostring(self.default_value)
	self:set_size(self.digs_len * self.fontsize, self.height)
	return true
end

-- ──────────────────────────────────────────
function nbx:keydown(sel, atoms)
	if not self.select then
		return
	end

	local key_num = atoms[1]
	local key
	if key_num == 13 or key_num == 10 then
		key = "Enter"
	else
		key = string.char(key_num)
	end

	if self.needclear then
		self.number = ""
		self.needclear = false
	end

	if key == "Enter" then
		self.select = false
		local val = tonumber(self.number) or 0
		if self.send ~= "empty" then
			pd.send(self.send, "float", { val })
		end
		self:outlet(1, "float", { val })
	elseif key == "Backspace" or key == "Delete" then
		self.number = self.number:sub(1, -2)
	elseif key == "." then
		self.number = self.number .. key
	elseif tonumber(key) then
		self.number = self.number .. tostring(key)
	else
		return
	end

	self:repaint()
end

-- ──────────────────────────────────────────
function nbx:finalize()
	if self.recv then
		self.recv:destruct()
	end
end

-- ──────────────────────────────────────────
function nbx:mouse_down(x, y)
	self.select = not self.select
	self.needclear = true
	self:repaint()
end

-- ──────────────────────────────────────────
function nbx:key_down(_, _, key) end

-- ──────────────────────────────────────────
function nbx:in_1_bang(val)
	if self.send ~= "empty" then
		pd.send(self.send, "float", { tonumber(self.number) })
	end
	self:outlet(1, "float", { tonumber(self.number) })
end

-- ──────────────────────────────────────────
function nbx:in_1_float(val)
	self.number = tostring(val)
	if self.send ~= "empty" then
		pd.send(self.send, "float", { val })
	end
	self:outlet(1, "float", { val })
	self:repaint()
end

-- ──────────────────────────────────────────
function nbx:in_1_symbol(_) end

-- ──────────────────────────────────────────
function nbx:in_1_list(args)
	if #args < 1 then
		return
	end
	local val = tonumber(args[1])
	self.number = tostring(val)
	if self.send ~= "empty" then
		pd.send(self.send, "float", { val })
	end
	self:outlet(1, "float", { val })
	self:repaint()
end

-- ──────────────────────────────────────────
function nbx:receive_value(sel, atoms)
	if #atoms < 1 then
		return
	end
	local val = atoms[1]
	self.number = tostring(val)
	if self.send ~= "empty" then
		pd.send(self.send, "float", { val })
	end
	self:outlet(1, "float", { val })
	self:repaint()
end

-- ──────────────────────────────────────────
function nbx:paint(g)
	local width, height = self:get_size()

	g:set_color(255, 255, 255)
	g:fill_all()

	g:set_color(0, 0, 0)
	g:stroke_rect(0, 0, width, height, 1)

	-- triangle
	local p = Path(0, 0)
	p:line_to(8, height / 2)
	p:line_to(0, height)
	g:stroke_path(p, 1)

	-- text
	if self.select then
		g:set_color(255, 0, 0)
	else
		g:set_color(0, 0, 0)
	end

	local number_str = tostring(self.number)

	if number_str:find("%.") then
		number_str = number_str:gsub("0+$", ""):gsub("%.$", "")
		number_str = string.sub(number_str, 1, self.digs_len)
	else
		if #number_str > self.digs_len then
			number_str = "+"
		end
	end

	g:draw_text(number_str, 10, height / 12, self.digs_len * self.fontsize, self.fontsize)
end

-- ──────────────────────────────────────────
function nbx:in_1_reload()
	self:dofilex(self._scriptname)
	self:initialize()
end
