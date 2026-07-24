local keyboard = pd.Class:new():register("l.keyboard")

local WHITE_SEMITONES = { 0, 2, 4, 5, 7, 9, 11 }
local BLACK_OFFSETS = { 0, 1, 3, 4, 5 }

local function clamp(value, low, high)
	return math.max(low, math.min(high, value))
end

local function number(value, fallback)
	return type(value) == "number" and value or fallback
end

function keyboard:initialize(_, args)
	self.inlets = 2
	self.outlets = 1
	self.key_w = 17
	self.key_h = 80
	self.octaves = 4
	self.low_c = 2
	self.toggle_mode = false
	self.normal_velocity = 0
	self.send = "empty"
	self.receive = "empty"
	self.input_velocity = 0
	self.notes = {}
	self.active_note = nil

	if args and #args > 0 then
		if type(args[1]) == "number" then
			self.key_w = number(args[1], self.key_w)
			self.key_h = number(args[2], self.key_h)
			self.octaves = number(args[3], self.octaves)
			self.low_c = number(args[4], self.low_c)
			self.toggle_mode = number(args[5], 0) ~= 0
			self.normal_velocity = number(args[6], self.normal_velocity)
			self.send = args[7] or self.send
			self.receive = args[8] or self.receive
		else
			local i = 1
			while i <= #args do
				local flag = args[i]
				if flag == "-tgl" then
					self.toggle_mode = true
					i = i + 1
				elseif
					flag == "-width"
					or flag == "-height"
					or flag == "-oct"
					or flag == "-lowc"
					or flag == "-norm"
					or flag == "-send"
					or flag == "-receive"
				then
					local value = args[i + 1]
					if value == nil then
						self:error("keyboard: missing value for " .. tostring(flag))
						return false
					end
					if flag == "-width" then
						self.key_w = number(value, self.key_w)
					elseif flag == "-height" then
						self.key_h = number(value, self.key_h)
					elseif flag == "-oct" then
						self.octaves = number(value, self.octaves)
					elseif flag == "-lowc" then
						self.low_c = number(value, self.low_c)
					elseif flag == "-norm" then
						self.normal_velocity = number(value, self.normal_velocity)
					elseif flag == "-send" then
						self.send = value
					elseif flag == "-receive" then
						self.receive = value
					end
					i = i + 2
				else
					self:error("keyboard: improper argument " .. tostring(flag))
					return false
				end
			end
		end
	end

	self.key_w = math.max(7, math.floor(self.key_w))
	self.key_h = math.max(10, math.floor(self.key_h))
	self.octaves = clamp(math.floor(self.octaves), 1, 10)
	self.low_c = clamp(math.floor(self.low_c), 0, 8)
	self.normal_velocity = clamp(math.floor(self.normal_velocity), 0, 127)
	self.first_c = self.low_c * 12 + 12
	self:set_size(self.key_w * 7 * self.octaves, self.key_h)
	self:replace_receiver(self.receive)
	return true
end

function keyboard:finalize()
	if self.receiver then
		self.receiver:destruct()
		self.receiver = nil
	end
end

function keyboard:replace_receiver(name)
	if self.receiver then
		self.receiver:destruct()
		self.receiver = nil
	end
	self.receive = name or "empty"
	if self.receive ~= "empty" then
		self.receiver = pd.Receive:new():register(self, self.receive, "_receiver")
	end
end

function keyboard:output(note, velocity)
	self:outlet(1, "list", { note, velocity })
	if self.send ~= "empty" then
		pd.send(self.send, "list", { note, velocity })
	end
end

function keyboard:set_note(note, velocity, output)
	note = math.floor(number(note, -1))
	if note < 0 or note >= 255 then
		return
	end
	velocity = clamp(math.floor(number(velocity, 0)), 0, 127)
	self.notes[note] = velocity
	if output then
		self:output(note, velocity)
	end
end

function keyboard:velocity_at(y, black)
	if self.normal_velocity > 0 then
		return self.normal_velocity
	end
	local height = black and self.key_h * 2 / 3 or self.key_h
	return clamp(math.floor((y / height) * 127), 1, 127)
end

function keyboard:note_at(x, y)
	if x < 0 or x >= self.key_w * 7 * self.octaves or y < 0 or y > self.key_h then
		return nil
	end
	local white_index = math.floor(x / self.key_w)
	local octave = math.floor(white_index / 7)
	local white = white_index % 7
	local local_x = x - octave * 7 * self.key_w
	local black_half_width = self.key_w / 3

	if y < self.key_h * 2 / 3 then
		for _, offset in ipairs(BLACK_OFFSETS) do
			local center = (offset + 1) * self.key_w
			if local_x >= center - black_half_width and local_x < center + black_half_width then
				local semitone = WHITE_SEMITONES[offset + 1] + 1
				return self.first_c + octave * 12 + semitone, self:velocity_at(y, true)
			end
		end
	end

	return self.first_c + octave * 12 + WHITE_SEMITONES[white + 1], self:velocity_at(y, false)
end

function keyboard:mouse_down(x, y)
	local note, velocity = self:note_at(x, y)
	if not note then
		return
	end
	if self.toggle_mode then
		velocity = (self.notes[note] or 0) > 0 and 0 or velocity
		self:set_note(note, velocity, true)
	else
		self.active_note = note
		self:set_note(note, velocity, true)
	end
	self:repaint()
end

function keyboard:mouse_drag(x, y)
	if self.toggle_mode then
		return
	end
	local note, velocity = self:note_at(x, y)
	if not note or note == self.active_note then
		return
	end
	if self.active_note then
		self:set_note(self.active_note, 0, true)
	end
	self.active_note = note
	self:set_note(note, velocity, true)
	self:repaint()
end

function keyboard:mouse_up(_, _)
	if not self.toggle_mode and self.active_note then
		self:set_note(self.active_note, 0, true)
		self.active_note = nil
		self:repaint()
	end
end

function keyboard:in_2_float(value)
	self.input_velocity = clamp(math.floor(number(value, 0)), 0, 127)
end

function keyboard:in_1_float(value)
	self:set_note(value, self.input_velocity, true)
	self:repaint()
end

function keyboard:in_1_list(args)
	if args[2] ~= nil then
		self.input_velocity = args[2]
	end
	self:in_1_float(args[1])
end

function keyboard:in_1_set(args)
	self:set_note(args[1], args[2], false)
	self:repaint()
end

function keyboard:in_1_on(args)
	for _, note in ipairs(args) do
		self:set_note(note, 127, true)
	end
	self:repaint()
end

function keyboard:in_1_off(args)
	for _, note in ipairs(args) do
		self:set_note(note, 0, true)
	end
	self:repaint()
end

function keyboard:in_1_flush()
	for note, velocity in pairs(self.notes) do
		if velocity > 0 then
			self:set_note(note, 0, true)
		end
	end
	self:repaint()
end

function keyboard:in_1_play(args)
	self:in_1_flush()
	self:in_1_on(args)
end

function keyboard:in_1_show(args)
	for note in pairs(self.notes) do
		self.notes[note] = 0
	end
	for _, note in ipairs(args) do
		self:set_note(note, 127, false)
	end
	self:repaint()
end

function keyboard:update_geometry()
	self.first_c = self.low_c * 12 + 12
	self:set_size(self.key_w * 7 * self.octaves, self.key_h)
	self:repaint()
end

function keyboard:in_1_width(args)
	self.key_w = math.max(7, math.floor(number(args[1], self.key_w)))
	self:update_geometry()
end

function keyboard:in_1_height(args)
	self.key_h = math.max(10, math.floor(number(args[1], self.key_h)))
	self:update_geometry()
end

function keyboard:in_1_8ves(args)
	self.octaves = clamp(math.floor(number(args[1], self.octaves)), 1, 10)
	self:update_geometry()
end

function keyboard:in_1_lowc(args)
	self.low_c = clamp(math.floor(number(args[1], self.low_c)), 0, 8)
	self:update_geometry()
end

function keyboard:in_1_oct(args)
	self.low_c = clamp(self.low_c + math.floor(number(args[1], 0)), 0, 8)
	self:update_geometry()
end

function keyboard:in_1_toggle(args)
	self.toggle_mode = number(args[1], 0) ~= 0
end

function keyboard:in_1_norm(args)
	self.normal_velocity = clamp(math.floor(number(args[1], 0)), 0, 127)
end

function keyboard:in_1_send(args)
	self.send = args[1] or "empty"
end

function keyboard:in_1_receive(args)
	self:replace_receiver(args[1])
end

function keyboard:_receiver(selector, atoms)
	local method = self["in_1_" .. selector]
	if not method then
		return
	end
	if selector == "float" then
		method(self, atoms[1])
	else
		method(self, atoms)
	end
end

function keyboard:paint(g)
	g:set_color(255, 255, 255)
	g:fill_all()

	for octave = 0, self.octaves - 1 do
		local base = octave * 7 * self.key_w
		for white = 0, 6 do
			local note = self.first_c + octave * 12 + WHITE_SEMITONES[white + 1]
			if (self.notes[note] or 0) > 0 then
				g:set_color(196, 0, 0)
			elseif note == 60 then
				g:set_color(122, 222, 255)
			else
				g:set_color(255, 255, 255)
			end
			g:fill_rect(base + white * self.key_w, 0, self.key_w, self.key_h)
			g:set_color(0, 0, 0)
			g:stroke_rect(base + white * self.key_w, 0, self.key_w, self.key_h, 1)
		end
	end

	local black_width = self.key_w * 2 / 3
	local black_height = self.key_h * 2 / 3
	for octave = 0, self.octaves - 1 do
		local base = octave * 7 * self.key_w
		for _, offset in ipairs(BLACK_OFFSETS) do
			local note = self.first_c + octave * 12 + WHITE_SEMITONES[offset + 1] + 1
			local x = base + (offset + 1) * self.key_w - black_width / 2
			g:set_color((self.notes[note] or 0) > 0 and 255 or 0, 0, 0)
			g:fill_rect(x, 0, black_width, black_height)
			g:set_color(0, 0, 0)
			g:stroke_rect(x, 0, black_width, black_height, 1)
		end
	end
end

function keyboard:in_1_reload()
	self:dofilex(self._scriptname)
	self:initialize()
end
