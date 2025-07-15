local nbx = pd.Class:new():register("l.nbx")

-- ──────────────────────────────────────────
function nbx:initialize(_, args)
	self.inlets = 1
	self.outlets = 1
	self.select = false
	self:set_size(5 * 10 + 4, 16)
	self.number = "0"
	return true
end

-- ──────────────────────────────────────────
function nbx:mouse_down(x, y)
	self.select = not self.select
	self.needclear = true
	self:repaint()
end

-- ──────────────────────────────────────────
function nbx:key_down(_, _, key)
	if not self.select then
		return
	end

	if self.needclear then
		self.number = ""
		self.needclear = false
	end

	if key == "Enter" then
		self.needclear = false
		self.select = false
		self:outlet(1, "float", { tonumber(self.number) })
		self:repaint()
		return
	elseif key == "Backspace" or key == "Delete" then
		self.number = self.number:sub(1, -2)
	elseif key == "." then
		self.number = self.number .. key
	elseif tonumber(key) then
		self.number = self.number .. key
	else
		return
	end

	self:repaint()
end

-- ──────────────────────────────────────────
function nbx:in_1_float(args)
	self.number = args
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
	g:draw_text(self.number, 10, height / 4, 50, 10)
end

-- ──────────────────────────────────────────
function nbx:in_1_reload()
	self:dofilex(self._scriptname)
	self:initialize()
end
