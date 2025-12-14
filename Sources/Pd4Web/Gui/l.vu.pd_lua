local vu = pd.Class:new():register("l.vu")

-- ──────────────────────────────────────────
function vu:initialize(_, args)
	self.inlets = 2
	self.outlets = 2
	self.blink_delay = 0
	self.filled = false
	self.need_update_args = false
	if args ~= nil and #args > 0 then
		self.width = args[1] -- 18
		self.height = args[2] -- 160
		self.receive = args[3] -- "empty"
		self.label = args[4] -- "empty"
		self.x_off = args[5] -- -1
		self.y_off = args[6] -- -9
		self.font_style = args[7] -- 0     ← estilo da fonte
		self.font_size = args[8] -- 10    ← tamanho da fonte
		self.bg_color = args[9] -- "#404040"
		self.fg_color = args[10] -- "#000000"
		self.scale = args[11] -- 1
		self.isa = args[12] -- 0
	else
		self.width = 18
		self.height = 160
		self.receive = "empty"
		self.label = "empty"
		self.x_off = -1
		self.y_off = -9
		self.font_style = 0
		self.font_size = 10
		self.bg_color = "#404040"
		self.fg_color = "#000000"
		self.scale = 1
		self.isa = 0
		self.need_update_args = true
	end

	self.clock = pd.Clock:new():register(self, "tick")
	self.clock:delay(0)

	if self.receive ~= "empty" then
		self.receiver = pd.Receive:new():register(self, self.receive, "_receiver")
	end

	self.rms = -101
	self:set_size(self.width, self.height)
	return true
end

-- ──────────────────────────────────────────
function vu:_receiver(sel, atoms)
	if sel == "float" then
		self.rms = atoms[1]
		self:outlet(1, "float", atoms)
	end
end

-- ──────────────────────────────────────────
function vu:update_args()
	local args = {}

	table.insert(args, self.width)
	table.insert(args, self.height)
	table.insert(args, self.receive)
	table.insert(args, self.label)
	table.insert(args, self.x_off)
	table.insert(args, self.y_off)
	table.insert(args, self.font_style)
	table.insert(args, self.font_size)
	table.insert(args, self.bg_color)
	table.insert(args, self.fg_color)
	table.insert(args, self.scale)
	table.insert(args, self.isa)

	self:set_args(args)
end

-- ──────────────────────────────────────────
function vu:tick()
	self:repaint(2)
	self:repaint(3)
	self.clock:delay(30)
end

-- ──────────────────────────────────────────
function vu:in_1_bang(_)
	self:outlet(1, "float", { self.rms })
end

-- ──────────────────────────────────────────
function vu:in_1_float(args)
	self.rms = args
	self:outlet(1, "float", { args })
end

-- ──────────────────────────────────────────
function vu:in_1_list(args)
	self.rms = args[1]
	self:outlet(1, "float", { args[1] })
end

-- ──────────────────────────────────────────
function vu:hex_to_rgb(hex)
	hex = hex:gsub("#", "")
	return {
		tonumber(hex:sub(1, 2), 16),
		tonumber(hex:sub(3, 4), 16),
		tonumber(hex:sub(5, 6), 16),
	}
end

-- ──────────────────────────────────────────
function vu:rms_to_linear(rms_db)
	local min_db, max_db = -100, 12
	if rms_db < min_db then
		rms_db = min_db
	end
	if rms_db > max_db then
		rms_db = max_db
	end
	return (rms_db - min_db) / (max_db - min_db)
end

-- ──────────────────────────────────────────
function vu:paint(g)
	if self.need_update_args then
		self.need_update_args = false
		self:update_args()
	end
	g:set_color(244, 244, 244)
	g:fill_all()
	g:stroke_rect(0, 0, self.width, self.height, 2)
	self:repaint(2)
end

-- ──────────────────────────────────────────
function vu:paint_layer_2(g)
	g:set_color(0, 0, 0)
	g:stroke_rect(0, 0, self.width, self.height, 1)
	local green_base_y = self.height - 3
	g:set_color(80, 235, 80)
	g:fill_rect(1, green_base_y + 1, self.width - 2, 1)
end

-- ──────────────────────────────────────────
function vu:paint_layer_3(g)
	local rms_db = self.rms or -100
	local min_db = -100
	local max_db = 6
	if rms_db <= min_db then
		return
	end

	-- log scale normalization
	local lin_min = 10 ^ (min_db / 20)
	local lin_max = 10 ^ (max_db / 20)
	local lin_val = 10 ^ (rms_db / 20)
	local norm = (lin_val - lin_min) / (lin_max - lin_min)
	norm = math.max(0, math.min(1, norm)) -- clamp [0,1]

	-- visual parameters
	local sq_size = 2
	local spacing = 1
	local total_height = self.height - 4
	local block_unit = sq_size + spacing
	local max_squares = math.floor(total_height / block_unit)
	local squares_to_draw = math.floor(max_squares * norm + 0.5)

	-- color based on normalized value
	local r = math.min(2 * norm, 1)
	local g_val = math.min(2 * (1 - norm), 1)
	g:set_color(80 + r * 155, 80 + g_val * 155, 80)

	local x_pos = 1
	local y_base = self.height - 3

	-- draw blocks from bottom up
	for i = 0, squares_to_draw - 1 do
		local y = y_base - block_unit * i - sq_size
		if y < 1 then
			break
		end
		g:fill_rect(x_pos + 1, y, self.width - 4, sq_size)
	end

	-- green base line
	g:set_color(80, 235, 80)
	g:fill_rect(1, self.height - 3, self.width - 2, 2)
end

-- ──────────────────────────────────────────
function vu:in_1_reload()
	self:dofilex(self._scriptname)
	self:initialize()
end
