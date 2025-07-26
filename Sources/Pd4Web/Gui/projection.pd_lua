local projection = pd.Class:new():register("projection")

--╭─────────────────────────────────────╮
--│            Buttom Class             │
--╰─────────────────────────────────────╯
local Button = {}
Button.__index = Button

-- ─────────────────────────────────────
function Button:new(father, x, y, w, h, font_size, label)
	local obj = {
		father = father,
		x = x,
		y = y,
		w = w,
		h = h,
		label = label,
		hovered = false,
		clicked = false,
		font_size = font_size,
	}
	setmetatable(obj, self)
	return obj
end

-- ─────────────────────────────────────
function Button:contains(px, py)
	return px >= self.x and px <= self.x + self.w and py >= self.y and py <= self.y + self.h
end

-- ─────────────────────────────────────
function Button:update_hover(mx, my)
	self.hovered = self:contains(mx, my)
end

-- ─────────────────────────────────────
function Button:on_mouse_click(callback)
	self.mouse_click_callback = callback
end

-- ─────────────────────────────────────
function Button:is_inside(mx, my)
	return mx >= self.x and mx <= (self.x + self.w) and my >= self.y and my <= (self.y + self.h)
end

-- ─────────────────────────────────────
function Button:mouse_move(mx, my)
	local hovered = self:is_inside(mx, my)
	if self.hovered ~= hovered then
		self.hovered = hovered
		self.father:repaint(2)
	end
end

-- ─────────────────────────────────────
function Button:mouse_down(mx, my)
	if self:is_inside(mx, my) then
		if self.mouse_click_callback then
			self.mouse_click_callback(self.father)
		end
		self.clicked = true
		return true
	end
	return false
end

-- ─────────────────────────────────────
function Button:mouse_up(mx, my)
	self.clicked = false
end

-- ─────────────────────────────────────
function Button:is_hover()
	return self.hovered
end

-- ─────────────────────────────────────
function Button:is_clicked()
	return self.clicked
end

-- ─────────────────────────────────────
function Button:draw(g)
	-- Background
	if self.clicked then
		g:set_color(180, 180, 180)
	elseif self.hovered then
		g:set_color(230, 230, 230)
	else
		g:set_color(200, 200, 200)
	end
	g:fill_rounded_rect(self.x, self.y, self.w, self.h, 2)

	-- Border
	g:set_color(0, 0, 0)
	g:stroke_rounded_rect(self.x, self.y, self.w, self.h, 2, 1)

	-- Text
	local char_width = self.font_size / 2
	local text_width = #self.label * char_width
	local text_height = self.font_size
	local text_x = self.x + (self.w - text_width) / 2
	local text_y = self.y + (self.h - text_height) / 2

	g:draw_text(self.label, text_x, text_y, text_width * 1.5, self.font_size)
end

--╭─────────────────────────────────────╮
--│        Object Initialization        │
--╰─────────────────────────────────────╯
function projection:initialize(name, args)
	self.inlets = 1
	self.outlets = 2

	-- initialization
	self.rotation_x = 0
	self.rotation_y = 0
	self.last_mouse_x = 0
	self.last_mouse_y = 0
	self.linex = {}
	self.liney = {}
	self.linez = {}
	self.now = 0
	self.curve_progress = 0
	self.animate = false
	self.clock_animation = pd.Clock:new():register(self, "point_animation")
	self.room_xyz = { x = 3, y = 3, z = 3 }
	self.trajectories = {}
	self.scale_global = 0.55

	-- inside the object, position goes from -0.5 to 0.5
	self.speakers = {
		{ -0.5, -0.4, -0.5 },
		{ 0.5, -0.4, -0.5 },
		{ 0.5, -0.4, 0.5 },
		{ -0.5, -0.4, 0.5 },

		{ -0.5, 0.4, -0.5 },
		{ 0.5, 0.4, -0.5 },
		{ 0.5, 0.4, 0.5 },
		{ -0.5, 0.4, 0.5 },
	}

	self.scale_xyz = {
		x = 2,
		y = 2,
		z = 2,
	}

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

	self:set_size(350, 350)
	local width, height = self:get_size()

	-- play
	self.play_button = Button:new(self, width - 42, 3, 40, 20, 12, "play")
	self.play_button:on_mouse_click(self.play_click)

	-- reset
	self.reset_button = Button:new(self, width - 85, 3, 40, 20, 12, "reset")
	self.reset_button:on_mouse_click(self.reset_click)

	-- zoom
	self.zoom_plus_button = Button:new(self, width - 118, 3, 30, 20, 12, "+")
	self.zoom_plus_button:on_mouse_click(self.zoom_plus_click)

	self.zoom_minus_button = Button:new(self, width - 150, 3, 30, 20, 12, "-")
	self.zoom_minus_button:on_mouse_click(self.zoom_minus_click)

	-- export
	-- self.export_button = Button:new(self, width - 135, 3, 47, 20, 12, "export")
	-- self.export_button:on_mouse_click(self.export_click)

	return true
end

--╭─────────────────────────────────────╮
--│           Buttons methods           │
--╰─────────────────────────────────────╯
function projection:play_click()
	self.animate = true
	self.now = 0
	local now = 0
	for _, trajectory in pairs(self.trajectories) do
		trajectory.start_time = now
	end
	self.clock_animation:delay(0)
end

-- ─────────────────────────────────────
function projection:reset_click()
	self.rotation_y = 0
	self.rotation_x = 0
	self:repaint()
end

-- ─────────────────────────────────────
function projection:zoom_plus_click()
	self.scale_global = self.scale_global + 0.1
	self:repaint()
end

-- ─────────────────────────────────────
function projection:zoom_minus_click()
	self.scale_global = self.scale_global - 0.1
	self:repaint()
end

-- ─────────────────────────────────────
function projection:export_click()
	local file, err = io.open("trajectories.txt", "w")
	if not file then
		self:error("Failed to open file: " .. err)
		return
	end

	for index, t in pairs(self.trajectories or {}) do
		local x = t.x or {}
		local y = t.y or {}
		local z = t.z or {}

		file:write("trajectory ", index, "\n")

		local len = math.max(#x, #y, #z)
		for i = 1, len do
			local xi = x[i] or 0
			local yi = y[i] or 0
			local zi = z[i] or 0
			file:write(string.format("%.6f,%.6f,%.6f\n", xi, yi, zi))
		end
		file:write("\n")
	end

	file:close()
	pd.post("done")
end

--╭─────────────────────────────────────╮
--│               Methods               │
--╰─────────────────────────────────────╯
function projection:in_1_size(args)
	local max_arg = math.max(args[1], args[2], args[3])
	if max_arg == 0 then
		max_arg = 1
	end
	self.room_xyz = { x = args[1], y = args[2], z = args[3] }

	local relative_scale = 2 / max_arg
	self.scale_xyz = {
		x = args[1] * relative_scale,
		y = args[2] * relative_scale,
		z = args[3] * relative_scale,
	}

	-- resetar rotação
	self.rotation_x = 0
	self.rotation_y = 0
	self:repaint()
end

-- ─────────────────────────────────────
function projection:in_1_reset(args)
	self.rotation_y = 0
	self.rotation_x = 0
	self:repaint()
end

-- ─────────────────────────────────────
function projection:in_1_point(args)
	if #args < 3 then
		return
	end

	self.point = {
		tonumber(args[1]) or 0,
		tonumber(args[2]) or 0,
		tonumber(args[3]) or 0,
	}

	self:repaint(4)
end

-- ─────────────────────────────────────
function projection:in_1_time(args)
	local index = math.floor(args[1])
	if self.trajectories[index] == nil then
		self.trajectories[index] = {}
	end

	local times = {}
	for i = 2, #args do
		times[i - 1] = tonumber(args[i])
	end

	self.trajectories[index]["time"] = times

	-- Precompute normalized durations (Δt) and store for interpolation
	if #times >= 2 then
		local segments = {}
		for i = 1, #times - 1 do
			local dt = times[i + 1] - times[i]
			if dt <= 0 then
				self:error("Non-increasing time values at index " .. i)
				return
			end
			table.insert(segments, dt)
		end
		self.trajectories[index]["duration_segments"] = segments
		self.trajectories[index]["start_time"] = nil -- will be set at :start()
	end
end

-- ─────────────────────────────────────
function projection:in_1_linex(args)
	local index = math.floor(args[1])
	local maxX = (self.room_xyz.x / 2)

	local scaled_args = {}
	for i = 2, #args do
		local v = args[i]
		if v >= maxX + 0.001 or v <= -maxX - 0.001 then
			self:error("Point out of range x: " .. v .. ". Should be between -" .. maxX .. " and " .. maxX)
			return
		end
		scaled_args[i - 1] = v / (2 * maxX)
	end

	if self.trajectories[index] == nil then
		self.trajectories[index] = {}
	end

	self.trajectories[index]["x"] = scaled_args
	local trajectoryX = self.trajectories[index]["x"]
	local trajectoryY = self.trajectories[index]["y"]
	local trajectoryZ = self.trajectories[index]["z"]

	if trajectoryX ~= nil and trajectoryY ~= nil and trajectoryZ ~= nil then
		local lenX = #trajectoryX
		local lenY = #trajectoryY
		local lenZ = #trajectoryZ
		if lenX > 0 and (lenX == lenY or lenX == lenZ) then
			self:repaint(3)
		end
	end
end

-- ─────────────────────────────────────
function projection:in_1_liney(args)
	local index = math.floor(args[1])
	local maxY = (self.room_xyz.y / 2) + 0.01

	local scaled_args = {}
	for i = 2, #args do
		local v = args[i]
		if v >= maxY or v <= -maxY then
			self:error("Point out of range y: " .. v .. ". Should be between -" .. maxY .. " and " .. maxY)
			return
		end
		scaled_args[i - 1] = v / (2 * maxY)
	end

	if self.trajectories[index] == nil then
		self.trajectories[index] = {}
	end

	self.trajectories[index]["y"] = scaled_args

	local trajectoryX = self.trajectories[index]["x"]
	local trajectoryY = self.trajectories[index]["y"]
	local trajectoryZ = self.trajectories[index]["z"]
	if trajectoryX ~= nil and trajectoryY ~= nil and trajectoryZ ~= nil then
		local lenX = #trajectoryX
		local lenY = #trajectoryY
		local lenZ = #trajectoryZ
		if lenX > 0 and (lenX == lenY or lenX == lenZ) then
			self:repaint(3)
		end
	end
end

-- ─────────────────────────────────────
function projection:in_1_linez(args)
	local index = math.floor(args[1])
	local maxZ = (self.room_xyz.z / 2) + 0.01

	local scaled_args = {}
	for i = 2, #args do
		local v = args[i]
		if v >= maxZ or v <= -maxZ then
			self:error("Point out of range z: " .. v .. ". Should be between -" .. maxZ .. " and " .. maxZ)
			return
		end
		scaled_args[i - 1] = v / (2 * maxZ)
	end
	if self.trajectories[index] == nil then
		self.trajectories[index] = {}
	end
	self.trajectories[index]["z"] = scaled_args

	local trajectoryX = self.trajectories[index]["x"]
	local trajectoryY = self.trajectories[index]["y"]
	local trajectoryZ = self.trajectories[index]["z"]

	if trajectoryX ~= nil and trajectoryY ~= nil and trajectoryZ ~= nil then
		local lenX = #trajectoryX
		local lenY = #trajectoryY
		local lenZ = #trajectoryZ
		if lenX > 0 and (lenX == lenY or lenX == lenZ) then
			self:repaint(3)
		end
	end
end

-- ─────────────────────────────────────
function projection:in_1_start()
	self.animate = true
	self.now = 0
	local now = 0
	for _, trajectory in pairs(self.trajectories) do
		trajectory.start_time = now
	end
	self.clock_animation:delay(0)
end

-- ─────────────────────────────────────
function projection:in_1_stop()
	self.animate = false
end

--╭─────────────────────────────────────╮
--│               Clocks                │
--╰─────────────────────────────────────╯
function projection:point_animation()
	if not self.animate then
		return
	end

	local now = self.now

	for index, trajectory in pairs(self.trajectories) do
		local tx, ty, tz = trajectory.x, trajectory.y, trajectory.z
		local times = trajectory.time
		local durations = trajectory.duration_segments
		local start_time = trajectory.start_time

		if tx and ty and tz and times and durations and start_time then
			local elapsed = now - start_time

			-- Find the correct segment
			local acc_time = 0
			local segment_index = nil
			for i = 1, #durations do
				acc_time = acc_time + durations[i]
				if elapsed <= acc_time then
					segment_index = i
					break
				end
			end

			if segment_index then
				local seg_start_time = acc_time - durations[segment_index]
				local seg_progress = (elapsed - seg_start_time) / durations[segment_index]
				local i = segment_index
				-- Linear interpolation
				local x = tx[i] * (1 - seg_progress) + tx[i + 1] * seg_progress
				local y = ty[i] * (1 - seg_progress) + ty[i + 1] * seg_progress
				local z = tz[i] * (1 - seg_progress) + tz[i + 1] * seg_progress
				trajectory.point = { x, y, z }
				trajectory.redraw = true
				self:outlet(2, "point", {
					index,
					(self.room_xyz.x / 2) + x * self.room_xyz.x,
					(self.room_xyz.y / 2) + y * self.room_xyz.y,
					(self.room_xyz.z / 2) + z * self.room_xyz.z,
				})
			else
				trajectory.redraw = false
				-- self:outlet(2, "point", { index, x * self.room_xyz.x, y * self.room_xyz.y, z * self.room_xyz.z })
			end
		end
	end

	self:repaint(4)
	self.clock_animation:delay(30)
	self.now = now + 15
end

--╭─────────────────────────────────────╮
--│            Mouse Methods            │
--╰─────────────────────────────────────╯
function projection:mouse_down(x, y)
	self.last_mouse_x = x
	self.last_mouse_y = y

	--
	self.play_button:mouse_down(x, y)
	self.reset_button:mouse_down(x, y)
	-- self.export_button:mouse_down(x, y)
	self.zoom_plus_button:mouse_down(x, y)
	self.zoom_minus_button:mouse_down(x, y)
end

-- ─────────────────────────────────────
function projection:mouse_up(x, y)
	self.play_button:mouse_up(x, y)
	self.reset_button:mouse_up(x, y)
	-- self.export_button:mouse_up(x, y)
	self.zoom_plus_button:mouse_up(x, y)
	self.zoom_minus_button:mouse_up(x, y)
end

-- ─────────────────────────────────────
function projection:mouse_move(x, y)
	self.play_button:mouse_move(x, y)
	self.reset_button:mouse_move(x, y)
	-- self.export_button:mouse_move(x, y)
	self.zoom_plus_button:mouse_move(x, y)
	self.zoom_minus_button:mouse_move(x, y)
end

-- ─────────────────────────────────────
function projection:mouse_drag(x, y)
	local dx = x - self.last_mouse_x
	local dy = y - self.last_mouse_y

	self.rotation_y = self.rotation_y + dx * 0.01
	self.rotation_x = self.rotation_x - dy * 0.01

	self.last_mouse_x = x
	self.last_mouse_y = y
	self:repaint()
end

-- ─────────────────────────────────────
function projection:index_to_color(index)
	local distinct_colors = {
		{ 230, 25, 75 }, -- Red
		{ 60, 180, 75 }, -- Green
		{ 255, 225, 25 }, -- Yellow
		{ 0, 130, 200 }, -- Blue
		{ 245, 130, 48 }, -- Orange
		{ 145, 30, 180 }, -- Purple
		{ 70, 240, 240 }, -- Cyan
		{ 240, 50, 230 }, -- Magenta
		{ 210, 245, 60 }, -- Lime
		{ 250, 190, 190 }, -- Pink
		{ 0, 128, 128 }, -- Teal
		{ 128, 128, 0 }, -- Olive
	}

	local color = distinct_colors[(index % #distinct_colors) + 1]
	return { color[1], color[2], color[3] }
end

--╭─────────────────────────────────────╮
--│         Paint: Make Buttom          │
--╰─────────────────────────────────────╯
function projection:draw_button(g, x, y, w, h, text, hover)
	local font_size = 12
	local char_width = font_size / 2 -- largura média estimada por caractere
	local text_width = #text * char_width
	local text_height = font_size

	local text_x = x + (w - text_width) / 2
	local text_y = y + (h - text_height) / 2

	if hover then
		g:set_color(200, 200, 200)
	else
		g:set_color(100, 100, 100)
	end

	g:fill_rounded_rect(x, y, w, h, 4)

	g:set_color(0, 0, 0)
	g:draw_text(text, text_x, text_y, text_width, font_size)

	g:set_color(0, 0, 0)
	g:stroke_rounded_rect(x, y, w, h, 4, 1)
end

--╭─────────────────────────────────────╮
--│     Paint: create box and coord     │
--╰─────────────────────────────────────╯
function projection:paint(g)
	g:set_color(240, 240, 240)
	g:fill_all()

	local width, height = self:get_size()
	local max_scale = math.max(self.scale_xyz.x, self.scale_xyz.y, self.scale_xyz.z)
	local scale = (math.min(width, height) * self.scale_global) / max_scale

	local offsetX = width / 2
	local offsetY = height / 2
	local distance = 2

	local rotated_3d = {}
	local projected = {}

	for i, p in ipairs(self.points) do
		-- Aplica escala
		local scaled = {
			p[1] * self.scale_xyz.x,
			p[2] * self.scale_xyz.y,
			p[3] * self.scale_xyz.z,
		}
		-- Rotaciona no eixo Y e X
		local rotated = self:rotate_y(scaled, self.rotation_y)
		rotated = self:rotate_x(rotated, self.rotation_x)
		rotated_3d[i] = rotated

		-- Projeção perspectiva
		local z_diff = distance - rotated[3]
		if z_diff <= 0.01 then
			z_diff = 0.01 -- evitar divisão por zero / clipping
		end
		local z = 1 / z_diff

		projected[i] = {
			rotated[1] * z * scale + offsetX,
			rotated[2] * z * scale + offsetY,
		}
	end

	-- Faces do cubo (indices dos vértices)
	local faces = {
		{ 1, 2, 3, 4 },
		{ 5, 6, 7, 8 },
		{ 1, 2, 6, 5 },
		{ 2, 3, 7, 6 },
		{ 3, 4, 8, 7 },
		{ 4, 1, 5, 8 },
	}

	-- Calcula profundidade média de cada face para ordenar desenho
	local face_depths = {}
	for _, face in ipairs(faces) do
		local depth = 0
		for j = 1, 4 do
			depth = depth + rotated_3d[face[j]][3]
		end
		depth = depth / 4
		table.insert(face_depths, { depth = depth, face = face })
	end

	-- Ordena as faces da mais distante para a mais próxima
	table.sort(face_depths, function(a, b)
		return a.depth > b.depth
	end)

	-- Desenha faces preenchidas
	for _, entry in ipairs(face_depths) do
		local face = entry.face
		local a, b, c, d = projected[face[1]], projected[face[2]], projected[face[3]], projected[face[4]]
		local p = Path(a[1], a[2])
		p:line_to(b[1], b[2])
		p:line_to(c[1], c[2])
		p:line_to(d[1], d[2])
		p:close()
		g:set_color(197, 220, 229)
		g:fill_path(p)
	end

	-- Desenha eixos X, Y, Z
	local corner_offsetX = 25
	local corner_offsetY = 25
	local fixed_scale = 100 -- escala razoável para visualizar eixo fixo
	local fixed_distance = 1.5 -- distância para projeção

	self:draw_3d_axis(
		g,
		{ 0.3, 0, 0 },
		"x",
		{ 255, 100, 100 },
		fixed_scale,
		corner_offsetX,
		corner_offsetY,
		fixed_distance
	)
	self:draw_3d_axis(
		g,
		{ 0, 0.3, 0 },
		"y",
		{ 100, 255, 100 },
		fixed_scale,
		corner_offsetX,
		corner_offsetY,
		fixed_distance
	)
	self:draw_3d_axis(
		g,
		{ 0, 0, 0.3 },
		"z",
		{ 100, 100, 255 },
		fixed_scale,
		corner_offsetX,
		corner_offsetY,
		fixed_distance
	)

	-- Desenha as arestas do cubo
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
		g:draw_line(a[1], a[2], b[1], b[2], 1)
	end

	-- DRAW SPEAKERS
	g:set_color(255, 165, 0) -- orange color for speakers

	for _, speaker in ipairs(self.speakers) do
		-- posição original
		local sx, sy, sz = speaker[1], speaker[2], speaker[3]

		-- direção para o centro (normalizado)
		local dx, dy, dz = -sx, -sy, -sz
		local length = math.sqrt(dx * dx + dy * dy + dz * dz)
		if length == 0 then
			length = 1
		end
		dx, dy, dz = dx / length, dy / length, dz / length

		-- posição + pequena extensão na direção do centro (para desenhar seta/lado frontal)
		local front = {
			sx + dx * 0.1, -- ajustável: tamanho do vetor "frente"
			sy + dy * 0.1,
			sz + dz * 0.1,
		}

		-- escala e rotação
		local function project(p)
			local scaled = {
				p[1] * self.scale_xyz.x,
				p[2] * self.scale_xyz.y,
				p[3] * self.scale_xyz.z,
			}
			local rotated = self:rotate_y(scaled, self.rotation_y)
			return self:rotate_x(rotated, self.rotation_x)
		end

		local p1 = project({ sx, sy, sz })
		local p2 = project(front)

		local z1 = 1 / math.max(0.1, distance - p1[3])
		local z2 = 1 / math.max(0.1, distance - p2[3])

		local x1 = p1[1] * z1 * scale + offsetX
		local y1 = p1[2] * z1 * scale + offsetY
		local x2 = p2[1] * z2 * scale + offsetX
		local y2 = p2[2] * z2 * scale + offsetY

		-- Tamanho do alto-falante baseado na profundidade
		local z_diff = distance - p1[3]
		local size = 20 / z_diff
		size = math.max(5, math.min(size, 15))

		local rect_size = size * 1.3
		g:set_color(50, 50, 50, 180)
		g:fill_rounded_rect(x1 - rect_size / 2, y1 - rect_size / 2, rect_size, rect_size, 2)

		-- Cone laranja (elipse)
		g:set_color(255, 165, 0)
		g:fill_ellipse(x1 - size / 2, y1 - size / 2, size, size)

		-- Contorno preto
		g:set_color(0, 0, 0)
		g:stroke_ellipse(x1 - size / 2, y1 - size / 2, size, size, 1)

		-- Círculo interno
		g:set_color(50, 50, 50)
		g:fill_ellipse(x1 - size / 6, y1 - size / 6, size / 3, size / 3)

		-- Linha frontal indicando orientação
		g:set_color(255, 100, 0)
		g:draw_line(x1, y1, x2, y2, 1)

		g:set_color(255, 165, 0) -- reset
	end
end

--╭─────────────────────────────────────╮
--│          PAINT: Buttons             │
--╰─────────────────────────────────────╯
function projection:paint_layer_2(g)
	self.play_button:draw(g)
	self.reset_button:draw(g)
	-- self.export_button:draw(g)
	self.zoom_plus_button:draw(g)
	self.zoom_minus_button:draw(g)
end

--╭─────────────────────────────────────╮
--│          PAINT: Trajectory          │
--╰─────────────────────────────────────╯
function projection:paint_layer_3(g)
	local width, height = self:get_size()
	local max_scale = math.max(self.scale_xyz.x, self.scale_xyz.y, self.scale_xyz.z)
	local scale = (math.min(width, height) * self.scale_global) / max_scale

	local offsetX = width / 2
	local offsetY = height / 2
	local distance = 2

	for index, trajectory in pairs(self.trajectories) do
		local linex = trajectory.x or {}
		local liney = trajectory.y or {}
		local linez = trajectory.z or {}

		if #linex > 1 and #linex == #liney and #linex == #linez then
			local points_2d = {}
			local depths = {}
			local points_3d = {} -- Store 3D points for later curve evaluation

			-- Projeta todos os pontos 3D para 2D
			for i = 1, #linex do
				local scaled = {
					linex[i] * self.scale_xyz.x,
					liney[i] * self.scale_xyz.y,
					linez[i] * self.scale_xyz.z,
				}
				local p = self:rotate_y(scaled, self.rotation_y)
				p = self:rotate_x(p, self.rotation_x)
				points_3d[i] = p -- Store the 3D rotated point

				local depth = math.max(0.1, distance - p[3])
				local z = 1 / depth
				local x2d = p[1] * z * scale + offsetX
				local y2d = p[2] * z * scale + offsetY

				points_2d[#points_2d + 1] = { x2d, y2d }
				depths[#depths + 1] = depth
			end

			-- Store the curve information for evaluation (only latest trajectory kept)
			self.curve_points_3d = points_3d
			self.curve_scale = scale
			self.curve_distance = distance
			self.curve_offset = { x = offsetX, y = offsetY }

			if #points_2d >= 2 then
				local avg_depth = 0
				for i = 1, #depths do
					avg_depth = avg_depth + depths[i]
				end

				g:set_color(table.unpack(self:index_to_color(index)))
				local path = Path(points_2d[1][1], points_2d[1][2])
				for i = 2, #points_2d do
					local p1 = points_2d[i]
					path:line_to(p1[1], p1[2])
				end

				g:stroke_path(path, 1)
			end
		end
	end
end

--╭─────────────────────────────────────╮
--│            Paint points             │
--╰─────────────────────────────────────╯
function projection:paint_layer_4(g)
	local width, height = self:get_size()
	local max_scale = math.max(self.scale_xyz.x, self.scale_xyz.y, self.scale_xyz.z)
	local scale = (math.min(width, height) * self.scale_global) / max_scale

	local offsetX = width / 2
	local offsetY = height / 2
	local distance = 2

	for index, trajectory in pairs(self.trajectories) do
		local point = trajectory.point
		if point then
			local scaled = {
				point[1] * self.scale_xyz.x,
				point[2] * self.scale_xyz.y,
				point[3] * self.scale_xyz.z,
			}
			local p = self:rotate_y(scaled, self.rotation_y)
			p = self:rotate_x(p, self.rotation_x)

			local depth = math.max(0.1, distance - p[3])
			local z = 1 / depth
			local x2d = p[1] * z * scale + offsetX
			local y2d = p[2] * z * scale + offsetY

			local size = 15 / (depth ^ 1.5)
			size = math.max(2, math.min(size, 10))

			-- Color per trajectory index
			g:set_color(255, 255, 255)
			g:fill_ellipse(x2d - size / 2, y2d - size / 2, size, size)
			g:draw_text(index, x2d + (size * 0.5), y2d + (size * 0.5), 20, size * 1.2) -- Draws text at the specified
		end
	end
end

--╭─────────────────────────────────────╮
--│         3D to 2D projection         │
--╰─────────────────────────────────────╯
-- https://en.wikipedia.org/wiki/Rotation_matrix
function projection:rotate_x(v, angle)
	if #v < 3 then
		self:error("Expected a 3D vector table")
	end

	local x, y, z = table.unpack(v)
	local c, s = math.cos(angle), math.sin(angle)
	return { x, y * c - z * s, y * s + z * c }
end

-- ─────────────────────────────────────
-- https://en.wikipedia.org/wiki/Rotation_matrix
function projection:rotate_y(v, angle)
	if #v < 3 then
		self:error("Expected a 3D vector table")
	end
	local x, y, z = table.unpack(v)
	local c, s = math.cos(angle), math.sin(angle)
	return { x * c + z * s, y, -x * s + z * c }
end

-- ─────────────────────────────────────
function projection:draw_3d_axis(g, axis_vec, label, color, scale, offsetX, offsetY, distance)
	if #axis_vec < 3 then
		self:error("Expected a 3D vector table")
	end
	local origin = { 0, 0, 0 }
	local tip = axis_vec

	origin = self:rotate_y(origin, self.rotation_y)
	origin = self:rotate_x(origin, self.rotation_x)

	tip = self:rotate_y(tip, self.rotation_y)
	tip = self:rotate_x(tip, self.rotation_x)

	local function project(v)
		local depth = distance - v[3]
		local z = 1 / depth
		return {
			v[1] * z * scale + offsetX,
			v[2] * z * scale + offsetY,
		}
	end

	local p1 = project(origin)
	local p2 = project(tip)

	g:set_color(table.unpack(color))
	g:draw_line(p1[1], p1[2], p2[1], p2[2], 1.5)

	g:draw_text(label, p2[1] + 3, p2[2] + 3, 12, 12)
end

-- ─────────────────────────────────────
function projection:evaluate_curve(t, trajectory)
	local tx = trajectory.x
	local ty = trajectory.y
	local tz = trajectory.z

	local count = math.min(#tx or 0, #ty or 0, #tz or 0)
	if count < 2 then
		self:error("Something wrong on evaluate_curve")
		return nil
	end

	-- Clamp t to [0, 1]
	t = math.max(0, math.min(1, t))
	local segment = t * (count - 1)
	local i = math.floor(segment) + 1
	local frac = segment - math.floor(segment)

	local i_next = math.min(count, i + 1)

	-- Linear interpolation in 3D space
	local x = tx[i] * (1 - frac) + tx[i_next] * frac
	local y = ty[i] * (1 - frac) + ty[i_next] * frac
	local z = tz[i] * (1 - frac) + tz[i_next] * frac

	return { x, y, z }
end

--╭─────────────────────────────────────╮
--│                 DEV                 │
--╰─────────────────────────────────────╯
function projection:in_1_reload()
	self:dofilex(self._scriptname)
	self:initialize("", {})
	self:repaint()
end
