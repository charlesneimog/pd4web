const ZOOM_LEVEL = 2;
var fontSize = 12;

window.addEventListener("resize", function () {
  // Your code to handle the window resize goes here
  console.log("Window resized!");
  // TODO: I believe that we need to redraw everything
});

function gobj_font_y_kludge(fontsize) {
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

let font_engine_sanity = false;

function font_map() {
  return {
    // pd_size: gui_size
    8: 8.33,
    12: 11.65,
    16: 16.65,
    24: 23.3,
    36: 36.6,
  };
}

function set_font_engine_sanity() {
  const canvas = document.createElement("pdPatch"),
    ctx = canvas.getContext("2d"),
    test_text = "struct theremin float x float y";
  canvas.id = "font_sanity_checker_canvas";
  document.body.appendChild(canvas);
  ctx.font = "13.65px DejaVu Sans Mono";
  if (Math.floor(ctx.measureText(test_text).width) <= 217) {
    font_engine_sanity = true;
  } else {
    font_engine_sanity = false;
  }
  canvas.parentNode.removeChild(canvas);
}

function suboptimal_font_map() {
  return {
    // pd_size: gui_size
    8: 12,
    12: 11.4,
    16: 16.45,
    24: 23.3,
    36: 36,
  };
}

function font_stack_is_maintained_by_troglodytes() {
  return true;
}

function gobj_fontsize_kludge(fontsize, return_type) {
  // These were tested on an X60 running Trisquel (based
  // on Ubuntu 14.04)
  var ret,
    prop,
    fmap = font_stack_is_maintained_by_troglodytes()
      ? suboptimal_font_map()
      : font_map();
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

function pd_fontsize_to_gui_fontsize(fontsize) {
  return gobj_fontsize_kludge(fontsize, "gui");
}

function font_height_map() {
  return {
    // fontsize: fontheight
    8: 11,
    10: 13,
    12: 16,
    16: 19,
    24: 29,
    36: 44,
  };
}

function gui_text_text(data, line_index) {
  const left_margin = 2;
  const fmap = font_height_map();
  const font_height = fmap[fontSize] * (line_index + 1);
  return {
    transform: `translate(${left_margin - 0.5})`,
    x: data.x_pos,
    y: data.y_pos + font_height + gobj_font_y_kludge(fontSize),
    "shape-rendering": "crispEdges",
    "font-size": pd_fontsize_to_gui_fontsize(fontSize) + "px",
    "font-weight": "normal",
    id: `${data.id}_text_${line_index}`,
    class: "unclickable",
  };
}

// captureLibPdMessages();
function create_item(type, args) {
  var canvas = document.getElementById("pdPatch");
  var item = document.createElementNS("http://www.w3.org/2000/svg", type);
  if (args !== null) {
    configure_item(item, args);
  }
  canvas.appendChild(item);
  return item;
}

function configure_item(item, attributes) {
  var value, i, attr;
  if (Array.isArray(attributes)) {
    for (i = 0; i < attributes.length; i += 2) {
      value = attributes[i + 1];
      item.setAttributeNS(
        null,
        attributes[i],
        Array.isArray(value) ? value.join(" ") : value,
      );
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

// ============
// Object Types
// ============
function draw_Bang(args) {
  var bng = {};
  bng.x_pos = parseInt(args[2]);
  bng.y_pos = parseInt(args[3]);
  bng.type = args[4];
  bng.size = parseInt(args[5]);
  bng.init = parseInt(args[6]);
  bng.send = args[7];
  bng.receive = args[8];
  bng.label = args[9] === "empty" ? "" : args[9];
  bng.x_off = parseInt(args[10]);
  bng.y_off = parseInt(args[11]);
  bng.font = parseInt(args[12]);
  bng.fontsize = parseInt(args[13]);
  bng.bg_color = isNaN(args[14]) ? args[14] : parseInt(args[14]);
  bng.fg_color = isNaN(args[15]) ? args[15] : parseInt(args[15]);
  bng.label_color = isNaN(args[16]) ? args[16] : parseInt(args[16]);
  bng.init_value = parseFloat(args[17]);
  bng.default_value = parseFloat(args[18]);
  bng.value = bng.init && bng.init_value ? bng.default_value : 0;
  var svg = document.createElementNS("http://www.w3.org/2000/svg", "svg");
  var rect = document.createElementNS("http://www.w3.org/2000/svg", "rect");
  svg.style.userSelect = "none";
  rect.setAttributeNS(null, "x", bng.x_pos * ZOOM_LEVEL);
  rect.setAttributeNS(null, "y", bng.y_pos * ZOOM_LEVEL);
  rect.setAttributeNS(null, "width", bng.size * ZOOM_LEVEL);
  rect.setAttributeNS(null, "height", bng.size * ZOOM_LEVEL);
  rect.setAttributeNS(null, "rx", 1);
  rect.setAttribute("fill", "rgba(0,0,0,0)");
  rect.setAttribute("stroke", "black");
  rect.setAttribute("stroke-width", 1);
  // rect.setAttribute("id", "bng_" + objId);
  var circle = document.createElementNS("http://www.w3.org/2000/svg", "circle");
  circle.setAttribute("cx", (bng.x_pos + bng.size / 2) * ZOOM_LEVEL);
  circle.setAttribute("cy", (bng.y_pos + bng.size / 2) * ZOOM_LEVEL);
  circle.setAttribute("r", bng.size - 1);
  circle.setAttribute("fill", "rgba(0,0,0,0)");
  circle.setAttribute("stroke", "black");
  circle.setAttribute("stroke-width", 1);
  // circle.setAttribute("id", "bng_" + objId);
  svg.appendChild(circle);
  svg.appendChild(rect);
  svg.onclick = function (event) {
    var eventTarget = event.target;
    var parent = eventTarget.parentNode;
    var children = parent.childNodes;
    var circle = children[0];
    circle.setAttribute("fill", "black");
    setTimeout(function () {
      circle.setAttribute("fill", "rgba(0,0,0,0)");
    }, 150);
  };
  return svg;
}

// ============
function draw_Obj(args) {
  let obj = {};
  obj.x_pos = parseInt(args[2]);
  obj.y_pos = parseInt(args[3]);
  obj.objText = args.slice(4).join(" ").replace(/;/g, "");
  let svg = document.createElementNS("http://www.w3.org/2000/svg", "svg");
  return svg;
}

// ============
function draw_Tgl(args) {
  var data = {};
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
  var svg = document.createElementNS("http://www.w3.org/2000/svg", "svg");
  var rect = document.createElementNS("http://www.w3.org/2000/svg", "rect");
  rect.setAttribute("x", data.x_pos * ZOOM_LEVEL);
  rect.setAttribute("y", data.y_pos * ZOOM_LEVEL);
  rect.setAttribute("width", data.size * ZOOM_LEVEL);
  rect.setAttribute("height", data.size * ZOOM_LEVEL);
  rect.setAttribute("fill", "rgba(0,0,0,0)");
  rect.setAttribute("stroke", data.fg_color);
  rect.setAttribute("stroke-width", 1);
  rect.setAttribute("rx", 1);
  var x1 = data.x_pos * ZOOM_LEVEL;
  var y1 = data.y_pos * ZOOM_LEVEL;
  var x2 = x1 + data.size * ZOOM_LEVEL;
  var y2 = y1 + data.size * ZOOM_LEVEL;
  var x3 = x1 + 3;
  var y3 = y2 - 3;
  var x4 = x2 - 3;
  var y4 = y1 + 3;

  var line1 = document.createElementNS("http://www.w3.org/2000/svg", "line");
  line1.setAttribute("x1", x1 + 3);
  line1.setAttribute("y1", y1 + 3);
  line1.setAttribute("x2", x2 - 3);
  line1.setAttribute("y2", y2 - 3);
  line1.setAttribute("stroke", "none");
  line1.setAttribute("stroke-width", ZOOM_LEVEL);

  var line2 = document.createElementNS("http://www.w3.org/2000/svg", "line");
  line2.setAttribute("x1", x3);
  line2.setAttribute("y1", y3);
  line2.setAttribute("x2", x4);
  line2.setAttribute("y2", y4);
  line2.setAttribute("stroke", "none");
  line2.setAttribute("stroke-width", ZOOM_LEVEL);
  svg.appendChild(line1);
  svg.appendChild(line2);
  svg.appendChild(rect);
  svg.setAttribute("receive", data.receive);
  svg.style.userSelect = "none";
  svg.onclick = function (event) {
    var eventTarget = event.target;
    var parent = eventTarget.parentNode;
    var receive = parent.getAttribute("receive");
    var children = parent.childNodes;
    var line1 = children[0];
    var line2 = children[1];
    if (line1.getAttribute("stroke") === "black") {
      line1.setAttribute("stroke", "none");
      line2.setAttribute("stroke", "none");
      sendFloat(receive, 0);
    } else {
      line1.setAttribute("stroke", "black");
      line2.setAttribute("stroke", "black");
      sendFloat(receive, 1);
    }
  };
  return svg;
}

// ============
function draw_Nbx(args) {
  const nbx = {};
  nbx.x_pos = parseInt(args[2]);
  nbx.y_pos = parseInt(args[3]);
  nbx.minValue = parseFloat(args[5]);
  nbx.maxValue = parseFloat(args[6]);
  nbx.sender = args[11];
  nbx.receiver = args[12];
  nbx.initValue = args[21];

  var svg = document.createElementNS("http://www.w3.org/2000/svg", "svg");
  svg.setAttribute("id", nbx.receiver);
  var rect = document.createElementNS("http://www.w3.org/2000/svg", "rect");

  // draw the box and put it in x_pos and y_pos position
  rect.setAttributeNS(null, "x", nbx.x_pos * ZOOM_LEVEL);
  rect.setAttributeNS(null, "y", nbx.y_pos * ZOOM_LEVEL);
  rect.setAttributeNS(null, "width", 40 * ZOOM_LEVEL);
  rect.setAttributeNS(null, "height", 10 * ZOOM_LEVEL);
  // rect.setAttributeNS(null, "rx", 2);
  rect.setAttribute("fill", "rgba(0,0,0,0)");
  rect.setAttribute("stroke", "black");
  rect.setAttribute("stroke-width", 0.5);

  // write and triangle
  var x1 = nbx.x_pos * ZOOM_LEVEL;
  var y1 = nbx.y_pos * ZOOM_LEVEL + 1;

  var x2 = nbx.x_pos * ZOOM_LEVEL;
  var y2 = nbx.y_pos * ZOOM_LEVEL + 10 * ZOOM_LEVEL - 1;

  var x3 = nbx.x_pos * ZOOM_LEVEL + 5 * ZOOM_LEVEL;
  var y3 = nbx.y_pos * ZOOM_LEVEL + 5 * ZOOM_LEVEL - 0.5;

  // Create a polygon element (triangle)
  var triangle = document.createElementNS(
    "http://www.w3.org/2000/svg",
    "polygon",
  );
  triangle.setAttribute("points", `${x1},${y1} ${x2},${y2} ${x3},${y3}`);
  triangle.setAttribute("fill", "black");
  triangle.setAttribute("stroke", "black");
  svg.appendChild(triangle);
  svg.appendChild(rect);

  var fontSize = 20;
  var text = document.createElementNS("http://www.w3.org/2000/svg", "text");
  text.setAttribute("x", nbx.x_pos * ZOOM_LEVEL + 7 * ZOOM_LEVEL);
  text.setAttribute("y", nbx.y_pos * ZOOM_LEVEL + fontSize * 0.8);
  text.setAttribute("fill", "black");
  text.setAttribute("font-size", fontSize + "px");
  text.setAttribute("id", "nbxtext");
  text.textContent = nbx.initValue;
  svg.appendChild(text);
  svg.setAttribute("receiver", nbx.receiver);
  svg.style.userSelect = "none";

  svg.onclick = function (event) {
    var eventTarget = event.target;
    var parent = eventTarget.parentNode;
    console.log(parent);
    var text = parent.getElementById("nbxtext");
    text.setAttribute("fill", "red");
    var value = prompt("Please enter new value", text.textContent);
    text.textContent = value;
    sendFloat(parent.getAttribute("receiver"), parseFloat(value));
    text.setAttribute("fill", "black");
  };
  return svg;
}

// ============
function draw_Vsl(args) {
  const vsl = {};
  vsl.x_pos = parseInt(args[2]);
  vsl.y_pos = parseInt(args[3]);
  vsl.type = args[4];
  vsl.width = parseInt(args[5]);
  vsl.height = parseInt(args[6]);
  vsl.bottom = parseInt(args[7]);
  vsl.top = parseInt(args[8]);
  vsl.log = parseInt(args[9]);
  vsl.init = parseInt(args[10]);
  vsl.send = args[11];
  vsl.receive = args[12];
  vsl.label = args[13] === "empty" ? "" : args[13];
  vsl.x_off = parseInt(args[14]);
  vsl.y_off = parseInt(args[15]);
  vsl.font = parseInt(args[16]);
  vsl.fontsize = parseInt(args[17]);
  vsl.bg_color = isNaN(args[18]) ? args[18] : parseInt(args[18]);
  vsl.fg_color = isNaN(args[19]) ? args[19] : parseInt(args[19]);
  vsl.label_color = isNaN(args[20]) ? args[20] : parseInt(args[20]);
  vsl.default_value = parseFloat(args[21]);
  vsl.steady_on_click = parseFloat(args[22]);
  vsl.value = vsl.init ? vsl.default_value : 0;
  var svg = document.createElementNS("http://www.w3.org/2000/svg", "svg");
  var rect = document.createElementNS("http://www.w3.org/2000/svg", "rect");
  rect.setAttributeNS(null, "x", vsl.x_pos * ZOOM_LEVEL);
  rect.setAttributeNS(null, "y", vsl.y_pos * ZOOM_LEVEL);
  rect.setAttributeNS(null, "width", vsl.width * ZOOM_LEVEL);
  rect.setAttributeNS(null, "height", vsl.height * ZOOM_LEVEL);
  rect.setAttributeNS(null, "rx", 2);
  rect.setAttribute("fill", "rgba(0,0,0,0)");
  rect.setAttribute("stroke", vsl.fg_color);
  rect.setAttribute("stroke-width", 1);

  // total range of slider
  var rangePd = vsl.top - vsl.bottom;
  var rangePx = vsl.height;
  var rangeInit = vsl.init;
  var rangeValue = vsl.value;

  var horizontalLine = document.createElementNS(
    "http://www.w3.org/2000/svg",
    "line",
  );
  horizontalLine.setAttribute("x1", vsl.x_pos * ZOOM_LEVEL);
  horizontalLine.setAttribute("y1", (vsl.y_pos + 10) * ZOOM_LEVEL);
  horizontalLine.setAttribute("x2", (vsl.x_pos + vsl.width) * ZOOM_LEVEL);
  horizontalLine.setAttribute("y2", (vsl.y_pos + 10) * ZOOM_LEVEL);
  horizontalLine.setAttribute("stroke", "black");
  horizontalLine.setAttribute("stroke-width", 4);
  horizontalLine.setAttribute("id", "vslpos");
  svg.setAttribute("maxValue", vsl.top);
  svg.setAttribute("minValue", vsl.bottom);
  svg.setAttribute("receive", vsl.receive);

  svg.appendChild(horizontalLine);
  svg.style.userSelect = "none";
  svg.addEventListener("click", function (event) {
    let rect = event.target;
    let svg = rect.parentNode;
    let horizontalLine = svg.getElementById("vslpos");
    var minValue = svg.getAttribute("minValue");
    var maxValue = svg.getAttribute("maxValue");
    var rangePd = maxValue - minValue;
    var y = event.offsetY;
    if (horizontalLine) {
      if (y < vsl.y_pos * ZOOM_LEVEL + 4) {
        y = vsl.y_pos * ZOOM_LEVEL + 4;
      } else if (y > (vsl.y_pos + vsl.height) * ZOOM_LEVEL - 4) {
        y = (vsl.y_pos + vsl.height) * ZOOM_LEVEL - 4;
      }
      horizontalLine.setAttribute("y1", y);
      horizontalLine.setAttribute("y2", y);
      // Calculate the percentage position within the valid range
      var percentage =
        1 -
        (y - (vsl.y_pos * ZOOM_LEVEL + 4)) /
          ((vsl.y_pos + vsl.height - 4) * ZOOM_LEVEL -
            (vsl.y_pos * ZOOM_LEVEL + 4));

      // Map the percentage to the range [0, 1]
      if (percentage > 1) {
        percentage = 1;
      }
      if (percentage < 0) {
        percentage = 0;
      }
      var finalValue = rangePd * percentage + minValue;
      sendFloat(svg.getAttribute("receive"), finalValue);
      return;
    }
  });
  svg.addEventListener("mousemove", function (event) {
    if (event.buttons !== 1) {
      return;
    }
    var rect = event.target;
    var svg = rect.parentNode;
    var horizontalLine = svg.getElementById("vslpos");
    var minValue = svg.getAttribute("minValue");
    var maxValue = svg.getAttribute("maxValue");
    var rangePd = maxValue - minValue;
    var y = event.offsetY;
    if (horizontalLine) {
      if (y < vsl.y_pos * ZOOM_LEVEL + 4) {
        y = vsl.y_pos * ZOOM_LEVEL + 4;
      } else if (y > (vsl.y_pos + vsl.height) * ZOOM_LEVEL - 4) {
        y = (vsl.y_pos + vsl.height) * ZOOM_LEVEL - 4;
      }
      horizontalLine.setAttribute("y1", y);
      horizontalLine.setAttribute("y2", y);
      // Calculate the percentage position within the valid range
      var percentage =
        1 -
        (y - (vsl.y_pos * ZOOM_LEVEL + 4)) /
          ((vsl.y_pos + vsl.height - 4) * ZOOM_LEVEL -
            (vsl.y_pos * ZOOM_LEVEL + 4));

      // Map the percentage to the range [0, 1]
      if (percentage > 1) {
        percentage = 1;
      }
      if (percentage < 0) {
        percentage = 0;
      }
      var finalValue = rangePd * percentage + minValue;
      sendFloat(svg.getAttribute("receive"), finalValue);

      return;
    }
  });
  svg.appendChild(rect);
  return svg;
}

function draw_TextOld(args) {
  if (args.length > 4) {
    const data = {};
    data.type = args[1];
    data.x_pos = parseInt(args[2]) * ZOOM_LEVEL;
    data.y_pos = parseInt(args[3]) * ZOOM_LEVEL;
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
    data.texts = [];
    for (let i = 0; i < data.comment.length; i++) {
      const text = create_item("text", gui_text_text(data, i));
      text.textContent = data.comment[i];
      data.texts.push(text);
    }
  }
}
function draw_Text(args) {
  if (args.length > 4) {
    const data = {};
    data.type = args[1];
    data.x_pos = parseInt(args[2]) * ZOOM_LEVEL;
    data.y_pos = parseInt(args[3]) * ZOOM_LEVEL;
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
    // create svg to draw text
    let text = document.createElementNS("http://www.w3.org/2000/svg", "svg");
    text.setAttribute("x", data.x_pos);
    text.setAttribute("y", data.y_pos);
    text.setAttribute("width", 100);
    text.setAttribute("height", 100);
    text.setAttribute("id", "text");
    text.setAttribute("class", "unclickable");
    text.style.userSelect = "none";
    // create text createElement
    let textElement = document.createElementNS(
      "http://www.w3.org/2000/svg",
      "text",
    );
    textElement.setAttribute("x", data.x_pos);
    textElement.setAttribute("y", data.y_pos);
    textElement.setAttribute("fill", "black");
    textElement.setAttribute("font-size", fontSize + "px");
    textElement.setAttribute("id", "textElement");
    textElement.setAttribute("class", "unclickable");
    textElement.style.userSelect = "none";
    text.appendChild(textElement);
    // create tspan
    let tspan = document.createElementNS("http://www.w3.org/2000/svg", "tspan");
    tspan.setAttribute("x", data.x_pos);
    tspan.setAttribute("y", data.y_pos);
    tspan.setAttribute("fill", "black");
    tspan.setAttribute("font-size", fontSize + "px");
    tspan.setAttribute("id", "tspan");
    tspan.setAttribute("class", "unclickable");
    tspan.style.userSelect = "none";
    textElement.appendChild(tspan);
    // create textNode
    let textNode = document.createTextNode(data.comment[0]);
    tspan.appendChild(textNode);
    // append text to canvas
    let canvas = document.getElementById("pdPatch");
    canvas.appendChild(text);
  }
}

// ============
// ============
// ============
function openPatch(file) {
  var request = new XMLHttpRequest();
  var objId = 0;
  let id = 0; // gui id
  request.open("GET", file, true);
  request.onreadystatechange = function () {
    if (request.readyState === 4) {
      if (request.status === 200 || request.status == 0) {
        var lines = request.responseText.split("\n");
        let canvasLevel = 0; // 0: no canvas, 1: main canvas, 2~: subcanvases
        const canvas = document.getElementById("pdPatch");
        for (let line of lines) {
          line = line.replace(/[\r\n]+/g, " ").trim(); // remove newlines & carriage returns
          objId++;
          const args = line.split(" ");
          const type = args.slice(0, 2).join(" ");
          switch (type) {
            case "#N canvas":
              canvasLevel++;
              if (canvasLevel === 1 && args.length === 7) {
                var canvasWidth = parseInt(args[4]);
                var canvasHeight = parseInt(args[5]);
                fontSize = parseInt(args[6]);
                const canvas = document.getElementById("pdPatch");
                canvas.setAttribute(
                  "viewBox",
                  `0 0 ${canvasWidth} ${canvasHeight}`,
                );
                canvas.setAttribute("width", canvasWidth);
                canvas.setAttribute("height", canvasHeight);
                // add small shadow for canvas
                canvas.style.boxShadow = "0px 0px 5px 0px rgba(0,0,0,0.75)";
              }
              break;
            case "#X obj":
              const objName = args[4];
              switch (objName) {
                case "tgl":
                  if (!(args[7] === "empty" || args[8] === "empty")) {
                    canvas.appendChild(draw_Tgl(args));
                  }
                  break;
                case "vsl":
                  canvas.appendChild(draw_Vsl(args));
                  break;
                case "hsl":
                  // console.log("hsl");
                  break;
                case "nbx":
                  canvas.appendChild(draw_Nbx(args));
                  break;
                case "bng":
                  canvas.appendChild(draw_Bang(args));
                  break;
                default:
                  canvas.appendChild(draw_Obj(args));
                  break;
              }
              break;
            case "#X text":
              draw_Text(args);
              break;
          }
        }
      }
    }
  };
  request.send(null);
}

document.addEventListener("DOMContentLoaded", function () {
  openPatch("./data/index.pd");
});
