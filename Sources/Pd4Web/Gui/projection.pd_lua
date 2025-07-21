local projection = pd.Class:new():register("projection")

-- ─────────────────────────────────────
function projection:initialize(name, args)
	self.inlets = 1
	self.outlets = 0

	self.angle = 0

	self.points = {
		{ -0.5, -0.5, -0.5 },
		{ 0.5, -0.5, -0.5 },
		{ 0.5, 0.5, -0.5 },
		{ -0.5, 0.5, -0.5 },
		{ -0.5, -0.5, 0.5 },
		{ 0.5, -0.5, 0.5 },
		{ 0.5, 0.5, 0.5 },
		{ -0.5, 0.5, 0.5 },
	}

	self:set_size(100, 100) 
	return true
end

-- ─────────────────────────────────────
function projection:in_1_bang()
	self.angle = self.angle + 0.01
	self:repaint()
end

-- ─────────────────────────────────────
function projection:paint(g)
	g:set_color(30, 30, 30) -- dark background
	g:fill_all()

	local width, height = self:get_size()

	local angle = self.angle
	local distance = 1.5
	local scale = width/2
	local offsetX = width / 2
	local offsetY = height / 2

	local projected = {}

	for i, p in ipairs(self.points) do
		local rotated = rotate_y(p, angle * 0.7)
		rotated = rotate_x(rotated, angle * 1.2)
		rotated = rotate_z(rotated, angle * 0.5)

		local z = 1 / (distance - rotated[3])
		local projection = {
			rotated[1] * z * scale + offsetX,
			rotated[2] * z * scale + offsetY,
		}
		projected[i] = projection
	end

	local faces = {
		{ 1, 2, 3, 4 }, -- back
		{ 5, 6, 7, 8 }, -- front
		{ 1, 2, 6, 5 }, -- bottom
		{ 2, 3, 7, 6 }, -- right
		{ 3, 4, 8, 7 }, -- top
		{ 4, 1, 5, 8 }, -- left
	}

	local cube_color = { 60, 180, 240 } -- gentle blue

	for _, face in ipairs(faces) do
		local a, b, c, d = projected[face[1]], projected[face[2]], projected[face[3]], projected[face[4]]
		local p = Path(a[1], a[2])
		p:line_to(b[1], b[2])
		p:line_to(c[1], c[2])
		p:line_to(d[1], d[2])
		p:close()
		g:set_color(cube_color[1], cube_color[2], cube_color[3], 200)
		g:fill_path(p)
        -- is better to remove this
		-- g:set_color(255, 255, 255, 255)
		-- g:stroke_path(p, 2)
	end

	-- Draw edges
	local edges = {
		{ 1, 2 },
		{ 2, 3 },
		{ 3, 4 },
		{ 4, 1 },
		{ 5, 6 },
		{ 6, 7 },
		{ 7, 8 },
		{ 8, 5 },
		{ 1, 5 },
		{ 2, 6 },
		{ 3, 7 },
		{ 4, 8 },
	}
	g:set_color(255, 255, 255, 255)
	for _, edge in ipairs(edges) do
		local a, b = projected[edge[1]], projected[edge[2]]
		g:draw_line(a[1], a[2], b[1], b[2], 2)
	end
end

-- ─────────────────────────────────────
function rotate_x(v, angle)
	local x, y, z = table.unpack(v)
	local c = math.cos(angle)
	local s = math.sin(angle)
	return { x, y * c - z * s, y * s + z * c }
end

function rotate_y(v, angle)
	local x, y, z = table.unpack(v)
	local c = math.cos(angle)
	local s = math.sin(angle)
	return { x * c + z * s, y, -x * s + z * c }
end

function rotate_z(v, angle)
	local x, y, z = table.unpack(v)
	local c = math.cos(angle)
	local s = math.sin(angle)
	return { x * c - y * s, x * s + y * c, z }
end
