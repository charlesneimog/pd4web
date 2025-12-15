// This is automatically generated code from pd4web.py script
#ifndef PD4WEB_CONFIG_H
#define PD4WEB_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// clang-format off
// Pd4Web Version
#define PD4WEB_VERSION_MAJOR @PD4WEB_VERSION_MAJOR@
#define PD4WEB_VERSION_MINOR @PD4WEB_VERSION_MINOR@
#define PD4WEB_VERSION_PATCH @PD4WEB_VERSION_PATCH@

// Sound
#define PD4WEB_PROJECT_NAME @PD4WEB_PROJECT_NAME@
#define PD4WEB_CHS_IN @PD4WEB_CHS_IN@
#define PD4WEB_CHS_OUT @PD4WEB_CHS_OUT@
#define PD4WEB_SR @PD4WEB_SR@

// Gui
#define PD4WEB_GUI @PD4WEB_GUI@
#define PD4WEB_PATCH_ZOOM @PD4WEB_PATCH_ZOOM@

// Midi
#define PD4WEB_MIDI @PD4WEB_MIDI@

// Mobile Keyboard Input
@PD4WEB_NUMBER_INPUT@
@PD4WEB_QWERTY_INPUT@

// clang-format on

// ╭─────────────────────────────────────╮
// │             Sound Icons             │
// ╰─────────────────────────────────────╯
#define ICON_SOUND_OFF                                                                             \
    "data:image/"                                                                                  \
    "svg+xml;base64,"                                                                              \
    "PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCA1NzYgNTEyIj48IS0tIUZv" \
    "bnQgQXdlc29tZSBGcmVlIDYuNi4wIGJ5IEBmb250YXdlc29tZSAtIGh0dHBzOi8vZm9udGF3ZXNvbWUuY29tIExpY2Vu" \
    "c2UgLSBodHRwczovL2ZvbnRhd2Vzb21lLmNvbS9saWNlbnNlL2ZyZWUgQ29weXJpZ2h0IDIwMjQgRm9udGljb25zLCBJ" \
    "bmMuLS0+"                                                                                     \
    "PHBhdGggZD0iTTMwMS4xIDM0LjhDMzEyLjYgNDAgMzIwIDUxLjQgMzIwIDY0bDAgMzg0YzAgMTIuNi03LjQgMjQtMTgu" \
    "OSAyOS4ycy0yNSAzLjEtMzQuNC01LjNMMTMxLjggMzUyIDY0IDM1MmMtMzUuMyAwLTY0LTI4LjctNjQtNjRsMC02NGMw" \
    "LTM1LjMgMjguNy02NCA2NC02NGw2Ny44IDBMMjY2LjcgNDAuMWM5LjQtOC40IDIyLjktMTAuNCAzNC40LTUuM3pNNDI1" \
    "IDE2N2w1NSA1NSA1NS01NWM5LjQtOS40IDI0LjYtOS40IDMzLjkgMHM5LjQgMjQuNiAwIDMzLjlsLTU1IDU1IDU1IDU1" \
    "YzkuNCA5LjQgOS40IDI0LjYgMCAzMy45cy0yNC42IDkuNC0zMy45IDBsLTU1LTU1LTU1IDU1Yy05LjQgOS40LTI0LjYg" \
    "OS40LTMzLjkgMHMtOS40LTI0LjYgMC0zMy45bDU1LTU1LTU1LTU1Yy05LjQtOS40LTkuNC0yNC42IDAtMzMuOXMyNC42" \
    "LTkuNCAzMy45IDB6Ii8+PC9zdmc+"

#define ICON_SOUND_ON                                                                              \
    "data:image/"                                                                                  \
    "svg+xml;base64,"                                                                              \
    "PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCA2NDAgNTEyIj48IS0tIUZv" \
    "bnQgQXdlc29tZSBGcmVlIDYuNi4wIGJ5IEBmb250YXdlc29tZSAtIGh0dHBzOi8vZm9udGF3ZXNvbWUuY29tIExpY2Vu" \
    "c2UgLSBodHRwczovL2ZvbnRhd2Vzb21lLmNvbS9saWNlbnNlL2ZyZWUgQ29weXJpZ2h0IDIwMjQgRm9udGljb25zLCBJ" \
    "bmMuLS0+"                                                                                     \
    "PHBhdGggZmlsbD0iIzAwMDAwMCIgZD0iTTUzMy42IDMyLjVDNTk4LjUgODUuMiA2NDAgMTY1LjggNjQwIDI1NnMtNDEu" \
    "NSAxNzAuNy0xMDYuNCAyMjMuNWMtMTAuMyA4LjQtMjUuNCA2LjgtMzMuOC0zLjVzLTYuOC0yNS40IDMuNS0zMy44QzU1" \
    "Ny41IDM5OC4yIDU5MiAzMzEuMiA1OTIgMjU2cy0zNC41LTE0Mi4yLTg4LjctMTg2LjNjLTEwLjMtOC40LTExLjgtMjMu" \
    "NS0zLjUtMzMuOHMyMy41LTExLjggMzMuOC0zLjV6TTQ3My4xIDEwN2M0My4yIDM1LjIgNzAuOSA4OC45IDcwLjkgMTQ5" \
    "cy0yNy43IDExMy44LTcwLjkgMTQ5Yy0xMC4zIDguNC0yNS40IDYuOC0zMy44LTMuNXMtNi44LTI1LjQgMy41LTMzLjhD" \
    "NDc1LjMgMzQxLjMgNDk2IDMwMS4xIDQ5NiAyNTZzLTIwLjctODUuMy01My4yLTExMS44Yy0xMC4zLTguNC0xMS44LTIz" \
    "LjUtMy41LTMzLjhzMjMuNS0xMS44IDMzLjgtMy41em0tNjAuNSA3NC41QzQzNC4xIDE5OS4xIDQ0OCAyMjUuOSA0NDgg" \
    "MjU2cy0xMy45IDU2LjktMzUuNCA3NC41Yy0xMC4zIDguNC0yNS40IDYuOC0zMy44LTMuNXMtNi44LTI1LjQgMy41LTMz" \
    "LjhDMzkzLjEgMjg0LjQgNDAwIDI3MSA0MDAgMjU2cy02LjktMjguNC0xNy43LTM3LjNjLTEwLjMtOC40LTExLjgtMjMu" \
    "NS0zLjUtMzMuOHMyMy41LTExLjggMzMuOC0zLjV6TTMwMS4xIDM0LjhDMzEyLjYgNDAgMzIwIDUxLjQgMzIwIDY0bDAg" \
    "Mzg0YzAgMTIuNi03LjQgMjQtMTguOSAyOS4ycy0yNSAzLjEtMzQuNC01LjNMMTMxLjggMzUyIDY0IDM1MmMtMzUuMyAw" \
    "LTY0LTI4LjctNjQtNjRsMC02NGMwLTM1LjMgMjguNy02NCA2NC02NGw2Ny44IDBMMjY2LjcgNDAuMWM5LjQtOC40IDIy" \
    "LjktMTAuNCAzNC40LTUuM3oiLz48L3N2Zz4="

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
        DRAW_SVG,
        STROKE_PATH,
        FILL_PATH,
        FILL_ALL,
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
    char *svg;
    int path_size;
} GuiCommand;

extern void ClearLayerCommand(const char *obj_layer_id, int layer, int x, int y, int w, int h);
extern void AddNewCommand(const char *obj_layer_id, int layer, GuiCommand *c);
extern void EndPaintLayerCommand(const char *obj_layer_id, int layer);

#ifdef __cplusplus
}
#endif

#endif
