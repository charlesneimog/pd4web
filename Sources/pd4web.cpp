#include "pd4web.hpp"
#include <cstdio>

std::atomic<bool> Pd4WebMainLoopRunning(false);
std::atomic<bool> Pd4WebSavingData(false);
Pd4WebGuiReceiverList Pd4WebGuiReceivers;

// ╭─────────────────────────────────────╮
// │        JavaScript Functions         │
// ╰─────────────────────────────────────╯
// Functions written in JavaScript Language, this are used for the WebAudio API.
// Then we don't need to pass the WebAudio Context as in version 1.0.
// clang-format off
// ─────────────────────────────────────
EM_JS(void, _JS_sendList, (void), {
    if (typeof Pd4Web.GuiReceivers === "undefined") {
        Pd4Web.GuiReceivers = {}; // defined in pd4web.cpp Pd4WebJsHelpers
    }

    Pd4Web.sendList = function (r, vec) {
        const vecLength = vec.length;
        var ok = Pd4Web._startMessage(vecLength);
        if (!ok) {
            console.error('Failed to start message');
            return;
        }
        for (let i = 0; i < vecLength; i++) {
            if (typeof vec[i] === 'string') {
                Pd4Web._addSymbol(vec[i]);
            } else if (typeof vec[i] === 'number') {
                Pd4Web._addFloat(vec[i]);
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
});

// ─────────────────────────────────────
EM_JS(void, _JS_loadGui, (bool AutoTheming), {
    if (document.getElementById("pd4web-gui") != null){
        return;
    }

    var script = document.createElement('script');
    script.type = "text/javascript";
    script.src = "./pd4web.gui.js";
    script.id = "pd4web-gui";
    script.onload = function() {
        Pd4WebInitGui(AutoTheming); // defined in pd4web.gui.js
    };

    document.head.appendChild(script); 
});

// ─────────────────────────────────────
EM_JS(void, _JS_loadStyle, (void), {
    if (document.getElementById("pd4web-style") != null){
        console.log("GUI already loaded");
        return;
    }

    // Load the CSS file
    var link = document.createElement('link');
    link.rel = "stylesheet";
    link.type = "text/css";
    link.href = "./pd4web.style.css";
    link.id = "pd4web-style";
    document.head.appendChild(link);
});

// ─────────────────────────────────────
EM_JS(void, _JS_setTitle, (const char *projectName), {
    let title = UTF8ToString(projectName);
    document.title = title;
});

// ─────────────────────────────────────
EM_JS(void, _JS_alert, (const char *msg), {
    alert(UTF8ToString(msg));
});

// ─────────────────────────────────────
EM_JS(void, _JS_post, (const char *msg), {
    console.log(UTF8ToString(msg));
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
    
    var script = document.createElement('script');
    script.type = "text/javascript";
    script.src = "./pd4web.midi.js";
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
            if (std::holds_alternative<std::string>(GuiReceiver.List[i])) {
                return std::get<float>(GuiReceiver.List[i]);
            }
        }
    }
    return 0;
}

// ╭─────────────────────────────────────╮
// │     Extra PureData Definitions      │
// ╰─────────────────────────────────────╯
extern "C" void sys_putmidibyte(int portno, int byte) { return; }

// ╭─────────────────────────────────────╮
// │       Audio Worklet Receivers       │
// ╰─────────────────────────────────────╯
void Pd4Web::receivedBang(const char *r) {
    for (auto &GuiReceiver : Pd4WebGuiReceivers) {
        if (GuiReceiver.Receiver == r) {
            GuiReceiver.BeingUpdated = true;
            GuiReceiver.Updated = true;
            GuiReceiver.Type = Pd4WebGuiReceiver::BANG;
            GuiReceiver.BeingUpdated = false;
        }
    }
};

// ─────────────────────────────────────
void Pd4Web::receivedFloat(const char *r, float f) {
    for (auto &GuiReceiver : Pd4WebGuiReceivers) {
        if (GuiReceiver.Receiver == r) {
            GuiReceiver.BeingUpdated = true;
            GuiReceiver.Updated = true;
            GuiReceiver.Type = Pd4WebGuiReceiver::FLOAT;
            GuiReceiver.Float = f;
            GuiReceiver.BeingUpdated = false;
        }
    }
};

// ─────────────────────────────────────
void Pd4Web::receivedSymbol(const char *r, const char *s) {
    for (auto &GuiReceiver : Pd4WebGuiReceivers) {
        if (GuiReceiver.Receiver == r) {
            GuiReceiver.BeingUpdated = true;
            GuiReceiver.Updated = true;
            GuiReceiver.Type = Pd4WebGuiReceiver::SYMBOL;
            GuiReceiver.Symbol = s;
            GuiReceiver.BeingUpdated = false;
        }
    }
};

// ─────────────────────────────────────
void Pd4Web::receivedList(const char *r, int argc, t_atom *argv) {
    for (auto &GuiReceiver : Pd4WebGuiReceivers) {
        if (GuiReceiver.Receiver == r) {
            GuiReceiver.BeingUpdated = true;
            GuiReceiver.Updated = true;
            GuiReceiver.Type = Pd4WebGuiReceiver::LIST;
            GuiReceiver.BeingUpdated = false;
            GuiReceiver.List.clear();
            for (int i = 0; i < argc; i++) {
                t_atom *a = argv + i;
                if (a->a_type == A_FLOAT) {
                    GuiReceiver.List.push_back(libpd_get_float(a));
                } else if (a->a_type == A_SYMBOL) {
                    GuiReceiver.List.push_back(libpd_get_symbol(a));
                }
            }
        }
    }
};

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
bool Pd4Web::_startMessage(int argc) {
    if (libpd_start_message(argc)) {
        return false;
    }
    return true;
}

// ─────────────────────────────────────
void Pd4Web::_addFloat(float f) { libpd_add_float(f); }
void Pd4Web::_addSymbol(std::string s) { libpd_add_symbol(s.c_str()); }
void Pd4Web::_finishMessage(std::string s) { libpd_finish_list(s.c_str()); }
bool Pd4Web::sendBang(std::string r) {
    int ok = libpd_bang(r.c_str());
    if (!ok) {
        return false;
    }
    return true;
}

// ─────────────────────────────────────
bool Pd4Web::sendFloat(std::string r, float f) {
    int ok = libpd_float(r.c_str(), f);
    if (!ok) {
        return false;
    }
    return true;
}

// ─────────────────────────────────────
bool Pd4Web::sendSymbol(std::string r, std::string s) {
    int ok = libpd_symbol(r.c_str(), s.c_str());
    if (!ok) {
        return false;
    }
    return true;
}

// ─────────────────────────────────────
void Pd4Web::noteOn(int channel, int pitch, int velocity) {
    libpd_noteon(channel, pitch, velocity);
}

// ─────────────────────────────────────
void Pd4Web::bindReceiver(std::string s) {
    void *Receiver = libpd_bind(s.c_str());
    for (auto &GuiReceiver : Pd4WebGuiReceivers) {
        if (GuiReceiver.Receiver == s) {
            return;
        }
    }
    Pd4WebGuiReceiver GuiReceiver;
    GuiReceiver.Receiver = s;
    Pd4WebGuiReceivers.push_back(GuiReceiver);
    return;
}

// ─────────────────────────────────────
void Pd4Web::addGuiReceiver(std::string s) {
    m_Receivers.push_back(s);
    return;
}

// ─────────────────────────────────────
void Pd4Web::bindGuiReceivers() {
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

EM_BOOL Pd4Web::process(int numInputs, const AudioSampleFrame *In, int numOutputs,
                        AudioSampleFrame *Out, int numParams, const AudioParamFrame *params,
                        void *userData) {

    int ChCount = Out[0].numberOfChannels;
    float LibPdOuts[128 * ChCount];

    libpd_process_float(2, In[0].data, LibPdOuts);
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

    EMSCRIPTEN_AUDIO_WORKLET_NODE_T AudioWorkletNode = emscripten_create_wasm_audio_worklet_node(
        audioContext, "pd4web", &options, &Pd4Web::process, 0);

    Pd4WebInitExternals();

    libpd_add_to_search_path("./Libs/");
    libpd_add_to_search_path("./Extras/");
    libpd_add_to_search_path("./Audios/");

    // turn audio on
    libpd_start_message(1);
    libpd_add_float(1.0f);
    libpd_finish_message("pd", "dsp");
    libpd_init_audio(NInCh, NOutCh, SR);

    if (!libpd_openfile("index.pd", "./")) {
        _JS_alert("Failed to open patch | Please Report!\n");
        return;
    }
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

// ╭─────────────────────────────────────╮
// │            Init Function            │
// ╰─────────────────────────────────────╯
void Pd4Web::init() {
    uint32_t SR = PD4WEB_SR;
    float NInCh = PD4WEB_CHS_IN;
    float NOutCh = PD4WEB_CHS_OUT;

    EmscriptenWebAudioCreateAttributes attrs = {
        .latencyHint = "interactive",
        .sampleRate = SR,
    };

    libpd_set_printhook(libpd_print_concatenator);
    libpd_set_concatenated_printhook(&Pd4Web::post);

    int ret = libpd_init();
    if (ret == -2) {
        _JS_alert("libpd_init() failed, please report!");
        return;
    }

    libpd_set_banghook(&Pd4Web::receivedBang);
    libpd_set_floathook(&Pd4Web::receivedFloat);
    libpd_set_symbolhook(&Pd4Web::receivedSymbol);
    libpd_set_listhook(&Pd4Web::receivedList);
    // libpd_set_queued_messagehook(ReceiveMessage);

    EMSCRIPTEN_WEBAUDIO_T AudioContext = emscripten_create_audio_context(&attrs);
    emscripten_start_wasm_audio_worklet_thread_async(AudioContext, WasmAudioWorkletStack,
                                                     sizeof(WasmAudioWorkletStack),
                                                     Pd4Web::audioWorkletInit, 0);

    m_Context = AudioContext;
    _JS_sendList();
    _JS_onReceived();

    if (PD4WEB_MIDI) {
        _JS_loadMidi();
    }

    bindGuiReceivers();

    return;
}
// ╭─────────────────────────────────────╮
// │              Main Loop              │
// ╰─────────────────────────────────────╯
void Pd4Web::guiLoop() {
    for (auto &GuiReceiver : Pd4WebGuiReceivers) {
        if (GuiReceiver.Updated) {
            switch (GuiReceiver.Type) {
            case Pd4WebGuiReceiver::BANG: {
                _JS_receiveBang(GuiReceiver.Receiver.c_str());
                break;
            }
            case Pd4WebGuiReceiver::FLOAT: {
                _JS_receiveFloat(GuiReceiver.Receiver.c_str(), GuiReceiver.Float);
                break;
            }
            case Pd4WebGuiReceiver::SYMBOL: {
                _JS_receiveSymbol(GuiReceiver.Receiver.c_str(), GuiReceiver.Receiver.c_str());
                break;
            }
            case Pd4WebGuiReceiver::LIST: {
                _JS_receiveList(GuiReceiver.Receiver.c_str());
                break;
            }
            case Pd4WebGuiReceiver::MESSAGE: {
                printf("Unhandled message\n");
            }
            }
            GuiReceiver.Updated = false;
        }
    }
}

// ╭─────────────────────────────────────╮
// │            Main Function            │
// ╰─────────────────────────────────────╯
int main() {
    if (PD4WEB_GUI) {
        _JS_loadStyle();
        _JS_loadGui(PD4WEB_AUTO_THEME);
    }
    _JS_setTitle(PD4WEB_PROJECT_NAME);

    printf("pd4web version %d.%d.%d\n", PD4WEB_MAJOR_VERSION, PD4WEB_MINOR_VERSION,
           PD4WEB_MICRO_VERSION);
    emscripten_set_main_loop(Pd4Web::guiLoop, PD4WEB_FPS, 1);

    return 0;
}
