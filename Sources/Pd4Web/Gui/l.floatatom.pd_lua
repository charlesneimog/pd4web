local floatatom = pd.Class:new():register("l.floatatom")

-- ─────────────────────────────────────
function floatatom:initialize(_, args)
	self.inlets = 1
	self.outlets = 1
	self.args = args or {}
	self.width = 5
	self.draglo = 0
	self.draghi = 0
	self.wherelabel = 0
	self.label = "-"
	self.receive = "-"
	self.send = "-"
	self.fontsize = 12
	self.padding = 2

	self.select = false
	self.needclear = false
	self.number = "0"
	self.keysreceiver = nil
	self.receiver = nil

	if args and #args > 0 then
		self.width = tonumber(args[1]) or self.width
		self.draglo = tonumber(args[2]) or self.draglo
		self.draghi = tonumber(args[3]) or self.draghi
		self.wherelabel = tonumber(args[4]) or self.wherelabel
		self.label = args[5] or self.label
		self.receive = args[6] or self.receive
		self.send = args[7] or self.send
		self.fontsize = tonumber(args[8]) or self.fontsize
		if self.fontsize == 0 then
			self.fontsize = 12
		end
	end

	self.keysreceiver = pd.Receive:new():register(self, "#key", "keyreceiver")
	if self.receive ~= "-" and self.receive ~= "empty" then
		self.receiver = pd.Receive:new():register(self, self.receive, "receive_value")
	end

	local object_width = math.max(1, self.width) * self.fontsize * 0.65
	local object_height = self.fontsize + (2 * self.padding)
	self:set_size(object_width, object_height)
	return true
end

-- ─────────────────────────────────────
function floatatom:finalize()
	if self.keysreceiver then
		self.keysreceiver:destruct()
		self.keysreceiver = nil
	end
	if self.receiver then
		self.receiver:destruct()
		self.receiver = nil
	end
end

-- ─────────────────────────────────────
function floatatom:keyreceiver(_, atoms)
    	if not self.select then
		return
	end

	local key_num = atoms[1]
	local key
	if key_num == 13 or key_num == 10 then
		key = "Enter"
	elseif key_num == 8 then
		key = "Backspace"
	elseif key_num == 46 or key_num == 110 or key_num == 190 or key_num == 44 or key_num == 188 then
		key = "."
	elseif key_num == 45 or key_num == 109 or key_num == 189 then
		key = "-"
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
		if self.send ~= "-" then
			pd.send(self.send, "float", { val })
		end
		self:outlet(1, "float", { val })
	elseif key == "Backspace" or key == "Delete" then
		self.number = self.number:sub(1, -2)
	elseif key == "." then
		if not self.number:find(".", 1, true) then
			self.number = self.number .. key
		end
	elseif key == "-" then
		if self.number == "" then
			self.number = key
		end
	elseif tonumber(key) then
		self.number = self.number .. tostring(key)
	else
		return
	end

	self:repaint()
end

-- ─────────────────────────────────────
function floatatom:mouse_down(_, _)
	self.select = not self.select
    if not self.select then
        self:in_1_bang()
    end
	self.needclear = true
	self.needtoreset = true
	self:repaint()
end

-- ─────────────────────────────────────
function floatatom:in_1_bang(_)
	if self.send ~= "-" then
		pd.send(self.send, "float", { tonumber(self.number) or 0 })
	end
	self:outlet(1, "float", { tonumber(self.number) or 0 })
end

-- ─────────────────────────────────────
function floatatom:in_1_float(value)
	self.number = tostring(value)
	self:outlet(1, "float", { value })
	if self.send ~= "-" then
		pd.send(self.send, "float", { value })
	end
	self:repaint()
end

-- ─────────────────────────────────────
function floatatom:in_1_list(args)
	self.number = tostring(args[1])
	if self.send ~= "-" then
		pd.send(self.send, "float", { tonumber(self.number) or 0 })
	end
	self:outlet(1, "float", { args[1] })
	self:repaint()
end

-- ─────────────────────────────────────
function floatatom:in_1_symbol(_)
	self.number = "0"
	self:outlet(1, "float", { 0 })
	if self.send ~= "-" then
		pd.send(self.send, "float", { 0 })
	end
	self:repaint()
end

-- ─────────────────────────────────────
function floatatom:receive_value(sel, atoms)
	if sel ~= "float" or #atoms < 1 then
		return
	end
	self.number = tostring(atoms[1])
	self:repaint()
end

-- ─────────────────────────────────────
function floatatom:paint(g)
	local w, h = self:get_size()
	g:set_color(255, 255, 255)
	g:fill_all()

	g:set_color(0, 0, 0)
	g:stroke_rect(0, 0, w, h, 1)

	if self.select then
		g:set_color(255, 0, 0)
	else
		g:set_color(0, 0, 0)
	end

	local number_str = tostring(self.number)
	-- Preserve incomplete input such as "1." and trailing zeroes while editing.
	-- Normalization is only appropriate after the value has been committed.
	if not self.select and number_str:find("%.") then
		number_str = number_str:gsub("0+$", ""):gsub("%.$", "")
	end

	if number_str == "" then
		number_str = "0"
	end
	local int_part, frac_part = number_str:match("^(%-?%d+)%.(%d+)$")
	local is_float = frac_part ~= nil
	local int_len = int_part and #int_part or #number_str

	if int_len > self.width then
		number_str = string.sub(int_part or number_str, 1, self.width - 1) .. ">"
	else
		if #number_str > self.width then
			if is_float then
				number_str = string.sub(number_str, 1, self.width)
			else
				number_str = string.sub(number_str, 1, self.width - 1) .. ">"
			end
		end
	end

	-- Inter's digit shapes sit slightly below the center of their em box.
	local text_y = math.max(0, math.floor((h - self.fontsize) / 2) - 1)
	g:draw_text(number_str, self.padding / 2, text_y, self.fontsize * self.width, self.fontsize)
end

-- ─────────────────────────────────────
function floatatom:in_1_reload()
	self:dofilex(self._scriptname)
	self:finalize()
	self:initialize("l.floatatom", self.args)
end
