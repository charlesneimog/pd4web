local test = pd.Class:new():register("test")

-- ──────────────────────────────────────────
function test:initialize(_, args)
	self.inlets = 1
	self.outlets = 1
	self:set_size(100, 100)
	return true
end

-- ──────────────────────────────────────────
function test:paint(g)
	g:set_color(255, 255, 255)
	g:fill_all()
	
	-- stroke rect
	g:set_color(255, 0, 255)
	g:stroke_rect(5, 5, 10, 10, 2)
	
	-- fill rect
	g:set_color(0, 255, 0)
	g:fill_rect(80, 80, 10, 10)
	
	-- ellipse
	g:set_color(0, 125, 0)
	g:fill_ellipse(50, 50, 10, 10)
	
	-- ellipse
	g:set_color(0, 125, 120)
	g:stroke_ellipse(10, 50, 10, 10, 2)
	
end


-- ──────────────────────────────────────────
function test:in_1_reload()
	self:dofilex(self._scriptname)
	self:initialize()
end
