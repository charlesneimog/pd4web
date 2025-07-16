// This is automatically generated code from pd4web.py script

// Pd4Web Version
#define PD4WEB_VERSION_MAJOR "test"
#define PD4WEB_VERSION_MINOR "dev"
#define PD4WEB_VERSION_PATCH "beta"

// Project Name
#define PD4WEB_PROJECT_NAME "Test10"

// Audio Config
#define PD4WEB_CHS_IN 1
#define PD4WEB_CHS_OUT 2
#define PD4WEB_SR 48000

// GUI Interface
#define PD4WEB_GUI true
#define PD4WEB_PATCH_ZOOM 2
#define PD4WEB_FPS 60
#define PD4WEB_AUTO_THEME true
#define PD4WEB_RENDER_THROUGH_IMAGE false
#define PD4WEB_MAX_FRAMERATE_MS 32  // 30fps limit (1000ms/30fps = 33.33ms, rounded to 32ms)

// Midi
#define PD4WEB_MIDI true

// ╭─────────────────────────────────────╮
// │          Lua Gui Interface          │
// ╰─────────────────────────────────────╯
enum LuaGuiCommands {
    FILL_ELLIPSE = 0,
    STROKE_ELLIPSE,
    FILL_RECT,
    STROKE_RECT,
    FILL_ROUNDED_RECT,
    STROKE_ROUNDED_RECT,
    DRAW_LINE,
    DRAW_TEXT,
    // DRAW_SVG, // TODO: implement this
    STROKE_PATH,
    FILL_PATH,
    FILL_ALL,
    // CLEAR_CANVAS,
};

typedef struct {
    enum LuaGuiCommands command;
    int drawed;
    char current_color[128];
    char layer_id[128];
    int objx;
    int objy;
    int objw;
    int objh;

    float x;
    float y;
    float w;
    float h;

    float line_width;
    float radius;
    float x1;
    float y1;
    float x2;
    float y2;
    char text[1024];
    float font_size;
    float stroke_width;
    float canvas_width;
    float canvas_height;
    float *path_coords;
    int path_size;
} GuiCommand;

extern void clear_layercommand(const char *obj_layer_id, int layer, int x, int y, int w, int h);
extern void add_newcommand(const char *obj_layer_id, int layer, GuiCommand *c);
extern void endpaint_layercommand(const char *obj_layer_id, int layer);
