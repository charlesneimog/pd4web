#include "pd4web.hpp"

// ╭─────────────────────────────────────╮
// │        JavaScript Functions         │
// ╰─────────────────────────────────────╯
// Functions written in JavaScript Language, this are used for the WebAudio API.
// Then we don't need to pass the WebAudio Context as in version 1.0.
// clang-format off
EM_JS(bool, JS_OpenKeyboard, (const char *key), {
      // TODO:
});

// ─────────────────────────────────────
EM_JS(int, JS_IsDarkMode, (), {
  return window.matchMedia &&
         window.matchMedia('(prefers-color-scheme: dark)').matches ? 1 : 0;
});

// ─────────────────────────────────────
EM_JS(void, JS_Alert, (const char *msg), {
    alert(UTF8ToString(msg));
});
    
// ─────────────────────────────────────
EM_JS(void, JS_GetMicAccess, (EMSCRIPTEN_WEBAUDIO_T audioContext, EMSCRIPTEN_AUDIO_WORKLET_NODE_T audioWorkletNode, int nInCh), {
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
EM_JS(void, JS_SuspendAudioWorklet, (EMSCRIPTEN_WEBAUDIO_T audioContext),{
    Pd4WebAudioContext = emscriptenGetAudioObject(audioContext);
    Pd4WebAudioContext.suspend();
});

// clang-format on
// ╭─────────────────────────────────────╮
// │            Senders Hooks            │
// ╰─────────────────────────────────────╯
void SenderCallback(t_pd *obj, void *data) {
    std::unique_ptr<Pd4WebSender> sender(static_cast<Pd4WebSender *>(data));

    switch (sender->type) {
    case BANG:
        libpd_bang(sender->receiver);
        break;
    case FLOAT:
        libpd_float(sender->receiver, sender->f);
        break;
    case SYMBOL:
        libpd_symbol(sender->receiver, sender->m);
        break;
    case LIST: {
        size_t llen = sender->l["length"].as<size_t>();
        if (libpd_start_message(llen)) {
            JS_Alert("Failed to start message for sendList");
            return;
        }
        for (unsigned i = 0; i < llen; ++i) {
            emscripten::val v = sender->l[i];
            if (v.isNumber()) {
                libpd_add_float(v.as<double>());
            } else if (v.isString()) {
                libpd_add_symbol(v.as<std::string>().c_str());
            } else {
                std::cerr << "Unsupported type at index " << i << " for sendList\n";
            }
        }
        if (libpd_finish_list(sender->receiver)) {
            JS_Alert("Failed to send message for sendList");
        }
        break;
    }
    case MESSAGE: {
        const unsigned len = sender->l["length"].as<unsigned>();
        t_atom atoms[len];
        for (unsigned i = 0; i < len; ++i) {
            emscripten::val v = sender->l[i];
            if (v.isNumber()) {
                libpd_set_float(&atoms[i], v.as<double>());
            } else if (v.isString()) {
                libpd_set_symbol(&atoms[i], v.as<std::string>().c_str());
            } else {
                std::cerr << "Unsupported type at index " << i << " for sendMessage\n";
            }
        }
        libpd_message(sender->receiver, sender->sel, len, atoms);
        break;
    }
    default:
        JS_Alert("Memory corruption, please report");
        break;
    }
}

// ─────────────────────────────────────
void Pd4Web::SendBang(std::string s) {
    auto sender = std::make_unique<Pd4WebSender>();
    sender->type = BANG;
    t_pd *receiver = gensym(s.c_str())->s_thing;
    pd_queue_mess(m_PdInstance, receiver, sender.release(), SenderCallback);
}

// ─────────────────────────────────────
void Pd4Web::SendFloat(std::string s, float f) {
    auto sender = std::make_unique<Pd4WebSender>();
    sender->type = FLOAT;
    sender->f = f;
    t_pd *receiver = gensym(s.c_str())->s_thing;
    pd_queue_mess(m_PdInstance, receiver, sender.release(), SenderCallback);
}

// ─────────────────────────────────────
void Pd4Web::SendSymbol(std::string s, std::string thing) {
    auto sender = std::make_unique<Pd4WebSender>();
    sender->type = SYMBOL;
    sender->m = thing.c_str();
    t_pd *receiver = gensym(s.c_str())->s_thing;
    pd_queue_mess(m_PdInstance, receiver, sender.release(), SenderCallback);
}

// ─────────────────────────────────────
void Pd4Web::SendList(std::string s, emscripten::val a) {
    auto sender = std::make_unique<Pd4WebSender>();
    sender->type = LIST;
    sender->l = a;
    t_pd *receiver = gensym(s.c_str())->s_thing;
    pd_queue_mess(m_PdInstance, receiver, sender.release(), SenderCallback);
}

// ─────────────────────────────────────
void Pd4Web::SendMessage(std::string r, std::string s, emscripten::val a) {
    auto sender = std::make_unique<Pd4WebSender>();
    sender->type = MESSAGE;
    sender->l = a;
    sender->sel = s.c_str();
    t_pd *receiver = gensym(s.c_str())->s_thing;
    pd_queue_mess(m_PdInstance, receiver, sender.release(), SenderCallback);
}

// ─────────────────────────────────────
void Pd4Web::SendFile(emscripten::val jsArrayBuffer, std::string filename) {
    size_t length = jsArrayBuffer["byteLength"].as<size_t>();
    emscripten::val uint8Array = emscripten::val::global("Uint8Array").new_(jsArrayBuffer);
    std::vector<uint8_t> buffer(length);
    uint8Array.call<void>("set", emscripten::typed_memory_view(length, buffer.data()));
    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        emscripten_log(EM_LOG_ERROR, "Failed to open output file");
        return;
    }
    out.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
    out.close();
}

// ╭─────────────────────────────────────╮
// │           Receivers Hooks           │
// ╰─────────────────────────────────────╯
/**
 * Registers a callback to be invoked when a bang is received from Pure Data.
 *
 * This function binds a JavaScript callback (`func`) to a specific receiver symbol (`r`).
 * When a bang is sent to `r` in Pure Data, the JavaScript function is called.
 *
 * @param r     The receiver symbol in Pure Data to listen for bangs.
 * @param func  The JavaScript callback function to invoke on bang.
 */
void Pd4Web::OnBangReceived(std::string r, emscripten::val func) {
    libpd_set_instance(m_PdInstance);

    void *s = libpd_bind(r.c_str());
    m_BindSymbols.push_back(s);

    if (func.typeOf().as<std::string>() != "function") {
        fprintf(stderr, "Error: passed value is not a function\n");
        return;
    }

    int declaredArgCount = func["length"].as<int>();
    if (declaredArgCount != 1) {
        fprintf(stderr, "Callback for onBangReceived must have 1 arguments, but has %d\n",
                declaredArgCount);
        return;
    }
    BangReceiverListeners[m_PdInstance][r] = func;
}

// ─────────────────────────────────────
/**
 * Registers a callback to be invoked when a float is received from Pure Data.
 *
 * Binds a JavaScript callback (`func`) to a receiver symbol (`r`), which will be
 * triggered when a float message is sent to `r` from Pure Data.
 *
 * @param r     The receiver symbol in Pure Data to listen for floats.
 * @param func  The JavaScript callback function to invoke with the float value.
 */
void Pd4Web::OnFloatReceived(std::string r, emscripten::val func) {
    libpd_set_instance(m_PdInstance);

    void *s = libpd_bind(r.c_str());
    m_BindSymbols.push_back(s);

    if (func.typeOf().as<std::string>() != "function") {
        fprintf(stderr, "Error: passed value is not a function\n");
        return;
    }

    int declaredArgCount = func["length"].as<int>();
    if (declaredArgCount != 2) {
        fprintf(stderr, "Callback for onFloatReceived must have 2 arguments, but has %d\n",
                declaredArgCount);
        return;
    }
    FloatReceiverListeners[m_PdInstance][r] = func;
}

// ─────────────────────────────────────
/**
 * Registers a callback to be invoked when a symbol is received from Pure Data.
 *
 * Binds a JavaScript callback (`func`) to a receiver symbol (`r`), which will be
 * triggered when a symbol message is sent to `r` from Pure Data.
 *
 * @param r     The receiver symbol in Pure Data to listen for symbols.
 * @param func  The JavaScript callback function to invoke with the symbol string.
 */
void Pd4Web::OnSymbolReceived(std::string r, emscripten::val func) {
    libpd_set_instance(m_PdInstance);

    void *s = libpd_bind(r.c_str());
    m_BindSymbols.push_back(s);

    if (func.typeOf().as<std::string>() != "function") {
        fprintf(stderr, "Error: passed value is not a function\n");
        return;
    }

    int declaredArgCount = func["length"].as<int>();
    if (declaredArgCount != 2) {
        fprintf(stderr, "Callback for onSymbolReceived must have 2 arguments, but has %d\n",
                declaredArgCount);
        return;
    }
    SymbolReceiverListeners[m_PdInstance][r] = func;
}

// ─────────────────────────────────────
/**
 * Registers a callback to be invoked when a list is received from Pure Data.
 *
 * Binds a JavaScript callback (`func`) to a receiver symbol (`r`), which will be
 * triggered when a list message is sent to `r` from Pure Data.
 *
 * @param r     The receiver symbol in Pure Data to listen for lists.
 * @param func  The JavaScript callback function to invoke with the list elements.
 */
void Pd4Web::OnListReceived(std::string r, emscripten::val func) {
    libpd_set_instance(m_PdInstance);

    void *s = libpd_bind(r.c_str());
    m_BindSymbols.push_back(s);

    if (func.typeOf().as<std::string>() != "function") {
        fprintf(stderr, "Error: passed value is not a function\n");
        return;
    }

    int declaredArgCount = func["length"].as<int>();
    if (declaredArgCount != 2) {
        fprintf(stderr, "Callback for onListReceived must have 2 arguments, but has %d\n",
                declaredArgCount);
        return;
    }
    ListReceiverListeners[m_PdInstance][r] = func;
}

// ─────────────────────────────────────
/**
 * Called when a print is received from Pure Data.
 *
 * @param msg The print message.
 */
void ReceivedPrintMsg(const char *msg) {
    if (msg[0] == '\n') {
        return;
    }
    emscripten_log(EM_LOG_CONSOLE, msg);
}

// ─────────────────────────────────────
/**
 * Called when a bang message is received from Pure Data.
 *
 * Looks up the registered JavaScript callback associated with the receiver symbol `r`
 * for the current Pure Data instance. If a valid function is found, it is invoked
 * with the receiver symbol as an argument.
 *
 * If no callback is found or the callback is not a function, an error message is logged.
 *
 * @param r  The receiver symbol of the bang message.
 */
void ReceivedBang(const char *r) {
    t_pdinstance *instance = libpd_this_instance();
    auto it = BangReceiverListeners.find(instance);
    if (it != BangReceiverListeners.end()) {
        auto funcMap = it->second;
        auto funcIt = funcMap.find(std::string(r));
        if (funcIt != funcMap.end()) {
            emscripten::val func = funcIt->second;
            if (func.typeOf().as<std::string>() == "function") {
                func(emscripten::val(r));
                return;
            }
        }
    }
    fprintf(stderr, "Callback not found or not a function\n");
}

// ─────────────────────────────────────
/**
 * Called when a float message is received from Pure Data.
 *
 * Looks up the registered JavaScript callback associated with the receiver symbol `r`
 * for the current Pure Data instance. If a valid function is found, it is invoked
 * with the float value as an argument.
 *
 * If no callback is found or the callback is not a function, an error message is logged.
 *
 * @param r  The receiver symbol of the float message.
 * @param f  The float value received.
 */
void ReceivedFloat(const char *r, float f) {
    t_pdinstance *instance = libpd_this_instance();
    auto it = FloatReceiverListeners.find(instance);
    if (it != FloatReceiverListeners.end()) {
        auto funcMap = it->second;
        auto funcIt = funcMap.find(std::string(r));
        if (funcIt != funcMap.end()) {
            emscripten::val func = funcIt->second;
            if (func.typeOf().as<std::string>() == "function") {
                func(emscripten::val(r), emscripten::val(f)); // Pass both r and f
                return;
            }
        }
    }
    fprintf(stderr, "Callback not found or not a function\n");
}

// ─────────────────────────────────────
/**
 * Called when a symbol message is received from Pure Data.
 *
 * Looks up the registered JavaScript callback associated with the receiver symbol `r`
 * for the current Pure Data instance. If a valid function is found, it is invoked
 * with the symbol string as an argument.
 *
 * If no callback is found or the callback is not a function, an error message is logged.
 *
 * @param r  The receiver symbol of the message.
 * @param s  The symbol string received.
 */
void ReceivedSymbol(const char *r, const char *s) {
    t_pdinstance *instance = libpd_this_instance();
    auto it = SymbolReceiverListeners.find(instance);
    if (it != SymbolReceiverListeners.end()) {
        auto funcMap = it->second;
        auto funcIt = funcMap.find(std::string(r));
        if (funcIt != funcMap.end()) {
            emscripten::val func = funcIt->second;
            if (func.typeOf().as<std::string>() == "function") {
                func(emscripten::val(r), emscripten::val(s)); // Pass both r and f
                return;
            }
        }
    }
    fprintf(stderr, "Callback not found or not a function\n");
}

// ─────────────────────────────────────
/**
 * Called when a list message is received from Pure Data.
 *
 * Looks up the registered JavaScript callback associated with the receiver symbol `r`
 * for the current Pure Data instance. If a valid function is found, it is invoked
 * with the list elements as arguments.
 *
 * If no callback is found or the callback is not a function, an error message is logged.
 *
 * @param r     The receiver symbol of the list message.
 * @param argc  The number of atoms in the list.
 * @param argv  Pointer to the array of atoms representing the list elements.
 */
void ReceivedList(const char *r, int argc, t_atom *argv) {
    // BUG: Command is repeated 4 times
    t_pdinstance *instance = libpd_this_instance();
    auto it = ListReceiverListeners.find(instance);
    if (it != ListReceiverListeners.end()) {
        auto funcMap = it->second;
        auto funcIt = funcMap.find(std::string(r));
        if (funcIt != funcMap.end()) {
            emscripten::val func = funcIt->second;
            if (func.typeOf().as<std::string>() == "function") {
                emscripten::val jsArray = emscripten::val::array();
                for (int i = 0; i < argc; ++i) {
                    if (libpd_is_float(&argv[i])) {
                        jsArray.call<void>("push", emscripten::val(argv[i].a_w.w_float));
                    } else if (libpd_is_symbol(&argv[i])) {
                        jsArray.call<void>("push", emscripten::val(argv[i].a_w.w_symbol->s_name));
                    } else {
                        fprintf(stderr, "only float and symbol supported\n");
                    }
                }
                func(emscripten::val(r), jsArray);
                return;
            }
        }
    }
    fprintf(stderr, "Callback not found or not a function\n");
}

// ─────────────────────────────────────
void ReceivedMessage(const char *r, const char *s, int argc, t_atom *argv) {
    // t_pdinstance *instance = libpd_this_instance();
    // emscripten::val func = ReceiverListeners[instance][std::string(r)];
    JS_Alert("ReceivedMessage not implemented");
}

// ─────────────────────────────────────
void ReceivedNoteON(int channel, int pitch, int velocity) {
    // typedef void (*t_libpd_noteonhook)(int channel, int pitch, int velocity)
}

// ─────────────────────────────────────
void ReceivedControlChannel(int channel, int controller, int value) {
    // typedef void (*t_libpd_controlchangehook)(int channel, int controller, int value)
}

// ─────────────────────────────────────
void ReceivedProgramChange(int channel, int program) {
    // typedef void (*t_libpd_programchangehook)(int channel, int program)
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
EM_BOOL Process(int numInputs, const AudioSampleFrame *In, int numOutputs, AudioSampleFrame *Out,
                int numParams, const AudioParamFrame *params, void *userData) {

    auto *ud = static_cast<Pd4WebUserData *>(userData);
    libpd_set_instance(ud->libpd);

    int ChCount = Out[0].numberOfChannels;
    float LibPdOuts[128 * ChCount];
    libpd_process_float(2, In[0].data, LibPdOuts);
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
void AudioWorkletProcessorCreated(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success,
                                  void *userData) {

    auto *ud = static_cast<Pd4WebUserData *>(userData);
    if (!success) {
        JS_Alert("Failed to create AudioWorkletProcessor, please report!\n");
        return;
    }

    uint32_t SR = ud->pd4web->GetSampleRate();
    int NInCh = ud->pd4web->GetChannelCountIn();
    int NOutCh = ud->pd4web->GetChannelCountOut();
    int nOutChannelsArray[1] = {NOutCh};

    EmscriptenAudioWorkletNodeCreateOptions options = {
        .numberOfInputs = NInCh,
        .numberOfOutputs = 1,
        .outputChannelCounts = nOutChannelsArray,
    };

    libpd_set_instance(ud->libpd);

    // turn audio on
    libpd_start_message(1);
    libpd_add_float(1.0f);
    libpd_finish_message("pd", "dsp");
    libpd_init_audio(NInCh, NOutCh, SR);

    std::string id = "pd4web_" + std::to_string(libpd_num_instances());
    EMSCRIPTEN_AUDIO_WORKLET_NODE_T AudioWorkletNode = emscripten_create_wasm_audio_worklet_node(
        audioContext, id.c_str(), &options, Process, userData);

    JS_GetMicAccess(audioContext, AudioWorkletNode, NInCh);
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
void AudioWorkletInit(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData) {
    auto *ud = static_cast<Pd4WebUserData *>(userData);
    if (!success) {
        JS_Alert("WebAudio worklet thread initialization failed!\n");
        return;
    }

    std::string id = "pd4web_" + std::to_string(libpd_num_instances());
    WebAudioWorkletProcessorCreateOptions opts = {
        .name = id.c_str(),
    };

    libpd_set_instance(ud->libpd);
    emscripten_create_wasm_audio_worklet_processor_async(audioContext, &opts,
                                                         AudioWorkletProcessorCreated, userData);
}

// ─────────────────────────────────────
/**
 * Toggle Audio
 */
void Pd4Web::ToggleAudio() {
    if (!m_Pd4WebAudioWorkletInit) {
        if (m_UseMidi) {
            SetupMIDI();
        }
        Init();
        m_Pd4WebAudioWorkletInit = true;
        m_AudioSuspended = false;
    } else {
        if (m_AudioSuspended) {
            emscripten_resume_audio_context_sync(GetWebAudioContext());
        } else {
            JS_SuspendAudioWorklet(GetWebAudioContext());
        }
    }
}

// ╭─────────────────────────────────────╮
// │          Midi Input/Output          │
// ╰─────────────────────────────────────╯
void OnMIDIInMessage(emscripten::val message) {
    emscripten::val data = message["data"];
    int byte1 = data[0].as<int>();
    int byte2 = data[1].as<int>();
    int byte3 = data[2].as<int>();

    int channel = byte1 & 0x0F;
    uint8_t status = byte1 & 0xF0;

    switch (status) {
    case 0x80:                           // Note Off
        libpd_noteon(channel, byte2, 0); // velocity 0 means note off in Pd
        break;

    case 0x90: // Note On
        libpd_noteon(channel, byte2, byte3);
        break;

    case 0xA0: // Polyphonic Aftertouch
        libpd_polyaftertouch(channel, byte2, byte3);
        break;

    case 0xB0: // Control Change
        libpd_controlchange(channel, byte2, byte3);
        break;

    case 0xC0: // Program Change
        libpd_programchange(channel, byte2);
        break;

    case 0xD0: // Channel Aftertouch
        libpd_aftertouch(channel, byte2);
        break;

    case 0xE0: // Pitch Bend
    {
        int value = (byte3 << 7) | byte2; // 14-bit value, MSB byte3, LSB byte2
        libpd_pitchbend(channel, value);
    } break;

    default:
        // System messages, SysEx, or unsupported status
        if (byte1 >= 0xF8) {
            libpd_sysrealtime(0, byte1);
        } else if (byte1 == 0xF0) {
            // You likely need to handle SysEx as a stream of bytes,
            // so this function may not fit SysEx processing properly.
            libpd_sysex(0, byte1);
            libpd_sysex(0, byte2);
            libpd_sysex(0, byte3);
        } else {
            libpd_midibyte(0, byte1);
            libpd_midibyte(0, byte2);
            libpd_midibyte(0, byte3);
        }
        break;
    }
}

// ─────────────────────────────────────
void OnMIDIOutMessage(emscripten::val message) {
    JS_Alert("MIDI not implemented yet");
}

// ─────────────────────────────────────
void OnMIDISuccess(emscripten::val midiAccess) {
    emscripten::val inputs = midiAccess["inputs"];
    emscripten::val iter = inputs.call<emscripten::val>("values");
    while (true) {
        emscripten::val next = iter.call<emscripten::val>("next");
        if (next["done"].as<bool>()) {
            break;
        }
        emscripten::val input = next["value"];
        input.set("onmidimessage", emscripten::val::module_property("_onMIDIMessage"));
    }

    emscripten::val outputs = midiAccess["outputs"];
    iter = outputs.call<emscripten::val>("values");

    while (true) {
        emscripten::val next = iter.call<emscripten::val>("next");
        if (next["done"].as<bool>()) {
            break;
        }

        emscripten::val output = next["value"];

        std::string name = output["name"].as<std::string>();
        std::string manufacturer = output["manufacturer"].as<std::string>();
        std::string id = output["id"].as<std::string>();

        std::cout << "MIDI Output: " << name << " | Manufacturer: " << manufacturer
                  << " | ID: " << id << std::endl;
    }
}

// ─────────────────────────────────────
void OnMIDIFailed(emscripten::val error) {
    JS_Alert("Access to MIDI devices failed.");
}

// ─────────────────────────────────────
void SetupMIDI() {
    emscripten::val navigator = emscripten::val::global("navigator");
    if (navigator["requestMIDIAccess"].typeOf().as<std::string>() != "function") {
        JS_Alert("Web MIDI API is not supported.");
        return;
    }
    emscripten::val promise = navigator.call<emscripten::val>("requestMIDIAccess");
    promise.call<emscripten::val>("then", // execute promise
                                  emscripten::val::module_property("_onMIDISuccess"),
                                  emscripten::val::module_property("_onMIDIFailed"));
}

// ╭─────────────────────────────────────╮
// │           Getter & Setter           │
// ╰─────────────────────────────────────╯
void Pd4Web::SetLastMousePosition(int x, int y) {
    m_LastMouseX = x;
    m_LastMouseY = y;
}

// ─────────────────────────────────────
void Pd4Web::GetLastMousePosition(int *x, int *y) {
    *x = m_LastMouseX;
    *y = m_LastMouseY;
}

// ─────────────────────────────────────
void Pd4Web::SetWebAudioContext(EMSCRIPTEN_WEBAUDIO_T ctx) {
    m_AudioContext = ctx;
}

// ─────────────────────────────────────
EMSCRIPTEN_WEBAUDIO_T Pd4Web::GetWebAudioContext() {
    return m_AudioContext;
}

// ─────────────────────────────────────
void Pd4Web::SetSampleRate(float sr) {
    m_SampleRate = sr;
}

// ─────────────────────────────────────
float Pd4Web::GetSampleRate() {
    return m_SampleRate;
}

// ─────────────────────────────────────
std::string Pd4Web::GetPatchPath() {
    return m_PatchPath;
}

// ─────────────────────────────────────
std::string Pd4Web::GetCanvasId() {
    return m_CanvasId;
}

// ─────────────────────────────────────
std::string Pd4Web::GetSoundToggleId() {
    return m_SoundToggleId;
}

// ─────────────────────────────────────
std::string Pd4Web::GetProjectName() {
    return m_ProjectName;
}

// ─────────────────────────────────────
int Pd4Web::GetChannelCountIn() {
    return m_ChannelCountIn;
}

// ─────────────────────────────────────
int Pd4Web::GetChannelCountOut() {
    return m_ChannelCountOut;
}

// ─────────────────────────────────────
float Pd4Web::GetPatchZoom() {
    return m_PatchZoom;
}

// ─────────────────────────────────────
bool Pd4Web::RenderGui() {
    return m_RenderGui;
}

// ─────────────────────────────────────
bool Pd4Web::UseMidi() {
    return m_UseMidi;
}

// ─────────────────────────────────────
int Pd4Web::GetFps() {
    return m_Fps;
}

// ─────────────────────────────────────
std::string Pd4Web::GetBGColor() {
    return m_BgColor;
}

// ─────────────────────────────────────
std::string Pd4Web::GetFGColor() {
    return m_FgColor;
}

// ╭─────────────────────────────────────╮
// │             USER EVENTS             │
// ╰─────────────────────────────────────╯
void QueueMouseClick(t_pd *obj, void *data) {
    Pd4WebUserData *ud = static_cast<Pd4WebUserData *>(data);
    libpd_set_instance(ud->libpd);

    t_canvas *canvas = pd_getcanvaslist();
    if (!canvas) {
        fprintf(stderr, "No pd canvas found\n");
        return;
    }

    for (t_gobj *obj = canvas->gl_list; obj != NULL; obj = obj->g_next) {
        int x1, y1, x2, y2;
        if (canvas_hitbox(canvas, obj, ud->xpos, ud->ypos, &x1, &y1, &x2, &y2, 0)) {
            (void)gobj_click(obj, canvas, ud->xpos, ud->ypos, 0, 0, 0, ud->doit);
        }
    }
    return;
}

// ─────────────────────────────────────
/**
 * Queues a keydown event to be processed by Lua within the Pd4Web environment on Audio Worklet
 Thread. On Pd4Web, when using pdlua objects is possible to define a method key_down,
 nbx:key_down(x, y, key), this function is called here for all lua objects when there is a keydown
 event.
 *
 * @param obj       Pointer to the Pure Data object triggering the event.
 * @param userData  Pointer to Pd4WebUserData containing the key string and libpd instance.
 */
void QueueKeyDown(t_pd *obj, void *userData) {
    Pd4WebUserData *ud = static_cast<Pd4WebUserData *>(userData);
    libpd_set_instance(ud->libpd);

    lua_State *L = __L();
    lua_getglobal(L, "pd");
    lua_getfield(L, -1, "_objects");
    lua_remove(L, -2);
    lua_pushnil(L);

    int count = 0;
    while (lua_next(L, -2) != 0) {
        count++;
        if (lua_istable(L, -1)) {
            lua_getfield(L, -1, "key_down");
            if (lua_isfunction(L, -1)) {
                lua_pushvalue(L, -2);
                lua_pushinteger(L, 20);
                lua_pushinteger(L, 20);
                lua_pushstring(L, ud->key.c_str());
                if (lua_pcall(L, 4, 0, 0) != LUA_OK) {
                    const char *err = lua_tostring(L, -1);
                    emscripten_log(EM_LOG_ERROR, "Erro when calling key_down: %s", err);
                    lua_pop(L, 1);
                }
            } else {
                lua_pop(L, 1);
            }
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
}

// ─────────────────────────────────────
/**
 * Keyboard event listener callback for Emscripten environment.
 *
 * Processes keyboard events and updates Pd4Web state accordingly inside Audio Worklet thread.
 *
 * @param eventType  The type of keyboard event (e.g., keydown, keyup).
 * @param e          Pointer to the EmscriptenKeyboardEvent containing event details.
 * @param userData   Pointer to user-defined data (typically Pd4WebUserData).
 * @return           Returns EM_TRUE if the event was handled, otherwise EM_FALSE.
 */
EM_BOOL KeyListener(int eventType, const EmscriptenKeyboardEvent *e, void *userData) {
    Pd4WebUserData *ud = (Pd4WebUserData *)userData;
    libpd_set_instance(ud->libpd);

    t_canvas *canvas = pd_getcanvaslist();
    if (!canvas) {
        fprintf(stderr, "No pd canvas found\n");
        return EM_FALSE;
    }

    ud->pd4web->GetLastMousePosition(&ud->xpos, &ud->ypos);
    ud->canvas = canvas;
    ud->doit = false;
    ud->key = e->key;

    t_class *obj = canvas->gl_list->g_pd->c_next;
    int x1, y1, x2, y2;
    ud->doit = ud->doit;
    ud->mousedown = ud->mousedown;
    ud->libpd = ud->libpd;
    pd_queue_mess(ud->libpd, &obj, ud, QueueKeyDown);

    return EM_TRUE;
}

// ─────────────────────────────────────
/**
 * Touch event listener callback for Emscripten environment.
 *
 * Handles touch events and updates Pd4Web state accordingly.
 * Intended to be registered with Emscripten's touch event system.
 *
 * @param eventType  The type of touch event (e.g., touchstart, touchend).
 * @param e          Pointer to the EmscriptenTouchEvent containing event details.
 * @param userData   Pointer to user-defined data (typically Pd4WebUserData).
 * @return           Returns EM_TRUE if the event was handled, otherwise EM_FALSE.
 */
EM_BOOL TouchListener(int eventType, const EmscriptenTouchEvent *e, void *userData) {
    Pd4WebUserData *ud = (Pd4WebUserData *)userData;
    libpd_set_instance(ud->libpd);
    t_canvas *canvas = pd_getcanvaslist();
    if (!canvas) {
        fprintf(stderr, "No pd canvas found\n");
        return EM_TRUE;
    }

    if (e->numTouches < 1) {
        return EM_TRUE;
    }

    int xpos = round((e->touches[0].targetX / ud->pd4web->GetPatchZoom()) + canvas->gl_xmargin);
    int ypos = round((e->touches[0].targetY / ud->pd4web->GetPatchZoom()) + canvas->gl_ymargin);
    ud->xpos = xpos;
    ud->ypos = ypos;
    ud->canvas = canvas;
    ud->doit = false;

    switch (eventType) {
    case EMSCRIPTEN_EVENT_TOUCHSTART:
        ud->mousedown = true;
        ud->doit = true;
        ud->pd4web->SetLastMousePosition(xpos, ypos);
        break;

    case EMSCRIPTEN_EVENT_TOUCHEND:
    case EMSCRIPTEN_EVENT_TOUCHCANCEL:
        ud->mousedown = false;
        ud->doit = false;
        break;

    case EMSCRIPTEN_EVENT_TOUCHMOVE:
        ud->doit = ud->mousedown;
        break;
    }

    t_class *obj = canvas->gl_list->g_pd->c_next;
    int x1, y1, x2, y2;
    ud->canvas = canvas;
    ud->xpos = xpos;
    ud->ypos = ypos;
    ud->doit = ud->doit;
    ud->mousedown = ud->mousedown;
    ud->libpd = ud->libpd;
    pd_queue_mess(ud->libpd, &obj, ud, QueueMouseClick);

    return EM_FALSE;
}

// ─────────────────────────────────────
/**
 * Mouse event listener callback for Emscripten environment.
 *
 * Handles mouse events and updates Pd4Web state accordingly.
 * Intended to be registered with Emscripten's mouse event system.
 *
 * @param eventType  The type of mouse event (e.g., click, mousemove).
 * @param e          Pointer to the EmscriptenMouseEvent containing event details.
 * @param userData   Pointer to user-defined data (typically Pd4WebUserData).
 * @return           Returns EM_TRUE if the event was handled, otherwise EM_FALSE.
 */
EM_BOOL MouseListener(int eventType, const EmscriptenMouseEvent *e, void *userData) {
    Pd4WebUserData *ud = (Pd4WebUserData *)userData;
    libpd_set_instance(ud->libpd);

    t_canvas *canvas = pd_getcanvaslist();
    if (!canvas) {
        fprintf(stderr, "Pd canvas is not valid for mouse event\n");
        return EM_TRUE;
    }

    int xpos = round((e->targetX / ud->pd4web->GetPatchZoom()) + canvas->gl_xmargin);
    int ypos = round((e->targetY / ud->pd4web->GetPatchZoom()) + canvas->gl_ymargin);

    ud->xpos = xpos;
    ud->ypos = ypos;
    ud->canvas = canvas;

    switch (eventType) {
    case EMSCRIPTEN_EVENT_MOUSEDOWN:
        ud->mousedown = true;
        ud->doit = true;
        break;

    case EMSCRIPTEN_EVENT_MOUSEUP:
        ud->mousedown = false;
        ud->doit = false;
        ud->pd4web->SetLastMousePosition(xpos, ypos);
        break;

    case EMSCRIPTEN_EVENT_MOUSEMOVE:
        ud->doit = ud->mousedown; // Always Process mousemove events
        break;
    }

    t_class *obj = canvas->gl_list->g_pd->c_next;
    int x1, y1, x2, y2;
    ud->canvas = canvas;
    ud->xpos = xpos;
    ud->ypos = ypos;
    ud->doit = ud->doit;
    ud->mousedown = ud->mousedown;
    ud->libpd = ud->libpd;
    pd_queue_mess(ud->libpd, &obj, ud, QueueMouseClick);


    return EM_FALSE;
}

// ─────────────────────────────────────
/**
 * Mouse event callback to toggle sound state in Pd4Web (sound icon).
 *
 * Listens for mouse events and toggles the sound output on or off.
 *
 * @param eventType  The type of mouse event triggering the toggle.
 * @param e          Pointer to the EmscriptenMouseEvent containing event details.
 * @param userData   Pointer to user-defined data (typically Pd4WebUserData).
 * @return           Returns EM_FALSE.
 */
EM_BOOL MouseSoundToggle(int eventType, const EmscriptenMouseEvent *e, void *userData) {
    Pd4WebUserData *ud = (Pd4WebUserData *)userData;
    if (ud == nullptr) {
        return EM_TRUE;
    }

    if (!ud->soundInit) {
        if (ud->pd4web->UseMidi()) {
            SetupMIDI();
        }
        ud->pd4web->Init();
        ud->soundInit = true;
        ud->soundSuspended = false;
        EM_ASM(
            {
                const el = document.getElementById(UTF8ToString($1));
                el.style.backgroundImage = "url(" + UTF8ToString($0) + ")";
                el.classList.remove("pulse-icon");
            },
            ICON_SOUND_ON, ud->soundToggleSel.c_str());
    } else {
        if (ud->soundSuspended) {
            EM_ASM(
                {
                    const el = document.getElementById(UTF8ToString($1));
                    el.style.backgroundImage = "url(" + UTF8ToString($0) + ")";
                },
                ICON_SOUND_ON, ud->soundToggleSel.c_str());
            ud->soundSuspended = false;
            if (ud->pd4web != nullptr) {
                emscripten_resume_audio_context_sync(ud->pd4web->GetWebAudioContext());
            }
        } else {
            EM_ASM(
                {
                    const el = document.getElementById(UTF8ToString($1));
                    el.style.backgroundImage = "url(" + UTF8ToString($0) + ")";
                },
                ICON_SOUND_OFF, ud->soundToggleSel.c_str());
            ud->soundSuspended = true;
            if (ud->pd4web != nullptr) {
                JS_SuspendAudioWorklet(ud->pd4web->GetWebAudioContext());
            }
        }
    }
    return EM_FALSE;
}

// ─────────────────────────────────────
/**
 * Sets up the asynchronous main Loop for Pd4Web.
 *
 * Retrieves the current canvas size and device pixel ratio, stores them in user data,
 * and registers the main Loop function (`Loop`) to be called repeatedly by Emscripten.
 *
 * @param userData  Pointer to Pd4WebUserData containing canvas selector and other state.
 */
void setAsyncMainLoop(void *userData) {
    Pd4WebUserData *ud = static_cast<Pd4WebUserData *>(userData);
    int fps = ud->pd4web->GetFps();
    GetGLContext(ud);

    RenderPatchComments(ud);
    emscripten_set_main_loop_arg(Loop, userData, fps, 20);
}

// ─────────────────────────────────────
/**
 * Opens a Pure Data patch with optional canvas and sound toggle identifiers.
 *
 * Extracts `canvasId` and `soundToggleId` from the JavaScript `options` object if provided,
 * then calls `OpenPatch` with the patch path and these identifiers.
 *
 * @param patchPath      The file path or URL of the patch to open.
 * @param options        JavaScript object containing optional `canvasId` and `soundToggleId`
 * strings.
 */
void Pd4Web::OpenPatchJS(const std::string &patchPath, emscripten::val options) {
    // Default values
    m_PatchPath = patchPath;
    if (m_PatchPath.empty()) {
        JS_Alert("You need to specify a patch path");
        return;
    }

    m_CanvasId = "";
    m_SoundToggleId = "";
    m_ProjectName = "";
    m_ChannelCountIn = 0;
    m_ChannelCountOut = 0;
    m_SampleRate = 48000.0f;
    m_PatchZoom = 1.0f;
    m_RenderGui = true;
    m_Fps = 0;
    m_UseMidi = false;

    if (JS_IsDarkMode()) {
        m_BgColor = "#303030";
        m_FgColor = "#fcfcfc";
    } else {
        m_BgColor = "#fcfcfc";
        m_FgColor = "#000000";
    }

    // Parse JS options
    if (!options.isUndefined()) {
        if (options.hasOwnProperty("canvasId")) {
            m_CanvasId = options["canvasId"].as<std::string>();
        }
        if (options.hasOwnProperty("soundToggleId")) {
            m_SoundToggleId = options["soundToggleId"].as<std::string>();
        }
        if (options.hasOwnProperty("patchZoom")) {
            m_PatchZoom = options["patchZoom"].as<float>();
        }
        if (options.hasOwnProperty("projectName")) {
            m_ProjectName = options["projectName"].as<std::string>();
            emscripten_set_window_title(m_ProjectName.c_str());
        }
        if (options.hasOwnProperty("channelCountIn")) {
            m_ChannelCountIn = options["channelCountIn"].as<int>();
        }
        if (options.hasOwnProperty("channelCountOut")) {
            m_ChannelCountOut = options["channelCountOut"].as<int>();
        }
        if (options.hasOwnProperty("sampleRate")) {
            m_SampleRate = options["sampleRate"].as<float>();
        }
        if (options.hasOwnProperty("renderGui")) {
            m_RenderGui = options["renderGui"].as<bool>();
        }
        if (options.hasOwnProperty("requestMidi")) {
            m_UseMidi = options["requestMidi"].as<bool>();
        }
        if (options.hasOwnProperty("fps")) {
            m_Fps = options["fps"].as<int>();
        }
        if (options.hasOwnProperty("bgColor")) {
            m_BgColor = options["bgColor"].as<std::string>();
        }
        if (options.hasOwnProperty("fgColor")) {
            m_FgColor = options["fgColor"].as<std::string>();
        }
    }

    // Call internal OpenPatch
    OpenPatch(m_PatchPath, m_CanvasId, m_SoundToggleId);
}

// ─────────────────────────────────────
/**
 * Opens a Pure Data patch with specified canvas and sound toggle elements.
 *
 * Loads the patch located at `PatchPath` and associates it with the HTML canvas
 * element identified by `PatchCanvaId`. If provided, links the sound toggle
 * control identified by `soundToggleId`.
 *
 * @param PatchPath        The file path or URL of the patch to open.
 * @param PatchCanvaId     The HTML canvas element ID for rendering audio visuals.
 * @param soundToggleId    The HTML element ID for toggling sound output.
 */
void Pd4Web::OpenPatch(std::string PatchPath, std::string PatchCanvaId, std::string soundToggleId) {
    m_PdInstance = libpd_new_instance();
    if (m_PdInstance == nullptr) {
        JS_Alert("libpd_new_instance() failed, please report!");
        return;
    }

    m_Pd4WebAudioWorkletInit = false;
    m_AudioSuspended = false;

    libpd_set_instance(m_PdInstance);
    libpd_init_audio(0, 0, m_SampleRate);

    m_UserData = std::make_shared<Pd4WebUserData>();
    m_UserData->pd4web = this;
    m_UserData->libpd = m_PdInstance;
    m_UserData->lastFrame = emscripten_get_now();

    (void)libpd_queued_init();

    libpd_set_queued_printhook(ReceivedPrintMsg);

    libpd_set_queued_banghook(ReceivedBang);
    libpd_set_queued_floathook(ReceivedFloat);
    libpd_set_queued_symbolhook(ReceivedSymbol);
    libpd_set_queued_listhook(ReceivedList);
    libpd_set_queued_messagehook(ReceivedMessage);

    // TODO: Midi messages
    // libpd_set_queued_noteonhook
    // libpd_set_queued_controlchangehook
    // libpd_set_queued_programchangehook
    // libpd_set_queued_pitchbendhook
    // libpd_set_queued_aftertouchhook
    // libpd_set_queued_polyaftertouchhook
    // libpd_set_queued_midibytehook

    // Set Audio on/off listener
    if (soundToggleId != "") {
        EM_ASM(
            {
                const el = document.getElementById(UTF8ToString($1));
                if (!el) {
                    alert("Not possible to find sound toggle icon!");
                    return;
                }
                el.style.backgroundImage = "url(" + UTF8ToString($0) + ")";
                el.style.backgroundRepeat = "no-repeat";
                el.classList.add("pulse-icon");
            },
            ICON_SOUND_OFF, soundToggleId.c_str());

        std::string sel = "#" + soundToggleId;
        m_UserData->soundToggleSel = soundToggleId;
        emscripten_set_mousedown_callback(sel.c_str(), m_UserData.get(), EM_TRUE, MouseSoundToggle);
    } else {
        emscripten_log(EM_LOG_WARN, "You don't assigned any sound toggle id, you need to run "
                                    "Pd4Web.init() from a click event!");
    }

    libpd_add_to_search_path("./Libs/");
    libpd_add_to_search_path("./Extras/");
    libpd_add_to_search_path("./Audios/");
    libpd_add_to_search_path("./Gui/");

    pdlua_setup();
    Pd4WebInitExternals();

    // open patch
    if (!libpd_openfile(PatchPath.c_str(), "./")) {
        JS_Alert("Failed to open patch | Please Report!");
        return;
    }

    // resize canvas
    if (RenderGui() && PatchCanvaId != "") {
        t_canvas *canvas = pd_getcanvaslist();
        int canvasWidth = canvas->gl_pixwidth;
        int canvasHeight = canvas->gl_pixheight;

        std::string PatchCanvaSel = "#" + PatchCanvaId;
        const char *sel = PatchCanvaSel.c_str();
        m_UserData->canvasSel = PatchCanvaSel;

        // set patch canvas size
        float dpr = emscripten_get_device_pixel_ratio();
        int cssW = canvasWidth * GetPatchZoom();
        int cssH = canvasHeight * GetPatchZoom();

        int backingW = static_cast<int>(cssW * dpr);
        int backingH = static_cast<int>(cssH * dpr);
        emscripten_set_canvas_element_size(sel, backingW, backingH);
        EM_ASM(
            {
                const canvas = document.querySelector(UTF8ToString($0));
                if (canvas) {
                    canvas.style.width = $1 + 'px';
                    canvas.style.height = $2 + 'px';
                }
            },
            sel, cssW, cssH);

        m_UserData->canvas_width = backingW;
        m_UserData->canvas_height = backingH;
        m_UserData->canvas_marginx = canvas->gl_xmargin;
        m_UserData->canvas_marginy = canvas->gl_ymargin;
        m_UserData->devicePixelRatio = dpr;

        for (t_gobj *obj = canvas->gl_list; obj; obj = obj->g_next) {
            gobj_vis(obj, canvas, 1);
        }

        m_UserData->mousedown = false;
        emscripten_set_mousedown_callback(sel, m_UserData.get(), EM_FALSE, MouseListener);
        emscripten_set_mouseup_callback(sel, m_UserData.get(), EM_FALSE, MouseListener);
        emscripten_set_mousemove_callback(sel, m_UserData.get(), EM_FALSE, MouseListener);

        // touchscreen
        emscripten_set_touchstart_callback(sel, m_UserData.get(), EM_TRUE, TouchListener);
        emscripten_set_touchend_callback(sel, m_UserData.get(), EM_TRUE, TouchListener);
        emscripten_set_touchmove_callback(sel, m_UserData.get(), EM_TRUE, TouchListener);
        emscripten_set_touchcancel_callback(sel, m_UserData.get(), EM_TRUE, TouchListener);

        // keydown (lua object must define obj::key_down)
        emscripten_set_keydown_callback(sel, m_UserData.get(), EM_FALSE, KeyListener);

        //
        // emscripten_set_focus_callback(sel, m_UserData.get(), EM_FALSE, FocusListener);

        m_UserData->libpd = m_PdInstance;
        m_UserData->pd4web = this;
        m_UserData->canvasSel = PatchCanvaSel;
        emscripten_async_call(setAsyncMainLoop, m_UserData.get(), 0);
    }
}

// ╭─────────────────────────────────────╮
// │            Gui Interface            │
// ╰─────────────────────────────────────╯
void RenderPatchComments(Pd4WebUserData *ud) {
    t_canvas *canvas = pd_getcanvaslist();
    if (!canvas) {
        return;
    }

    for (t_gobj *obj = canvas->gl_list; obj; obj = obj->g_next) {
        if (obj->g_pd && obj->g_pd->c_name && strcmp(obj->g_pd->c_name->s_name, "text") == 0) {
            t_text *txt = (t_text *)obj;
            t_binbuf *bb = txt->te_binbuf;
            if (!bb) {
                continue;
            }

            char *textbuf = nullptr;
            int textsize = 0;
            binbuf_gettext(bb, &textbuf, &textsize);

            if (!textbuf || textsize <= 0) {
                continue;
            }

            char safe_buf[1025];
            int copy_len = (textsize < 1024) ? textsize : 1024;
            memcpy(safe_buf, textbuf, copy_len);
            safe_buf[copy_len] = '\0';

            GuiCommand cmd;
            cmd.command = DRAW_TEXT;

            std::string fg = ud->pd4web->GetFGColor();
            strncpy(cmd.current_color, fg.c_str(), sizeof(cmd.current_color) - 1);
            cmd.current_color[sizeof(cmd.current_color) - 1] = '\0';

            strncpy(cmd.layer_id, "default_layer_0", sizeof(cmd.layer_id) - 1);
            cmd.layer_id[sizeof(cmd.layer_id) - 1] = '\0';
            snprintf(cmd.text, sizeof(cmd.text), "%s", safe_buf);
            cmd.x1 = 0;
            cmd.y1 = 0;

            cmd.w = txt->te_width;
            cmd.h = 16;
            if (txt->te_width == 0) {
                float bounds[4];
                cmd.w = nvgTextBounds(ud->vg, 0, 0, cmd.text, nullptr, bounds);
                cmd.h = bounds[3] - bounds[1];
            } else {
                nvgFontSize(ud->vg, 10);

                float bounds[4];
                float charWidthBounds[4];
                nvgTextBounds(ud->vg, 0, 0, "A", nullptr, charWidthBounds);
                float charWidth = (charWidthBounds[2] - charWidthBounds[0]) / 1.35;
                float wrapWidth = charWidth * txt->te_width;
                nvgTextBoxBounds(ud->vg, 0, 0, wrapWidth, cmd.text, nullptr, bounds);
                cmd.w = (bounds[2] - bounds[0]);
                cmd.h = (bounds[3] - bounds[1]);
                cmd.objw = (bounds[2] - bounds[0]);
                cmd.objh = (bounds[3] - bounds[1]);
            }

            cmd.font_size = 10;
            cmd.objx = txt->te_xpix;
            cmd.objy = txt->te_ypix;

            char obj_layer_id[64];
            snprintf(obj_layer_id, 64, "layer_%p", obj);
            ClearLayerCommand(obj_layer_id, 0, cmd.objx, cmd.objy, cmd.w, cmd.h);
            AddNewCommand(obj_layer_id, 0, &cmd);
            EndPaintLayerCommand(obj_layer_id, 0);
        }
    }
}

// ─────────────────────────────────────
static void HexToRgbNormalized(const char *hex, float *r, float *g, float *b) {
    if (hex[0] == '#') {
        hex++;
    }

    char rs[3] = {hex[0], hex[1], '\0'};
    char gs[3] = {hex[2], hex[3], '\0'};
    char bs[3] = {hex[4], hex[5], '\0'};

    int ri = (int)strtol(rs, NULL, 16);
    int gi = (int)strtol(gs, NULL, 16);
    int bi = (int)strtol(bs, NULL, 16);

    *r = ri / 255.0f;
    *g = gi / 255.0f;
    *b = bi / 255.0f;
}

// ─────────────────────────────────────
/**
 * Creates and initializes a WebGL2 context for rendering within Pd4Web.
 *
 * If the context is already ready, it sets it as the current context.
 * Otherwise, it configures WebGL2 attributes for high-performance rendering,
 * creates the context, and initializes NanoVG for vector graphics rendering.
 * It also loads a font ("DejaVuSans.ttf") named "roboto" into NanoVG.
 * The background color is set to a default value, and the color and stencil buffers are cleared.
 *
 * If any step fails, appropriate cleanup is performed and alerts are shown.
 *
 * @param ud  Pointer to Pd4WebUserData containing canvas selector and context state.
 */
// ─────────────────────────────────────
void GetGLContext(Pd4WebUserData *ud) {
    if (ud->contextReady) {
        emscripten_webgl_make_context_current(ud->ctx);
        return;
    }

    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);

    //
    attrs.alpha = false; // or true if you want canvas transparency
    attrs.depth = false;
    attrs.stencil = true; // needed for NanoVG and FBO clipping
    attrs.antialias = true;
    attrs.premultipliedAlpha = false;
    attrs.preserveDrawingBuffer = true; // <-- THIS IS IMPORTANT FOR DIRTY RECT
    attrs.powerPreference = EM_WEBGL_POWER_PREFERENCE_HIGH_PERFORMANCE;
    attrs.majorVersion = 2;
    attrs.minorVersion = 0;
    attrs.enableExtensionsByDefault = true;

    // Optional, usually not needed for dirty rects:
    attrs.explicitSwapControl = false;
    attrs.renderViaOffscreenBackBuffer = false;

    ud->ctx = emscripten_webgl_create_context(ud->canvasSel.c_str(), &attrs);
    if (ud->ctx <= 0) {
        return;
    }

    emscripten_webgl_make_context_current(ud->ctx);
    ud->vg = nvgCreateContext(NVG_ANTIALIAS);
    if (!ud->vg) {
        JS_Alert("Failed to create NanoVG context");
        emscripten_webgl_destroy_context(ud->ctx);
        return;
    }

    ud->font_handler = nvgCreateFont(ud->vg, "DejaVuSans", "DejaVuSans.ttf");
    if (ud->font_handler == -1) {
        JS_Alert("Failed to create NanoVG font");
        return;
    }
    nvgFontFaceId(ud->vg, ud->font_handler);

    float r, g, b;
    std::string bg = ud->pd4web->GetBGColor();
    HexToRgbNormalized(bg.c_str(), &r, &g, &b);
    glClearColor(r, g, b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    ud->contextReady = true;
}

// ─────────────────────────────────────
/**
 * Retrieves the GUI command handlers (from pd-lua) for the current Pure Data instance.
 *
 * Returns a reference to the PdLuaObjsGui map associated with the current
 * libpd instance, allowing access to GUI-related Lua objects and commands.
 *
 * @return Reference to PdLuaObjsGui for the current Pure Data instance.
 */
PdLuaObjsGui &GetLibPDInstanceCommands() {
    static PdInstanceGui GuiCommands;
    t_pdinstance *pd = libpd_this_instance();
    return GuiCommands[pd];
}

// ─────────────────────────────────────
/**
 * Clears all GUI commands and resets drawing parameters for a specified object layer.
 *
 * Frees any allocated path coordinates in GUI commands, clears the command list,
 * marks the layer as dirty and ready for redraw, and updates the drawing rectangle
 * with the given position `(x, y)` and size `(w, h)`.
 *
 * @param obj_layer_id  Identifier string for the target object layer.
 * @param layer         Layer index within the object layer.
 * @param x             X-coordinate of the drawing area.
 * @param y             Y-coordinate of the drawing area.
 * @param w             Width of the drawing area.
 * @param h             Height of the drawing area.
 */
void ClearLayerCommand(const char *obj_layer_id, int layer, int x, int y, int w, int h) {
    std::string layer_id(obj_layer_id);

    PdLuaObjsGui &pdlua_objs = GetLibPDInstanceCommands();
    PdLuaObjLayers &obj_layers = pdlua_objs[layer_id];
    PdLuaObjGuiLayer &obj_layer = obj_layers[layer];

    for (GuiCommand &cmd : obj_layer.gui_commands) {
        if (cmd.path_coords) {
            free(cmd.path_coords);
            cmd.path_coords = nullptr;
            cmd.path_size = 0;
        }
    }
    obj_layer.gui_commands.clear();
    obj_layer.dirty = true;
    obj_layer.drawing = true;
    obj_layer.objw = w;
    obj_layer.objh = h;
    obj_layer.objx = x;
    obj_layer.objy = y;
}

// ─────────────────────────────────────
/**
 * Marks the end of the painting Process for a specified layer.
 *
 * Sets the drawing flag of the given layer to false, indicating that
 * rendering operations are complete for this layer.
 *
 * @param obj_layer_id  Identifier string for the target object layer.
 * @param layer         Layer index within the object layer.
 */
void EndPaintLayerCommand(const char *obj_layer_id, int layer) {
    std::string layer_id(obj_layer_id);

    PdLuaObjsGui &pdlua_objs = GetLibPDInstanceCommands();
    PdLuaObjLayers &obj_layers = pdlua_objs[layer_id];
    PdLuaObjGuiLayer &obj_layer = obj_layers[layer];
    obj_layer.drawing = false;
}

// ─────────────────────────────────────
/**
 * Adds a new GUI command to the specified object layer and layer index.
 *
 * Creates a deep copy of the provided `GuiCommand`, including duplicating
 * its path coordinates if present, then appends it to the layer's command list.
 * Marks the layer as dirty and currently drawing.
 *
 * @param obj_layer_id  Identifier string for the target object layer.
 * @param layer         Layer index within the object layer.
 * @param c             Pointer to the GuiCommand to add (must not be null).
 */
void AddNewCommand(const char *obj_layer_id, int layer, GuiCommand *c) {
    if (!c) {
        fprintf(stderr, "NULL command\n");
        return;
    }

    std::string layer_id(obj_layer_id);

    PdLuaObjsGui &pdlua_objs = GetLibPDInstanceCommands();
    PdLuaObjLayers &obj_layers = pdlua_objs[layer_id];
    PdLuaObjGuiLayer &obj_layer = obj_layers[layer];
    GuiCommand copy = *c;

    if (c->path_size > 0 && c->path_coords) {
        size_t total_coords = c->path_size * 2;
        copy.path_coords = (float *)malloc(sizeof(float) * total_coords);

        if (copy.path_coords) {
            memcpy(copy.path_coords, c->path_coords, sizeof(float) * total_coords);
            copy.path_size = c->path_size;
        } else {
            copy.path_coords = nullptr;
            copy.path_size = 0;
        }
    } else {
        copy.path_coords = nullptr;
        copy.path_size = 0;
    }

    obj_layer.gui_commands.push_back(copy);
    obj_layer.dirty = true;
    obj_layer.drawing = true;
}

// ─────────────────────────────────────
/**
 * Renders a GUI command using NanoVG based on its type and parameters.
 *
 * Sets fill and stroke colors from the command's current color, then
 * performs the appropriate drawing operation such as filling/stroking
 * rectangles, ellipses, paths, lines, rounded rectangles, or drawing text.
 *
 * @param ud   Pointer to Pd4WebUserData containing the NanoVG context.
 * @param cmd  Pointer to the GuiCommand to be drawn.
 */
void Pd4WebDraw(Pd4WebUserData *ud, GuiCommand *cmd) {
    float r, g, b;
    HexToRgbNormalized(cmd->current_color, &r, &g, &b);

    nvgFillColor(ud->vg, nvgRGBAf(r, g, b, 1.0f));
    nvgStrokeColor(ud->vg, nvgRGBAf(r, g, b, 1.0f));

    switch (cmd->command) {
    case FILL_ALL: {
        nvgBeginPath(ud->vg);
        nvgRect(ud->vg, 0, 0, cmd->canvas_width, cmd->canvas_height);
        nvgFill(ud->vg);
        nvgClosePath(ud->vg);

        nvgBeginPath(ud->vg);
        std::string fg = ud->pd4web->GetFGColor();
        HexToRgbNormalized(fg.c_str(), &r, &g, &b);
        nvgStrokeColor(ud->vg, nvgRGBAf(r, g, b, 1.0f));
        nvgStrokeWidth(ud->vg, 1);
        nvgRect(ud->vg, 0, 0, cmd->canvas_width, cmd->canvas_height);
        nvgStroke(ud->vg);
        nvgClosePath(ud->vg);

        break;
    }
    case FILL_RECT: {
        nvgBeginPath(ud->vg);
        nvgRect(ud->vg, cmd->x1, cmd->y1, cmd->w, cmd->h);
        nvgFill(ud->vg);
        nvgClosePath(ud->vg);
        break;
    }
    case STROKE_RECT: {
        float x = cmd->x1;
        float y = cmd->y1;
        float width = cmd->w;
        float height = cmd->h;
        float thickness = cmd->line_width;
        nvgBeginPath(ud->vg);
        nvgRect(ud->vg, x, y, width, height);
        nvgStrokeWidth(ud->vg, thickness);
        nvgStroke(ud->vg);
        nvgClosePath(ud->vg);
        break;
    }
    case FILL_ELLIPSE: {
        float x = cmd->x1;
        float y = cmd->y1;
        float width = cmd->w;
        float height = cmd->h;
        float cx = x + width * 0.5f;
        float cy = y + height * 0.5f;
        float rx = width * 0.5f;
        float ry = height * 0.5f;
        nvgBeginPath(ud->vg);
        nvgFillColor(ud->vg, nvgRGBAf(r, g, b, 1.0f));
        nvgEllipse(ud->vg, cx, cy, rx, ry);
        nvgFill(ud->vg);
        nvgClosePath(ud->vg);
        break;
    }
    case STROKE_ELLIPSE: {
        float x = cmd->x1;
        float y = cmd->y1;
        float width = cmd->w;
        float height = cmd->h;
        float line_width = cmd->line_width;
        float cx = x + width * 0.5f;
        float cy = y + height * 0.5f;
        float rx = width * 0.5f;
        float ry = height * 0.5f;
        nvgBeginPath(ud->vg);
        nvgStrokeWidth(ud->vg, line_width);
        nvgEllipse(ud->vg, cx, cy, rx, ry);

        nvgStroke(ud->vg);
        nvgClosePath(ud->vg);
        break;
    }

    case FILL_ROUNDED_RECT: {
        float x = cmd->x1;
        float y = cmd->y1;
        float width = cmd->w;
        float height = cmd->h;
        float radius = cmd->radius;
        nvgBeginPath(ud->vg);
        nvgFillColor(ud->vg, nvgRGBAf(r, g, b, 1.0f));
        nvgRoundedRect(ud->vg, x, y, width, height, radius);
        nvgFill(ud->vg);
        break;
    }

    case STROKE_ROUNDED_RECT: {
        float x = cmd->x1;
        float y = cmd->y1;
        float width = cmd->w;
        float height = cmd->h;
        float radius = cmd->radius;
        float thickness = cmd->line_width;
        nvgBeginPath(ud->vg);
        nvgStrokeColor(ud->vg, nvgRGBAf(r, g, b, 1.0f));
        nvgRoundedRect(ud->vg, x, y, width, height, radius);
        nvgStrokeWidth(ud->vg, thickness);
        nvgStroke(ud->vg);
        break;
    }

    case DRAW_LINE: {
        float x1 = cmd->x1;
        float y1 = cmd->y1;
        float x2 = cmd->x2;
        float y2 = cmd->y2;
        float line_width = cmd->line_width;
        nvgBeginPath(ud->vg);
        nvgStrokeWidth(ud->vg, line_width);
        nvgStrokeColor(ud->vg, nvgRGBAf(r, g, b, 1.0f));
        nvgMoveTo(ud->vg, x1, y1);
        nvgLineTo(ud->vg, x2, y2);
        nvgStroke(ud->vg);
        break;
    }

    case STROKE_PATH: {
        float line_width = cmd->line_width;
        float *coords = cmd->path_coords;
        int coords_len = cmd->path_size;
        nvgBeginPath(ud->vg);
        nvgStrokeWidth(ud->vg, line_width);
        nvgStrokeColor(ud->vg, nvgRGBAf(r, g, b, 1.0f));
        if (coords_len >= 2) {
            nvgMoveTo(ud->vg, coords[0], coords[1]);
            for (int i = 1; i < coords_len; i++) {
                nvgLineTo(ud->vg, coords[i * 2], coords[i * 2 + 1]);
            }
        }
        nvgStroke(ud->vg);
        break;
    }
    case FILL_PATH: {
        float *coords = cmd->path_coords;
        int coords_len = cmd->path_size;
        nvgBeginPath(ud->vg);
        if (coords_len >= 2) {
            nvgMoveTo(ud->vg, coords[0], coords[1]);
            for (int i = 1; i < coords_len; i++) {
                nvgLineTo(ud->vg, coords[i * 2], coords[i * 2 + 1]);
            }
            nvgClosePath(ud->vg);
        }
        nvgFillColor(ud->vg, nvgRGBAf(r, g, b, 1.0f));
        nvgFill(ud->vg);
        break;
    }
    case DRAW_TEXT: {
        if (cmd->text[0] == '\0' || cmd->w <= 0 || cmd->font_size <= 0) {
            break;
        }
        if (ud->font_handler >= 0) {
            nvgBeginPath(ud->vg);
            nvgFontSize(ud->vg, cmd->font_size);
            nvgTextAlign(ud->vg, NVG_ALIGN_TOP | NVG_ALIGN_LEFT);
            nvgTextBox(ud->vg, round(cmd->x1), round(cmd->y1), cmd->w, cmd->text, nullptr);
        }
        break;
    }
    }
}

// ─────────────────────────────────────
/**
 * Main Loop function called repeatedly by Emscripten.
 *
 * Handles processing and rendering tasks using the provided user data.
 *
 * @param userData  Pointer to user-defined data (Pd4WebUserData).
 */
void Loop(void *userData) {
    Pd4WebUserData *ud = static_cast<Pd4WebUserData *>(userData);
    libpd_set_instance(ud->libpd);
    libpd_queued_receive_pd_messages();
    libpd_queued_receive_midi_messages();

    GetGLContext(ud);
    if (ud->vg == nullptr) {
        emscripten_log(EM_LOG_ERROR, "NanoVG context invalid");
        return;
    }
    if (!ud->soundInit) {
        // Before audio initialization, the internal tick of Pd is driven by the main loop.
        // After audio is initialized, control of the tick is handed over to the audio processing
        // thread and cannot be reverted. As a result, if audio is initialized and later suspended,
        // the graphical interface becomes static and unresponsive due to the absence of tick
        // updates.

        float sampleRate = ud->pd4web->GetSampleRate();
        float blockSize = 64.0f;
        double now = emscripten_get_now();
        double elapsed = now - ud->lastFrame;
        ud->lastFrame = now;
        int ticks = static_cast<int>((elapsed / 1000.0) * (sampleRate / blockSize));
        libpd_process_float(ticks, nullptr, nullptr);
    }

    float zoom = ud->pd4web->GetPatchZoom();
    float pxRatio = ud->devicePixelRatio;

    PdLuaObjsGui &pdlua_objs = GetLibPDInstanceCommands();
    std::vector<PdLuaObjLayers> objs_to_redraw;

    int dirtySceneMinX = std::numeric_limits<int>::max();
    int dirtySceneMinY = std::numeric_limits<int>::max();
    int dirtySceneMaxX = std::numeric_limits<int>::min();
    int dirtySceneMaxY = std::numeric_limits<int>::min();

    bool needs_redraw = false;

    for (auto &obj_pair : pdlua_objs) {
        PdLuaObjLayers &obj_layers = obj_pair.second;
        bool object_dirty = false;

        for (auto &layer_pair : obj_layers) {
            PdLuaObjGuiLayer &layer = layer_pair.second;

            if (layer.objw < 1 || layer.objh < 1 || !layer.dirty || layer.drawing) {
                continue;
            }

            object_dirty = true;
            needs_redraw = true;

            int fbw = static_cast<int>(layer.objw * zoom * pxRatio);
            int fbh = static_cast<int>(layer.objh * zoom * pxRatio);

            if (!layer.fb) {
                layer.fb = nvgluCreateFramebuffer(ud->vg, fbw, fbh,
                                                  NVG_IMAGE_PREMULTIPLIED | NVG_IMAGE_NEAREST);
            }

            nvgluBindFramebuffer(layer.fb);
            glViewport(0, 0, fbw, fbh);

            nvgBeginFrame(ud->vg, fbw, fbh, pxRatio);
            nvgScissor(ud->vg, 0, 0, fbw, fbh);
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            nvgSave(ud->vg);
            nvgScale(ud->vg, zoom * pxRatio, zoom * pxRatio);

            for (GuiCommand &cmd : layer.gui_commands) {
                Pd4WebDraw(ud, &cmd);
            }
            nvgRestore(ud->vg);
            nvgResetScissor(ud->vg);
            nvgEndFrame(ud->vg);
            nvgluBindFramebuffer(nullptr);

            layer.dirty = false;

            int lx = layer.objx - ud->canvas_marginx;
            int ly = layer.objy - ud->canvas_marginy;
            int lw = layer.objw;
            int lh = layer.objh;

            dirtySceneMinX = std::min(dirtySceneMinX, lx);
            dirtySceneMinY = std::min(dirtySceneMinY, ly);
            dirtySceneMaxX = std::max(dirtySceneMaxX, lx + lw);
            dirtySceneMaxY = std::max(dirtySceneMaxY, ly + lh);
        }

        if (object_dirty) {
            objs_to_redraw.push_back(obj_layers);
        }
    }

    if (!needs_redraw || objs_to_redraw.empty()) {
        return;
    }

    int dirtyW = dirtySceneMaxX - dirtySceneMinX;
    int dirtyH = dirtySceneMaxY - dirtySceneMinY;

    if (!ud->mainFBO) {
        ud->mainFBO = nvgluCreateFramebuffer(ud->vg, ud->canvas_width, ud->canvas_height,
                                             NVG_IMAGE_PREMULTIPLIED);
    }

    nvgluBindFramebuffer(ud->mainFBO);
    glViewport(0, 0, ud->canvas_width, ud->canvas_height);

    int scissorX = static_cast<int>(dirtySceneMinX * zoom * pxRatio);
    int scissorY = static_cast<int>(dirtySceneMinY * zoom * pxRatio);
    int scissorW = static_cast<int>(dirtyW * zoom * pxRatio);
    int scissorH = static_cast<int>(dirtyH * zoom * pxRatio);

    glEnable(GL_SCISSOR_TEST);
    glScissor(scissorX, scissorY, scissorW, scissorH);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    nvgBeginFrame(ud->vg, ud->canvas_width, ud->canvas_height, pxRatio);
    nvgSave(ud->vg);
    nvgScissor(ud->vg, dirtySceneMinX * zoom * pxRatio, dirtySceneMinY * zoom * pxRatio,
               dirtyW * zoom * pxRatio, dirtyH * zoom * pxRatio);

    for (const auto &obj_layers : objs_to_redraw) {
        for (const auto &layer_pair : obj_layers) {
            const PdLuaObjGuiLayer &layer = layer_pair.second;
            if (!layer.fb) {
                continue;
            }

            float dstX = (layer.objx - ud->canvas_marginx) * zoom * pxRatio;
            float dstY = (layer.objy - ud->canvas_marginy) * zoom * pxRatio;
            float dstW = layer.objw * zoom * pxRatio;
            float dstH = layer.objh * zoom * pxRatio;

            NVGpaint paint =
                nvgImagePattern(ud->vg, dstX, dstY, dstW, dstH, 0, layer.fb->image, 1.0f);
            nvgBeginPath(ud->vg);
            nvgRect(ud->vg, dstX, dstY, dstW, dstH);
            nvgFillPaint(ud->vg, paint);
            nvgFill(ud->vg);
        }
    }

    nvgResetScissor(ud->vg);
    nvgRestore(ud->vg);
    nvgEndFrame(ud->vg);
    nvgluBindFramebuffer(nullptr);

    // Final draw on screen
    nvgBeginFrame(ud->vg, ud->canvas_width, ud->canvas_height, pxRatio);
    nvgScissor(ud->vg, dirtySceneMinX * zoom * pxRatio, dirtySceneMinY * zoom * pxRatio,
               dirtyW * zoom * pxRatio, dirtyH * zoom * pxRatio);

    NVGpaint screenPaint = nvgImagePattern(ud->vg, 0, 0, ud->canvas_width, ud->canvas_height, 0,
                                           ud->mainFBO->image, 1.0f);
    nvgBeginPath(ud->vg);
    nvgRect(ud->vg, dirtySceneMinX * zoom * pxRatio, dirtySceneMinY * zoom * pxRatio,
            dirtyW * zoom * pxRatio, dirtyH * zoom * pxRatio);
    nvgFillPaint(ud->vg, screenPaint);
    nvgFill(ud->vg);

    nvgResetScissor(ud->vg);
    nvgEndFrame(ud->vg);
}

// ╭─────────────────────────────────────╮
// │            Init Function            │
// ╰─────────────────────────────────────╯
/**
 * Initializes the Pd4Web instance.
 *
 * Sets up the Web Audio context with specified sample rate and channels,
 * creates user data, and starts the audio worklet thread asynchronously.
 */
void Pd4Web::Init() {
    EmscriptenWebAudioCreateAttributes attrs = {
        .latencyHint = "interactive",
        .sampleRate =  static_cast<uint32_t>(m_SampleRate),
    };
    // Start the audio context
    static uint8_t WasmAudioWorkletStack[1024 * 1024];
    m_AudioContext = emscripten_create_audio_context(&attrs);

    m_UserData->pd4web = this;
    m_UserData->libpd = m_PdInstance;

    libpd_set_instance(m_PdInstance);
    SetSampleRate(m_SampleRate);
    emscripten_start_wasm_audio_worklet_thread_async(m_AudioContext, WasmAudioWorkletStack,
                                                     sizeof(WasmAudioWorkletStack),
                                                     AudioWorkletInit, m_UserData.get());
    m_AudioSuspended = false;
}

// ╭─────────────────────────────────────╮
// │            Main Function            │
// ╰─────────────────────────────────────╯
/**
 * Entry point of the application.
 *
 * Sets the window title, initializes libpd with a print hook,
 * and outputs the pd4web version.
 *
 * @return 0 on successful execution.
 */
int main() {
    libpd_set_printhook(ReceivedPrintMsg);
    int result = libpd_init();
    if (result != 0) {
        JS_Alert("Failed to initialize libpd, please report to pd4web");
        abort();
    }

    std::cout << std::format("pd4web version {}.{}.{}", PD4WEB_VERSION_MAJOR, PD4WEB_VERSION_MINOR,
                             PD4WEB_VERSION_PATCH)
              << std::endl;
    return 0;
}
