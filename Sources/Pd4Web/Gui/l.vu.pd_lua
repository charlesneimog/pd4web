local vu = pd.Class:new():register("l.vu")

local VU_STEPS = 40
local VU_MIN_DB = -100
local VU_MAX_DB = 12
local VU_BOTTOM_PADDING = 3

-- First dB value displayed by each of Pd's 40 VU steps.  These are the
-- transition points encoded by iemgui_vu_db2i in g_all_guis.c.
local VU_THRESHOLDS = {
	-100, -80, -60, -55, -50, -45, -40, -35, -30, -27.5,
	-25, -22.5, -20, -18, -16, -14, -12, -10.5, -9, -7.5,
	-6, -5, -4, -3, -2, -1.5, -1, -0.5, 0, 0.5,
	1, 1.5, 2, 3, 4, 5, 6, 7.5, 9, 12,
}

function vu:initialize(_, args)
	self.inlets = 2
	self.outlets = 2
	self.need_update_args = false

	if args ~= nil and #args > 0 then
		self.width = args[1]
		self.height = args[2]
		self.receive = args[3]
		self.label = args[4]
		self.x_off = args[5]
		self.y_off = args[6]
		self.font_style = args[7]
		self.font_size = args[8]
		self.bg_color = args[9]
		self.fg_color = args[10]
		self.scale = args[11]
		self.isa = args[12] or 0
	else
		self.width = 18
		self.height = 160
		self.receive = "empty"
		self.label = "empty"
		self.x_off = -1
		self.y_off = -8
		self.font_style = 0
		self.font_size = 10
		self.bg_color = "#404040"
		self.fg_color = "#000000"
		self.scale = 1
		self.isa = 0
		self.need_update_args = true
	end

	self.width = math.max(8, math.floor(self.width))
	local step_height = math.max(2, math.floor(self.height / VU_STEPS))
	self.height = VU_STEPS * step_height
	self.rms = -101
	self.peak = -101

	if self.receive ~= "empty" then
		self.receiver = pd.Receive:new():register(self, self.receive, "_receiver")
	end

	self:set_size(self.width, self.height)
	return true
end

function vu:update_args()
	self:set_args({
		self.width,
		self.height,
		self.receive,
		self.label,
		self.x_off,
		self.y_off,
		self.font_style,
		self.font_size,
		self.bg_color,
		self.fg_color,
		self.scale,
		self.isa,
	})
end

function vu:hex_to_rgb(hex)
	hex = tostring(hex):gsub("#", "")
	return {
		tonumber(hex:sub(1, 2), 16) or 0,
		tonumber(hex:sub(3, 4), 16) or 0,
		tonumber(hex:sub(5, 6), 16) or 0,
	}
end

function vu:db_to_step(db)
	if type(db) ~= "number" or db ~= db or db <= VU_MIN_DB then
		return 0
	end
	if db >= VU_MAX_DB then
		return VU_STEPS
	end

	for step = VU_STEPS, 2, -1 do
		if db >= VU_THRESHOLDS[step] then
			return step
		end
	end
	return 1
end

function vu:set_rms(value, output)
	if type(value) ~= "number" or value ~= value then
		return
	end
	self.rms = math.floor(value * 100 + 0.5) / 100
	if output then
		self:outlet(1, "float", { self.rms })
	end
	self:repaint(2)
end

function vu:set_peak(value, output)
	if type(value) ~= "number" or value ~= value then
		return
	end
	self.peak = math.floor(value * 100 + 0.5) / 100
	if output then
		self:outlet(2, "float", { self.peak })
	end
	self:repaint(2)
end

function vu:_receiver(sel, atoms)
	if sel == "float" then
		self:set_rms(atoms[1], true)
	end
end

function vu:in_1_bang()
	-- Pd sends the peak first, then the RMS value.
	self:outlet(2, "float", { self.peak })
	self:outlet(1, "float", { self.rms })
	self:repaint(2)
end

function vu:in_1_float(value)
	self:set_rms(value, true)
end

function vu:in_1_list(atoms)
	self:set_rms(atoms[1], true)
end

function vu:in_2_float(value)
	self:set_peak(value, true)
end

function vu:in_2_list(atoms)
	self:set_peak(atoms[1], true)
end

function vu:in_1_size(atoms)
	self.width = math.max(8, math.floor(atoms[1] or self.width))
	if atoms[2] ~= nil then
		local step_height = math.max(2, math.floor(atoms[2] / VU_STEPS))
		self.height = VU_STEPS * step_height
	end
	self:set_size(self.width, self.height)
	self:update_args()
	self:repaint()
end

function vu:in_1_scale(atoms)
	self.scale = (atoms[1] or 0) == 0 and 0 or 1
	self:update_args()
	self:repaint()
end

function vu:paint(g)
	if self.need_update_args then
		self.need_update_args = false
		self:update_args()
	end

	g:set_color(table.unpack(self:hex_to_rgb(self.bg_color)))
	g:fill_all()
	g:set_color(table.unpack(self:hex_to_rgb(self.fg_color)))
	g:stroke_rect(0, 0, self.width, self.height, 1)
end

function vu:paint_layer_2(g)
	local step_height = (self.height - VU_BOTTOM_PADDING) / VU_STEPS
	local led_width = math.max(1, step_height - 1)
	local quarter = math.floor(self.width / 4)
	local x1 = quarter + 1
	local x2 = self.width - quarter
	local rms_step = self:db_to_step(self.rms)

	-- Pd draws these as horizontal LED lines, from the bottom upwards.
	g:set_color(80, 235, 80)
	for step = 1, rms_step do
		local y = -step_height / 2 + step_height * (VU_STEPS + 1 - step)
		g:draw_line(x1, y, x2, y, led_width)
	end

	local peak_step = self:db_to_step(self.peak)
	if peak_step > 0 then
		local y = step_height * (VU_STEPS + 1 - peak_step) - step_height / 2
		g:draw_line(0, y, self.width, y, led_width + 1)
	end
end

function vu:in_1_reload()
	self:dofilex(self._scriptname)
	self:initialize()
end
