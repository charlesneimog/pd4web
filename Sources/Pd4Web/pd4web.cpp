#include "pd4web.hpp"

// ╭─────────────────────────────────────╮
// │        JavaScript Functions         │
// ╰─────────────────────────────────────╯
// Functions written in JavaScript Language, this are used for the WebAudio API.
// Then we don't need to pass the WebAudio Context as in version 1.0.
// clang-format off

// ─────────────────────────────────────
EM_JS(int, JS_isDarkMode, (), {
  return window.matchMedia &&
         window.matchMedia('(prefers-color-scheme: dark)').matches ? 1 : 0;
});

// ─────────────────────────────────────
EM_JS(void, JS_alert, (const char *msg), {
    alert(UTF8ToString(msg));
});

// ─────────────────────────────────────
EM_JS(void, JS_warning, (const char *msg), {
    console.warn(UTF8ToString(msg));
});

// ─────────────────────────────────────
EM_JS(void, JS_post, (const char *msg), {
    let msgJS = UTF8ToString(msg);
    if (msgJS == "\n"){
        return;
    }
    console.log(msgJS);
});
    
// ─────────────────────────────────────
EM_JS(void, JS_getMicAccess, (EMSCRIPTEN_WEBAUDIO_T audioContext, EMSCRIPTEN_AUDIO_WORKLET_NODE_T audioWorkletNode, int nInCh), {
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
EM_JS(void, JS_suspendAudioWorkLet, (EMSCRIPTEN_WEBAUDIO_T audioContext),{
    Pd4WebAudioContext = emscriptenGetAudioObject(audioContext);
    Pd4WebAudioContext.suspend();
});

// clang-format on
// ╭─────────────────────────────────────╮
// │            Senders Hooks            │
// ╰─────────────────────────────────────╯
void senderCallBack(t_pd *obj, void *data) {
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
            JS_alert("Failed to start message for sendList");
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
            JS_alert("Failed to send message for sendList");
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
        JS_alert("Memory corruption, please report");
        break;
    }
}

// ─────────────────────────────────────
void Pd4Web::sendBang(std::string s) {
    auto sender = std::make_unique<Pd4WebSender>();
    sender->type = BANG;
    t_pd *receiver = gensym(s.c_str())->s_thing;
    pd_queue_mess(m_NewPdInstance, receiver, sender.release(), senderCallBack);
}

// ─────────────────────────────────────
void Pd4Web::sendFloat(std::string s, float f) {
    auto sender = std::make_unique<Pd4WebSender>();
    sender->type = FLOAT;
    sender->f = f;
    t_pd *receiver = gensym(s.c_str())->s_thing;
    pd_queue_mess(m_NewPdInstance, receiver, sender.release(), senderCallBack);
}

// ─────────────────────────────────────
void Pd4Web::sendSymbol(std::string s, std::string thing) {
    auto sender = std::make_unique<Pd4WebSender>();
    sender->type = SYMBOL;
    sender->m = thing.c_str();
    t_pd *receiver = gensym(s.c_str())->s_thing;
    pd_queue_mess(m_NewPdInstance, receiver, sender.release(), senderCallBack);
}

// ─────────────────────────────────────
void Pd4Web::sendList(std::string s, emscripten::val a) {
    auto sender = std::make_unique<Pd4WebSender>();
    sender->type = LIST;
    sender->l = a;
    t_pd *receiver = gensym(s.c_str())->s_thing;
    pd_queue_mess(m_NewPdInstance, receiver, sender.release(), senderCallBack);
}

// ─────────────────────────────────────
void Pd4Web::sendMessage(std::string r, std::string s, emscripten::val a) {
    auto sender = std::make_unique<Pd4WebSender>();
    sender->type = MESSAGE;
    sender->l = a;
    sender->sel = s.c_str();
    t_pd *receiver = gensym(s.c_str())->s_thing;
    pd_queue_mess(m_NewPdInstance, receiver, sender.release(), senderCallBack);
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

void Pd4Web::onBangReceived(std::string r, emscripten::val func) {
    libpd_set_instance(m_NewPdInstance);

    void *s = libpd_bind(r.c_str());
    m_bindSymbols.push_back(s);

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
    BangReceiverListeners[m_NewPdInstance][r] = func;
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
void Pd4Web::onFloatReceived(std::string r, emscripten::val func) {
    libpd_set_instance(m_NewPdInstance);

    void *s = libpd_bind(r.c_str());
    m_bindSymbols.push_back(s);

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
    FloatReceiverListeners[m_NewPdInstance][r] = func;
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
void Pd4Web::onSymbolReceived(std::string r, emscripten::val func) {
    libpd_set_instance(m_NewPdInstance);

    void *s = libpd_bind(r.c_str());
    m_bindSymbols.push_back(s);

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
    SymbolReceiverListeners[m_NewPdInstance][r] = func;
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
void Pd4Web::onListReceived(std::string r, emscripten::val func) {
    libpd_set_instance(m_NewPdInstance);

    void *s = libpd_bind(r.c_str());
    m_bindSymbols.push_back(s);

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
    ListReceiverListeners[m_NewPdInstance][r] = func;
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
void receivedBang(const char *r) {
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
void receivedFloat(const char *r, float f) {
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
void receivedSymbol(const char *r, const char *s) {
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
void receivedList(const char *r, int argc, t_atom *argv) {
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
void receivedMessage(const char *r, const char *s, int argc, t_atom *argv) {
    // t_pdinstance *instance = libpd_this_instance();
    // emscripten::val func = ReceiverListeners[instance][std::string(r)];
    // _JS_post("here i am");
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
EM_BOOL process(int numInputs, const AudioSampleFrame *In, int numOutputs, AudioSampleFrame *Out,
                int numParams, const AudioParamFrame *params, void *data) {

    Pd4WebUserData *ud = static_cast<Pd4WebUserData *>(data);
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
void audioWorkletProcessorCreated(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success,
                                  void *userData) {

    if (!success) {
        JS_alert("Failed to create AudioWorkletProcessor, please report!\n");
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
    Pd4WebUserData *ud = (Pd4WebUserData *)userData;
    libpd_set_instance(ud->libpd);

    // turn audio on
    libpd_start_message(1);
    libpd_add_float(1.0f);
    libpd_finish_message("pd", "dsp");
    libpd_init_audio(NInCh, NOutCh, SR);

    std::string id = "pd4web_" + std::to_string(libpd_num_instances());
    EMSCRIPTEN_AUDIO_WORKLET_NODE_T AudioWorkletNode = emscripten_create_wasm_audio_worklet_node(
        audioContext, id.c_str(), &options, process, userData);
    JS_getMicAccess(audioContext, AudioWorkletNode, NInCh);
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
void audioWorkletInit(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData) {
    // LOG("Pd4Web::audioWorkletInit");
    Pd4WebUserData *ud = (Pd4WebUserData *)userData;
    if (!success) {
        JS_alert("WebAudio worklet thread initialization failed!\n");
        return;
    }

    std::string id = "pd4web_" + std::to_string(libpd_num_instances());
    WebAudioWorkletProcessorCreateOptions opts = {
        .name = id.c_str(),
    };

    libpd_set_instance(ud->libpd);
    emscripten_create_wasm_audio_worklet_processor_async(audioContext, &opts,
                                                         audioWorkletProcessorCreated, userData);
}

// ─────────────────────────────────────
/**
 * Suspends the audio worklet using JavaScript.
 */
void Pd4Web::suspendAudio() {
    JS_suspendAudioWorkLet(m_Context);
}

// ╭─────────────────────────────────────╮
// │          Midi Input/Output          │
// ╰─────────────────────────────────────╯
void onMIDIInMessage(emscripten::val message) {
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
void onMIDIOutMessage(emscripten::val message) {
    JS_alert("MIDI not implemented yet");
}

// ─────────────────────────────────────
void onMIDISuccess(emscripten::val midiAccess) {
    emscripten::val inputs = midiAccess["inputs"];
    emscripten::val iter = inputs.call<emscripten::val>("values");
    while (true) {
        emscripten::val next = iter.call<emscripten::val>("next");
        if (next["done"].as<bool>()) {
            break;
        }
        emscripten::val input = next["value"];
        input.set("onmidimessage", emscripten::val::module_property("onMIDIMessage"));
    }
}

// ─────────────────────────────────────
void onMIDIFailed(emscripten::val error) {
    JS_alert("Access to MIDI devices failed.");
}

// ─────────────────────────────────────
void setupMIDI() {
    emscripten::val navigator = emscripten::val::global("navigator");
    if (navigator["requestMIDIAccess"].typeOf().as<std::string>() != "function") {
        JS_alert("Web MIDI API is not supported.");
        return;
    }
    emscripten::val promise = navigator.call<emscripten::val>("requestMIDIAccess");
    promise.call<emscripten::val>("then", // execute promise
                                  emscripten::val::module_property("onMIDISuccess"),
                                  emscripten::val::module_property("onMIDIFailed"));
}

// ╭─────────────────────────────────────╮
// │           Getter & Setter           │
// ╰─────────────────────────────────────╯
void Pd4Web::setLastMousePosition(int x, int y) {
    m_LastMouseX = x;
    m_LastMouseY = y;
}

// ─────────────────────────────────────
void Pd4Web::getLastMousePosition(int *x, int *y) {
    *x = m_LastMouseX;
    *y = m_LastMouseY;
}

// ─────────────────────────────────────
void Pd4Web::setWebAudioContext(EMSCRIPTEN_WEBAUDIO_T ctx) {
    m_Context = ctx;
}

// ─────────────────────────────────────
EMSCRIPTEN_WEBAUDIO_T Pd4Web::getWebAudioContext() {
    return m_Context;
}

// ─────────────────────────────────────
void Pd4Web::setSampleRate(float sr) {
    m_SampleRate = sr;
}

// ─────────────────────────────────────
float Pd4Web::getSampleRate() {
    return m_SampleRate;
}

// ╭─────────────────────────────────────╮
// │             USER EVENTS             │
// ╰─────────────────────────────────────╯
void pd4web_queue_mouseclick(t_pd *obj, void *data) {
    Pd4WebUserData *d = (Pd4WebUserData *)data;
    libpd_set_instance(d->libpd);
    (void)gobj_click(d->obj, d->canvas, d->xpos, d->ypos, 0, 0, 0, d->doit);
    delete d;
}

// ─────────────────────────────────────
/**
 * Queues a keydown event to be processed by Lua within the Pd4Web environment. On Pd4Web, when
 using pdlua objects is possible to define a method key_down, nbx:key_down(x, y, key), this function
 is called here for all lua objects when there is a keydown event.
 *
 * @param obj       Pointer to the Pure Data object triggering the event.
 * @param userData  Pointer to Pd4WebUserData containing the key string and libpd instance.
 */
void pd4web_queue_keydown(t_pd *obj, void *userData) {
    Pd4WebUserData *data = (Pd4WebUserData *)userData;
    libpd_set_instance(data->libpd);

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
                lua_pushstring(L, data->key.c_str());

                if (lua_pcall(L, 4, 0, 0) != LUA_OK) {
                    const char *err = lua_tostring(L, -1);
                    fprintf(stderr, "Erro when calling key_down: %s\n", err);
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
 * Processes keyboard events and updates Pd4Web state accordingly.
 * This function is designed to be registered with Emscripten's event system.
 *
 * @param eventType  The type of keyboard event (e.g., keydown, keyup).
 * @param e          Pointer to the EmscriptenKeyboardEvent containing event details.
 * @param userData   Pointer to user-defined data (typically Pd4WebUserData).
 * @return           Returns EM_TRUE if the event was handled, otherwise EM_FALSE.
 */
EM_BOOL key_listener(int eventType, const EmscriptenKeyboardEvent *e, void *userData) {
    Pd4WebUserData *data = (Pd4WebUserData *)userData;
    libpd_set_instance(data->libpd);

    t_canvas *canvas = pd_getcanvaslist();
    if (!canvas) {
        fprintf(stderr, "No pd canvas found\n");
        return EM_FALSE;
    }

    data->pd4web->getLastMousePosition(&data->xpos, &data->ypos);

    data->canvas = canvas;
    data->doit = false;

    for (t_gobj *obj = canvas->gl_list; obj != NULL; obj = obj->g_next) {
        int x1, y1, x2, y2;
        if (canvas_hitbox(canvas, obj, data->xpos, data->ypos, &x1, &y1, &x2, &y2, 0)) {
            Pd4WebUserData *d = new Pd4WebUserData();
            d->obj = obj;
            d->canvas = canvas;
            d->xpos = data->xpos;
            d->ypos = data->ypos;
            d->doit = data->doit;
            d->libpd = data->libpd;
            d->key = e->key;
            pd_queue_mess(data->libpd, &obj->g_pd, (void *)d, pd4web_queue_keydown);
            break;
        }
    }

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
EM_BOOL touch_listener(int eventType, const EmscriptenTouchEvent *e, void *userData) {
    Pd4WebUserData *data = (Pd4WebUserData *)userData;
    libpd_set_instance(data->libpd);

    t_canvas *canvas = pd_getcanvaslist();
    if (!canvas) {
        fprintf(stderr, "No pd canvas found\n");
        return EM_FALSE;
    }

    if (e->numTouches < 1) {
        return EM_FALSE;
    }

    // Use o primeiro toque apenas (pode ser estendido para multitouch se necessário)
    int xpos = round((float)e->touches[0].targetX / PD4WEB_PATCH_ZOOM);
    int ypos = round((float)e->touches[0].targetY / PD4WEB_PATCH_ZOOM);

    data->xpos = xpos;
    data->ypos = ypos;
    data->canvas = canvas;
    data->doit = false;

    switch (eventType) {
    case EMSCRIPTEN_EVENT_TOUCHSTART:
        data->mousedown = true;
        data->doit = true;
        data->pd4web->setLastMousePosition(xpos, ypos);
        break;

    case EMSCRIPTEN_EVENT_TOUCHEND:
    case EMSCRIPTEN_EVENT_TOUCHCANCEL:
        data->mousedown = false;
        data->doit = false;
        break;

    case EMSCRIPTEN_EVENT_TOUCHMOVE:
        data->doit = data->mousedown;
        break;
    }

    for (t_gobj *obj = canvas->gl_list; obj != NULL; obj = obj->g_next) {
        int x1, y1, x2, y2;
        if (canvas_hitbox(canvas, obj, xpos, ypos, &x1, &y1, &x2, &y2, 0)) {
            Pd4WebUserData *d = new Pd4WebUserData();
            d->obj = obj;
            d->canvas = canvas;
            d->xpos = xpos;
            d->ypos = ypos;
            d->doit = data->doit;
            d->libpd = data->libpd;
            pd_queue_mess(data->libpd, &obj->g_pd, (void *)d, pd4web_queue_mouseclick);
            break;
        }
    }

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
EM_BOOL mouse_listener(int eventType, const EmscriptenMouseEvent *e, void *userData) {
    Pd4WebUserData *data = (Pd4WebUserData *)userData;
    libpd_set_instance(data->libpd);

    t_canvas *canvas = pd_getcanvaslist();
    if (!canvas) {
        fprintf(stderr, "Pd canvas is not valid for mouse event\n");
        return EM_TRUE;
    }

    int xpos = round((float)e->targetX / PD4WEB_PATCH_ZOOM);
    int ypos = round((float)e->targetY / PD4WEB_PATCH_ZOOM);

    data->xpos = xpos;
    data->ypos = ypos;
    data->canvas = canvas;

    switch (eventType) {
    case EMSCRIPTEN_EVENT_MOUSEDOWN:
        data->mousedown = true;
        data->doit = true;
        break;

    case EMSCRIPTEN_EVENT_MOUSEUP:
        data->mousedown = false;
        data->doit = false;
        data->pd4web->setLastMousePosition(xpos, ypos);
        break;

    case EMSCRIPTEN_EVENT_MOUSEMOVE:
        data->doit = data->mousedown; // Always process mousemove events
        break;
    }

    for (t_gobj *obj = canvas->gl_list; obj != NULL; obj = obj->g_next) {
        int x1, y1, x2, y2;
        if (canvas_hitbox(canvas, obj, xpos, ypos, &x1, &y1, &x2, &y2, 0)) {
            Pd4WebUserData *d = new Pd4WebUserData();
            d->obj = obj;
            d->canvas = canvas;
            d->xpos = xpos;
            d->ypos = ypos;
            d->doit = data->doit;           // Use the doit value we determined above
            d->mousedown = data->mousedown; // Pass mousedown state separately
            d->libpd = data->libpd;
            pd_queue_mess(data->libpd, &obj->g_pd, (void *)d, pd4web_queue_mouseclick);
            break;
        }
    }

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
 * @return           Returns EM_TRUE if the event was handled, otherwise EM_FALSE.
 */
EM_BOOL sound_toggle(int eventType, const EmscriptenMouseEvent *e, void *userData) {
    Pd4WebUserData *data = (Pd4WebUserData *)userData;
    setupMIDI();
    if (!data->soundInit) {
        data->pd4web->init();
        data->soundInit = true;
        data->soundSuspended = false;
        EM_ASM(
            {
                const el = document.getElementById(UTF8ToString($1));
                el.style.backgroundImage = "url(" + UTF8ToString($0) + ")";
                el.classList.remove("pulse-icon");
            },
            ICON_SOUND_ON, data->soundToggleSel.c_str());
    } else {
        if (data->soundSuspended) {
            EM_ASM(
                {
                    const el = document.getElementById(UTF8ToString($1));
                    el.style.backgroundImage = "url(" + UTF8ToString($0) + ")";
                },
                ICON_SOUND_ON, data->soundToggleSel.c_str());
            data->soundSuspended = false;
            emscripten_resume_audio_context_sync(data->pd4web->getWebAudioContext());
        } else {
            EM_ASM(
                {
                    const el = document.getElementById(UTF8ToString($1));
                    el.style.backgroundImage = "url(" + UTF8ToString($0) + ")";
                },
                ICON_SOUND_OFF, data->soundToggleSel.c_str());
            data->soundSuspended = true;
            JS_suspendAudioWorkLet(data->pd4web->getWebAudioContext());
        }
    }
    return EM_FALSE;
}

// ─────────────────────────────────────
/**
 * Sets up the asynchronous main loop for Pd4Web.
 *
 * Retrieves the current canvas size and device pixel ratio, stores them in user data,
 * and registers the main loop function (`loop`) to be called repeatedly by Emscripten.
 *
 * @param userData  Pointer to Pd4WebUserData containing canvas selector and other state.
 */
void setAsyncMainLoop(void *userData) {
    Pd4WebUserData *ud = (Pd4WebUserData *)userData;
    int canvas_width, canvas_height;
    emscripten_get_canvas_element_size(ud->canvasSel.c_str(), &canvas_width, &canvas_height);
    ud->canvas_width = canvas_width;
    ud->canvas_height = canvas_height;
    ud->devicePixelRatio = emscripten_get_device_pixel_ratio();
    emscripten_set_main_loop_arg(loop, (void *)userData, 30, 0);
}

// ─────────────────────────────────────
/**
 * Opens a Pure Data patch with optional canvas and sound toggle identifiers.
 *
 * Extracts `canvasId` and `soundToggleId` from the JavaScript `options` object if provided,
 * then calls `openPatch` with the patch path and these identifiers.
 *
 * @param patchPath      The file path or URL of the patch to open.
 * @param options        JavaScript object containing optional `canvasId` and `soundToggleId`
 * strings.
 */
void Pd4Web::openPatchJS(const std::string &patchPath, emscripten::val options) {
    std::string canvasId = "";
    std::string soundToggleId = "";

    if (!options.isUndefined()) {
        if (options.hasOwnProperty("canvasId")) {
            canvasId = options["canvasId"].as<std::string>();
        }
        if (options.hasOwnProperty("soundToggleId")) {
            soundToggleId = options["soundToggleId"].as<std::string>();
        }
    }

    openPatch(patchPath, canvasId, soundToggleId);
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
void Pd4Web::openPatch(std::string PatchPath, std::string PatchCanvaId, std::string soundToggleId) {
    m_NewPdInstance = libpd_new_instance();
    if (m_NewPdInstance == NULL) {
        JS_alert("libpd_init() failed, please report!");
        return;
    }

    libpd_set_instance(m_NewPdInstance);
    (void)libpd_queued_init();

    libpd_set_queued_banghook(receivedBang);
    libpd_set_queued_floathook(receivedFloat);
    libpd_set_queued_symbolhook(receivedSymbol);
    libpd_set_queued_listhook(receivedList);
    libpd_set_queued_messagehook(receivedMessage);

    // TODO: Midi messages

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
        m_SoundBtn = std::make_unique<Pd4WebUserData>();
        m_SoundBtn->soundInit = false;
        m_SoundBtn->soundSuspended = false;
        m_SoundBtn->pd4web = this;
        m_SoundBtn->soundToggleSel = soundToggleId;
        emscripten_set_mousedown_callback(sel.c_str(), m_SoundBtn.get(), EM_TRUE, sound_toggle);
    }

    libpd_add_to_search_path("./Libs/");
    libpd_add_to_search_path("./Extras/");
    libpd_add_to_search_path("./Audios/");
    libpd_add_to_search_path("./Gui/");

    // setup objects
#if PD4WEB_LUA
    pdlua_setup();
#endif
    Pd4WebInitExternals();

    // open patch
    if (!libpd_openfile(PatchPath.c_str(), "./")) {
        JS_alert("Failed to open patch | Please Report!");
        return;
    }

    // resize canvas
    if (PatchCanvaId != "") {
        t_canvas *canvas = pd_getcanvaslist();
        int canvasWidth = canvas->gl_pixwidth;
        int canvasHeight = canvas->gl_pixheight;
        if (canvasWidth == 0 && canvasHeight == 0) {
            canvasWidth = canvas->gl_screenx2 - canvas->gl_screenx1;
            canvasHeight = canvas->gl_screeny2 - canvas->gl_screeny1;
        }

        std::string PatchCanvaSel = "#" + PatchCanvaId;
        const char *sel = PatchCanvaSel.c_str();
        if (PD4WEB_GUI) {
            int zoom = PD4WEB_PATCH_ZOOM;
            emscripten_set_canvas_element_size(sel, canvasWidth * zoom, canvasHeight * zoom);

            t_canvas *canvas = pd_getcanvaslist();
            for (t_gobj *obj = canvas->gl_list; obj; obj = obj->g_next) {
                gobj_vis(obj, canvas, 1);
            }

            m_EventCtx = std::make_unique<Pd4WebUserData>();
            m_EventCtx->libpd = m_NewPdInstance;
            m_EventCtx->mousedown = false;
            m_EventCtx->pd4web = this;

            // mouse
            emscripten_set_mousedown_callback(sel, m_EventCtx.get(), EM_FALSE, mouse_listener);
            emscripten_set_mouseup_callback(sel, m_EventCtx.get(), EM_FALSE, mouse_listener);
            emscripten_set_mousemove_callback(sel, m_EventCtx.get(), EM_FALSE, mouse_listener);

            // touchscreen
            emscripten_set_touchstart_callback(sel, m_EventCtx.get(), EM_FALSE, touch_listener);
            emscripten_set_touchend_callback(sel, m_EventCtx.get(), EM_FALSE, touch_listener);
            emscripten_set_touchmove_callback(sel, m_EventCtx.get(), EM_FALSE, touch_listener);
            emscripten_set_touchcancel_callback(sel, m_EventCtx.get(), EM_FALSE, touch_listener);

            emscripten_set_keydown_callback(sel, m_EventCtx.get(), EM_FALSE, key_listener);
        }

        m_GuiLoopCtx = std::make_unique<Pd4WebUserData>();
        m_GuiLoopCtx->libpd = m_NewPdInstance;
        m_GuiLoopCtx->pd4web = this;
        m_GuiLoopCtx->canvasSel = PatchCanvaSel;
        getGlCtx(m_GuiLoopCtx.get());

        if (m_GuiLoopCtx->vg == nullptr) {
            JS_alert("NanoVG invalid Context, not rendering patch");
            return;
        }

        // TODO: create comments text
        emscripten_async_call(setAsyncMainLoop, m_GuiLoopCtx.get(), 0);
        // call async to avoid unwind error
    }
}

// ╭─────────────────────────────────────╮
// │            Gui Interface            │
// ╰─────────────────────────────────────╯
void getDefaultColor(std::string key, float *r, float *g, float *b) {
    // TODO: pd-0.57
    static bool themeDefined = false;
    static bool darkTheme = false;

    if (!themeDefined) {
        darkTheme = JS_isDarkMode();
    }

    if (darkTheme) {
        if (key == "bg") {
            *r = *g = *b = 0.20;
        } else if (key == "fg") {
            *r = *g = *b = 0.95;
        }
    } else {
        if (key == "bg") {
            *r = *g = *b = 0.98;
        } else if (key == "fg") {
            *r = *g = *b = 0.2;
        }
    }
}

// ─────────────────────────────────────
static void hex_to_rgb_normalized(const char *hex, float *r, float *g, float *b) {
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
void getGlCtx(Pd4WebUserData *ud) {
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
    ud->vg = nvgCreateContext(0);
    if (!ud->vg) {
        JS_alert("Failed to create NanoVG context");
        emscripten_webgl_destroy_context(ud->ctx);
        return;
    }

    ud->font_handler = nvgCreateFont(ud->vg, "roboto", "DejaVuSans.ttf");
    if (ud->font_handler == -1) {
        JS_alert("Failed to create NanoVG font");
        return;
    }

    float r, g, b;
    getDefaultColor("bg", &r, &g, &b);

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
PdLuaObjsGui &get_libpd_instance_commands() {
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
void clear_layercommand(const char *obj_layer_id, int layer, int x, int y, int w, int h) {
    std::string layer_id(obj_layer_id);

    PdLuaObjsGui &pdlua_objs = get_libpd_instance_commands();
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
 * Marks the end of the painting process for a specified layer.
 *
 * Sets the drawing flag of the given layer to false, indicating that
 * rendering operations are complete for this layer.
 *
 * @param obj_layer_id  Identifier string for the target object layer.
 * @param layer         Layer index within the object layer.
 */
void endpaint_layercommand(const char *obj_layer_id, int layer) {
    std::string layer_id(obj_layer_id);

    PdLuaObjsGui &pdlua_objs = get_libpd_instance_commands();
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
void add_newcommand(const char *obj_layer_id, int layer, GuiCommand *c) {
    if (!c) {
        fprintf(stderr, "NULL command\n");
        return;
    }

    std::string layer_id(obj_layer_id);

    PdLuaObjsGui &pdlua_objs = get_libpd_instance_commands();
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
void pd4webdraw(Pd4WebUserData *ud, GuiCommand *cmd) {
#ifdef PD4WEB_WEBGPU
#else
    float r, g, b;
    hex_to_rgb_normalized(cmd->current_color, &r, &g, &b);
    nvgFillColor(ud->vg, nvgRGBAf(r, g, b, 1.0f));
    nvgStrokeColor(ud->vg, nvgRGBAf(r, g, b, 1.0f));

    switch (cmd->command) {
    case FILL_ALL: {
        nvgBeginPath(ud->vg);
        nvgRect(ud->vg, 0, 0, cmd->canvas_width, cmd->canvas_height);
        nvgFill(ud->vg);

        float r, g, b;
        getDefaultColor("fg", &r, &g, &b);
        nvgStrokeColor(ud->vg, nvgRGBAf(r, g, b, 1.0f));
        nvgStrokeWidth(ud->vg, 2);
        nvgStroke(ud->vg);
        break;
    }
    case FILL_RECT: {
        nvgBeginPath(ud->vg);
        nvgRect(ud->vg, cmd->x1, cmd->y1, cmd->w, cmd->h);
        nvgFill(ud->vg);
        break;
    }
    case STROKE_RECT: {
        float x = cmd->x1;
        float y = cmd->y1;
        float width = cmd->w;
        float height = cmd->h;
        float thickness = cmd->line_width;
        nvgBeginPath(ud->vg);
        nvgStrokeWidth(ud->vg, thickness);
        nvgRect(ud->vg, x, y, width, height);
        nvgStroke(ud->vg);
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
        nvgStrokeColor(ud->vg, nvgRGBAf(r, g, b, 1.0f));
        nvgEllipse(ud->vg, cx, cy, rx, ry);
        nvgStroke(ud->vg);
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
            nvgFontFaceId(ud->vg, ud->font_handler);
            nvgFontSize(ud->vg, cmd->font_size);
            nvgTextAlign(ud->vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
            nvgTextBox(ud->vg, round(cmd->x1), round(cmd->y1), cmd->w, cmd->text, nullptr);
        }
        break;
    }
    }
#endif
}

// ─────────────────────────────────────
/**
 * Main loop function called repeatedly by Emscripten.
 *
 * Handles processing and rendering tasks using the provided user data.
 *
 * @param userData  Pointer to user-defined data (Pd4WebUserData).
 */
void loop(void *userData) {
    Pd4WebUserData *ud = static_cast<Pd4WebUserData *>(userData);
    libpd_set_instance(ud->libpd);
    libpd_queued_receive_pd_messages();
    libpd_queued_receive_midi_messages();

    getGlCtx(ud);
    if (ud->vg == nullptr) {
        JS_warning("NanoVG context invalid");
        return;
    }

    float zoom = PD4WEB_PATCH_ZOOM;
    // ud->devicePixelRatio = 1;

    PdLuaObjsGui &pdlua_objs = get_libpd_instance_commands();
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

            int fbw = static_cast<int>(layer.objw * zoom);
            int fbh = static_cast<int>(layer.objh * zoom);

            if (!layer.fb) {
                layer.fb = nvgluCreateFramebuffer(ud->vg, fbw, fbh, NVG_IMAGE_PREMULTIPLIED);
            }

            nvgluBindFramebuffer(layer.fb);
            glViewport(0, 0, fbw, fbh);

            nvgBeginFrame(ud->vg, fbw, fbh, ud->devicePixelRatio);
            nvgScissor(ud->vg, 0, 0, fbw, fbh);
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            nvgSave(ud->vg);
            nvgScale(ud->vg, zoom, zoom); // zoom aqui é correto

            for (GuiCommand &cmd : layer.gui_commands) {
                pd4webdraw(ud, &cmd);
            }

            nvgRestore(ud->vg);
            nvgResetScissor(ud->vg);
            nvgEndFrame(ud->vg);
            nvgluBindFramebuffer(nullptr);

            layer.dirty = false;

            // Região suja em coordenadas lógicas
            int lx = layer.objx;
            int ly = layer.objy;
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

    // Criação do mainFBO, se necessário
    if (!ud->mainFBO) {
        ud->mainFBO = nvgluCreateFramebuffer(ud->vg, ud->canvas_width, ud->canvas_height,
                                             NVG_IMAGE_PREMULTIPLIED);
    }

    // Limpa a área suja do mainFBO
    nvgluBindFramebuffer(ud->mainFBO);
    glViewport(0, 0, ud->canvas_width, ud->canvas_height);

    int scissorX = static_cast<int>(dirtySceneMinX * zoom);
    int scissorY = static_cast<int>(dirtySceneMinY * zoom);
    int scissorW = static_cast<int>(dirtyW * zoom);
    int scissorH = static_cast<int>(dirtyH * zoom);

    glEnable(GL_SCISSOR_TEST);
    glScissor(scissorX, scissorY, scissorW, scissorH);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    // Composição no mainFBO
    nvgBeginFrame(ud->vg, ud->canvas_width, ud->canvas_height, ud->devicePixelRatio);
    nvgSave(ud->vg);
    nvgScissor(ud->vg, dirtySceneMinX * zoom, dirtySceneMinY * zoom, dirtyW * zoom, dirtyH * zoom);

    for (const auto &obj_layers : objs_to_redraw) {
        for (const auto &layer_pair : obj_layers) {
            const PdLuaObjGuiLayer &layer = layer_pair.second;
            if (!layer.fb) {
                continue;
            }

            float dstX = layer.objx * zoom;
            float dstY = layer.objy * zoom;
            float dstW = layer.objw * zoom;
            float dstH = layer.objh * zoom;

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

    // Composição final na tela
    nvgBeginFrame(ud->vg, ud->canvas_width, ud->canvas_height, ud->devicePixelRatio);

    // Aqui: zoom aplicado manualmente, então nada de nvgScale
    nvgScissor(ud->vg, dirtySceneMinX * zoom, dirtySceneMinY * zoom, dirtyW * zoom, dirtyH * zoom);

    NVGpaint screenPaint = nvgImagePattern(ud->vg, 0, 0, ud->canvas_width, ud->canvas_height, 0,
                                           ud->mainFBO->image, 1.0f);

    nvgBeginPath(ud->vg);
    nvgRect(ud->vg, dirtySceneMinX * zoom, dirtySceneMinY * zoom, dirtyW * zoom, dirtyH * zoom);
    nvgFillPaint(ud->vg, screenPaint);
    nvgFill(ud->vg);

    nvgResetScissor(ud->vg);
    nvgEndFrame(ud->vg);
}

// ─────────────────────────────────────
void loop2(void *userData) {
    Pd4WebUserData *ud = static_cast<Pd4WebUserData *>(userData);
    libpd_set_instance(ud->libpd);
    libpd_queued_receive_pd_messages();
    libpd_queued_receive_midi_messages();

    getGlCtx(ud);
    if (ud->vg == nullptr) {
        JS_warning("NanoVG context invalid");
        return;
    }
    float zoom = PD4WEB_PATCH_ZOOM;

#ifdef PD4WEB_WEBGPU
    // need to draw
#else
    bool needs_redraw = false;
    PdLuaObjsGui &pdlua_objs = get_libpd_instance_commands();
    for (auto &obj_pair : pdlua_objs) {
        PdLuaObjLayers &obj_layers = obj_pair.second;

        for (auto &layer_pair : obj_layers) {
            int layer_num = layer_pair.first;
            PdLuaObjGuiLayer &layer = layer_pair.second;
            if (layer.objw < 1 || layer.objh < 1 || !layer.dirty || layer.drawing) {
                continue;
            }
            needs_redraw = true;

            int fbw = static_cast<int>(layer.objw * zoom);
            int fbh = static_cast<int>(layer.objh * zoom);

            if (!layer.fb) {
                layer.fb = nvgluCreateFramebuffer(ud->vg, fbw, fbh, NVG_IMAGE_PREMULTIPLIED);
            }

            // Render to the offscreen framebuffer
            nvgluBindFramebuffer(layer.fb);
            glViewport(0, 0, fbw, fbh);
            nvgBeginFrame(ud->vg, fbw, fbh, ud->devicePixelRatio);
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            // Draw something in the framebuffer, e.g., a red rectangle
            nvgSave(ud->vg);
            nvgScale(ud->vg, zoom, zoom);
            for (GuiCommand &cmd : layer.gui_commands) {
                pd4webdraw(ud, &cmd);
            }

            nvgRestore(ud->vg); // desfaz o scale
            nvgEndFrame(ud->vg);
            nvgluBindFramebuffer(nullptr);
            layer.dirty = false;
        }
    }
    if (!needs_redraw) {
        return;
    }

    // size of main canvas
    glViewport(0, 0, ud->canvas_width, ud->canvas_height);
    nvgBeginFrame(ud->vg, ud->canvas_width, ud->canvas_height, ud->devicePixelRatio);

    float r, g, b;
    getDefaultColor("bg", &r, &g, &b);
    glClearColor(r, g, b, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    nvgSave(ud->vg);
    nvgScale(ud->vg, zoom, zoom);
    for (auto &obj_pair : pdlua_objs) {
        std::string layer_id = obj_pair.first;
        PdLuaObjLayers &obj_layers = obj_pair.second;
        for (size_t i = 0; i < obj_layers.size(); ++i) {
            PdLuaObjGuiLayer &layer = obj_layers[i];
            if (!layer.fb) {
                continue;
            }
            int x = layer.objx;
            int y = layer.objy;
            int w = layer.objw;
            int h = layer.objh;
            int fbImage = layer.fb->image;
            NVGpaint paint = nvgImagePattern(ud->vg, x, y, w, h, 0, fbImage, 1.0f);
            nvgBeginPath(ud->vg);
            nvgRect(ud->vg, x, y, w, h);
            nvgFillPaint(ud->vg, paint);
            nvgFill(ud->vg);
        }
    }
    nvgRestore(ud->vg);
    nvgEndFrame(ud->vg);
#endif
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
void Pd4Web::init() {
    float NInCh = PD4WEB_CHS_IN;
    float NOutCh = PD4WEB_CHS_OUT;

    EmscriptenWebAudioCreateAttributes attrs = {
        .latencyHint = "interactive",
        .sampleRate = PD4WEB_SR,
    };

    // TODO: replace this by smart pointer
    Pd4WebUserData *userData = new Pd4WebUserData();
    userData->libpd = m_NewPdInstance;
    libpd_set_instance(m_NewPdInstance);

    // Start the audio context
    static uint8_t WasmAudioWorkletStack[1024 * 1024];
    EMSCRIPTEN_WEBAUDIO_T AudioContext = emscripten_create_audio_context(&attrs);
    setSampleRate(PD4WEB_SR);

    emscripten_start_wasm_audio_worklet_thread_async(AudioContext, WasmAudioWorkletStack,
                                                     sizeof(WasmAudioWorkletStack),
                                                     audioWorkletInit, (void *)userData);
    m_Context = AudioContext;
    m_audioSuspended = false;
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
    emscripten_set_window_title(PD4WEB_PROJECT_NAME);
    libpd_set_printhook(JS_post);
    int result = libpd_init();
    if (result != 0) {
        JS_alert("Failed to initialize libpd, please report to pd4web");
        abort();
    }
    std::cout << std::format("pd4web version {}.{}.{}", PD4WEB_VERSION_MAJOR, PD4WEB_VERSION_MINOR,
                             PD4WEB_VERSION_PATCH)
              << std::endl;
    return 0;
}
