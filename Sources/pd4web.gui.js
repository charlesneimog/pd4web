// Copyright (c) 2020 Zack Lee: cuinjune@gmail.com
// GNU General Public License v3.0
// For information on usage and redistribution, and for a DISCLAIMER OF ALL WARRANTIES, see the file, "LICENSE" in this distribution.
// Code From: https://github.com/cuinjune/PdWebParty

//╭─────────────────────────────────────╮
//│            Gui Handling             │
//╰─────────────────────────────────────╯
function CreateItem(type, args) {
    var item = document.createElementNS("http://www.w3.org/2000/svg", type);
    if (args !== null) {
        ConfigureItem(item, args);
    }
    Pd4Web.Canvas.appendChild(item);
    return item;
}

// ─────────────────────────────────────
function ConfigureItem(item, attributes) {
    var value, i, attr;
    if (Array.isArray(attributes)) {
        for (i = 0; i < attributes.length; i += 2) {
            value = attributes[i + 1];
            item.setAttributeNS(null, attributes[i], Array.isArray(value) ? value.join(" ") : value);
        }
    } else {
        for (attr in attributes) {
            if (attributes.hasOwnProperty(attr)) {
                if (item) {
                    item.setAttributeNS(null, attr, attributes[attr]);
                }
            }
        }
    }
}

// ─────────────────────────────────────
function IEMFontFamily(font) {
    let family = "";
    if (font === 1) {
        family = "'Helvetica', 'DejaVu Sans', 'sans-serif'";
    } else if (font === 2) {
        family = "'Times New Roman', 'DejaVu Serif', 'FreeSerif', 'serif'";
    } else {
        family = "'DejaVu Sans Mono', 'monospace'";
    }
    return family;
}

// ─────────────────────────────────────
function ColFromLoad(col) {
    // decimal to hex color
    if (typeof col === "string") {
        return col;
    }
    col = -1 - col;
    col = ((col & 0x3f000) << 6) | ((col & 0xfc0) << 4) | ((col & 0x3f) << 2);
    return "#" + ("000000" + col.toString(16)).slice(-6);
}

//╭─────────────────────────────────────╮
//│          Binder Receivers           │
//╰─────────────────────────────────────╯
function BindGuiReceiver(data) {
    if (data.receive in Pd4Web.GuiReceivers) {
        Pd4Web.GuiReceivers[data.receive].push(data);
    } else {
        Pd4Web.GuiReceivers[data.receive] = [data];
    }

    if (Pd4Web) {
        Pd4Web.bindGuiReceiver(data.receive, data.type);
    } else {
        alert("Pd4Web not found, please report");
    }
}

// ─────────────────────────────────────
function UnbindGuiReceiver(data) {
    if (data.receive in Pd4Web.GuiReceivers) {
        const len = Pd4Web.GuiReceivers[data.receive].length;
        for (let i = 0; i < len; i++) {
            if (Pd4Web.GuiReceivers[data.receive][i].id === data.id) {
                Pd4Web.unbindReceiver(data.receive);
                Pd4Web.GuiReceivers[data.receive].splice(i, 1);
                if (!Pd4Web.GuiReceivers[data.receive].length) {
                    delete Pd4Web.GuiReceivers[data.receive];
                }
                break;
            }
        }
    }
}

// ─────────────────────────────────────
function GuiRect(data) {
    return {
        x: data.x_pos,
        y: data.y_pos,
        width: data.size,
        height: data.size,
        fill: ColFromLoad(data.bg_color),
        id: `${data.id}_rect`,
        class: "border clickable",
    };
}

// ─────────────────────────────────────
function GuiText(data) {
    return {
        x: data.x_pos + data.x_off,
        y: data.y_pos + data.y_off,
        "font-family": IEMFontFamily(data.font),
        "font-weight": "normal",
        "font-size": `${data.fontsize}px`,
        fill: ColFromLoad(data.label_color),
        transform: `translate(0, ${(data.fontsize / 2) * 0.6})`, // note: modified
        id: `${data.id}_text`,
        class: "unclickable",
    };
}

// ─────────────────────────────────────
function GuiMousePoint(e) {
    // transforms the mouse position
    let point = Pd4Web.Canvas.createSVGPoint();
    point.x = e.clientX;
    point.y = e.clientY;
    point = point.matrixTransform(Pd4Web.Canvas.getScreenCTM().inverse());
    return point;
}

//╭─────────────────────────────────────╮
//│              Bang: Bng              │
//╰─────────────────────────────────────╯
function GuiBngRect(data) {
    return GuiRect(data);
}

// ─────────────────────────────────────
function GuiBngCircle(data) {
    const r = (data.size - 2) / 2;
    const cx = data.x_pos + r + 1;
    const cy = data.y_pos + r + 1;
    return {
        cx: cx,
        cy: cy,
        r: r,
        fill: "none",
        id: `${data.id}_circle`,
        class: "border unclickable",
    };
}

// ─────────────────────────────────────
function GuiBngText(data) {
    return GuiText(data);
}

function GuiBngUpdateCircle(data) {
    if (data.flashed) {
        data.flashed = false;
        ConfigureItem(data.circle, {
            fill: ColFromLoad(data.fg_color),
        });
        if (data.interrupt_timer) {
            clearTimeout(data.interrupt_timer);
        }
        data.interrupt_timer = setTimeout(function () {
            data.interrupt_timer = null;
            ConfigureItem(data.circle, {
                fill: "none",
            });
        }, data.interrupt);
        data.flashed = true;
    } else {
        data.flashed = true;
        ConfigureItem(data.circle, {
            fill: ColFromLoad(data.fg_color),
        });
    }
    if (data.hold_timer) {
        clearTimeout(data.hold_timer);
    }
    data.hold_timer = setTimeout(function () {
        data.flashed = false;
        data.hold_timer = null;
        ConfigureItem(data.circle, {
            fill: "none",
        });
    }, data.hold);
}

// ─────────────────────────────────────
function GuiBngOnMouseDown(data) {
    GuiBngUpdateCircle(data);
    Pd4Web.sendBang(data.receive);
}

//╭─────────────────────────────────────╮
//│             Toggle: Tgl             │
//╰─────────────────────────────────────╯
function GuiTglRect(data) {
    return GuiRect(data);
}

// ─────────────────────────────────────
function GuiTglCross1(data) {
    const w = ((data.size + 29) / 30) * 0.75; // note: modified
    const x1 = data.x_pos;
    const y1 = data.y_pos;
    const x2 = x1 + data.size;
    const y2 = y1 + data.size;
    const p1 = x1 + w + 1;
    const p2 = y1 + w + 1;
    const p3 = x2 - w - 1;
    const p4 = y2 - w - 1;
    const points = [p1, p2, p3, p4].join(" ");
    return {
        points: points,
        stroke: ColFromLoad(data.fg_color),
        "stroke-width": w,
        fill: "none",
        display: data.value ? "inline" : "none",
        id: `${data.id}_cross1`,
        class: "unclickable",
    };
}

// ─────────────────────────────────────
function GuiTglCross2(data) {
    const w = ((data.size + 29) / 30) * 0.75; // note: modified
    const x1 = data.x_pos;
    const y1 = data.y_pos;
    const x2 = x1 + data.size;
    const y2 = y1 + data.size;
    const p1 = x1 + w + 1;
    const p2 = y2 - w - 1;
    const p3 = x2 - w - 1;
    const p4 = y1 + w + 1;
    const points = [p1, p2, p3, p4].join(" ");
    return {
        points: points,
        stroke: ColFromLoad(data.fg_color),
        "stroke-width": w,
        fill: "none",
        display: data.value ? "inline" : "none",
        id: `${data.id}_cross2`,
        class: "unclickable",
    };
}

// ─────────────────────────────────────
function GuiTglText(data) {
    return GuiText(data);
}

// ─────────────────────────────────────
function GuiTglUpdateCross(data) {
    ConfigureItem(data.cross1, {
        display: data.value ? "inline" : "none",
    });
    ConfigureItem(data.cross2, {
        display: data.value ? "inline" : "none",
    });
}

// ─────────────────────────────────────
function GuiTglOnMouseDown(data) {
    data.value = data.value ? 0 : data.default_value;
    GuiTglUpdateCross(data);
    Pd4Web.sendFloat(data.send, data.value);
}

//╭─────────────────────────────────────╮
//│             Number: Nbx             │
//╰─────────────────────────────────────╯
function GuiNbxInvisibleRect(data) {
    let x = data.x_pos;
    let y = data.y_pos;
    let width = data.width * 9;
    let height = data.height;
    return {
        x: x,
        y: y,
        width: width,
        height: height,
        opacity: 0, // Set opacity to 0 to make it transparent
        fill: ColFromLoad(data.bg_color),
        id: `${data.id}_rect`,
        class: "border clickable",
    };
}

// ─────────────────────────────────────
function GuiNbxPolygon(data) {
    // Points
    const x1 = data.x_pos; // Replace with your actual x1 coordinate
    const y1 = data.y_pos; // Replace with your actual y1 coordinate

    const x2 = data.x_pos + data.width * 8; // Replace with your actual x2 coordinate
    const y2 = data.y_pos; // Replace with your actual y2 coordinate

    const x3 = data.x_pos + data.width * 9; // Replace with your actual x3 coordinate
    const y3 = data.y_pos + data.height * 0.2; // Replace with your actual y3 coordinate

    const x4 = data.x_pos + data.width * 9; // Replace with your actual x3 coordinate
    const y4 = data.y_pos + data.height; // Replace with your actual y3 coordinate

    const x5 = data.x_pos;
    const y5 = data.y_pos + data.height; // Replace with your actual y4 coordinate

    return {
        points: `${x1},${y1} ${x2},${y2} ${x3},${y3} ${x4},${y4} ${x5}, ${y5}`,
        id: `${data.id}_polygon`,
        fill: "none",
        class: "border clickable",
    };
}

// ─────────────────────────────────────
function GuiNbxTriangle(data) {
    var x1 = data.x_pos;
    var y1 = data.y_pos + 1;
    var x2 = data.x_pos;
    var y2 = data.y_pos + data.height - 1;
    var x3 = data.x_pos + 6;
    var y3 = data.y_pos + 7;

    return {
        points: `${x1 + 0.5},${y1} ${x2 + 0.5},${y2} ${x3},${y3}`,
        fill: "none",

        id: `${data.id}_triangle`,
        class: "border clickable",
    };
}

// ─────────────────────────────────────
function GuiNbxNumbers(data) {
    return {
        x: data.x_pos + 7,
        y: data.y_pos + data.fontsize * 0.85,
        "font-family": IEMFontFamily(data.font),
        "font-weight": "bold",
        "font-size": `${data.fontsize}px`,
        fill: ColFromLoad(data.label_color),
        transform: `translate(0, ${(data.fontsize / 2) * 0.6})`, // note: modified
        id: `${data.id}_numbers`,
        class: "unclickable",
    };
}

// ─────────────────────────────────────
function GuiNbxOnMouseDown(data, e, id) {
    const p = GuiMousePoint(e);
    // console.log(p);
    if (!data.steady_on_click) {
    }
    // gui_slider_bang(data);
    Pd4Web.Touches[id] = {
        data: data,
        point: p,
        value: data.value,
    };
}

// ─────────────────────────────────────
function GuiNbxClick(data, e, _) {
    // TODO: Make this better
    const p = GuiMousePoint(e);
    const numberText = data.numbers;
    let numberInput = prompt("Enter a number", numberText.textContent);
    numberText.textContent = parseFloat(numberInput);
    if (isNaN(parseFloat(numberInput))) {
        alert("Please enter a valid number");
        return;
    }
    sendFloat(data.receive, parseFloat(numberInput));
}

//╭─────────────────────────────────────╮
//│           Slider: vsl/hsl           │
//╰─────────────────────────────────────╯
function GuiSliderRect(data) {
    let x = data.x_pos;
    let y = data.y_pos;
    let width = data.width;
    let height = data.height;
    // if (data.type === "vsl") {
    //   y -= 2; // note: modified
    //   height += 5;
    // } else {
    //   x -= 3; // note: modified
    //   width += 5;
    // }
    return {
        x: x,
        y: y,
        width: width,
        height: height,
        fill: ColFromLoad(data.bg_color),
        id: `${data.id}_rect`,
        class: "border clickable",
    };
}

// ─────────────────────────────────────
function GuiSliderIndicatorPoints(data) {
    let x1 = data.x_pos;
    let y1 = data.y_pos;
    let x2 = x1 + data.width;
    let y2 = y1 + data.height;
    let r = 0;
    let p1 = 0;
    let p2 = 0;
    let p3 = 0;
    let p4 = 0;
    if (data.type === "vsl") {
        r = y2 - 3 - (data.value + 50) / 100;
        r = Math.max(y1 + 3, Math.min(r, y2 - 3));
        p1 = x1 + 2; //* 0.75; // note: modified | Largura
        p2 = r;
        p3 = x2 - 2; //* 0.75; // note: modified | Largura
        p4 = r;
    } else {
        r = x1 + 3 + (data.value + 50) / 100;
        r = Math.max(x1 + 3, Math.min(r, x2 - 3)); // Ensure r stays within the horizontal boundaries
        p1 = r;
        p2 = y1 + 2; //* 0.75; // note: modified | Largura
        p3 = r;
        p4 = y2 - 2; //* 0.75; // note: modified | Largura
    }
    return {
        x1: p1,
        y1: p2,
        x2: p3,
        y2: p4,
    };
}

// ─────────────────────────────────────
function GuiSliderIndicator(data) {
    const p = GuiSliderIndicatorPoints(data);
    return {
        x1: p.x1,
        y1: p.y1,
        x2: p.x2,
        y2: p.y2,
        stroke: ColFromLoad(data.fg_color),
        "stroke-width": 3,
        fill: "none",
        id: `${data.id}_indicator`,
        class: "unclickable",
    };
}

// ─────────────────────────────────────
function GuiSliderText(data) {
    return GuiText(data);
}

// ─────────────────────────────────────
function GuiSliderUpdateIndicator(data) {
    const p = GuiSliderIndicatorPoints(data);
    ConfigureItem(data.indicator, {
        x1: p.x1,
        y1: p.y1,
        x2: p.x2,
        y2: p.y2,
    });
}

// slider events

// ─────────────────────────────────────
function GuiSliderCheckMinMax(data) {
    if (data.log) {
        if (!data.bottom && !data.top) {
            data.top = 1;
        }
        if (data.top > 0) {
            if (data.bottom <= 0) {
                data.bottom = 0.01 * data.top;
            }
        } else {
            if (data.bottom > 0) {
                data.top = 0.01 * data.bottom;
            }
        }
    }
    data.reverse = data.bottom > data.top;
    const w = data.type === "vsl" ? data.height : data.width;
    if (data.log) {
        data.k = Math.log(data.top / data.bottom) / (w - 1);
    } else {
        data.k = (data.top - data.bottom) / (w - 1);
    }
}

// ─────────────────────────────────────
function GuiSliderSet(data, f) {
    let g = 0;

    if (data.reverse) {
        f = Math.max(Math.min(f, data.bottom), data.top);
    } else {
        f = Math.max(Math.min(f, data.top), data.bottom);
    }

    if (data.log) {
        g = Math.log(f / data.bottom) / data.k;
    } else {
        g = (f - data.bottom) / data.k;
    }

    data.value = 100 * g + 0.49999;
    GuiSliderUpdateIndicator(data);
}

// ─────────────────────────────────────
function GuiSliderBang(data) {
    let out = 0;
    if (data.log) {
        out = data.bottom * Math.exp(data.k * data.value * 0.01);
    } else {
        out = data.value * 0.01 * data.k + data.bottom;
    }
    if (data.reverse) {
        out = Math.max(Math.min(out, data.bottom), data.top);
    } else {
        out = Math.max(Math.min(out, data.top), data.bottom);
    }
    if (out < 1.0e-10 && out > -1.0e-10) {
        out = 0;
    }
    if (Pd4Web) {
        Pd4Web.sendFloat(data.receive, out);
    }
}

// ─────────────────────────────────────
function GuiSliderOnMouseDown(data, e, id) {
    const p = GuiMousePoint(e);
    if (!data.steady_on_click) {
        if (data.type === "vsl") {
            data.value = Math.max(Math.min(100 * (data.height + data.y_pos - p.y), (data.height - 50) * 100), 0);
        } else {
            data.value = Math.max(Math.min(100 * (p.x - data.x_pos), (data.width - 1) * 100), 0);
        }
        GuiSliderUpdateIndicator(data);
    }
    GuiSliderBang(data);
    Pd4Web.Touches[id] = {
        data: data,
        point: p,
        value: data.value,
    };
}

// ─────────────────────────────────────
function GuiSliderOnMouseMove(e, id) {
    if (id in Pd4Web.Touches) {
        const { data, point, value } = Pd4Web.Touches[id];
        const p = GuiMousePoint(e);
        if (data.type === "vsl") {
            data.value = Math.max(Math.min(value + (point.y - p.y) * 100, (data.height - 1) * 100), 0);
        } else {
            data.value = Math.max(Math.min(value + (p.x - point.x) * 100, (data.width - 1) * 100), 0);
        }
        GuiSliderUpdateIndicator(data);
        GuiSliderBang(data);
    }
}

// ─────────────────────────────────────
function GuiSliderOnMouseUp(id) {
    if (id in Pd4Web.Touches) {
        delete Pd4Web.Touches[id];
    }
}

//╭─────────────────────────────────────╮
//│        Radio: vradio/hradio         │
//╰─────────────────────────────────────╯
function GuiRadioRect(data) {
    let width = data.size;
    let height = data.size;
    if (data.type === "vradio") {
        height *= data.number;
    } else {
        width *= data.number;
    }
    return {
        x: data.x_pos,
        y: data.y_pos,
        stroke: "black",
        width: width,
        height: height,
        fill: ColFromLoad(data.bg_color),
        id: `${data.id}_rect`,
        class: "border clickable",
    };
}

// ─────────────────────────────────────
function GuiRadioLine(data, p1, p2, p3, p4, button_index) {
    return {
        x1: p1,
        y1: p2,
        x2: p3,
        y2: p4,
        id: `${data.id}_line_${button_index}`,
        class: "border unclickable",
    };
}

// ─────────────────────────────────────
function GuiRadioButton(data, p1, p2, p3, p4, button_index, state) {
    return {
        x: p1,
        y: p2,
        width: p3 - p1,
        height: p4 - p2,
        fill: ColFromLoad(data.fg_color),
        stroke: ColFromLoad(data.fg_color),
        display: state ? "inline" : "none",
        id: `${data.id}_button_${button_index}`,
        class: "unclickable",
    };
}

// ─────────────────────────────────────
function GuiRadioRemoveLinesButtons(data) {
    for (const line of data.lines) {
        line.parentNode.removeChild(line);
    }
    for (const button of data.buttons) {
        button.parentNode.removeChild(button);
    }
}

// ─────────────────────────────────────
function GuiRadioLinesButtons(data, is_creating) {
    const n = data.number;
    const d = data.size;
    const s = d / 4;
    const x1 = data.x_pos;
    const y1 = data.y_pos;
    let xi = x1;
    let yi = y1;
    const on = data.value;
    data.drawn = on;
    for (let i = 0; i < n; i++) {
        if (data.type === "vradio") {
            if (is_creating) {
                if (i) {
                    const line = CreateItem("line", GuiRadioLine(data, x1, yi, x1 + d, yi, i));
                    data.lines.push(line);
                }
                const button = CreateItem(
                    "rect",
                    GuiRadioButton(data, x1 + s, yi + s, x1 + d - s, yi + d - s, i, on === i),
                );
                data.buttons.push(button);
            } else {
                if (i) {
                    ConfigureItem(data.lines[i - 1], GuiRadioLine(data, x1, yi, x1 + d, yi, i));
                }
                ConfigureItem(
                    data.buttons[i],
                    GuiRadioButton(data, x1 + s, yi + s, x1 + d - s, yi + d - s, i, on === i),
                );
            }
            yi += d;
        } else {
            if (is_creating) {
                if (i) {
                    const line = CreateItem("line", GuiRadioLine(data, xi, y1, xi, y1 + d, i));
                    data.lines.push(line);
                }
                const button = CreateItem(
                    "rect",
                    GuiRadioButton(data, xi + s, y1 + s, xi + d - s, yi + d - s, i, on === i),
                );
                data.buttons.push(button);
            } else {
                if (i) {
                    ConfigureItem(data.lines[i - 1], GuiRadioLine(data, xi, y1, xi, y1 + d, i));
                }
                ConfigureItem(
                    data.buttons[i],
                    GuiRadioButton(data, xi + s, y1 + s, xi + d - s, yi + d - s, i, on === i),
                );
            }
            xi += d;
        }
    }
}

// ─────────────────────────────────────
function GuiRadioCreateLinesButtons(data) {
    data.lines = [];
    data.buttons = [];
    GuiRadioLinesButtons(data, true);
}

// ─────────────────────────────────────
function GuiRadioUpdateLinesButtons(data) {
    GuiRadioLinesButtons(data, false);
}

// ─────────────────────────────────────
function GuiRadioText(data) {
    return GuiText(data);
}

// ─────────────────────────────────────
function GuiRadioUpdateButton(data) {
    ConfigureItem(data.buttons[data.drawn], {
        display: "none",
    });
    ConfigureItem(data.buttons[data.value], {
        fill: ColFromLoad(data.fg_color),
        stroke: ColFromLoad(data.fg_color),
        display: "inline",
    });
    data.drawn = data.value;
}

// ─────────────────────────────────────
function GuiRadioOnMouseDown(data, e) {
    const p = GuiMousePoint(e);
    if (data.type === "vradio") {
        data.value = Math.floor((p.y - data.y_pos) / data.size);
    } else {
        data.value = Math.floor((p.x - data.x_pos) / data.size);
    }
    GuiRadioUpdateButton(data);
    sendFloat(data.receive, data.value);
}

//╭─────────────────────────────────────╮
//│             Vu: VuRect              │
//╰─────────────────────────────────────╯
function GuiVuRect(data) {
    let width = data.width;
    let height = data.height;
    return {
        x: data.x_pos,
        y: data.y_pos,
        width: width,
        height: height,
        fill: ColFromLoad(data.bg_color),
        id: `${data.id}_rect`,
        class: "unclickable",
    };
}

// ─────────────────────────────────────
function GuiVudBRects(data) {
    // inside this vu_rect I need to write 40 retangles
    var all_rects = [];
    var mini_rects_width = data.width - 6;
    var mini_rects_height = (data.height - 2) / 40;
    var minirect_x = data.x_pos + 3;
    var minirect_y = data.y_pos + 1;
    for (var i = 0; i < 40; i++) {
        var color = "#000000";
        if (i == 39) {
            color = "#28f4f4";
        } else if (i >= 23 && i <= 38) {
            color = "#14e814";
        } else if (i >= 14 && i <= 22) {
            color = "#e8e828";
        } else if (i >= 11 && i <= 13) {
            color = "#fcac44";
        } else if (i >= 1 && i <= 10) {
            color = "#fc2828";
        } else {
            color = "#f430f0";
        }
        var mini_rect = {
            x: minirect_x,
            y: minirect_y,
            fill: color,
            display: "none",
            width: mini_rects_width,
            index: i,
            height: mini_rects_height - 1,
            id: `${data.id}_mini_rect_${i}`,
        };
        var newrect = CreateItem("rect", mini_rect);
        all_rects.push(newrect);
        minirect_y += mini_rects_height;
    }
    data.mini_rects = all_rects;
}

// ─────────────────────────────────────
function GuiVuUpdateGain(data) {
    var amount_rect_to_draw = 0;
    if (data.value < -99) {
        amount_rect_to_draw = 0;
    }
    if (data.value > -100) {
        amount_rect_to_draw = 1;
    }
    if (data.value > -80) {
        amount_rect_to_draw = 2;
    }
    if (data.value > -60) {
        amount_rect_to_draw = 3;
    }
    if (data.value > -55) {
        amount_rect_to_draw = 4;
    }
    if (data.value > -50) {
        amount_rect_to_draw = 5;
    }
    if (data.value > -45) {
        amount_rect_to_draw = 6;
    }
    if (data.value > -40) {
        amount_rect_to_draw = 7;
    }
    if (data.value > -35) {
        amount_rect_to_draw = 8;
    }
    if (data.value > -30) {
        amount_rect_to_draw = 9;
    }
    if (data.value > -27) {
        amount_rect_to_draw = 10;
    }
    if (data.value > -25) {
        amount_rect_to_draw = 11;
    }
    if (data.value > -22) {
        amount_rect_to_draw = 12;
    }
    if (data.value > -20) {
        amount_rect_to_draw = 13;
    }
    if (data.value > -18) {
        amount_rect_to_draw = 13;
    }
    if (data.value > -16) {
        amount_rect_to_draw = 14;
    }
    if (data.value > -14) {
        amount_rect_to_draw = 15;
    }
    if (data.value > -12) {
        amount_rect_to_draw = 16;
    }
    if (data.value > -10) {
        amount_rect_to_draw = 17;
    }
    if (data.value > -9) {
        amount_rect_to_draw = 18;
    }
    if (data.value > -7) {
        amount_rect_to_draw = 19;
    }
    if (data.value > -6) {
        amount_rect_to_draw = 20;
    }
    if (data.value > -5) {
        amount_rect_to_draw = 21;
    }
    if (data.value > -4) {
        amount_rect_to_draw = 22;
    }
    if (data.value > -3) {
        amount_rect_to_draw = 23;
    }
    if (data.value > -2) {
        amount_rect_to_draw = 24;
    }
    if (data.value > -1) {
        amount_rect_to_draw = 26;
    }
    if (data.value > 0) {
        amount_rect_to_draw = 28;
    }
    if (data.value > 1) {
        amount_rect_to_draw = 30;
    }
    if (data.value > 2) {
        amount_rect_to_draw = 32;
    }
    if (data.value > 3) {
        amount_rect_to_draw = 33;
    }
    if (data.value > 4) {
        amount_rect_to_draw = 34;
    }
    if (data.value > 5) {
        amount_rect_to_draw = 35;
    }
    if (data.value > 6) {
        amount_rect_to_draw = 36;
    }
    if (data.value > 8) {
        amount_rect_to_draw = 37;
    }
    if (data.value > 9) {
        amount_rect_to_draw = 38;
    }
    if (data.value > 10) {
        amount_rect_to_draw = 39;
    }
    if (data.value > 11) {
        amount_rect_to_draw = 40;
    }

    for (var i = 0; i < 40; i++) {
        var coloredRect = 40 - i;
        if (coloredRect < amount_rect_to_draw) {
            data.mini_rects[i].setAttribute("display", "inline");
        } else {
            data.mini_rects[i].setAttribute("display", "none");
        }
    }
}

//╭─────────────────────────────────────╮
//│             Canvas: Cnv             │
//╰─────────────────────────────────────╯
function GuiCnvVisibleRect(data) {
    return {
        x: data.x_pos,
        y: data.y_pos,
        width: data.width,
        height: data.height,
        fill: ColFromLoad(data.bg_color),
        stroke: ColFromLoad(data.bg_color),
        id: `${data.id}_visible_rect`,
        class: "unclickable",
    };
}

// ─────────────────────────────────────
function GuiCnvSelectableRect(data) {
    return {
        x: data.x_pos,
        y: data.y_pos,
        width: data.size,
        height: data.size,
        fill: "none",
        stroke: ColFromLoad(data.bg_color),
        id: `${data.id}_selectable_rect`,
        class: "unclickable",
    };
}

// ─────────────────────────────────────
function GuiCnvText(data) {
    return GuiText(data);
}

//╭─────────────────────────────────────╮
//│                Font                 │
//╰─────────────────────────────────────╯
function GObjFontyKludge(fontsize) {
    switch (fontsize) {
        case 8:
            return -0.5;
        case 10:
            return -1;
        case 12:
            return -1;
        case 16:
            return -1.5;
        case 24:
            return -3;
        case 36:
            return -6;
        default:
            return 0;
    }
}

// ─────────────────────────────────────
function SetFontEngineSanity() {
    const canvas = document.createElement("canvas"),
        ctx = canvas.getContext("2d"),
        test_text = "struct theremin float x float y";
    canvas.id = "font_sanity_checker_canvas";

    if (document.body) {
        document.body.appendChild(canvas);
    }
    ctx.font = "11.65px DejaVu Sans Mono";
    if (Math.floor(ctx.measureText(test_text).width) <= 217) {
        Pd4Web.FontEngineSanity = true;
    } else {
        Pd4Web.FontEngineSanity = false;
    }
    if (canvas.parentNode) {
        canvas.parentNode.removeChild(canvas);
    }
}

// ─────────────────────────────────────
function FontStackIsMaintainedByTroglodytes() {
    return !Pd4Web.FontEngineSanity;
}

// ─────────────────────────────────────
function FontMap() {
    return {
        8: 8.33,
        12: 11.65,
        16: 16.65,
        24: 23.3,
        36: 36.6,
    };
}

// ─────────────────────────────────────
function SubOptimalFontMap() {
    return {
        8: 8.45,
        12: 11.4,
        16: 16.45,
        24: 23.3,
        36: 36,
    };
}

// ─────────────────────────────────────
function FontHeightMap() {
    return {
        8: 11,
        10: 13,
        12: 16,
        16: 19,
        24: 29,
        36: 44,
    };
}

// ─────────────────────────────────────
function GObjFontSizeKludge(fontsize, return_type) {
    var ret,
        prop,
        fmap = FontStackIsMaintainedByTroglodytes() ? SubOptimalFontMap() : FontMap();
    if (return_type === "gui") {
        ret = fmap[fontsize];
        return ret ? ret : fontsize;
    } else {
        for (prop in fmap) {
            if (fmap.hasOwnProperty(prop)) {
                if (fmap[prop] == fontsize) {
                    return +prop;
                }
            }
        }
        return fontsize;
    }
}

// ─────────────────────────────────────
function PdFontSizeToGuiFontSize(fontsize) {
    return GObjFontSizeKludge(fontsize, "gui");
}

// ─────────────────────────────────────
function GuiTextText(data, line_index) {
    const left_margin = 2;
    const fmap = FontHeightMap();
    const font_height = fmap[Pd4Web.FontSize] * (line_index + 1);
    return {
        transform: `translate(${left_margin - 0.5})`,
        x: data.x_pos,
        y: data.y_pos + font_height + GObjFontyKludge(Pd4Web.FontSize),
        "shape-rendering": "crispEdges",
        "font-size": PdFontSizeToGuiFontSize(Pd4Web.FontSize) + "px",
        "font-weight": "normal",
        id: `${data.id}_text_${line_index}`,
        class: "comment unclickable",
    };
}

//╭─────────────────────────────────────╮
//│           Patch Handling            │
//╰─────────────────────────────────────╯
function UpdatePatchDivSize(content) {
    const patchDiv = document.getElementById("Pd4WebPatchDiv");
    if (patchDiv == null) {
        return;
    }
    const lines = content.split(";\n");
    var args = lines[0].split(" ");
    const canvasWidth = parseInt(args[4]);
    const canvasHeight = parseInt(args[5]);
    const totalWidth = window.innerWidth;
    const totalHeight = window.innerHeight;

    const proporH = (totalHeight / canvasHeight) * 0.6;
    const proporW = (totalWidth / canvasWidth) * 0.6;

    patchDiv.style.width = canvasWidth * proporW + "px";
    patchDiv.style.height = canvasHeight * proporH + "px";
    patchDiv.style.marginLeft = "auto";
    patchDiv.style.marginRight = "auto";
    patchDiv.style.marginTop = "auto";
    patchDiv.style.marginBottom = "auto";
}

// ─────────────────────────────────────
function OpenPatch(content) {
    let canvasLevel = 0;
    let id = 0;

    while (Pd4Web.Canvas.lastChild) {
        Pd4Web.Canvas.removeChild(Pd4Web.Canvas.lastChild);
    }

    const lines = content.split(";\n");
    for (let line of lines) {
        line = line.replace(/[\r\n]+/g, " ").trim(); // remove newlines & carriage returns
        const args = line.split(" ");
        const type = args.slice(0, 2).join(" ");
        switch (type) {
            case "#N canvas":
                canvasLevel++;
                if (canvasLevel === 1 && args.length === 7) {
                    // should be called only once
                    Pd4Web.CanvasWidth = parseInt(args[4]);
                    Pd4Web.CanvasHeight = parseInt(args[5]);
                    Pd4Web.FontSize = parseInt(args[6]);
                    Pd4Web.Canvas.setAttributeNS(null, "viewBox", `0 0 ${Pd4Web.CanvasWidth} ${Pd4Web.CanvasHeight}`);
                }
                break;
            case "#X restore":
                canvasLevel--;
                break;
            case "#X obj":
                if (args.length > 4) {
                    switch (args[4]) {
                        case "bng":
                            if (
                                canvasLevel === 1 &&
                                args.length === 19 &&
                                args[9] !== "empty" &&
                                args[10] !== "empty"
                            ) {
                                const data = {};
                                data.x_pos = parseInt(args[2]);
                                data.y_pos = parseInt(args[3]);
                                data.type = args[4];
                                data.size = parseInt(args[5]);
                                data.hold = parseInt(args[6]);
                                data.interrupt = parseInt(args[7]);
                                data.init = parseInt(args[8]);
                                data.send = args[9];
                                data.receive = args[10];
                                data.label = args[11] === "empty" ? "" : args[11];
                                data.x_off = parseInt(args[12]);
                                data.y_off = parseInt(args[13]);
                                data.font = parseInt(args[14]);
                                data.fontsize = parseInt(args[15]);
                                data.bg_color = isNaN(args[16]) ? args[16] : parseInt(args[16]);
                                data.fg_color = isNaN(args[17]) ? args[17] : parseInt(args[17]);
                                data.label_color = isNaN(args[18]) ? args[18] : parseInt(args[18]);
                                data.id = `${data.type}_${id++}`;

                                // create svg
                                data.rect = CreateItem("rect", GuiBngRect(data));
                                data.circle = CreateItem("circle", GuiBngCircle(data));
                                data.text = CreateItem("text", GuiBngText(data));
                                data.text.textContent = data.label;

                                // handle event
                                data.flashed = false;
                                data.interrupt_timer = null;
                                data.hold_timer = null;
                                if (Pd4Web.isMobile) {
                                    data.rect.addEventListener("touchstart", function () {
                                        GuiBngOnMouseDown(data);
                                    });
                                } else {
                                    data.rect.addEventListener("mousedown", function () {
                                        GuiBngOnMouseDown(data);
                                    });
                                }
                                // subscribe receiver
                                BindGuiReceiver(data);
                            }
                            break;
                        case "tgl":
                            if (canvasLevel === 1 && args.length === 19 && args[7] !== "empty" && args[8] !== "empty") {
                                const data = {};
                                data.x_pos = parseInt(args[2]);
                                data.y_pos = parseInt(args[3]);
                                data.type = args[4];
                                data.size = parseInt(args[5]);
                                data.init = parseInt(args[6]);
                                data.send = args[7];
                                data.receive = args[8];
                                data.label = args[9] === "empty" ? "" : args[9];
                                data.x_off = parseInt(args[10]);
                                data.y_off = parseInt(args[11]);
                                data.font = parseInt(args[12]);
                                data.fontsize = parseInt(args[13]);
                                data.bg_color = isNaN(args[14]) ? args[14] : parseInt(args[14]);
                                data.fg_color = isNaN(args[15]) ? args[15] : parseInt(args[15]);
                                data.label_color = isNaN(args[16]) ? args[16] : parseInt(args[16]);
                                data.init_value = parseFloat(args[17]);
                                data.default_value = parseFloat(args[18]);
                                data.value = data.init && data.init_value ? data.default_value : 0;
                                data.id = `${data.type}_${id++}`;

                                // create svg
                                data.rect = CreateItem("rect", GuiTglRect(data));
                                data.cross1 = CreateItem("polyline", GuiTglCross1(data));
                                data.cross2 = CreateItem("polyline", GuiTglCross2(data));
                                data.text = CreateItem("text", GuiTglText(data));
                                data.text.textContent = data.label;

                                // handle event
                                if (Pd4Web.isMobile) {
                                    data.rect.addEventListener("touchstart", function () {
                                        GuiTglOnMouseDown(data);
                                    });
                                } else {
                                    data.rect.addEventListener("mousedown", function () {
                                        GuiTglOnMouseDown(data);
                                    });
                                }
                                BindGuiReceiver(data);
                            }
                            break;

                        case "nbx":
                            if (canvasLevel === 1 && args.length === 23 && args[7] !== "empty" && args[8] !== "empty") {
                                const data = {};
                                data.x_pos = parseInt(args[2]);
                                data.y_pos = parseInt(args[3]);
                                data.type = args[4];
                                data.width = parseInt(args[5]);
                                data.height = parseInt(args[6]);
                                data.bottom = parseInt(args[7]);
                                data.top = parseInt(args[8]);
                                data.log = parseInt(args[9]);
                                data.init = parseInt(args[10]);
                                data.send = args[11];
                                data.receive = args[12];
                                data.label = args[13] === "empty" ? "" : args[13];
                                data.x_off = parseInt(args[14]);
                                data.y_off = parseInt(args[15]);
                                data.font = parseInt(args[16]);
                                data.fontsize = parseInt(args[17]);
                                data.bg_color = isNaN(args[18]) ? args[18] : parseInt(args[18]);
                                data.fg_color = isNaN(args[19]) ? args[19] : parseInt(args[19]);
                                data.label_color = isNaN(args[20]) ? args[20] : parseInt(args[20]);
                                data.default_value = parseFloat(args[21]);
                                data.log_height = parseFloat(args[22]);
                                data.value = data.init ? data.default_value : 0;
                                data.id = `${data.type}_${id++}`;

                                // create svg
                                data.rect = CreateItem("rect", GuiNbxInvisibleRect(data));
                                data.polygon = CreateItem("polygon", GuiNbxPolygon(data));
                                data.triangle = CreateItem("polygon", GuiNbxTriangle(data));
                                data.numbers = CreateItem("text", GuiNbxNumbers(data));
                                data.numbers.textContent = data.init;

                                if (Pd4Web.isMobile) {
                                    data.rect.addEventListener("touchstart", function (e) {
                                        for (const _ of e.changedTouches) {
                                            // for (const touch of e.changedTouches) {
                                            // TODO: Check this
                                            // gui_slider_onmousedown(data, touch, touch.identifier);
                                        }
                                    });
                                } else {
                                    data.rect.addEventListener("click", function (e) {
                                        GuiNbxClick(data, e, 0);
                                    });
                                }
                                break;
                            }

                        case "vsl":
                        case "hsl":
                            if (
                                canvasLevel === 1 &&
                                args.length === 23 &&
                                args[11] !== "empty" &&
                                args[12] !== "empty"
                            ) {
                                const data = {};
                                data.x_pos = parseInt(args[2]);
                                data.y_pos = parseInt(args[3]);
                                data.type = args[4];
                                data.width = parseInt(args[5]);
                                data.height = parseInt(args[6]);
                                data.bottom = parseInt(args[7]);
                                data.top = parseInt(args[8]);
                                data.log = parseInt(args[9]);
                                data.init = parseInt(args[10]);
                                data.send = args[11];
                                data.receive = args[12];
                                data.label = args[13] === "empty" ? "" : args[13];
                                data.x_off = parseInt(args[14]);
                                data.y_off = parseInt(args[15]);
                                data.font = parseInt(args[16]);
                                data.fontsize = parseInt(args[17]);
                                data.bg_color = isNaN(args[18]) ? args[18] : parseInt(args[18]);
                                data.fg_color = isNaN(args[19]) ? args[19] : parseInt(args[19]);
                                data.label_color = isNaN(args[20]) ? args[20] : parseInt(args[20]);
                                data.default_value = parseFloat(args[21]);
                                data.steady_on_click = parseFloat(args[22]);
                                data.value = data.init ? data.default_value : 0;
                                data.id = `${data.type}_${id++}`;

                                // create svg
                                data.rect = CreateItem("rect", GuiSliderRect(data));
                                data.indicator = CreateItem("line", GuiSliderIndicator(data));
                                data.text = CreateItem("text", GuiSliderText(data));
                                data.text.textContent = data.label;

                                // handle event
                                GuiSliderCheckMinMax(data);
                                if (Pd4Web.isMobile) {
                                    data.rect.addEventListener("touchstart", function (e) {
                                        for (const touch of e.changedTouches) {
                                            GuiSliderOnMouseDown(data, touch, touch.identifier);
                                        }
                                    });
                                } else {
                                    data.rect.addEventListener("mousedown", function (e) {
                                        GuiSliderOnMouseDown(data, e, 0);
                                    });
                                }
                                // subscribe receiver
                                BindGuiReceiver(data);
                            }
                            break;
                        case "vradio":
                        case "hradio":
                            if (
                                canvasLevel === 1 &&
                                args.length === 20 &&
                                args[9] !== "empty" &&
                                args[10] !== "empty"
                            ) {
                                const data = {};
                                data.x_pos = parseInt(args[2]);
                                data.y_pos = parseInt(args[3]);
                                data.type = args[4];
                                data.size = parseInt(args[5]);
                                data.new_old = parseInt(args[6]);
                                data.init = parseInt(args[7]);
                                data.number = parseInt(args[8]) || 1;
                                data.send = args[9];
                                data.receive = args[10];
                                data.label = args[11] === "empty" ? "" : args[11];
                                data.x_off = parseInt(args[12]);
                                data.y_off = parseInt(args[13]);
                                data.font = parseInt(args[14]);
                                data.fontsize = parseInt(args[15]);
                                data.bg_color = isNaN(args[16]) ? args[16] : parseInt(args[16]);
                                data.fg_color = isNaN(args[17]) ? args[17] : parseInt(args[17]);
                                data.label_color = isNaN(args[18]) ? args[18] : parseInt(args[18]);
                                data.default_value = parseFloat(args[19]);
                                data.value = data.init ? data.default_value : 0;
                                data.id = `${data.type}_${id++}`;

                                // create svg
                                data.rect = CreateItem("rect", GuiRadioRect(data));
                                GuiRadioCreateLinesButtons(data);
                                data.text = CreateItem("text", GuiRadioText(data));
                                data.text.textContent = data.label;

                                // handle event
                                if (Pd4Web.isMobile) {
                                    data.rect.addEventListener("touchstart", function (e) {
                                        for (const touch of e.changedTouches) {
                                            GuiRadioOnMouseDown(data, touch);
                                        }
                                    });
                                } else {
                                    data.rect.addEventListener("mousedown", function (e) {
                                        GuiRadioOnMouseDown(data, e);
                                    });
                                }
                                BindGuiReceiver(data);
                            }
                            break;

                        case "vu":
                            if (canvasLevel === 1 && args.length === 17 && args[7] !== "empty") {
                                const data = {};
                                data.x_pos = parseInt(args[2]);
                                data.y_pos = parseInt(args[3]);
                                data.type = args[4];
                                // #X obj 331 111 vu 18 160 empty empty -1 -9 0 10 #404040 #000000 1 0;
                                data.width = args[5];
                                data.height = args[6];
                                data.receive = args[7];
                                // data.receive = args[8];
                                data.id = `${data.type}_${id++}`;

                                // create svg
                                data.rect = CreateItem("rect", GuiVuRect(data));
                                GuiVudBRects(data);

                                // subscribe receiver
                                BindGuiReceiver(data);
                            }
                            break;

                        case "cnv":
                            if (canvasLevel === 1 && args.length === 18 && args[8] !== "empty" && args[9] !== "empty") {
                                const data = {};
                                data.x_pos = parseInt(args[2]);
                                data.y_pos = parseInt(args[3]);
                                data.type = args[4];
                                data.size = parseInt(args[5]);
                                data.width = parseInt(args[6]);
                                data.height = parseInt(args[7]);
                                data.send = args[8];
                                data.receive = args[9];
                                data.label = args[10] === "empty" ? "" : args[10];
                                data.x_off = parseInt(args[11]);
                                data.y_off = parseInt(args[12]);
                                data.font = parseInt(args[13]);
                                data.fontsize = parseInt(args[14]);
                                data.bg_color = isNaN(args[15]) ? args[15] : parseInt(args[15]);
                                data.label_color = isNaN(args[16]) ? args[16] : parseInt(args[16]);
                                data.unknown = parseFloat(args[17]);
                                data.id = `${data.type}_${id++}`;

                                // create svg
                                data.visible_rect = CreateItem("rect", GuiCnvVisibleRect(data));
                                data.selectable_rect = CreateItem("rect", GuiCnvSelectableRect(data));
                                data.text = CreateItem("text", GuiCnvText(data));
                                data.text.textContent = data.label;

                                // subscribe receiver
                                BindGuiReceiver(data);
                            }
                            break;
                    }
                }
                break;
            case "#X text":
                if (args.length > 4 && canvasLevel === 1) {
                    // console.log(canvasLevel);
                    const data = {};
                    data.type = args[1];
                    data.x_pos = parseInt(args[2]);
                    data.y_pos = parseInt(args[3]);
                    data.comment = [];
                    const lines = args
                        .slice(4)
                        .join(" ")
                        .replace(/ \\,/g, ",")
                        .replace(/\\; /g, ";\n")
                        .replace(/ ;/g, ";")
                        .split("\n");
                    for (const line of lines) {
                        const lines = line.match(/.{1,60}(\s|$)/g);
                        for (const line of lines) {
                            data.comment.push(line.trim());
                        }
                    }
                    data.id = `${data.type}_${id++}`;
                    // TODO: Need to remove f {WIDTH} for when width of the text is especified

                    // create svg
                    data.texts = [];
                    for (let i = 0; i < data.comment.length; i++) {
                        const text = CreateItem("text", GuiTextText(data, i));
                        text.textContent = data.comment[i];
                        data.texts.push(text);
                    }
                }
                break;
        }
    }
    if (!canvasLevel) {
        alert("The main canvas not found in the pd file.");
        return;
    }
}

// ─────────────────────────────────────
async function Pd4WebInitGui() {
    if (Pd4Web === undefined) {
        setTimeout(Pd4WebInitGui, 150);
        console.log("Pd4Web is not defined yet");
        return;
    }

    Pd4Web.isMobile = navigator.userAgent.indexOf("IEMobile") !== -1;
    Pd4Web.CanvasWidth = 450;
    Pd4Web.CanvasHeight = 300;
    Pd4Web.FontSize = 12;
    Pd4Web.GuiReceivers = {};
    Pd4Web.Canvas = document.getElementById("Pd4WebCanvas");
    Pd4Web.Touches = {};
    Pd4Web.FontEngineSanity = false;

    if (Pd4Web.isMobile) {
        window.addEventListener("touchmove", function (e) {
            for (const touch of e.changedTouches) {
                GuiSliderOnMouseMove(touch, touch.identifier);
            }
        });
        window.addEventListener("touchend", function (e) {
            for (const touch of e.changedTouches) {
                GuiSliderOnMouseUp(touch.identifier);
            }
        });
        window.addEventListener("touchcancel", function (e) {
            for (const touch of e.changedTouches) {
                GuiSliderOnMouseUp(touch.identifier);
            }
        });
    } else {
        window.addEventListener("mousemove", function (e) {
            GuiSliderOnMouseMove(e, 0);
        });
        window.addEventListener("mouseup", function (_) {
            GuiSliderOnMouseUp(0);
        });
        window.addEventListener("mouseleave", function (_) {
            GuiSliderOnMouseUp(0);
        });
    }

    SetFontEngineSanity();
    var File = "./index.pd";
    fetch(File)
        .then((response) => {
            if (!response.ok) {
                throw new Error("Network response was not ok");
            }
            return response.text();
        })
        .then((textContent) => {
            UpdatePatchDivSize(textContent);
            OpenPatch(textContent);
        })
        .catch((error) => {
            console.error("There has been a problem with your fetch operation:", error);
        });
}
