var Pd4WebModule = (() => {
  var _scriptName = typeof document != 'undefined' ? document.currentScript?.src : undefined;
  if (typeof __filename != 'undefined') _scriptName = _scriptName || __filename;
  return (
async function(moduleArg = {}) {
  var moduleRtn;

// include: shell.js
// The Module object: Our interface to the outside world. We import
// and export values on it. There are various ways Module can be used:
// 1. Not defined. We create it here
// 2. A function parameter, function(moduleArg) => Promise<Module>
// 3. pre-run appended it, var Module = {}; ..generated code..
// 4. External script tag defines var Module.
// We need to check if Module already exists (e.g. case 3 above).
// Substitution will be replaced with actual code on later stage of the build,
// this way Closure Compiler will not mangle it (e.g. case 4. above).
// Note that if you want to run closure, and also to use Module
// after the generated code, you will need to define   var Module = {};
// before the code. Then that object will be used in the code, and you
// can continue to use Module afterwards as well.
var Module = moduleArg;

// Set up the promise that indicates the Module is initialized
var readyPromiseResolve, readyPromiseReject;
var readyPromise = new Promise((resolve, reject) => {
  readyPromiseResolve = resolve;
  readyPromiseReject = reject;
});

// Determine the runtime environment we are in. You can customize this by
// setting the ENVIRONMENT setting at compile time (see settings.js).

var ENVIRONMENT_IS_AUDIO_WORKLET = typeof AudioWorkletGlobalScope !== 'undefined';

// Attempt to auto-detect the environment
var ENVIRONMENT_IS_WEB = typeof window == 'object';
var ENVIRONMENT_IS_WORKER = typeof WorkerGlobalScope != 'undefined';
// N.b. Electron.js environment is simultaneously a NODE-environment, but
// also a web environment.
var ENVIRONMENT_IS_NODE = typeof process == 'object' && typeof process.versions == 'object' && typeof process.versions.node == 'string' && process.type != 'renderer';
var ENVIRONMENT_IS_SHELL = !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_WORKER && !ENVIRONMENT_IS_AUDIO_WORKLET;

// Three configurations we can be running in:
// 1) We could be the application main() thread running in the main JS UI thread. (ENVIRONMENT_IS_WORKER == false and ENVIRONMENT_IS_PTHREAD == false)
// 2) We could be the application main() thread proxied to worker. (with Emscripten -sPROXY_TO_WORKER) (ENVIRONMENT_IS_WORKER == true, ENVIRONMENT_IS_PTHREAD == false)
// 3) We could be an application pthread running in a worker. (ENVIRONMENT_IS_WORKER == true and ENVIRONMENT_IS_PTHREAD == true)

// The way we signal to a worker that it is hosting a pthread is to construct
// it with a specific name.
var ENVIRONMENT_IS_PTHREAD = ENVIRONMENT_IS_WORKER && self.name?.startsWith('em-pthread');

if (ENVIRONMENT_IS_PTHREAD) {
  assert(!globalThis.moduleLoaded, 'module should only be loaded once on each pthread worker');
  globalThis.moduleLoaded = true;
}

if (ENVIRONMENT_IS_NODE) {

  var worker_threads = require('worker_threads');
  global.Worker = worker_threads.Worker;
  ENVIRONMENT_IS_WORKER = !worker_threads.isMainThread;
  // Under node we set `workerData` to `em-pthread` to signal that the worker
  // is hosting a pthread.
  ENVIRONMENT_IS_PTHREAD = ENVIRONMENT_IS_WORKER && worker_threads['workerData'] == 'em-pthread'
}

var ENVIRONMENT_IS_WASM_WORKER = !!Module['$ww'];

// --pre-jses are emitted after the Module integration code, so that they can
// refer to Module (if they choose; they can also define Module)
// include: /tmp/tmpbba4td8u.js

  Module['expectedDataFileDownloads'] ??= 0;
  Module['expectedDataFileDownloads']++;
  (() => {
    // Do not attempt to redownload the virtual filesystem data when in a pthread or a Wasm Worker context.
    var isPthread = typeof ENVIRONMENT_IS_PTHREAD != 'undefined' && ENVIRONMENT_IS_PTHREAD;
    var isWasmWorker = typeof ENVIRONMENT_IS_WASM_WORKER != 'undefined' && ENVIRONMENT_IS_WASM_WORKER;
    if (isPthread || isWasmWorker) return;
    var isNode = typeof process === 'object' && typeof process.versions === 'object' && typeof process.versions.node === 'string';
    function loadPackage(metadata) {

      var PACKAGE_PATH = '';
      if (typeof window === 'object') {
        PACKAGE_PATH = window['encodeURIComponent'](window.location.pathname.substring(0, window.location.pathname.lastIndexOf('/')) + '/');
      } else if (typeof process === 'undefined' && typeof location !== 'undefined') {
        // web worker
        PACKAGE_PATH = encodeURIComponent(location.pathname.substring(0, location.pathname.lastIndexOf('/')) + '/');
      }
      var PACKAGE_NAME = '/home/neimog/Downloads/test/WebPatch/pd4web.data';
      var REMOTE_PACKAGE_BASE = 'pd4web.data';
      var REMOTE_PACKAGE_NAME = Module['locateFile'] ? Module['locateFile'](REMOTE_PACKAGE_BASE, '') : REMOTE_PACKAGE_BASE;
var REMOTE_PACKAGE_SIZE = metadata['remote_package_size'];

      function fetchRemotePackage(packageName, packageSize, callback, errback) {
        if (isNode) {
          require('fs').readFile(packageName, (err, contents) => {
            if (err) {
              errback(err);
            } else {
              callback(contents.buffer);
            }
          });
          return;
        }
        Module['dataFileDownloads'] ??= {};
        fetch(packageName)
          .catch((cause) => Promise.reject(new Error(`Network Error: ${packageName}`, {cause}))) // If fetch fails, rewrite the error to include the failing URL & the cause.
          .then((response) => {
            if (!response.ok) {
              return Promise.reject(new Error(`${response.status}: ${response.url}`));
            }

            if (!response.body && response.arrayBuffer) { // If we're using the polyfill, readers won't be available...
              return response.arrayBuffer().then(callback);
            }

            const reader = response.body.getReader();
            const iterate = () => reader.read().then(handleChunk).catch((cause) => {
              return Promise.reject(new Error(`Unexpected error while handling : ${response.url} ${cause}`, {cause}));
            });

            const chunks = [];
            const headers = response.headers;
            const total = Number(headers.get('Content-Length') ?? packageSize);
            let loaded = 0;

            const handleChunk = ({done, value}) => {
              if (!done) {
                chunks.push(value);
                loaded += value.length;
                Module['dataFileDownloads'][packageName] = {loaded, total};

                let totalLoaded = 0;
                let totalSize = 0;

                for (const download of Object.values(Module['dataFileDownloads'])) {
                  totalLoaded += download.loaded;
                  totalSize += download.total;
                }

                Module['setStatus']?.(`Downloading data... (${totalLoaded}/${totalSize})`);
                return iterate();
              } else {
                const packageData = new Uint8Array(chunks.map((c) => c.length).reduce((a, b) => a + b, 0));
                let offset = 0;
                for (const chunk of chunks) {
                  packageData.set(chunk, offset);
                  offset += chunk.length;
                }
                callback(packageData.buffer);
              }
            };

            Module['setStatus']?.('Downloading data...');
            return iterate();
          });
      };

      function handleError(error) {
        console.error('package error:', error);
      };

      var fetchedCallback = null;
      var fetched = Module['getPreloadedPackage'] ? Module['getPreloadedPackage'](REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE) : null;

      if (!fetched) fetchRemotePackage(REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE, (data) => {
        if (fetchedCallback) {
          fetchedCallback(data);
          fetchedCallback = null;
        } else {
          fetched = data;
        }
      }, handleError);

    function runWithFS(Module) {

      function assert(check, msg) {
        if (!check) throw msg + new Error().stack;
      }
Module['FS_createPath']("/", "Extras", true, true);

      /** @constructor */
      function DataRequest(start, end, audio) {
        this.start = start;
        this.end = end;
        this.audio = audio;
      }
      DataRequest.prototype = {
        requests: {},
        open: function(mode, name) {
          this.name = name;
          this.requests[name] = this;
          Module['addRunDependency'](`fp ${this.name}`);
        },
        send: function() {},
        onload: function() {
          var byteArray = this.byteArray.subarray(this.start, this.end);
          this.finish(byteArray);
        },
        finish: function(byteArray) {
          var that = this;
          // canOwn this data in the filesystem, it is a slide into the heap that will never change
          Module['FS_createDataFile'](this.name, null, byteArray, true, true, true);
          Module['removeRunDependency'](`fp ${that.name}`);
          this.requests[this.name] = null;
        }
      };

      var files = metadata['files'];
      for (var i = 0; i < files.length; ++i) {
        new DataRequest(files[i]['start'], files[i]['end'], files[i]['audio'] || 0).open('GET', files[i]['filename']);
      }

      function processPackageData(arrayBuffer) {
        assert(arrayBuffer, 'Loading data file failed.');
        assert(arrayBuffer.constructor.name === ArrayBuffer.name, 'bad input to processPackageData');
        var byteArray = new Uint8Array(arrayBuffer);
        var curr;
        // Reuse the bytearray from the XHR as the source for file reads.
          DataRequest.prototype.byteArray = byteArray;
          var files = metadata['files'];
          for (var i = 0; i < files.length; ++i) {
            DataRequest.prototype.requests[files[i].filename].onload();
          }          Module['removeRunDependency']('datafile_/home/neimog/Downloads/test/WebPatch/pd4web.data');

      };
      Module['addRunDependency']('datafile_/home/neimog/Downloads/test/WebPatch/pd4web.data');

      Module['preloadResults'] ??= {};

      Module['preloadResults'][PACKAGE_NAME] = {fromCache: false};
      if (fetched) {
        processPackageData(fetched);
        fetched = null;
      } else {
        fetchedCallback = processPackageData;
      }

    }
    if (Module['calledRun']) {
      runWithFS(Module);
    } else {
      (Module['preRun'] ??= []).push(runWithFS); // FS is not initialized yet, wait for it
    }

    }
    loadPackage({"files": [{"filename": "/Extras/gui (Copy).lua", "start": 0, "end": 6752}, {"filename": "/Extras/gui.pd_lua", "start": 6752, "end": 13745}, {"filename": "/index.pd", "start": 13745, "end": 14117}, {"filename": "/pd.lua", "start": 14117, "end": 32013}, {"filename": "/pdx.lua", "start": 32013, "end": 40648}], "remote_package_size": 40648});

  })();

// end include: /tmp/tmpbba4td8u.js
// include: /tmp/tmpbak55vqa.js

    // All the pre-js content up to here must remain later on, we need to run
    // it.
    if (Module['$ww'] || (typeof ENVIRONMENT_IS_PTHREAD != 'undefined' && ENVIRONMENT_IS_PTHREAD)) Module['preRun'] = [];
    var necessaryPreJSTasks = Module['preRun'].slice();
  // end include: /tmp/tmpbak55vqa.js
// include: /home/neimog/.config/miniconda3.dir/lib/python3.12/site-packages/pd4web/pd4web.gui.js
// Copyright (c) 2020 Zack Lee: cuinjune@gmail.com
// GNU General Public License v3.0
// For information on usage and redistribution, and for a DISCLAIMER OF ALL WARRANTIES, see the file, "LICENSE" in this distribution.
// Code From: https://github.com/cuinjune/PdWebParty

/*
window.addEventListener("load", function () {
    setTimeout(function () {
        if (typeof SharedArrayBuffer === "undefined") {
            console.log("SharedArrayBuffer is not defined");
            alert(
                "Pd4Web could not load. Please try reloading the page. If the issue persists, contact pd4web developers. (pd4web does not work on private tabs).",
            );
        }
    }, 3000);
});
*/

//╭─────────────────────────────────────╮
//│            Auto Theming             │
//╰─────────────────────────────────────╯
function GetStyleRuleValue(className, stylesProb) {
    let style = {};
    var el = document.createElement("div");
    el.className = className;
    document.body.appendChild(el);
    var computedStyle = window.getComputedStyle(el);
    for (let prob in stylesProb) {
        let thisProb = stylesProb[prob];
        let value = computedStyle.getPropertyValue(thisProb);
        style[thisProb] = value;
    }
    document.body.removeChild(el);
    return style;
}

// TODO: Fazer pd4web em uma class

// ─────────────────────────────────────
function getCssVariable(variableName) {
    return getComputedStyle(document.documentElement).getPropertyValue(variableName).trim();
}

// ─────────────────────────────────────
function setSoundIcon(icon, animation) {
    let soundSwitch = document.getElementById("Pd4WebAudioSwitch");

    if (soundSwitch) {
        const soundOffSvg = getComputedStyle(document.documentElement).getPropertyValue(icon).trim();
        const svgData = soundOffSvg.match(/url\("data:image\/svg\+xml;base64,(.*)"\)/)?.[1];
        if (svgData === undefined) {
            window.setTimeout(() => {
                setSoundIcon(icon, animation);
            }, 1000);
            return;
        }

        const svgDecoded = atob(svgData);
        const parser = new DOMParser();
        const svgDoc = parser.parseFromString(svgDecoded, "image/svg+xml");
        const svgElement = svgDoc.querySelector("svg");
        if (svgElement) {
            if (Pd4Web.isMobile) {
                svgElement.setAttribute("width", "48");
                svgElement.setAttribute("height", "48");
            } else {
                svgElement.setAttribute("width", "24");
                svgElement.setAttribute("height", "24");
            }
            svgElement.style.display = "inline-block";
            svgElement.style.animation = animation;
            soundSwitch.innerHTML = "";
            soundSwitch.appendChild(svgElement);
        }
    } else {
        console.error("Pd4WebAudioSwitch not found");
    }
}

// ─────────────────────────────────────
function GetNeededStyles() {
    Pd4Web.Style = {};
    Pd4Web.Style.Bg = getCssVariable("--bg");
    Pd4Web.Style.Fg = getCssVariable("--fg");
    Pd4Web.Style.Sel = getCssVariable("--selected");

    let elements = document.querySelectorAll(".nbx-text");
    for (let element of elements) {
        element.style.fill = getCssVariable("--nbx-text");
    }

    elements = document.querySelectorAll(".key-black");
    for (let element of elements) {
        element.style.fill = getCssVariable("--keyboard-black-key");
    }

    elements = document.querySelectorAll(".key-white");
    for (let element of elements) {
        element.style.fill = getCssVariable("--keyboard-white-key");
    }

    elements = document.querySelectorAll(".vu-mini-rect");
    for (let element of elements) {
        element.style.fill = getCssVariable("--vu-active");
    }
}

// ─────────────────────────────────────
function ThemeListener(_) {
    GetNeededStyles();
}

// ─────────────────────────────────────
function GetRBG(hex) {
    hex = hex.replace(/^#/, "");
    let r = parseInt(hex.substr(0, 2), 16);
    let g = parseInt(hex.substr(2, 2), 16);
    let b = parseInt(hex.substr(4, 2), 16);
    return { r, g, b };
}

// ─────────────────────────────────────
function AlmostWhiteOrBlack(hex) {
    let rgb = GetRBG(hex);
    let almostBlack = rgb.r < 20 && rgb.g < 20 && rgb.b < 20;
    let almostWhite = rgb.r > 235 && rgb.g > 235 && rgb.b > 235;
    return almostBlack || almostWhite;
}

// ─────────────────────────────────────
// Interative listener for the objects
function MessageListener(source, symbol, list) {
    for (const data of Pd4Web.GuiReceivers[source]) {
        switch (data.type) {
            case "bng":
                switch (symbol) {
                    case "size":
                        data.size = list[0] || 8;
                        ConfigureItem(data.rect, GuiBngRect(data));
                        ConfigureItem(data.circle, GuiBngCircle(data));
                        break;
                    case "flashtime":
                        data.interrupt = list[0] || 10;
                        data.hold = list[1] || 50;
                        break;
                    case "init":
                        data.init = list[0];
                        break;
                    case "send":
                        data.send = list[0];
                        break;
                    case "receive":
                        UnbindGuiReceiver(data);
                        data.receive = list[0];
                        BindGuiReceiver(data);
                        break;
                    case "label":
                        data.label = list[0] === "empty" ? "" : list[0];
                        data.text.textContent = data.label;
                        break;
                    case "label_pos":
                        data.x_off = list[0];
                        data.y_off = list[1] || 0;
                        ConfigureItem(data.text, GuiBngText(data));
                        break;
                    case "label_font":
                        data.font = list[0];
                        data.fontsize = list[1] || 0;
                        ConfigureItem(data.text, GuiBngText(data));
                        break;
                    case "color":
                        data.bg_color = list[0];
                        data.fg_color = list[1] || 0;
                        data.label_color = list[2] || 0;
                        ConfigureItem(data.rect, GuiBngRect(data));
                        ConfigureItem(data.text, GuiBngText(data));
                        break;
                    case "pos":
                        data.x_pos = list[0];
                        data.y_pos = list[1] || 0;
                        ConfigureItem(data.rect, GuiBngRect(data));
                        ConfigureItem(data.circle, GuiBngCircle(data));
                        ConfigureItem(data.text, GuiBngText(data));
                        break;
                    case "delta":
                        data.x_pos += list[0];
                        data.y_pos += list[1] || 0;
                        ConfigureItem(data.rect, GuiBngRect(data));
                        ConfigureItem(data.circle, GuiBngCircle(data));
                        ConfigureItem(data.text, GuiBngText(data));
                        break;
                    default:
                        GuiBngUpdateCircle(data);
                }
                break;
            case "tgl":
                switch (symbol) {
                    case "size":
                        data.size = list[0] || 8;
                        ConfigureItem(data.rect, GuiTglRect(data));
                        ConfigureItem(data.cross1, GuiTglCross1(data));
                        ConfigureItem(data.cross2, GuiTglCross2(data));
                        break;
                    case "nonzero":
                        data.default_value = list[0];
                        break;
                    case "init":
                        data.init = list[0];
                        break;
                    case "send":
                        data.send = list[0];
                        break;
                    case "receive":
                        UnbindGuiReceiver(data);
                        data.receive = list[0];
                        BindGuiReceiver(data);
                        break;
                    case "label":
                        data.label = list[0] === "empty" ? "" : list[0];
                        data.text.textContent = data.label;
                        break;
                    case "label_pos":
                        data.x_off = list[0];
                        data.y_off = list[1] || 0;
                        ConfigureItem(data.text, GuiTglLabel(data));
                        break;
                    case "label_font":
                        data.font = list[0];
                        data.fontsize = list[1] || 0;
                        ConfigureItem(data.text, GuiTglLabel(data));
                        break;
                    case "color":
                        data.bg_color = list[0];
                        data.fg_color = list[1] || 0;
                        data.label_color = list[2] || 0;
                        ConfigureItem(data.rect, GuiTglRect(data));
                        ConfigureItem(data.cross1, GuiTglCross1(data));
                        ConfigureItem(data.cross2, GuiTglCross2(data));
                        ConfigureItem(data.text, GuiTglLabel(data));
                        break;
                    case "pos":
                        data.x_pos = list[0];
                        data.y_pos = list[1] || 0;
                        ConfigureItem(data.rect, GuiTglRect(data));
                        ConfigureItem(data.cross1, GuiTglCross1(data));
                        ConfigureItem(data.cross2, GuiTglCross2(data));
                        ConfigureItem(data.text, GuiTglLabel(data));
                        break;
                    case "delta":
                        data.x_pos += list[0];
                        data.y_pos += list[1] || 0;
                        ConfigureItem(data.rect, GuiTglRect(data));
                        ConfigureItem(data.cross1, GuiTglCross1(data));
                        ConfigureItem(data.cross2, GuiTglCross2(data));
                        ConfigureItem(data.text, GuiTglLabel(data));
                        break;
                    case "set":
                        data.default_value = list[0];
                        data.value = data.default_value;
                        GuiTglUpdateCross(data);
                        break;
                }
                break;
            case "vsl":
            case "hsl":
                switch (symbol) {
                    case "size":
                        if (list.length === 1) {
                            data.width = list[0] || 8;
                        } else {
                            data.width = list[0] || 8;
                            data.height = list[1] || 2;
                        }
                        ConfigureItem(data.rect, gui_slider_rect(data));
                        ConfigureItem(data.indicator, gui_slider_indicator(data));
                        GuiSliderCheckMinmax(data);
                        break;
                    case "range":
                        data.bottom = list[0];
                        data.top = list[1] || 0;
                        GuiSliderCheckMinmax(data);
                        break;
                    case "lin":
                        data.log = 0;
                        GuiSliderCheckMinmax(data);
                        break;
                    case "log":
                        data.log = 1;
                        GuiSliderCheckMinmax(data);
                        break;
                    case "init":
                        data.init = list[0];
                        break;
                    case "steady":
                        data.steady_on_click = list[0];
                        break;
                    case "send":
                        data.send = list[0];
                        break;
                    case "receive":
                        UnbindGuiReceiver(data);
                        data.receive = list[0];
                        BindGuiReceiver(data);
                        break;
                    case "label":
                        data.label = list[0] === "empty" ? "" : list[0];
                        data.text.textContent = data.label;
                        break;
                    case "label_pos":
                        data.x_off = list[0];
                        data.y_off = list[1] || 0;
                        ConfigureItem(data.text, gui_slider_text(data));
                        break;
                    case "label_font":
                        data.font = list[0];
                        data.fontsize = list[1] || 0;
                        ConfigureItem(data.text, gui_slider_text(data));
                        break;
                    case "color":
                        data.bg_color = list[0];
                        data.fg_color = list[1] || 0;
                        data.label_color = list[2] || 0;
                        ConfigureItem(data.rect, gui_slider_rect(data));
                        ConfigureItem(data.indicator, gui_slider_indicator(data));
                        ConfigureItem(data.text, gui_slider_text(data));
                        break;
                    case "pos":
                        data.x_pos = list[0];
                        data.y_pos = list[1] || 0;
                        ConfigureItem(data.rect, gui_slider_rect(data));
                        ConfigureItem(data.indicator, gui_slider_indicator(data));
                        ConfigureItem(data.text, gui_slider_text(data));
                        break;
                    case "delta":
                        data.x_pos += list[0];
                        data.y_pos += list[1] || 0;
                        ConfigureItem(data.rect, gui_slider_rect(data));
                        ConfigureItem(data.indicator, gui_slider_indicator(data));
                        ConfigureItem(data.text, gui_slider_text(data));
                        break;
                    case "set":
                        GuiSliderSet(data, list[0]);
                        break;
                }
                break;
            case "vradio":
            case "hradio":
                switch (symbol) {
                    case "size":
                        data.size = list[0] || 8;
                        ConfigureItem(data.rect, GuiRadioRect(data));
                        GuiRadioUpdateLinesButtons(data);
                        break;
                    case "init":
                        data.init = list[0];
                        break;
                    case "number":
                        const n = Math.min(Math.max(Math.floor(list[0]), 1), 128);
                        if (n !== data.number) {
                            data.number = n;
                            if (data.value >= data.number) {
                                data.value = data.number - 1;
                            }
                            ConfigureItem(data.rect, GuiRadioRect(data));
                            GuiRadioRemoveLinesButtons(data);
                            GuiRadioCreateLinesButtons(data);
                        }
                        break;
                    case "send":
                        data.send = list[0];
                        break;
                    case "receive":
                        UnbindGuiReceiver(data);
                        data.receive = list[0];
                        BindGuiReceiver(data);
                        break;
                    case "label":
                        data.label = list[0] === "empty" ? "" : list[0];
                        data.text.textContent = data.label;
                        break;
                    case "label_pos":
                        data.x_off = list[0];
                        data.y_off = list[1] || 0;
                        ConfigureItem(data.text, GuiRadioText(data));
                        break;
                    case "label_font":
                        data.font = list[0];
                        data.fontsize = list[1] || 0;
                        ConfigureItem(data.text, GuiRadioText(data));
                        break;
                    case "color":
                        data.bg_color = list[0];
                        data.fg_color = list[1] || 0;
                        data.label_color = list[2] || 0;
                        ConfigureItem(data.rect, GuiRadioRect(data));
                        GuiRadioUpdateLinesButtons(data);
                        ConfigureItem(data.text, GuiRadioText(data));
                        break;
                    case "pos":
                        data.x_pos = list[0];
                        data.y_pos = list[1] || 0;
                        ConfigureItem(data.rect, GuiRadioRect(data));
                        GuiRadioUpdateLinesButtons(data);
                        ConfigureItem(data.text, GuiRadioText(data));
                        break;
                    case "delta":
                        data.x_pos += list[0];
                        data.y_pos += list[1] || 0;
                        ConfigureItem(data.rect, GuiRadioRect(data));
                        GuiRadioUpdateLinesButtons(data);
                        ConfigureItem(data.text, GuiRadioText(data));
                        break;
                    case "set":
                        data.value = Math.min(Math.max(Math.floor(list[0]), 0), data.number - 1);
                        GuiRadioUpdateButton(data);
                        break;
                }
                break;
            case "cnv":
                switch (symbol) {
                    case "size":
                        data.size = list[0] || 1;
                        ConfigureItem(data.selectable_rect, GuiCnvSelectableRect(data));
                        break;
                    case "vis_size":
                        if (list.length === 1) {
                            data.width = list[0] || 1;
                            data.height = data.width;
                        } else {
                            data.width = list[0] || 1;
                            data.height = list[1] || 1;
                        }
                        ConfigureItem(data.visible_rect, GuiCnvVisibleRect(data));
                        break;
                    case "send":
                        data.send = list[0];
                        break;
                    case "receive":
                        UnbindGuiReceiver(data);
                        data.receive = list[0];
                        BindGuiReceiver(data);
                        break;
                    case "label":
                        data.label = list[0] === "empty" ? "" : list[0];
                        data.text.textContent = data.label;
                        break;
                    case "label_pos":
                        data.x_off = list[0];
                        data.y_off = list[1] || 0;
                        ConfigureItem(data.text, GuiCnvText(data));
                        break;
                    case "label_font":
                        data.font = list[0];
                        data.fontsize = list[1] || 0;
                        ConfigureItem(data.text, GuiCnvText(data));
                        break;
                    case "get_pos":
                        break;
                    case "color":
                        data.bg_color = list[0];
                        data.label_color = list[1] || 0;
                        ConfigureItem(data.visible_rect, GuiCnvVisibleRect(data));
                        ConfigureItem(data.selectable_rect, GuiCnvSelectableRect(data));
                        ConfigureItem(data.text, GuiCnvText(data));
                        break;
                    case "pos":
                        data.x_pos = list[0];
                        data.y_pos = list[1] || 0;
                        ConfigureItem(data.visible_rect, GuiCnvVisibleRect(data));
                        ConfigureItem(data.selectable_rect, GuiCnvSelectableRect(data));
                        ConfigureItem(data.text, GuiCnvText(data));
                        break;
                    case "delta":
                        data.x_pos += list[0];
                        data.y_pos += list[1] || 0;
                        ConfigureItem(data.visible_rect, GuiCnvVisibleRect(data));
                        ConfigureItem(data.selectable_rect, GuiCnvSelectableRect(data));
                        ConfigureItem(data.text, GuiCnvText(data));
                        break;
                }
                break;
        }
    }
}

//╭─────────────────────────────────────╮
//│            Gui Handling             │
//╰─────────────────────────────────────╯
function CreateItem(type, args) {
    var item = document.createElementNS("http://www.w3.org/2000/svg", type);
    if (args !== null) {
        ConfigureItem(item, args);
    }
    var canvas = document.getElementById("Pd4WebCanvas");
    canvas.appendChild(item);
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
        Pd4Web.addGuiReceiver(data.receive);
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
    let rect = {
        x: data.x_pos,
        y: data.y_pos,
        rx: 2,
        ry: 2,
        width: data.size,
        height: data.size,
        id: `${data.id}_rect`,
        class: "border clickable",
    };
    if (!Pd4Web.AutoTheme) {
        rect.fill = ColFromLoad(data.bg_color);
    }
    return rect;
}

// ─────────────────────────────────────
function GuiText(data) {
    let x = data.x_pos + data.x_off;
    let y = data.y_pos + data.y_off;
    let text = {
        x: x,
        y: y,
        "font-family": IEMFontFamily(data.font),
        "font-weight": "normal",
        "font-size": `${data.fontsize}px`,
        transform: `translate(0, ${(data.fontsize / 2) * 0.6})`, // note: modified
        id: `${data.id}_text`,
        class: "unclickable",
    };
    if (!Pd4Web.AutoTheme) {
        text.fill = ColFromLoad(data.label_color);
    }
    return text;
}

// ─────────────────────────────────────
function GuiMousePoint(e) {
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
    let rect = {
        x: data.x_pos,
        y: data.y_pos,
        rx: 2,
        ry: 2,
        width: data.size,
        height: data.size,
        id: `${data.id}_rect`,
        class: "border bng-rect clickable",
    };
    if (!Pd4Web.AutoTheme) {
        rect.fill = ColFromLoad(data.bg_color);
    }
    return rect;
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
        fill: "transparent",
        id: `${data.id}_circle`,
        class: "unclickable border",
    };
}

// ─────────────────────────────────────
function GuiBngText(data) {
    return GuiText(data);
}

// ─────────────────────────────────────
function GuiBngUpdateCircle(data) {
    let color;
    if (Pd4Web.AutoTheme) {
        color = ColFromLoad(getCssVariable("--bng-circle"));
    } else {
        color = data.fg_color;
    }

    if (data.flashed) {
        data.flashed = false;
        ConfigureItem(data.circle, {
            fill: color,
        });
        if (data.interrupt_timer) {
            clearTimeout(data.interrupt_timer);
        }
        data.interrupt_timer = setTimeout(function () {
            data.interrupt_timer = null;
            ConfigureItem(data.circle, {
                fill: color,
            });
        }, data.interrupt);
        data.flashed = true;
    } else {
        data.flashed = true;
        ConfigureItem(data.circle, {
            fill: color,
        });
    }

    //

    if (data.hold_timer) {
        clearTimeout(data.hold_timer);
    }
    data.hold_timer = setTimeout(function () {
        data.flashed = false;
        data.hold_timer = null;
        ConfigureItem(data.circle, {
            fill: "transparent",
        });
    }, data.hold);
}

// ─────────────────────────────────────
function GuiBngOnMouseDown(data) {
    GuiBngUpdateCircle(data);
    Pd4Web.sendBang(data.send);
}

// ─────────────────────────────────────
function GuiBngSetup(args, id) {
    const data = {};
    data.x_pos = parseInt(args[2]) - Pd4Web.x_pos;
    data.y_pos = parseInt(args[3]) - Pd4Web.y_pos;
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
    data.id = `${data.type}_${id}`;

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

//╭─────────────────────────────────────╮
//│             Toggle: Tgl             │
//╰─────────────────────────────────────╯
function GuiTglRect(data) {
    let rect = {
        x: data.x_pos,
        y: data.y_pos,
        rx: 2,
        ry: 2,
        width: data.size,
        height: data.size,
        id: `${data.id}_rect`,
        class: "border tgl-rect clickable",
    };
    if (!Pd4Web.AutoTheme) {
        rect.fill = ColFromLoad(data.bg_color);
    }
    return rect;
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
    let cross1 = {
        points: points,
        "stroke-width": w,
        stroke: getCssVariable("--fg2"),
        id: `${data.id}_cross1`,
        class: "unclickable tgl-cross",
    };
    if (!Pd4Web.AutoTheme) {
        //cross1.fill = "none";
    }
    return cross1;
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
    let cross2 = {
        points: points,
        "stroke-width": w,
        stroke: getCssVariable("--fg2"),
        id: `${data.id}_cross1`,
        class: "unclickable tgl-cross",
    };
    if (!Pd4Web.AutoTheme) {
        cross2.fill = ColFromLoad(data.fg_color);
    }
    return cross2;
}

// ─────────────────────────────────────
function GuiTglLabel(data) {
    return GuiText(data);
}

// ─────────────────────────────────────
function GuiTglUpdateCross(data) {
    let colorOn;
    let colorOff = getCssVariable("--fg2");
    if (!Pd4Web.AutoTheme) {
        colorOn = ColFromLoad(data.fg_color);
    } else {
        colorOn = getCssVariable("--tgl-cross");
    }

    ConfigureItem(data.cross1, {
        stroke: data.value ? colorOn : colorOff,
    });
    ConfigureItem(data.cross2, {
        stroke: data.value ? colorOn : colorOff,
    });
}

// ─────────────────────────────────────
function GuiTglOnMouseDown(data) {
    data.value = data.value ? 0 : data.default_value;
    GuiTglUpdateCross(data);
    Pd4Web.sendFloat(data.send, data.value);
}

// ─────────────────────────────────────
function GuiTglSetup(args, id) {
    const data = {};
    data.x_pos = parseInt(args[2]) - Pd4Web.x_pos;
    data.y_pos = parseInt(args[3]) - Pd4Web.y_pos;
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
    data.id = `${data.type}_${id}`;

    // create svg
    data.rect = CreateItem("rect", GuiTglRect(data));
    data.cross1 = CreateItem("polyline", GuiTglCross1(data));
    data.cross2 = CreateItem("polyline", GuiTglCross2(data));
    data.text = CreateItem("text", GuiTglLabel(data));
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

//╭─────────────────────────────────────╮
//│             Number: Nbx             │
//╰─────────────────────────────────────╯
function GuiNbxUpdateNumber(data, f) {
    const txt = document.getElementById(`${data.id}_text`); // Find the associated text element
    var text = f.toString();
    var isFloat = text.includes(".");
    if (text.length >= data.width) {
        if (isFloat) {
            text = text.slice(0, data.width);
            txt.textContent = text;
        } else {
            text = text.slice(0, data.width - 1);
            text += ">";
            txt.textContent = text;
        }
    } else {
        txt.textContent = text;
    }
}

// ─────────────────────────────────────
function GuiNbxKeyDownListener(e) {
    const data = Pd4Web.NbxSelected; // Get the currently selected SVG element
    const txt = document.getElementById(`${data.id}_text`); // Find the associated text element
    const key = e.key; // Get the key that was pressed

    // TODO: Need to implement e numbers
    // check if key is between 0-9 or . or + and i
    if (key >= "0" && key <= "9") {
    } else if (key === ".") {
    } else if ((key === "+") | (key === "-")) {
    } else if (key == "Enter") {
        let textColor;
        if (Pd4Web.AutoTheme) {
            textColor = getCssVariable("--nbx-text");
        } else {
            textColor = ColFromLoad(data.label_color);
        }
        txt.style.fill = textColor;
        txt.clicked = false;
        const svgElement = document.getElementById("Pd4WebCanvas");
        svgElement.removeAttribute("tabindex"); // Remove tabindex
        Pd4Web.NbxSelected = null;
        svgElement.removeEventListener("keypress", GuiNbxKeyDownListener);
        if (txt.numberContent.length > data.width) {
            txt.textContent = "+";
        } else {
            txt.textContent = txt.numberContent;
        }
        Pd4Web.sendFloat(data.send, parseFloat(txt.numberContent));
        return;
    } else {
        return;
    }

    if (txt) {
        if (data.inputCnt == 0) {
            txt.textContent = key; // Update the text content of the SVG text element
            txt.numberContent = key;
        } else {
            var text = txt.textContent + key;
            txt.numberContent += key;
            if (text.length >= data.width) {
                // remove olds > and add new one
                for (var i = 0; i < data.width; i++) {
                    if (text[i] == ">") {
                        text = text.slice(0, i) + text.slice(i + 1);
                        break;
                    }
                }
                text = text.slice(-data.width + 1) + ">";
                txt.textContent = text;
            } else {
                txt.textContent = txt.textContent + key;
            }
        }
        data.inputCnt++;
    } else {
        console.error(`Text element with id ${data.id}_text not found.`);
    }
}

// ─────────────────────────────────────
function GuiNbxRect(data) {
    return {
        x: data.x_pos,
        y: data.y_pos,
        width: data.width * data.fontsize,
        height: data.height,
        rx: 2,
        ry: 2,
        id: `${data.id}_rect`,
        fill: ColFromLoad(data.bg_color),
        "stroke-width": "1px",
        class: "border clickable nbx",
    };
}

// ─────────────────────────────────────
// Draw the small triangle (for up/down control)
function GuiNbxTriangle(data) {
    const height = data.height;
    const tri_size = height * 0.3; // Size of the triangle

    const gap = 1.5; // Gap between the triangle and the
    const x1 = data.x_pos + gap; // Adjust the triangle position to the left of the box
    const y1 = data.y_pos + gap;

    const x2 = x1; // Same x for the bottom left corner of the triangle
    const y2 = data.y_pos + data.height - gap;

    const x3 = x1 + tri_size;
    const y3 = data.y_pos + data.height / 2;

    return {
        points: `${x1},${y1} ${x3},${y3} ${x2},${y2}, ${x3},${y3}`,
        "stroke-width": "1px",
        id: `${data.id}_triangle`,
        class: "unclickable border",
    };
}

// ─────────────────────────────────────
// Draw the number text inside the nbx
function GuiNbxText(data) {
    const start_text = data.height * 0.5; // Size of the triangle
    let textColor;
    if (Pd4Web.AutoTheme) {
        textColor = Pd4Web.Style.Text;
    } else {
        textColor = ColFromLoad(data.label_color);
    }

    return {
        x: data.x_pos + start_text, // Adjust the x position to center the text in the box
        y: data.y_pos + data.height / 2 + data.fontsize * 0.4, // Center the text vertically
        "font-family": IEMFontFamily(data.font), // Use the specified font family
        "font-weight": "bold", // Use bold text to match Pd number box style
        "font-size": `${data.fontsize}px`, // Set font size
        fill: textColor, // Set the color from data
        "text-anchor": "left", // Center the text horizontally
        id: `${data.id}_text`,
        // set text using data.label.
        text: data.label,
        clicked: false,
        class: "unclickable nbx-text",
    };
}

// ─────────────────────────────────────
function GuiNbxLabel(data) {
    return GuiText(data);
}

// ─────────────────────────────────────
function GuiNbxSetup(args, id) {
    const data = {};
    data.x_pos = parseInt(args[2]) - Pd4Web.x_pos;
    data.y_pos = parseInt(args[3]) - Pd4Web.y_pos;
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
    data.id = `${data.type}_${id}`;

    data.rect = CreateItem("rect", GuiNbxRect(data));
    data.triangle = CreateItem("polygon", GuiNbxTriangle(data));
    data.numbers = CreateItem("text", GuiNbxText(data));
    data.numbers.textContent = data.init;

    data.drawlabel = CreateItem("text", GuiNbxLabel(data));
    data.drawlabel.textContent = data.label;

    if (Pd4Web.isMobile) {
        data.rect.addEventListener("touchstart", function (e) {
            for (const _ of e.changedTouches) {
                // Call your function here
                // gui_slider_onmousedown(data, touch, touch.identifier);
            }
        });
    } else {
        data.rect.addEventListener("click", function (_) {
            const id = data.id + "_text";
            const txt = document.getElementById(id);
            if (typeof txt.numberContent === "undefined") {
                txt.numberContent = txt.textContent;
            }
            if (txt.clicked) {
                let textColor;
                if (Pd4Web.AutoTheme) {
                    textColor = getCssVariable("--nbx-text-selected");
                } else {
                    textColor = ColFromLoad(data.label_color);
                }
                txt.style.fill = textColor; // Change fill color to black
                txt.clicked = false;
                const svgElement = document.getElementById("Pd4WebCanvas");
                svgElement.removeAttribute("tabindex"); // Remove tabindex
                Pd4Web.NbxSelected = null;
                svgElement.removeEventListener("keypress", GuiNbxKeyDownListener);
                if (txt.numberContent.length > data.width) {
                    txt.textContent = "+";
                } else {
                    txt.textContent = txt.numberContent;
                }
                Pd4Web.sendFloat(data.send, parseFloat(txt.numberContent));
            } else {
                txt.style.fill = getCssVariable("--nbx-selected"); // Change fill color to black
                txt.clicked = true;
                const svgElement = document.getElementById("Pd4WebCanvas");
                svgElement.setAttribute("tabindex", "0"); // "0" makes it focusable
                svgElement.focus();
                data.inputCnt = 0;
                Pd4Web.NbxSelected = data;
                svgElement.addEventListener("keypress", GuiNbxKeyDownListener);
            }
        });
    }
    BindGuiReceiver(data);
}

//╭─────────────────────────────────────╮
//│           Slider: vsl/hsl           │
//╰─────────────────────────────────────╯
function GuiSliderRect(data) {
    let x = data.x_pos;
    let y = data.y_pos;
    let color = ColFromLoad(data.bg_color);
    let width = data.width;
    let height = data.height;
    if (Pd4Web.AutoTheme) {
        return {
            x: x,
            y: y,
            rx: 2,
            ry: 2,
            width: width,
            height: height,
            id: `${data.id}_rect`,
            class: "border clickable slider-fill",
        };
    } else {
        return {
            x: x,
            y: y,
            rx: 2,
            ry: 2,
            width: width,
            height: height,
            //fill: color,
            id: `${data.id}_rect`,
            class: "border clickable",
        };
    }
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
        p1 = x1 + 2;
        p2 = r;
        p3 = x2 - 2;
        p4 = r;
    } else {
        r = x1 + 3 + (data.value + 50) / 100;
        r = Math.max(x1 + 3, Math.min(r, x2 - 3));
        p1 = r;
        p2 = y1 + 2;
        p3 = r;
        p4 = y2 - 2;
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
    let rgb = GetRBG(data.fg_color);

    if (!Pd4Web.AutoTheme) {
        return {
            x1: p.x1,
            y1: p.y1,
            x2: p.x2,
            y2: p.y2,
            stroke: ColFromLoad(data.fg_color),
            "stroke-linecap": "round",
            fill: "none",
            id: `${data.id}_indicator`,
            class: "unclickable slider-indicator",
        };
    } else {
        let almostBlack = rgb.r < 20 && rgb.g < 20 && rgb.b < 20;
        let almostWhite = rgb.r > 235 && rgb.g > 235 && rgb.b > 235;
        if (almostBlack || almostWhite) {
            return {
                x1: p.x1,
                y1: p.y1,
                x2: p.x2,
                y2: p.y2,
                rx: 2,
                ry: 2,
                "stroke-linecap": "round",
                stroke: ColFromLoad(data.fg_color),
                fill: "none",
                id: `${data.id}_indicator`,
                class: "unclickable slider-indicator",
            };
        }
        return {
            x1: p.x1,
            y1: p.y1,
            x2: p.x2,
            y2: p.y2,
            rx: 2,
            ry: 2,
            stroke: ColFromLoad(data.fg_color),
            "stroke-width": 3,
            "stroke-linecap": "round",
            fill: "none",
            id: `${data.id}_indicator`,
            class: "unclickable",
        };
    }
}

// ─────────────────────────────────────
function GuiSliderText(data) {
    return GuiText(data);
}

// ─────────────────────────────────────
function GuiSliderUpdateIndicatorRect(data) {
    const p = GuiSliderIndicatorPoints(data);
    ConfigureItem(data.indicator, {
        x1: p.x1,
        y1: p.y1,
        x2: p.x2,
        y2: p.y2,
    });
}

// ─────────────────────────────────────
function GuiSliderUpdateIndicator(data) {
    const p = GuiSliderIndicatorPoints(data);
    ConfigureItem(data.indicator, {
        x1: p.x1,
        y1: p.y1,
        x2: p.x2,
        y2: p.y2,
        class: "unclickable slider-indicator",
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
        Pd4Web.sendFloat(data.send, out);
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
        if (Pd4Web.isMobile) {
            document.body.style.overflow = "hidden";
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

    if (Pd4Web.isMobile) {
        document.body.style.overflow = "auto";
    }
}

// ─────────────────────────────────────
function GuiSliderSetup(args, id) {
    const data = {};
    data.type = args[4];

    if (data.type == "hsl") {
        data.x_pos = parseInt(args[2]) - 3 - Pd4Web.x_pos;
    } else {
        data.x_pos = parseInt(args[2]) - Pd4Web.x_pos;
    }

    if (data.type == "vsl") {
        data.y_pos = parseInt(args[3]) - 2 - Pd4Web.y_pos;
    } else {
        data.y_pos = parseInt(args[3]) - Pd4Web.y_pos;
    }
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
    data.id = `${data.type}_${id}`;

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
    let radio = {
        x: data.x_pos,
        y: data.y_pos,
        rx: 2,
        ry: 2,
        width: width,
        height: height,
        id: `${data.id}_rect`,
        class: "border clickable radio",
    };
    return radio;
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
        rx: 0.5,
        ry: 0.5,
        width: p3 - p1,
        height: p4 - p2,
        fill: ColFromLoad(data.fg_color),
        stroke: ColFromLoad(data.fg_color),
        display: state ? "inline" : "none",
        id: `${data.id}_button_${button_index}`,
        class: "radio-buttom unclickable",
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
    Pd4Web.sendFloat(data.receive, data.value);
}

// ─────────────────────────────────────
function GuiRadioSetup(args, id) {
    const data = {};
    data.x_pos = parseInt(args[2]) - Pd4Web.x_pos;
    data.y_pos = parseInt(args[3]) - Pd4Web.y_pos;
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
    data.id = `${data.type}_${id}`;

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

//╭─────────────────────────────────────╮
//│             Vu: VuRect              │
//╰─────────────────────────────────────╯
function GuiVuRect(data) {
    return {
        x: data.x_pos,
        y: data.y_pos,
        rx: 2,
        ry: 2,
        width: data.width,
        height: data.height,
        fill: Pd4Web.AutoTheme ? "transparent" : ColFromLoad(data.bg_color),
        id: `${data.id}_rect`,
        class: "border unclickable",
    };
}

// ─────────────────────────────────────
function GuiVudBRects(data) {
    const colors = ["#f430f0", "#fc2828", "#e8e828", "#ff8001", "#00ffff"];
    const getColor = (i) =>
        i === 39
            ? colors[4]
            : i >= 11 && i <= 38
              ? colors[3]
              : i >= 14 && i <= 22
                ? colors[2]
                : i >= 1 && i <= 10
                  ? colors[1]
                  : colors[0];

    const miniWidth = data.width - 3,
        miniHeight = (data.height - 2) / 40;
    data.mini_rects = Array.from({ length: 40 }, (_, i) => {
        const rect = CreateItem("rect", {
            x: data.x_pos + 1.5,
            y: data.y_pos + 1 + i * miniHeight,
            width: miniWidth,
            height: miniHeight - 0.1,
            fill: "transparent",
            active: getColor(i),
            id: `${data.id}_mini_rect_${i}`,
            class: "unclickable vu-mini-rect",
            rx: 0.5,
            ry: 0.5,
        });
        return rect;
    });
}

// ─────────────────────────────────────
function GuiVuUpdateGain(data) {
    const thresholds = [
        -99, -100, -80, -60, -55, -50, -45, -40, -35, -30, -27, -25, -22, -20, -18, -16, -14, -12, -10, -9, -7, -6, -5,
        -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 11,
    ];
    const amount = thresholds.findIndex((t) => data.value <= t) || 40;
    if (data.value < -100) {
        return;
    }

    data.mini_rects.forEach((rect, i) => {
        rect.style.fill = 41 - i < amount ? rect.getAttribute("active") : getCssVariable("--vu-active");
    });
}

// ─────────────────────────────────────
function GuiVuSetup(args, id) {
    const data = {
        x_pos: parseInt(args[2]) - Pd4Web.x_pos,
        y_pos: parseInt(args[3]) - Pd4Web.y_pos,
        type: args[4],
        width: args[5],
        height: parseInt(args[6]),
        receive: args[7],
        id: `${args[4]}_${id}`,
    };
    data.rect = CreateItem("rect", GuiVuRect(data));
    GuiVudBRects(data);
    BindGuiReceiver(data);
}

//╭─────────────────────────────────────╮
//│             Canvas: Cnv             │
//╰─────────────────────────────────────╯
function GuiCnvVisibleRect(data) {
    return {
        x: data.x_pos,
        y: data.y_pos,
        rx: 2,
        ry: 2,
        width: data.width,
        height: data.height,
        fill: ColFromLoad(data.bg_color),
        // stroke: ColFromLoad(data.bg_color),
        id: `${data.id}_visible_rect`,
        class: "unclickable",
    };
}

// ─────────────────────────────────────
function GuiCnvSelectableRect(data) {
    return {
        x: data.x_pos,
        y: data.y_pos,
        rx: 2,
        ry: 2,
        width: data.size,
        height: data.size,
        // fill: "none",
        fill: ColFromLoad(data.bg_color),
        id: `${data.id}_selectable_rect`,
        class: "unclickable",
    };
}

// ─────────────────────────────────────
function GuiCnvText(data) {
    return GuiText(data);
}

// ─────────────────────────────────────
function GuiCnvSetup(args, id) {
    const data = {};
    if (Pd4Web.x_pos === undefined && Pd4Web.y_pos === undefined) {
        data.x_pos = parseInt(args[2]);
        data.y_pos = parseInt(args[3]);
    } else {
        data.x_pos = parseInt(args[2]) - Pd4Web.x_pos;
        data.y_pos = parseInt(args[3]) - Pd4Web.y_pos;
    }

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
    data.id = `${data.type}_${id}`;

    // create svg
    data.visible_rect = CreateItem("rect", GuiCnvVisibleRect(data));
    data.selectable_rect = CreateItem("rect", GuiCnvSelectableRect(data));
    data.text = CreateItem("text", GuiCnvText(data));
    data.text.textContent = data.label;

    // subscribe receiver
    BindGuiReceiver(data);
}

//╭─────────────────────────────────────╮
//│           else/knob Knob            │
//╰─────────────────────────────────────╯
function GuiKnobRect(data) {
    return {
        x: data.x_pos,
        y: data.y_pos,
        rx: 2,
        ry: 2,
        width: data.size,
        height: data.size,
        id: `${data.id}_rect`,
        class: "border clickable knob",
    };
}

// ─────────────────────────────────────
function GuiKnobCircleCenter(data) {
    const r = (data.size - 2) / 2;
    const cx = data.x_pos + r + 1;
    const cy = data.y_pos + r + 1;
    return {
        cx: cx,
        cy: cy,
        r: data.size / 50,
        //fill: "black",
        //stroke: "black",
        "stroke-width": 0.5,
        id: `${data.id}_knob_center`,
        class: "unclickable",
    };
}

// ─────────────────────────────────────
function GuiKnobArc(data) {
    const r = ((data.size - 2) / 2) * (data.radius * 1.3); // Radius scaled down by 0.9
    const cx = data.x_pos + (data.size - 2) / 2 + 1;
    const cy = data.y_pos + (data.size - 2) / 2 + 1;

    // Angle in degrees (0 to 360)
    const angle = data.ag_range;

    // Calculate the half angle to center the arc around 12 o'clock
    const halfAngle = Math.PI * (angle / 360);
    const startAngle = -Math.PI / 2 - halfAngle; // Start point adjusted based on angle
    const endAngle = -Math.PI / 2 + halfAngle; // End point adjusted to be symmetric

    // Calculate the coordinates for the arc's endpoints
    const x1 = cx + r * Math.cos(startAngle);
    const y1 = cy + r * Math.sin(startAngle);
    const x2 = cx + r * Math.cos(endAngle);
    const y2 = cy + r * Math.sin(endAngle);

    // Large arc flag: 1 if angle is greater than 180 degrees, else 0
    const largeArcFlag = angle > 180 ? 1 : 0;

    // SVG path for the arc
    const d = `M ${x1} ${y1} A ${r} ${r} 0 ${largeArcFlag} 1 ${x2} ${y2}`;

    return {
        d: d,
        fill: "transparent",
        "stroke-width": 1,
        "stroke-linecap": "round",
        id: `${data.id}_knob_arc`,
        class: "unclickable border",
    };
}

// ─────────────────────────────────────
function GuiKnobPointer(data) {
    var r = (data.size - 2) / 2;
    data.cx = data.x_pos + r + 1;
    data.cy = data.y_pos + r + 1;
    r = r * data.radius; // Adjust pointer length relative to knob size

    const halfAngle = Math.PI * (data.ag_range / 360);
    const startAngle = -Math.PI / 2 - halfAngle; // Start point adjusted based on angle
    const x1 = data.cx + r * Math.cos(startAngle);
    const y1 = data.cy + r * Math.sin(startAngle);

    return {
        x1: x1,
        y1: y1,
        x2: data.cx,
        y2: data.cy,
        stroke: "white",
        "stroke-width": 2,
        "stroke-linecap": "round",
        class: "unclickable knob-pointer",
    };
}
// ─────────────────────────────────────
function GuiGetTickPosition(data, i) {
    const cx = data.x_pos + (data.size - 2) / 2 + 1;
    const cy = data.y_pos + (data.size - 2) / 2 + 1;
    const tickRadius = ((data.size - 2) / 2) * data.radius; // Slightly smaller than pointer radius

    // Add tick marks
    const halfAngle = Math.PI * (data.ag_range / 360);
    const startAngle = -Math.PI / 2 - halfAngle;
    const endAngle = -Math.PI / 2 + halfAngle;
    const tickStep = data.ticks > 1 ? (endAngle - startAngle) / (data.ticks - 1) : 0;
    const tickAngle = startAngle + i * tickStep;

    // Calculate tick position
    const x1 = cx + tickRadius * Math.cos(tickAngle);
    const y1 = cy + tickRadius * Math.sin(tickAngle);
    return { x1, y1 };
}

// ─────────────────────────────────────
function GuiKnobTicks(data) {
    // Calculate center position and radius for ticks
    const cx = data.x_pos + (data.size - 2) / 2 + 1;
    const cy = data.y_pos + (data.size - 2) / 2 + 1;
    const tickRadius = ((data.size - 2) / 2) * data.radius * 1.5; // Slightly smaller than pointer radius

    // Add tick marks
    const halfAngle = Math.PI * (data.ag_range / 360);
    const startAngle = -Math.PI / 2 - halfAngle;
    const endAngle = -Math.PI / 2 + halfAngle;
    const tickStep = data.ticks > 1 ? (endAngle - startAngle) / (data.ticks - 1) : 0;

    for (let i = 0; i < data.ticks; i++) {
        const tickAngle = startAngle + i * tickStep;

        // Calculate tick position
        const x1 = cx + tickRadius * Math.cos(tickAngle);
        const y1 = cy + tickRadius * Math.sin(tickAngle);
        const x2 = cx + (tickRadius - 1) * Math.cos(tickAngle); // Shorter inner line
        const y2 = cy + (tickRadius - 1) * Math.sin(tickAngle);

        // Create the tick line
        const tick = CreateItem("line", {
            x1: x1,
            y1: y1,
            x2: x2,
            y2: y2,
            //stroke: getCssVariable("--knob-tick"),
            "stroke-linecap": "round",
            "stroke-width": 1.5,
            class: "unclickable border",
        });

        // Append tick to the knob
        data.rect.parentNode.appendChild(tick);
    }
}

// ─────────────────────────────────────
function GuiKnobOnMouseDown(data, e, n) {
    data.beingDragged = true;
    data.pointer.style.stroke = getCssVariable("--knob-selected");
    data.startMoveX = e.clientX;
    data.startMoveY = e.clientY;
    data.startValue = data.value || 0;
}

// ─────────────────────────────────────
function GuiKnobOnMouseMove(data, e) {
    let mouseIsDown = e.buttons === 1;
    let selectedCol = getCssVariable("--knob-selected");
    if (data.pointer.getAttribute("stroke") !== selectedCol && mouseIsDown) {
        data.beingDragged = true;
        data.pointer.style.stroke = selectedCol;
        data.pointer.setAttribute("stroke", selectedCol);
        data.startMoveX = e.clientX;
        data.startMoveY = e.clientY;
        data.startValue = data.value || 0;
    }

    if (!data.beingDragged) {
        return;
    }

    // Calculate the vertical movement (dy) since the dragging started
    const startY = data.startMoveY;
    const endY = e.clientY;
    const dy = startY - endY; // Positive dy means upward movement, negative means downward

    // Calculate the change in value based on dy
    const knobRange = data.ag_range; // Maximum knob value (in degrees)
    const sensitivity = data.size * 2; // Full range achieved with twice the size of the knob
    const valueChange = (dy / sensitivity) * knobRange;

    // Update the knob value based on the initial value when dragging started
    data.value = Math.min(Math.max(data.startValue + valueChange, 0), knobRange);

    // Calculate the half angle to center the arc around 12 o'clock
    const halfAngle = Math.PI * (data.ag_range / 360);
    const startAngle = -Math.PI / 2 - halfAngle; // Adjusted start point

    if (data.discrete) {
        const tickValue = data.ag_range / (data.ticks - 1);
        let discreteAngle = 0;
        for (let i = 0; i < data.ticks; i++) {
            if (data.value >= i * tickValue && data.value <= (i + 1) * tickValue) {
                const midPoint = i * tickValue + tickValue / 2;
                discreteAngle = data.value < midPoint ? i : i + 1;
                break;
            }
        }
        let pos = GuiGetTickPosition(data, discreteAngle);
        data.pointer.setAttribute("x1", pos.x1);
        data.pointer.setAttribute("y1", pos.y1);
        let mappedValue = ((discreteAngle * tickValue) / knobRange) * (data.max - data.min) + data.min;
        if (Pd4Web) {
            Pd4Web.sendFloat(data.send, mappedValue);
        }
    } else {
        const angleRadians = startAngle + (data.value / knobRange) * (2 * halfAngle);
        const pointerRadius = ((data.size - 2) / 2) * data.radius; // Adjust pointer length relative to knob size
        const x1 = data.cx + pointerRadius * Math.cos(angleRadians);
        const y1 = data.cy + pointerRadius * Math.sin(angleRadians);
        data.pointer.setAttribute("x1", x1);
        data.pointer.setAttribute("y1", y1);
        let mappedValue = (data.value / knobRange) * (data.max - data.min) + data.min;
        if (Pd4Web) {
            Pd4Web.sendFloat(data.send, mappedValue);
        }
    }
}

// ─────────────────────────────────────
function GuiKnobOnMouseUp(data, e, n) {
    let color = getCssVariable("--knob-pointer");
    data.pointer.setAttribute("stroke", color);
    data.pointer.style.stroke = getCssVariable(color);
    data.beingDragged = false;
}

// ─────────────────────────────────────
function GuiKnobSetup(args, id) {
    const data = {};
    data.x_pos = parseInt(args[2]);
    data.y_pos = parseInt(args[3]);
    data.type = args[4];

    data.size = parseInt(args[5]);
    data.min = parseFloat(args[6]);
    data.max = parseFloat(args[7]);
    data.init_value = parseFloat(args[8]);
    data.value = data.init_value;
    data.exp = parseFloat(args[9]);
    data.send = args[10];
    data.receive = args[11];
    data.bg = args[12];
    data.arc_color = args[13];
    data.fg = args[14];

    //data.something = args[15];
    data.circular_drag = args[16]; // not supported
    data.ticks = parseInt(args[17]);
    data.discrete = parseInt(args[18]);
    data.show_arc = parseInt(args[19]);

    data.ag_range = parseInt(args[20]);
    data.offset = parseFloat(args[21]);

    data.id = `${data.type}_${id}`;
    data.radius = 0.6;

    // create svg
    data.rect = CreateItem("rect", GuiKnobRect(data));
    data.circle = CreateItem("circle", GuiKnobCircleCenter(data));
    if (data.show_arc) {
        data.circleBg = CreateItem("path", GuiKnobArc(data));
    }
    data.pointer = CreateItem("line", GuiKnobPointer(data));
    data.svgTicks = GuiKnobTicks(data);

    data.rect.addEventListener("mousemove", function (e) {
        GuiKnobOnMouseMove(data, e, 0);
    });
    data.rect.addEventListener("mouseup", function (e) {
        GuiKnobOnMouseUp(data, e, 0);
    });

    // subscribe receiver
    BindGuiReceiver(data);
}

//╭─────────────────────────────────────╮
//│              KeyBoard               │
//╰─────────────────────────────────────╯
function GuiKeyboardRect(data) {
    let width = data.width;
    let height = data.height;
    return {
        x: data.x_pos,
        y: data.y_pos,
        rx: 1,
        ry: 1,
        midi: data.midi,
        send: data.send,
        width: width,
        height: height,
        stroke: data.stroke,
        class: data.class,
        id: `${data.id}_key`,
    };
}

// ─────────────────────────────────────
function GuiKeyboardSetup(args, id) {
    const data = {};
    data.x_pos = parseInt(args[2]) - Pd4Web.x_pos;
    data.y_pos = parseInt(args[3]) - Pd4Web.y_pos;
    data.type = args[4];
    data.width = parseInt(args[5]);
    data.height = parseInt(args[6]);
    data.octave = parseInt(args[7]);
    data.lowC = parseInt(args[8]);
    data.velocity_nor = parseInt(args[9]);
    data.toggle = parseInt(args[10]);
    data.send = args[11];
    data.receive = args[12];

    data.keys = [];
    let keyI = 0;
    let keyX = data.x_pos;

    let allKeys = [];
    for (let i = 0; i < data.octave; i++) {
        for (let j = 0; j < 7; j++) {
            let key = {};
            key.x_pos = keyX;
            key.y_pos = data.y_pos;
            key.width = data.width;
            key.height = data.height;
            key.stroke = "black";
            key.class = "key-white";
            key.midi = (data.lowC + 1) * 12 + keyI;
            key.send = data.send;
            key.id = "white_" + keyI;
            key.index = keyI;
            allKeys[keyI] = key;
            keyI += 1;
            if (j !== 2 && j !== 6) {
                let blackKey = {};
                blackKey.id = "black_" + keyI;
                blackKey.x_pos = keyX + data.width / 1.5;
                blackKey.y_pos = data.y_pos;
                blackKey.stroke = "black";
                blackKey.class = "key-black";
                blackKey.midi = (data.lowC + 1) * 12 + keyI;
                blackKey.send = data.send;
                blackKey.width = data.width * 0.66;
                blackKey.height = data.height * 0.6;
                blackKey.index = keyI;
                allKeys[keyI] = blackKey;
                keyI += 1;
            }
            keyX += data.width;
        }
    }
    for (let key of allKeys) {
        if (key.class === "key-white") {
            data.keys[key.index] = CreateItem("rect", GuiKeyboardRect(key));
        }
    }
    for (let key of allKeys) {
        if (key.class === "key-black") {
            data.keys[key.index] = CreateItem("rect", GuiKeyboardRect(key));
        }
    }

    data.keys.forEach((keyElement) => {
        keyElement.addEventListener("mousedown", function (e) {
            const p = GuiMousePoint(e);
            let midi = e.target.getAttribute("midi");
            let vel = ((p.y - data.y_pos) / data.height) * 127;
            e.target.style.fill = getCssVariable("--key-down");
            if (Pd4Web) {
                if (Pd4Web.sendList !== undefined) {
                    Pd4Web.sendList(e.target.getAttribute("send"), [parseFloat(midi), vel]);
                }
            }
        });
        keyElement.addEventListener("mouseup", function (e) {
            if (e.target.getAttribute("class").includes("key-black")) {
                e.target.style.fill = getCssVariable("--keyboard-black-key");
            } else {
                e.target.style.fill = getCssVariable("--keyboard-white-key");
            }
            let midi = e.target.getAttribute("midi");
            if (Pd4Web) {
                if (Pd4Web.sendList !== undefined) {
                    Pd4Web.sendList(e.target.getAttribute("send"), [parseFloat(midi), 0]);
                }
            }
        });
    });
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
        class: "comment",
    };
}

//╭─────────────────────────────────────╮
//│           Patch Handling            │
//╰─────────────────────────────────────╯
function UpdatePatchDivSize(content, patch_zoom) {
    const patchDiv = document.getElementById("Pd4WebPatchDiv");
    if (patchDiv == null) {
        return;
    }

    if (Pd4Web.isMobile) {
        patchDiv.style.width = "90%";
        patchDiv.style.marginLeft = "auto";
        patchDiv.style.marginRight = "auto";
    } else {
        const lines = content.split(";\n");
        var args = lines[0].split(" ");
        const canvasHeight = parseInt(args[5]);
        const canvasWidth = parseInt(args[4]);
        console.log(canvasWidth, canvasHeight);
        patchDiv.style.width = canvasWidth * patch_zoom + "px";
        patchDiv.style.height = canvasHeight * patch_zoom + "px";
        patchDiv.style.marginLeft = "auto";
        patchDiv.style.marginRight = "auto";
    }
}

// ─────────────────────────────────────
function UpdatePatchDivSizeCoords(width, height, patch_zoom) {
    const patchDiv = document.getElementById("Pd4WebPatchDiv");
    if (patchDiv == null) {
        console.warn("Patch div not found");
        return;
    }
    if (Pd4Web.isMobile) {
        patchDiv.style.width = "90%";
        patchDiv.style.marginLeft = "auto";
        patchDiv.style.marginRight = "auto";
    } else {
        patchDiv.style.width = width * patch_zoom + "px";
        patchDiv.style.height = height * patch_zoom + "px";
        patchDiv.style.marginLeft = "auto";
        patchDiv.style.marginRight = "auto";
    }
}

// ─────────────────────────────────────
function OpenPatch(content) {
    content = content.replace(/\r/g, "");
    let canvasLevel = 0;
    let id = 0;

    if (Pd4Web.Canvas) {
        while (Pd4Web.Canvas.lastChild) {
            Pd4Web.Canvas.removeChild(Pd4Web.Canvas.lastChild);
        }
    }

    UpdatePatchDivSize(content, Pd4Web.Zoom);

    const lines = content.split(";\n");
    Pd4Web.x_pos = 0;
    Pd4Web.y_pos = 0;
    let canvasLevelLocal = 0;
    for (let line of lines) {
        line = line.replace(/[\r\n]+/g, " ").trim();
        const args = line.split(" ");
        const type = args.slice(0, 2).join(" ");
        switch (type) {
            case "#N canvas":
                canvasLevelLocal++;
                if (canvasLevelLocal == 1) {
                    Pd4Web.width = parseInt(args[4]);
                    Pd4Web.height = parseInt(args[5]);
                }
                break;
            case "#X restore":
                canvasLevelLocal--;
                break;
            case "#X coords":
                if (canvasLevelLocal == 1) {
                    if (args.length == 11) {
                        Pd4Web.width = parseInt(args[6]);
                        Pd4Web.height = parseInt(args[7]);
                        Pd4Web.x_pos = parseInt(args[9]);
                        Pd4Web.y_pos = parseInt(args[10]);
                        UpdatePatchDivSizeCoords(Pd4Web.width, Pd4Web.height, Pd4Web.Zoom);
                    }
                }
                break;
        }
    }

    for (let line of lines) {
        line = line.replace(/[\r\n]+/g, " ").trim(); // remove newlines & carriage returns
        id++;
        const args = line.split(" ");
        const type = args.slice(0, 2).join(" ");
        switch (type) {
            case "#N canvas":
                canvasLevel++;
                if (canvasLevel === 1 && args.length === 7) {
                    Pd4Web.CanvasWidth = Pd4Web.width;
                    Pd4Web.CanvasHeight = Pd4Web.height;
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
                                GuiBngSetup(args, id);
                            }
                            break;
                        case "tgl":
                            if (canvasLevel === 1 && args.length === 19 && args[7] !== "empty" && args[8] !== "empty") {
                                GuiTglSetup(args, id);
                            }
                            break;

                        case "nbx":
                            if (canvasLevel === 1 && args.length === 23 && args[7] !== "empty" && args[8] !== "empty") {
                                GuiNbxSetup(args, id);
                            }
                            break;

                        case "vsl":
                        case "hsl":
                            if (
                                canvasLevel === 1 &&
                                args.length === 23 &&
                                args[11] !== "empty" &&
                                args[12] !== "empty"
                            ) {
                                GuiSliderSetup(args, id);
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
                                GuiRadioSetup(args, id);
                            }
                            break;
                        case "vu":
                            if (canvasLevel === 1 && args.length === 17 && args[7] !== "empty") {
                                GuiVuSetup(args, id);
                            }
                            break;
                        case "cnv":
                            if (canvasLevel === 1 && args.length === 18) {
                                GuiCnvSetup(args, id);
                            }
                            break;

                        //╭─────────────────────────────────────╮
                        //│        External Gui Objects         │
                        //╰─────────────────────────────────────╯
                        case "knob": // ELSE/KNOB
                            if (canvasLevel === 1) {
                                GuiKnobSetup(args, id);
                            }
                            break;
                        case "keyboard": // ELSE/KEYBOARD
                            if (canvasLevel === 1) {
                                GuiKeyboardSetup(args, id);
                            }
                            break;
                    }
                }
                break;
            case "#X text":
                if (args.length > 4 && canvasLevel === 1) {
                    const data = {};
                    data.type = args[1];
                    data.x_pos = parseInt(args[2]) - Pd4Web.x_pos;
                    data.y_pos = parseInt(args[3]) - Pd4Web.y_pos;
                    data.comment = [];
                    const lines = args
                        .slice(4)
                        .join(" ")
                        .replace(/ \\,/g, ",")
                        .replace(/\\; /g, ";\n")
                        .replace(/ ;/g, ";")
                        .split("\n");

                    const regex = /, f (\d+)$/;
                    const match = lines[lines.length - 1].match(regex);
                    let width = 60;
                    if (match) {
                        width = parseInt(match[1], 10); // Extract the number and convert to an integer
                        lines[lines.length - 1] = lines[lines.length - 1].replace(match[0], "").trim();
                    }

                    for (const line of lines) {
                        const dynamicRegex = new RegExp(`.{1,${width}}(\\s|$)`, "g");
                        const parts = line.match(dynamicRegex);
                        if (parts) {
                            for (const part of parts) {
                                data.comment.push(part.trim());
                            }
                        }
                    }

                    data.id = `${data.type}_${id}`;
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
async function Pd4WebInitGui(patch) {
    if (Pd4Web === undefined) {
        setTimeout(Pd4WebInitGui, 150);
        console.log("Pd4Web is not defined yet, wait...");
        return;
    }

    // Get the element
    setSoundIcon("--sound-off", "pulse 1s infinite");

    Pd4Web.isMobile = /Mobi|Android|iPhone|iPad|iPod|Opera Mini|IEMobile|WPDesktop/i.test(navigator.userAgent);
    Pd4Web.CanvasWidth = 450;
    Pd4Web.CanvasHeight = 300;
    Pd4Web.FontSize = 12;
    if (typeof Pd4Web.GuiReceivers === "undefined") {
        Pd4Web.GuiReceivers = {}; // defined in pd4web.cpp Pd4WebJsHelpers
    }
    Pd4Web.Canvas = document.getElementById("Pd4WebCanvas");
    Pd4Web.Touches = {};
    Pd4Web.FontEngineSanity = false;
    Pd4Web.AutoTheme = true;

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

    // Auto Theming
    GetNeededStyles();
    const darkModeMediaQuery = window.matchMedia("(prefers-color-scheme: dark)");
    darkModeMediaQuery.addEventListener("change", ThemeListener);

    // Open Patch
    if (Pd4Web.Canvas) {
        var File = patch;
        fetch(File)
            .then((response) => {
                if (!response.ok) {
                    throw new Error("Network response was not ok");
                }
                return response.text();
            })
            .then((textContent) => {
                OpenPatch(textContent);
            })
            .catch((error) => {
                console.error("There has been a problem with your fetch operation:", error);
            });
    }
}

//╭─────────────────────────────────────╮
//│                                     │
//╰─────────────────────────────────────╯
class Pd4WebClass {
    constructor(patch = "index.pd") {
        this.patch = patch;
    }

    async init() {
        this.Pd4Web = null; // Pd4Web object must be declared in the global scope
        Pd4WebModule().then((Pd4WebModulePromise) => {
            this.Pd4Web = new Pd4WebModulePromise.Pd4Web();
            this.Pd4Web.init();
        });
    }

    OpenPatch(content) {
        content = content.replace(/\r/g, "");
        let canvasLevel = 0;
        let id = 0;

        if (this.Pd4Web.Canvas) {
            while (this.Pd4Web.Canvas.lastChild) {
                this.Pd4Web.Canvas.removeChild(this.Pd4Web.Canvas.lastChild);
            }
        }

        UpdatePatchDivSize(content, this.Pd4Web.Zoom);

        const lines = content.split(";\n");
        this.Pd4Web.x_pos = 0;
        this.Pd4Web.y_pos = 0;
        let canvasLevelLocal = 0;
        for (let line of lines) {
            line = line.replace(/[\r\n]+/g, " ").trim();
            const args = line.split(" ");
            const type = args.slice(0, 2).join(" ");
            switch (type) {
                case "#N canvas":
                    canvasLevelLocal++;
                    if (canvasLevelLocal == 1) {
                        this.Pd4Web.width = parseInt(args[4]);
                        this.Pd4Web.height = parseInt(args[5]);
                    }
                    break;
                case "#X restore":
                    canvasLevelLocal--;
                    break;
                case "#X coords":
                    if (canvasLevelLocal == 1) {
                        if (args.length == 11) {
                            this.Pd4Web.width = parseInt(args[6]);
                            this.Pd4Web.height = parseInt(args[7]);
                            this.Pd4Web.x_pos = parseInt(args[9]);
                            this.Pd4Web.y_pos = parseInt(args[10]);
                            UpdatePatchDivSizeCoords(this.Pd4Web.width, this.Pd4Web.height, this.Pd4Web.Zoom);
                        }
                    }
                    break;
            }
        }

        for (let line of lines) {
            line = line.replace(/[\r\n]+/g, " ").trim(); // remove newlines & carriage returns
            id++;
            const args = line.split(" ");
            const type = args.slice(0, 2).join(" ");
            switch (type) {
                case "#N canvas":
                    canvasLevel++;
                    if (canvasLevel === 1 && args.length === 7) {
                        this.Pd4Web.CanvasWidth = this.Pd4Web.width;
                        this.Pd4Web.CanvasHeight = this.Pd4Web.height;
                        this.Pd4Web.FontSize = parseInt(args[6]);
                        this.Pd4Web.Canvas.setAttributeNS(
                            null,
                            "viewBox",
                            `0 0 ${this.Pd4Web.CanvasWidth} ${this.Pd4Web.CanvasHeight}`,
                        );
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
                                    GuiBngSetup(args, id);
                                }
                                break;
                            case "tgl":
                                if (
                                    canvasLevel === 1 &&
                                    args.length === 19 &&
                                    args[7] !== "empty" &&
                                    args[8] !== "empty"
                                ) {
                                    GuiTglSetup(args, id);
                                }
                                break;

                            case "nbx":
                                if (
                                    canvasLevel === 1 &&
                                    args.length === 23 &&
                                    args[7] !== "empty" &&
                                    args[8] !== "empty"
                                ) {
                                    GuiNbxSetup(args, id);
                                }
                                break;

                            case "vsl":
                            case "hsl":
                                if (
                                    canvasLevel === 1 &&
                                    args.length === 23 &&
                                    args[11] !== "empty" &&
                                    args[12] !== "empty"
                                ) {
                                    GuiSliderSetup(args, id);
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
                                    GuiRadioSetup(args, id);
                                }
                                break;
                            case "vu":
                                if (canvasLevel === 1 && args.length === 17 && args[7] !== "empty") {
                                    GuiVuSetup(args, id);
                                }
                                break;
                            case "cnv":
                                if (canvasLevel === 1 && args.length === 18) {
                                    GuiCnvSetup(args, id);
                                }
                                break;

                            //╭─────────────────────────────────────╮
                            //│        External Gui Objects         │
                            //╰─────────────────────────────────────╯
                            case "knob": // ELSE/KNOB
                                if (canvasLevel === 1) {
                                    GuiKnobSetup(args, id);
                                }
                                break;
                            case "keyboard": // ELSE/KEYBOARD
                                if (canvasLevel === 1) {
                                    GuiKeyboardSetup(args, id);
                                }
                                break;
                        }
                    }
                    break;
                case "#X text":
                    if (args.length > 4 && canvasLevel === 1) {
                        const data = {};
                        data.type = args[1];
                        data.x_pos = parseInt(args[2]) - this.Pd4Web.x_pos;
                        data.y_pos = parseInt(args[3]) - this.Pd4Web.y_pos;
                        data.comment = [];
                        const lines = args
                            .slice(4)
                            .join(" ")
                            .replace(/ \\,/g, ",")
                            .replace(/\\; /g, ";\n")
                            .replace(/ ;/g, ";")
                            .split("\n");

                        const regex = /, f (\d+)$/;
                        const match = lines[lines.length - 1].match(regex);
                        let width = 60;
                        if (match) {
                            width = parseInt(match[1], 10); // Extract the number and convert to an integer
                            lines[lines.length - 1] = lines[lines.length - 1].replace(match[0], "").trim();
                        }

                        for (const line of lines) {
                            const dynamicRegex = new RegExp(`.{1,${width}}(\\s|$)`, "g");
                            const parts = line.match(dynamicRegex);
                            if (parts) {
                                for (const part of parts) {
                                    data.comment.push(part.trim());
                                }
                            }
                        }

                        data.id = `${data.type}_${id}`;
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

    async Pd4WebInitGui() {
        if (this.Pd4Web === undefined) {
            setTimeout(this.Pd4WebInitGui, 150);
            return;
        }

        // Get the element
        setSoundIcon("--sound-off", "pulse 1s infinite");

        this.Pd4Web.isMobile = /Mobi|Android|iPhone|iPad|iPod|Opera Mini|IEMobile|WPDesktop/i.test(navigator.userAgent);
        this.Pd4Web.CanvasWidth = 450;
        this.Pd4Web.CanvasHeight = 300;
        this.Pd4Web.FontSize = 12;
        if (typeof this.Pd4Web.GuiReceivers === "undefined") {
            this.Pd4Web.GuiReceivers = {}; // defined in pd4web.cpp Pd4WebJsHelpers
        }
        this.Canvas = document.getElementById("Pd4WebCanvas");
        this.Touches = {};
        this.FontEngineSanity = false;
        this.AutoTheme = true;

        if (this.isMobile) {
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

        // Auto Theming
        GetNeededStyles();
        const darkModeMediaQuery = window.matchMedia("(prefers-color-scheme: dark)");
        darkModeMediaQuery.addEventListener("change", ThemeListener);

        // Open Patch
        if (this.Canvas) {
            var File = patch;
            fetch(File)
                .then((response) => {
                    if (!response.ok) {
                        throw new Error("Network response was not ok");
                    }
                    return response.text();
                })
                .then((textContent) => {
                    OpenPatch(textContent);
                })
                .catch((error) => {
                    console.error("There has been a problem with your fetch operation:", error);
                });
        }
    }
}
// end include: /home/neimog/.config/miniconda3.dir/lib/python3.12/site-packages/pd4web/pd4web.gui.js
// include: /tmp/tmp68nwadqm.js

    if (!Module['preRun']) throw 'Module.preRun should exist because file support used it; did a pre-js delete it?';
    necessaryPreJSTasks.forEach((task) => {
      if (Module['preRun'].indexOf(task) < 0) throw 'All preRun tasks that exist before user pre-js code should remain after; did you replace Module or modify Module.preRun?';
    });
  // end include: /tmp/tmp68nwadqm.js


// Sometimes an existing Module object exists with properties
// meant to overwrite the default module functionality. Here
// we collect those properties and reapply _after_ we configure
// the current environment's defaults to avoid having to be so
// defensive during initialization.
var moduleOverrides = Object.assign({}, Module);

var arguments_ = [];
var thisProgram = './this.program';
var quit_ = (status, toThrow) => {
  throw toThrow;
};

// `/` should be present at the end if `scriptDirectory` is not empty
var scriptDirectory = '';
function locateFile(path) {
  if (Module['locateFile']) {
    return Module['locateFile'](path, scriptDirectory);
  }
  return scriptDirectory + path;
}

// Hooks that are implemented differently in different runtime environments.
var readAsync, readBinary;

if (ENVIRONMENT_IS_NODE) {
  if (typeof process == 'undefined' || !process.release || process.release.name !== 'node') throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');

  var nodeVersion = process.versions.node;
  var numericVersion = nodeVersion.split('.').slice(0, 3);
  numericVersion = (numericVersion[0] * 10000) + (numericVersion[1] * 100) + (numericVersion[2].split('-')[0] * 1);
  var minVersion = 160400;
  if (numericVersion < 160400) {
    throw new Error('This emscripten-generated code requires node v16.04.4.0 (detected v' + nodeVersion + ')');
  }

  // These modules will usually be used on Node.js. Load them eagerly to avoid
  // the complexity of lazy-loading.
  var fs = require('fs');
  var nodePath = require('path');

  scriptDirectory = __dirname + '/';

// include: node_shell_read.js
readBinary = (filename) => {
  // We need to re-wrap `file://` strings to URLs.
  filename = isFileURI(filename) ? new URL(filename) : filename;
  var ret = fs.readFileSync(filename);
  assert(Buffer.isBuffer(ret));
  return ret;
};

readAsync = async (filename, binary = true) => {
  // See the comment in the `readBinary` function.
  filename = isFileURI(filename) ? new URL(filename) : filename;
  var ret = fs.readFileSync(filename, binary ? undefined : 'utf8');
  assert(binary ? Buffer.isBuffer(ret) : typeof ret == 'string');
  return ret;
};
// end include: node_shell_read.js
  if (!Module['thisProgram'] && process.argv.length > 1) {
    thisProgram = process.argv[1].replace(/\\/g, '/');
  }

  arguments_ = process.argv.slice(2);

  // MODULARIZE will export the module in the proper place outside, we don't need to export here

  quit_ = (status, toThrow) => {
    process.exitCode = status;
    throw toThrow;
  };

} else
if (ENVIRONMENT_IS_SHELL) {

  if ((typeof process == 'object' && typeof require === 'function') || typeof window == 'object' || typeof WorkerGlobalScope != 'undefined') throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');

} else

// Note that this includes Node.js workers when relevant (pthreads is enabled).
// Node.js workers are detected as a combination of ENVIRONMENT_IS_WORKER and
// ENVIRONMENT_IS_NODE.
if (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) {
  if (ENVIRONMENT_IS_WORKER) { // Check worker, not web, since window could be polyfilled
    scriptDirectory = self.location.href;
  } else if (typeof document != 'undefined' && document.currentScript) { // web
    scriptDirectory = document.currentScript.src;
  }
  // When MODULARIZE, this JS may be executed later, after document.currentScript
  // is gone, so we saved it, and we use it here instead of any other info.
  if (_scriptName) {
    scriptDirectory = _scriptName;
  }
  // blob urls look like blob:http://site.com/etc/etc and we cannot infer anything from them.
  // otherwise, slice off the final part of the url to find the script directory.
  // if scriptDirectory does not contain a slash, lastIndexOf will return -1,
  // and scriptDirectory will correctly be replaced with an empty string.
  // If scriptDirectory contains a query (starting with ?) or a fragment (starting with #),
  // they are removed because they could contain a slash.
  if (scriptDirectory.startsWith('blob:')) {
    scriptDirectory = '';
  } else {
    scriptDirectory = scriptDirectory.slice(0, scriptDirectory.replace(/[?#].*/, '').lastIndexOf('/')+1);
  }

  if (!(typeof window == 'object' || typeof WorkerGlobalScope != 'undefined')) throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');

  // Differentiate the Web Worker from the Node Worker case, as reading must
  // be done differently.
  if (!ENVIRONMENT_IS_NODE)
  {
// include: web_or_worker_shell_read.js
if (ENVIRONMENT_IS_WORKER) {
    readBinary = (url) => {
      var xhr = new XMLHttpRequest();
      xhr.open('GET', url, false);
      xhr.responseType = 'arraybuffer';
      xhr.send(null);
      return new Uint8Array(/** @type{!ArrayBuffer} */(xhr.response));
    };
  }

  readAsync = async (url) => {
    // Fetch has some additional restrictions over XHR, like it can't be used on a file:// url.
    // See https://github.com/github/fetch/pull/92#issuecomment-140665932
    // Cordova or Electron apps are typically loaded from a file:// url.
    // So use XHR on webview if URL is a file URL.
    if (isFileURI(url)) {
      return new Promise((resolve, reject) => {
        var xhr = new XMLHttpRequest();
        xhr.open('GET', url, true);
        xhr.responseType = 'arraybuffer';
        xhr.onload = () => {
          if (xhr.status == 200 || (xhr.status == 0 && xhr.response)) { // file URLs can return 0
            resolve(xhr.response);
            return;
          }
          reject(xhr.status);
        };
        xhr.onerror = reject;
        xhr.send(null);
      });
    }
    var response = await fetch(url, { credentials: 'same-origin' });
    if (response.ok) {
      return response.arrayBuffer();
    }
    throw new Error(response.status + ' : ' + response.url);
  };
// end include: web_or_worker_shell_read.js
  }
} else
if (!ENVIRONMENT_IS_AUDIO_WORKLET)
{
  throw new Error('environment detection error');
}

// Set up the out() and err() hooks, which are how we can print to stdout or
// stderr, respectively.
// Normally just binding console.log/console.error here works fine, but
// under node (with workers) we see missing/out-of-order messages so route
// directly to stdout and stderr.
// See https://github.com/emscripten-core/emscripten/issues/14804
var defaultPrint = console.log.bind(console);
var defaultPrintErr = console.error.bind(console);
if (ENVIRONMENT_IS_NODE) {
  defaultPrint = (...args) => fs.writeSync(1, args.join(' ') + '\n');
  defaultPrintErr = (...args) => fs.writeSync(2, args.join(' ') + '\n');
}
var out = Module['print'] || defaultPrint;
var err = Module['printErr'] || defaultPrintErr;

// Merge back in the overrides
Object.assign(Module, moduleOverrides);
// Free the object hierarchy contained in the overrides, this lets the GC
// reclaim data used.
moduleOverrides = null;
checkIncomingModuleAPI();

// Emit code to handle expected values on the Module object. This applies Module.x
// to the proper local x. This has two benefits: first, we only emit it if it is
// expected to arrive, and second, by using a local everywhere else that can be
// minified.

if (Module['arguments']) arguments_ = Module['arguments'];legacyModuleProp('arguments', 'arguments_');

if (Module['thisProgram']) thisProgram = Module['thisProgram'];legacyModuleProp('thisProgram', 'thisProgram');

// perform assertions in shell.js after we set up out() and err(), as otherwise if an assertion fails it cannot print the message
// Assertions on removed incoming Module JS APIs.
assert(typeof Module['memoryInitializerPrefixURL'] == 'undefined', 'Module.memoryInitializerPrefixURL option was removed, use Module.locateFile instead');
assert(typeof Module['pthreadMainPrefixURL'] == 'undefined', 'Module.pthreadMainPrefixURL option was removed, use Module.locateFile instead');
assert(typeof Module['cdInitializerPrefixURL'] == 'undefined', 'Module.cdInitializerPrefixURL option was removed, use Module.locateFile instead');
assert(typeof Module['filePackagePrefixURL'] == 'undefined', 'Module.filePackagePrefixURL option was removed, use Module.locateFile instead');
assert(typeof Module['read'] == 'undefined', 'Module.read option was removed');
assert(typeof Module['readAsync'] == 'undefined', 'Module.readAsync option was removed (modify readAsync in JS)');
assert(typeof Module['readBinary'] == 'undefined', 'Module.readBinary option was removed (modify readBinary in JS)');
assert(typeof Module['setWindowTitle'] == 'undefined', 'Module.setWindowTitle option was removed (modify emscripten_set_window_title in JS)');
assert(typeof Module['TOTAL_MEMORY'] == 'undefined', 'Module.TOTAL_MEMORY has been renamed Module.INITIAL_MEMORY');
legacyModuleProp('asm', 'wasmExports');
legacyModuleProp('readAsync', 'readAsync');
legacyModuleProp('readBinary', 'readBinary');
legacyModuleProp('setWindowTitle', 'setWindowTitle');
var IDBFS = 'IDBFS is no longer included by default; build with -lidbfs.js';
var PROXYFS = 'PROXYFS is no longer included by default; build with -lproxyfs.js';
var WORKERFS = 'WORKERFS is no longer included by default; build with -lworkerfs.js';
var FETCHFS = 'FETCHFS is no longer included by default; build with -lfetchfs.js';
var ICASEFS = 'ICASEFS is no longer included by default; build with -licasefs.js';
var JSFILEFS = 'JSFILEFS is no longer included by default; build with -ljsfilefs.js';
var OPFS = 'OPFS is no longer included by default; build with -lopfs.js';

var NODEFS = 'NODEFS is no longer included by default; build with -lnodefs.js';

assert(
  ENVIRONMENT_IS_AUDIO_WORKLET ||
  ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER || ENVIRONMENT_IS_NODE, 'Pthreads do not work in this environment yet (need Web Workers, or an alternative to them)');

assert(!ENVIRONMENT_IS_SHELL, 'shell environment detected but not enabled at build time.  Add `shell` to `-sENVIRONMENT` to enable.');

// end include: shell.js

// include: preamble.js
// === Preamble library stuff ===

// Documentation for the public APIs defined in this file must be updated in:
//    site/source/docs/api_reference/preamble.js.rst
// A prebuilt local version of the documentation is available at:
//    site/build/text/docs/api_reference/preamble.js.txt
// You can also build docs locally as HTML or other formats in site/
// An online HTML version (which may be of a different version of Emscripten)
//    is up at http://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html

var wasmBinary = Module['wasmBinary'];legacyModuleProp('wasmBinary', 'wasmBinary');

if (typeof WebAssembly != 'object') {
  err('no native wasm support detected');
}

// Wasm globals

var wasmMemory;

// For sending to workers.
var wasmModule;

//========================================
// Runtime essentials
//========================================

// whether we are quitting the application. no code should run after this.
// set in exit() and abort()
var ABORT = false;

// set by exit() and abort().  Passed to 'onExit' handler.
// NOTE: This is also used as the process return code code in shell environments
// but only when noExitRuntime is false.
var EXITSTATUS;

// In STRICT mode, we only define assert() when ASSERTIONS is set.  i.e. we
// don't define it at all in release modes.  This matches the behaviour of
// MINIMAL_RUNTIME.
// TODO(sbc): Make this the default even without STRICT enabled.
/** @type {function(*, string=)} */
function assert(condition, text) {
  if (!condition) {
    abort('Assertion failed' + (text ? ': ' + text : ''));
  }
}

// We used to include malloc/free by default in the past. Show a helpful error in
// builds with assertions.

// Memory management

var HEAP,
/** @type {!Int8Array} */
  HEAP8,
/** @type {!Uint8Array} */
  HEAPU8,
/** @type {!Int16Array} */
  HEAP16,
/** @type {!Uint16Array} */
  HEAPU16,
/** @type {!Int32Array} */
  HEAP32,
/** @type {!Uint32Array} */
  HEAPU32,
/** @type {!Float32Array} */
  HEAPF32,
/* BigInt64Array type is not correctly defined in closure
/** not-@type {!BigInt64Array} */
  HEAP64,
/* BigUint64Array type is not correctly defined in closure
/** not-t@type {!BigUint64Array} */
  HEAPU64,
/** @type {!Float64Array} */
  HEAPF64;

var runtimeInitialized = false;

/**
 * Indicates whether filename is delivered via file protocol (as opposed to http/https)
 * @noinline
 */
var isFileURI = (filename) => filename.startsWith('file://');

// include: runtime_shared.js
// include: runtime_stack_check.js
// Initializes the stack cookie. Called at the startup of main and at the startup of each thread in pthreads mode.
function writeStackCookie() {
  var max = _emscripten_stack_get_end();
  assert((max & 3) == 0);
  // If the stack ends at address zero we write our cookies 4 bytes into the
  // stack.  This prevents interference with SAFE_HEAP and ASAN which also
  // monitor writes to address zero.
  if (max == 0) {
    max += 4;
  }
  // The stack grow downwards towards _emscripten_stack_get_end.
  // We write cookies to the final two words in the stack and detect if they are
  // ever overwritten.
  HEAPU32[((max)>>2)] = 0x02135467;
  HEAPU32[(((max)+(4))>>2)] = 0x89BACDFE;
  // Also test the global address 0 for integrity.
  HEAPU32[((0)>>2)] = 1668509029;
}

function checkStackCookie() {
  if (ABORT) return;
  var max = _emscripten_stack_get_end();
  // See writeStackCookie().
  if (max == 0) {
    max += 4;
  }
  var cookie1 = HEAPU32[((max)>>2)];
  var cookie2 = HEAPU32[(((max)+(4))>>2)];
  if (cookie1 != 0x02135467 || cookie2 != 0x89BACDFE) {
    abort(`Stack overflow! Stack cookie has been overwritten at ${ptrToString(max)}, expected hex dwords 0x89BACDFE and 0x2135467, but received ${ptrToString(cookie2)} ${ptrToString(cookie1)}`);
  }
  // Also test the global address 0 for integrity.
  if (HEAPU32[((0)>>2)] != 0x63736d65 /* 'emsc' */) {
    abort('Runtime error: The application has corrupted its heap memory area (address zero)!');
  }
}
// end include: runtime_stack_check.js
// include: runtime_exceptions.js
// end include: runtime_exceptions.js
// include: runtime_debug.js
// Endianness check
(() => {
  var h16 = new Int16Array(1);
  var h8 = new Int8Array(h16.buffer);
  h16[0] = 0x6373;
  if (h8[0] !== 0x73 || h8[1] !== 0x63) throw 'Runtime error: expected the system to be little-endian! (Run with -sSUPPORT_BIG_ENDIAN to bypass)';
})();

if (Module['ENVIRONMENT']) {
  throw new Error('Module.ENVIRONMENT has been deprecated. To force the environment, use the ENVIRONMENT compile-time option (for example, -sENVIRONMENT=web or -sENVIRONMENT=node)');
}

function legacyModuleProp(prop, newName, incoming=true) {
  if (!Object.getOwnPropertyDescriptor(Module, prop)) {
    Object.defineProperty(Module, prop, {
      configurable: true,
      get() {
        let extra = incoming ? ' (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name)' : '';
        abort(`\`Module.${prop}\` has been replaced by \`${newName}\`` + extra);

      }
    });
  }
}

function ignoredModuleProp(prop) {
  if (Object.getOwnPropertyDescriptor(Module, prop)) {
    abort(`\`Module.${prop}\` was supplied but \`${prop}\` not included in INCOMING_MODULE_JS_API`);
  }
}

// forcing the filesystem exports a few things by default
function isExportedByForceFilesystem(name) {
  return name === 'FS_createPath' ||
         name === 'FS_createDataFile' ||
         name === 'FS_createPreloadedFile' ||
         name === 'FS_unlink' ||
         name === 'addRunDependency' ||
         name === 'removeRunDependency';
}

/**
 * Intercept access to a global symbol.  This enables us to give informative
 * warnings/errors when folks attempt to use symbols they did not include in
 * their build, or no symbols that no longer exist.
 */
function hookGlobalSymbolAccess(sym, func) {
  // In MODULARIZE mode the generated code runs inside a function scope and not
  // the global scope, and JavaScript does not provide access to function scopes
  // so we cannot dynamically modify the scrope using `defineProperty` in this
  // case.
  //
  // In this mode we simply ignore requests for `hookGlobalSymbolAccess`. Since
  // this is a debug-only feature, skipping it is not major issue.
}

function missingGlobal(sym, msg) {
  hookGlobalSymbolAccess(sym, () => {
    warnOnce(`\`${sym}\` is not longer defined by emscripten. ${msg}`);
  });
}

missingGlobal('buffer', 'Please use HEAP8.buffer or wasmMemory.buffer');
missingGlobal('asm', 'Please use wasmExports instead');

function missingLibrarySymbol(sym) {
  hookGlobalSymbolAccess(sym, () => {
    // Can't `abort()` here because it would break code that does runtime
    // checks.  e.g. `if (typeof SDL === 'undefined')`.
    var msg = `\`${sym}\` is a library symbol and not included by default; add it to your library.js __deps or to DEFAULT_LIBRARY_FUNCS_TO_INCLUDE on the command line`;
    // DEFAULT_LIBRARY_FUNCS_TO_INCLUDE requires the name as it appears in
    // library.js, which means $name for a JS name with no prefix, or name
    // for a JS name like _name.
    var librarySymbol = sym;
    if (!librarySymbol.startsWith('_')) {
      librarySymbol = '$' + sym;
    }
    msg += ` (e.g. -sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE='${librarySymbol}')`;
    if (isExportedByForceFilesystem(sym)) {
      msg += '. Alternatively, forcing filesystem support (-sFORCE_FILESYSTEM) can export this for you';
    }
    warnOnce(msg);
  });

  // Any symbol that is not included from the JS library is also (by definition)
  // not exported on the Module object.
  unexportedRuntimeSymbol(sym);
}

function unexportedRuntimeSymbol(sym) {
  if (ENVIRONMENT_IS_PTHREAD) {
    return;
  }
  if (!Object.getOwnPropertyDescriptor(Module, sym)) {
    Object.defineProperty(Module, sym, {
      configurable: true,
      get() {
        var msg = `'${sym}' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the Emscripten FAQ)`;
        if (isExportedByForceFilesystem(sym)) {
          msg += '. Alternatively, forcing filesystem support (-sFORCE_FILESYSTEM) can export this for you';
        }
        abort(msg);
      }
    });
  }
}

// Used by XXXXX_DEBUG settings to output debug messages.
function dbg(...args) {
  // Avoid using the console for debugging in multi-threaded node applications
  // See https://github.com/emscripten-core/emscripten/issues/14804
  if (ENVIRONMENT_IS_NODE && fs) {
    fs.writeSync(2, args.join(' ') + '\n');
  } else
  // TODO(sbc): Make this configurable somehow.  Its not always convenient for
  // logging to show up as warnings.
  console.warn(...args);
}
// end include: runtime_debug.js
// include: memoryprofiler.js
// end include: memoryprofiler.js
// include: runtime_pthread.js
// Pthread Web Worker handling code.
// This code runs only on pthread web workers and handles pthread setup
// and communication with the main thread via postMessage.

// Unique ID of the current pthread worker (zero on non-pthread-workers
// including the main thread).
var workerID = 0;

if (ENVIRONMENT_IS_PTHREAD) {
  var wasmModuleReceived;

  // Node.js support
  if (ENVIRONMENT_IS_NODE) {
    // Create as web-worker-like an environment as we can.

    var parentPort = worker_threads['parentPort'];
    parentPort.on('message', (msg) => onmessage({ data: msg }));

    Object.assign(globalThis, {
      self: global,
      postMessage: (msg) => parentPort.postMessage(msg),
    });
  }

  // Thread-local guard variable for one-time init of the JS state
  var initializedJS = false;

  function threadPrintErr(...args) {
    var text = args.join(' ');
    // See https://github.com/emscripten-core/emscripten/issues/14804
    if (ENVIRONMENT_IS_NODE) {
      fs.writeSync(2, text + '\n');
      return;
    }
    console.error(text);
  }

  if (!Module['printErr'])
    err = threadPrintErr;
  dbg = threadPrintErr;
  function threadAlert(...args) {
    var text = args.join(' ');
    postMessage({cmd: 'alert', text, threadId: _pthread_self()});
  }
  self.alert = threadAlert;

  // Turn unhandled rejected promises into errors so that the main thread will be
  // notified about them.
  self.onunhandledrejection = (e) => { throw e.reason || e; };

  function handleMessage(e) {
    try {
      var msgData = e['data'];
      //dbg('msgData: ' + Object.keys(msgData));
      var cmd = msgData.cmd;
      if (cmd === 'load') { // Preload command that is called once per worker to parse and load the Emscripten code.
        workerID = msgData.workerID;

        // Until we initialize the runtime, queue up any further incoming messages.
        let messageQueue = [];
        self.onmessage = (e) => messageQueue.push(e);

        // And add a callback for when the runtime is initialized.
        self.startWorker = (instance) => {
          // Notify the main thread that this thread has loaded.
          postMessage({ cmd: 'loaded' });
          // Process any messages that were queued before the thread was ready.
          for (let msg of messageQueue) {
            handleMessage(msg);
          }
          // Restore the real message handler.
          self.onmessage = handleMessage;
        };

        // Use `const` here to ensure that the variable is scoped only to
        // that iteration, allowing safe reference from a closure.
        for (const handler of msgData.handlers) {
          // The the main module has a handler for a certain even, but no
          // handler exists on the pthread worker, then proxy that handler
          // back to the main thread.
          if (!Module[handler] || Module[handler].proxy) {
            Module[handler] = (...args) => {
              postMessage({ cmd: 'callHandler', handler, args: args });
            }
            // Rebind the out / err handlers if needed
            if (handler == 'print') out = Module[handler];
            if (handler == 'printErr') err = Module[handler];
          }
        }

        wasmMemory = msgData.wasmMemory;
        updateMemoryViews();

        wasmModuleReceived(msgData.wasmModule);
      } else if (cmd === 'run') {
        assert(msgData.pthread_ptr);
        // Call inside JS module to set up the stack frame for this pthread in JS module scope.
        // This needs to be the first thing that we do, as we cannot call to any C/C++ functions
        // until the thread stack is initialized.
        establishStackSpace(msgData.pthread_ptr);

        // Pass the thread address to wasm to store it for fast access.
        __emscripten_thread_init(msgData.pthread_ptr, /*is_main=*/0, /*is_runtime=*/0, /*can_block=*/1, 0, 0);

        PThread.receiveObjectTransfer(msgData);
        PThread.threadInitTLS();

        // Await mailbox notifications with `Atomics.waitAsync` so we can start
        // using the fast `Atomics.notify` notification path.
        __emscripten_thread_mailbox_await(msgData.pthread_ptr);

        if (!initializedJS) {
          // Embind must initialize itself on all threads, as it generates support JS.
          // We only do this once per worker since they get reused
          __embind_initialize_bindings();
          initializedJS = true;
        }

        try {
          invokeEntryPoint(msgData.start_routine, msgData.arg);
        } catch(ex) {
          if (ex != 'unwind') {
            // The pthread "crashed".  Do not call `_emscripten_thread_exit` (which
            // would make this thread joinable).  Instead, re-throw the exception
            // and let the top level handler propagate it back to the main thread.
            throw ex;
          }
        }
      } else if (msgData.target === 'setimmediate') {
        // no-op
      } else if (cmd === 'checkMailbox') {
        if (initializedJS) {
          checkMailbox();
        }
      } else if (cmd) {
        // The received message looks like something that should be handled by this message
        // handler, (since there is a cmd field present), but is not one of the
        // recognized commands:
        err(`worker: received unknown command ${cmd}`);
        err(msgData);
      }
    } catch(ex) {
      err(`worker: onmessage() captured an uncaught exception: ${ex}`);
      if (ex?.stack) err(ex.stack);
      __emscripten_thread_crashed();
      throw ex;
    }
  };

  self.onmessage = handleMessage;

} // ENVIRONMENT_IS_PTHREAD
// end include: runtime_pthread.js


function updateMemoryViews() {
  var b = wasmMemory.buffer;
  Module['HEAP8'] = HEAP8 = new Int8Array(b);
  Module['HEAP16'] = HEAP16 = new Int16Array(b);
  Module['HEAPU8'] = HEAPU8 = new Uint8Array(b);
  Module['HEAPU16'] = HEAPU16 = new Uint16Array(b);
  Module['HEAP32'] = HEAP32 = new Int32Array(b);
  Module['HEAPU32'] = HEAPU32 = new Uint32Array(b);
  Module['HEAPF32'] = HEAPF32 = new Float32Array(b);
  Module['HEAPF64'] = HEAPF64 = new Float64Array(b);
  Module['HEAP64'] = HEAP64 = new BigInt64Array(b);
  Module['HEAPU64'] = HEAPU64 = new BigUint64Array(b);
}

// end include: runtime_shared.js
assert(!Module['STACK_SIZE'], 'STACK_SIZE can no longer be set at runtime.  Use -sSTACK_SIZE at link time')

assert(typeof Int32Array != 'undefined' && typeof Float64Array !== 'undefined' && Int32Array.prototype.subarray != undefined && Int32Array.prototype.set != undefined,
       'JS engine does not provide full typed array support');

// In non-standalone/normal mode, we create the memory here.
// include: runtime_init_memory.js
// Create the wasm memory. (Note: this only applies if IMPORTED_MEMORY is defined)

// check for full engine support (use string 'subarray' to avoid closure compiler confusion)

if (!ENVIRONMENT_IS_PTHREAD) {

  if (Module['wasmMemory']) {
    wasmMemory = Module['wasmMemory'];
  } else
  {
    var INITIAL_MEMORY = Module['INITIAL_MEMORY'] || 134217728;legacyModuleProp('INITIAL_MEMORY', 'INITIAL_MEMORY');

    assert(INITIAL_MEMORY >= 65536, 'INITIAL_MEMORY should be larger than STACK_SIZE, was ' + INITIAL_MEMORY + '! (STACK_SIZE=' + 65536 + ')');
    /** @suppress {checkTypes} */
    wasmMemory = new WebAssembly.Memory({
      'initial': INITIAL_MEMORY / 65536,
      'maximum': INITIAL_MEMORY / 65536,
      'shared': true,
    });
  }

  updateMemoryViews();
}

// end include: runtime_init_memory.js

function preRun() {
  assert(!ENVIRONMENT_IS_PTHREAD); // PThreads reuse the runtime from the main thread.
  if (Module['preRun']) {
    if (typeof Module['preRun'] == 'function') Module['preRun'] = [Module['preRun']];
    while (Module['preRun'].length) {
      addOnPreRun(Module['preRun'].shift());
    }
  }
  callRuntimeCallbacks(onPreRuns);
}

function initRuntime() {
  assert(!runtimeInitialized);
  runtimeInitialized = true;

  if (ENVIRONMENT_IS_WASM_WORKER) return _wasmWorkerInitializeRuntime();

  if (ENVIRONMENT_IS_PTHREAD) return startWorker(Module);

  checkStackCookie();

  

  wasmExports['__wasm_call_ctors']();

  
}

function preMain() {
  checkStackCookie();
  
}

function postRun() {
  checkStackCookie();
  if (ENVIRONMENT_IS_PTHREAD) return; // PThreads reuse the runtime from the main thread.

  if (Module['postRun']) {
    if (typeof Module['postRun'] == 'function') Module['postRun'] = [Module['postRun']];
    while (Module['postRun'].length) {
      addOnPostRun(Module['postRun'].shift());
    }
  }

  callRuntimeCallbacks(onPostRuns);
}

// A counter of dependencies for calling run(). If we need to
// do asynchronous work before running, increment this and
// decrement it. Incrementing must happen in a place like
// Module.preRun (used by emcc to add file preloading).
// Note that you can add dependencies in preRun, even though
// it happens right before run - run will be postponed until
// the dependencies are met.
var runDependencies = 0;
var dependenciesFulfilled = null; // overridden to take different actions when all run dependencies are fulfilled
var runDependencyTracking = {};
var runDependencyWatcher = null;

function getUniqueRunDependency(id) {
  var orig = id;
  while (1) {
    if (!runDependencyTracking[id]) return id;
    id = orig + Math.random();
  }
}

function addRunDependency(id) {
  runDependencies++;

  Module['monitorRunDependencies']?.(runDependencies);

  if (id) {
    assert(!runDependencyTracking[id]);
    runDependencyTracking[id] = 1;
    if (runDependencyWatcher === null && typeof setInterval != 'undefined') {
      // Check for missing dependencies every few seconds
      runDependencyWatcher = setInterval(() => {
        if (ABORT) {
          clearInterval(runDependencyWatcher);
          runDependencyWatcher = null;
          return;
        }
        var shown = false;
        for (var dep in runDependencyTracking) {
          if (!shown) {
            shown = true;
            err('still waiting on run dependencies:');
          }
          err(`dependency: ${dep}`);
        }
        if (shown) {
          err('(end of list)');
        }
      }, 10000);
    }
  } else {
    err('warning: run dependency added without ID');
  }
}

function removeRunDependency(id) {
  runDependencies--;

  Module['monitorRunDependencies']?.(runDependencies);

  if (id) {
    assert(runDependencyTracking[id]);
    delete runDependencyTracking[id];
  } else {
    err('warning: run dependency removed without ID');
  }
  if (runDependencies == 0) {
    if (runDependencyWatcher !== null) {
      clearInterval(runDependencyWatcher);
      runDependencyWatcher = null;
    }
    if (dependenciesFulfilled) {
      var callback = dependenciesFulfilled;
      dependenciesFulfilled = null;
      callback(); // can add another dependenciesFulfilled
    }
  }
}

/** @param {string|number=} what */
function abort(what) {
  Module['onAbort']?.(what);

  what = 'Aborted(' + what + ')';
  // TODO(sbc): Should we remove printing and leave it up to whoever
  // catches the exception?
  err(what);

  ABORT = true;

  // Use a wasm runtime error, because a JS error might be seen as a foreign
  // exception, which means we'd run destructors on it. We need the error to
  // simply make the program stop.
  // FIXME This approach does not work in Wasm EH because it currently does not assume
  // all RuntimeErrors are from traps; it decides whether a RuntimeError is from
  // a trap or not based on a hidden field within the object. So at the moment
  // we don't have a way of throwing a wasm trap from JS. TODO Make a JS API that
  // allows this in the wasm spec.

  // Suppress closure compiler warning here. Closure compiler's builtin extern
  // definition for WebAssembly.RuntimeError claims it takes no arguments even
  // though it can.
  // TODO(https://github.com/google/closure-compiler/pull/3913): Remove if/when upstream closure gets fixed.
  /** @suppress {checkTypes} */
  var e = new WebAssembly.RuntimeError(what);

  readyPromiseReject(e);
  // Throw the error whether or not MODULARIZE is set because abort is used
  // in code paths apart from instantiation where an exception is expected
  // to be thrown when abort is called.
  throw e;
}

function createExportWrapper(name, nargs) {
  return (...args) => {
    assert(runtimeInitialized, `native function \`${name}\` called before runtime initialization`);
    var f = wasmExports[name];
    assert(f, `exported native function \`${name}\` not found`);
    // Only assert for too many arguments. Too few can be valid since the missing arguments will be zero filled.
    assert(args.length <= nargs, `native function \`${name}\` called with ${args.length} args but expects ${nargs}`);
    return f(...args);
  };
}

var wasmBinaryFile;
function findWasmBinary() {
    return locateFile('pd4web.wasm');
}

function getBinarySync(file) {
  if (file == wasmBinaryFile && wasmBinary) {
    return new Uint8Array(wasmBinary);
  }
  if (readBinary) {
    return readBinary(file);
  }
  throw 'both async and sync fetching of the wasm failed';
}

async function getWasmBinary(binaryFile) {
  // If we don't have the binary yet, load it asynchronously using readAsync.
  if (!wasmBinary) {
    // Fetch the binary using readAsync
    try {
      var response = await readAsync(binaryFile);
      return new Uint8Array(response);
    } catch {
      // Fall back to getBinarySync below;
    }
  }

  // Otherwise, getBinarySync should be able to get it synchronously
  return getBinarySync(binaryFile);
}

async function instantiateArrayBuffer(binaryFile, imports) {
  try {
    var binary = await getWasmBinary(binaryFile);
    var instance = await WebAssembly.instantiate(binary, imports);
    return instance;
  } catch (reason) {
    err(`failed to asynchronously prepare wasm: ${reason}`);

    // Warn on some common problems.
    if (isFileURI(wasmBinaryFile)) {
      err(`warning: Loading from a file URI (${wasmBinaryFile}) is not supported in most browsers. See https://emscripten.org/docs/getting_started/FAQ.html#how-do-i-run-a-local-webserver-for-testing-why-does-my-program-stall-in-downloading-or-preparing`);
    }
    abort(reason);
  }
}

async function instantiateAsync(binary, binaryFile, imports) {
  if (!binary && typeof WebAssembly.instantiateStreaming == 'function'
      // Don't use streaming for file:// delivered objects in a webview, fetch them synchronously.
      && !isFileURI(binaryFile)
      // Avoid instantiateStreaming() on Node.js environment for now, as while
      // Node.js v18.1.0 implements it, it does not have a full fetch()
      // implementation yet.
      //
      // Reference:
      //   https://github.com/emscripten-core/emscripten/pull/16917
      && !ENVIRONMENT_IS_NODE
     ) {
    try {
      var response = fetch(binaryFile, { credentials: 'same-origin' });
      var instantiationResult = await WebAssembly.instantiateStreaming(response, imports);
      return instantiationResult;
    } catch (reason) {
      // We expect the most common failure cause to be a bad MIME type for the binary,
      // in which case falling back to ArrayBuffer instantiation should work.
      err(`wasm streaming compile failed: ${reason}`);
      err('falling back to ArrayBuffer instantiation');
      // fall back of instantiateArrayBuffer below
    };
  }
  return instantiateArrayBuffer(binaryFile, imports);
}

function getWasmImports() {
  assignWasmImports();
  // prepare imports
  return {
    'env': wasmImports,
    'wasi_snapshot_preview1': wasmImports,
  }
}

// Create the wasm instance.
// Receives the wasm imports, returns the exports.
async function createWasm() {
  // Load the wasm module and create an instance of using native support in the JS engine.
  // handle a generated wasm instance, receiving its exports and
  // performing other necessary setup
  /** @param {WebAssembly.Module=} module*/
  function receiveInstance(instance, module) {
    wasmExports = instance.exports;

    

    registerTLSInit(wasmExports['_emscripten_tls_init']);

    wasmTable = wasmExports['__indirect_function_table'];
    Module['wasmTable'] = wasmTable;
    assert(wasmTable, 'table not found in wasm exports');

    // We now have the Wasm module loaded up, keep a reference to the compiled module so we can post it to the workers.
    wasmModule = module;
    removeRunDependency('wasm-instantiate');
    return wasmExports;
  }
  // wait for the pthread pool (if any)
  addRunDependency('wasm-instantiate');

  // Prefer streaming instantiation if available.
  // Async compilation can be confusing when an error on the page overwrites Module
  // (for example, if the order of elements is wrong, and the one defining Module is
  // later), so we save Module and check it later.
  var trueModule = Module;
  function receiveInstantiationResult(result) {
    // 'result' is a ResultObject object which has both the module and instance.
    // receiveInstance() will swap in the exports (to Module.asm) so they can be called
    assert(Module === trueModule, 'the Module object should not be replaced during async compilation - perhaps the order of HTML elements is wrong?');
    trueModule = null;
    return receiveInstance(result['instance'], result['module']);
  }

  var info = getWasmImports();

  // User shell pages can write their own Module.instantiateWasm = function(imports, successCallback) callback
  // to manually instantiate the Wasm module themselves. This allows pages to
  // run the instantiation parallel to any other async startup actions they are
  // performing.
  // Also pthreads and wasm workers initialize the wasm instance through this
  // path.
  if (Module['instantiateWasm']) {
    return new Promise((resolve, reject) => {
      try {
        Module['instantiateWasm'](info, (mod, inst) => {
          receiveInstance(mod, inst);
          resolve(mod.exports);
        });
      } catch(e) {
        err(`Module.instantiateWasm callback failed with error: ${e}`);
        reject(e);
      }
    });
  }

  if (ENVIRONMENT_IS_PTHREAD) {
    return new Promise((resolve) => {
      wasmModuleReceived = (module) => {
        // Instantiate from the module posted from the main thread.
        // We can just use sync instantiation in the worker.
        var instance = new WebAssembly.Instance(module, getWasmImports());
        resolve(receiveInstance(instance, module));
      };
    });
  }

  wasmBinaryFile ??= findWasmBinary();

  try {
    var result = await instantiateAsync(wasmBinary, wasmBinaryFile, info);
    var exports = receiveInstantiationResult(result);
    return exports;
  } catch (e) {
    // If instantiation fails, reject the module ready promise.
    readyPromiseReject(e);
    return Promise.reject(e);
  }
}

// === Body ===

var ASM_CONSTS = {
  320300: ($0, $1, $2) => { var width = $0; var height = $1; var zoom = $2; const patchDiv = document.getElementById("Pd4WebPatchDiv"); patchDiv.style.width = (width * zoom) + "px"; patchDiv.style.height = (height * zoom) + "px"; patchDiv.style.marginLeft = "auto"; patchDiv.style.marginRight = "auto"; const canvas = document.getElementById("Pd4WebCanvas"); const value = "0 0 " + width + " " + height; canvas.setAttributeNS(null, "viewBox", value); },  
 320727: () => { setTimeout(function() { Pd4Web._vis(); }, 100); },  
 320779: ($0) => { var layer_id = UTF8ToString($0); var item = document.getElementById(layer_id); if (item == null) { console.log("fill_ellipse: item not found"); return; } const ctx = item.getContext('2d'); ctx.clearRect(0, 0, item.width, item.height); },  
 321018: ($0, $1, $2, $3, $4, $5) => { var layer_id = UTF8ToString($0); var color = UTF8ToString($1); let x = $2; let y = $3; let w = $4; let h = $5; var item = document.getElementById(layer_id); if (item == null) { console.log("fill_ellipse: item not found"); return; } const ctx = item.getContext('2d'); let centerX = x + w / 2; let centerY = y + h / 2; ctx.fillStyle = color; ctx.beginPath(); ctx.ellipse(centerX, centerY, w / 2, h / 2, 0, 0, 2 * Math.PI); ctx.fill(); },  
 321455: ($0, $1, $2, $3, $4, $5, $6) => { var layer_id = UTF8ToString($0); var color = UTF8ToString($1); let x = $2; let y = $3; let w = $4; let h = $5; let line_width = $6; var item = document.getElementById(layer_id); if (item == null) { console.log("stroke_ellipse: item not found"); return; } const ctx = item.getContext('2d'); let centerX = x + w / 2; let centerY = y + h / 2; ctx.strokeStyle = color; ctx.lineWidth = line_width; ctx.beginPath(); ctx.ellipse(centerX, centerY, w / 2, h / 2, 0, 0, 2 * Math.PI); ctx.stroke(); },  
 321947: ($0, $1, $2, $3, $4, $5) => { var color = UTF8ToString($0); var layer_id = UTF8ToString($1); var item = document.getElementById(layer_id); if (item == null) { console.log("fill_rect: item not found"); return; } const ctx = item.getContext('2d'); ctx.fillStyle = color; ctx.fillRect($2, $3, $4, $5); },  
 322220: ($0, $1, $2, $3, $4, $5, $6) => { var color = UTF8ToString($0); var layer_id = UTF8ToString($1); var item = document.getElementById(layer_id); if (item == null) { console.log("stroke_rect: item not found"); return; } const ctx = item.getContext('2d'); ctx.strokeStyle = color; ctx.lineWidth = $6; ctx.strokeRect($2, $3, $4, $5); ctx.lineJoin = "bevel"; },  
 322543: ($0, $1, $2, $3, $4, $5, $6) => { var color = UTF8ToString($0); var layer_id = UTF8ToString($1); var item = document.getElementById(layer_id); if (item == null) { console.log("fill_rounded_rect: item not found"); return; } const ctx = item.getContext('2d'); ctx.fillStyle = color; ctx.beginPath(); ctx.moveTo($2 + $6, $3); ctx.lineTo($2 + $4 - $6, $3); ctx.arcTo($2 + $4, $3, $2 + $4, $3 + $6, $6); ctx.lineTo($2 + $4, $3 + $5 - $6); ctx.arcTo($2 + $4, $3 + $5, $2 + $4 - $6, $3 + $5, $6); ctx.lineTo($2 + $6, $3 + $5); ctx.arcTo($2, $3 + $5, $2, $3 + $5 - $6, $6); ctx.lineTo($2, $3 + $6); ctx.arcTo($2, $3, $2 + $6, $3, $6); ctx.closePath(); ctx.fill(); },  
 323169: ($0, $1, $2, $3, $4, $5, $6) => { var color = UTF8ToString($0); var layer_id = UTF8ToString($1); var item = document.getElementById(layer_id); if (item == null) { console.log("stroke_rounded_rect: item not found"); return; } const ctx = item.getContext('2d'); ctx.strokeStyle = color; ctx.beginPath(); ctx.moveTo($2 + $6, $3); ctx.lineTo($2 + $4 - $6, $3); ctx.arcTo($2 + $4, $3, $2 + $4, $3 + $6, $6); ctx.lineTo($2 + $4, $3 + $5 - $6); ctx.arcTo($2 + $4, $3 + $5, $2 + $4 - $6, $3 + $5, $6); ctx.lineTo($2 + $6, $3 + $5); ctx.arcTo($2, $3 + $5, $2, $3 + $5 - $6, $6); ctx.lineTo($2, $3 + $6); ctx.arcTo($2, $3, $2 + $6, $3, $6); ctx.closePath(); ctx.stroke(); },  
 323801: ($0, $1, $2, $3, $4, $5, $6) => { var color = UTF8ToString($0); var layer_id = UTF8ToString($1); var item = document.getElementById(layer_id); if (item == null) { console.log("draw_line: item not found"); return; } const ctx = item.getContext('2d'); ctx.strokeStyle = color; ctx.lineWidth = $6; ctx.lineJoin = "bevel"; ctx.beginPath(); ctx.moveTo($2, $3); ctx.lineTo($4, $5); ctx.stroke(); },  
 324161: ($0, $1, $2, $3, $4, $5, $6) => { var color = UTF8ToString($0); var layer_id = UTF8ToString($1); var text = UTF8ToString($2); var x = $3; var y = $4; var maxWidth = $5; var fontSize = $6; var item = document.getElementById(layer_id); if (!item) { console.log("draw_text: canvas not found"); return; } const ctx = item.getContext('2d'); ctx.fillStyle = color; ctx.font = fontSize + "px Arial"; ctx.textAlign = "left"; ctx.textBaseline = "top"; function pdTextWrap(context, text, x, y, maxWidth) { var lines = []; var currentLine = []; var words = text.split(' '); var lineHeight = fontSize * 1.2; var currentY = y; words.forEach(word => { var testLine = currentLine.concat(word).join(' '); var metrics = context.measureText(testLine); if (metrics.width <= maxWidth) { currentLine.push(word); } else { if (currentLine.length === 0) { var splitPoint = 0; for (var i = 1; i <= word.length; i++) { if (context.measureText(word.substr(0, i)).width <= maxWidth) { splitPoint = i; } } if (splitPoint > 0) { currentLine.push(word.substr(0, splitPoint)); words.unshift(word.substr(splitPoint)); } } lines.push(currentLine.join(' ')); currentLine = [word]; } }); lines.push(currentLine.join(' ')); lines.forEach(line => { context.fillText(line, x, currentY); currentY += lineHeight; }); } if (maxWidth > 0) { pdTextWrap(ctx, text, x, y, maxWidth); } else { ctx.fillText(text, x, y); } },  
 325505: ($0, $1, $2) => { var layer_id = UTF8ToString($0); var x = $1; var y = $2; var item = document.getElementById(layer_id); if (item == null) { console.log("stroke_path: item not found"); return; } const ctx = item.getContext('2d'); ctx.beginPath(); ctx.moveTo(x, y); },  
 325756: ($0, $1, $2) => { var layer_id = UTF8ToString($0); var x = $1; var y = $2; var item = document.getElementById(layer_id); if (item == null) { console.log("stroke_path: item not found"); return; } const ctx = item.getContext('2d'); ctx.lineTo(x, y); },  
 325990: ($0, $1, $2) => { var layer_id = UTF8ToString($0); var item = document.getElementById(layer_id); if (item == null) { console.log("stroke_path: item not found"); return; } const ctx = item.getContext('2d'); ctx.strokeStyle = UTF8ToString($1); ctx.lineWidth = $2; ctx.stroke(); ctx.closePath(); },  
 326269: ($0, $1, $2) => { var layer_id = UTF8ToString($0); var x = $1; var y = $2; var item = document.getElementById(layer_id); if (item == null) { console.log("stroke_path: item not found"); return; } const ctx = item.getContext('2d'); ctx.beginPath(); ctx.moveTo(x, y); },  
 326520: ($0, $1, $2) => { var layer_id = UTF8ToString($0); var x = $1; var y = $2; var item = document.getElementById(layer_id); if (item == null) { console.log("stroke_path: item not found"); return; } const ctx = item.getContext('2d'); ctx.lineTo(x, y); },  
 326754: ($0, $1) => { var layer_id = UTF8ToString($0); var item = document.getElementById(layer_id); if (item == null) { console.log("stroke_path: item not found"); return; } const ctx = item.getContext('2d'); ctx.fillStyle = UTF8ToString($1); ctx.fill(); ctx.closePath(); },  
 327009: ($0, $1) => { var color = UTF8ToString($0); var layer_id = UTF8ToString($1); var item = document.getElementById(layer_id); if (item == null) { console.log("fill_all: item not found"); return; } const ctx = item.getContext('2d'); ctx.fillStyle = color; ctx.fillRect(1, 1, item.width - 2, item.height - 2); },  
 327304: () => { alert("Failed to malloc memory"); },  
 327342: () => { alert("Failed to malloc memory"); },  
 327380: () => { alert("Failed to malloc memory"); },  
 327418: () => { alert("Failed to malloc memory"); },  
 327456: () => { alert("Failed to malloc memory"); },  
 327494: () => { alert("Failed to malloc memory"); },  
 327532: () => { alert("Failed to malloc memory"); },  
 327570: () => { alert("Failed to malloc memory"); },  
 327608: ($0, $1, $2, $3, $4, $5) => { var color = UTF8ToString($0); var objpointer = UTF8ToString($1); var svgText = UTF8ToString($2); var x = $3; var y = $4; var scale = $5; var canvas = document.getElementById(objpointer); if (!canvas) { console.error("draw_svg: Canvas not found"); return; } var ctx = canvas.getContext('2d'); var svgBlob = new Blob([svgText], { type: 'image/svg+xml;charset=utf-8' }); var url = URL.createObjectURL(svgBlob); var img = new Image(); img.onload = function() { ctx.save(); ctx.translate(x, y); ctx.scale(scale, scale); ctx.globalCompositeOperation = 'source-atop'; ctx.drawImage(img, 0, 0); ctx.fillStyle = color; ctx.globalAlpha = 0.99; ctx.fillRect(0, 0, img.width, img.height); ctx.restore(); URL.revokeObjectURL(url); }; img.onerror = function() { console.error("Error loading SVG image"); URL.revokeObjectURL(url); }; img.src = url; },  
 328446: () => { alert("Failed to malloc memory"); },  
 328484: () => { alert("Failed to malloc memory"); },  
 328522: () => { alert("Failed to malloc memory"); },  
 328560: () => { alert("Failed to malloc memory"); },  
 328598: ($0, $1, $2, $3, $4, $5) => { let layer_id = UTF8ToString($0); let x_pos = $1; let y_pos = $2; let width = $3; let height = $4; let zoom = $5; const container = document.getElementById("Pd4WebPatchDiv"); var item = document.getElementById(layer_id); if (document.getElementById(layer_id) == null) { item = document.createElement("canvas"); container.appendChild(item); } else { item = document.getElementById(layer_id); } const containerX = container.getBoundingClientRect().left; const containerY = container.getBoundingClientRect().top; item.id = layer_id; item.width = width * zoom; item.height = height * zoom; item.style.position = "absolute"; item.style.left = containerX + (x_pos * zoom) + "px"; item.style.top = containerY + (y_pos * zoom) + "px"; const ctx = item.getContext('2d'); ctx.strokeRect(0, 0, width * zoom, height * zoom); },  
 329414: () => { alert("Failed to malloc memory"); },  
 329452: () => { alert("Failed to malloc memory"); },  
 329490: () => { alert("Failed to malloc memory"); }
};
function _JS_post2(msg) { console.log(UTF8ToString(msg)); }
function _JS_addSoundToggle() { let soundSwitch = document.getElementById("Pd4WebAudioSwitch"); if (soundSwitch == null || typeof Pd4Web === "undefined") { return; } soundSwitch.addEventListener("click", function () { if (Pd4Web){ Pd4Web.soundToggle(); } }); }
function _JS_pd4webCppClass(Pd4Web) { console.log("Received Pd4Web pointer:", Pd4Web); }
function _JS_setIcon(icon,animation) { let jsIcon = UTF8ToString(icon); let jsAnimation = UTF8ToString(animation); function tryCallSetSoundIcon() { if (typeof setSoundIcon === 'function') { setSoundIcon(jsIcon, jsAnimation); } else { setTimeout(() => { if (typeof setSoundIcon === 'function') { setSoundIcon(jsIcon, jsAnimation); } }, 200); } } setTimeout(() => tryCallSetSoundIcon(), 200); }
function _JS_sendList() { if (typeof Pd4Web.GuiReceivers === "undefined") { Pd4Web.GuiReceivers = {}; } Pd4Web.sendList = function (r, vec) { const vecLength = vec.length; var ok = Pd4Web._startMessage(r, vecLength); if (!ok) { console.error('Failed to start message'); return; } for (let i = 0; i < vecLength; i++) { if (typeof vec[i] === 'string') { Pd4Web._addSymbol(r, vec[i]); } else if (typeof vec[i] === 'number') { Pd4Web._addFloat(r, vec[i]); } else{ console.error('Invalid type'); } } Pd4Web._finishMessage(r); }; }
function _JS_onReceived() { Pd4Web.onBangReceived = function(receiver, myFunc) { if (typeof Pd4Web._userBangFunc === 'undefined') { Pd4Web._userBangFunc = {}; } const paramCount = myFunc.length; if (paramCount !== 0) { console.error('Invalid number of arguments for function, expected 0 arguments'); return; } Pd4Web.bindReceiver(receiver); Pd4Web._userBangFunc[receiver] = myFunc; }; Pd4Web.onFloatReceived = function(receiver, myFunc) { if (typeof Pd4Web._userFloatFunc === 'undefined') { Pd4Web._userFloatFunc = {}; } const paramCount = myFunc.length; if (paramCount !== 1) { console.error('Invalid number of arguments for function, expected 1, just the float received'); return; } Pd4Web.bindReceiver(receiver); Pd4Web._userFloatFunc[receiver] = myFunc; }; Pd4Web.onSymbolReceived = function(receiver, myFunc) { if (typeof Pd4Web._userSymbolFunc === 'undefined') { Pd4Web._userSymbolFunc = {}; } const paramCount = myFunc.length; if (paramCount !== 1) { console.error('Invalid number of arguments for function. Required 1, just the symbol (aka string) received'); return; } Pd4Web.bindReceiver(receiver); Pd4Web._userSymbolFunc[receiver] = myFunc; }; Pd4Web.onListReceived = function(receiver, myFunc) { if (typeof Pd4Web._userListFunc === 'undefined') { Pd4Web._userListFunc = {}; } const paramCount = myFunc.length; if (paramCount !== 1) { console.error('Invalid number of arguments for function. Required 1, just the list received'); return; } Pd4Web.bindReceiver(receiver); Pd4Web._userListFunc[receiver] = myFunc; }; }
function _JS_loadGui(AutoTheming,Zoom) { if (document.getElementById("pd4web-gui") != null){ return; } let scripts = document.getElementsByTagName('script'); let pd4webPath = null; for (let script of scripts) { if (script.src && script.src.includes('pd4web.js')) { pd4webPath = script.src.substring(0, script.src.lastIndexOf('/') + 1); break; } } var script = document.createElement('script'); script.type = "text/javascript"; script.src = pd4webPath + "pd4web.gui.js"; script.id = "pd4web-gui"; script.onload = function() { Pd4Web.Zoom = Zoom; Pd4WebInitGui("index.pd"); }; script.onerror = function() { console.warn("GUI file not found."); }; document.head.appendChild(script); }
function _JS_resizeCanvas(x,y,zoom) { var width = x; var height = y; const patchDiv = document.getElementById("Pd4WebPatchDiv"); patchDiv.style.width = (width * zoom) + "px"; patchDiv.style.height = (height * zoom) + "px"; patchDiv.style.marginLeft = "auto"; patchDiv.style.marginRight = "auto"; }
function _JS_loadStyle() { if (document.getElementById("pd4web-style") != null){ return; } let scripts = document.getElementsByTagName('script'); let pd4webPath = null; for (let script of scripts) { if (script.src && script.src.includes('pd4web.js')) { pd4webPath = script.src.substring(0, script.src.lastIndexOf('/') + 1); break; } } var link = document.createElement('link'); link.rel = "stylesheet"; link.type = "text/css"; link.href = pd4webPath + "pd4web.style.css"; link.id = "pd4web-style"; document.head.appendChild(link); link.onerror = function() { console.warn("CSS file not found."); }; }
function _JS_setTitle(projectName) { let title = UTF8ToString(projectName); document.title = title; }
function _JS_alert(msg) { alert(UTF8ToString(msg)); }
function _JS_addAlertOnError() { window.addEventListener('error', function(event) { console.log(event.filename); }); }
function _JS_post(msg) { console.log(UTF8ToString(msg)); }
function _JS_addMessagePort(audioWorkletNode) { Pd4WebAudioWorkletNode = emscriptenGetAudioObject(audioWorkletNode); Pd4WebAudioWorkletNode.port.onmessage = function(event) { console.log(event); }; }
function _JS_getMicAccess(audioContext,audioWorkletNode,nInCh) { Pd4WebAudioContext = emscriptenGetAudioObject(audioContext); Pd4WebAudioWorkletNode = emscriptenGetAudioObject(audioWorkletNode); Pd4WebAudioWorkletNode.port.onmessage = function(event) { console.log("Message from worklet:", event.data); }; async function _GetMicAccess(stream) { try { const SourceNode = Pd4WebAudioContext.createMediaStreamSource(stream); SourceNode.connect(Pd4WebAudioWorkletNode); Pd4WebAudioWorkletNode.connect(Pd4WebAudioContext.destination); } catch (err) { alert(err); } } if (nInCh > 0) { navigator.mediaDevices .getUserMedia({ video: false, audio: { echoCancellation: false, noiseSuppression: false, autoGainControl: false, }, }) .then((stream) => _GetMicAccess(stream)); } else { Pd4WebAudioWorkletNode.connect(Pd4WebAudioContext.destination); } }
function _JS_suspendAudioWorkLet(audioContext) { Pd4WebAudioContext = emscriptenGetAudioObject(audioContext); Pd4WebAudioContext.suspend(); }
function _JS_loadMidi() { if (document.getElementById("pd4web-midi") != null){ return; } let scripts = document.getElementsByTagName('script'); let pd4webPath = null; for (let script of scripts) { if (script.src && script.src.includes('pd4web.js')) { pd4webPath = script.src.substring(0, script.src.lastIndexOf('/') + 1); break; } } var script = document.createElement('script'); script.type = "text/javascript"; script.src = pd4webPath + "pd4web.midi.js"; script.id = "pd4web-midi"; script.onload = function() { if (typeof WebMidi != "object") { console.error("Midi: failed to find the 'WebMidi' object"); return; } WebMidi.enable(function (err) { if (err) { console.error("Midi: failed to enable midi", err); return; } WebMidi.inputs.forEach(input => { console.log(input.channels); input.channels[1].addListener("noteon", function(e) { if (typeof e.channel === 'undefined') { Pd4Web.noteOn(1, e.note.number, e.rawVelocity); } else{ Pd4Web.noteOn(e.channel, e.note.number, e.rawVelocity); } }); input.channels[1].addListener("noteoff", function(e) { if (typeof e.channel === 'undefined') { Pd4Web.noteOn(1, e.note.number, 0); } else{ Pd4Web.noteOn(e.channel, e.note.number, 0); } }); }); }, false); }; document.head.appendChild(script); }
function _JS_receiveBang(r) { var source = UTF8ToString(r); if (source in Pd4Web.GuiReceivers) { for (const data of Pd4Web.GuiReceivers[source]) { switch (data.type) { case "bng": GuiBngUpdateCircle(data); break; case "tgl": data.value = data.value ? 0 : data.default_value; GuiTglUpdateCross(data); break; case "vsl": case "hsl": GuiSliderBang(data); break; case "vradio": case "hradio": Pd4Web.sendFloat(data.send, data.value); break; } } } else{ if (typeof Pd4Web._userBangFunc === 'undefined'){ alert("Turn audio on first"); return; } let bangFunc = Pd4Web._userBangFunc[source]; if (typeof bangFunc === 'function') { bangFunc(); } } }
function _JS_receiveFloat(r,f) { var source = UTF8ToString(r); if (source in Pd4Web.GuiReceivers) { for (const data of Pd4Web.GuiReceivers[source]) { switch (data.type) { case "bng": GuiBngUpdateCircle(data); break; case "tgl": data.value = data.value ? 0 : data.default_value; GuiTglUpdateCross(data); break; case "vsl": case "hsl": GuiSliderSet(data, f); GuiSliderBang(data); break; case "nbx": GuiNbxUpdateNumber(data, f); break; case "vradio": case "hradio": data.value = Math.min(Math.max(Math.floor(f), 0), data.number - 1); GuiRadioUpdateButton(data); Pd4Web.sendFloat(data.send, data.value); break; case "vu": data.value = f; GuiVuUpdateGain(data); break; } } } else{ if (typeof Pd4Web._userFloatFunc === 'undefined'){ alert("Turn audio on first"); return; } let floatFunc = Pd4Web._userFloatFunc[source]; if (typeof floatFunc === 'function') { floatFunc(f); } } }
function _JS_receiveSymbol(r,s) { var source = UTF8ToString(r); var symbol = UTF8ToString(s); if (source in Pd4Web.GuiReceivers) { for (const data of Pd4Web.GuiReceivers[source]) { switch (data.type) { case "bng": GuiBngUpdateCircle(data); break; } } } else{ if (typeof Pd4Web._userSymbolFunc === 'undefined'){ alert("Turn audio on first"); return; } let symbolFunc = Pd4Web._userSymbolFunc[source]; if (typeof symbolFunc === 'function') { symbolFunc(symbol); } } }
function _JS_receiveList(r) { var source = UTF8ToString(r); if (source in Pd4Web.GuiReceivers) { return; } else{ if (typeof Pd4Web._userListFunc === 'undefined'){ alert("Turn audio on first"); return; } let listFunc = Pd4Web._userListFunc[source]; const listSize = Pd4Web._getReceivedListSize(source); var pdList = []; for (let i = 0; i < listSize; i++) { let type = Pd4Web._getItemFromListType(source, i); if (type === "float") { pdList.push(Pd4Web._getItemFromListFloat(source, i)); } else if (type === "symbol") { pdList.push(Pd4Web._getItemFromListSymbol(source, i)); } else{ console.error("Invalid type"); } } if (typeof listFunc === 'function') { listFunc(pdList); } } }
function _JS_domIsDefined() { console.log(Pd4Web); }
function _JS_createTgl(p,x_pos,y_pos,size,zoom,id,bg) { var objpointer = UTF8ToString(p); var background = UTF8ToString(bg); let groupProps = { id: "tgl_" + id, class: "border clickable" }; var groupObj = CreateItem("g", groupProps); let rectProps = { x: x_pos, y: y_pos, rx: 2, ry: 2, width: size, height: size, fill: background }; var rectObj = CreateItem("rect", rectProps); let line1Props = { x1: x_pos + 2, y1: y_pos + 2, x2: x_pos + size - 2, y2: y_pos + size - 2, stroke: "none", "stroke-width": 2 }; var line1Obj = CreateItem("line", line1Props); let line2Props = { x1: x_pos + 2, y1: y_pos + size - 2, x2: x_pos + size - 2, y2: y_pos + 2, stroke: "none", "stroke-width": 2 }; var line2Obj = CreateItem("line", line2Props); groupObj.appendChild(rectObj); groupObj.appendChild(line1Obj); groupObj.appendChild(line2Obj); const svgElement = document.getElementById("Pd4WebCanvas"); svgElement.appendChild(groupObj); let crossVisible = false; groupObj.addEventListener("click", function(e) { const svgBox = svgElement.getBoundingClientRect(); const x = Math.round((e.clientX - svgBox.x) / zoom); const y = Math.round((e.clientY - svgBox.y) / zoom); Pd4Web._objclick(objpointer, x, y); crossVisible = !crossVisible; if (crossVisible) { line1Obj.setAttribute("stroke", "black"); line2Obj.setAttribute("stroke", "black"); } else { line1Obj.setAttribute("stroke", "none"); line2Obj.setAttribute("stroke", "none"); } }); }
function _JS_createBng(p,x_pos,y_pos,size,id,bg) { var objpointer = UTF8ToString(p); var background = UTF8ToString(bg); let rect = { id: "bng_" + id, x: x_pos, y: y_pos, rx: 2, ry: 2, width: size, height: size, fill: background, class: "border clickable", }; var guiObj = CreateItem("rect", rect); guiObj.addEventListener("click", function(e) { console.log("Rectangle " + objpointer + " clicked!"); console.log(e.clientX, e.clientY); Pd4Web._objclick(objpointer, e.clientX, e.clientY); }); }
function _JS_receiveMessage(r) { var source = UTF8ToString(r); if (typeof Pd4Web._getReceivedListSize === 'undefined'){ alert("Turn audio on first"); return; } const listSize = Pd4Web._getReceivedListSize(source); var pdList = []; for (let i = 0; i < listSize; i++) { let type = Pd4Web._getItemFromListType(source, i); if (type === "float") { pdList.push(Pd4Web._getItemFromListFloat(source, i)); } else if (type === "symbol") { pdList.push(Pd4Web._getItemFromListSymbol(source, i)); } else{ console.error("Invalid type"); } } if (source in Pd4Web.GuiReceivers) { let sel = Pd4Web._getMessageSelector(source); MessageListener(source, sel, pdList); return; } else{ console.error("Not implemented"); } }

// end include: preamble.js


  class ExitStatus {
      name = 'ExitStatus';
      constructor(status) {
        this.message = `Program terminated with exit(${status})`;
        this.status = status;
      }
    }

  
  var terminateWorker = (worker) => {
      worker.terminate();
      // terminate() can be asynchronous, so in theory the worker can continue
      // to run for some amount of time after termination.  However from our POV
      // the worker now dead and we don't want to hear from it again, so we stub
      // out its message handler here.  This avoids having to check in each of
      // the onmessage handlers if the message was coming from valid worker.
      worker.onmessage = (e) => {
        var cmd = e['data'].cmd;
        err(`received "${cmd}" command from terminated worker: ${worker.workerID}`);
      };
    };
  
  var cleanupThread = (pthread_ptr) => {
      assert(!ENVIRONMENT_IS_PTHREAD, 'Internal Error! cleanupThread() can only ever be called from main application thread!');
      assert(pthread_ptr, 'Internal Error! Null pthread_ptr in cleanupThread!');
      var worker = PThread.pthreads[pthread_ptr];
      assert(worker);
      PThread.returnWorkerToPool(worker);
    };
  
  var callRuntimeCallbacks = (callbacks) => {
      while (callbacks.length > 0) {
        // Pass the module as the first argument.
        callbacks.shift()(Module);
      }
    };
  var onPreRuns = [];
  var addOnPreRun = (cb) => onPreRuns.unshift(cb);
  
  var spawnThread = (threadParams) => {
      assert(!ENVIRONMENT_IS_PTHREAD, 'Internal Error! spawnThread() can only ever be called from main application thread!');
      assert(threadParams.pthread_ptr, 'Internal error, no pthread ptr!');
  
      var worker = PThread.getNewWorker();
      if (!worker) {
        // No available workers in the PThread pool.
        return 6;
      }
      assert(!worker.pthread_ptr, 'Internal error!');
  
      PThread.runningWorkers.push(worker);
  
      // Add to pthreads map
      PThread.pthreads[threadParams.pthread_ptr] = worker;
  
      worker.pthread_ptr = threadParams.pthread_ptr;
      var msg = {
          cmd: 'run',
          start_routine: threadParams.startRoutine,
          arg: threadParams.arg,
          pthread_ptr: threadParams.pthread_ptr,
      };
      if (ENVIRONMENT_IS_NODE) {
        // Mark worker as weakly referenced once we start executing a pthread,
        // so that its existence does not prevent Node.js from exiting.  This
        // has no effect if the worker is already weakly referenced (e.g. if
        // this worker was previously idle/unused).
        worker.unref();
      }
      // Ask the worker to start executing its pthread entry point function.
      worker.postMessage(msg, threadParams.transferList);
      return 0;
    };
  
  
  
  var runtimeKeepaliveCounter = 0;
  var keepRuntimeAlive = () => noExitRuntime || runtimeKeepaliveCounter > 0;
  
  var stackSave = () => _emscripten_stack_get_current();
  
  var stackRestore = (val) => __emscripten_stack_restore(val);
  
  var stackAlloc = (sz) => __emscripten_stack_alloc(sz);
  
  
  var INT53_MAX = 9007199254740992;
  
  var INT53_MIN = -9007199254740992;
  var bigintToI53Checked = (num) => (num < INT53_MIN || num > INT53_MAX) ? NaN : Number(num);
  
  /** @type{function(number, (number|boolean), ...number)} */
  var proxyToMainThread = (funcIndex, emAsmAddr, sync, ...callArgs) => {
      // EM_ASM proxying is done by passing a pointer to the address of the EM_ASM
      // content as `emAsmAddr`.  JS library proxying is done by passing an index
      // into `proxiedJSCallArgs` as `funcIndex`. If `emAsmAddr` is non-zero then
      // `funcIndex` will be ignored.
      // Additional arguments are passed after the first three are the actual
      // function arguments.
      // The serialization buffer contains the number of call params, and then
      // all the args here.
      // We also pass 'sync' to C separately, since C needs to look at it.
      // Allocate a buffer, which will be copied by the C code.
      //
      // First passed parameter specifies the number of arguments to the function.
      // When BigInt support is enabled, we must handle types in a more complex
      // way, detecting at runtime if a value is a BigInt or not (as we have no
      // type info here). To do that, add a "prefix" before each value that
      // indicates if it is a BigInt, which effectively doubles the number of
      // values we serialize for proxying. TODO: pack this?
      var serializedNumCallArgs = callArgs.length * 2;
      var sp = stackSave();
      var args = stackAlloc(serializedNumCallArgs * 8);
      var b = ((args)>>3);
      for (var i = 0; i < callArgs.length; i++) {
        var arg = callArgs[i];
        if (typeof arg == 'bigint') {
          // The prefix is non-zero to indicate a bigint.
          HEAP64[b + 2*i] = 1n;
          HEAP64[b + 2*i + 1] = arg;
        } else {
          // The prefix is zero to indicate a JS Number.
          HEAP64[b + 2*i] = 0n;
          HEAPF64[b + 2*i + 1] = arg;
        }
      }
      var rtn = __emscripten_run_on_main_thread_js(funcIndex, emAsmAddr, serializedNumCallArgs, args, sync);
      stackRestore(sp);
      return rtn;
    };
  
  function _proc_exit(code) {
  if (ENVIRONMENT_IS_PTHREAD)
    return proxyToMainThread(0, 0, 1, code);
  
      EXITSTATUS = code;
      if (!keepRuntimeAlive()) {
        PThread.terminateAllThreads();
        Module['onExit']?.(code);
        ABORT = true;
      }
      quit_(code, new ExitStatus(code));
    
  }
  
  
  
  
  var handleException = (e) => {
      // Certain exception types we do not treat as errors since they are used for
      // internal control flow.
      // 1. ExitStatus, which is thrown by exit()
      // 2. "unwind", which is thrown by emscripten_unwind_to_js_event_loop() and others
      //    that wish to return to JS event loop.
      if (e instanceof ExitStatus || e == 'unwind') {
        return EXITSTATUS;
      }
      checkStackCookie();
      if (e instanceof WebAssembly.RuntimeError) {
        if (_emscripten_stack_get_current() <= 0) {
          err('Stack overflow detected.  You can try increasing -sSTACK_SIZE (currently set to 65536)');
        }
      }
      quit_(1, e);
    };
  
  
  
  function exitOnMainThread(returnCode) {
  if (ENVIRONMENT_IS_PTHREAD)
    return proxyToMainThread(1, 0, 0, returnCode);
  
      _exit(returnCode);
    
  }
  
  
  /** @suppress {duplicate } */
  /** @param {boolean|number=} implicit */
  var exitJS = (status, implicit) => {
      EXITSTATUS = status;
  
      checkUnflushedContent();
  
      if (ENVIRONMENT_IS_PTHREAD) {
        // implicit exit can never happen on a pthread
        assert(!implicit);
        // When running in a pthread we propagate the exit back to the main thread
        // where it can decide if the whole process should be shut down or not.
        // The pthread may have decided not to exit its own runtime, for example
        // because it runs a main loop, but that doesn't affect the main thread.
        exitOnMainThread(status);
        throw 'unwind';
      }
  
      // if exit() was called explicitly, warn the user if the runtime isn't actually being shut down
      if (keepRuntimeAlive() && !implicit) {
        var msg = `program exited (with status: ${status}), but keepRuntimeAlive() is set (counter=${runtimeKeepaliveCounter}) due to an async operation, so halting execution but not exiting the runtime or preventing further async execution (you can use emscripten_force_exit, if you want to force a true shutdown)`;
        readyPromiseReject(msg);
        err(msg);
      }
  
      _proc_exit(status);
    };
  var _exit = exitJS;
  
  var ptrToString = (ptr) => {
      assert(typeof ptr === 'number');
      // With CAN_ADDRESS_2GB or MEMORY64, pointers are already unsigned.
      ptr >>>= 0;
      return '0x' + ptr.toString(16).padStart(8, '0');
    };
  
  
  var PThread = {
  unusedWorkers:[],
  runningWorkers:[],
  tlsInitFunctions:[],
  pthreads:{
  },
  nextWorkerID:1,
  debugInit() {
        function pthreadLogPrefix() {
          var t = 0;
          if (runtimeInitialized && typeof _pthread_self != 'undefined'
          ) {
            t = _pthread_self();
          }
          return `w:${workerID},t:${ptrToString(t)}: `;
        }
  
        // Prefix all err()/dbg() messages with the calling thread ID.
        var origDbg = dbg;
        dbg = (...args) => origDbg(pthreadLogPrefix() + args.join(' '));
      },
  init() {
        PThread.debugInit();
        if ((!(ENVIRONMENT_IS_PTHREAD||ENVIRONMENT_IS_WASM_WORKER))) {
          PThread.initMainThread();
        }
      },
  initMainThread() {
        var pthreadPoolSize = 4;
        // Start loading up the Worker pool, if requested.
        while (pthreadPoolSize--) {
          PThread.allocateUnusedWorker();
        }
        // MINIMAL_RUNTIME takes care of calling loadWasmModuleToAllWorkers
        // in postamble_minimal.js
        addOnPreRun(() => {
          addRunDependency('loading-workers')
          PThread.loadWasmModuleToAllWorkers(() => removeRunDependency('loading-workers'));
        });
      },
  terminateAllThreads:() => {
        assert(!ENVIRONMENT_IS_PTHREAD, 'Internal Error! terminateAllThreads() can only ever be called from main application thread!');
        // Attempt to kill all workers.  Sadly (at least on the web) there is no
        // way to terminate a worker synchronously, or to be notified when a
        // worker in actually terminated.  This means there is some risk that
        // pthreads will continue to be executing after `worker.terminate` has
        // returned.  For this reason, we don't call `returnWorkerToPool` here or
        // free the underlying pthread data structures.
        for (var worker of PThread.runningWorkers) {
          terminateWorker(worker);
        }
        for (var worker of PThread.unusedWorkers) {
          terminateWorker(worker);
        }
        PThread.unusedWorkers = [];
        PThread.runningWorkers = [];
        PThread.pthreads = {};
      },
  returnWorkerToPool:(worker) => {
        // We don't want to run main thread queued calls here, since we are doing
        // some operations that leave the worker queue in an invalid state until
        // we are completely done (it would be bad if free() ends up calling a
        // queued pthread_create which looks at the global data structures we are
        // modifying). To achieve that, defer the free() til the very end, when
        // we are all done.
        var pthread_ptr = worker.pthread_ptr;
        delete PThread.pthreads[pthread_ptr];
        // Note: worker is intentionally not terminated so the pool can
        // dynamically grow.
        PThread.unusedWorkers.push(worker);
        PThread.runningWorkers.splice(PThread.runningWorkers.indexOf(worker), 1);
        // Not a running Worker anymore
        // Detach the worker from the pthread object, and return it to the
        // worker pool as an unused worker.
        worker.pthread_ptr = 0;
  
        // Finally, free the underlying (and now-unused) pthread structure in
        // linear memory.
        __emscripten_thread_free_data(pthread_ptr);
      },
  receiveObjectTransfer(data) {
      },
  threadInitTLS() {
        // Call thread init functions (these are the _emscripten_tls_init for each
        // module loaded.
        PThread.tlsInitFunctions.forEach((f) => f());
      },
  loadWasmModuleToWorker:(worker) => new Promise((onFinishedLoading) => {
        worker.onmessage = (e) => {
          var d = e['data'];
          var cmd = d.cmd;
  
          // If this message is intended to a recipient that is not the main
          // thread, forward it to the target thread.
          if (d.targetThread && d.targetThread != _pthread_self()) {
            var targetWorker = PThread.pthreads[d.targetThread];
            if (targetWorker) {
              targetWorker.postMessage(d, d.transferList);
            } else {
              err(`Internal error! Worker sent a message "${cmd}" to target pthread ${d.targetThread}, but that thread no longer exists!`);
            }
            return;
          }
  
          if (cmd === 'checkMailbox') {
            checkMailbox();
          } else if (cmd === 'spawnThread') {
            spawnThread(d);
          } else if (cmd === 'cleanupThread') {
            cleanupThread(d.thread);
          } else if (cmd === 'loaded') {
            worker.loaded = true;
            // Check that this worker doesn't have an associated pthread.
            if (ENVIRONMENT_IS_NODE && !worker.pthread_ptr) {
              // Once worker is loaded & idle, mark it as weakly referenced,
              // so that mere existence of a Worker in the pool does not prevent
              // Node.js from exiting the app.
              worker.unref();
            }
            onFinishedLoading(worker);
          } else if (cmd === 'alert') {
            alert(`Thread ${d.threadId}: ${d.text}`);
          } else if (d.target === 'setimmediate') {
            // Worker wants to postMessage() to itself to implement setImmediate()
            // emulation.
            worker.postMessage(d);
          } else if (cmd === 'callHandler') {
            Module[d.handler](...d.args);
          } else if (cmd) {
            // The received message looks like something that should be handled by this message
            // handler, (since there is a e.data.cmd field present), but is not one of the
            // recognized commands:
            err(`worker sent an unknown command ${cmd}`);
          }
        };
  
        worker.onerror = (e) => {
          var message = 'worker sent an error!';
          if (worker.pthread_ptr) {
            message = `Pthread ${ptrToString(worker.pthread_ptr)} sent an error!`;
          }
          err(`${message} ${e.filename}:${e.lineno}: ${e.message}`);
          throw e;
        };
  
        if (ENVIRONMENT_IS_NODE) {
          worker.on('message', (data) => worker.onmessage({ data: data }));
          worker.on('error', (e) => worker.onerror(e));
        }
  
        assert(wasmMemory instanceof WebAssembly.Memory, 'WebAssembly memory should have been loaded by now!');
        assert(wasmModule instanceof WebAssembly.Module, 'WebAssembly Module should have been loaded by now!');
  
        // When running on a pthread, none of the incoming parameters on the module
        // object are present. Proxy known handlers back to the main thread if specified.
        var handlers = [];
        var knownHandlers = [
          'onExit',
          'onAbort',
          'print',
          'printErr',
        ];
        for (var handler of knownHandlers) {
          if (Module.propertyIsEnumerable(handler)) {
            handlers.push(handler);
          }
        }
  
        worker.workerID = PThread.nextWorkerID++;
  
        // Ask the new worker to load up the Emscripten-compiled page. This is a heavy operation.
        worker.postMessage({
          cmd: 'load',
          handlers: handlers,
          wasmMemory,
          wasmModule,
          'workerID': worker.workerID,
        });
      }),
  loadWasmModuleToAllWorkers(onMaybeReady) {
        // Instantiation is synchronous in pthreads.
        if (
          ENVIRONMENT_IS_PTHREAD
          || ENVIRONMENT_IS_WASM_WORKER
        ) {
          return onMaybeReady();
        }
  
        let pthreadPoolReady = Promise.all(PThread.unusedWorkers.map(PThread.loadWasmModuleToWorker));
        pthreadPoolReady.then(onMaybeReady);
      },
  allocateUnusedWorker() {
        var worker;
        var workerOptions = {
          // This is the way that we signal to the node worker that it is hosting
          // a pthread.
          'workerData': 'em-pthread',
          // This is the way that we signal to the Web Worker that it is hosting
          // a pthread.
          'name': 'em-pthread-' + PThread.nextWorkerID,
        };
        var pthreadMainJs = _scriptName;
        // We can't use makeModuleReceiveWithVar here since we want to also
        // call URL.createObjectURL on the mainScriptUrlOrBlob.
        if (Module['mainScriptUrlOrBlob']) {
          pthreadMainJs = Module['mainScriptUrlOrBlob'];
          if (typeof pthreadMainJs != 'string') {
            pthreadMainJs = URL.createObjectURL(pthreadMainJs);
          }
        }
        worker = new Worker(pthreadMainJs, workerOptions);
        PThread.unusedWorkers.push(worker);
      },
  getNewWorker() {
        if (PThread.unusedWorkers.length == 0) {
  // PTHREAD_POOL_SIZE_STRICT should show a warning and, if set to level `2`, return from the function.
  // However, if we're in Node.js, then we can create new workers on the fly and PTHREAD_POOL_SIZE_STRICT
  // should be ignored altogether.
          if (!ENVIRONMENT_IS_NODE) {
              err('Tried to spawn a new thread, but the thread pool is exhausted.\n' +
              'This might result in a deadlock unless some threads eventually exit or the code explicitly breaks out to the event loop.\n' +
              'If you want to increase the pool size, use setting `-sPTHREAD_POOL_SIZE=...`.'
                + '\nIf you want to throw an explicit error instead of the risk of deadlocking in those cases, use setting `-sPTHREAD_POOL_SIZE_STRICT=2`.'
              );
          }
          PThread.allocateUnusedWorker();
          PThread.loadWasmModuleToWorker(PThread.unusedWorkers[0]);
        }
        return PThread.unusedWorkers.pop();
      },
  };

  var _wasmWorkerDelayedMessageQueue = [];
  
  
  
  
  
  var maybeExit = () => {
      if (!keepRuntimeAlive()) {
        try {
          if (ENVIRONMENT_IS_PTHREAD) __emscripten_thread_exit(EXITSTATUS);
          else
          _exit(EXITSTATUS);
        } catch (e) {
          handleException(e);
        }
      }
    };
  var callUserCallback = (func) => {
      if (ABORT) {
        err('user callback triggered after runtime exited or application aborted.  Ignoring.');
        return;
      }
      try {
        func();
        maybeExit();
      } catch (e) {
        handleException(e);
      }
    };
  
  var wasmTableMirror = [];
  
  /** @type {WebAssembly.Table} */
  var wasmTable;
  var getWasmTableEntry = (funcPtr) => {
      var func = wasmTableMirror[funcPtr];
      if (!func) {
        if (funcPtr >= wasmTableMirror.length) wasmTableMirror.length = funcPtr + 1;
        /** @suppress {checkTypes} */
        wasmTableMirror[funcPtr] = func = wasmTable.get(funcPtr);
      }
      /** @suppress {checkTypes} */
      assert(wasmTable.get(funcPtr) == func, 'JavaScript-side Wasm function table mirror is out of date!');
      return func;
    };
  var _wasmWorkerRunPostMessage = (e) => {
      // '_wsc' is short for 'wasm call', trying to use an identifier name that
      // will never conflict with user code
      let data = e.data;
      let wasmCall = data['_wsc'];
      wasmCall && callUserCallback(() => getWasmTableEntry(wasmCall)(...data['x']));
    };
  
  var _wasmWorkerAppendToQueue = (e) => {
      _wasmWorkerDelayedMessageQueue.push(e);
    };
  
  
  var _wasmWorkerInitializeRuntime = () => {
      let m = Module;
      assert(m['sb'] % 16 == 0);
      assert(m['sz'] % 16 == 0);
  
      // Wasm workers basically never exit their runtime
      noExitRuntime = 1;
  
      // Run the C side Worker initialization for stack and TLS.
      __emscripten_wasm_worker_initialize(m['sb'], m['sz']);
      // Record the pthread configuration, and whether this Wasm Worker supports synchronous blocking in emscripten_futex_wait().
      // (regular Wasm Workers do, AudioWorklets don't)
      ___set_thread_state(/*thread_ptr=*/0, /*is_main_thread=*/0, /*is_runtime_thread=*/0, /*supports_wait=*/ typeof AudioWorkletGlobalScope === 'undefined');
  
      // Write the stack cookie last, after we have set up the proper bounds and
      // current position of the stack.
      writeStackCookie();
  
      // Audio Worklets do not have postMessage()ing capabilities.
      if (typeof AudioWorkletGlobalScope === 'undefined') {
        // The Wasm Worker runtime is now up, so we can start processing
        // any postMessage function calls that have been received. Drop the temp
        // message handler that queued any pending incoming postMessage function calls ...
        removeEventListener('message', _wasmWorkerAppendToQueue);
        // ... then flush whatever messages we may have already gotten in the queue,
        //     and clear _wasmWorkerDelayedMessageQueue to undefined ...
        _wasmWorkerDelayedMessageQueue = _wasmWorkerDelayedMessageQueue.forEach(_wasmWorkerRunPostMessage);
        // ... and finally register the proper postMessage handler that immediately
        // dispatches incoming function calls without queueing them.
        addEventListener('message', _wasmWorkerRunPostMessage);
      }
    };

  var onPostRuns = [];
  var addOnPostRun = (cb) => onPostRuns.unshift(cb);



  
  
  var establishStackSpace = (pthread_ptr) => {
      var stackHigh = HEAPU32[(((pthread_ptr)+(52))>>2)];
      var stackSize = HEAPU32[(((pthread_ptr)+(56))>>2)];
      var stackLow = stackHigh - stackSize;
      assert(stackHigh != 0);
      assert(stackLow != 0);
      assert(stackHigh > stackLow, 'stackHigh must be higher then stackLow');
      // Set stack limits used by `emscripten/stack.h` function.  These limits are
      // cached in wasm-side globals to make checks as fast as possible.
      _emscripten_stack_set_limits(stackHigh, stackLow);
  
      // Call inside wasm module to set up the stack frame for this pthread in wasm module scope
      stackRestore(stackHigh);
  
      // Write the stack cookie last, after we have set up the proper bounds and
      // current position of the stack.
      writeStackCookie();
    };

  
    /**
     * @param {number} ptr
     * @param {string} type
     */
  function getValue(ptr, type = 'i8') {
    if (type.endsWith('*')) type = '*';
    switch (type) {
      case 'i1': return HEAP8[ptr];
      case 'i8': return HEAP8[ptr];
      case 'i16': return HEAP16[((ptr)>>1)];
      case 'i32': return HEAP32[((ptr)>>2)];
      case 'i64': return HEAP64[((ptr)>>3)];
      case 'float': return HEAPF32[((ptr)>>2)];
      case 'double': return HEAPF64[((ptr)>>3)];
      case '*': return HEAPU32[((ptr)>>2)];
      default: abort(`invalid type for getValue: ${type}`);
    }
  }

  
  
  
  
  var invokeEntryPoint = (ptr, arg) => {
      // An old thread on this worker may have been canceled without returning the
      // `runtimeKeepaliveCounter` to zero. Reset it now so the new thread won't
      // be affected.
      runtimeKeepaliveCounter = 0;
  
      // Same for noExitRuntime.  The default for pthreads should always be false
      // otherwise pthreads would never complete and attempts to pthread_join to
      // them would block forever.
      // pthreads can still choose to set `noExitRuntime` explicitly, or
      // call emscripten_unwind_to_js_event_loop to extend their lifetime beyond
      // their main function.  See comment in src/runtime_pthread.js for more.
      noExitRuntime = 0;
  
      // pthread entry points are always of signature 'void *ThreadMain(void *arg)'
      // Native codebases sometimes spawn threads with other thread entry point
      // signatures, such as void ThreadMain(void *arg), void *ThreadMain(), or
      // void ThreadMain().  That is not acceptable per C/C++ specification, but
      // x86 compiler ABI extensions enable that to work. If you find the
      // following line to crash, either change the signature to "proper" void
      // *ThreadMain(void *arg) form, or try linking with the Emscripten linker
      // flag -sEMULATE_FUNCTION_POINTER_CASTS to add in emulation for this x86
      // ABI extension.
  
      var result = getWasmTableEntry(ptr)(arg);
  
      checkStackCookie();
      function finish(result) {
        if (keepRuntimeAlive()) {
          EXITSTATUS = result;
        } else {
          __emscripten_thread_exit(result);
        }
      }
      finish(result);
    };

  var noExitRuntime = Module['noExitRuntime'] || true;


  var registerTLSInit = (tlsInitFunc) => PThread.tlsInitFunctions.push(tlsInitFunc);

  
    /**
     * @param {number} ptr
     * @param {number} value
     * @param {string} type
     */
  function setValue(ptr, value, type = 'i8') {
    if (type.endsWith('*')) type = '*';
    switch (type) {
      case 'i1': HEAP8[ptr] = value; break;
      case 'i8': HEAP8[ptr] = value; break;
      case 'i16': HEAP16[((ptr)>>1)] = value; break;
      case 'i32': HEAP32[((ptr)>>2)] = value; break;
      case 'i64': HEAP64[((ptr)>>3)] = BigInt(value); break;
      case 'float': HEAPF32[((ptr)>>2)] = value; break;
      case 'double': HEAPF64[((ptr)>>3)] = value; break;
      case '*': HEAPU32[((ptr)>>2)] = value; break;
      default: abort(`invalid type for setValue: ${type}`);
    }
  }



  var warnOnce = (text) => {
      warnOnce.shown ||= {};
      if (!warnOnce.shown[text]) {
        warnOnce.shown[text] = 1;
        if (ENVIRONMENT_IS_NODE) text = 'warning: ' + text;
        err(text);
      }
    };

  var UTF8Decoder = typeof TextDecoder != 'undefined' ? new TextDecoder() : undefined;
  
    /**
     * Given a pointer 'idx' to a null-terminated UTF8-encoded string in the given
     * array that contains uint8 values, returns a copy of that string as a
     * Javascript String object.
     * heapOrArray is either a regular array, or a JavaScript typed array view.
     * @param {number=} idx
     * @param {number=} maxBytesToRead
     * @return {string}
     */
  var UTF8ArrayToString = (heapOrArray, idx = 0, maxBytesToRead = NaN) => {
      var endIdx = idx + maxBytesToRead;
      var endPtr = idx;
      // TextDecoder needs to know the byte length in advance, it doesn't stop on
      // null terminator by itself.  Also, use the length info to avoid running tiny
      // strings through TextDecoder, since .subarray() allocates garbage.
      // (As a tiny code save trick, compare endPtr against endIdx using a negation,
      // so that undefined/NaN means Infinity)
      while (heapOrArray[endPtr] && !(endPtr >= endIdx)) ++endPtr;
  
      if (endPtr - idx > 16 && heapOrArray.buffer && UTF8Decoder) {
        return UTF8Decoder.decode(heapOrArray.buffer instanceof ArrayBuffer ? heapOrArray.subarray(idx, endPtr) : heapOrArray.slice(idx, endPtr));
      }
      var str = '';
      // If building with TextDecoder, we have already computed the string length
      // above, so test loop end condition against that
      while (idx < endPtr) {
        // For UTF8 byte structure, see:
        // http://en.wikipedia.org/wiki/UTF-8#Description
        // https://www.ietf.org/rfc/rfc2279.txt
        // https://tools.ietf.org/html/rfc3629
        var u0 = heapOrArray[idx++];
        if (!(u0 & 0x80)) { str += String.fromCharCode(u0); continue; }
        var u1 = heapOrArray[idx++] & 63;
        if ((u0 & 0xE0) == 0xC0) { str += String.fromCharCode(((u0 & 31) << 6) | u1); continue; }
        var u2 = heapOrArray[idx++] & 63;
        if ((u0 & 0xF0) == 0xE0) {
          u0 = ((u0 & 15) << 12) | (u1 << 6) | u2;
        } else {
          if ((u0 & 0xF8) != 0xF0) warnOnce('Invalid UTF-8 leading byte ' + ptrToString(u0) + ' encountered when deserializing a UTF-8 string in wasm memory to a JS string!');
          u0 = ((u0 & 7) << 18) | (u1 << 12) | (u2 << 6) | (heapOrArray[idx++] & 63);
        }
  
        if (u0 < 0x10000) {
          str += String.fromCharCode(u0);
        } else {
          var ch = u0 - 0x10000;
          str += String.fromCharCode(0xD800 | (ch >> 10), 0xDC00 | (ch & 0x3FF));
        }
      }
      return str;
    };
  
    /**
     * Given a pointer 'ptr' to a null-terminated UTF8-encoded string in the
     * emscripten HEAP, returns a copy of that string as a Javascript String object.
     *
     * @param {number} ptr
     * @param {number=} maxBytesToRead - An optional length that specifies the
     *   maximum number of bytes to read. You can omit this parameter to scan the
     *   string until the first 0 byte. If maxBytesToRead is passed, and the string
     *   at [ptr, ptr+maxBytesToReadr[ contains a null byte in the middle, then the
     *   string will cut short at that byte index (i.e. maxBytesToRead will not
     *   produce a string of exact length [ptr, ptr+maxBytesToRead[) N.B. mixing
     *   frequent uses of UTF8ToString() with and without maxBytesToRead may throw
     *   JS JIT optimizations off, so it is worth to consider consistently using one
     * @return {string}
     */
  var UTF8ToString = (ptr, maxBytesToRead) => {
      assert(typeof ptr == 'number', `UTF8ToString expects a number (got ${typeof ptr})`);
      return ptr ? UTF8ArrayToString(HEAPU8, ptr, maxBytesToRead) : '';
    };
  var ___assert_fail = (condition, filename, line, func) =>
      abort(`Assertion failed: ${UTF8ToString(condition)}, at: ` + [filename ? UTF8ToString(filename) : 'unknown filename', line, func ? UTF8ToString(func) : 'unknown function']);

  var ___call_sighandler = (fp, sig) => getWasmTableEntry(fp)(sig);

  class ExceptionInfo {
      // excPtr - Thrown object pointer to wrap. Metadata pointer is calculated from it.
      constructor(excPtr) {
        this.excPtr = excPtr;
        this.ptr = excPtr - 24;
      }
  
      set_type(type) {
        HEAPU32[(((this.ptr)+(4))>>2)] = type;
      }
  
      get_type() {
        return HEAPU32[(((this.ptr)+(4))>>2)];
      }
  
      set_destructor(destructor) {
        HEAPU32[(((this.ptr)+(8))>>2)] = destructor;
      }
  
      get_destructor() {
        return HEAPU32[(((this.ptr)+(8))>>2)];
      }
  
      set_caught(caught) {
        caught = caught ? 1 : 0;
        HEAP8[(this.ptr)+(12)] = caught;
      }
  
      get_caught() {
        return HEAP8[(this.ptr)+(12)] != 0;
      }
  
      set_rethrown(rethrown) {
        rethrown = rethrown ? 1 : 0;
        HEAP8[(this.ptr)+(13)] = rethrown;
      }
  
      get_rethrown() {
        return HEAP8[(this.ptr)+(13)] != 0;
      }
  
      // Initialize native structure fields. Should be called once after allocated.
      init(type, destructor) {
        this.set_adjusted_ptr(0);
        this.set_type(type);
        this.set_destructor(destructor);
      }
  
      set_adjusted_ptr(adjustedPtr) {
        HEAPU32[(((this.ptr)+(16))>>2)] = adjustedPtr;
      }
  
      get_adjusted_ptr() {
        return HEAPU32[(((this.ptr)+(16))>>2)];
      }
    }
  
  var exceptionLast = 0;
  
  var uncaughtExceptionCount = 0;
  var ___cxa_throw = (ptr, type, destructor) => {
      var info = new ExceptionInfo(ptr);
      // Initialize ExceptionInfo content after it was allocated in __cxa_allocate_exception.
      info.init(type, destructor);
      exceptionLast = ptr;
      uncaughtExceptionCount++;
      assert(false, 'Exception thrown, but exception catching is not enabled. Compile with -sNO_DISABLE_EXCEPTION_CATCHING or -sEXCEPTION_CATCHING_ALLOWED=[..] to catch.');
    };

  
  
  
  
  
  function pthreadCreateProxied(pthread_ptr, attr, startRoutine, arg) {
  if (ENVIRONMENT_IS_PTHREAD)
    return proxyToMainThread(2, 0, 1, pthread_ptr, attr, startRoutine, arg);
  return ___pthread_create_js(pthread_ptr, attr, startRoutine, arg)
  }
  
  
  var _emscripten_has_threading_support = () => typeof SharedArrayBuffer != 'undefined';
  
  var ___pthread_create_js = (pthread_ptr, attr, startRoutine, arg) => {
      if (!_emscripten_has_threading_support()) {
        dbg('pthread_create: environment does not support SharedArrayBuffer, pthreads are not available');
        return 6;
      }
  
      // List of JS objects that will transfer ownership to the Worker hosting the thread
      var transferList = [];
      var error = 0;
  
      // Synchronously proxy the thread creation to main thread if possible. If we
      // need to transfer ownership of objects, then proxy asynchronously via
      // postMessage.
      if (ENVIRONMENT_IS_PTHREAD && (transferList.length === 0 || error)) {
        return pthreadCreateProxied(pthread_ptr, attr, startRoutine, arg);
      }
  
      // If on the main thread, and accessing Canvas/OffscreenCanvas failed, abort
      // with the detected error.
      if (error) return error;
  
      var threadParams = {
        startRoutine,
        pthread_ptr,
        arg,
        transferList,
      };
  
      if (ENVIRONMENT_IS_PTHREAD) {
        // The prepopulated pool of web workers that can host pthreads is stored
        // in the main JS thread. Therefore if a pthread is attempting to spawn a
        // new thread, the thread creation must be deferred to the main JS thread.
        threadParams.cmd = 'spawnThread';
        postMessage(threadParams, transferList);
        // When we defer thread creation this way, we have no way to detect thread
        // creation synchronously today, so we have to assume success and return 0.
        return 0;
      }
  
      // We are the main thread, so we have the pthread warmup pool in this
      // thread and can fire off JS thread creation directly ourselves.
      return spawnThread(threadParams);
    };

  var __abort_js = () =>
      abort('native code called abort()');

  var embindRepr = (v) => {
      if (v === null) {
          return 'null';
      }
      var t = typeof v;
      if (t === 'object' || t === 'array' || t === 'function') {
          return v.toString();
      } else {
          return '' + v;
      }
    };
  
  var embind_init_charCodes = () => {
      var codes = new Array(256);
      for (var i = 0; i < 256; ++i) {
          codes[i] = String.fromCharCode(i);
      }
      embind_charCodes = codes;
    };
  var embind_charCodes;
  var readLatin1String = (ptr) => {
      var ret = "";
      var c = ptr;
      while (HEAPU8[c]) {
          ret += embind_charCodes[HEAPU8[c++]];
      }
      return ret;
    };
  
  var awaitingDependencies = {
  };
  
  var registeredTypes = {
  };
  
  var typeDependencies = {
  };
  
  var BindingError;
  var throwBindingError = (message) => { throw new BindingError(message); };
  
  
  
  
  var InternalError;
  var throwInternalError = (message) => { throw new InternalError(message); };
  var whenDependentTypesAreResolved = (myTypes, dependentTypes, getTypeConverters) => {
      myTypes.forEach((type) => typeDependencies[type] = dependentTypes);
  
      function onComplete(typeConverters) {
        var myTypeConverters = getTypeConverters(typeConverters);
        if (myTypeConverters.length !== myTypes.length) {
          throwInternalError('Mismatched type converter count');
        }
        for (var i = 0; i < myTypes.length; ++i) {
          registerType(myTypes[i], myTypeConverters[i]);
        }
      }
  
      var typeConverters = new Array(dependentTypes.length);
      var unregisteredTypes = [];
      var registered = 0;
      dependentTypes.forEach((dt, i) => {
        if (registeredTypes.hasOwnProperty(dt)) {
          typeConverters[i] = registeredTypes[dt];
        } else {
          unregisteredTypes.push(dt);
          if (!awaitingDependencies.hasOwnProperty(dt)) {
            awaitingDependencies[dt] = [];
          }
          awaitingDependencies[dt].push(() => {
            typeConverters[i] = registeredTypes[dt];
            ++registered;
            if (registered === unregisteredTypes.length) {
              onComplete(typeConverters);
            }
          });
        }
      });
      if (0 === unregisteredTypes.length) {
        onComplete(typeConverters);
      }
    };
  /** @param {Object=} options */
  function sharedRegisterType(rawType, registeredInstance, options = {}) {
      var name = registeredInstance.name;
      if (!rawType) {
        throwBindingError(`type "${name}" must have a positive integer typeid pointer`);
      }
      if (registeredTypes.hasOwnProperty(rawType)) {
        if (options.ignoreDuplicateRegistrations) {
          return;
        } else {
          throwBindingError(`Cannot register type '${name}' twice`);
        }
      }
  
      registeredTypes[rawType] = registeredInstance;
      delete typeDependencies[rawType];
  
      if (awaitingDependencies.hasOwnProperty(rawType)) {
        var callbacks = awaitingDependencies[rawType];
        delete awaitingDependencies[rawType];
        callbacks.forEach((cb) => cb());
      }
    }
  /** @param {Object=} options */
  function registerType(rawType, registeredInstance, options = {}) {
      if (registeredInstance.argPackAdvance === undefined) {
        throw new TypeError('registerType registeredInstance requires argPackAdvance');
      }
      return sharedRegisterType(rawType, registeredInstance, options);
    }
  
  var integerReadValueFromPointer = (name, width, signed) => {
      // integers are quite common, so generate very specialized functions
      switch (width) {
          case 1: return signed ?
              (pointer) => HEAP8[pointer] :
              (pointer) => HEAPU8[pointer];
          case 2: return signed ?
              (pointer) => HEAP16[((pointer)>>1)] :
              (pointer) => HEAPU16[((pointer)>>1)]
          case 4: return signed ?
              (pointer) => HEAP32[((pointer)>>2)] :
              (pointer) => HEAPU32[((pointer)>>2)]
          case 8: return signed ?
              (pointer) => HEAP64[((pointer)>>3)] :
              (pointer) => HEAPU64[((pointer)>>3)]
          default:
              throw new TypeError(`invalid integer width (${width}): ${name}`);
      }
    };
  /** @suppress {globalThis} */
  var __embind_register_bigint = (primitiveType, name, size, minRange, maxRange) => {
      name = readLatin1String(name);
  
      var isUnsignedType = (name.indexOf('u') != -1);
  
      // maxRange comes through as -1 for uint64_t (see issue 13902). Work around that temporarily
      if (isUnsignedType) {
        maxRange = (1n << 64n) - 1n;
      }
  
      registerType(primitiveType, {
        name,
        'fromWireType': (value) => value,
        'toWireType': function(destructors, value) {
          if (typeof value != "bigint" && typeof value != "number") {
            throw new TypeError(`Cannot convert "${embindRepr(value)}" to ${this.name}`);
          }
          if (typeof value == "number") {
            value = BigInt(value);
          }
          if (value < minRange || value > maxRange) {
            throw new TypeError(`Passing a number "${embindRepr(value)}" from JS side to C/C++ side to an argument of type "${name}", which is outside the valid range [${minRange}, ${maxRange}]!`);
          }
          return value;
        },
        argPackAdvance: GenericWireTypeSize,
        'readValueFromPointer': integerReadValueFromPointer(name, size, !isUnsignedType),
        destructorFunction: null, // This type does not need a destructor
      });
    };

  
  
  var GenericWireTypeSize = 8;
  /** @suppress {globalThis} */
  var __embind_register_bool = (rawType, name, trueValue, falseValue) => {
      name = readLatin1String(name);
      registerType(rawType, {
          name,
          'fromWireType': function(wt) {
              // ambiguous emscripten ABI: sometimes return values are
              // true or false, and sometimes integers (0 or 1)
              return !!wt;
          },
          'toWireType': function(destructors, o) {
              return o ? trueValue : falseValue;
          },
          argPackAdvance: GenericWireTypeSize,
          'readValueFromPointer': function(pointer) {
              return this['fromWireType'](HEAPU8[pointer]);
          },
          destructorFunction: null, // This type does not need a destructor
      });
    };

  
  
  var shallowCopyInternalPointer = (o) => {
      return {
        count: o.count,
        deleteScheduled: o.deleteScheduled,
        preservePointerOnDelete: o.preservePointerOnDelete,
        ptr: o.ptr,
        ptrType: o.ptrType,
        smartPtr: o.smartPtr,
        smartPtrType: o.smartPtrType,
      };
    };
  
  var throwInstanceAlreadyDeleted = (obj) => {
      function getInstanceTypeName(handle) {
        return handle.$$.ptrType.registeredClass.name;
      }
      throwBindingError(getInstanceTypeName(obj) + ' instance already deleted');
    };
  
  var finalizationRegistry = false;
  
  var detachFinalizer = (handle) => {};
  
  var runDestructor = ($$) => {
      if ($$.smartPtr) {
        $$.smartPtrType.rawDestructor($$.smartPtr);
      } else {
        $$.ptrType.registeredClass.rawDestructor($$.ptr);
      }
    };
  var releaseClassHandle = ($$) => {
      $$.count.value -= 1;
      var toDelete = 0 === $$.count.value;
      if (toDelete) {
        runDestructor($$);
      }
    };
  
  var downcastPointer = (ptr, ptrClass, desiredClass) => {
      if (ptrClass === desiredClass) {
        return ptr;
      }
      if (undefined === desiredClass.baseClass) {
        return null; // no conversion
      }
  
      var rv = downcastPointer(ptr, ptrClass, desiredClass.baseClass);
      if (rv === null) {
        return null;
      }
      return desiredClass.downcast(rv);
    };
  
  var registeredPointers = {
  };
  
  var registeredInstances = {
  };
  
  var getBasestPointer = (class_, ptr) => {
      if (ptr === undefined) {
          throwBindingError('ptr should not be undefined');
      }
      while (class_.baseClass) {
          ptr = class_.upcast(ptr);
          class_ = class_.baseClass;
      }
      return ptr;
    };
  var getInheritedInstance = (class_, ptr) => {
      ptr = getBasestPointer(class_, ptr);
      return registeredInstances[ptr];
    };
  
  
  var makeClassHandle = (prototype, record) => {
      if (!record.ptrType || !record.ptr) {
        throwInternalError('makeClassHandle requires ptr and ptrType');
      }
      var hasSmartPtrType = !!record.smartPtrType;
      var hasSmartPtr = !!record.smartPtr;
      if (hasSmartPtrType !== hasSmartPtr) {
        throwInternalError('Both smartPtrType and smartPtr must be specified');
      }
      record.count = { value: 1 };
      return attachFinalizer(Object.create(prototype, {
        $$: {
          value: record,
          writable: true,
        },
      }));
    };
  /** @suppress {globalThis} */
  function RegisteredPointer_fromWireType(ptr) {
      // ptr is a raw pointer (or a raw smartpointer)
  
      // rawPointer is a maybe-null raw pointer
      var rawPointer = this.getPointee(ptr);
      if (!rawPointer) {
        this.destructor(ptr);
        return null;
      }
  
      var registeredInstance = getInheritedInstance(this.registeredClass, rawPointer);
      if (undefined !== registeredInstance) {
        // JS object has been neutered, time to repopulate it
        if (0 === registeredInstance.$$.count.value) {
          registeredInstance.$$.ptr = rawPointer;
          registeredInstance.$$.smartPtr = ptr;
          return registeredInstance['clone']();
        } else {
          // else, just increment reference count on existing object
          // it already has a reference to the smart pointer
          var rv = registeredInstance['clone']();
          this.destructor(ptr);
          return rv;
        }
      }
  
      function makeDefaultHandle() {
        if (this.isSmartPointer) {
          return makeClassHandle(this.registeredClass.instancePrototype, {
            ptrType: this.pointeeType,
            ptr: rawPointer,
            smartPtrType: this,
            smartPtr: ptr,
          });
        } else {
          return makeClassHandle(this.registeredClass.instancePrototype, {
            ptrType: this,
            ptr,
          });
        }
      }
  
      var actualType = this.registeredClass.getActualType(rawPointer);
      var registeredPointerRecord = registeredPointers[actualType];
      if (!registeredPointerRecord) {
        return makeDefaultHandle.call(this);
      }
  
      var toType;
      if (this.isConst) {
        toType = registeredPointerRecord.constPointerType;
      } else {
        toType = registeredPointerRecord.pointerType;
      }
      var dp = downcastPointer(
          rawPointer,
          this.registeredClass,
          toType.registeredClass);
      if (dp === null) {
        return makeDefaultHandle.call(this);
      }
      if (this.isSmartPointer) {
        return makeClassHandle(toType.registeredClass.instancePrototype, {
          ptrType: toType,
          ptr: dp,
          smartPtrType: this,
          smartPtr: ptr,
        });
      } else {
        return makeClassHandle(toType.registeredClass.instancePrototype, {
          ptrType: toType,
          ptr: dp,
        });
      }
    }
  var attachFinalizer = (handle) => {
      if ('undefined' === typeof FinalizationRegistry) {
        attachFinalizer = (handle) => handle;
        return handle;
      }
      // If the running environment has a FinalizationRegistry (see
      // https://github.com/tc39/proposal-weakrefs), then attach finalizers
      // for class handles.  We check for the presence of FinalizationRegistry
      // at run-time, not build-time.
      finalizationRegistry = new FinalizationRegistry((info) => {
        console.warn(info.leakWarning);
        releaseClassHandle(info.$$);
      });
      attachFinalizer = (handle) => {
        var $$ = handle.$$;
        var hasSmartPtr = !!$$.smartPtr;
        if (hasSmartPtr) {
          // We should not call the destructor on raw pointers in case other code expects the pointee to live
          var info = { $$: $$ };
          // Create a warning as an Error instance in advance so that we can store
          // the current stacktrace and point to it when / if a leak is detected.
          // This is more useful than the empty stacktrace of `FinalizationRegistry`
          // callback.
          var cls = $$.ptrType.registeredClass;
          var err = new Error(`Embind found a leaked C++ instance ${cls.name} <${ptrToString($$.ptr)}>.\n` +
          "We'll free it automatically in this case, but this functionality is not reliable across various environments.\n" +
          "Make sure to invoke .delete() manually once you're done with the instance instead.\n" +
          "Originally allocated"); // `.stack` will add "at ..." after this sentence
          if ('captureStackTrace' in Error) {
            Error.captureStackTrace(err, RegisteredPointer_fromWireType);
          }
          info.leakWarning = err.stack.replace(/^Error: /, '');
          finalizationRegistry.register(handle, info, handle);
        }
        return handle;
      };
      detachFinalizer = (handle) => finalizationRegistry.unregister(handle);
      return attachFinalizer(handle);
    };
  
  
  
  
  var deletionQueue = [];
  var flushPendingDeletes = () => {
      while (deletionQueue.length) {
        var obj = deletionQueue.pop();
        obj.$$.deleteScheduled = false;
        obj['delete']();
      }
    };
  
  var delayFunction;
  var init_ClassHandle = () => {
      Object.assign(ClassHandle.prototype, {
        "isAliasOf"(other) {
          if (!(this instanceof ClassHandle)) {
            return false;
          }
          if (!(other instanceof ClassHandle)) {
            return false;
          }
  
          var leftClass = this.$$.ptrType.registeredClass;
          var left = this.$$.ptr;
          other.$$ = /** @type {Object} */ (other.$$);
          var rightClass = other.$$.ptrType.registeredClass;
          var right = other.$$.ptr;
  
          while (leftClass.baseClass) {
            left = leftClass.upcast(left);
            leftClass = leftClass.baseClass;
          }
  
          while (rightClass.baseClass) {
            right = rightClass.upcast(right);
            rightClass = rightClass.baseClass;
          }
  
          return leftClass === rightClass && left === right;
        },
  
        "clone"() {
          if (!this.$$.ptr) {
            throwInstanceAlreadyDeleted(this);
          }
  
          if (this.$$.preservePointerOnDelete) {
            this.$$.count.value += 1;
            return this;
          } else {
            var clone = attachFinalizer(Object.create(Object.getPrototypeOf(this), {
              $$: {
                value: shallowCopyInternalPointer(this.$$),
              }
            }));
  
            clone.$$.count.value += 1;
            clone.$$.deleteScheduled = false;
            return clone;
          }
        },
  
        "delete"() {
          if (!this.$$.ptr) {
            throwInstanceAlreadyDeleted(this);
          }
  
          if (this.$$.deleteScheduled && !this.$$.preservePointerOnDelete) {
            throwBindingError('Object already scheduled for deletion');
          }
  
          detachFinalizer(this);
          releaseClassHandle(this.$$);
  
          if (!this.$$.preservePointerOnDelete) {
            this.$$.smartPtr = undefined;
            this.$$.ptr = undefined;
          }
        },
  
        "isDeleted"() {
          return !this.$$.ptr;
        },
  
        "deleteLater"() {
          if (!this.$$.ptr) {
            throwInstanceAlreadyDeleted(this);
          }
          if (this.$$.deleteScheduled && !this.$$.preservePointerOnDelete) {
            throwBindingError('Object already scheduled for deletion');
          }
          deletionQueue.push(this);
          if (deletionQueue.length === 1 && delayFunction) {
            delayFunction(flushPendingDeletes);
          }
          this.$$.deleteScheduled = true;
          return this;
        },
      });
    };
  /** @constructor */
  function ClassHandle() {
    }
  
  var createNamedFunction = (name, body) => Object.defineProperty(body, 'name', {
      value: name
    });
  
  
  var ensureOverloadTable = (proto, methodName, humanName) => {
      if (undefined === proto[methodName].overloadTable) {
        var prevFunc = proto[methodName];
        // Inject an overload resolver function that routes to the appropriate overload based on the number of arguments.
        proto[methodName] = function(...args) {
          // TODO This check can be removed in -O3 level "unsafe" optimizations.
          if (!proto[methodName].overloadTable.hasOwnProperty(args.length)) {
            throwBindingError(`Function '${humanName}' called with an invalid number of arguments (${args.length}) - expects one of (${proto[methodName].overloadTable})!`);
          }
          return proto[methodName].overloadTable[args.length].apply(this, args);
        };
        // Move the previous function into the overload table.
        proto[methodName].overloadTable = [];
        proto[methodName].overloadTable[prevFunc.argCount] = prevFunc;
      }
    };
  
  /** @param {number=} numArguments */
  var exposePublicSymbol = (name, value, numArguments) => {
      if (Module.hasOwnProperty(name)) {
        if (undefined === numArguments || (undefined !== Module[name].overloadTable && undefined !== Module[name].overloadTable[numArguments])) {
          throwBindingError(`Cannot register public name '${name}' twice`);
        }
  
        // We are exposing a function with the same name as an existing function. Create an overload table and a function selector
        // that routes between the two.
        ensureOverloadTable(Module, name, name);
        if (Module[name].overloadTable.hasOwnProperty(numArguments)) {
          throwBindingError(`Cannot register multiple overloads of a function with the same number of arguments (${numArguments})!`);
        }
        // Add the new function into the overload table.
        Module[name].overloadTable[numArguments] = value;
      } else {
        Module[name] = value;
        Module[name].argCount = numArguments;
      }
    };
  
  var char_0 = 48;
  
  var char_9 = 57;
  var makeLegalFunctionName = (name) => {
      assert(typeof name === 'string');
      name = name.replace(/[^a-zA-Z0-9_]/g, '$');
      var f = name.charCodeAt(0);
      if (f >= char_0 && f <= char_9) {
        return `_${name}`;
      }
      return name;
    };
  
  
  /** @constructor */
  function RegisteredClass(name,
                               constructor,
                               instancePrototype,
                               rawDestructor,
                               baseClass,
                               getActualType,
                               upcast,
                               downcast) {
      this.name = name;
      this.constructor = constructor;
      this.instancePrototype = instancePrototype;
      this.rawDestructor = rawDestructor;
      this.baseClass = baseClass;
      this.getActualType = getActualType;
      this.upcast = upcast;
      this.downcast = downcast;
      this.pureVirtualFunctions = [];
    }
  
  
  var upcastPointer = (ptr, ptrClass, desiredClass) => {
      while (ptrClass !== desiredClass) {
        if (!ptrClass.upcast) {
          throwBindingError(`Expected null or instance of ${desiredClass.name}, got an instance of ${ptrClass.name}`);
        }
        ptr = ptrClass.upcast(ptr);
        ptrClass = ptrClass.baseClass;
      }
      return ptr;
    };
  /** @suppress {globalThis} */
  function constNoSmartPtrRawPointerToWireType(destructors, handle) {
      if (handle === null) {
        if (this.isReference) {
          throwBindingError(`null is not a valid ${this.name}`);
        }
        return 0;
      }
  
      if (!handle.$$) {
        throwBindingError(`Cannot pass "${embindRepr(handle)}" as a ${this.name}`);
      }
      if (!handle.$$.ptr) {
        throwBindingError(`Cannot pass deleted object as a pointer of type ${this.name}`);
      }
      var handleClass = handle.$$.ptrType.registeredClass;
      var ptr = upcastPointer(handle.$$.ptr, handleClass, this.registeredClass);
      return ptr;
    }
  
  
  /** @suppress {globalThis} */
  function genericPointerToWireType(destructors, handle) {
      var ptr;
      if (handle === null) {
        if (this.isReference) {
          throwBindingError(`null is not a valid ${this.name}`);
        }
  
        if (this.isSmartPointer) {
          ptr = this.rawConstructor();
          if (destructors !== null) {
            destructors.push(this.rawDestructor, ptr);
          }
          return ptr;
        } else {
          return 0;
        }
      }
  
      if (!handle || !handle.$$) {
        throwBindingError(`Cannot pass "${embindRepr(handle)}" as a ${this.name}`);
      }
      if (!handle.$$.ptr) {
        throwBindingError(`Cannot pass deleted object as a pointer of type ${this.name}`);
      }
      if (!this.isConst && handle.$$.ptrType.isConst) {
        throwBindingError(`Cannot convert argument of type ${(handle.$$.smartPtrType ? handle.$$.smartPtrType.name : handle.$$.ptrType.name)} to parameter type ${this.name}`);
      }
      var handleClass = handle.$$.ptrType.registeredClass;
      ptr = upcastPointer(handle.$$.ptr, handleClass, this.registeredClass);
  
      if (this.isSmartPointer) {
        // TODO: this is not strictly true
        // We could support BY_EMVAL conversions from raw pointers to smart pointers
        // because the smart pointer can hold a reference to the handle
        if (undefined === handle.$$.smartPtr) {
          throwBindingError('Passing raw pointer to smart pointer is illegal');
        }
  
        switch (this.sharingPolicy) {
          case 0: // NONE
            // no upcasting
            if (handle.$$.smartPtrType === this) {
              ptr = handle.$$.smartPtr;
            } else {
              throwBindingError(`Cannot convert argument of type ${(handle.$$.smartPtrType ? handle.$$.smartPtrType.name : handle.$$.ptrType.name)} to parameter type ${this.name}`);
            }
            break;
  
          case 1: // INTRUSIVE
            ptr = handle.$$.smartPtr;
            break;
  
          case 2: // BY_EMVAL
            if (handle.$$.smartPtrType === this) {
              ptr = handle.$$.smartPtr;
            } else {
              var clonedHandle = handle['clone']();
              ptr = this.rawShare(
                ptr,
                Emval.toHandle(() => clonedHandle['delete']())
              );
              if (destructors !== null) {
                destructors.push(this.rawDestructor, ptr);
              }
            }
            break;
  
          default:
            throwBindingError('Unsupporting sharing policy');
        }
      }
      return ptr;
    }
  
  
  /** @suppress {globalThis} */
  function nonConstNoSmartPtrRawPointerToWireType(destructors, handle) {
      if (handle === null) {
        if (this.isReference) {
          throwBindingError(`null is not a valid ${this.name}`);
        }
        return 0;
      }
  
      if (!handle.$$) {
        throwBindingError(`Cannot pass "${embindRepr(handle)}" as a ${this.name}`);
      }
      if (!handle.$$.ptr) {
        throwBindingError(`Cannot pass deleted object as a pointer of type ${this.name}`);
      }
      if (handle.$$.ptrType.isConst) {
          throwBindingError(`Cannot convert argument of type ${handle.$$.ptrType.name} to parameter type ${this.name}`);
      }
      var handleClass = handle.$$.ptrType.registeredClass;
      var ptr = upcastPointer(handle.$$.ptr, handleClass, this.registeredClass);
      return ptr;
    }
  
  
  /** @suppress {globalThis} */
  function readPointer(pointer) {
      return this['fromWireType'](HEAPU32[((pointer)>>2)]);
    }
  
  
  var init_RegisteredPointer = () => {
      Object.assign(RegisteredPointer.prototype, {
        getPointee(ptr) {
          if (this.rawGetPointee) {
            ptr = this.rawGetPointee(ptr);
          }
          return ptr;
        },
        destructor(ptr) {
          this.rawDestructor?.(ptr);
        },
        argPackAdvance: GenericWireTypeSize,
        'readValueFromPointer': readPointer,
        'fromWireType': RegisteredPointer_fromWireType,
      });
    };
  /** @constructor
      @param {*=} pointeeType,
      @param {*=} sharingPolicy,
      @param {*=} rawGetPointee,
      @param {*=} rawConstructor,
      @param {*=} rawShare,
      @param {*=} rawDestructor,
       */
  function RegisteredPointer(
      name,
      registeredClass,
      isReference,
      isConst,
  
      // smart pointer properties
      isSmartPointer,
      pointeeType,
      sharingPolicy,
      rawGetPointee,
      rawConstructor,
      rawShare,
      rawDestructor
    ) {
      this.name = name;
      this.registeredClass = registeredClass;
      this.isReference = isReference;
      this.isConst = isConst;
  
      // smart pointer properties
      this.isSmartPointer = isSmartPointer;
      this.pointeeType = pointeeType;
      this.sharingPolicy = sharingPolicy;
      this.rawGetPointee = rawGetPointee;
      this.rawConstructor = rawConstructor;
      this.rawShare = rawShare;
      this.rawDestructor = rawDestructor;
  
      if (!isSmartPointer && registeredClass.baseClass === undefined) {
        if (isConst) {
          this['toWireType'] = constNoSmartPtrRawPointerToWireType;
          this.destructorFunction = null;
        } else {
          this['toWireType'] = nonConstNoSmartPtrRawPointerToWireType;
          this.destructorFunction = null;
        }
      } else {
        this['toWireType'] = genericPointerToWireType;
        // Here we must leave this.destructorFunction undefined, since whether genericPointerToWireType returns
        // a pointer that needs to be freed up is runtime-dependent, and cannot be evaluated at registration time.
        // TODO: Create an alternative mechanism that allows removing the use of var destructors = []; array in
        //       craftInvokerFunction altogether.
      }
    }
  
  /** @param {number=} numArguments */
  var replacePublicSymbol = (name, value, numArguments) => {
      if (!Module.hasOwnProperty(name)) {
        throwInternalError('Replacing nonexistent public symbol');
      }
      // If there's an overload table for this symbol, replace the symbol in the overload table instead.
      if (undefined !== Module[name].overloadTable && undefined !== numArguments) {
        Module[name].overloadTable[numArguments] = value;
      } else {
        Module[name] = value;
        Module[name].argCount = numArguments;
      }
    };
  
  
  
  var embind__requireFunction = (signature, rawFunction) => {
      signature = readLatin1String(signature);
  
      function makeDynCaller() {
        return getWasmTableEntry(rawFunction);
      }
  
      var fp = makeDynCaller();
      if (typeof fp != "function") {
          throwBindingError(`unknown function pointer with signature ${signature}: ${rawFunction}`);
      }
      return fp;
    };
  
  
  
  var extendError = (baseErrorType, errorName) => {
      var errorClass = createNamedFunction(errorName, function(message) {
        this.name = errorName;
        this.message = message;
  
        var stack = (new Error(message)).stack;
        if (stack !== undefined) {
          this.stack = this.toString() + '\n' +
              stack.replace(/^Error(:[^\n]*)?\n/, '');
        }
      });
      errorClass.prototype = Object.create(baseErrorType.prototype);
      errorClass.prototype.constructor = errorClass;
      errorClass.prototype.toString = function() {
        if (this.message === undefined) {
          return this.name;
        } else {
          return `${this.name}: ${this.message}`;
        }
      };
  
      return errorClass;
    };
  var UnboundTypeError;
  
  
  
  var getTypeName = (type) => {
      var ptr = ___getTypeName(type);
      var rv = readLatin1String(ptr);
      _free(ptr);
      return rv;
    };
  var throwUnboundTypeError = (message, types) => {
      var unboundTypes = [];
      var seen = {};
      function visit(type) {
        if (seen[type]) {
          return;
        }
        if (registeredTypes[type]) {
          return;
        }
        if (typeDependencies[type]) {
          typeDependencies[type].forEach(visit);
          return;
        }
        unboundTypes.push(type);
        seen[type] = true;
      }
      types.forEach(visit);
  
      throw new UnboundTypeError(`${message}: ` + unboundTypes.map(getTypeName).join([', ']));
    };
  
  var __embind_register_class = (rawType,
                             rawPointerType,
                             rawConstPointerType,
                             baseClassRawType,
                             getActualTypeSignature,
                             getActualType,
                             upcastSignature,
                             upcast,
                             downcastSignature,
                             downcast,
                             name,
                             destructorSignature,
                             rawDestructor) => {
      name = readLatin1String(name);
      getActualType = embind__requireFunction(getActualTypeSignature, getActualType);
      upcast &&= embind__requireFunction(upcastSignature, upcast);
      downcast &&= embind__requireFunction(downcastSignature, downcast);
      rawDestructor = embind__requireFunction(destructorSignature, rawDestructor);
      var legalFunctionName = makeLegalFunctionName(name);
  
      exposePublicSymbol(legalFunctionName, function() {
        // this code cannot run if baseClassRawType is zero
        throwUnboundTypeError(`Cannot construct ${name} due to unbound types`, [baseClassRawType]);
      });
  
      whenDependentTypesAreResolved(
        [rawType, rawPointerType, rawConstPointerType],
        baseClassRawType ? [baseClassRawType] : [],
        (base) => {
          base = base[0];
  
          var baseClass;
          var basePrototype;
          if (baseClassRawType) {
            baseClass = base.registeredClass;
            basePrototype = baseClass.instancePrototype;
          } else {
            basePrototype = ClassHandle.prototype;
          }
  
          var constructor = createNamedFunction(name, function(...args) {
            if (Object.getPrototypeOf(this) !== instancePrototype) {
              throw new BindingError("Use 'new' to construct " + name);
            }
            if (undefined === registeredClass.constructor_body) {
              throw new BindingError(name + " has no accessible constructor");
            }
            var body = registeredClass.constructor_body[args.length];
            if (undefined === body) {
              throw new BindingError(`Tried to invoke ctor of ${name} with invalid number of parameters (${args.length}) - expected (${Object.keys(registeredClass.constructor_body).toString()}) parameters instead!`);
            }
            return body.apply(this, args);
          });
  
          var instancePrototype = Object.create(basePrototype, {
            constructor: { value: constructor },
          });
  
          constructor.prototype = instancePrototype;
  
          var registeredClass = new RegisteredClass(name,
                                                    constructor,
                                                    instancePrototype,
                                                    rawDestructor,
                                                    baseClass,
                                                    getActualType,
                                                    upcast,
                                                    downcast);
  
          if (registeredClass.baseClass) {
            // Keep track of class hierarchy. Used to allow sub-classes to inherit class functions.
            registeredClass.baseClass.__derivedClasses ??= [];
  
            registeredClass.baseClass.__derivedClasses.push(registeredClass);
          }
  
          var referenceConverter = new RegisteredPointer(name,
                                                         registeredClass,
                                                         true,
                                                         false,
                                                         false);
  
          var pointerConverter = new RegisteredPointer(name + '*',
                                                       registeredClass,
                                                       false,
                                                       false,
                                                       false);
  
          var constPointerConverter = new RegisteredPointer(name + ' const*',
                                                            registeredClass,
                                                            false,
                                                            true,
                                                            false);
  
          registeredPointers[rawType] = {
            pointerType: pointerConverter,
            constPointerType: constPointerConverter
          };
  
          replacePublicSymbol(legalFunctionName, constructor);
  
          return [referenceConverter, pointerConverter, constPointerConverter];
        }
      );
    };

  var heap32VectorToArray = (count, firstElement) => {
      var array = [];
      for (var i = 0; i < count; i++) {
        // TODO(https://github.com/emscripten-core/emscripten/issues/17310):
        // Find a way to hoist the `>> 2` or `>> 3` out of this loop.
        array.push(HEAPU32[(((firstElement)+(i * 4))>>2)]);
      }
      return array;
    };
  
  
  var runDestructors = (destructors) => {
      while (destructors.length) {
        var ptr = destructors.pop();
        var del = destructors.pop();
        del(ptr);
      }
    };
  
  
  
  
  
  
  
  function usesDestructorStack(argTypes) {
      // Skip return value at index 0 - it's not deleted here.
      for (var i = 1; i < argTypes.length; ++i) {
        // The type does not define a destructor function - must use dynamic stack
        if (argTypes[i] !== null && argTypes[i].destructorFunction === undefined) {
          return true;
        }
      }
      return false;
    }
  
  function newFunc(constructor, argumentList) {
      if (!(constructor instanceof Function)) {
        throw new TypeError(`new_ called with constructor type ${typeof(constructor)} which is not a function`);
      }
      /*
       * Previously, the following line was just:
       *   function dummy() {};
       * Unfortunately, Chrome was preserving 'dummy' as the object's name, even
       * though at creation, the 'dummy' has the correct constructor name.  Thus,
       * objects created with IMVU.new would show up in the debugger as 'dummy',
       * which isn't very helpful.  Using IMVU.createNamedFunction addresses the
       * issue.  Doubly-unfortunately, there's no way to write a test for this
       * behavior.  -NRD 2013.02.22
       */
      var dummy = createNamedFunction(constructor.name || 'unknownFunctionName', function(){});
      dummy.prototype = constructor.prototype;
      var obj = new dummy;
  
      var r = constructor.apply(obj, argumentList);
      return (r instanceof Object) ? r : obj;
    }
  
  
  function checkArgCount(numArgs, minArgs, maxArgs, humanName, throwBindingError) {
      if (numArgs < minArgs || numArgs > maxArgs) {
        var argCountMessage = minArgs == maxArgs ? minArgs : `${minArgs} to ${maxArgs}`;
        throwBindingError(`function ${humanName} called with ${numArgs} arguments, expected ${argCountMessage}`);
      }
    }
  function createJsInvoker(argTypes, isClassMethodFunc, returns, isAsync) {
      var needsDestructorStack = usesDestructorStack(argTypes);
      var argCount = argTypes.length - 2;
      var argsList = [];
      var argsListWired = ['fn'];
      if (isClassMethodFunc) {
        argsListWired.push('thisWired');
      }
      for (var i = 0; i < argCount; ++i) {
        argsList.push(`arg${i}`)
        argsListWired.push(`arg${i}Wired`)
      }
      argsList = argsList.join(',')
      argsListWired = argsListWired.join(',')
  
      var invokerFnBody = `return function (${argsList}) {\n`;
  
      invokerFnBody += "checkArgCount(arguments.length, minArgs, maxArgs, humanName, throwBindingError);\n";
  
      if (needsDestructorStack) {
        invokerFnBody += "var destructors = [];\n";
      }
  
      var dtorStack = needsDestructorStack ? "destructors" : "null";
      var args1 = ["humanName", "throwBindingError", "invoker", "fn", "runDestructors", "retType", "classParam"];
  
      if (isClassMethodFunc) {
        invokerFnBody += `var thisWired = classParam['toWireType'](${dtorStack}, this);\n`;
      }
  
      for (var i = 0; i < argCount; ++i) {
        invokerFnBody += `var arg${i}Wired = argType${i}['toWireType'](${dtorStack}, arg${i});\n`;
        args1.push(`argType${i}`);
      }
  
      invokerFnBody += (returns || isAsync ? "var rv = ":"") + `invoker(${argsListWired});\n`;
  
      var returnVal = returns ? "rv" : "";
  
      if (needsDestructorStack) {
        invokerFnBody += "runDestructors(destructors);\n";
      } else {
        for (var i = isClassMethodFunc?1:2; i < argTypes.length; ++i) { // Skip return value at index 0 - it's not deleted here. Also skip class type if not a method.
          var paramName = (i === 1 ? "thisWired" : ("arg"+(i - 2)+"Wired"));
          if (argTypes[i].destructorFunction !== null) {
            invokerFnBody += `${paramName}_dtor(${paramName});\n`;
            args1.push(`${paramName}_dtor`);
          }
        }
      }
  
      if (returns) {
        invokerFnBody += "var ret = retType['fromWireType'](rv);\n" +
                         "return ret;\n";
      } else {
      }
  
      invokerFnBody += "}\n";
  
      args1.push('checkArgCount', 'minArgs', 'maxArgs');
      invokerFnBody = `if (arguments.length !== ${args1.length}){ throw new Error(humanName + "Expected ${args1.length} closure arguments " + arguments.length + " given."); }\n${invokerFnBody}`;
      return [args1, invokerFnBody];
    }
  
  function getRequiredArgCount(argTypes) {
      var requiredArgCount = argTypes.length - 2;
      for (var i = argTypes.length - 1; i >= 2; --i) {
        if (!argTypes[i].optional) {
          break;
        }
        requiredArgCount--;
      }
      return requiredArgCount;
    }
  
  function craftInvokerFunction(humanName, argTypes, classType, cppInvokerFunc, cppTargetFunc, /** boolean= */ isAsync) {
      // humanName: a human-readable string name for the function to be generated.
      // argTypes: An array that contains the embind type objects for all types in the function signature.
      //    argTypes[0] is the type object for the function return value.
      //    argTypes[1] is the type object for function this object/class type, or null if not crafting an invoker for a class method.
      //    argTypes[2...] are the actual function parameters.
      // classType: The embind type object for the class to be bound, or null if this is not a method of a class.
      // cppInvokerFunc: JS Function object to the C++-side function that interops into C++ code.
      // cppTargetFunc: Function pointer (an integer to FUNCTION_TABLE) to the target C++ function the cppInvokerFunc will end up calling.
      // isAsync: Optional. If true, returns an async function. Async bindings are only supported with JSPI.
      var argCount = argTypes.length;
  
      if (argCount < 2) {
        throwBindingError("argTypes array size mismatch! Must at least get return value and 'this' types!");
      }
  
      assert(!isAsync, 'Async bindings are only supported with JSPI.');
  
      var isClassMethodFunc = (argTypes[1] !== null && classType !== null);
  
      // Free functions with signature "void function()" do not need an invoker that marshalls between wire types.
  // TODO: This omits argument count check - enable only at -O3 or similar.
  //    if (ENABLE_UNSAFE_OPTS && argCount == 2 && argTypes[0].name == "void" && !isClassMethodFunc) {
  //       return FUNCTION_TABLE[fn];
  //    }
  
      // Determine if we need to use a dynamic stack to store the destructors for the function parameters.
      // TODO: Remove this completely once all function invokers are being dynamically generated.
      var needsDestructorStack = usesDestructorStack(argTypes);
  
      var returns = (argTypes[0].name !== "void");
  
      var expectedArgCount = argCount - 2;
      var minArgs = getRequiredArgCount(argTypes);
    // Builld the arguments that will be passed into the closure around the invoker
    // function.
    var closureArgs = [humanName, throwBindingError, cppInvokerFunc, cppTargetFunc, runDestructors, argTypes[0], argTypes[1]];
    for (var i = 0; i < argCount - 2; ++i) {
      closureArgs.push(argTypes[i+2]);
    }
    if (!needsDestructorStack) {
      for (var i = isClassMethodFunc?1:2; i < argTypes.length; ++i) { // Skip return value at index 0 - it's not deleted here. Also skip class type if not a method.
        if (argTypes[i].destructorFunction !== null) {
          closureArgs.push(argTypes[i].destructorFunction);
        }
      }
    }
    closureArgs.push(checkArgCount, minArgs, expectedArgCount);
  
    let [args, invokerFnBody] = createJsInvoker(argTypes, isClassMethodFunc, returns, isAsync);
    args.push(invokerFnBody);
    var invokerFn = newFunc(Function, args)(...closureArgs);
      return createNamedFunction(humanName, invokerFn);
    }
  var __embind_register_class_constructor = (
      rawClassType,
      argCount,
      rawArgTypesAddr,
      invokerSignature,
      invoker,
      rawConstructor
    ) => {
      assert(argCount > 0);
      var rawArgTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
      invoker = embind__requireFunction(invokerSignature, invoker);
      var args = [rawConstructor];
      var destructors = [];
  
      whenDependentTypesAreResolved([], [rawClassType], (classType) => {
        classType = classType[0];
        var humanName = `constructor ${classType.name}`;
  
        if (undefined === classType.registeredClass.constructor_body) {
          classType.registeredClass.constructor_body = [];
        }
        if (undefined !== classType.registeredClass.constructor_body[argCount - 1]) {
          throw new BindingError(`Cannot register multiple constructors with identical number of parameters (${argCount-1}) for class '${classType.name}'! Overload resolution is currently only performed using the parameter count, not actual type info!`);
        }
        classType.registeredClass.constructor_body[argCount - 1] = () => {
          throwUnboundTypeError(`Cannot construct ${classType.name} due to unbound types`, rawArgTypes);
        };
  
        whenDependentTypesAreResolved([], rawArgTypes, (argTypes) => {
          // Insert empty slot for context type (argTypes[1]).
          argTypes.splice(1, 0, null);
          classType.registeredClass.constructor_body[argCount - 1] = craftInvokerFunction(humanName, argTypes, null, invoker, rawConstructor);
          return [];
        });
        return [];
      });
    };

  
  
  
  
  
  
  var getFunctionName = (signature) => {
      signature = signature.trim();
      const argsIndex = signature.indexOf("(");
      if (argsIndex === -1) return signature;
      assert(signature.endsWith(")"), "Parentheses for argument names should match.");
      return signature.slice(0, argsIndex);
    };
  var __embind_register_class_function = (rawClassType,
                                      methodName,
                                      argCount,
                                      rawArgTypesAddr, // [ReturnType, ThisType, Args...]
                                      invokerSignature,
                                      rawInvoker,
                                      context,
                                      isPureVirtual,
                                      isAsync,
                                      isNonnullReturn) => {
      var rawArgTypes = heap32VectorToArray(argCount, rawArgTypesAddr);
      methodName = readLatin1String(methodName);
      methodName = getFunctionName(methodName);
      rawInvoker = embind__requireFunction(invokerSignature, rawInvoker);
  
      whenDependentTypesAreResolved([], [rawClassType], (classType) => {
        classType = classType[0];
        var humanName = `${classType.name}.${methodName}`;
  
        if (methodName.startsWith("@@")) {
          methodName = Symbol[methodName.substring(2)];
        }
  
        if (isPureVirtual) {
          classType.registeredClass.pureVirtualFunctions.push(methodName);
        }
  
        function unboundTypesHandler() {
          throwUnboundTypeError(`Cannot call ${humanName} due to unbound types`, rawArgTypes);
        }
  
        var proto = classType.registeredClass.instancePrototype;
        var method = proto[methodName];
        if (undefined === method || (undefined === method.overloadTable && method.className !== classType.name && method.argCount === argCount - 2)) {
          // This is the first overload to be registered, OR we are replacing a
          // function in the base class with a function in the derived class.
          unboundTypesHandler.argCount = argCount - 2;
          unboundTypesHandler.className = classType.name;
          proto[methodName] = unboundTypesHandler;
        } else {
          // There was an existing function with the same name registered. Set up
          // a function overload routing table.
          ensureOverloadTable(proto, methodName, humanName);
          proto[methodName].overloadTable[argCount - 2] = unboundTypesHandler;
        }
  
        whenDependentTypesAreResolved([], rawArgTypes, (argTypes) => {
          var memberFunction = craftInvokerFunction(humanName, argTypes, classType, rawInvoker, context, isAsync);
  
          // Replace the initial unbound-handler-stub function with the
          // appropriate member function, now that all types are resolved. If
          // multiple overloads are registered for this function, the function
          // goes into an overload table.
          if (undefined === proto[methodName].overloadTable) {
            // Set argCount in case an overload is registered later
            memberFunction.argCount = argCount - 2;
            proto[methodName] = memberFunction;
          } else {
            proto[methodName].overloadTable[argCount - 2] = memberFunction;
          }
  
          return [];
        });
        return [];
      });
    };

  
  var emval_freelist = [];
  
  var emval_handles = [];
  var __emval_decref = (handle) => {
      if (handle > 9 && 0 === --emval_handles[handle + 1]) {
        assert(emval_handles[handle] !== undefined, `Decref for unallocated handle.`);
        emval_handles[handle] = undefined;
        emval_freelist.push(handle);
      }
    };
  
  
  
  
  
  var count_emval_handles = () => {
      return emval_handles.length / 2 - 5 - emval_freelist.length;
    };
  
  var init_emval = () => {
      // reserve 0 and some special values. These never get de-allocated.
      emval_handles.push(
        0, 1,
        undefined, 1,
        null, 1,
        true, 1,
        false, 1,
      );
      assert(emval_handles.length === 5 * 2);
      Module['count_emval_handles'] = count_emval_handles;
    };
  var Emval = {
  toValue:(handle) => {
        if (!handle) {
            throwBindingError('Cannot use deleted val. handle = ' + handle);
        }
        // handle 2 is supposed to be `undefined`.
        assert(handle === 2 || emval_handles[handle] !== undefined && handle % 2 === 0, `invalid handle: ${handle}`);
        return emval_handles[handle];
      },
  toHandle:(value) => {
        switch (value) {
          case undefined: return 2;
          case null: return 4;
          case true: return 6;
          case false: return 8;
          default:{
            const handle = emval_freelist.pop() || emval_handles.length;
            emval_handles[handle] = value;
            emval_handles[handle + 1] = 1;
            return handle;
          }
        }
      },
  };
  
  
  var EmValType = {
      name: 'emscripten::val',
      'fromWireType': (handle) => {
        var rv = Emval.toValue(handle);
        __emval_decref(handle);
        return rv;
      },
      'toWireType': (destructors, value) => Emval.toHandle(value),
      argPackAdvance: GenericWireTypeSize,
      'readValueFromPointer': readPointer,
      destructorFunction: null, // This type does not need a destructor
  
      // TODO: do we need a deleteObject here?  write a test where
      // emval is passed into JS via an interface
    };
  var __embind_register_emval = (rawType) => registerType(rawType, EmValType);

  
  var floatReadValueFromPointer = (name, width) => {
      switch (width) {
          case 4: return function(pointer) {
              return this['fromWireType'](HEAPF32[((pointer)>>2)]);
          };
          case 8: return function(pointer) {
              return this['fromWireType'](HEAPF64[((pointer)>>3)]);
          };
          default:
              throw new TypeError(`invalid float width (${width}): ${name}`);
      }
    };
  
  
  var __embind_register_float = (rawType, name, size) => {
      name = readLatin1String(name);
      registerType(rawType, {
        name,
        'fromWireType': (value) => value,
        'toWireType': (destructors, value) => {
          if (typeof value != "number" && typeof value != "boolean") {
            throw new TypeError(`Cannot convert ${embindRepr(value)} to ${this.name}`);
          }
          // The VM will perform JS to Wasm value conversion, according to the spec:
          // https://www.w3.org/TR/wasm-js-api-1/#towebassemblyvalue
          return value;
        },
        argPackAdvance: GenericWireTypeSize,
        'readValueFromPointer': floatReadValueFromPointer(name, size),
        destructorFunction: null, // This type does not need a destructor
      });
    };

  
  
  
  /** @suppress {globalThis} */
  var __embind_register_integer = (primitiveType, name, size, minRange, maxRange) => {
      name = readLatin1String(name);
      // LLVM doesn't have signed and unsigned 32-bit types, so u32 literals come
      // out as 'i32 -1'. Always treat those as max u32.
      if (maxRange === -1) {
        maxRange = 4294967295;
      }
  
      var fromWireType = (value) => value;
  
      if (minRange === 0) {
        var bitshift = 32 - 8*size;
        fromWireType = (value) => (value << bitshift) >>> bitshift;
      }
  
      var isUnsignedType = (name.includes('unsigned'));
      var checkAssertions = (value, toTypeName) => {
        if (typeof value != "number" && typeof value != "boolean") {
          throw new TypeError(`Cannot convert "${embindRepr(value)}" to ${toTypeName}`);
        }
        if (value < minRange || value > maxRange) {
          throw new TypeError(`Passing a number "${embindRepr(value)}" from JS side to C/C++ side to an argument of type "${name}", which is outside the valid range [${minRange}, ${maxRange}]!`);
        }
      }
      var toWireType;
      if (isUnsignedType) {
        toWireType = function(destructors, value) {
          checkAssertions(value, this.name);
          return value >>> 0;
        }
      } else {
        toWireType = function(destructors, value) {
          checkAssertions(value, this.name);
          // The VM will perform JS to Wasm value conversion, according to the spec:
          // https://www.w3.org/TR/wasm-js-api-1/#towebassemblyvalue
          return value;
        }
      }
      registerType(primitiveType, {
        name,
        'fromWireType': fromWireType,
        'toWireType': toWireType,
        argPackAdvance: GenericWireTypeSize,
        'readValueFromPointer': integerReadValueFromPointer(name, size, minRange !== 0),
        destructorFunction: null, // This type does not need a destructor
      });
    };

  
  var __embind_register_memory_view = (rawType, dataTypeIndex, name) => {
      var typeMapping = [
        Int8Array,
        Uint8Array,
        Int16Array,
        Uint16Array,
        Int32Array,
        Uint32Array,
        Float32Array,
        Float64Array,
        BigInt64Array,
        BigUint64Array,
      ];
  
      var TA = typeMapping[dataTypeIndex];
  
      function decodeMemoryView(handle) {
        var size = HEAPU32[((handle)>>2)];
        var data = HEAPU32[(((handle)+(4))>>2)];
        return new TA(HEAP8.buffer, data, size);
      }
  
      name = readLatin1String(name);
      registerType(rawType, {
        name,
        'fromWireType': decodeMemoryView,
        argPackAdvance: GenericWireTypeSize,
        'readValueFromPointer': decodeMemoryView,
      }, {
        ignoreDuplicateRegistrations: true,
      });
    };

  
  
  
  
  var stringToUTF8Array = (str, heap, outIdx, maxBytesToWrite) => {
      assert(typeof str === 'string', `stringToUTF8Array expects a string (got ${typeof str})`);
      // Parameter maxBytesToWrite is not optional. Negative values, 0, null,
      // undefined and false each don't write out any bytes.
      if (!(maxBytesToWrite > 0))
        return 0;
  
      var startIdx = outIdx;
      var endIdx = outIdx + maxBytesToWrite - 1; // -1 for string null terminator.
      for (var i = 0; i < str.length; ++i) {
        // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code
        // unit, not a Unicode code point of the character! So decode
        // UTF16->UTF32->UTF8.
        // See http://unicode.org/faq/utf_bom.html#utf16-3
        // For UTF8 byte structure, see http://en.wikipedia.org/wiki/UTF-8#Description
        // and https://www.ietf.org/rfc/rfc2279.txt
        // and https://tools.ietf.org/html/rfc3629
        var u = str.charCodeAt(i); // possibly a lead surrogate
        if (u >= 0xD800 && u <= 0xDFFF) {
          var u1 = str.charCodeAt(++i);
          u = 0x10000 + ((u & 0x3FF) << 10) | (u1 & 0x3FF);
        }
        if (u <= 0x7F) {
          if (outIdx >= endIdx) break;
          heap[outIdx++] = u;
        } else if (u <= 0x7FF) {
          if (outIdx + 1 >= endIdx) break;
          heap[outIdx++] = 0xC0 | (u >> 6);
          heap[outIdx++] = 0x80 | (u & 63);
        } else if (u <= 0xFFFF) {
          if (outIdx + 2 >= endIdx) break;
          heap[outIdx++] = 0xE0 | (u >> 12);
          heap[outIdx++] = 0x80 | ((u >> 6) & 63);
          heap[outIdx++] = 0x80 | (u & 63);
        } else {
          if (outIdx + 3 >= endIdx) break;
          if (u > 0x10FFFF) warnOnce('Invalid Unicode code point ' + ptrToString(u) + ' encountered when serializing a JS string to a UTF-8 string in wasm memory! (Valid unicode code points should be in range 0-0x10FFFF).');
          heap[outIdx++] = 0xF0 | (u >> 18);
          heap[outIdx++] = 0x80 | ((u >> 12) & 63);
          heap[outIdx++] = 0x80 | ((u >> 6) & 63);
          heap[outIdx++] = 0x80 | (u & 63);
        }
      }
      // Null-terminate the pointer to the buffer.
      heap[outIdx] = 0;
      return outIdx - startIdx;
    };
  var stringToUTF8 = (str, outPtr, maxBytesToWrite) => {
      assert(typeof maxBytesToWrite == 'number', 'stringToUTF8(str, outPtr, maxBytesToWrite) is missing the third parameter that specifies the length of the output buffer!');
      return stringToUTF8Array(str, HEAPU8, outPtr, maxBytesToWrite);
    };
  
  var lengthBytesUTF8 = (str) => {
      var len = 0;
      for (var i = 0; i < str.length; ++i) {
        // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code
        // unit, not a Unicode code point of the character! So decode
        // UTF16->UTF32->UTF8.
        // See http://unicode.org/faq/utf_bom.html#utf16-3
        var c = str.charCodeAt(i); // possibly a lead surrogate
        if (c <= 0x7F) {
          len++;
        } else if (c <= 0x7FF) {
          len += 2;
        } else if (c >= 0xD800 && c <= 0xDFFF) {
          len += 4; ++i;
        } else {
          len += 3;
        }
      }
      return len;
    };
  
  
  
  var __embind_register_std_string = (rawType, name) => {
      name = readLatin1String(name);
      var stdStringIsUTF8
      = true;
  
      registerType(rawType, {
        name,
        // For some method names we use string keys here since they are part of
        // the public/external API and/or used by the runtime-generated code.
        'fromWireType'(value) {
          var length = HEAPU32[((value)>>2)];
          var payload = value + 4;
  
          var str;
          if (stdStringIsUTF8) {
            var decodeStartPtr = payload;
            // Looping here to support possible embedded '0' bytes
            for (var i = 0; i <= length; ++i) {
              var currentBytePtr = payload + i;
              if (i == length || HEAPU8[currentBytePtr] == 0) {
                var maxRead = currentBytePtr - decodeStartPtr;
                var stringSegment = UTF8ToString(decodeStartPtr, maxRead);
                if (str === undefined) {
                  str = stringSegment;
                } else {
                  str += String.fromCharCode(0);
                  str += stringSegment;
                }
                decodeStartPtr = currentBytePtr + 1;
              }
            }
          } else {
            var a = new Array(length);
            for (var i = 0; i < length; ++i) {
              a[i] = String.fromCharCode(HEAPU8[payload + i]);
            }
            str = a.join('');
          }
  
          _free(value);
  
          return str;
        },
        'toWireType'(destructors, value) {
          if (value instanceof ArrayBuffer) {
            value = new Uint8Array(value);
          }
  
          var length;
          var valueIsOfTypeString = (typeof value == 'string');
  
          if (!(valueIsOfTypeString || value instanceof Uint8Array || value instanceof Uint8ClampedArray || value instanceof Int8Array)) {
            throwBindingError('Cannot pass non-string to std::string');
          }
          if (stdStringIsUTF8 && valueIsOfTypeString) {
            length = lengthBytesUTF8(value);
          } else {
            length = value.length;
          }
  
          // assumes POINTER_SIZE alignment
          var base = _malloc(4 + length + 1);
          var ptr = base + 4;
          HEAPU32[((base)>>2)] = length;
          if (stdStringIsUTF8 && valueIsOfTypeString) {
            stringToUTF8(value, ptr, length + 1);
          } else {
            if (valueIsOfTypeString) {
              for (var i = 0; i < length; ++i) {
                var charCode = value.charCodeAt(i);
                if (charCode > 255) {
                  _free(base);
                  throwBindingError('String has UTF-16 code units that do not fit in 8 bits');
                }
                HEAPU8[ptr + i] = charCode;
              }
            } else {
              for (var i = 0; i < length; ++i) {
                HEAPU8[ptr + i] = value[i];
              }
            }
          }
  
          if (destructors !== null) {
            destructors.push(_free, base);
          }
          return base;
        },
        argPackAdvance: GenericWireTypeSize,
        'readValueFromPointer': readPointer,
        destructorFunction(ptr) {
          _free(ptr);
        },
      });
    };

  
  
  
  var UTF16Decoder = typeof TextDecoder != 'undefined' ? new TextDecoder('utf-16le') : undefined;;
  var UTF16ToString = (ptr, maxBytesToRead) => {
      assert(ptr % 2 == 0, 'Pointer passed to UTF16ToString must be aligned to two bytes!');
      var endPtr = ptr;
      // TextDecoder needs to know the byte length in advance, it doesn't stop on
      // null terminator by itself.
      // Also, use the length info to avoid running tiny strings through
      // TextDecoder, since .subarray() allocates garbage.
      var idx = endPtr >> 1;
      var maxIdx = idx + maxBytesToRead / 2;
      // If maxBytesToRead is not passed explicitly, it will be undefined, and this
      // will always evaluate to true. This saves on code size.
      while (!(idx >= maxIdx) && HEAPU16[idx]) ++idx;
      endPtr = idx << 1;
  
      if (endPtr - ptr > 32 && UTF16Decoder)
        return UTF16Decoder.decode(HEAPU8.slice(ptr, endPtr));
  
      // Fallback: decode without UTF16Decoder
      var str = '';
  
      // If maxBytesToRead is not passed explicitly, it will be undefined, and the
      // for-loop's condition will always evaluate to true. The loop is then
      // terminated on the first null char.
      for (var i = 0; !(i >= maxBytesToRead / 2); ++i) {
        var codeUnit = HEAP16[(((ptr)+(i*2))>>1)];
        if (codeUnit == 0) break;
        // fromCharCode constructs a character from a UTF-16 code unit, so we can
        // pass the UTF16 string right through.
        str += String.fromCharCode(codeUnit);
      }
  
      return str;
    };
  
  var stringToUTF16 = (str, outPtr, maxBytesToWrite) => {
      assert(outPtr % 2 == 0, 'Pointer passed to stringToUTF16 must be aligned to two bytes!');
      assert(typeof maxBytesToWrite == 'number', 'stringToUTF16(str, outPtr, maxBytesToWrite) is missing the third parameter that specifies the length of the output buffer!');
      // Backwards compatibility: if max bytes is not specified, assume unsafe unbounded write is allowed.
      maxBytesToWrite ??= 0x7FFFFFFF;
      if (maxBytesToWrite < 2) return 0;
      maxBytesToWrite -= 2; // Null terminator.
      var startPtr = outPtr;
      var numCharsToWrite = (maxBytesToWrite < str.length*2) ? (maxBytesToWrite / 2) : str.length;
      for (var i = 0; i < numCharsToWrite; ++i) {
        // charCodeAt returns a UTF-16 encoded code unit, so it can be directly written to the HEAP.
        var codeUnit = str.charCodeAt(i); // possibly a lead surrogate
        HEAP16[((outPtr)>>1)] = codeUnit;
        outPtr += 2;
      }
      // Null-terminate the pointer to the HEAP.
      HEAP16[((outPtr)>>1)] = 0;
      return outPtr - startPtr;
    };
  
  var lengthBytesUTF16 = (str) => str.length*2;
  
  var UTF32ToString = (ptr, maxBytesToRead) => {
      assert(ptr % 4 == 0, 'Pointer passed to UTF32ToString must be aligned to four bytes!');
      var i = 0;
  
      var str = '';
      // If maxBytesToRead is not passed explicitly, it will be undefined, and this
      // will always evaluate to true. This saves on code size.
      while (!(i >= maxBytesToRead / 4)) {
        var utf32 = HEAP32[(((ptr)+(i*4))>>2)];
        if (utf32 == 0) break;
        ++i;
        // Gotcha: fromCharCode constructs a character from a UTF-16 encoded code (pair), not from a Unicode code point! So encode the code point to UTF-16 for constructing.
        // See http://unicode.org/faq/utf_bom.html#utf16-3
        if (utf32 >= 0x10000) {
          var ch = utf32 - 0x10000;
          str += String.fromCharCode(0xD800 | (ch >> 10), 0xDC00 | (ch & 0x3FF));
        } else {
          str += String.fromCharCode(utf32);
        }
      }
      return str;
    };
  
  var stringToUTF32 = (str, outPtr, maxBytesToWrite) => {
      assert(outPtr % 4 == 0, 'Pointer passed to stringToUTF32 must be aligned to four bytes!');
      assert(typeof maxBytesToWrite == 'number', 'stringToUTF32(str, outPtr, maxBytesToWrite) is missing the third parameter that specifies the length of the output buffer!');
      // Backwards compatibility: if max bytes is not specified, assume unsafe unbounded write is allowed.
      maxBytesToWrite ??= 0x7FFFFFFF;
      if (maxBytesToWrite < 4) return 0;
      var startPtr = outPtr;
      var endPtr = startPtr + maxBytesToWrite - 4;
      for (var i = 0; i < str.length; ++i) {
        // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code unit, not a Unicode code point of the character! We must decode the string to UTF-32 to the heap.
        // See http://unicode.org/faq/utf_bom.html#utf16-3
        var codeUnit = str.charCodeAt(i); // possibly a lead surrogate
        if (codeUnit >= 0xD800 && codeUnit <= 0xDFFF) {
          var trailSurrogate = str.charCodeAt(++i);
          codeUnit = 0x10000 + ((codeUnit & 0x3FF) << 10) | (trailSurrogate & 0x3FF);
        }
        HEAP32[((outPtr)>>2)] = codeUnit;
        outPtr += 4;
        if (outPtr + 4 > endPtr) break;
      }
      // Null-terminate the pointer to the HEAP.
      HEAP32[((outPtr)>>2)] = 0;
      return outPtr - startPtr;
    };
  
  var lengthBytesUTF32 = (str) => {
      var len = 0;
      for (var i = 0; i < str.length; ++i) {
        // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code unit, not a Unicode code point of the character! We must decode the string to UTF-32 to the heap.
        // See http://unicode.org/faq/utf_bom.html#utf16-3
        var codeUnit = str.charCodeAt(i);
        if (codeUnit >= 0xD800 && codeUnit <= 0xDFFF) ++i; // possibly a lead surrogate, so skip over the tail surrogate.
        len += 4;
      }
  
      return len;
    };
  var __embind_register_std_wstring = (rawType, charSize, name) => {
      name = readLatin1String(name);
      var decodeString, encodeString, readCharAt, lengthBytesUTF;
      if (charSize === 2) {
        decodeString = UTF16ToString;
        encodeString = stringToUTF16;
        lengthBytesUTF = lengthBytesUTF16;
        readCharAt = (pointer) => HEAPU16[((pointer)>>1)];
      } else if (charSize === 4) {
        decodeString = UTF32ToString;
        encodeString = stringToUTF32;
        lengthBytesUTF = lengthBytesUTF32;
        readCharAt = (pointer) => HEAPU32[((pointer)>>2)];
      }
      registerType(rawType, {
        name,
        'fromWireType': (value) => {
          // Code mostly taken from _embind_register_std_string fromWireType
          var length = HEAPU32[((value)>>2)];
          var str;
  
          var decodeStartPtr = value + 4;
          // Looping here to support possible embedded '0' bytes
          for (var i = 0; i <= length; ++i) {
            var currentBytePtr = value + 4 + i * charSize;
            if (i == length || readCharAt(currentBytePtr) == 0) {
              var maxReadBytes = currentBytePtr - decodeStartPtr;
              var stringSegment = decodeString(decodeStartPtr, maxReadBytes);
              if (str === undefined) {
                str = stringSegment;
              } else {
                str += String.fromCharCode(0);
                str += stringSegment;
              }
              decodeStartPtr = currentBytePtr + charSize;
            }
          }
  
          _free(value);
  
          return str;
        },
        'toWireType': (destructors, value) => {
          if (!(typeof value == 'string')) {
            throwBindingError(`Cannot pass non-string to C++ string type ${name}`);
          }
  
          // assumes POINTER_SIZE alignment
          var length = lengthBytesUTF(value);
          var ptr = _malloc(4 + length + charSize);
          HEAPU32[((ptr)>>2)] = length / charSize;
  
          encodeString(value, ptr + 4, length + charSize);
  
          if (destructors !== null) {
            destructors.push(_free, ptr);
          }
          return ptr;
        },
        argPackAdvance: GenericWireTypeSize,
        'readValueFromPointer': readPointer,
        destructorFunction(ptr) {
          _free(ptr);
        }
      });
    };

  
  var __embind_register_void = (rawType, name) => {
      name = readLatin1String(name);
      registerType(rawType, {
        isVoid: true, // void return values can be optimized out sometimes
        name,
        argPackAdvance: 0,
        'fromWireType': () => undefined,
        // TODO: assert if anything else is given?
        'toWireType': (destructors, o) => undefined,
      });
    };

  var __emscripten_init_main_thread_js = (tb) => {
      // Pass the thread address to the native code where they stored in wasm
      // globals which act as a form of TLS. Global constructors trying
      // to access this value will read the wrong value, but that is UB anyway.
      __emscripten_thread_init(
        tb,
        /*is_main=*/!ENVIRONMENT_IS_WORKER,
        /*is_runtime=*/1,
        /*can_block=*/!ENVIRONMENT_IS_WEB,
        /*default_stacksize=*/65536,
        /*start_profiling=*/false,
      );
      PThread.threadInitTLS();
    };

  
  
  
  var __emscripten_thread_mailbox_await = (pthread_ptr) => {
      if (typeof Atomics.waitAsync === 'function') {
        // Wait on the pthread's initial self-pointer field because it is easy and
        // safe to access from sending threads that need to notify the waiting
        // thread.
        // TODO: How to make this work with wasm64?
        var wait = Atomics.waitAsync(HEAP32, ((pthread_ptr)>>2), pthread_ptr);
        assert(wait.async);
        wait.value.then(checkMailbox);
        var waitingAsync = pthread_ptr + 128;
        Atomics.store(HEAP32, ((waitingAsync)>>2), 1);
      }
      // If `Atomics.waitAsync` is not implemented, then we will always fall back
      // to postMessage and there is no need to do anything here.
    };
  
  var checkMailbox = () => {
      // Only check the mailbox if we have a live pthread runtime. We implement
      // pthread_self to return 0 if there is no live runtime.
      var pthread_ptr = _pthread_self();
      if (pthread_ptr) {
        // If we are using Atomics.waitAsync as our notification mechanism, wait
        // for a notification before processing the mailbox to avoid missing any
        // work that could otherwise arrive after we've finished processing the
        // mailbox and before we're ready for the next notification.
        __emscripten_thread_mailbox_await(pthread_ptr);
        callUserCallback(__emscripten_check_mailbox);
      }
    };
  
  var __emscripten_notify_mailbox_postmessage = (targetThread, currThreadId) => {
      if (targetThread == currThreadId) {
        setTimeout(checkMailbox);
      } else if (ENVIRONMENT_IS_PTHREAD) {
        postMessage({targetThread, cmd: 'checkMailbox'});
      } else {
        var worker = PThread.pthreads[targetThread];
        if (!worker) {
          err(`Cannot send message to thread with ID ${targetThread}, unknown thread ID!`);
          return;
        }
        worker.postMessage({cmd: 'checkMailbox'});
      }
    };

  
  var proxiedJSCallArgs = [];
  
  var __emscripten_receive_on_main_thread_js = (funcIndex, emAsmAddr, callingThread, numCallArgs, args) => {
      // Sometimes we need to backproxy events to the calling thread (e.g.
      // HTML5 DOM events handlers such as
      // emscripten_set_mousemove_callback()), so keep track in a globally
      // accessible variable about the thread that initiated the proxying.
      numCallArgs /= 2;
      proxiedJSCallArgs.length = numCallArgs;
      var b = ((args)>>3);
      for (var i = 0; i < numCallArgs; i++) {
        if (HEAP64[b + 2*i]) {
          // It's a BigInt.
          proxiedJSCallArgs[i] = HEAP64[b + 2*i + 1];
        } else {
          // It's a Number.
          proxiedJSCallArgs[i] = HEAPF64[b + 2*i + 1];
        }
      }
      // Proxied JS library funcs use funcIndex and EM_ASM functions use emAsmAddr
      var func = emAsmAddr ? ASM_CONSTS[emAsmAddr] : proxiedFunctionTable[funcIndex];
      assert(!(funcIndex && emAsmAddr));
      assert(func.length == numCallArgs, 'Call args mismatch in _emscripten_receive_on_main_thread_js');
      PThread.currentProxiedOperationCallerThread = callingThread;
      var rtn = func(...proxiedJSCallArgs);
      PThread.currentProxiedOperationCallerThread = 0;
      // Proxied functions can return any type except bigint.  All other types
      // cooerce to f64/double (the return type of this function in C) but not
      // bigint.
      assert(typeof rtn != "bigint");
      return rtn;
    };

  var __emscripten_runtime_keepalive_clear = () => {
      noExitRuntime = false;
      runtimeKeepaliveCounter = 0;
    };

  var __emscripten_system = (command) => {
      if (ENVIRONMENT_IS_NODE) {
        if (!command) return 1; // shell is available
  
        var cmdstr = UTF8ToString(command);
        if (!cmdstr.length) return 0; // this is what glibc seems to do (shell works test?)
  
        var cp = require('child_process');
        var ret = cp.spawnSync(cmdstr, [], {shell:true, stdio:'inherit'});
  
        var _W_EXITCODE = (ret, sig) => ((ret) << 8 | (sig));
  
        // this really only can happen if process is killed by signal
        if (ret.status === null) {
          // sadly node doesn't expose such function
          var signalToNumber = (sig) => {
            // implement only the most common ones, and fallback to SIGINT
            switch (sig) {
              case 'SIGHUP': return 1;
              case 'SIGQUIT': return 3;
              case 'SIGFPE': return 8;
              case 'SIGKILL': return 9;
              case 'SIGALRM': return 14;
              case 'SIGTERM': return 15;
              default: return 2;
            }
          }
          return _W_EXITCODE(0, signalToNumber(ret.signal));
        }
  
        return _W_EXITCODE(ret.status, 0);
      }
      // int system(const char *command);
      // http://pubs.opengroup.org/onlinepubs/000095399/functions/system.html
      // Can't call external programs.
      if (!command) return 0; // no shell available
      return -52;
    };

  var __emscripten_thread_cleanup = (thread) => {
      // Called when a thread needs to be cleaned up so it can be reused.
      // A thread is considered reusable when it either returns from its
      // entry point, calls pthread_exit, or acts upon a cancellation.
      // Detached threads are responsible for calling this themselves,
      // otherwise pthread_join is responsible for calling this.
      if (!ENVIRONMENT_IS_PTHREAD) cleanupThread(thread);
      else postMessage({ cmd: 'cleanupThread', thread });
    };


  var __emscripten_thread_set_strongref = (thread) => {
      // Called when a thread needs to be strongly referenced.
      // Currently only used for:
      // - keeping the "main" thread alive in PROXY_TO_PTHREAD mode;
      // - crashed threads that needs to propagate the uncaught exception
      //   back to the main thread.
      if (ENVIRONMENT_IS_NODE) {
        PThread.pthreads[thread].ref();
      }
    };

  var __emscripten_throw_longjmp = () => {
      throw Infinity;
    };

  function __gmtime_js(time, tmPtr) {
    time = bigintToI53Checked(time);
  
    
      var date = new Date(time * 1000);
      HEAP32[((tmPtr)>>2)] = date.getUTCSeconds();
      HEAP32[(((tmPtr)+(4))>>2)] = date.getUTCMinutes();
      HEAP32[(((tmPtr)+(8))>>2)] = date.getUTCHours();
      HEAP32[(((tmPtr)+(12))>>2)] = date.getUTCDate();
      HEAP32[(((tmPtr)+(16))>>2)] = date.getUTCMonth();
      HEAP32[(((tmPtr)+(20))>>2)] = date.getUTCFullYear()-1900;
      HEAP32[(((tmPtr)+(24))>>2)] = date.getUTCDay();
      var start = Date.UTC(date.getUTCFullYear(), 0, 1, 0, 0, 0, 0);
      var yday = ((date.getTime() - start) / (1000 * 60 * 60 * 24))|0;
      HEAP32[(((tmPtr)+(28))>>2)] = yday;
    ;
  }

  var isLeapYear = (year) => year%4 === 0 && (year%100 !== 0 || year%400 === 0);
  
  var MONTH_DAYS_LEAP_CUMULATIVE = [0,31,60,91,121,152,182,213,244,274,305,335];
  
  var MONTH_DAYS_REGULAR_CUMULATIVE = [0,31,59,90,120,151,181,212,243,273,304,334];
  var ydayFromDate = (date) => {
      var leap = isLeapYear(date.getFullYear());
      var monthDaysCumulative = (leap ? MONTH_DAYS_LEAP_CUMULATIVE : MONTH_DAYS_REGULAR_CUMULATIVE);
      var yday = monthDaysCumulative[date.getMonth()] + date.getDate() - 1; // -1 since it's days since Jan 1
  
      return yday;
    };
  
  function __localtime_js(time, tmPtr) {
    time = bigintToI53Checked(time);
  
    
      var date = new Date(time*1000);
      HEAP32[((tmPtr)>>2)] = date.getSeconds();
      HEAP32[(((tmPtr)+(4))>>2)] = date.getMinutes();
      HEAP32[(((tmPtr)+(8))>>2)] = date.getHours();
      HEAP32[(((tmPtr)+(12))>>2)] = date.getDate();
      HEAP32[(((tmPtr)+(16))>>2)] = date.getMonth();
      HEAP32[(((tmPtr)+(20))>>2)] = date.getFullYear()-1900;
      HEAP32[(((tmPtr)+(24))>>2)] = date.getDay();
  
      var yday = ydayFromDate(date)|0;
      HEAP32[(((tmPtr)+(28))>>2)] = yday;
      HEAP32[(((tmPtr)+(36))>>2)] = -(date.getTimezoneOffset() * 60);
  
      // Attention: DST is in December in South, and some regions don't have DST at all.
      var start = new Date(date.getFullYear(), 0, 1);
      var summerOffset = new Date(date.getFullYear(), 6, 1).getTimezoneOffset();
      var winterOffset = start.getTimezoneOffset();
      var dst = (summerOffset != winterOffset && date.getTimezoneOffset() == Math.min(winterOffset, summerOffset))|0;
      HEAP32[(((tmPtr)+(32))>>2)] = dst;
    ;
  }

  
  var __mktime_js = function(tmPtr) {
  
    var ret = (() => { 
      var date = new Date(HEAP32[(((tmPtr)+(20))>>2)] + 1900,
                          HEAP32[(((tmPtr)+(16))>>2)],
                          HEAP32[(((tmPtr)+(12))>>2)],
                          HEAP32[(((tmPtr)+(8))>>2)],
                          HEAP32[(((tmPtr)+(4))>>2)],
                          HEAP32[((tmPtr)>>2)],
                          0);
  
      // There's an ambiguous hour when the time goes back; the tm_isdst field is
      // used to disambiguate it.  Date() basically guesses, so we fix it up if it
      // guessed wrong, or fill in tm_isdst with the guess if it's -1.
      var dst = HEAP32[(((tmPtr)+(32))>>2)];
      var guessedOffset = date.getTimezoneOffset();
      var start = new Date(date.getFullYear(), 0, 1);
      var summerOffset = new Date(date.getFullYear(), 6, 1).getTimezoneOffset();
      var winterOffset = start.getTimezoneOffset();
      var dstOffset = Math.min(winterOffset, summerOffset); // DST is in December in South
      if (dst < 0) {
        // Attention: some regions don't have DST at all.
        HEAP32[(((tmPtr)+(32))>>2)] = Number(summerOffset != winterOffset && dstOffset == guessedOffset);
      } else if ((dst > 0) != (dstOffset == guessedOffset)) {
        var nonDstOffset = Math.max(winterOffset, summerOffset);
        var trueOffset = dst > 0 ? dstOffset : nonDstOffset;
        // Don't try setMinutes(date.getMinutes() + ...) -- it's messed up.
        date.setTime(date.getTime() + (trueOffset - guessedOffset)*60000);
      }
  
      HEAP32[(((tmPtr)+(24))>>2)] = date.getDay();
      var yday = ydayFromDate(date)|0;
      HEAP32[(((tmPtr)+(28))>>2)] = yday;
      // To match expected behavior, update fields from date
      HEAP32[((tmPtr)>>2)] = date.getSeconds();
      HEAP32[(((tmPtr)+(4))>>2)] = date.getMinutes();
      HEAP32[(((tmPtr)+(8))>>2)] = date.getHours();
      HEAP32[(((tmPtr)+(12))>>2)] = date.getDate();
      HEAP32[(((tmPtr)+(16))>>2)] = date.getMonth();
      HEAP32[(((tmPtr)+(20))>>2)] = date.getYear();
  
      var timeMs = date.getTime();
      if (isNaN(timeMs)) {
        return -1;
      }
      // Return time in microseconds
      return timeMs / 1000;
     })();
    return BigInt(ret);
  };

  var timers = {
  };
  
  
  
  var _emscripten_get_now;
      // AudioWorkletGlobalScope does not have performance.now()
      // (https://github.com/WebAudio/web-audio-api/issues/2527), so if building
      // with
      // Audio Worklets enabled, do a dynamic check for its presence.
      if (typeof performance != 'undefined' && performance.now) {
        _emscripten_get_now = () => performance.timeOrigin + performance.now();
      } else {
        _emscripten_get_now = Date.now;
      }
  ;
  
  
  function __setitimer_js(which, timeout_ms) {
  if (ENVIRONMENT_IS_PTHREAD)
    return proxyToMainThread(3, 0, 1, which, timeout_ms);
  
      // First, clear any existing timer.
      if (timers[which]) {
        clearTimeout(timers[which].id);
        delete timers[which];
      }
  
      // A timeout of zero simply cancels the current timeout so we have nothing
      // more to do.
      if (!timeout_ms) return 0;
  
      var id = setTimeout(() => {
        assert(which in timers);
        delete timers[which];
        callUserCallback(() => __emscripten_timeout(which, _emscripten_get_now()));
      }, timeout_ms);
      timers[which] = { id, timeout_ms };
      return 0;
    
  }
  

  
  var __tzset_js = (timezone, daylight, std_name, dst_name) => {
      // TODO: Use (malleable) environment variables instead of system settings.
      var currentYear = new Date().getFullYear();
      var winter = new Date(currentYear, 0, 1);
      var summer = new Date(currentYear, 6, 1);
      var winterOffset = winter.getTimezoneOffset();
      var summerOffset = summer.getTimezoneOffset();
  
      // Local standard timezone offset. Local standard time is not adjusted for
      // daylight savings.  This code uses the fact that getTimezoneOffset returns
      // a greater value during Standard Time versus Daylight Saving Time (DST).
      // Thus it determines the expected output during Standard Time, and it
      // compares whether the output of the given date the same (Standard) or less
      // (DST).
      var stdTimezoneOffset = Math.max(winterOffset, summerOffset);
  
      // timezone is specified as seconds west of UTC ("The external variable
      // `timezone` shall be set to the difference, in seconds, between
      // Coordinated Universal Time (UTC) and local standard time."), the same
      // as returned by stdTimezoneOffset.
      // See http://pubs.opengroup.org/onlinepubs/009695399/functions/tzset.html
      HEAPU32[((timezone)>>2)] = stdTimezoneOffset * 60;
  
      HEAP32[((daylight)>>2)] = Number(winterOffset != summerOffset);
  
      var extractZone = (timezoneOffset) => {
        // Why inverse sign?
        // Read here https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Date/getTimezoneOffset
        var sign = timezoneOffset >= 0 ? "-" : "+";
  
        var absOffset = Math.abs(timezoneOffset)
        var hours = String(Math.floor(absOffset / 60)).padStart(2, "0");
        var minutes = String(absOffset % 60).padStart(2, "0");
  
        return `UTC${sign}${hours}${minutes}`;
      }
  
      var winterName = extractZone(winterOffset);
      var summerName = extractZone(summerOffset);
      assert(winterName);
      assert(summerName);
      assert(lengthBytesUTF8(winterName) <= 16, `timezone name truncated to fit in TZNAME_MAX (${winterName})`);
      assert(lengthBytesUTF8(summerName) <= 16, `timezone name truncated to fit in TZNAME_MAX (${summerName})`);
      if (summerOffset < winterOffset) {
        // Northern hemisphere
        stringToUTF8(winterName, std_name, 17);
        stringToUTF8(summerName, dst_name, 17);
      } else {
        stringToUTF8(winterName, dst_name, 17);
        stringToUTF8(summerName, std_name, 17);
      }
    };

  var __wasmfs_copy_preloaded_file_data = (index, buffer) =>
      HEAPU8.set(wasmFSPreloadedFiles[index].fileData, buffer);

  var wasmFSPreloadedDirs = [];
  var __wasmfs_get_num_preloaded_dirs = () => wasmFSPreloadedDirs.length;

  var wasmFSPreloadedFiles = [];
  
  var wasmFSPreloadingFlushed = false;
  var __wasmfs_get_num_preloaded_files = () => {
      // When this method is called from WasmFS it means that we are about to
      // flush all the preloaded data, so mark that. (There is no call that
      // occurs at the end of that flushing, which would be more natural, but it
      // is fine to mark the flushing here as during the flushing itself no user
      // code can run, so nothing will check whether we have flushed or not.)
      wasmFSPreloadingFlushed = true;
      return wasmFSPreloadedFiles.length;
    };

  var __wasmfs_get_preloaded_child_path = (index, childNameBuffer) => {
      var s = wasmFSPreloadedDirs[index].childName;
      var len = lengthBytesUTF8(s) + 1;
      stringToUTF8(s, childNameBuffer, len);
    };

  var __wasmfs_get_preloaded_file_mode = (index) => wasmFSPreloadedFiles[index].mode;

  var __wasmfs_get_preloaded_file_size = (index) =>
      wasmFSPreloadedFiles[index].fileData.length;

  var __wasmfs_get_preloaded_parent_path = (index, parentPathBuffer) => {
      var s = wasmFSPreloadedDirs[index].parentPath;
      var len = lengthBytesUTF8(s) + 1;
      stringToUTF8(s, parentPathBuffer, len);
    };

  
  var __wasmfs_get_preloaded_path_name = (index, fileNameBuffer) => {
      var s = wasmFSPreloadedFiles[index].pathName;
      var len = lengthBytesUTF8(s) + 1;
      stringToUTF8(s, fileNameBuffer, len);
    };

  var __wasmfs_jsimpl_alloc_file = (backend, file) => {
      assert(wasmFS$backends[backend]);
      return wasmFS$backends[backend].allocFile(file);
    };

  var __wasmfs_jsimpl_free_file = (backend, file) => {
      assert(wasmFS$backends[backend]);
      return wasmFS$backends[backend].freeFile(file);
    };

  var __wasmfs_jsimpl_get_size = (backend, file) => {
      assert(wasmFS$backends[backend]);
      return wasmFS$backends[backend].getSize(file);
    };

  function __wasmfs_jsimpl_read(backend, file, buffer, length, offset) {
    offset = bigintToI53Checked(offset);
  
    
      assert(wasmFS$backends[backend]);
      if (!wasmFS$backends[backend].read) {
        return -28;
      }
      return wasmFS$backends[backend].read(file, buffer, length, offset);
    ;
  }

  function __wasmfs_jsimpl_set_size(backend, file, size) {
    size = bigintToI53Checked(size);
  
    
      assert(wasmFS$backends[backend]);
      return wasmFS$backends[backend].setSize(file, size);
    ;
  }

  function __wasmfs_jsimpl_write(backend, file, buffer, length, offset) {
    offset = bigintToI53Checked(offset);
  
    
      assert(wasmFS$backends[backend]);
      if (!wasmFS$backends[backend].write) {
        return -28;
      }
      return wasmFS$backends[backend].write(file, buffer, length, offset);
    ;
  }

  var FS_stdin_getChar_buffer = [];
  
  
  /** @type {function(string, boolean=, number=)} */
  var intArrayFromString = (stringy, dontAddNull, length) => {
      var len = length > 0 ? length : lengthBytesUTF8(stringy)+1;
      var u8array = new Array(len);
      var numBytesWritten = stringToUTF8Array(stringy, u8array, 0, u8array.length);
      if (dontAddNull) u8array.length = numBytesWritten;
      return u8array;
    };
  var FS_stdin_getChar = () => {
      if (!FS_stdin_getChar_buffer.length) {
        var result = null;
        if (ENVIRONMENT_IS_NODE) {
          // we will read data by chunks of BUFSIZE
          var BUFSIZE = 256;
          var buf = Buffer.alloc(BUFSIZE);
          var bytesRead = 0;
  
          // For some reason we must suppress a closure warning here, even though
          // fd definitely exists on process.stdin, and is even the proper way to
          // get the fd of stdin,
          // https://github.com/nodejs/help/issues/2136#issuecomment-523649904
          // This started to happen after moving this logic out of library_tty.js,
          // so it is related to the surrounding code in some unclear manner.
          /** @suppress {missingProperties} */
          var fd = process.stdin.fd;
  
          try {
            bytesRead = fs.readSync(fd, buf, 0, BUFSIZE);
          } catch(e) {
            // Cross-platform differences: on Windows, reading EOF throws an
            // exception, but on other OSes, reading EOF returns 0. Uniformize
            // behavior by treating the EOF exception to return 0.
            if (e.toString().includes('EOF')) bytesRead = 0;
            else throw e;
          }
  
          if (bytesRead > 0) {
            result = buf.slice(0, bytesRead).toString('utf-8');
          }
        } else
        if (typeof window != 'undefined' &&
          typeof window.prompt == 'function') {
          // Browser.
          result = window.prompt('Input: ');  // returns null on cancel
          if (result !== null) {
            result += '\n';
          }
        } else
        {}
        if (!result) {
          return null;
        }
        FS_stdin_getChar_buffer = intArrayFromString(result, true);
      }
      return FS_stdin_getChar_buffer.shift();
    };
  var __wasmfs_stdin_get_char = () => {
      // Return the read character, or -1 to indicate EOF.
      var c = FS_stdin_getChar();
      if (typeof c === 'number') {
        return c;
      }
      return -1;
    };

  
  var _emscripten_date_now = () => Date.now();
  
  var nowIsMonotonic = 1;
  
  var checkWasiClock = (clock_id) => clock_id >= 0 && clock_id <= 3;
  
  function _clock_time_get(clk_id, ignored_precision, ptime) {
    ignored_precision = bigintToI53Checked(ignored_precision);
  
    
      if (!checkWasiClock(clk_id)) {
        return 28;
      }
      var now;
      // all wasi clocks but realtime are monotonic
      if (clk_id === 0) {
        now = _emscripten_date_now();
      } else if (nowIsMonotonic) {
        now = _emscripten_get_now();
      } else {
        return 52;
      }
      // "now" is in ms, and wasi times are in ns.
      var nsec = Math.round(now * 1000 * 1000);
      HEAP64[((ptime)>>3)] = BigInt(nsec);
      return 0;
    ;
  }

  var readEmAsmArgsArray = [];
  var readEmAsmArgs = (sigPtr, buf) => {
      // Nobody should have mutated _readEmAsmArgsArray underneath us to be something else than an array.
      assert(Array.isArray(readEmAsmArgsArray));
      // The input buffer is allocated on the stack, so it must be stack-aligned.
      assert(buf % 16 == 0);
      readEmAsmArgsArray.length = 0;
      var ch;
      // Most arguments are i32s, so shift the buffer pointer so it is a plain
      // index into HEAP32.
      while (ch = HEAPU8[sigPtr++]) {
        var chr = String.fromCharCode(ch);
        var validChars = ['d', 'f', 'i', 'p'];
        // In WASM_BIGINT mode we support passing i64 values as bigint.
        validChars.push('j');
        assert(validChars.includes(chr), `Invalid character ${ch}("${chr}") in readEmAsmArgs! Use only [${validChars}], and do not specify "v" for void return argument.`);
        // Floats are always passed as doubles, so all types except for 'i'
        // are 8 bytes and require alignment.
        var wide = (ch != 105);
        wide &= (ch != 112);
        buf += wide && (buf % 8) ? 4 : 0;
        readEmAsmArgsArray.push(
          // Special case for pointers under wasm64 or CAN_ADDRESS_2GB mode.
          ch == 112 ? HEAPU32[((buf)>>2)] :
          ch == 106 ? HEAP64[((buf)>>3)] :
          ch == 105 ?
            HEAP32[((buf)>>2)] :
            HEAPF64[((buf)>>3)]
        );
        buf += wide ? 8 : 4;
      }
      return readEmAsmArgsArray;
    };
  
  var runMainThreadEmAsm = (emAsmAddr, sigPtr, argbuf, sync) => {
      var args = readEmAsmArgs(sigPtr, argbuf);
      if (ENVIRONMENT_IS_PTHREAD) {
        // EM_ASM functions are variadic, receiving the actual arguments as a buffer
        // in memory. the last parameter (argBuf) points to that data. We need to
        // always un-variadify that, *before proxying*, as in the async case this
        // is a stack allocation that LLVM made, which may go away before the main
        // thread gets the message. For that reason we handle proxying *after* the
        // call to readEmAsmArgs, and therefore we do that manually here instead
        // of using __proxy. (And dor simplicity, do the same in the sync
        // case as well, even though it's not strictly necessary, to keep the two
        // code paths as similar as possible on both sides.)
        return proxyToMainThread(0, emAsmAddr, sync, ...args);
      }
      assert(ASM_CONSTS.hasOwnProperty(emAsmAddr), `No EM_ASM constant found at address ${emAsmAddr}.  The loaded WebAssembly file is likely out of sync with the generated JavaScript.`);
      return ASM_CONSTS[emAsmAddr](...args);
    };
  var _emscripten_asm_const_async_on_main_thread = (emAsmAddr, sigPtr, argbuf) => runMainThreadEmAsm(emAsmAddr, sigPtr, argbuf, 0);

  var runEmAsmFunction = (code, sigPtr, argbuf) => {
      var args = readEmAsmArgs(sigPtr, argbuf);
      assert(ASM_CONSTS.hasOwnProperty(code), `No EM_ASM constant found at address ${code}.  The loaded WebAssembly file is likely out of sync with the generated JavaScript.`);
      return ASM_CONSTS[code](...args);
    };
  var _emscripten_asm_const_int = (code, sigPtr, argbuf) => {
      return runEmAsmFunction(code, sigPtr, argbuf);
    };

  
  var _emscripten_check_blocking_allowed = () => {
      if (ENVIRONMENT_IS_NODE) return;
  
      if (ENVIRONMENT_IS_WORKER) return; // Blocking in a worker/pthread is fine.
  
      warnOnce('Blocking on the main thread is very dangerous, see https://emscripten.org/docs/porting/pthreads.html#blocking-on-the-main-browser-thread');
  
    };

  var EmAudio = {
  };
  
  var EmAudioCounter = 0;
  var emscriptenRegisterAudioObject = (object) => {
      assert(object, 'Called emscriptenRegisterAudioObject() with a null object handle!');
      EmAudio[++EmAudioCounter] = object;
      return EmAudioCounter;
    };
  
  var emscriptenGetAudioObject = (objectHandle) => EmAudio[objectHandle];
  
  var _emscripten_create_audio_context = (options) => {
      let ctx = window.AudioContext || window.webkitAudioContext;
      if (!ctx) console.error('emscripten_create_audio_context failed! Web Audio is not supported.');
      options >>= 2;
  
      let opts = options ? {
        latencyHint: HEAPU32[options] ? UTF8ToString(HEAPU32[options]) : void 0,
        sampleRate: HEAP32[options+1] || void 0
      } : void 0;
  
      return ctx && emscriptenRegisterAudioObject(new ctx(opts));
    };

  var emscriptenGetContextQuantumSize = (contextHandle) => {
      // TODO: in a future release this will be something like:
      //   return EmAudio[contextHandle].renderQuantumSize || 128;
      // It comes two caveats: it needs the hint when generating the context adding to
      // emscripten_create_audio_context(), and altering the quantum requires a secure
      // context and fallback implementing. Until then we simply use the 1.0 API value:
      return 128;
    };
  
  var _emscripten_create_wasm_audio_worklet_node = (contextHandle, name, options, callback, userData) => {
      assert(contextHandle, `Called emscripten_create_wasm_audio_worklet_node() with a null Web Audio Context handle!`);
      assert(EmAudio[contextHandle], `Called emscripten_create_wasm_audio_worklet_node() with a nonexisting/already freed Web Audio Context handle ${contextHandle}!`);
      assert(EmAudio[contextHandle] instanceof (window.AudioContext || window.webkitAudioContext), `Called emscripten_create_wasm_audio_worklet_node() on a context handle ${contextHandle} that is not an AudioContext, but of type ${typeof EmAudio[contextHandle]}`);
      options >>= 2;
  
      function readChannelCountArray(heapIndex, numOutputs) {
        let channelCounts = [];
        while (numOutputs--) channelCounts.push(HEAPU32[heapIndex++]);
        return channelCounts;
      }
  
      let opts = options ? {
        numberOfInputs: HEAP32[options],
        numberOfOutputs: HEAP32[options+1],
        outputChannelCount: HEAPU32[options+2] ? readChannelCountArray(HEAPU32[options+2]>>2, HEAP32[options+1]) : void 0,
        processorOptions: {
          'cb': callback,
          'ud': userData,
          'sc': emscriptenGetContextQuantumSize(contextHandle)
        }
      } : void 0;
  
      return emscriptenRegisterAudioObject(new AudioWorkletNode(EmAudio[contextHandle], UTF8ToString(name), opts));
    };

  var _emscripten_create_wasm_audio_worklet_processor_async = (contextHandle, options, callback, userData) => {
      assert(contextHandle, `Called emscripten_create_wasm_audio_worklet_processor_async() with a null Web Audio Context handle!`);
      assert(EmAudio[contextHandle], `Called emscripten_create_wasm_audio_worklet_processor_async() with a nonexisting/already freed Web Audio Context handle ${contextHandle}!`);
      assert(EmAudio[contextHandle] instanceof (window.AudioContext || window.webkitAudioContext), `Called emscripten_create_wasm_audio_worklet_processor_async() on a context handle ${contextHandle} that is not an AudioContext, but of type ${typeof EmAudio[contextHandle]}`);
  
      options >>= 2;
      let audioParams = [],
        numAudioParams = HEAPU32[options+1],
        audioParamDescriptors = HEAPU32[options+2] >> 2,
        i = 0;
  
      while (numAudioParams--) {
        audioParams.push({
          name: i++,
          defaultValue: HEAPF32[audioParamDescriptors++],
          minValue: HEAPF32[audioParamDescriptors++],
          maxValue: HEAPF32[audioParamDescriptors++],
          automationRate: ['a','k'][HEAPU32[audioParamDescriptors++]] + '-rate',
        });
      }
  
      EmAudio[contextHandle].audioWorklet.bootstrapMessage.port.postMessage({
        // Deliberately mangled and short names used here ('_wpn', the 'Worklet
        // Processor Name' used as a 'key' to verify the message type so as to
        // not get accidentally mixed with user submitted messages, the remainder
        // for space saving reasons, abbreviated from their variable names).
        '_wpn': UTF8ToString(HEAPU32[options]),
        'ap': audioParams,
        'ch': contextHandle,
        'cb': callback,
        'ud': userData
      });
    };


  var _emscripten_err = (str) => err(UTF8ToString(str));

  var runtimeKeepalivePush = () => {
      runtimeKeepaliveCounter += 1;
    };
  var _emscripten_exit_with_live_runtime = () => {
      runtimeKeepalivePush();
      throw 'unwind';
    };

  var getHeapMax = () =>
      HEAPU8.length;
  var _emscripten_get_heap_max = () => getHeapMax();


  var _emscripten_num_logical_cores = () =>
      ENVIRONMENT_IS_NODE ? require('os').cpus().length :
      navigator['hardwareConcurrency'];

  var _emscripten_out = (str) => out(UTF8ToString(str));

  
  var alignMemory = (size, alignment) => {
      assert(alignment, "alignment argument is required");
      return Math.ceil(size / alignment) * alignment;
    };
  
  var abortOnCannotGrowMemory = (requestedSize) => {
      abort(`Cannot enlarge memory arrays to size ${requestedSize} bytes (OOM). Either (1) compile with -sINITIAL_MEMORY=X with X higher than the current value ${HEAP8.length}, (2) compile with -sALLOW_MEMORY_GROWTH which allows increasing the size at runtime, or (3) if you want malloc to return NULL (0) instead of this abort, compile with -sABORTING_MALLOC=0`);
    };
  var _emscripten_resize_heap = (requestedSize) => {
      var oldSize = HEAPU8.length;
      // With CAN_ADDRESS_2GB or MEMORY64, pointers are already unsigned.
      requestedSize >>>= 0;
      abortOnCannotGrowMemory(requestedSize);
    };

  var _emscripten_resume_audio_context_sync = (contextHandle) => {
      assert(EmAudio[contextHandle], `Called emscripten_resume_audio_context_sync() on a nonexisting context handle ${contextHandle}`);
      assert(EmAudio[contextHandle] instanceof (window.AudioContext || window.webkitAudioContext), `Called emscripten_resume_audio_context_sync() on a context handle ${contextHandle} that is not an AudioContext, but of type ${typeof EmAudio[contextHandle]}`);
      EmAudio[contextHandle].resume();
    };

  
  
  
  var _emscripten_set_main_loop_timing = (mode, value) => {
      MainLoop.timingMode = mode;
      MainLoop.timingValue = value;
  
      if (!MainLoop.func) {
        err('emscripten_set_main_loop_timing: Cannot set timing mode for main loop since a main loop does not exist! Call emscripten_set_main_loop first to set one up.');
        return 1; // Return non-zero on failure, can't set timing mode when there is no main loop.
      }
  
      if (!MainLoop.running) {
        runtimeKeepalivePush();
        MainLoop.running = true;
      }
      if (mode == 0) {
        MainLoop.scheduler = function MainLoop_scheduler_setTimeout() {
          var timeUntilNextTick = Math.max(0, MainLoop.tickStartTime + value - _emscripten_get_now())|0;
          setTimeout(MainLoop.runner, timeUntilNextTick); // doing this each time means that on exception, we stop
        };
        MainLoop.method = 'timeout';
      } else if (mode == 1) {
        MainLoop.scheduler = function MainLoop_scheduler_rAF() {
          MainLoop.requestAnimationFrame(MainLoop.runner);
        };
        MainLoop.method = 'rAF';
      } else if (mode == 2) {
        if (typeof MainLoop.setImmediate == 'undefined') {
          if (typeof setImmediate == 'undefined') {
            // Emulate setImmediate. (note: not a complete polyfill, we don't emulate clearImmediate() to keep code size to minimum, since not needed)
            var setImmediates = [];
            var emscriptenMainLoopMessageId = 'setimmediate';
            /** @param {Event} event */
            var MainLoop_setImmediate_messageHandler = (event) => {
              // When called in current thread or Worker, the main loop ID is structured slightly different to accommodate for --proxy-to-worker runtime listening to Worker events,
              // so check for both cases.
              if (event.data === emscriptenMainLoopMessageId || event.data.target === emscriptenMainLoopMessageId) {
                event.stopPropagation();
                setImmediates.shift()();
              }
            };
            addEventListener("message", MainLoop_setImmediate_messageHandler, true);
            MainLoop.setImmediate = /** @type{function(function(): ?, ...?): number} */((func) => {
              setImmediates.push(func);
              if (ENVIRONMENT_IS_WORKER) {
                Module['setImmediates'] ??= [];
                Module['setImmediates'].push(func);
                postMessage({target: emscriptenMainLoopMessageId}); // In --proxy-to-worker, route the message via proxyClient.js
              } else postMessage(emscriptenMainLoopMessageId, "*"); // On the main thread, can just send the message to itself.
            });
          } else {
            MainLoop.setImmediate = setImmediate;
          }
        }
        MainLoop.scheduler = function MainLoop_scheduler_setImmediate() {
          MainLoop.setImmediate(MainLoop.runner);
        };
        MainLoop.method = 'immediate';
      }
      return 0;
    };
  var MainLoop = {
  running:false,
  scheduler:null,
  method:"",
  currentlyRunningMainloop:0,
  func:null,
  arg:0,
  timingMode:0,
  timingValue:0,
  currentFrameNumber:0,
  queue:[],
  preMainLoop:[],
  postMainLoop:[],
  pause() {
        MainLoop.scheduler = null;
        // Incrementing this signals the previous main loop that it's now become old, and it must return.
        MainLoop.currentlyRunningMainloop++;
      },
  resume() {
        MainLoop.currentlyRunningMainloop++;
        var timingMode = MainLoop.timingMode;
        var timingValue = MainLoop.timingValue;
        var func = MainLoop.func;
        MainLoop.func = null;
        // do not set timing and call scheduler, we will do it on the next lines
        setMainLoop(func, 0, false, MainLoop.arg, true);
        _emscripten_set_main_loop_timing(timingMode, timingValue);
        MainLoop.scheduler();
      },
  updateStatus() {
        if (Module['setStatus']) {
          var message = Module['statusMessage'] || 'Please wait...';
          var remaining = MainLoop.remainingBlockers ?? 0;
          var expected = MainLoop.expectedBlockers ?? 0;
          if (remaining) {
            if (remaining < expected) {
              Module['setStatus'](`{message} ({expected - remaining}/{expected})`);
            } else {
              Module['setStatus'](message);
            }
          } else {
            Module['setStatus']('');
          }
        }
      },
  init() {
        Module['preMainLoop'] && MainLoop.preMainLoop.push(Module['preMainLoop']);
        Module['postMainLoop'] && MainLoop.postMainLoop.push(Module['postMainLoop']);
      },
  runIter(func) {
        if (ABORT) return;
        for (var pre of MainLoop.preMainLoop) {
          if (pre() === false) {
            return; // |return false| skips a frame
          }
        }
        callUserCallback(func);
        for (var post of MainLoop.postMainLoop) {
          post();
        }
        checkStackCookie();
      },
  nextRAF:0,
  fakeRequestAnimationFrame(func) {
        // try to keep 60fps between calls to here
        var now = Date.now();
        if (MainLoop.nextRAF === 0) {
          MainLoop.nextRAF = now + 1000/60;
        } else {
          while (now + 2 >= MainLoop.nextRAF) { // fudge a little, to avoid timer jitter causing us to do lots of delay:0
            MainLoop.nextRAF += 1000/60;
          }
        }
        var delay = Math.max(MainLoop.nextRAF - now, 0);
        setTimeout(func, delay);
      },
  requestAnimationFrame(func) {
        if (typeof requestAnimationFrame == 'function') {
          requestAnimationFrame(func);
          return;
        }
        var RAF = MainLoop.fakeRequestAnimationFrame;
        RAF(func);
      },
  };
  
  
  
  
  var runtimeKeepalivePop = () => {
      assert(runtimeKeepaliveCounter > 0);
      runtimeKeepaliveCounter -= 1;
    };
  
    /**
     * @param {number=} arg
     * @param {boolean=} noSetTiming
     */
  var setMainLoop = (iterFunc, fps, simulateInfiniteLoop, arg, noSetTiming) => {
      assert(!MainLoop.func, 'emscripten_set_main_loop: there can only be one main loop function at once: call emscripten_cancel_main_loop to cancel the previous one before setting a new one with different parameters.');
      MainLoop.func = iterFunc;
      MainLoop.arg = arg;
  
      var thisMainLoopId = MainLoop.currentlyRunningMainloop;
      function checkIsRunning() {
        if (thisMainLoopId < MainLoop.currentlyRunningMainloop) {
          runtimeKeepalivePop();
          maybeExit();
          return false;
        }
        return true;
      }
  
      // We create the loop runner here but it is not actually running until
      // _emscripten_set_main_loop_timing is called (which might happen a
      // later time).  This member signifies that the current runner has not
      // yet been started so that we can call runtimeKeepalivePush when it
      // gets it timing set for the first time.
      MainLoop.running = false;
      MainLoop.runner = function MainLoop_runner() {
        if (ABORT) return;
        if (MainLoop.queue.length > 0) {
          var start = Date.now();
          var blocker = MainLoop.queue.shift();
          blocker.func(blocker.arg);
          if (MainLoop.remainingBlockers) {
            var remaining = MainLoop.remainingBlockers;
            var next = remaining%1 == 0 ? remaining-1 : Math.floor(remaining);
            if (blocker.counted) {
              MainLoop.remainingBlockers = next;
            } else {
              // not counted, but move the progress along a tiny bit
              next = next + 0.5; // do not steal all the next one's progress
              MainLoop.remainingBlockers = (8*remaining + next)/9;
            }
          }
          MainLoop.updateStatus();
  
          // catches pause/resume main loop from blocker execution
          if (!checkIsRunning()) return;
  
          setTimeout(MainLoop.runner, 0);
          return;
        }
  
        // catch pauses from non-main loop sources
        if (!checkIsRunning()) return;
  
        // Implement very basic swap interval control
        MainLoop.currentFrameNumber = MainLoop.currentFrameNumber + 1 | 0;
        if (MainLoop.timingMode == 1 && MainLoop.timingValue > 1 && MainLoop.currentFrameNumber % MainLoop.timingValue != 0) {
          // Not the scheduled time to render this frame - skip.
          MainLoop.scheduler();
          return;
        } else if (MainLoop.timingMode == 0) {
          MainLoop.tickStartTime = _emscripten_get_now();
        }
  
        if (MainLoop.method === 'timeout' && Module['ctx']) {
          warnOnce('Looks like you are rendering without using requestAnimationFrame for the main loop. You should use 0 for the frame rate in emscripten_set_main_loop in order to use requestAnimationFrame, as that can greatly improve your frame rates!');
          MainLoop.method = ''; // just warn once per call to set main loop
        }
  
        MainLoop.runIter(iterFunc);
  
        // catch pauses from the main loop itself
        if (!checkIsRunning()) return;
  
        MainLoop.scheduler();
      }
  
      if (!noSetTiming) {
        if (fps > 0) {
          _emscripten_set_main_loop_timing(0, 1000.0 / fps);
        } else {
          // Do rAF by rendering each frame (no decimating)
          _emscripten_set_main_loop_timing(1, 1);
        }
  
        MainLoop.scheduler();
      }
  
      if (simulateInfiniteLoop) {
        throw 'unwind';
      }
    };
  
  var _emscripten_set_main_loop = (func, fps, simulateInfiniteLoop) => {
      var iterFunc = getWasmTableEntry(func);
      setMainLoop(iterFunc, fps, simulateInfiniteLoop);
    };

  var _wasmWorkersID = 1;
  
  var _EmAudioDispatchProcessorCallback = (e) => {
      let data = e.data;
      // '_wsc' is short for 'wasm call', trying to use an identifier name that
      // will never conflict with user code
      let wasmCall = data['_wsc'];
      wasmCall && getWasmTableEntry(wasmCall)(...data['x']);
    };
  
  
  
  
  var _emscripten_start_wasm_audio_worklet_thread_async = (contextHandle, stackLowestAddress, stackSize, callback, userData) => {
  
      assert(contextHandle, `Called emscripten_start_wasm_audio_worklet_thread_async() with a null Web Audio Context handle!`);
      assert(EmAudio[contextHandle], `Called emscripten_start_wasm_audio_worklet_thread_async() with a nonexisting/already freed Web Audio Context handle ${contextHandle}!`);
      assert(EmAudio[contextHandle] instanceof (window.AudioContext || window.webkitAudioContext), `Called emscripten_start_wasm_audio_worklet_thread_async() on a context handle ${contextHandle} that is not an AudioContext, but of type ${typeof EmAudio[contextHandle]}`);
  
      let audioContext = EmAudio[contextHandle],
        audioWorklet = audioContext.audioWorklet;
  
      assert(stackLowestAddress != 0, 'AudioWorklets require a dedicated stack space for audio data marshalling between Wasm and JS!');
      assert(stackLowestAddress % 16 == 0, `AudioWorklet stack should be aligned to 16 bytes! (was ${stackLowestAddress} == ${stackLowestAddress%16} mod 16) Use e.g. memalign(16, stackSize) to align the stack!`);
      assert(stackSize != 0, 'AudioWorklets require a dedicated stack space for audio data marshalling between Wasm and JS!');
      assert(stackSize % 16 == 0, `AudioWorklet stack size should be a multiple of 16 bytes! (was ${stackSize} == ${stackSize%16} mod 16)`);
      assert(!audioContext.audioWorkletInitialized, 'emscripten_create_wasm_audio_worklet() was already called for AudioContext ' + contextHandle + '! Only call this function once per AudioContext!');
      audioContext.audioWorkletInitialized = 1;
  
      let audioWorkletCreationFailed = () => {
        getWasmTableEntry(callback)(contextHandle, 0/*EM_FALSE*/, userData);
      };
  
      // Does browser not support AudioWorklets?
      if (!audioWorklet) {
        return audioWorkletCreationFailed();
      }
  
      // TODO: In MINIMAL_RUNTIME builds, read this file off of a preloaded Blob,
      // and/or embed from a string like with WASM_WORKERS==2 mode.
      audioWorklet.addModule('pd4web.aw.js').then(() => {
        audioWorklet.bootstrapMessage = new AudioWorkletNode(audioContext, 'message', {
          processorOptions: {
            // Assign the loaded AudioWorkletGlobalScope a Wasm Worker ID so that
            // it can utilized its own TLS slots, and it is recognized to not be
            // the main browser thread.
            '$ww': _wasmWorkersID++,
            'wasm': wasmModule,
            'wasmMemory': wasmMemory,
            'sb': stackLowestAddress, // sb = stack base
            'sz': stackSize,          // sz = stack size
          }
        });
        audioWorklet.bootstrapMessage.port.onmessage = _EmAudioDispatchProcessorCallback;
  
        // AudioWorklets do not have a importScripts() function like Web Workers
        // do (and AudioWorkletGlobalScope does not allow dynamic import()
        // either), but instead, the main thread must load all JS code into the
        // worklet scope. Send the application main JS script to the audio
        // worklet.
        return audioWorklet.addModule(
          Module['mainScriptUrlOrBlob'] || _scriptName
        );
      }).then(() => {
        getWasmTableEntry(callback)(contextHandle, 1/*EM_TRUE*/, userData);
      }).catch(audioWorkletCreationFailed);
    };

  var ENV = {
  };
  
  var getExecutableName = () => thisProgram || './this.program';
  var getEnvStrings = () => {
      if (!getEnvStrings.strings) {
        // Default values.
        // Browser language detection #8751
        var lang = ((typeof navigator == 'object' && navigator.languages && navigator.languages[0]) || 'C').replace('-', '_') + '.UTF-8';
        var env = {
          'USER': 'web_user',
          'LOGNAME': 'web_user',
          'PATH': '/',
          'PWD': '/',
          'HOME': '/home/web_user',
          'LANG': lang,
          '_': getExecutableName()
        };
        // Apply the user-provided values, if any.
        for (var x in ENV) {
          // x is a key in ENV; if ENV[x] is undefined, that means it was
          // explicitly set to be so. We allow user code to do that to
          // force variables with default values to remain unset.
          if (ENV[x] === undefined) delete env[x];
          else env[x] = ENV[x];
        }
        var strings = [];
        for (var x in env) {
          strings.push(`${x}=${env[x]}`);
        }
        getEnvStrings.strings = strings;
      }
      return getEnvStrings.strings;
    };
  
  var stringToAscii = (str, buffer) => {
      for (var i = 0; i < str.length; ++i) {
        assert(str.charCodeAt(i) === (str.charCodeAt(i) & 0xff));
        HEAP8[buffer++] = str.charCodeAt(i);
      }
      // Null-terminate the string
      HEAP8[buffer] = 0;
    };
  
  var _environ_get = 
  function(__environ, environ_buf) {
  if (ENVIRONMENT_IS_PTHREAD)
    return proxyToMainThread(4, 0, 1, __environ, environ_buf);
  
      var bufSize = 0;
      getEnvStrings().forEach((string, i) => {
        var ptr = environ_buf + bufSize;
        HEAPU32[(((__environ)+(i*4))>>2)] = ptr;
        stringToAscii(string, ptr);
        bufSize += string.length + 1;
      });
      return 0;
    
  }
  ;

  
  var _environ_sizes_get = 
  function(penviron_count, penviron_buf_size) {
  if (ENVIRONMENT_IS_PTHREAD)
    return proxyToMainThread(5, 0, 1, penviron_count, penviron_buf_size);
  
      var strings = getEnvStrings();
      HEAPU32[((penviron_count)>>2)] = strings.length;
      var bufSize = 0;
      strings.forEach((string) => bufSize += string.length + 1);
      HEAPU32[((penviron_buf_size)>>2)] = bufSize;
      return 0;
    
  }
  ;


  var Sockets = {
  BUFFER_SIZE:10240,
  MAX_BUFFER_SIZE:10485760,
  nextFd:1,
  fds:{
  },
  nextport:1,
  maxport:65535,
  peer:null,
  connections:{
  },
  portmap:{
  },
  localAddr:4261412874,
  addrPool:[33554442,50331658,67108874,83886090,100663306,117440522,134217738,150994954,167772170,184549386,201326602,218103818,234881034],
  };
  
  var inetPton4 = (str) => {
      var b = str.split('.');
      for (var i = 0; i < 4; i++) {
        var tmp = Number(b[i]);
        if (isNaN(tmp)) return null;
        b[i] = tmp;
      }
      return (b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24)) >>> 0;
    };
  
  
  /** @suppress {checkTypes} */
  var jstoi_q = (str) => parseInt(str);
  var inetPton6 = (str) => {
      var words;
      var w, offset, z, i;
      /* http://home.deds.nl/~aeron/regex/ */
      var valid6regx = /^((?=.*::)(?!.*::.+::)(::)?([\dA-F]{1,4}:(:|\b)|){5}|([\dA-F]{1,4}:){6})((([\dA-F]{1,4}((?!\3)::|:\b|$))|(?!\2\3)){2}|(((2[0-4]|1\d|[1-9])?\d|25[0-5])\.?\b){4})$/i
      var parts = [];
      if (!valid6regx.test(str)) {
        return null;
      }
      if (str === "::") {
        return [0, 0, 0, 0, 0, 0, 0, 0];
      }
      // Z placeholder to keep track of zeros when splitting the string on ":"
      if (str.startsWith("::")) {
        str = str.replace("::", "Z:"); // leading zeros case
      } else {
        str = str.replace("::", ":Z:");
      }
  
      if (str.indexOf(".") > 0) {
        // parse IPv4 embedded stress
        str = str.replace(new RegExp('[.]', 'g'), ":");
        words = str.split(":");
        words[words.length-4] = jstoi_q(words[words.length-4]) + jstoi_q(words[words.length-3])*256;
        words[words.length-3] = jstoi_q(words[words.length-2]) + jstoi_q(words[words.length-1])*256;
        words = words.slice(0, words.length-2);
      } else {
        words = str.split(":");
      }
  
      offset = 0; z = 0;
      for (w=0; w < words.length; w++) {
        if (typeof words[w] == 'string') {
          if (words[w] === 'Z') {
            // compressed zeros - write appropriate number of zero words
            for (z = 0; z < (8 - words.length+1); z++) {
              parts[w+z] = 0;
            }
            offset = z-1;
          } else {
            // parse hex to field to 16-bit value and write it in network byte-order
            parts[w+offset] = _htons(parseInt(words[w],16));
          }
        } else {
          // parsed IPv4 words
          parts[w+offset] = words[w];
        }
      }
      return [
        (parts[1] << 16) | parts[0],
        (parts[3] << 16) | parts[2],
        (parts[5] << 16) | parts[4],
        (parts[7] << 16) | parts[6]
      ];
    };
  var DNS = {
  address_map:{
  id:1,
  addrs:{
  },
  names:{
  },
  },
  lookup_name(name) {
        // If the name is already a valid ipv4 / ipv6 address, don't generate a fake one.
        var res = inetPton4(name);
        if (res !== null) {
          return name;
        }
        res = inetPton6(name);
        if (res !== null) {
          return name;
        }
  
        // See if this name is already mapped.
        var addr;
  
        if (DNS.address_map.addrs[name]) {
          addr = DNS.address_map.addrs[name];
        } else {
          var id = DNS.address_map.id++;
          assert(id < 65535, 'exceeded max address mappings of 65535');
  
          addr = '172.29.' + (id & 0xff) + '.' + (id & 0xff00);
  
          DNS.address_map.names[addr] = name;
          DNS.address_map.addrs[name] = addr;
        }
  
        return addr;
      },
  lookup_addr(addr) {
        if (DNS.address_map.names[addr]) {
          return DNS.address_map.names[addr];
        }
  
        return null;
      },
  };
  
  
  var inetNtop4 = (addr) =>
      (addr & 0xff) + '.' + ((addr >> 8) & 0xff) + '.' + ((addr >> 16) & 0xff) + '.' + ((addr >> 24) & 0xff);
  
  
  
  var inetNtop6 = (ints) => {
      //  ref:  http://www.ietf.org/rfc/rfc2373.txt - section 2.5.4
      //  Format for IPv4 compatible and mapped  128-bit IPv6 Addresses
      //  128-bits are split into eight 16-bit words
      //  stored in network byte order (big-endian)
      //  |                80 bits               | 16 |      32 bits        |
      //  +-----------------------------------------------------------------+
      //  |               10 bytes               |  2 |      4 bytes        |
      //  +--------------------------------------+--------------------------+
      //  +               5 words                |  1 |      2 words        |
      //  +--------------------------------------+--------------------------+
      //  |0000..............................0000|0000|    IPv4 ADDRESS     | (compatible)
      //  +--------------------------------------+----+---------------------+
      //  |0000..............................0000|FFFF|    IPv4 ADDRESS     | (mapped)
      //  +--------------------------------------+----+---------------------+
      var str = "";
      var word = 0;
      var longest = 0;
      var lastzero = 0;
      var zstart = 0;
      var len = 0;
      var i = 0;
      var parts = [
        ints[0] & 0xffff,
        (ints[0] >> 16),
        ints[1] & 0xffff,
        (ints[1] >> 16),
        ints[2] & 0xffff,
        (ints[2] >> 16),
        ints[3] & 0xffff,
        (ints[3] >> 16)
      ];
  
      // Handle IPv4-compatible, IPv4-mapped, loopback and any/unspecified addresses
  
      var hasipv4 = true;
      var v4part = "";
      // check if the 10 high-order bytes are all zeros (first 5 words)
      for (i = 0; i < 5; i++) {
        if (parts[i] !== 0) { hasipv4 = false; break; }
      }
  
      if (hasipv4) {
        // low-order 32-bits store an IPv4 address (bytes 13 to 16) (last 2 words)
        v4part = inetNtop4(parts[6] | (parts[7] << 16));
        // IPv4-mapped IPv6 address if 16-bit value (bytes 11 and 12) == 0xFFFF (6th word)
        if (parts[5] === -1) {
          str = "::ffff:";
          str += v4part;
          return str;
        }
        // IPv4-compatible IPv6 address if 16-bit value (bytes 11 and 12) == 0x0000 (6th word)
        if (parts[5] === 0) {
          str = "::";
          //special case IPv6 addresses
          if (v4part === "0.0.0.0") v4part = ""; // any/unspecified address
          if (v4part === "0.0.0.1") v4part = "1";// loopback address
          str += v4part;
          return str;
        }
      }
  
      // Handle all other IPv6 addresses
  
      // first run to find the longest contiguous zero words
      for (word = 0; word < 8; word++) {
        if (parts[word] === 0) {
          if (word - lastzero > 1) {
            len = 0;
          }
          lastzero = word;
          len++;
        }
        if (len > longest) {
          longest = len;
          zstart = word - longest + 1;
        }
      }
  
      for (word = 0; word < 8; word++) {
        if (longest > 1) {
          // compress contiguous zeros - to produce "::"
          if (parts[word] === 0 && word >= zstart && word < (zstart + longest) ) {
            if (word === zstart) {
              str += ":";
              if (zstart === 0) str += ":"; //leading zeros case
            }
            continue;
          }
        }
        // converts 16-bit words from big-endian to little-endian before converting to hex string
        str += Number(_ntohs(parts[word] & 0xffff)).toString(16);
        str += word < 7 ? ":" : "";
      }
      return str;
    };
  
  
  
  
  var zeroMemory = (address, size) => {
      HEAPU8.fill(0, address, address + size);
    };
  
  /** @param {number=} addrlen */
  var writeSockaddr = (sa, family, addr, port, addrlen) => {
      switch (family) {
        case 2:
          addr = inetPton4(addr);
          zeroMemory(sa, 16);
          if (addrlen) {
            HEAP32[((addrlen)>>2)] = 16;
          }
          HEAP16[((sa)>>1)] = family;
          HEAP32[(((sa)+(4))>>2)] = addr;
          HEAP16[(((sa)+(2))>>1)] = _htons(port);
          break;
        case 10:
          addr = inetPton6(addr);
          zeroMemory(sa, 28);
          if (addrlen) {
            HEAP32[((addrlen)>>2)] = 28;
          }
          HEAP32[((sa)>>2)] = family;
          HEAP32[(((sa)+(8))>>2)] = addr[0];
          HEAP32[(((sa)+(12))>>2)] = addr[1];
          HEAP32[(((sa)+(16))>>2)] = addr[2];
          HEAP32[(((sa)+(20))>>2)] = addr[3];
          HEAP16[(((sa)+(2))>>1)] = _htons(port);
          break;
        default:
          return 5;
      }
      return 0;
    };
  
  
  
  
  
  function _getaddrinfo(node, service, hint, out) {
  if (ENVIRONMENT_IS_PTHREAD)
    return proxyToMainThread(6, 0, 1, node, service, hint, out);
  
      // Note getaddrinfo currently only returns a single addrinfo with ai_next defaulting to NULL. When NULL
      // hints are specified or ai_family set to AF_UNSPEC or ai_socktype or ai_protocol set to 0 then we
      // really should provide a linked list of suitable addrinfo values.
      var addrs = [];
      var canon = null;
      var addr = 0;
      var port = 0;
      var flags = 0;
      var family = 0;
      var type = 0;
      var proto = 0;
      var ai, last;
  
      function allocaddrinfo(family, type, proto, canon, addr, port) {
        var sa, salen, ai;
        var errno;
  
        salen = family === 10 ?
          28 :
          16;
        addr = family === 10 ?
          inetNtop6(addr) :
          inetNtop4(addr);
        sa = _malloc(salen);
        errno = writeSockaddr(sa, family, addr, port);
        assert(!errno);
  
        ai = _malloc(32);
        HEAP32[(((ai)+(4))>>2)] = family;
        HEAP32[(((ai)+(8))>>2)] = type;
        HEAP32[(((ai)+(12))>>2)] = proto;
        HEAPU32[(((ai)+(24))>>2)] = canon;
        HEAPU32[(((ai)+(20))>>2)] = sa;
        if (family === 10) {
          HEAP32[(((ai)+(16))>>2)] = 28;
        } else {
          HEAP32[(((ai)+(16))>>2)] = 16;
        }
        HEAP32[(((ai)+(28))>>2)] = 0;
  
        return ai;
      }
  
      if (hint) {
        flags = HEAP32[((hint)>>2)];
        family = HEAP32[(((hint)+(4))>>2)];
        type = HEAP32[(((hint)+(8))>>2)];
        proto = HEAP32[(((hint)+(12))>>2)];
      }
      if (type && !proto) {
        proto = type === 2 ? 17 : 6;
      }
      if (!type && proto) {
        type = proto === 17 ? 2 : 1;
      }
  
      // If type or proto are set to zero in hints we should really be returning multiple addrinfo values, but for
      // now default to a TCP STREAM socket so we can at least return a sensible addrinfo given NULL hints.
      if (proto === 0) {
        proto = 6;
      }
      if (type === 0) {
        type = 1;
      }
  
      if (!node && !service) {
        return -2;
      }
      if (flags & ~(1|2|4|
          1024|8|16|32)) {
        return -1;
      }
      if (hint !== 0 && (HEAP32[((hint)>>2)] & 2) && !node) {
        return -1;
      }
      if (flags & 32) {
        // TODO
        return -2;
      }
      if (type !== 0 && type !== 1 && type !== 2) {
        return -7;
      }
      if (family !== 0 && family !== 2 && family !== 10) {
        return -6;
      }
  
      if (service) {
        service = UTF8ToString(service);
        port = parseInt(service, 10);
  
        if (isNaN(port)) {
          if (flags & 1024) {
            return -2;
          }
          // TODO support resolving well-known service names from:
          // http://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.txt
          return -8;
        }
      }
  
      if (!node) {
        if (family === 0) {
          family = 2;
        }
        if ((flags & 1) === 0) {
          if (family === 2) {
            addr = _htonl(2130706433);
          } else {
            addr = [0, 0, 0, _htonl(1)];
          }
        }
        ai = allocaddrinfo(family, type, proto, null, addr, port);
        HEAPU32[((out)>>2)] = ai;
        return 0;
      }
  
      //
      // try as a numeric address
      //
      node = UTF8ToString(node);
      addr = inetPton4(node);
      if (addr !== null) {
        // incoming node is a valid ipv4 address
        if (family === 0 || family === 2) {
          family = 2;
        }
        else if (family === 10 && (flags & 8)) {
          addr = [0, 0, _htonl(0xffff), addr];
          family = 10;
        } else {
          return -2;
        }
      } else {
        addr = inetPton6(node);
        if (addr !== null) {
          // incoming node is a valid ipv6 address
          if (family === 0 || family === 10) {
            family = 10;
          } else {
            return -2;
          }
        }
      }
      if (addr != null) {
        ai = allocaddrinfo(family, type, proto, node, addr, port);
        HEAPU32[((out)>>2)] = ai;
        return 0;
      }
      if (flags & 4) {
        return -2;
      }
  
      //
      // try as a hostname
      //
      // resolve the hostname to a temporary fake address
      node = DNS.lookup_name(node);
      addr = inetPton4(node);
      if (family === 0) {
        family = 2;
      } else if (family === 10) {
        addr = [0, 0, _htonl(0xffff), addr];
      }
      ai = allocaddrinfo(family, type, proto, null, addr, port);
      HEAPU32[((out)>>2)] = ai;
      return 0;
    
  }
  


  var initRandomFill = () => {
      // This block is not needed on v19+ since crypto.getRandomValues is builtin
      if (ENVIRONMENT_IS_NODE) {
        var nodeCrypto = require('crypto');
        return (view) => nodeCrypto.randomFillSync(view);
      }
  
      // like with most Web APIs, we can't use Web Crypto API directly on shared memory,
      // so we need to create an intermediate buffer and copy it to the destination
      return (view) => view.set(crypto.getRandomValues(new Uint8Array(view.byteLength)));
    };
  var randomFill = (view) => {
      // Lazily init on the first invocation.
      (randomFill = initRandomFill())(view);
    };
  var _random_get = (buffer, size) => {
      randomFill(HEAPU8.subarray(buffer, buffer + size));
      return 0;
    };




  var MEMFS = {
  createBackend(opts) {
        return _wasmfs_create_memory_backend();
      },
  };
  
  
  
  
  var PATH = {
  isAbs:(path) => path.charAt(0) === '/',
  splitPath:(filename) => {
        var splitPathRe = /^(\/?|)([\s\S]*?)((?:\.{1,2}|[^\/]+?|)(\.[^.\/]*|))(?:[\/]*)$/;
        return splitPathRe.exec(filename).slice(1);
      },
  normalizeArray:(parts, allowAboveRoot) => {
        // if the path tries to go above the root, `up` ends up > 0
        var up = 0;
        for (var i = parts.length - 1; i >= 0; i--) {
          var last = parts[i];
          if (last === '.') {
            parts.splice(i, 1);
          } else if (last === '..') {
            parts.splice(i, 1);
            up++;
          } else if (up) {
            parts.splice(i, 1);
            up--;
          }
        }
        // if the path is allowed to go above the root, restore leading ..s
        if (allowAboveRoot) {
          for (; up; up--) {
            parts.unshift('..');
          }
        }
        return parts;
      },
  normalize:(path) => {
        var isAbsolute = PATH.isAbs(path),
            trailingSlash = path.slice(-1) === '/';
        // Normalize the path
        path = PATH.normalizeArray(path.split('/').filter((p) => !!p), !isAbsolute).join('/');
        if (!path && !isAbsolute) {
          path = '.';
        }
        if (path && trailingSlash) {
          path += '/';
        }
        return (isAbsolute ? '/' : '') + path;
      },
  dirname:(path) => {
        var result = PATH.splitPath(path),
            root = result[0],
            dir = result[1];
        if (!root && !dir) {
          // No dirname whatsoever
          return '.';
        }
        if (dir) {
          // It has a dirname, strip trailing slash
          dir = dir.slice(0, -1);
        }
        return root + dir;
      },
  basename:(path) => path && path.match(/([^\/]+|\/)\/*$/)[1],
  join:(...paths) => PATH.normalize(paths.join('/')),
  join2:(l, r) => PATH.normalize(l + '/' + r),
  };
  
  
  
  var stringToUTF8OnStack = (str) => {
      var size = lengthBytesUTF8(str) + 1;
      var ret = stackAlloc(size);
      stringToUTF8(str, ret, size);
      return ret;
    };
  
  
  var withStackSave = (f) => {
      var stack = stackSave();
      var ret = f();
      stackRestore(stack);
      return ret;
    };
  
  var readI53FromI64 = (ptr) => {
      return HEAPU32[((ptr)>>2)] + HEAP32[(((ptr)+(4))>>2)] * 4294967296;
    };
  
  var readI53FromU64 = (ptr) => {
      return HEAPU32[((ptr)>>2)] + HEAPU32[(((ptr)+(4))>>2)] * 4294967296;
    };
  
  
  
  var FS_mknod = (path, mode, dev) => FS.handleError(withStackSave(() => {
      var pathBuffer = stringToUTF8OnStack(path);
      return __wasmfs_mknod(pathBuffer, mode, dev);
    }));
  var FS_create = (path, mode = 0o666) => {
      mode &= 4095;
      mode |= 32768;
      return FS_mknod(path, mode, 0);
    };
  
  
  
  var FS_writeFile = (path, data) => {
      var sp = stackSave();
      var pathBuffer = stringToUTF8OnStack(path);
      if (typeof data == 'string') {
        var buf = new Uint8Array(lengthBytesUTF8(data) + 1);
        var actualNumBytes = stringToUTF8Array(data, buf, 0, buf.length);
        data = buf.slice(0, actualNumBytes);
      }
      var dataBuffer = _malloc(data.length);
      assert(dataBuffer);
      for (var i = 0; i < data.length; i++) {
        HEAP8[(dataBuffer)+(i)] = data[i];
      }
      var ret = __wasmfs_write_file(pathBuffer, dataBuffer, data.length);
      _free(dataBuffer);
      stackRestore(sp);
      return ret;
    };
  var FS_createDataFile = (parent, name, fileData, canRead, canWrite, canOwn) => {
      var pathName = name ? parent + '/' + name : parent;
      var mode = FS_getMode(canRead, canWrite);
  
      if (!wasmFSPreloadingFlushed) {
        // WasmFS code in the wasm is not ready to be called yet. Cache the
        // files we want to create here in JS, and WasmFS will read them
        // later.
        wasmFSPreloadedFiles.push({pathName, fileData, mode});
      } else {
        // WasmFS is already running, so create the file normally.
        FS_create(pathName, mode);
        FS_writeFile(pathName, fileData);
      }
    };
  
  var asyncLoad = async (url) => {
      var arrayBuffer = await readAsync(url);
      assert(arrayBuffer, `Loading data file "${url}" failed (no arrayBuffer).`);
      return new Uint8Array(arrayBuffer);
    };
  
  
  
  var PATH_FS = {
  resolve:(...args) => {
        var resolvedPath = '',
          resolvedAbsolute = false;
        for (var i = args.length - 1; i >= -1 && !resolvedAbsolute; i--) {
          var path = (i >= 0) ? args[i] : FS.cwd();
          // Skip empty and invalid entries
          if (typeof path != 'string') {
            throw new TypeError('Arguments to path.resolve must be strings');
          } else if (!path) {
            return ''; // an invalid portion invalidates the whole thing
          }
          resolvedPath = path + '/' + resolvedPath;
          resolvedAbsolute = PATH.isAbs(path);
        }
        // At this point the path should be resolved to a full absolute path, but
        // handle relative paths to be safe (might happen when process.cwd() fails)
        resolvedPath = PATH.normalizeArray(resolvedPath.split('/').filter((p) => !!p), !resolvedAbsolute).join('/');
        return ((resolvedAbsolute ? '/' : '') + resolvedPath) || '.';
      },
  relative:(from, to) => {
        from = PATH_FS.resolve(from).slice(1);
        to = PATH_FS.resolve(to).slice(1);
        function trim(arr) {
          var start = 0;
          for (; start < arr.length; start++) {
            if (arr[start] !== '') break;
          }
          var end = arr.length - 1;
          for (; end >= 0; end--) {
            if (arr[end] !== '') break;
          }
          if (start > end) return [];
          return arr.slice(start, end - start + 1);
        }
        var fromParts = trim(from.split('/'));
        var toParts = trim(to.split('/'));
        var length = Math.min(fromParts.length, toParts.length);
        var samePartsLength = length;
        for (var i = 0; i < length; i++) {
          if (fromParts[i] !== toParts[i]) {
            samePartsLength = i;
            break;
          }
        }
        var outputParts = [];
        for (var i = samePartsLength; i < fromParts.length; i++) {
          outputParts.push('..');
        }
        outputParts = outputParts.concat(toParts.slice(samePartsLength));
        return outputParts.join('/');
      },
  };
  
  
  var preloadPlugins = Module['preloadPlugins'] || [];
  var FS_handledByPreloadPlugin = (byteArray, fullname, finish, onerror) => {
      // Ensure plugins are ready.
      if (typeof Browser != 'undefined') Browser.init();
  
      var handled = false;
      preloadPlugins.forEach((plugin) => {
        if (handled) return;
        if (plugin['canHandle'](fullname)) {
          plugin['handle'](byteArray, fullname, finish, onerror);
          handled = true;
        }
      });
      return handled;
    };
  var FS_createPreloadedFile = (parent, name, url, canRead, canWrite, onload, onerror, dontCreateFile, canOwn, preFinish) => {
      // TODO we should allow people to just pass in a complete filename instead
      // of parent and name being that we just join them anyways
      var fullname = name ? PATH_FS.resolve(PATH.join2(parent, name)) : parent;
      var dep = getUniqueRunDependency(`cp ${fullname}`); // might have several active requests for the same fullname
      function processData(byteArray) {
        function finish(byteArray) {
          preFinish?.();
          if (!dontCreateFile) {
            FS_createDataFile(parent, name, byteArray, canRead, canWrite, canOwn);
          }
          onload?.();
          removeRunDependency(dep);
        }
        if (FS_handledByPreloadPlugin(byteArray, fullname, finish, () => {
          onerror?.();
          removeRunDependency(dep);
        })) {
          return;
        }
        finish(byteArray);
      }
      addRunDependency(dep);
      if (typeof url == 'string') {
        asyncLoad(url).then(processData, onerror);
      } else {
        processData(url);
      }
    };
  
  var FS_getMode = (canRead, canWrite) => {
      var mode = 0;
      if (canRead) mode |= 292 | 73;
      if (canWrite) mode |= 146;
      return mode;
    };
  
  
  var FS_modeStringToFlags = (str) => {
      var flagModes = {
        'r': 0,
        'r+': 2,
        'w': 512 | 64 | 1,
        'w+': 512 | 64 | 2,
        'a': 1024 | 64 | 1,
        'a+': 1024 | 64 | 2,
      };
      var flags = flagModes[str];
      if (typeof flags == 'undefined') {
        throw new Error(`Unknown file open mode: ${str}`);
      }
      return flags;
    };
  
  
  
  var FS_mkdir = (path, mode = 0o777) => FS.handleError(withStackSave(() => {
      var buffer = stringToUTF8OnStack(path);
      return __wasmfs_mkdir(buffer, mode);
    }));
  
  
    /**
     * @param {number=} mode Optionally, the mode to create in. Uses mkdir's
     *                       default if not set.
     */
  var FS_mkdirTree = (path, mode) => {
      var dirs = path.split('/');
      var d = '';
      for (var i = 0; i < dirs.length; ++i) {
        if (!dirs[i]) continue;
        d += '/' + dirs[i];
        try {
          FS_mkdir(d, mode);
        } catch(e) {
          if (e.errno != 20) throw e;
        }
      }
    };
  
  
  var FS_unlink = (path) => withStackSave(() => {
      var buffer = stringToUTF8OnStack(path);
      return __wasmfs_unlink(buffer);
    });
  
  
  
  
  var wasmFS$backends = {
  };
  
  var wasmFSDevices = {
  };
  
  var wasmFSDeviceStreams = {
  };
  
  var FS = {
  init() {
        FS.ensureErrnoError();
      },
  ErrnoError:null,
  handleError(returnValue) {
        // Assume errors correspond to negative returnValues
        // since some functions like _wasmfs_open() return positive
        // numbers on success (some callers of this function may need to negate the parameter).
        if (returnValue < 0) {
          throw new FS.ErrnoError(-returnValue);
        }
  
        return returnValue;
      },
  ensureErrnoError() {
        if (FS.ErrnoError) return;
        FS.ErrnoError = /** @this{Object} */ function ErrnoError(code) {
          this.errno = code;
          this.message = 'FS error';
          this.name = "ErrnoError";
        }
        FS.ErrnoError.prototype = new Error();
        FS.ErrnoError.prototype.constructor = FS.ErrnoError;
      },
  createDataFile(parent, name, fileData, canRead, canWrite, canOwn) {
        FS_createDataFile(parent, name, fileData, canRead, canWrite, canOwn);
      },
  createPath(parent, path, canRead, canWrite) {
        // Cache file path directory names.
        var parts = path.split('/').reverse();
        while (parts.length) {
          var part = parts.pop();
          if (!part) continue;
          var current = PATH.join2(parent, part);
          if (!wasmFSPreloadingFlushed) {
            wasmFSPreloadedDirs.push({parentPath: parent, childName: part});
          } else {
            FS.mkdir(current);
          }
          parent = current;
        }
        return current;
      },
  createPreloadedFile(parent, name, url, canRead, canWrite, onload, onerror, dontCreateFile, canOwn, preFinish) {
        return FS_createPreloadedFile(parent, name, url, canRead, canWrite, onload, onerror, dontCreateFile, canOwn, preFinish);
      },
  readFile(path, opts = {}) {
        opts.encoding = opts.encoding || 'binary';
        if (opts.encoding !== 'utf8' && opts.encoding !== 'binary') {
          throw new Error('Invalid encoding type "' + opts.encoding + '"');
        }
  
        // Copy the file into a JS buffer on the heap.
        var sp = stackSave();
        var buf = __wasmfs_read_file(stringToUTF8OnStack(path));
        stackRestore(sp);
  
        // The signed integer length resides in the first 8 bytes of the buffer.
        var length = readI53FromI64(buf);
  
        // Default return type is binary.
        // The buffer contents exist 8 bytes after the returned pointer.
        var ret = new Uint8Array(HEAPU8.subarray(buf + 8, buf + 8 + length));
        if (opts.encoding === 'utf8') {
          ret = UTF8ArrayToString(ret);
        }
  
        return ret;
      },
  cwd:() => UTF8ToString(__wasmfs_get_cwd()),
  analyzePath(path) {
        // TODO: Consider simplifying this API, which for now matches the JS FS.
        var exists = !!FS.findObject(path);
        return {
          exists,
          object: {
            contents: exists ? FS.readFile(path) : null
          }
        };
      },
  mkdir:(path, mode) => FS_mkdir(path, mode),
  mkdirTree:(path, mode) => FS_mkdirTree(path, mode),
  rmdir:(path) => FS.handleError(
        withStackSave(() => __wasmfs_rmdir(stringToUTF8OnStack(path)))
      ),
  open:(path, flags, mode = 0o666) => withStackSave(() => {
        flags = typeof flags == 'string' ? FS_modeStringToFlags(flags) : flags;
        var buffer = stringToUTF8OnStack(path);
        var fd = FS.handleError(__wasmfs_open(buffer, flags, mode));
        return { fd : fd };
      }),
  create:(path, mode) => FS_create(path, mode),
  close:(stream) => FS.handleError(-__wasmfs_close(stream.fd)),
  unlink:(path) => FS_unlink(path),
  chdir:(path) => withStackSave(() => {
        var buffer = stringToUTF8OnStack(path);
        return __wasmfs_chdir(buffer);
      }),
  read(stream, buffer, offset, length, position) {
        var seeking = typeof position != 'undefined';
  
        var dataBuffer = _malloc(length);
  
        var bytesRead;
        if (seeking) {
          bytesRead = __wasmfs_pread(stream.fd, dataBuffer, length, BigInt(position));
        } else {
          bytesRead = __wasmfs_read(stream.fd, dataBuffer, length);
        }
        bytesRead = FS.handleError(bytesRead);
  
        for (var i = 0; i < length; i++) {
          buffer[offset + i] = HEAP8[(dataBuffer)+(i)]
        }
  
        _free(dataBuffer);
        return bytesRead;
      },
  write(stream, buffer, offset, length, position, canOwn) {
        var seeking = typeof position != 'undefined';
  
        var dataBuffer = _malloc(length);
        for (var i = 0; i < length; i++) {
          HEAP8[(dataBuffer)+(i)] = buffer[offset + i];
        }
  
        var bytesRead;
        if (seeking) {
          bytesRead = __wasmfs_pwrite(stream.fd, dataBuffer, length, BigInt(position));
        } else {
          bytesRead = __wasmfs_write(stream.fd, dataBuffer, length);
        }
        bytesRead = FS.handleError(bytesRead);
        _free(dataBuffer);
  
        return bytesRead;
      },
  allocate(stream, offset, length) {
        return FS.handleError(__wasmfs_allocate(stream.fd, BigInt(offset), BigInt(length)));
      },
  writeFile:(path, data) => FS_writeFile(path, data),
  mmap:(stream, length, offset, prot, flags) => {
        var buf = FS.handleError(__wasmfs_mmap(length, prot, flags, stream.fd, BigInt(offset)));
        return { ptr: buf, allocated: true };
      },
  msync:(stream, bufferPtr, offset, length, mmapFlags) => {
        assert(offset === 0);
        // TODO: assert that stream has the fd corresponding to the mapped buffer (bufferPtr).
        return FS.handleError(__wasmfs_msync(bufferPtr, length, mmapFlags));
      },
  munmap:(addr, length) => (
        FS.handleError(__wasmfs_munmap(addr, length))
      ),
  symlink:(target, linkpath) => withStackSave(() => (
        __wasmfs_symlink(stringToUTF8OnStack(target), stringToUTF8OnStack(linkpath))
      )),
  readlink(path) {
        var readBuffer = FS.handleError(withStackSave(() => __wasmfs_readlink(stringToUTF8OnStack(path))));
        return UTF8ToString(readBuffer);
      },
  statBufToObject(statBuf) {
        // i53/u53 are enough for times and ino in practice.
        return {
            dev: HEAPU32[((statBuf)>>2)],
            mode: HEAPU32[(((statBuf)+(4))>>2)],
            nlink: HEAPU32[(((statBuf)+(8))>>2)],
            uid: HEAPU32[(((statBuf)+(12))>>2)],
            gid: HEAPU32[(((statBuf)+(16))>>2)],
            rdev: HEAPU32[(((statBuf)+(20))>>2)],
            size: readI53FromI64((statBuf)+(24)),
            blksize: HEAPU32[(((statBuf)+(32))>>2)],
            blocks: HEAPU32[(((statBuf)+(36))>>2)],
            atime: readI53FromI64((statBuf)+(40)),
            mtime: readI53FromI64((statBuf)+(56)),
            ctime: readI53FromI64((statBuf)+(72)),
            ino: readI53FromU64((statBuf)+(88))
        }
      },
  stat(path) {
        var statBuf = _malloc(96);
        FS.handleError(withStackSave(() =>
          __wasmfs_stat(stringToUTF8OnStack(path), statBuf)
        ));
        var stats = FS.statBufToObject(statBuf);
        _free(statBuf);
  
        return stats;
      },
  lstat(path) {
        var statBuf = _malloc(96);
        FS.handleError(withStackSave(() =>
          __wasmfs_lstat(stringToUTF8OnStack(path), statBuf)
        ));
        var stats = FS.statBufToObject(statBuf);
        _free(statBuf);
  
        return stats;
      },
  chmod(path, mode) {
        return FS.handleError(withStackSave(() => {
          var buffer = stringToUTF8OnStack(path);
          return __wasmfs_chmod(buffer, mode);
        }));
      },
  lchmod(path, mode) {
        return FS.handleError(withStackSave(() => {
          var buffer = stringToUTF8OnStack(path);
          return __wasmfs_lchmod(buffer, mode);
        }));
      },
  fchmod(fd, mode) {
        return FS.handleError(__wasmfs_fchmod(fd, mode));
      },
  utime:(path, atime, mtime) => (
        FS.handleError(withStackSave(() => (
          __wasmfs_utime(stringToUTF8OnStack(path), atime, mtime)
        )))
      ),
  truncate(path, len) {
        return FS.handleError(withStackSave(() => (__wasmfs_truncate(stringToUTF8OnStack(path), BigInt(len)))));
      },
  ftruncate(fd, len) {
        return FS.handleError(__wasmfs_ftruncate(fd, BigInt(len)));
      },
  findObject(path) {
        var result = withStackSave(() => __wasmfs_identify(stringToUTF8OnStack(path)));
        if (result == 44) {
          return null;
        }
        return {
          isFolder: result == 31,
          isDevice: false, // TODO: wasmfs support for devices
        };
      },
  readdir:(path) => withStackSave(() => {
        var pathBuffer = stringToUTF8OnStack(path);
        var entries = [];
        var state = __wasmfs_readdir_start(pathBuffer);
        if (!state) {
          // TODO: The old FS threw an ErrnoError here.
          throw new Error("No such directory");
        }
        var entry;
        while (entry = __wasmfs_readdir_get(state)) {
          entries.push(UTF8ToString(entry));
        }
        __wasmfs_readdir_finish(state);
        return entries;
      }),
  mount:(type, opts, mountpoint) => {
        if (typeof type == 'string') {
          // The filesystem was not included, and instead we have an error
          // message stored in the variable.
          throw type;
        }
        var backendPointer = type.createBackend(opts);
        return FS.handleError(withStackSave(() => __wasmfs_mount(stringToUTF8OnStack(mountpoint), backendPointer)));
      },
  unmount:(mountpoint) => (
        FS.handleError(withStackSave(() => __wasmfs_unmount(stringToUTF8OnStack(mountpoint))))
      ),
  mknod:(path, mode, dev) => FS_mknod(path, mode, dev),
  makedev:(ma, mi) => ((ma) << 8 | (mi)),
  registerDevice(dev, ops) {
        var backendPointer = _wasmfs_create_jsimpl_backend();
        var definedOps = {
          userRead: ops.read,
          userWrite: ops.write,
  
          allocFile: (file) => {
            wasmFSDeviceStreams[file] = {}
          },
          freeFile: (file) => {
            wasmFSDeviceStreams[file] = undefined;
          },
          getSize: (file) => {},
          // Devices cannot be resized.
          setSize: (file, size) => 0,
          read: (file, buffer, length, offset) => {
            var bufferArray = Module.HEAP8.subarray(buffer, buffer + length);
            try {
              var bytesRead = definedOps.userRead(wasmFSDeviceStreams[file], bufferArray, 0, length, offset);
            } catch (e) {
              return -e.errno;
            }
            Module.HEAP8.set(bufferArray, buffer);
            return bytesRead;
          },
          write: (file, buffer, length, offset) => {
            var bufferArray = Module.HEAP8.subarray(buffer, buffer + length);
            try {
              var bytesWritten = definedOps.userWrite(wasmFSDeviceStreams[file], bufferArray, 0, length, offset);
            } catch (e) {
              return -e.errno;
            }
            Module.HEAP8.set(bufferArray, buffer);
            return bytesWritten;
          },
        };
  
        wasmFS$backends[backendPointer] = definedOps;
        wasmFSDevices[dev] = backendPointer;
      },
  createDevice(parent, name, input, output) {
        if (typeof parent != 'string') {
          // The old API allowed parents to be objects, which do not exist in WasmFS.
          throw new Error("Only string paths are accepted");
        }
        var path = PATH.join2(parent, name);
        var mode = FS_getMode(!!input, !!output);
        FS.createDevice.major ??= 64;
        var dev = FS.makedev(FS.createDevice.major++, 0);
        // Create a fake device with a set of stream ops to emulate
        // the old API's createDevice().
        FS.registerDevice(dev, {
          read(stream, buffer, offset, length, pos /* ignored */) {
            var bytesRead = 0;
            for (var i = 0; i < length; i++) {
              var result;
              try {
                result = input();
              } catch (e) {
                throw new FS.ErrnoError(29);
              }
              if (result === undefined && bytesRead === 0) {
                throw new FS.ErrnoError(6);
              }
              if (result === null || result === undefined) break;
              bytesRead++;
              buffer[offset+i] = result;
            }
            return bytesRead;
          },
          write(stream, buffer, offset, length, pos) {
            for (var i = 0; i < length; i++) {
              try {
                output(buffer[offset+i]);
              } catch (e) {
                throw new FS.ErrnoError(29);
              }
            }
            return i;
          }
        });
        return FS.mkdev(path, mode, dev);
      },
  mkdev(path, mode, dev) {
        if (typeof dev === 'undefined') {
          dev = mode;
          mode = 0o666;
        }
  
        var deviceBackend = wasmFSDevices[dev];
        if (!deviceBackend) {
          throw new Error("Invalid device ID.");
        }
  
        return FS.handleError(withStackSave(() => (
          _wasmfs_create_file(stringToUTF8OnStack(path), mode, deviceBackend)
        )));
      },
  rename(oldPath, newPath) {
        return FS.handleError(withStackSave(() => {
          var oldPathBuffer = stringToUTF8OnStack(oldPath);
          var newPathBuffer = stringToUTF8OnStack(newPath);
          return __wasmfs_rename(oldPathBuffer, newPathBuffer);
        }));
      },
  llseek(stream, offset, whence) {
        return FS.handleError(__wasmfs_llseek(stream.fd, BigInt(offset), whence));
      },
  };





  var FS_createPath = FS.createPath;



PThread.init();;
embind_init_charCodes();
BindingError = Module['BindingError'] = class BindingError extends Error { constructor(message) { super(message); this.name = 'BindingError'; }};
InternalError = Module['InternalError'] = class InternalError extends Error { constructor(message) { super(message); this.name = 'InternalError'; }};
init_ClassHandle();
init_RegisteredPointer();
UnboundTypeError = Module['UnboundTypeError'] = extendError(Error, 'UnboundTypeError');;
init_emval();;

      Module['requestAnimationFrame'] = MainLoop.requestAnimationFrame;
      Module['pauseMainLoop'] = MainLoop.pause;
      Module['resumeMainLoop'] = MainLoop.resume;
      MainLoop.init();;

  FS.init();
  ;

// proxiedFunctionTable specifies the list of functions that can be called
// either synchronously or asynchronously from other threads in postMessage()d
// or internally queued events. This way a pthread in a Worker can synchronously
// access e.g. the DOM on the main thread.
var proxiedFunctionTable = [
  _proc_exit,
  exitOnMainThread,
  pthreadCreateProxied,
  __setitimer_js,
  _environ_get,
  _environ_sizes_get,
  _getaddrinfo
];

function checkIncomingModuleAPI() {
  ignoredModuleProp('fetchSettings');
}
var wasmImports;
function assignWasmImports() {
  wasmImports = {
    /** @export */
    _JS_addAlertOnError,
    /** @export */
    _JS_addSoundToggle,
    /** @export */
    _JS_alert,
    /** @export */
    _JS_createBng,
    /** @export */
    _JS_createTgl,
    /** @export */
    _JS_getMicAccess,
    /** @export */
    _JS_loadStyle,
    /** @export */
    _JS_onReceived,
    /** @export */
    _JS_post,
    /** @export */
    _JS_receiveBang,
    /** @export */
    _JS_receiveFloat,
    /** @export */
    _JS_receiveList,
    /** @export */
    _JS_receiveMessage,
    /** @export */
    _JS_receiveSymbol,
    /** @export */
    _JS_sendList,
    /** @export */
    _JS_setIcon,
    /** @export */
    _JS_setTitle,
    /** @export */
    _JS_suspendAudioWorkLet,
    /** @export */
    __assert_fail: ___assert_fail,
    /** @export */
    __call_sighandler: ___call_sighandler,
    /** @export */
    __cxa_throw: ___cxa_throw,
    /** @export */
    __pthread_create_js: ___pthread_create_js,
    /** @export */
    _abort_js: __abort_js,
    /** @export */
    _embind_register_bigint: __embind_register_bigint,
    /** @export */
    _embind_register_bool: __embind_register_bool,
    /** @export */
    _embind_register_class: __embind_register_class,
    /** @export */
    _embind_register_class_constructor: __embind_register_class_constructor,
    /** @export */
    _embind_register_class_function: __embind_register_class_function,
    /** @export */
    _embind_register_emval: __embind_register_emval,
    /** @export */
    _embind_register_float: __embind_register_float,
    /** @export */
    _embind_register_integer: __embind_register_integer,
    /** @export */
    _embind_register_memory_view: __embind_register_memory_view,
    /** @export */
    _embind_register_std_string: __embind_register_std_string,
    /** @export */
    _embind_register_std_wstring: __embind_register_std_wstring,
    /** @export */
    _embind_register_void: __embind_register_void,
    /** @export */
    _emscripten_init_main_thread_js: __emscripten_init_main_thread_js,
    /** @export */
    _emscripten_notify_mailbox_postmessage: __emscripten_notify_mailbox_postmessage,
    /** @export */
    _emscripten_receive_on_main_thread_js: __emscripten_receive_on_main_thread_js,
    /** @export */
    _emscripten_runtime_keepalive_clear: __emscripten_runtime_keepalive_clear,
    /** @export */
    _emscripten_system: __emscripten_system,
    /** @export */
    _emscripten_thread_cleanup: __emscripten_thread_cleanup,
    /** @export */
    _emscripten_thread_mailbox_await: __emscripten_thread_mailbox_await,
    /** @export */
    _emscripten_thread_set_strongref: __emscripten_thread_set_strongref,
    /** @export */
    _emscripten_throw_longjmp: __emscripten_throw_longjmp,
    /** @export */
    _gmtime_js: __gmtime_js,
    /** @export */
    _localtime_js: __localtime_js,
    /** @export */
    _mktime_js: __mktime_js,
    /** @export */
    _setitimer_js: __setitimer_js,
    /** @export */
    _tzset_js: __tzset_js,
    /** @export */
    _wasmfs_copy_preloaded_file_data: __wasmfs_copy_preloaded_file_data,
    /** @export */
    _wasmfs_get_num_preloaded_dirs: __wasmfs_get_num_preloaded_dirs,
    /** @export */
    _wasmfs_get_num_preloaded_files: __wasmfs_get_num_preloaded_files,
    /** @export */
    _wasmfs_get_preloaded_child_path: __wasmfs_get_preloaded_child_path,
    /** @export */
    _wasmfs_get_preloaded_file_mode: __wasmfs_get_preloaded_file_mode,
    /** @export */
    _wasmfs_get_preloaded_file_size: __wasmfs_get_preloaded_file_size,
    /** @export */
    _wasmfs_get_preloaded_parent_path: __wasmfs_get_preloaded_parent_path,
    /** @export */
    _wasmfs_get_preloaded_path_name: __wasmfs_get_preloaded_path_name,
    /** @export */
    _wasmfs_jsimpl_alloc_file: __wasmfs_jsimpl_alloc_file,
    /** @export */
    _wasmfs_jsimpl_free_file: __wasmfs_jsimpl_free_file,
    /** @export */
    _wasmfs_jsimpl_get_size: __wasmfs_jsimpl_get_size,
    /** @export */
    _wasmfs_jsimpl_read: __wasmfs_jsimpl_read,
    /** @export */
    _wasmfs_jsimpl_set_size: __wasmfs_jsimpl_set_size,
    /** @export */
    _wasmfs_jsimpl_write: __wasmfs_jsimpl_write,
    /** @export */
    _wasmfs_stdin_get_char: __wasmfs_stdin_get_char,
    /** @export */
    clock_time_get: _clock_time_get,
    /** @export */
    emscripten_asm_const_async_on_main_thread: _emscripten_asm_const_async_on_main_thread,
    /** @export */
    emscripten_asm_const_int: _emscripten_asm_const_int,
    /** @export */
    emscripten_check_blocking_allowed: _emscripten_check_blocking_allowed,
    /** @export */
    emscripten_create_audio_context: _emscripten_create_audio_context,
    /** @export */
    emscripten_create_wasm_audio_worklet_node: _emscripten_create_wasm_audio_worklet_node,
    /** @export */
    emscripten_create_wasm_audio_worklet_processor_async: _emscripten_create_wasm_audio_worklet_processor_async,
    /** @export */
    emscripten_date_now: _emscripten_date_now,
    /** @export */
    emscripten_err: _emscripten_err,
    /** @export */
    emscripten_exit_with_live_runtime: _emscripten_exit_with_live_runtime,
    /** @export */
    emscripten_get_heap_max: _emscripten_get_heap_max,
    /** @export */
    emscripten_get_now: _emscripten_get_now,
    /** @export */
    emscripten_num_logical_cores: _emscripten_num_logical_cores,
    /** @export */
    emscripten_out: _emscripten_out,
    /** @export */
    emscripten_resize_heap: _emscripten_resize_heap,
    /** @export */
    emscripten_resume_audio_context_sync: _emscripten_resume_audio_context_sync,
    /** @export */
    emscripten_set_main_loop: _emscripten_set_main_loop,
    /** @export */
    emscripten_start_wasm_audio_worklet_thread_async: _emscripten_start_wasm_audio_worklet_thread_async,
    /** @export */
    environ_get: _environ_get,
    /** @export */
    environ_sizes_get: _environ_sizes_get,
    /** @export */
    exit: _exit,
    /** @export */
    getaddrinfo: _getaddrinfo,
    /** @export */
    invoke_vii,
    /** @export */
    memory: wasmMemory,
    /** @export */
    proc_exit: _proc_exit,
    /** @export */
    random_get: _random_get
  };
}
var wasmExports = await createWasm();
var ___wasm_call_ctors = createExportWrapper('__wasm_call_ctors', 0);
var _malloc = createExportWrapper('malloc', 1);
var _main = Module['_main'] = createExportWrapper('main', 2);
var ___getTypeName = createExportWrapper('__getTypeName', 1);
var __embind_initialize_bindings = createExportWrapper('_embind_initialize_bindings', 0);
var _pthread_self = () => (_pthread_self = wasmExports['pthread_self'])();
var _free = createExportWrapper('free', 1);
var _fflush = createExportWrapper('fflush', 1);
var _ntohs = createExportWrapper('ntohs', 1);
var _htons = createExportWrapper('htons', 1);
var __emscripten_tls_init = createExportWrapper('_emscripten_tls_init', 0);
var _emscripten_builtin_memalign = createExportWrapper('emscripten_builtin_memalign', 2);
var __emscripten_thread_init = createExportWrapper('_emscripten_thread_init', 6);
var ___set_thread_state = createExportWrapper('__set_thread_state', 4);
var __emscripten_thread_crashed = createExportWrapper('_emscripten_thread_crashed', 0);
var _htonl = createExportWrapper('htonl', 1);
var _emscripten_stack_get_base = () => (_emscripten_stack_get_base = wasmExports['emscripten_stack_get_base'])();
var _emscripten_stack_get_end = () => (_emscripten_stack_get_end = wasmExports['emscripten_stack_get_end'])();
var __emscripten_run_on_main_thread_js = createExportWrapper('_emscripten_run_on_main_thread_js', 5);
var __emscripten_thread_free_data = createExportWrapper('_emscripten_thread_free_data', 1);
var __emscripten_thread_exit = createExportWrapper('_emscripten_thread_exit', 1);
var __emscripten_timeout = createExportWrapper('_emscripten_timeout', 2);
var __emscripten_check_mailbox = createExportWrapper('_emscripten_check_mailbox', 0);
var _setThrew = createExportWrapper('setThrew', 2);
var _emscripten_stack_init = () => (_emscripten_stack_init = wasmExports['emscripten_stack_init'])();
var _emscripten_stack_set_limits = (a0, a1) => (_emscripten_stack_set_limits = wasmExports['emscripten_stack_set_limits'])(a0, a1);
var _emscripten_stack_get_free = () => (_emscripten_stack_get_free = wasmExports['emscripten_stack_get_free'])();
var __emscripten_stack_restore = (a0) => (__emscripten_stack_restore = wasmExports['_emscripten_stack_restore'])(a0);
var __emscripten_stack_alloc = (a0) => (__emscripten_stack_alloc = wasmExports['_emscripten_stack_alloc'])(a0);
var _emscripten_stack_get_current = () => (_emscripten_stack_get_current = wasmExports['emscripten_stack_get_current'])();
var __emscripten_wasm_worker_initialize = createExportWrapper('_emscripten_wasm_worker_initialize', 2);
var __wasmfs_read_file = createExportWrapper('_wasmfs_read_file', 1);
var __wasmfs_write_file = createExportWrapper('_wasmfs_write_file', 3);
var __wasmfs_mkdir = createExportWrapper('_wasmfs_mkdir', 2);
var __wasmfs_rmdir = createExportWrapper('_wasmfs_rmdir', 1);
var __wasmfs_open = createExportWrapper('_wasmfs_open', 3);
var __wasmfs_allocate = createExportWrapper('_wasmfs_allocate', 3);
var __wasmfs_mknod = createExportWrapper('_wasmfs_mknod', 3);
var __wasmfs_unlink = createExportWrapper('_wasmfs_unlink', 1);
var __wasmfs_chdir = createExportWrapper('_wasmfs_chdir', 1);
var __wasmfs_symlink = createExportWrapper('_wasmfs_symlink', 2);
var __wasmfs_readlink = createExportWrapper('_wasmfs_readlink', 1);
var __wasmfs_write = createExportWrapper('_wasmfs_write', 3);
var __wasmfs_pwrite = createExportWrapper('_wasmfs_pwrite', 4);
var __wasmfs_chmod = createExportWrapper('_wasmfs_chmod', 2);
var __wasmfs_fchmod = createExportWrapper('_wasmfs_fchmod', 2);
var __wasmfs_lchmod = createExportWrapper('_wasmfs_lchmod', 2);
var __wasmfs_llseek = createExportWrapper('_wasmfs_llseek', 3);
var __wasmfs_rename = createExportWrapper('_wasmfs_rename', 2);
var __wasmfs_read = createExportWrapper('_wasmfs_read', 3);
var __wasmfs_pread = createExportWrapper('_wasmfs_pread', 4);
var __wasmfs_truncate = createExportWrapper('_wasmfs_truncate', 2);
var __wasmfs_ftruncate = createExportWrapper('_wasmfs_ftruncate', 2);
var __wasmfs_close = createExportWrapper('_wasmfs_close', 1);
var __wasmfs_mmap = createExportWrapper('_wasmfs_mmap', 5);
var __wasmfs_msync = createExportWrapper('_wasmfs_msync', 3);
var __wasmfs_munmap = createExportWrapper('_wasmfs_munmap', 2);
var __wasmfs_utime = createExportWrapper('_wasmfs_utime', 3);
var __wasmfs_stat = createExportWrapper('_wasmfs_stat', 2);
var __wasmfs_lstat = createExportWrapper('_wasmfs_lstat', 2);
var __wasmfs_mount = createExportWrapper('_wasmfs_mount', 2);
var __wasmfs_unmount = createExportWrapper('_wasmfs_unmount', 1);
var __wasmfs_identify = createExportWrapper('_wasmfs_identify', 1);
var __wasmfs_readdir_start = createExportWrapper('_wasmfs_readdir_start', 1);
var __wasmfs_readdir_get = createExportWrapper('_wasmfs_readdir_get', 1);
var __wasmfs_readdir_finish = createExportWrapper('_wasmfs_readdir_finish', 1);
var __wasmfs_get_cwd = createExportWrapper('_wasmfs_get_cwd', 0);
var _wasmfs_create_jsimpl_backend = createExportWrapper('wasmfs_create_jsimpl_backend', 0);
var _wasmfs_create_memory_backend = createExportWrapper('wasmfs_create_memory_backend', 0);
var _wasmfs_create_file = createExportWrapper('wasmfs_create_file', 3);
var _wasmfs_flush = createExportWrapper('wasmfs_flush', 0);

function invoke_vii(index,a1,a2) {
  var sp = stackSave();
  try {
    getWasmTableEntry(index)(a1,a2);
  } catch(e) {
    stackRestore(sp);
    if (e !== e+0) throw e;
    _setThrew(1, 0);
  }
}


// include: postamble.js
// === Auto-generated postamble setup entry stuff ===

Module['addRunDependency'] = addRunDependency;
Module['removeRunDependency'] = removeRunDependency;
Module['stackSave'] = stackSave;
Module['stackRestore'] = stackRestore;
Module['stackAlloc'] = stackAlloc;
Module['wasmTable'] = wasmTable;
Module['FS_createPreloadedFile'] = FS_createPreloadedFile;
Module['FS_unlink'] = FS_unlink;
Module['FS_createPath'] = FS_createPath;
Module['FS_createDataFile'] = FS_createDataFile;
var missingLibrarySymbols = [
  'writeI53ToI64',
  'writeI53ToI64Clamped',
  'writeI53ToI64Signaling',
  'writeI53ToU64Clamped',
  'writeI53ToU64Signaling',
  'convertI32PairToI53',
  'convertI32PairToI53Checked',
  'convertU32PairToI53',
  'getTempRet0',
  'setTempRet0',
  'growMemory',
  'strError',
  'readSockaddr',
  'emscriptenLog',
  'listenOnce',
  'autoResumeAudioContext',
  'getDynCaller',
  'dynCall',
  'asmjsMangle',
  'mmapAlloc',
  'HandleAllocator',
  'getNativeTypeSize',
  'addOnInit',
  'addOnPostCtor',
  'addOnPreMain',
  'addOnExit',
  'STACK_SIZE',
  'STACK_ALIGN',
  'POINTER_SIZE',
  'ASSERTIONS',
  'getCFunc',
  'ccall',
  'cwrap',
  'uleb128Encode',
  'sigToWasmTypes',
  'generateFuncType',
  'convertJsFunctionToWasm',
  'getEmptyTableSlot',
  'updateTableMap',
  'getFunctionAddress',
  'addFunction',
  'removeFunction',
  'reallyNegative',
  'unSign',
  'strLen',
  'reSign',
  'formatString',
  'intArrayToString',
  'AsciiToString',
  'stringToNewUTF8',
  'writeArrayToMemory',
  'registerKeyEventCallback',
  'maybeCStringToJsString',
  'findEventTarget',
  'getBoundingClientRect',
  'fillMouseEventData',
  'registerMouseEventCallback',
  'registerWheelEventCallback',
  'registerUiEventCallback',
  'registerFocusEventCallback',
  'fillDeviceOrientationEventData',
  'registerDeviceOrientationEventCallback',
  'fillDeviceMotionEventData',
  'registerDeviceMotionEventCallback',
  'screenOrientation',
  'fillOrientationChangeEventData',
  'registerOrientationChangeEventCallback',
  'fillFullscreenChangeEventData',
  'registerFullscreenChangeEventCallback',
  'JSEvents_requestFullscreen',
  'JSEvents_resizeCanvasForFullscreen',
  'registerRestoreOldStyle',
  'hideEverythingExceptGivenElement',
  'restoreHiddenElements',
  'setLetterbox',
  'softFullscreenResizeWebGLRenderTarget',
  'doRequestFullscreen',
  'fillPointerlockChangeEventData',
  'registerPointerlockChangeEventCallback',
  'registerPointerlockErrorEventCallback',
  'requestPointerLock',
  'fillVisibilityChangeEventData',
  'registerVisibilityChangeEventCallback',
  'registerTouchEventCallback',
  'fillGamepadEventData',
  'registerGamepadEventCallback',
  'registerBeforeUnloadEventCallback',
  'fillBatteryEventData',
  'battery',
  'registerBatteryEventCallback',
  'setCanvasElementSizeCallingThread',
  'setCanvasElementSizeMainThread',
  'setCanvasElementSize',
  'getCanvasSizeCallingThread',
  'getCanvasSizeMainThread',
  'getCanvasElementSize',
  'jsStackTrace',
  'getCallstack',
  'convertPCtoSourceLocation',
  'flush_NO_FILESYSTEM',
  'wasiRightsToMuslOFlags',
  'wasiOFlagsToMuslOFlags',
  'safeSetTimeout',
  'setImmediateWrapped',
  'safeRequestAnimationFrame',
  'clearImmediateWrapped',
  'registerPostMainLoop',
  'registerPreMainLoop',
  'getPromise',
  'makePromise',
  'idsToPromises',
  'makePromiseCallback',
  'findMatchingCatch',
  'Browser_asyncPrepareDataCounter',
  'arraySum',
  'addDays',
  'wasmfsNodeConvertNodeCode',
  'wasmfsTry',
  'wasmfsNodeFixStat',
  'wasmfsNodeLstat',
  'wasmfsNodeFstat',
  'wasmfsOPFSProxyFinish',
  'wasmfsOPFSGetOrCreateFile',
  'wasmfsOPFSGetOrCreateDir',
  'heapObjectForWebGLType',
  'toTypedArrayIndex',
  'webgl_enable_ANGLE_instanced_arrays',
  'webgl_enable_OES_vertex_array_object',
  'webgl_enable_WEBGL_draw_buffers',
  'webgl_enable_WEBGL_multi_draw',
  'webgl_enable_EXT_polygon_offset_clamp',
  'webgl_enable_EXT_clip_control',
  'webgl_enable_WEBGL_polygon_mode',
  'emscriptenWebGLGet',
  'computeUnpackAlignedImageSize',
  'colorChannelsInGlTextureFormat',
  'emscriptenWebGLGetTexPixelData',
  'emscriptenWebGLGetUniform',
  'webglGetUniformLocation',
  'webglPrepareUniformLocationsBeforeFirstUse',
  'webglGetLeftBracePos',
  'emscriptenWebGLGetVertexAttrib',
  '__glGetActiveAttribOrUniform',
  'writeGLArray',
  'emscripten_webgl_destroy_context_before_on_calling_thread',
  'registerWebGlEventCallback',
  'runAndAbortIfError',
  'ALLOC_NORMAL',
  'ALLOC_STACK',
  'allocate',
  'writeStringToMemory',
  'writeAsciiToMemory',
  'setErrNo',
  'demangle',
  'stackTrace',
  '_wasmWorkerPostFunction1',
  '_wasmWorkerPostFunction2',
  '_wasmWorkerPostFunction3',
  'emscripten_audio_worklet_post_function_1',
  'emscripten_audio_worklet_post_function_2',
  'emscripten_audio_worklet_post_function_3',
  'getFunctionArgsName',
  'requireRegisteredType',
  'createJsInvokerSignature',
  'registerInheritedInstance',
  'unregisterInheritedInstance',
  'getInheritedInstanceCount',
  'getLiveInheritedInstances',
  'enumReadValueFromPointer',
  'setDelayFunction',
  'validateThis',
  'getStringOrSymbol',
  'emval_get_global',
  'emval_returnValue',
  'emval_lookupTypes',
  'emval_addMethodCaller',
];
missingLibrarySymbols.forEach(missingLibrarySymbol)

var unexportedSymbols = [
  'run',
  'out',
  'err',
  'callMain',
  'abort',
  'wasmMemory',
  'wasmExports',
  'writeStackCookie',
  'checkStackCookie',
  'readI53FromI64',
  'readI53FromU64',
  'INT53_MAX',
  'INT53_MIN',
  'bigintToI53Checked',
  'ptrToString',
  'zeroMemory',
  'exitJS',
  'getHeapMax',
  'abortOnCannotGrowMemory',
  'ENV',
  'ERRNO_CODES',
  'inetPton4',
  'inetNtop4',
  'inetPton6',
  'inetNtop6',
  'writeSockaddr',
  'DNS',
  'Protocols',
  'Sockets',
  'timers',
  'warnOnce',
  'readEmAsmArgsArray',
  'readEmAsmArgs',
  'runEmAsmFunction',
  'runMainThreadEmAsm',
  'jstoi_q',
  'jstoi_s',
  'getExecutableName',
  'handleException',
  'keepRuntimeAlive',
  'runtimeKeepalivePush',
  'runtimeKeepalivePop',
  'callUserCallback',
  'maybeExit',
  'asyncLoad',
  'alignMemory',
  'noExitRuntime',
  'addOnPreRun',
  'addOnPostRun',
  'freeTableIndexes',
  'functionsInTableMap',
  'setValue',
  'getValue',
  'PATH',
  'PATH_FS',
  'UTF8Decoder',
  'UTF8ArrayToString',
  'UTF8ToString',
  'stringToUTF8Array',
  'stringToUTF8',
  'lengthBytesUTF8',
  'intArrayFromString',
  'stringToAscii',
  'UTF16Decoder',
  'UTF16ToString',
  'stringToUTF16',
  'lengthBytesUTF16',
  'UTF32ToString',
  'stringToUTF32',
  'lengthBytesUTF32',
  'stringToUTF8OnStack',
  'JSEvents',
  'specialHTMLTargets',
  'findCanvasEventTarget',
  'currentFullscreenStrategy',
  'restoreOldWindowedStyle',
  'UNWIND_CACHE',
  'ExitStatus',
  'getEnvStrings',
  'checkWasiClock',
  'initRandomFill',
  'randomFill',
  'emSetImmediate',
  'emClearImmediate_deps',
  'emClearImmediate',
  'promiseMap',
  'uncaughtExceptionCount',
  'exceptionLast',
  'exceptionCaught',
  'ExceptionInfo',
  'Browser',
  'getPreloadedImageData__data',
  'wget',
  'MONTH_DAYS_REGULAR',
  'MONTH_DAYS_LEAP',
  'MONTH_DAYS_REGULAR_CUMULATIVE',
  'MONTH_DAYS_LEAP_CUMULATIVE',
  'isLeapYear',
  'ydayFromDate',
  'preloadPlugins',
  'FS_modeStringToFlags',
  'FS_getMode',
  'FS_stdin_getChar_buffer',
  'FS_stdin_getChar',
  'FS_createDevice',
  'FS_readFile',
  'MEMFS',
  'wasmFSPreloadedFiles',
  'wasmFSPreloadedDirs',
  'wasmFSPreloadingFlushed',
  'wasmFSDevices',
  'wasmFSDeviceStreams',
  'FS',
  'FS_mknod',
  'FS_create',
  'FS_writeFile',
  'FS_mkdir',
  'FS_mkdirTree',
  'wasmFS$JSMemoryFiles',
  'wasmFS$backends',
  'wasmfsNodeIsWindows',
  'wasmfsOPFSDirectoryHandles',
  'wasmfsOPFSFileHandles',
  'wasmfsOPFSAccessHandles',
  'wasmfsOPFSBlobs',
  'tempFixedLengthArray',
  'miniTempWebGLFloatBuffers',
  'miniTempWebGLIntBuffers',
  'GL',
  'AL',
  'GLUT',
  'EGL',
  'GLEW',
  'IDBStore',
  'SDL',
  'SDL_gfx',
  'allocateUTF8',
  'allocateUTF8OnStack',
  'print',
  'printErr',
  'PThread',
  'terminateWorker',
  'cleanupThread',
  'registerTLSInit',
  'spawnThread',
  'exitOnMainThread',
  'proxyToMainThread',
  'proxiedJSCallArgs',
  'invokeEntryPoint',
  'checkMailbox',
  '_wasmWorkers',
  '_wasmWorkersID',
  '_wasmWorkerDelayedMessageQueue',
  '_wasmWorkerAppendToQueue',
  '_wasmWorkerRunPostMessage',
  '_wasmWorkerInitializeRuntime',
  'EmAudio',
  'EmAudioCounter',
  'emscriptenRegisterAudioObject',
  'emscriptenDestroyAudioContext',
  'emscriptenGetAudioObject',
  'emscriptenGetContextQuantumSize',
  '_EmAudioDispatchProcessorCallback',
  'InternalError',
  'BindingError',
  'throwInternalError',
  'throwBindingError',
  'registeredTypes',
  'awaitingDependencies',
  'typeDependencies',
  'tupleRegistrations',
  'structRegistrations',
  'sharedRegisterType',
  'whenDependentTypesAreResolved',
  'embind_charCodes',
  'embind_init_charCodes',
  'readLatin1String',
  'getTypeName',
  'getFunctionName',
  'heap32VectorToArray',
  'usesDestructorStack',
  'checkArgCount',
  'getRequiredArgCount',
  'createJsInvoker',
  'UnboundTypeError',
  'PureVirtualError',
  'GenericWireTypeSize',
  'EmValType',
  'EmValOptionalType',
  'throwUnboundTypeError',
  'ensureOverloadTable',
  'exposePublicSymbol',
  'replacePublicSymbol',
  'extendError',
  'createNamedFunction',
  'embindRepr',
  'registeredInstances',
  'getBasestPointer',
  'getInheritedInstance',
  'registeredPointers',
  'registerType',
  'integerReadValueFromPointer',
  'floatReadValueFromPointer',
  'readPointer',
  'runDestructors',
  'newFunc',
  'craftInvokerFunction',
  'embind__requireFunction',
  'genericPointerToWireType',
  'constNoSmartPtrRawPointerToWireType',
  'nonConstNoSmartPtrRawPointerToWireType',
  'init_RegisteredPointer',
  'RegisteredPointer',
  'RegisteredPointer_fromWireType',
  'runDestructor',
  'releaseClassHandle',
  'finalizationRegistry',
  'detachFinalizer_deps',
  'detachFinalizer',
  'attachFinalizer',
  'makeClassHandle',
  'init_ClassHandle',
  'ClassHandle',
  'throwInstanceAlreadyDeleted',
  'deletionQueue',
  'flushPendingDeletes',
  'delayFunction',
  'RegisteredClass',
  'shallowCopyInternalPointer',
  'downcastPointer',
  'upcastPointer',
  'char_0',
  'char_9',
  'makeLegalFunctionName',
  'emval_freelist',
  'emval_handles',
  'emval_symbols',
  'init_emval',
  'count_emval_handles',
  'Emval',
  'emval_methodCallers',
  'reflectConstruct',
];
unexportedSymbols.forEach(unexportedRuntimeSymbol);



var calledRun;

function callMain() {
  assert(runDependencies == 0, 'cannot call main when async dependencies remain! (listen on Module["onRuntimeInitialized"])');
  assert(typeof onPreRuns === 'undefined' || onPreRuns.length == 0, 'cannot call main when preRun functions remain to be called');

  var entryFunction = _main;

  var argc = 0;
  var argv = 0;

  try {

    var ret = entryFunction(argc, argv);

    // if we're not running an evented main loop, it's time to exit
    exitJS(ret, /* implicit = */ true);
    return ret;
  } catch (e) {
    return handleException(e);
  }
}

function stackCheckInit() {
  // This is normally called automatically during __wasm_call_ctors but need to
  // get these values before even running any of the ctors so we call it redundantly
  // here.
  // See $establishStackSpace for the equivalent code that runs on a thread
  assert(!ENVIRONMENT_IS_PTHREAD);
  _emscripten_stack_init();
  // TODO(sbc): Move writeStackCookie to native to to avoid this.
  writeStackCookie();
}

function run() {

  if (runDependencies > 0) {
    dependenciesFulfilled = run;
    return;
  }

  if ((ENVIRONMENT_IS_PTHREAD||ENVIRONMENT_IS_WASM_WORKER)) {
    readyPromiseResolve(Module);
    initRuntime();
    return;
  }

  stackCheckInit();

  preRun();

  // a preRun added a dependency, run will be called later
  if (runDependencies > 0) {
    dependenciesFulfilled = run;
    return;
  }

  function doRun() {
    // run may have just been called through dependencies being fulfilled just in this very frame,
    // or while the async setStatus time below was happening
    assert(!calledRun);
    calledRun = true;
    Module['calledRun'] = true;

    if (ABORT) return;

    initRuntime();

    preMain();

    readyPromiseResolve(Module);
    Module['onRuntimeInitialized']?.();

    var noInitialRun = Module['noInitialRun'];legacyModuleProp('noInitialRun', 'noInitialRun');
    if (!noInitialRun) callMain();

    postRun();
  }

  if (Module['setStatus']) {
    Module['setStatus']('Running...');
    setTimeout(() => {
      setTimeout(() => Module['setStatus'](''), 1);
      doRun();
    }, 1);
  } else
  {
    doRun();
  }
  checkStackCookie();
}

function checkUnflushedContent() {
  // Compiler settings do not allow exiting the runtime, so flushing
  // the streams is not possible. but in ASSERTIONS mode we check
  // if there was something to flush, and if so tell the user they
  // should request that the runtime be exitable.
  // Normally we would not even include flush() at all, but in ASSERTIONS
  // builds we do so just for this check, and here we see if there is any
  // content to flush, that is, we check if there would have been
  // something a non-ASSERTIONS build would have not seen.
  // How we flush the streams depends on whether we are in SYSCALLS_REQUIRE_FILESYSTEM=0
  // mode (which has its own special function for this; otherwise, all
  // the code is inside libc)
  var oldOut = out;
  var oldErr = err;
  var has = false;
  out = err = (x) => {
    has = true;
  }
  try { // it doesn't matter if it fails
    // In WasmFS we must also flush the WasmFS internal buffers, for this check
    // to work.
    _wasmfs_flush();
  } catch(e) {}
  out = oldOut;
  err = oldErr;
  if (has) {
    warnOnce('stdio streams had content in them that was not flushed. you should set EXIT_RUNTIME to 1 (see the Emscripten FAQ), or make sure to emit a newline when you printf etc.');
    warnOnce('(this may also be due to not including full filesystem support - try building with -sFORCE_FILESYSTEM)');
  }
}

if (Module['preInit']) {
  if (typeof Module['preInit'] == 'function') Module['preInit'] = [Module['preInit']];
  while (Module['preInit'].length > 0) {
    Module['preInit'].pop()();
  }
}

run();

// end include: postamble.js

// include: postamble_modularize.js
// In MODULARIZE mode we wrap the generated code in a factory function
// and return either the Module itself, or a promise of the module.
//
// We assign to the `moduleRtn` global here and configure closure to see
// this as and extern so it won't get minified.

moduleRtn = readyPromise;

// Assertion for attempting to access module properties on the incoming
// moduleArg.  In the past we used this object as the prototype of the module
// and assigned properties to it, but now we return a distinct object.  This
// keeps the instance private until it is ready (i.e the promise has been
// resolved).
for (const prop of Object.keys(Module)) {
  if (!(prop in moduleArg)) {
    Object.defineProperty(moduleArg, prop, {
      configurable: true,
      get() {
        abort(`Access to module property ('${prop}') is no longer possible via the module constructor argument; Instead, use the result of the module constructor.`)
      }
    });
  }
}
// end include: postamble_modularize.js



  return moduleRtn;
}
);
})();
(() => {
  // Create a small, never-async wrapper around Pd4WebModule which
  // checks for callers incorrectly using it with `new`.
  var real_Pd4WebModule = Pd4WebModule;
  Pd4WebModule = function(arg) {
    if (new.target) throw new Error("Pd4WebModule() should not be called with `new Pd4WebModule()`");
    return real_Pd4WebModule(arg);
  }
})();
globalThis.AudioWorkletModule = Pd4WebModule;
if (typeof exports === 'object' && typeof module === 'object') {
  module.exports = Pd4WebModule;
  // This default export looks redundant, but it allows TS to import this
  // commonjs style module.
  module.exports.default = Pd4WebModule;
} else if (typeof define === 'function' && define['amd'])
  define([], () => Pd4WebModule);
var isPthread = globalThis.self?.name?.startsWith('em-pthread');
var isNode = typeof globalThis.process?.versions?.node == 'string';
if (isNode) isPthread = require('worker_threads').workerData === 'em-pthread'

// When running as a pthread, construct a new instance on startup
isPthread && Pd4WebModule();
