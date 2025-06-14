#include "pd4web.hpp"

#include <string>

int PD4WEB_INSTANCES = 0;
Pd4WebGuiReceiverList Pd4WebGuiReceivers;
Pd4WebGuiReceiverList Pd4WebGuiSenders;

std::vector<Pd4Web *> Pd4WebInstances;

// t_pdinstance *Pd4WebPdInstance;

// ╭─────────────────────────────────────╮
// │        JavaScript Functions         │
// ╰─────────────────────────────────────╯
// Functions written in JavaScript Language, this are used for the WebAudio API.
// Then we don't need to pass the WebAudio Context as in version 1.0.
// clang-format off
// ─────────────────────────────────────
EM_JS(void, _JS_addSoundToggle, (void), {
    const soundSwitch = document.getElementById("Pd4WebAudioSwitch");
    if (!soundSwitch || typeof Pd4Web === "undefined") {
        return;
    }
    soundSwitch.addEventListener("click", () => {
        if (Pd4Web && typeof Pd4Web.soundToggle === "function") {
            Pd4Web.soundToggle();
        }
    });
});

// ─────────────────────────────────────
EM_JS(void, _JS_pd4webCppClass, (void *Pd4Web), {
    console.log("Received Pd4Web pointer:", Pd4Web);
});

// ─────────────────────────────────────
EM_JS(void, _JS_setIcon, (const char *icon, const char *animation), {
    let jsIcon = UTF8ToString(icon);
    let jsAnimation = UTF8ToString(animation);

    function tryCallSetSoundIcon() {
        if (typeof setSoundIcon === 'function') {
            setSoundIcon(jsIcon, jsAnimation);
        } else {
            setTimeout(() => {
                if (typeof setSoundIcon === 'function') {
                    setSoundIcon(jsIcon, jsAnimation);
                }
            }, 200); // Retry after 200ms
        }
    }

    setTimeout(() => tryCallSetSoundIcon(), 200);
});

// ─────────────────────────────────────
EM_JS(void, _JS_sendList, (void), {
    if (typeof Pd4Web.GuiReceivers === "undefined") {
        Pd4Web.GuiReceivers = {}; // defined in pd4web.cpp Pd4WebJsHelpers
    }

    Pd4Web.sendList = function (r, vec) {
        const vecLength = vec.length;
        var ok = Pd4Web._startMessage(r, vecLength);
        if (!ok) {
            console.error('Failed to start message');
            return;
        }
        for (let i = 0; i < vecLength; i++) {
            if (typeof vec[i] === 'string') {
                Pd4Web._addSymbol(r, vec[i]);
            } else if (typeof vec[i] === 'number') {
                Pd4Web._addFloat(r, vec[i]);
            } else{
                console.error('Invalid type');
            }
        }
        Pd4Web._finishMessage(r);
    };

});


// ─────────────────────────────────────
EM_JS(void, _JS_onReceived, (), {
    // Bangs 
    Pd4Web.onBangReceived = function(receiver, myFunc) {
        if (typeof Pd4Web._userBangFunc === 'undefined') {
            Pd4Web._userBangFunc = {};
        }
        const paramCount = myFunc.length;
        if (paramCount !== 0) {
            console.error('Invalid number of arguments for function, expected 0 arguments');
            return;
        }
        Pd4Web.bindReceiver(receiver);
        Pd4Web._userBangFunc[receiver] = myFunc;
    };

    // Floats
    Pd4Web.onFloatReceived = function(receiver, myFunc) {
        if (typeof Pd4Web._userFloatFunc === 'undefined') {
            Pd4Web._userFloatFunc = {};
        }
        const paramCount = myFunc.length;
        if (paramCount !== 1) {
            console.error('Invalid number of arguments for function, expected 1, just the float received');
            return;
        }
        Pd4Web.bindReceiver(receiver);
        Pd4Web._userFloatFunc[receiver] = myFunc;
    };

    // Symbols 
    Pd4Web.onSymbolReceived = function(receiver, myFunc) {
        if (typeof Pd4Web._userSymbolFunc === 'undefined') {
            Pd4Web._userSymbolFunc = {};
        }
        const paramCount = myFunc.length;
        if (paramCount !== 1) {
            console.error('Invalid number of arguments for function. Required 1, just the symbol (aka string) received');
            return;
        }
        Pd4Web.bindReceiver(receiver);
        Pd4Web._userSymbolFunc[receiver] = myFunc;
    };

    // Lists
    Pd4Web.onListReceived = function(receiver, myFunc) {
        if (typeof Pd4Web._userListFunc === 'undefined') {
            Pd4Web._userListFunc = {};
        }
        const paramCount = myFunc.length;
        if (paramCount !== 1) {
            console.error('Invalid number of arguments for function. Required 1, just the list received');
            return;
        }
        Pd4Web.bindReceiver(receiver);
        Pd4Web._userListFunc[receiver] = myFunc;
    };

    // TODO: Implementing the message receiver
});

// ─────────────────────────────────────
EM_JS(void, _JS_loadGui, (bool AutoTheming, double Zoom), {
    if (document.getElementById("pd4web-gui") != null){
        return;
    }
    let scripts = document.getElementsByTagName('script');
    let pd4webPath = null;
    for (let script of scripts) {
        if (script.src && script.src.includes('pd4web.js')) {
            pd4webPath = script.src.substring(0, script.src.lastIndexOf('/') + 1);
            break;
        }
    }

    var script = document.createElement('script');
    script.type = "text/javascript";
    script.src = pd4webPath + "pd4web.gui.js";
    script.id = "pd4web-gui";
    script.onload = function() {
        Pd4Web.Zoom = Zoom;
        Pd4WebInitGui("index.pd"); // defined in pd4web.gui.js
    };
    script.onerror = function() {
        console.warn("GUI file not found.");
    };
    document.head.appendChild(script); 
});

// ─────────────────────────────────────
EM_JS(void, _JS_resizeCanvas, (int x, int y, int zoom), {
    var width = x;
    var height = y;
    const patchDiv = document.getElementById("Pd4WebPatchDiv");
    patchDiv.style.width = (width * zoom) + "px";
    patchDiv.style.height = (height * zoom) + "px";
    patchDiv.style.marginLeft = "auto";
    patchDiv.style.marginRight = "auto";

});

// ─────────────────────────────────────
EM_JS(void, _JS_loadStyle, (void), {
    if (document.getElementById("pd4web-style") != null){
        return;
    }
    let scripts = document.getElementsByTagName('script');
    let pd4webPath = null;
    for (let script of scripts) {
        if (script.src && script.src.includes('pd4web.js')) {
            pd4webPath = script.src.substring(0, script.src.lastIndexOf('/') + 1);
            break;
        }
    }

    // Load the CSS file
    var link = document.createElement('link');
    link.rel = "stylesheet";
    link.type = "text/css";
    link.href = pd4webPath + "pd4web.style.css";
    link.id = "pd4web-style";
    document.head.appendChild(link);
    link.onerror = function() {
        console.warn("CSS file not found.");
    };
});

// ─────────────────────────────────────
EM_JS(void, _JS_alert, (const char *msg), {
    alert(UTF8ToString(msg));
});

// ─────────────────────────────────────
EM_JS(void, _JS_addAlertOnError, (), {
    window.addEventListener('error', function(event) {
      console.log(event.filename);
    });
});

// ─────────────────────────────────────
EM_JS(void, _JS_post, (const char *msg), {
    console.log(UTF8ToString(msg));
});
    
EM_JS(void, _JS_addMessagePort, (EMSCRIPTEN_AUDIO_WORKLET_NODE_T audioWorkletNode), {
    Pd4WebAudioWorkletNode = emscriptenGetAudioObject(audioWorkletNode);
    Pd4WebAudioWorkletNode.port.onmessage = function(event) {
        console.log(event);
    };
});


// ─────────────────────────────────────
EM_JS(void, _JS_getMicAccess, (EMSCRIPTEN_WEBAUDIO_T audioContext, EMSCRIPTEN_AUDIO_WORKLET_NODE_T audioWorkletNode, int nInCh), {
    Pd4WebAudioContext = emscriptenGetAudioObject(audioContext);
    Pd4WebAudioWorkletNode = emscriptenGetAudioObject(audioWorkletNode);

    async function _GetMicAccess(stream) {
      try {
        const SourceNode = Pd4WebAudioContext.createMediaStreamSource(stream);
        SourceNode.connect(Pd4WebAudioWorkletNode);
        Pd4WebAudioWorkletNode.connect(Pd4WebAudioContext.destination);
      } catch (err) {
        alert(err);
      }
    }

    if (nInCh > 0) {
        navigator.mediaDevices
            .getUserMedia({
                video: false,
                audio: {
                    echoCancellation: false,
                    noiseSuppression: false,
                    autoGainControl: false,

                },
            })
            .then((stream) => _GetMicAccess(stream));
    } else {
        Pd4WebAudioWorkletNode.connect(Pd4WebAudioContext.destination);
    }
});

// ─────────────────────────────────────
EM_JS(void, _JS_suspendAudioWorkLet, (EMSCRIPTEN_WEBAUDIO_T audioContext),{
    Pd4WebAudioContext = emscriptenGetAudioObject(audioContext);
    Pd4WebAudioContext.suspend();
});

//╭─────────────────────────────────────╮
//│          JS Midi Functions          │
//╰─────────────────────────────────────╯
EM_JS(void, _JS_loadMidi, (void), {
    if (document.getElementById("pd4web-midi") != null){
        return;
    }
    let scripts = document.getElementsByTagName('script');
    let pd4webPath = null;
    for (let script of scripts) {
        if (script.src && script.src.includes('pd4web.js')) {
            pd4webPath = script.src.substring(0, script.src.lastIndexOf('/') + 1);
            break;
        }
    }
    
    var script = document.createElement('script');
    script.type = "text/javascript";
    script.src = pd4webPath + "pd4web.midi.js";
    script.id = "pd4web-midi";
    script.onload = function() {
        if (typeof WebMidi != "object") {
            console.error("Midi: failed to find the 'WebMidi' object");
            return;
        }
        WebMidi.enable(function (err) {
            if (err) {
                console.error("Midi: failed to enable midi", err);
                return;
            }

            WebMidi.inputs.forEach(input => {
                console.log(input.channels);
                input.channels[1].addListener("noteon", function(e) {
                    if (typeof e.channel === 'undefined') {
                        Pd4Web.noteOn(1, e.note.number, e.rawVelocity);
                    } else{
                        Pd4Web.noteOn(e.channel, e.note.number, e.rawVelocity);
                    }
                });
                input.channels[1].addListener("noteoff", function(e) {
                    if (typeof e.channel === 'undefined') {
                        Pd4Web.noteOn(1, e.note.number, 0);
                    } else{
                        Pd4Web.noteOn(e.channel, e.note.number, 0);
                    }
                });
            });
        }, false); 

    };
    document.head.appendChild(script);
});

//╭─────────────────────────────────────╮
//│         JS Receivers Hooks          │
//╰─────────────────────────────────────╯
EM_JS(void, _JS_receiveBang, (const char *r),{
    var source = UTF8ToString(r);
    if (source in Pd4Web.GuiReceivers) {
        for (const data of Pd4Web.GuiReceivers[source]) {
            switch (data.type) {
                case "bng":
                    GuiBngUpdateCircle(data);
                    break;
                case "tgl":
                    data.value = data.value ? 0 : data.default_value;
                    GuiTglUpdateCross(data);
                    break;
                case "vsl":
                case "hsl":
                    GuiSliderBang(data);
                    break;
                case "vradio":
                case "hradio":
                    Pd4Web.sendFloat(data.send, data.value);
                    break;
            }
        }
    } else{
        if (typeof Pd4Web._userBangFunc === 'undefined'){
          alert("Turn audio on first");
          return;
        }
        let bangFunc = Pd4Web._userBangFunc[source];
        if (typeof bangFunc === 'function') {
            bangFunc();
        }
    }
});

// ─────────────────────────────────────
EM_JS(void, _JS_receiveFloat, (const char *r, float f),{
    var source = UTF8ToString(r);
    if (source in Pd4Web.GuiReceivers) {
        for (const data of Pd4Web.GuiReceivers[source]) {
            switch (data.type) {
                case "bng":
                    GuiBngUpdateCircle(data);
                    break;
                case "tgl":
                    data.value = data.value ? 0 : data.default_value;
                    GuiTglUpdateCross(data);
                    break;
                case "vsl":
                case "hsl":
                    GuiSliderSet(data, f);
                    GuiSliderBang(data);
                    break;
                case "nbx":
                    GuiNbxUpdateNumber(data, f);
                    break;
                case "vradio":
                case "hradio":
                    data.value = Math.min(Math.max(Math.floor(f), 0), data.number - 1);
                    GuiRadioUpdateButton(data);
                    Pd4Web.sendFloat(data.send, data.value);
                    break;
                case "vu":
                    data.value = f;
                    GuiVuUpdateGain(data);
                    break;
            }
        }
    } else{
        if (typeof Pd4Web._userFloatFunc === 'undefined'){
          alert("Turn audio on first");
          return;
        }
        let floatFunc = Pd4Web._userFloatFunc[source];
        if (typeof floatFunc === 'function') {
            floatFunc(f);
        }
    }
});

// ─────────────────────────────────────
EM_JS(void, _JS_receiveSymbol, (const char *r, const char *s),{
    var source = UTF8ToString(r);
    var symbol = UTF8ToString(s);
    if (source in Pd4Web.GuiReceivers) {
        for (const data of Pd4Web.GuiReceivers[source]) {
            switch (data.type) {
                case "bng":
                    GuiBngUpdateCircle(data);
                    break;
            }
        }
    } else{
        if (typeof Pd4Web._userSymbolFunc === 'undefined'){
          alert("Turn audio on first");
          return;
        }
        let symbolFunc = Pd4Web._userSymbolFunc[source];
        if (typeof symbolFunc === 'function') {
            symbolFunc(symbol);
        }
    }
});

// ─────────────────────────────────────
EM_JS(void, _JS_receiveList, (const char *r),{
    var source = UTF8ToString(r);
    if (source in Pd4Web.GuiReceivers) {
        return;
    } else{
        if (typeof Pd4Web._userListFunc === 'undefined'){
          alert("Turn audio on first");
          return;
        }
        let listFunc = Pd4Web._userListFunc[source];
        const listSize = Pd4Web._getReceivedListSize(source);
        var pdList = [];
        for (let i = 0; i < listSize; i++) {
            let type = Pd4Web._getItemFromListType(source, i);
            if (type === "float") {
                pdList.push(Pd4Web._getItemFromListFloat(source, i));
            } else if (type === "symbol") {
                pdList.push(Pd4Web._getItemFromListSymbol(source, i));
            } else{
                console.error("Invalid type");
            }
        }
        if (typeof listFunc === 'function') {
            listFunc(pdList);
        }
    }
});

// ─────────────────────────────────────
EM_JS(void, _JS_domIsDefined, (void),{
      console.log(Pd4Web);
});


// ─────────────────────────────────────
EM_JS(void, _JS_createTgl, (const char *p, float x_pos, float y_pos, float size, float zoom, int id, const char *bg), {
    var objpointer = UTF8ToString(p);
    var background = UTF8ToString(bg);

    // Create group <g>
    let groupProps = {
        id: "tgl_" + id,
        class: "border clickable"
    };
    var groupObj = CreateItem("g", groupProps);

    // Create rectangle
    let rectProps = {
        x: x_pos,
        y: y_pos,
        rx: 2,
        ry: 2,
        width: size,
        height: size,
        fill: background
    };
    var rectObj = CreateItem("rect", rectProps);

    // Create lines for the cross
    let line1Props = {
        x1: x_pos + 2,
        y1: y_pos + 2,
        x2: x_pos + size - 2,
        y2: y_pos + size - 2,
        stroke: "none",         // Initially hidden
        "stroke-width": 2
    };
    var line1Obj = CreateItem("line", line1Props);

    let line2Props = {
        x1: x_pos + 2,
        y1: y_pos + size - 2,
        x2: x_pos + size - 2,
        y2: y_pos + 2,
        stroke: "none",         // Initially hidden
        "stroke-width": 2
    };
    var line2Obj = CreateItem("line", line2Props);

    // Append rect & lines to the group
    groupObj.appendChild(rectObj);
    groupObj.appendChild(line1Obj);
    groupObj.appendChild(line2Obj);

    // Append the group to the main <svg>
    const svgElement = document.getElementById("Pd4WebCanvas");
    svgElement.appendChild(groupObj);

    // Keep track of whether the cross is visible
    let crossVisible = false;

    // Add event listener for clicks
    groupObj.addEventListener("click", function(e) {
        // 1) Let Pure Data know we clicked
        const svgBox = svgElement.getBoundingClientRect();
        const x = Math.round((e.clientX - svgBox.x) / zoom);
        const y = Math.round((e.clientY - svgBox.y) / zoom);
        Pd4Web._objclick(objpointer, x, y);

        // 2) Toggle cross visibility
        crossVisible = !crossVisible;
        if (crossVisible) {
            line1Obj.setAttribute("stroke", "black");
            line2Obj.setAttribute("stroke", "black");
        } else {
            line1Obj.setAttribute("stroke", "none");
            line2Obj.setAttribute("stroke", "none");
        }
    });
});

// ─────────────────────────────────────
EM_JS(void, _JS_createBng, (const char *p, float x_pos, float y_pos, float size, int id, const char *bg), {
    var objpointer = UTF8ToString(p);
    var background = UTF8ToString(bg);
    var circleId = "bng_circle_" + id;
    
    // Create rectangle
    let rect = {
        id: "bng_" + id,
        x: x_pos,
        y: y_pos,
        rx: 2,
        ry: 2,
        width: size,
        height: size,
        fill: background,
        class: "border clickable",
    };

    var guiObj = CreateItem("rect", rect);

    // Create circle
    let circleParams = {
        id: circleId,
        cx: x_pos + size/2,
        cy: y_pos + size/2,
        r: size * 0.45,
        stroke: "black",
        fill: "none",
        style: "pointer-events: none;",
    };

    var circleObj = CreateItem("circle", circleParams);

    // Add click event
    guiObj.addEventListener("click", function(e) {
        var circle = document.getElementById(circleId);
        if (circle) {
            // Flash the circle
            circle.style.fill = "black";
            setTimeout(() => {
                circle.style.fill = "none";
            }, 300);
        }
        Pd4Web._objclick(objpointer, e.clientX, e.clientY);
    });
});

// ─────────────────────────────────────
EM_JS(void, _JS_receiveMessage, (const char *r),{
    var source = UTF8ToString(r);
    if (typeof Pd4Web._getReceivedListSize === 'undefined'){
          alert("Turn audio on first");
          return;
    }
    const listSize = Pd4Web._getReceivedListSize(source);
    var pdList = [];
    for (let i = 0; i < listSize; i++) {
        let type = Pd4Web._getItemFromListType(source, i);
        if (type === "float") {
            pdList.push(Pd4Web._getItemFromListFloat(source, i));
        } else if (type === "symbol") {
            pdList.push(Pd4Web._getItemFromListSymbol(source, i));
        } else{
            console.error("Invalid type");
        }
    }

    if (source in Pd4Web.GuiReceivers) {
        let sel = Pd4Web._getMessageSelector(source);
        MessageListener(source, sel, pdList); 
        return;
    } else{
        console.error("Not implemented");
    }

});

// clang-format on
// ─────────────────────────────────────
int Pd4Web::_getReceivedListSize(std::string r) {
    for (auto &GuiReceiver : Pd4WebGuiReceivers) {
        if (GuiReceiver.Receiver == r) {
            return GuiReceiver.List.size();
        }
    }
    return -1;
}

// ─────────────────────────────────────
std::string Pd4Web::_getItemFromListType(std::string r, int i) {
    for (auto &GuiReceiver : Pd4WebGuiReceivers) {
        if (GuiReceiver.Receiver == r) {
            if (std::holds_alternative<float>(GuiReceiver.List[i])) {
                return "float";
            } else if (std::holds_alternative<std::string>(GuiReceiver.List[i])) {
                return "symbol";
            }
        }
    }
    return "";
}

// ─────────────────────────────────────
std::string Pd4Web::_getItemFromListSymbol(std::string r, int i) {
    for (auto &GuiReceiver : Pd4WebGuiReceivers) {
        if (GuiReceiver.Receiver == r) {
            if (std::holds_alternative<std::string>(GuiReceiver.List[i])) {
                return std::get<std::string>(GuiReceiver.List[i]);
            }
        }
    }
    return "";
}

// ─────────────────────────────────────
float Pd4Web::_getItemFromListFloat(std::string r, int i) {
    for (auto &GuiReceiver : Pd4WebGuiReceivers) {
        if (GuiReceiver.Receiver == r) {
            if (std::holds_alternative<float>(GuiReceiver.List[i])) {
                return std::get<float>(GuiReceiver.List[i]);
            }
        }
    }
    return 0;
}

// ─────────────────────────────────────
std::string Pd4Web::_getMessageSelector(std::string r) {
    for (auto &GuiReceiver : Pd4WebGuiReceivers) {
        if (GuiReceiver.Receiver == r) {
            return GuiReceiver.Selector;
        }
    }
    return "";
}

// ╭─────────────────────────────────────╮
// │       Audio Worklet Receivers       │
// ╰─────────────────────────────────────╯
void Pd4Web::receivedBang(const char *r) {
    LOG("Pd4Web::receivedBang");
    for (auto &GuiReceiver : Pd4WebGuiReceivers) {
        if (GuiReceiver.Receiver == r) {
            GuiReceiver.BeingUpdated = true;
            GuiReceiver.Updated = true;
            GuiReceiver.Type = Pd4WebGuiConnector::BANG;
            GuiReceiver.BeingUpdated = false;
        }
    }
};

// ─────────────────────────────────────
void Pd4Web::receivedFloat(const char *r, float f) {
    LOG("Pd4Web::receivedFloat");
    for (auto &GuiReceiver : Pd4WebGuiReceivers) {
        if (GuiReceiver.Receiver == r) {
            GuiReceiver.BeingUpdated = true;
            GuiReceiver.Updated = true;
            GuiReceiver.Type = Pd4WebGuiConnector::FLOAT;
            GuiReceiver.Float = f;
            GuiReceiver.BeingUpdated = false;
        }
    }
};

// ─────────────────────────────────────
void Pd4Web::receivedSymbol(const char *r, const char *s) {
    LOG("Pd4Web::receivedSymbol");
    for (auto &GuiReceiver : Pd4WebGuiReceivers) {
        if (GuiReceiver.Receiver == r) {
            GuiReceiver.BeingUpdated = true;
            GuiReceiver.Updated = true;
            GuiReceiver.Type = Pd4WebGuiConnector::SYMBOL;
            GuiReceiver.Symbol = s;
            GuiReceiver.BeingUpdated = false;
        }
    }
};

// ─────────────────────────────────────
void Pd4Web::receivedList(const char *r, int argc, t_atom *argv) {
    LOG("Pd4Web::receivedList");
    for (auto &GuiReceiver : Pd4WebGuiReceivers) {
        if (GuiReceiver.Receiver == r) {
            GuiReceiver.Updated = true;
            GuiReceiver.Type = Pd4WebGuiConnector::LIST;
            GuiReceiver.List.clear();
            for (int i = 0; i < argc; i++) {
                t_atom *a = argv + i;
                if (a->a_type == A_FLOAT) {
                    GuiReceiver.List.push_back(libpd_get_float(a));
                } else if (a->a_type == A_SYMBOL) {
                    GuiReceiver.List.push_back(libpd_get_symbol(a));
                }
            }
            break;
        }
    }
};

// ─────────────────────────────────────
void Pd4Web::receivedMessage(const char *r, const char *s, int argc, t_atom *argv) {
    LOG("Pd4Web::receivedMessage");
    for (auto &GuiReceiver : Pd4WebGuiReceivers) {
        if (GuiReceiver.Receiver == r) {
            GuiReceiver.Updated = true;
            GuiReceiver.Type = Pd4WebGuiConnector::MESSAGE;
            GuiReceiver.Selector = s;
            GuiReceiver.List.clear();
            for (int i = 0; i < argc; i++) {
                t_atom *a = argv + i;
                if (a->a_type == A_FLOAT) {
                    GuiReceiver.List.push_back(libpd_get_float(a));
                } else if (a->a_type == A_SYMBOL) {
                    GuiReceiver.List.push_back(libpd_get_symbol(a));
                } else {
                    _JS_post("Unhandled message");
                }
            }
            break;
        }
    }
    return;
}

// ╭─────────────────────────────────────╮
// │           Receivers Hooks           │
// ╰─────────────────────────────────────╯
void Pd4Web::post(const char *message) {
    if (message[0] == '\n') {
        return;
    }
    _JS_post(message);
    return;
}

// ╭─────────────────────────────────────╮
// │            Senders Hooks            │
// ╰─────────────────────────────────────╯
bool Pd4Web::_startMessage(std::string r, int argc) {
    bool found = false;
    for (auto &GuiSender : Pd4WebGuiSenders) {
        if (GuiSender.Sender == r) {
            GuiSender.List.clear();
            GuiSender.Updated = true;
            found = true;
        }
    }

    if (!found) {
        Pd4WebGuiConnector GuiSender;
        GuiSender.Sender = r;
        GuiSender.Type = Pd4WebGuiConnector::LIST;
        GuiSender.Updated = true;
        Pd4WebGuiSenders.push_back(GuiSender);
    }

    return true;
}

// ─────────────────────────────────────
void Pd4Web::_addFloat(std::string r, float f) {
    for (auto &GuiSender : Pd4WebGuiSenders) {
        if (GuiSender.Sender == r) {
            GuiSender.Updated = true;
            GuiSender.List.push_back(f);
        }
    }
}

// ─────────────────────────────────────
void Pd4Web::_addSymbol(std::string r, std::string s) {
    for (auto &GuiSender : Pd4WebGuiSenders) {
        if (GuiSender.Sender == r) {
            GuiSender.Updated = true;
            GuiSender.List.push_back(s);
        }
    }
}

// ─────────────────────────────────────
int Pd4Web::_finishMessage(std::string s) {
    //
    return true;
}

// ─────────────────────────────────────
bool Pd4Web::sendBang(std::string s) {
    bool found = false;
    for (auto &GuiSender : Pd4WebGuiSenders) {
        if (GuiSender.Sender == s) {
            GuiSender.Updated = true;
            GuiSender.Type = Pd4WebGuiConnector::BANG;
            found = true;
        }
    }

    if (!found) {
        Pd4WebGuiConnector GuiSender;
        GuiSender.Sender = s;
        GuiSender.Updated = true;
        GuiSender.Type = Pd4WebGuiConnector::BANG;
        Pd4WebGuiSenders.push_back(GuiSender);
    }
    return true;
}

// ─────────────────────────────────────
bool Pd4Web::sendFloat(std::string s, float f) {
    bool found = false;
    for (auto &GuiSender : Pd4WebGuiSenders) {
        if (GuiSender.Sender == s) {
            GuiSender.Updated = true;
            GuiSender.Type = Pd4WebGuiConnector::FLOAT;
            GuiSender.Float = f;
            found = true;
        }
    }

    if (!found) {
        Pd4WebGuiConnector GuiSender;
        GuiSender.Sender = s;
        GuiSender.Updated = true;
        GuiSender.Type = Pd4WebGuiConnector::FLOAT;
        GuiSender.Float = f;
        Pd4WebGuiSenders.push_back(GuiSender);
    }
    return true;
}

// ─────────────────────────────────────
bool Pd4Web::sendSymbol(std::string s, std::string thing) {
    LOG("Pd4Web::sendSymbol");

    bool found = false;
    for (auto &GuiSender : Pd4WebGuiSenders) {
        if (GuiSender.Sender == s) {
            GuiSender.Updated = true;
            GuiSender.Type = Pd4WebGuiConnector::SYMBOL;
            GuiSender.Symbol = thing;
            found = true;
        }
    }

    if (!found) {
        Pd4WebGuiConnector GuiSender;
        GuiSender.Sender = s;
        GuiSender.Updated = true;
        GuiSender.Type = Pd4WebGuiConnector::SYMBOL;
        GuiSender.Symbol = thing;
        Pd4WebGuiSenders.push_back(GuiSender);
    }
    return true;
}

// ─────────────────────────────────────
void Pd4Web::noteOn(int channel, int pitch, int velocity) {
    LOG("Pd4Web::noteOn");
    libpd_noteon(channel, pitch, velocity);
}

// ─────────────────────────────────────
void Pd4Web::bindReceiver(std::string s) {
    LOG("Pd4Web::bindReceiver");
    void *Receiver = libpd_bind(s.c_str());
    for (auto &GuiReceiver : Pd4WebGuiReceivers) {
        if (GuiReceiver.Receiver == s) {
            return;
        }
    }
    Pd4WebGuiConnector GuiReceiver;
    GuiReceiver.Receiver = s;
    Pd4WebGuiReceivers.push_back(GuiReceiver);
    return;
}

// ─────────────────────────────────────
void Pd4Web::addGuiReceiver(std::string s) {
    LOG("Pd4Web::addGuiReceiver");
    m_Receivers.push_back(s);
    return;
}

// ─────────────────────────────────────
void Pd4Web::bindGuiReceivers() {
    LOG("Pd4Web::bindGuiReceivers");
    for (auto &s : m_Receivers) {
        bindReceiver(s);
    }
    m_Receivers.clear();
    return;
}

// ─────────────────────────────────────
void Pd4Web::unbindReceiver() {
    // TODO:

    return;
}

// ╭─────────────────────────────────────╮
// │            WebAudioPatch            │
// ╰─────────────────────────────────────╯
/**
 * Process the audio block.
 *
 * This function processes the audio block using libpd_process_float.
 *
 * @param numInputs Number of input buffers.
 * @param inputs Array of input audio frames.
 * @param numOutputs Number of output buffers.
 * @param outputs Array of output audio frames.
 * @param numParams Number of audio parameters.
 * @param params Array of audio parameters.
 * @param userData Pointer to user data (not used in this function).
 * @return true if processing succeeded, false otherwise.
 */

extern "C" int processing_block = 0;
EM_BOOL Pd4Web::process(int numInputs, const AudioSampleFrame *In, int numOutputs,
                        AudioSampleFrame *Out, int numParams, const AudioParamFrame *params,
                        void *data) {

    UserData *userData = (UserData *)data;
    int instance = userData->instance;

    for (auto &GuiSender : Pd4WebGuiSenders) {
        if (GuiSender.Updated) {
            switch (GuiSender.Type) {
            case Pd4WebGuiConnector::BANG: {
                libpd_bang(GuiSender.Sender.c_str());
                break;
            }
            case Pd4WebGuiConnector::FLOAT: {
                libpd_float(GuiSender.Sender.c_str(), GuiSender.Float);
                break;
            }
            case Pd4WebGuiConnector::SYMBOL: {
                libpd_symbol(GuiSender.Sender.c_str(), GuiSender.Symbol.c_str());
                break;
            }
            case Pd4WebGuiConnector::LIST: {
                int size = GuiSender.List.size();
                libpd_start_message(size);
                for (int i = 0; i < size; i++) {
                    if (std::holds_alternative<float>(GuiSender.List[i])) {
                        float f = std::get<float>(GuiSender.List[i]);
                        libpd_add_float(f);
                    } else if (std::holds_alternative<std::string>(GuiSender.List[i])) {
                        std::string s = std::get<std::string>(GuiSender.List[i]);
                        libpd_add_symbol(s.c_str());
                    }
                }
                libpd_finish_list(GuiSender.Sender.c_str());

                break;
            }
            case Pd4WebGuiConnector::MESSAGE: {
                printf("Unhandled message\n");
            }
            }
            GuiSender.Updated = false;
        }
    }

    int ChCount = Out[0].numberOfChannels;
    float LibPdOuts[128 * ChCount];

    // TODO: Temporary fix, thing other way
    processing_block = 1;
    libpd_process_float(2, In[0].data, LibPdOuts);
    processing_block = 0;

    // TODO: Fix multiple channels
    int OutI = 0;
    for (int i = 0; i < ChCount; i++) {
        for (int j = i; j < (128 * ChCount); j += ChCount) {
            Out[0].data[OutI] = LibPdOuts[j];
            OutI++;
        }
    }

    return EM_TRUE;
}

// ─────────────────────────────────────
void Pd4Web::initGuiInterface(void) {
    t_canvas *canvas = pd_getcanvaslist();
    int canvasWidth = canvas->gl_pixwidth;
    int canvasHeight = canvas->gl_pixheight;
    if (canvasWidth == 0 && canvasHeight == 0) {
        canvasWidth = canvas->gl_screenx2 - canvas->gl_screenx1;
        canvasHeight = canvas->gl_screeny2 - canvas->gl_screeny1;
    }

    EM_ASM(
        {
            var width = $0;
            var height = $1;
            var zoom = $2;

            const patchDiv = document.getElementById("Pd4WebPatchDiv");
            patchDiv.style.width = (width * zoom) + "px";
            patchDiv.style.height = (height * zoom) + "px";
            patchDiv.style.marginLeft = "auto";
            patchDiv.style.marginRight = "auto";

            const canvas = document.getElementById("Pd4WebCanvas");
            const value = "0 0 " + width + " " + height;
            canvas.setAttributeNS(null, "viewBox", value);
        },
        canvasWidth, canvasHeight, PD4WEB_PATCH_ZOOM);

    int objId = 0;
    for (t_gobj *obj = canvas->gl_list; obj; obj = obj->g_next) {
        std::string obj_name = obj->g_pd->c_name->s_name;
        if (obj_name == "tgl") {
            t_toggle *tgl = (t_toggle *)obj;
            int color = tgl->x_gui.x_bcol;
            std::string hexcolor = std::format("#{:06x}", color);
            std::string pointer = std::format("{}", static_cast<void *>(obj));
            _JS_createTgl(pointer.c_str(), tgl->x_gui.x_obj.te_xpix, tgl->x_gui.x_obj.te_ypix,
                          tgl->x_gui.x_w, PD4WEB_PATCH_ZOOM, objId, hexcolor.c_str());
        } else if (obj_name == "bng") {
            t_bng *bng = (t_bng *)obj;
            int color = bng->x_gui.x_bcol;
            std::string hexcolor = std::format("#{:06x}", color);
            std::string pointer = std::format("{}", static_cast<void *>(obj));
            _JS_createBng(pointer.c_str(), bng->x_gui.x_obj.te_xpix, bng->x_gui.x_obj.te_ypix,
                          bng->x_gui.x_w, objId, hexcolor.c_str());
        }
        objId++;
    }

    // rezise canvas
}

// ─────────────────────────────────────
/**
 * Callback function called when AudioWorkletProcessor is created.
 *
 * This function handles the creation of AudioWorkletProcessor and handles errors if creation
 * fails.
 *
 * @param audioContext The Emscripten Web Audio context.
 * @param success Boolean indicating whether creation of AudioWorkletProcessor was successful.
 * @param userData Pointer to user data (not used in this function).
 */
void Pd4Web::audioWorkletProcessorCreated(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success,
                                          void *userData) {

    if (!success) {
        _JS_alert("Failed to create AudioWorkletProcessor, please report!\n");
        return;
    }

    uint32_t SR = PD4WEB_SR;
    int NInCh = PD4WEB_CHS_IN;
    int NOutCh = PD4WEB_CHS_OUT;

    int nOutChannelsArray[1] = {NOutCh};

    EmscriptenAudioWorkletNodeCreateOptions options = {
        .numberOfInputs = NInCh,
        .numberOfOutputs = 1,
        .outputChannelCounts = nOutChannelsArray,
    };

    // turn audio on
    libpd_start_message(1);
    libpd_add_float(1.0f);
    libpd_finish_message("pd", "dsp");
    libpd_init_audio(NInCh, NOutCh, SR);

    EMSCRIPTEN_AUDIO_WORKLET_NODE_T AudioWorkletNode = emscripten_create_wasm_audio_worklet_node(
        audioContext, "pd4web", &options, &Pd4Web::process, userData);
    _JS_getMicAccess(audioContext, AudioWorkletNode, NInCh);
}

// ─────────────────────────────────────
/**
 * Callback function called when WebAudio worklet thread is initialized.
 *
 * This function handles the initialization of WebAudio worklet thread and
 * asynchronously creates an AudioWorkletProcessor.
 *
 * @param audioContext The Emscripten Web Audio context.
 * @param success Boolean indicating whether initialization was successful.
 * @param userData Pointer to user data (not used in this function).
 */
void Pd4Web::audioWorkletInit(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData) {
    LOG("Pd4Web::audioWorkletInit");
    if (!success) {
        _JS_alert("WebAudio worklet thread initialization failed!\n");
        return;
    }

    WebAudioWorkletProcessorCreateOptions opts = {
        .name = "pd4web",
    };

    emscripten_create_wasm_audio_worklet_processor_async(
        audioContext, &opts, &Pd4Web::audioWorkletProcessorCreated, userData);
}

// ─────────────────────────────────────
/**
 * Resumes the audio context synchronously.
 */
void Pd4Web::resumeAudio() { emscripten_resume_audio_context_sync(m_Context); }

/**
 * Suspends the audio worklet using JavaScript.
 */
void Pd4Web::suspendAudio() { _JS_suspendAudioWorkLet(m_Context); }

/* Sound Switch */
void Pd4Web::soundToggle() {
    if (m_audioSuspended) {
        resumeAudio();
        m_audioSuspended = false;
        if (PD4WEB_GUI) {
            _JS_setIcon("--sound-on", "");
        }
    } else {
        suspendAudio();
        m_audioSuspended = true;
        if (PD4WEB_GUI) {
            _JS_setIcon("--sound-off", "");
        }
    }
}
// ╭─────────────────────────────────────╮
// │            Lua Functions            │
// ╰─────────────────────────────────────╯
void Pd4Web::click(std::string p, int xpix, int ypix) {
    t_canvas *canvas = pd_getcanvaslist();
    int objId = 0;
    for (t_gobj *obj = canvas->gl_list; obj; obj = obj->g_next) {
        std::string obj_p = std::format("{}", static_cast<void *>(obj));
        if (obj_p == p) {
            const struct _widgetbehavior *c_wb; // Confirmed as const
            c_wb = obj->g_pd->c_wb;
            t_clickfn w_clickfn = c_wb->w_clickfn;
            if (w_clickfn) {
                w_clickfn(obj, canvas->gl_owner, xpix, ypix, 0, 0, 0, 1);
            }
        }
    }
}

// ╭─────────────────────────────────────╮
// │            Init Function            │
// ╰─────────────────────────────────────╯
void Pd4Web::init() {
    PD4WEB_INSTANCES++;
    LOG("Pd4Web::init");

    uint32_t SR = PD4WEB_SR;
    float NInCh = PD4WEB_CHS_IN;
    float NOutCh = PD4WEB_CHS_OUT;

    EmscriptenWebAudioCreateAttributes attrs = {
        .latencyHint = "interactive",
        .sampleRate = SR,
    };

    UserData *userData = new UserData();
    userData->instance = PD4WEB_INSTANCES;

    // Start the audio context
    EMSCRIPTEN_WEBAUDIO_T AudioContext = emscripten_create_audio_context(&attrs);
    emscripten_start_wasm_audio_worklet_thread_async(AudioContext, WasmAudioWorkletStack,
                                                     sizeof(WasmAudioWorkletStack),
                                                     Pd4Web::audioWorkletInit, (void *)userData);
    m_Context = AudioContext;

    // After load, it defines some extra functions
    _JS_sendList();
    _JS_onReceived();

    // Bind the receivers
    bindGuiReceivers();

    if (PD4WEB_GUI) {
        _JS_setIcon("--sound-on", "");
        _JS_addSoundToggle();
    }
    m_audioSuspended = false;

    return;
}

// ╭─────────────────────────────────────╮
// │              Main Loop              │
// ╰─────────────────────────────────────╯
void Pd4Web::vis(void) {
    t_canvas *canvas = pd_getcanvaslist();
    int canvasWidth = canvas->gl_pixwidth;
    int canvasHeight = canvas->gl_pixheight;
    if (canvasWidth == 0 && canvasHeight == 0) {
        canvasWidth = canvas->gl_screenx2 - canvas->gl_screenx1;
        canvasHeight = canvas->gl_screeny2 - canvas->gl_screeny1;
    }

    int objId = 0;
    for (t_gobj *obj = canvas->gl_list; obj; obj = obj->g_next) {
        const t_widgetbehavior *wb = (t_widgetbehavior *)obj->g_pd->c_wb;
        wb->w_visfn(obj, canvas, 1);
    }
}

// ─────────────────────────────────────
void Pd4Web::guiLoop() {
    LOG("guiLoop");

    for (auto &GuiReceiver : Pd4WebGuiReceivers) {
        if (GuiReceiver.Updated) {
            switch (GuiReceiver.Type) {
            case Pd4WebGuiConnector::BANG: {
                _JS_receiveBang(GuiReceiver.Receiver.c_str());
                break;
            }
            case Pd4WebGuiConnector::FLOAT: {
                _JS_receiveFloat(GuiReceiver.Receiver.c_str(), GuiReceiver.Float);
                break;
            }
            case Pd4WebGuiConnector::SYMBOL: {
                _JS_receiveSymbol(GuiReceiver.Receiver.c_str(), GuiReceiver.Symbol.c_str());
                break;
            }
            case Pd4WebGuiConnector::LIST: {
                _JS_receiveList(GuiReceiver.Receiver.c_str());
                break;
            }
            case Pd4WebGuiConnector::MESSAGE: {
                _JS_receiveMessage(GuiReceiver.Receiver.c_str());
            }
            }
            GuiReceiver.Updated = false;
        }
    }

#if PD4WEB_LUA
    pd4weblua_draw();
#endif
}

// ╭─────────────────────────────────────╮
// │            Main Function            │
// ╰─────────────────────────────────────╯
int main() {
    LOG("main");
    _JS_setIcon("--sound-loading", "spin 2s linear infinite");

    if (PD4WEB_GUI) {
        _JS_loadStyle();
        emscripten_set_window_title(PD4WEB_PROJECT_NAME);
    }
    _JS_addAlertOnError();

    if (PD4WEB_MIDI) {
        _JS_post("Loading Midi");
        _JS_loadMidi();
    }

    libpd_set_printhook(libpd_print_concatenator);
    libpd_set_concatenated_printhook(&Pd4Web::post);

    int ret = libpd_init();
    if (ret) {
        _JS_alert("libpd_init() failed, please report!");
        return -1;
    }

    Pd4WebInitExternals();

    libpd_add_to_search_path("./Libs/");
    libpd_add_to_search_path("./Extras/");
    libpd_add_to_search_path("./Audios/");
    if (!libpd_openfile("index.pd", "./")) {
        _JS_alert("Failed to open patch | Please Report!\n");
        return -1;
    }

    // Init Gui interface
    if (PD4WEB_GUI) {
        Pd4Web::initGuiInterface();
    }

    printf("pd4web version %s.%s.%s\n", PD4WEB_VERSION_MAJOR, PD4WEB_VERSION_MINOR,
           PD4WEB_VERSION_PATCH);
    _JS_setIcon("--sound-off", "pulse 1s infinite");

    // Need to fix this, the problem is that the lua position of the object is wrong;
    MAIN_THREAD_ASYNC_EM_ASM({ setTimeout(function() { Pd4Web._vis(); }, 100); });

    emscripten_set_main_loop(Pd4Web::guiLoop, 0, 1);

    return 0;
}
