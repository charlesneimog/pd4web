local floatatom = pd.Class:new():register("floatatom")

-- ─────────────────────────────────────
function floatatom:initialize(_, args)
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
	self.needclear = false
	self.number = "0"

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
function floatatom:keyreceiver(_, atoms)
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
		if self.send ~= "-" then
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
		pd.send(self.send, "float", tonumber(self.number))
	end
	self:outlet(1, "float", { tonumber(self.number) })
end

-- ─────────────────────────────────────
function floatatom:in_1_float(args)
	self.number = tostring(args)
	self:outlet(1, "float", { args })
	if self.send ~= "-" then
		pd.send(self.send, "float", args[1])
	end
	self:repaint()
end

-- ─────────────────────────────────────
function floatatom:in_1_list(args)
	self.number = tostring(args[1])
	if self.send ~= "-" then
		pd.send(self.send, "float", tonumber(self.number))
	end
	self:outlet(1, "float", { args[1] })
	self:repaint()
end

-- ─────────────────────────────────────
function floatatom:in_1_symbol(_)
	self.number = "0"
	self:outlet(1, "float", { 0 })
	if self.send ~= "-" then
		pd.send(self.send, "float", 0)
	end
	self:repaint()
end

-- ─────────────────────────────────────
function floatatom:paint(g)
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

	local number_str = tostring(self.number)
	if number_str:find("%.") then
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

	g:draw_text(number_str, self.padding / 2, self.padding, self.fontsize * self.width, self.fontsize)
end

-- ─────────────────────────────────────
function floatatom:in_1_reload()
	self:dofilex(self._scriptname)
	self.keysreceiver:destruct()
	self:initialize("floatatom", self.args)
end
