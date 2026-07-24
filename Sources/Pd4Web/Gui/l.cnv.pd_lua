local cnv = pd.Class:new():register("l.cnv")

local IEM_COLORS = {
	0xfcfcfc, 0xa0a0a0, 0x404040, 0xfce0e0, 0xfce0c0,
	0xfcfcc8, 0xd8fcd8, 0xd8fcfc, 0xdce4fc, 0xf8d8fc,
	0xe0e0e0, 0x7c7c7c, 0x202020, 0xfc2828, 0xfcac44,
	0xe8e828, 0x14e814, 0x28f4f4, 0x3c50fc, 0xf430f0,
	0xbcbcbc, 0x606060, 0x000000, 0x8c0808, 0x583000,
	0x782814, 0x285014, 0x004450, 0x001488, 0x580050,
}

function cnv:initialize(_, args)
	self.inlets = 0
	self.outlets = 0
	self.need_update_args = false

	if args ~= nil and #args > 0 then
		self.size = args[1]
		self.width = args[2]
		self.height = args[3]
		self.send = args[4]
		self.receive = args[5]
		self.label = args[6]
		self.x_off = args[7]
		self.y_off = args[8]
		self.font_style = args[9]
		self.font_size = args[10]
		self.bg_color = args[11]
		self.label_color = args[12]
		self.isa = args[13] or 0
	else
		self.size = 15
		self.width = 100
		self.height = 60
		self.send = "empty"
		self.receive = "empty"
		self.label = "empty"
		self.x_off = 20
		self.y_off = 12
		self.font_style = 0
		self.font_size = 10
		self.bg_color = "#e0e0e0"
		self.label_color = "#404040"
		self.isa = 0
		self.need_update_args = true
	end

	self:clip_sizes()
	if self.receive ~= "empty" then
		self.receiver = pd.Receive:new():register(self, self.receive, "_receiver")
	end
	self:apply_size()
	return true
end

function cnv:clip_sizes()
	self.size = math.max(1, math.floor(tonumber(self.size) or 1))
	self.width = math.max(1, math.floor(tonumber(self.width) or 1))
	self.height = math.max(1, math.floor(tonumber(self.height) or 1))
	self.font_size = math.max(4, math.floor(tonumber(self.font_size) or 10))
end

function cnv:apply_size()
	-- pd.Class:set_size only forwards the visual width and height.  cnv also
	-- needs its smaller Pd selection bounds, so call Pd4Web's extended bridge.
	_gfx_internal.set_size(self._object, self.width, self.height, self.size, self.size)
end

function cnv:update_args()
	self:set_args({
		self.size,
		self.width,
		self.height,
		self.send,
		self.receive,
		self.label,
		self.x_off,
		self.y_off,
		self.font_style,
		self.font_size,
		self.bg_color,
		self.label_color,
		self.isa,
	})
end

function cnv:color_to_rgb(color)
	local value
	if type(color) == "number" then
		if color >= 0 then
			value = IEM_COLORS[(math.floor(color) % #IEM_COLORS) + 1]
		else
			value = (-1 - math.floor(color)) % 0x1000000
		end
	else
		local hex = tostring(color):gsub("#", "")
		value = tonumber(hex, 16) or 0
	end

	return {
		math.floor(value / 0x10000) % 0x100,
		math.floor(value / 0x100) % 0x100,
		value % 0x100,
	}
end

function cnv:replace_receiver(name)
	if self.receiver ~= nil then
		self.receiver:destruct()
		self.receiver = nil
	end
	self.receive = name or "empty"
	if self.receive ~= "empty" then
		self.receiver = pd.Receive:new():register(self, self.receive, "_receiver")
	end
end

function cnv:_receiver(sel, atoms)
	local handler = self["in_1_" .. sel]
	if handler ~= nil then
		handler(self, atoms)
	end
end

function cnv:in_1_size(atoms)
	self.size = atoms[1] or self.size
	self:clip_sizes()
	self:apply_size()
	self:update_args()
	self:repaint()
end

function cnv:in_1_vis_size(atoms)
	self.width = atoms[1] or self.width
	self.height = atoms[2] or self.width
	self:clip_sizes()
	self:apply_size()
	self:update_args()
	self:repaint()
end

function cnv:in_1_color(atoms)
	if atoms[1] ~= nil then
		self.bg_color = atoms[1]
	end
	if atoms[2] ~= nil then
		self.label_color = atoms[2]
	end
	self:update_args()
	self:repaint()
end

function cnv:in_1_send(atoms)
	self.send = atoms[1] or "empty"
	self:update_args()
end

function cnv:in_1_receive(atoms)
	self:replace_receiver(atoms[1])
	self:update_args()
end

function cnv:in_1_label(atoms)
	self.label = atoms[1] or "empty"
	self:update_args()
	self:repaint()
end

function cnv:in_1_label_pos(atoms)
	self.x_off = atoms[1] or self.x_off
	self.y_off = atoms[2] or self.y_off
	self:update_args()
	self:repaint()
end

function cnv:in_1_label_font(atoms)
	self.font_style = atoms[1] or self.font_style
	self.font_size = atoms[2] or self.font_size
	self:clip_sizes()
	self:update_args()
	self:repaint()
end

function cnv:paint(g)
	if self.need_update_args then
		self.need_update_args = false
		self:update_args()
	end

	g:set_color(table.unpack(self:color_to_rgb(self.bg_color)))
	g:fill_all()

	-- Pd's selectable base is a square at the upper-left of the visible area.
	-- Its outline normally has the same color as the canvas background.
	g:stroke_rect(0, 0, math.min(self.size, self.width), math.min(self.size, self.height), 1)

	if self.label ~= nil and self.label ~= "empty" then
		g:set_color(table.unpack(self:color_to_rgb(self.label_color)))
		local text_y = self.y_off - math.floor(self.font_size / 2)
		g:draw_text(
			tostring(self.label),
			self.x_off,
			text_y,
			math.max(1, self.width - self.x_off),
			self.font_size
		)
	end
end

function cnv:in_1_reload()
	self:dofilex(self._scriptname)
	self:initialize()
end
