local keyboard = pd.Class:new():register("l.keyboard")

-- ──────────────────────────────────────────
function keyboard:initialize(name, args)
	self.inlets = 2
	self.outlets = 1
	self.octaves = 2
	self.key_h = 70
	self.key_w = math.floor(self.key_h * 0.25)
	self.lower_note = 48 -- C3
	self:set_size(7 * self.octaves * self.key_w, self.key_h)

	-- semitons reais de cada tecla branca (C, D, E, F, G, A, B)
	self.white_semitones = { 0, 2, 4, 5, 7, 9, 11 }
	-- offsets fixos para a posição visual das pretas (entre brancas)
	self.black_key_offsets = { 0, 1, 3, 4, 5 }

	-- popula ALL keys UMA ÚNICA VEZ, com o midi correto
	self.keys = {}
	for octave = 0, self.octaves - 1 do
		-- brancas
		for i = 0, 6 do
			local id = "w" .. (octave * 7 + i)
			local semitone = self.white_semitones[i + 1]
			local midi = self.lower_note + 12 * octave + semitone
			self.keys[id] = {
				midi = midi,
				velocity = 0,
				id = id,
				note = self:get_note_name(midi),
				pressed = false,
			}
		end
		-- pretas
		for j, off in ipairs(self.black_key_offsets) do
			local id = "b" .. (octave * 5 + j - 1)
			local semitone = self.white_semitones[off + 1] + 1
			local midi = self.lower_note + 12 * octave + semitone
			self.keys[id] = {
				midi = midi,
				velocity = 0,
				id = id,
				note = self:get_note_name(midi),
				pressed = false,
			}
		end
	end

	self.active_key = nil
	return true
end

-- ──────────────────────────────────────────
function keyboard:get_note_name(midi)
	local names = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }
	local name = names[(midi % 12) + 1]
	local oct = math.floor(midi / 12) - 1
	return name .. oct
end

-- ──────────────────────────────────────────
function keyboard:mouse_down(x, y)
	self.active_key = self:hit_test(x, y, true)
	self:repaint()
end

function keyboard:mouse_up(x, y)
	if self.active_key then
		local key = self.keys[self.active_key]
		self.keys[self.active_key].pressed = false
		self.active_key = nil
		key.velocity = 0
		self:outlet(1, "list", { key.midi, key.velocity })
	end
	self:repaint()
end

function keyboard:mouse_drag(x, y)
	self.active_key = self:hit_test(x, y, true)
	self:repaint()
end

-- ──────────────────────────────────────────
function keyboard:in_1_list(args)
	local midi, vel = args[1], args[2]
	for _, key in pairs(self.keys) do
		if key.midi == midi then
			key.pressed = (vel > 0)
			key.velocity = vel
			self:outlet(1, "list", { key.midi, key.velocity })
		end
	end
	self:repaint()
end

-- ──────────────────────────────────────────
function keyboard:hit_test(x, y, on_press)
	local w = self.key_w
	local h = self.key_h
	local bw = w * 0.6
	local bh = h * 0.6

	for k, v in pairs(self.keys) do
		v.pressed = false
	end

	-- pretas primeiro (mesma posição visual antiga)
	for octave = 0, self.octaves - 1 do
		local base = octave * 7 * w
		for j, off in ipairs(self.black_key_offsets) do
			local id = "b" .. (octave * 5 + j - 1)
			local bx = base + (off + 1) * w - bw / 2
			if x >= bx and x < bx + bw and y >= 0 and y <= bh then
				local key = self.keys[id]
				local vel = math.floor(127 * (1 - (y / bh)))
				key.velocity = math.max(0, math.min(127, vel))
				key.pressed = on_press
				self:outlet(1, "list", { key.midi, key.velocity })
				return id
			end
		end
	end

	-- brancas (igual antes)
	for octave = 0, self.octaves - 1 do
		local base = octave * 7 * w
		for i = 0, 6 do
			local id = "w" .. (octave * 7 + i)
			local wx = base + i * w
			if x >= wx and x < wx + w and y >= 0 and y <= h then
				local key = self.keys[id]
				local vel = math.floor(127 * (1 - (y / h)))
				key.velocity = math.max(0, math.min(127, vel))
				key.pressed = on_press
				self:outlet(1, "list", { key.midi, key.velocity })
				return id
			end
		end
	end

	return nil
end

-- ──────────────────────────────────────────
function keyboard:paint(g)
	local w = self.key_w
	local h = self.key_h
	local bw = w * 0.6
	local bh = h * 0.6

	-- fundo
	g:set_color(255, 255, 255)
	g:fill_all()

	-- brancas
	for octave = 0, self.octaves - 1 do
		local base = octave * 7 * w
		for i = 0, 6 do
			local id = "w" .. (octave * 7 + i)
			local key = self.keys[id]
			if key.pressed then
				g:set_color(255, 0, 0)
			else
				g:set_color(255, 255, 255)
			end
			g:fill_rect(base + i * w, 0, w, h)
			g:set_color(0, 0, 0)
			g:stroke_rect(base + i * w, 0, w, h, 1)
		end
	end

	-- pretas
	for octave = 0, self.octaves - 1 do
		local base = octave * 7 * w
		for j, off in ipairs(self.black_key_offsets) do
			local id = "b" .. (octave * 5 + j - 1)
			local key = self.keys[id]
			local bx = base + (off + 1) * w - bw / 2
			if key.pressed then
				g:set_color(255, 0, 0)
			else
				g:set_color(0, 0, 0)
			end
			g:fill_rect(bx, 0, bw, bh)
			g:set_color(0, 0, 0)
			g:stroke_rect(bx, 0, bw, bh, 1)
		end
	end
end

-- ──────────────────────────────────────────
function keyboard:in_1_reload()
	self:dofilex(self._scriptname)
	self:initialize()
end
