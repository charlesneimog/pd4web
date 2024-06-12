#include "pd4web.hpp"
#include <sstream>

// ╭─────────────────────────────────────╮
// │        JavaScript Functions         │
// ╰─────────────────────────────────────╯
// Functions written in JavaScript Language, this are used for the WebAudio API.
// Then we don't need to pass the WebAudio Context as in version 1.0.

// clang-format off
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
// │            WebAudioPatch            │
// ╰─────────────────────────────────────╯

// This functions will be used to create the WebAudio Worklet Processor.

static EM_BOOL ProcessPdPatch(int numInputs, const AudioSampleFrame *inputs,
                              int numOutputs, AudioSampleFrame *outputs,
                              int numParams, const AudioParamFrame *params,
                              void *userData) {

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
static void AudioWorkletProcessorCreated(EMSCRIPTEN_WEBAUDIO_T audioContext,
                                         EM_BOOL success, void *userData) {
    if (!success) {
        Alert("Failed to create AudioWorkletProcessor, please report!\n");
        return;
    }

    int nOutChannelsArray[1] = {N_CH_OUT};

    EmscriptenAudioWorkletNodeCreateOptions options = {
        .numberOfInputs = N_CH_IN,
        .numberOfOutputs = 1,
        .outputChannelCounts = nOutChannelsArray,
    };

    EMSCRIPTEN_AUDIO_WORKLET_NODE_T AudioWorkletNode =
        emscripten_create_wasm_audio_worklet_node(audioContext, "pd4web",
                                                  &options, &ProcessPdPatch, 0);

    GetMicAccess(audioContext, AudioWorkletNode, N_CH_IN);
}

// ─────────────────────────────────────
static void WebAudioWorkletThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext,
                                             EM_BOOL success, void *userData) {

    if (!success) {
        return;
    }

    WebAudioWorkletProcessorCreateOptions opts = {
        .name = "pd4web",
    };

    emscripten_create_wasm_audio_worklet_processor_async(
        audioContext, &opts, &AudioWorkletProcessorCreated, 0);
}

// ─────────────────────────────────────
void Pd4Web::ResumeAudio() { emscripten_resume_audio_context_sync(Context); }
void Pd4Web::SuspendAudio() { JsSuspendAudioWorkLet(Context); }

// ╭─────────────────────────────────────╮
// │             Libpd Hooks             │
// ╰─────────────────────────────────────╯
void Pd4Web::receivePrint(const char *s) {
    if (s[0] == '\n') {
        return;
    }
    WebPost(s);
    return;
}

// ─────────────────────────────────────
void Pd4Web::receiveMessage(const char *source, const char *symbol, int argc,
                            t_atom *argv) {
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

    if (pdInit) {
        ResumeAudio();
        return;
    }

    srand(time(NULL));
    assert(!emscripten_current_thread_is_audio_worklet());

    printf("pd4web version %d.%d.%d\n", PD4WEB_MAJOR_VERSION,
           PD4WEB_MINOR_VERSION, PD4WEB_MICRO_VERSION);

    EmscriptenWebAudioCreateAttributes attrs = {
        .latencyHint = "interactive",
        // .sampleRate = SAMPLE_RATE,
    };

    EMSCRIPTEN_WEBAUDIO_T AudioContext =
        emscripten_create_audio_context(&attrs);

    emscripten_start_wasm_audio_worklet_thread_async(
        AudioContext, wasmAudioWorkletStack, sizeof(wasmAudioWorkletStack),
        WebAudioWorkletThreadInitialized, 0);

    t_libpd_printhook libpd_printhook =
        (t_libpd_printhook)libpd_print_concatenator;
    t_libpd_printhook libpd_concatenated_printhook =
        (t_libpd_printhook)receivePrint;

    Context = AudioContext;

    libpd_set_printhook(receivePrint);

    // libpd_set_banghoo(receiveBang);
    // libpd_set_floathook(receiveFloat);
    // libpd_set_symbolhook(receiveSymbol);
    // libpd_set_listhook(receiveList);
    // libpd_set_messagehook(receiveMessage);
    // libpd_set_noteonhook(receiveNoteOn);
    // libpd_set_controlchangehook(receiveControlChange);
    // libpd_set_programchangehook(receiveProgramChange);
    // libpd_set_pitchbendhook(receivePitchBend);
    // libpd_set_aftertouchhook(receiveAftertouch);
    // libpd_set_polyaftertouchhook(receivePolyAftertouch);
    // libpd_set_midibytehook(receiveMidiByte);

    libpd_init();

    libpd_start_message(1);
    libpd_add_float(1.0f);
    libpd_finish_message("pd", "dsp");

    libpd_init_audio(N_CH_IN, N_CH_OUT, SAMPLE_RATE);

    if (!libpd_openfile("index.pd", "./")) {
        printf("Failed to open patch\n");
        return;
    }

    pdInit = true;
    ResumeAudio();
    return;
}
