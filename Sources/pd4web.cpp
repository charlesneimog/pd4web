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
    // Gui
    Pd4Web.GuiReceivers = {};


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

    // User functions for receiving messages
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

    // Symbol Received
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
});

// ─────────────────────────────────────
EM_JS(void, _JS_enableThreads, (void), {
    if (document.getElementById("pd4web.threads") != null){
        return;
    }
    var script = document.createElement('script');
    script.type = "text/javascript";
    script.src = "./pd4web.threads.js";
    script.id = "pd4web.threads";
    document.head.appendChild(script); 
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
        // TODO: Implement some function defined by user

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

//╭─────────────────────────────────────╮
//│       Audio Worklet Receivers       │
//╰─────────────────────────────────────╯
void Pd4Web::AW_ReceivedBang(const char *r) {
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
void Pd4Web::AW_ReceivedFloat(const char *r, float f) {
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
void Pd4Web::AW_ReceivedSymbol(const char *r, const char *s) {
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

// ╭─────────────────────────────────────╮
// │         Audio Configuration         │
// ╰─────────────────────────────────────╯
int Pd4Web::GetNInputChannels() { return PD4WEB_CHS_IN; }
int Pd4Web::GetNOutputChannels() { return PD4WEB_CHS_OUT; }
uint32_t Pd4Web::GetSampleRate() { return 48000; }

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
void Pd4Web::bindGuiReceiver(std::string s, std::string obj) {
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
    float TmpOuts[128 * ChCount];

    libpd_process_float(2, In[0].data, TmpOuts);

    int OutI = 0;
    for (int i = 0; i < ChCount; i++) {
        for (int j = i; j < (128 * ChCount); j += 2) {
            Out[0].data[OutI] = TmpOuts[j];
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
void Pd4Web::AudioWorkletProcessorCreated(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success,
                                          void *userData) {

    if (!success) {
        _JS_alert("Failed to create AudioWorkletProcessor, please report!\n");
        return;
    }

    uint32_t SR = GetSampleRate();
    int NInCh = GetNInputChannels();
    int NOutCh = GetNOutputChannels();

    int nOutChannelsArray[1] = {NOutCh};

    EmscriptenAudioWorkletNodeCreateOptions options = {
        .numberOfInputs = NInCh,
        .numberOfOutputs = 1,
        .outputChannelCounts = nOutChannelsArray,
    };

    // Init Instance

    EMSCRIPTEN_AUDIO_WORKLET_NODE_T AudioWorkletNode = emscripten_create_wasm_audio_worklet_node(
        audioContext, "pd4web", &options, &process, 0);

    if (PD4WEB_GUI) {
        _JS_loadGui(PD4WEB_AUTO_THEME);
    }

    Pd4WebInitExternals();

    libpd_start_message(1);
    libpd_add_float(1.0f);
    libpd_finish_message("pd", "dsp");
    libpd_init_audio(NInCh, NOutCh, SR);
    libpd_add_to_search_path("Libs/");

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
void Pd4Web::audioWorkletInit(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success,
                                             void *userData) {
    if (!success) {
        _JS_alert("WebAudio worklet thread initialization failed!\n");
        return;
    }

    WebAudioWorkletProcessorCreateOptions opts = {
        .name = "pd4web",
    };

    emscripten_create_wasm_audio_worklet_processor_async(
        audioContext, &opts, &Pd4Web::AudioWorkletProcessorCreated, userData);
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
    uint32_t SR = GetSampleRate();
    float NInCh = GetNInputChannels();
    float NOutCh = GetNOutputChannels();

    EmscriptenWebAudioCreateAttributes attrs = {
        .latencyHint = "interactive",
        .sampleRate = SR,
    };

    libpd_set_printhook(Pd4Web::post); // <== Print

    int ret = libpd_init();
    if (ret == -2) {
        _JS_alert("libpd_queued_init() failed, please report!");
        return;
    }

    libpd_set_banghook(&Pd4Web::AW_ReceivedBang);
    libpd_set_floathook(&Pd4Web::AW_ReceivedFloat);
    libpd_set_symbolhook(&Pd4Web::AW_ReceivedSymbol);
    // libpd_set_queued_listhook(ReceiveList);
    // libpd_set_queued_messagehook(ReceiveMessage);

    EMSCRIPTEN_WEBAUDIO_T AudioContext = emscripten_create_audio_context(&attrs);
    emscripten_start_wasm_audio_worklet_thread_async(AudioContext, WasmAudioWorkletStack,
                                                     sizeof(WasmAudioWorkletStack),
                                                     Pd4Web::audioWorkletInit, 0);

    m_Context = AudioContext;
    _JS_sendList();
    _JS_loadMidi();

    return;
}
// ╭─────────────────────────────────────╮
// │              Main Loop              │
// ╰─────────────────────────────────────╯
void Pd4Web::mainLoop() {
    for (auto &GuiReceiver : Pd4WebGuiReceivers) {
        if (GuiReceiver.Updated) {
            switch (GuiReceiver.Type) {
                case Pd4WebGuiReceiver::BANG:
                    _JS_receiveBang(GuiReceiver.Receiver.c_str());
                    GuiReceiver.Updated = false;
                    break;
                case Pd4WebGuiReceiver::FLOAT:
                    _JS_receiveFloat(GuiReceiver.Receiver.c_str(), GuiReceiver.Float);
                    GuiReceiver.Updated = false;
                    break;
                case Pd4WebGuiReceiver::SYMBOL:
                    _JS_receiveSymbol(GuiReceiver.Receiver.c_str(), GuiReceiver.Receiver.c_str());
                    printf("Symbol received: %s\n", GuiReceiver.Symbol.c_str());
                    GuiReceiver.Updated = false;
                    break;
                case Pd4WebGuiReceiver::MESSAGE:
                    break;
                default:
                break;
            }
            GuiReceiver.Updated = false;
        }
    }
}

// ╭─────────────────────────────────────╮
// │            Main Function            │
// ╰─────────────────────────────────────╯
int main() {

    _JS_enableThreads(); // <== For Github Pages
    _JS_loadStyle();

    printf("pd4web version %d.%d.%d\n", PD4WEB_MAJOR_VERSION, PD4WEB_MINOR_VERSION,
           PD4WEB_MICRO_VERSION);
    emscripten_set_main_loop(Pd4Web::mainLoop, PD4WEB_FPS, 1);

    return 0;
}

