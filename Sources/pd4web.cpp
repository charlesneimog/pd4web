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
EM_JS(void, Pd4WebJSHelpers, (void), {
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
EM_JS(void, Pd4WebEnableThreads, (void), {
    if (document.getElementById("pd4web.threads") != null){
        console.log("Threads already loaded");
        return;
    }
    var script = document.createElement('script');
    script.type = "text/javascript";
    script.src = "./pd4web.threads.js";
    script.id = "pd4web.threads";
    document.head.appendChild(script); 
});

// ─────────────────────────────────────
EM_JS(void, Pd4WebLoadGui, (void), {
    if (document.getElementById("pd4web-gui") != null){
        console.log("GUI already loaded");
        return;
    }


    var script = document.createElement('script');
    script.type = "text/javascript";
    script.src = "./pd4web.gui.js";
    script.id = "pd4web-gui";
    script.onload = function() {
        Pd4WebInitGui(); // defined in pd4web.gui.js
    };

    document.head.appendChild(script); 
});

// ─────────────────────────────────────
EM_JS(void, Pd4WebLoadStyle, (void), {
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
EM_JS(void, Alert, (const char *msg), {
    alert(UTF8ToString(msg));
});

// ─────────────────────────────────────
EM_JS(void, WebPost, (const char *msg), {
    console.log(UTF8ToString(msg));
});
    
// ─────────────────────────────────────
EM_JS(void, GetMicAccess, (EMSCRIPTEN_WEBAUDIO_T audioContext, EMSCRIPTEN_AUDIO_WORKLET_NODE_T audioWorkletNode, int nInCh), {
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
EM_JS(void, JsSuspendAudioWorkLet, (EMSCRIPTEN_WEBAUDIO_T audioContext),{
    Pd4WebAudioContext = emscriptenGetAudioObject(audioContext);
    Pd4WebAudioContext.suspend();
});

//╭─────────────────────────────────────╮
//│         JS Receivers Hooks          │
//╰─────────────────────────────────────╯
EM_JS(void, JsReceiveBang, (const char *r),{
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
EM_JS(void, JsReceiveFloat, (const char *r, float f),{
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
        // TODO: Implement some function defined by user

    }
});

// ─────────────────────────────────────
EM_JS(void, JsReceiveSymbol, (const char *r, const char *s),{
    var source = UTF8ToString(r);
    if (source in Pd4Web.GuiReceivers) {
        for (const data of Pd4Web.GuiReceivers[source]) {
            switch (data.type) {
                case "bng":
                    GuiBngUpdateCircle(data);
                    break;
            }
        }
    } else{
        // TODO: Implement some function defined by user

    }
});

//╭─────────────────────────────────────╮
//│       Audio Worklet Receivers       │
//╰─────────────────────────────────────╯
void AW_ReceivedBang(const char *r) {
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
void AW_ReceivedFloat(const char *r, float f) {
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
void AW_ReceivedSymbol(const char *r, const char *s) {
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
static void Pd4WebPrint(const char *message) {
    if (message[0] == '\n') {
        return;
    }
    WebPost(message);
    return;
}

// ╭─────────────────────────────────────╮
// │            Senders Hooks            │
// ╰─────────────────────────────────────╯
bool Pd4Web::SendBang(std::string r) {
    int ok = libpd_bang(r.c_str());
    if (!ok) {
        return false;
    }
    return true;
}

// ─────────────────────────────────────
bool Pd4Web::SendFloat(std::string r, float f) {
    int ok = libpd_float(r.c_str(), f);
    if (!ok) {
        return false;
    }
    return true;
}

// ─────────────────────────────────────
bool Pd4Web::SendSymbol(std::string r, std::string s) {
    int ok = libpd_symbol(r.c_str(), s.c_str());
    if (!ok) {
        return false;
    }
    return true;
}

// ─────────────────────────────────────
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

// ─────────────────────────────────────
void Pd4Web::BindReceiver(std::string s) {
    void *Receiver = libpd_bind(s.c_str());

    // TODO:
    // The Receiver must save the void *Receiver in a array if we want unbind
    // Unbind using the name.

    return;
}

// ─────────────────────────────────────
void Pd4Web::BindGuiReceiver(std::string s, std::string obj) {
    void *Receiver = libpd_bind(s.c_str());

    // Check if Receiver already in Global List
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
void Pd4Web::UnbindReceiver() {
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

static EM_BOOL ProcessPdPatch(int numInputs, const AudioSampleFrame *In, int numOutputs,
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
        Alert("Failed to create AudioWorkletProcessor, please report!\n");
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
        audioContext, "pd4web", &options, &ProcessPdPatch, 0);

    if (PD4WEB_GUI) {
        Pd4WebLoadGui();
    }

    Pd4WebInitExternals();

    libpd_start_message(1);
    libpd_add_float(1.0f);
    libpd_finish_message("pd", "dsp");
    libpd_init_audio(NInCh, NOutCh, SR);
    libpd_add_to_search_path("Libs/");

    if (!libpd_openfile("index.pd", "./")) {
        Alert("Failed to open patch | Please Report!\n");
        return;
    }
    GetMicAccess(audioContext, AudioWorkletNode, NInCh);
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
static void WebAudioWorkletThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success,
                                             void *userData) {
    if (!success) {
        Alert("WebAudio worklet thread initialization failed!\n");
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
void Pd4Web::ResumeAudio() { emscripten_resume_audio_context_sync(m_Context); }

/**
 * Suspends the audio worklet using JavaScript.
 */
void Pd4Web::SuspendAudio() { JsSuspendAudioWorkLet(m_Context); }

// ╭─────────────────────────────────────╮
// │            Init Function            │
// ╰─────────────────────────────────────╯
void Pd4Web::Init() {
    uint32_t SR = GetSampleRate();
    float NInCh = GetNInputChannels();
    float NOutCh = GetNOutputChannels();

    EmscriptenWebAudioCreateAttributes attrs = {
        .latencyHint = "interactive",
        .sampleRate = SR,
    };

    libpd_set_printhook(Pd4WebPrint); // <== Print

    int ret = libpd_init();
    if (ret == -2) {
        Alert("libpd_queued_init() failed, please report!");
        return;
    }

    libpd_set_banghook(AW_ReceivedBang);
    libpd_set_floathook(AW_ReceivedFloat);
    libpd_set_queued_symbolhook(AW_ReceivedSymbol);
    // libpd_set_queued_listhook(ReceiveList);
    // libpd_set_queued_messagehook(ReceiveMessage);

    EMSCRIPTEN_WEBAUDIO_T AudioContext = emscripten_create_audio_context(&attrs);
    emscripten_start_wasm_audio_worklet_thread_async(AudioContext, WasmAudioWorkletStack,
                                                     sizeof(WasmAudioWorkletStack),
                                                     WebAudioWorkletThreadInitialized, 0);

    m_Context = AudioContext;
    Pd4WebJSHelpers();
    return;
}
// ╭─────────────────────────────────────╮
// │              Main Loop              │
// ╰─────────────────────────────────────╯
static void Pd4WebMainLoop() {
    for (auto &GuiReceiver : Pd4WebGuiReceivers) {
        if (GuiReceiver.Updated) {
            switch (GuiReceiver.Type) {
                case Pd4WebGuiReceiver::BANG:
                    JsReceiveBang(GuiReceiver.Receiver.c_str());
                    GuiReceiver.Updated = false;
                    break;
                case Pd4WebGuiReceiver::FLOAT:
                    JsReceiveFloat(GuiReceiver.Receiver.c_str(), GuiReceiver.Float);
                    GuiReceiver.Updated = false;
                    break;
                case Pd4WebGuiReceiver::SYMBOL:
                    JsReceiveSymbol(GuiReceiver.Receiver.c_str(), GuiReceiver.Receiver.c_str());
                    GuiReceiver.Updated = false;
                    break;
                case Pd4WebGuiReceiver::MESSAGE:
                    break;
                default:
                // Handle unexpected value
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
    printf("pd4web version %d.%d.%d\n", PD4WEB_MAJOR_VERSION, PD4WEB_MINOR_VERSION,
           PD4WEB_MICRO_VERSION);

    Pd4WebEnableThreads(); // <== For Github Pages
    Pd4WebLoadStyle();
    emscripten_set_main_loop(Pd4WebMainLoop, PD4WEB_FPS, 1);

    return 0;
}

