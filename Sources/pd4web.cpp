#include "pd4web.hpp"
#include <cstdio>
#include <sstream>

// ╭─────────────────────────────────────╮
// │      Internal Testes For Debug      │
// ╰─────────────────────────────────────╯
static void _HooksTests() {
    libpd_bind("bang_test");
    libpd_bind("float_test");
}
// ╭─────────────────────────────────────╮
// │        JavaScript Functions         │
// ╰─────────────────────────────────────╯
// Functions written in JavaScript Language, this are used for the WebAudio API.
// Then we don't need to pass the WebAudio Context as in version 1.0.
// clang-format off
// ─────────────────────────────────────
EM_JS(void, _Pd4WebJSFunctions, (void), {

        // sendList
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

        // Functions

});

// ─────────────────────────────────────
EM_JS(void, _Pd4WebInitGui, (void), {
    if (document.getElementById('pd4web-gui') != null){
        return;
    }

    var script = document.createElement('script');
    script.type = 'text/javascript';
    script.src = './gui.js';
    script.id = 'pd4web-gui';
    script.onload = function() {
        Pd4WebInitGui();
    };
    document.head.appendChild(script); 
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

    async function getMicAccess(stream) {
      try {
        if (nInCh > 0) {
            const sourceNode = Pd4WebAudioContext.createMediaStreamSource(stream);
            sourceNode.connect(Pd4WebAudioWorkletNode);
        }
        Pd4WebAudioWorkletNode.connect(Pd4WebAudioContext.destination);
      } catch (err) {
        alert(err);
      }
    }

    navigator.mediaDevices
        .getUserMedia({
            video: false,
            audio: {
                echoCancellation: false,
                noiseSuppression: false,
                autoGainControl: false,

            },
        })
        .then((stream) => getMicAccess(stream));
});

// ─────────────────────────────────────
EM_JS(void, JsSuspendAudioWorkLet, (EMSCRIPTEN_WEBAUDIO_T audioContext),{
    Pd4WebAudioContext = emscriptenGetAudioObject(audioContext);
    Pd4WebAudioContext.suspend();
});

// clang-format on

// ╭─────────────────────────────────────╮
// │              PureData               │
// ╰─────────────────────────────────────╯
int Pd4Web::GetNInputChannels() { return N_CH_IN; }
int Pd4Web::GetNOutputChannels() { return N_CH_OUT; }
uint32_t Pd4Web::GetSampleRate() { return SAMPLE_RATE; }

// ╭─────────────────────────────────────╮
// │            WebAudioPatch            │
// ╰─────────────────────────────────────╯
// This functions will be used to create the WebAudio Worklet Processor.

static EM_BOOL ProcessPdPatch(int numInputs, const AudioSampleFrame *inputs, int numOutputs,
                              AudioSampleFrame *outputs, int numParams,
                              const AudioParamFrame *params, void *userData) {

    int inCh = inputs[0].numberOfChannels;
    int outCh = outputs[0].numberOfChannels;
    float outChsArrays[128 * outCh];

    libpd_process_float(2, inputs[0].data, outChsArrays);

    int outputIndex = 0;

    for (int i = 0; i < outCh; i++) {
        for (int j = i; j < (128 * outCh); j += 2) {
            outputs[0].data[outputIndex] = outChsArrays[j];
            outputIndex++;
        }
    }

    return EM_TRUE;
}

// ─────────────────────────────────────
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

    EMSCRIPTEN_AUDIO_WORKLET_NODE_T AudioWorkletNode = emscripten_create_wasm_audio_worklet_node(
        audioContext, "pd4web", &options, &ProcessPdPatch, 0);

    GetMicAccess(audioContext, AudioWorkletNode, NInCh);
}

// ─────────────────────────────────────
static void WebAudioWorkletThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success,
                                             void *userData) {

    if (!success) {
        return;
    }

    WebAudioWorkletProcessorCreateOptions opts = {
        .name = "pd4web",
    };

    emscripten_create_wasm_audio_worklet_processor_async(audioContext, &opts,
                                                         &Pd4Web::AudioWorkletProcessorCreated, 0);
}

// ─────────────────────────────────────
void Pd4Web::ResumeAudio() { emscripten_resume_audio_context_sync(Context); }
void Pd4Web::SuspendAudio() { JsSuspendAudioWorkLet(Context); }

// ╭─────────────────────────────────────╮
// │           Receivers Hooks           │
// ╰─────────────────────────────────────╯
static void rPrint(const char *message) {
    if (message[0] == '\n') {
        return;
    }
    WebPost(message);
    return;
}

// ─────────────────────────────────────
static void ReceiveBang(const char *r) {
    // TODO:
    return;
}

// ─────────────────────────────────────
static void ReceiveFloat(const char *r, float f) {
    // TODO:
    return;
}

// ─────────────────────────────────────
static void ReceiveSymbol(const char *r, const char *s) {
    // TODO:
    return;
}

// ─────────────────────────────────────
static void ReceiveList(const char *r, int argc, t_atom *argv) {
    // TODO:
    return;
}

// ─────────────────────────────────────
static void ReceiveMessage(const char *r, const char *s, int argc, t_atom *argv) {
    // TODO:
    return;
}

// ╭─────────────────────────────────────╮
// │            Senders Hooks            │
// ╰─────────────────────────────────────╯
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
    // The Receiver must save the void *Receiver in a array with his name to
    // Unbind using the name.

    return;
}

// ─────────────────────────────────────
void Pd4Web::UnbindReceiver() {
    // TODO: Implement this function

    return;
}

// ─────────────────────────────────────
void Pd4Web::receiveMessage(const char *source, const char *symbol, int argc, t_atom *argv) {
    std::ostringstream ss;
    for (int i = 0; i < argc; ++i) {
        if (argv[i].a_type == A_FLOAT) {
            ss << argv[i].a_w.w_float;
        } else if (argv[i].a_type == A_SYMBOL) {
            ss << argv[i].a_w.w_symbol->s_name;
        }
        if (i != argc - 1) {
            ss << ',';
        }
    }
}

// ╭─────────────────────────────────────╮
// │            Init Function            │
// ╰─────────────────────────────────────╯
EMSCRIPTEN_KEEPALIVE void Pd4Web::Init() {
    uint32_t SR = GetSampleRate();
    float NInCh = GetNInputChannels();
    float NOutCh = GetNOutputChannels();

    if (pdInit) {
        ResumeAudio();
        return;
    }

    srand(time(NULL));
    assert(!emscripten_current_thread_is_audio_worklet());

    printf("pd4web version %d.%d.%d\n", PD4WEB_MAJOR_VERSION, PD4WEB_MINOR_VERSION,
           PD4WEB_MICRO_VERSION);

    EmscriptenWebAudioCreateAttributes attrs = {
        .latencyHint = "interactive",
        .sampleRate = SR,
    };

    EMSCRIPTEN_WEBAUDIO_T AudioContext = emscripten_create_audio_context(&attrs);

    emscripten_start_wasm_audio_worklet_thread_async(AudioContext, wasmAudioWorkletStack,
                                                     sizeof(wasmAudioWorkletStack),
                                                     WebAudioWorkletThreadInitialized, 0);

    t_libpd_printhook libpd_printhook = (t_libpd_printhook)libpd_print_concatenator;
    t_libpd_printhook libpd_concatenated_printhook = (t_libpd_printhook)rPrint;

    Context = AudioContext;

    libpd_set_printhook(rPrint);
    libpd_set_banghook(ReceiveBang);
    libpd_set_floathook(ReceiveFloat);
    libpd_set_symbolhook(ReceiveSymbol);
    libpd_set_listhook(ReceiveList);
    libpd_set_messagehook(ReceiveMessage);

    // TODO: Implement Midi
    // libpd_set_noteonhook(receiveNoteOn);
    // libpd_set_controlchangehook(receiveControlChange);
    // libpd_set_programchangehook(receiveProgramChange);
    // libpd_set_pitchbendhook(receivePitchBend);
    // libpd_set_aftertouchhook(receiveAftertouch);
    // libpd_set_polyaftertouchhook(receivePolyAftertouch);
    // libpd_set_midibytehook(receiveMidiByte);

    libpd_init();
    Pd4WebInitExternals();

    libpd_start_message(1);
    libpd_add_float(1.0f);
    libpd_finish_message("pd", "dsp");
    libpd_init_audio(NInCh, NOutCh, SR);

    if (!libpd_openfile("index.pd", "./")) {
        printf("Failed to open patch\n");
        return;
    }

#if PD4WEB_DEBUG
    _HooksTests();
#endif

    pdInit = true;
    ResumeAudio();
    return;
}

// ╭─────────────────────────────────────╮
// │            Main Function            │
// ╰─────────────────────────────────────╯
int main() {

    _Pd4WebInitGui();
    _Pd4WebJSFunctions();

    return 0;
}
