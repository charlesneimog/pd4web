#include "pd4web.hpp"

// ╭─────────────────────────────────────╮
// │        JavaScript Functions         │
// ╰─────────────────────────────────────╯
// Functions written in JavaScript Language, this are used for the WebAudio API.
// Then we don't need to pass the WebAudio Context as in version 1.0.
// clang-format off
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

// ╭─────────────────────────────────────╮
// │            Senders Hooks            │
// ╰─────────────────────────────────────╯
// clang-format on
struct Pd4WebSender {
    const char *receiver;
    Pd4WebSenderType type;
    float f;
    const char *m;
};

// ─────────────────────────────────────
void senderCallBack(t_pd *obj, void *data) {
    Pd4WebSender *sender = (Pd4WebSender *)data;
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
    case LIST:
        // TODO:
        break;
    case MESSAGE:
        // TODO:
        break;
    default:
        break;
    }

    delete sender;
}

// ─────────────────────────────────────
void Pd4Web::sendBang(std::string s) {
    Pd4WebSender *sender = new Pd4WebSender();
    sender->type = BANG;
    t_pd *receiver = gensym(s.c_str())->s_thing;
    pd_queue_mess(m_NewPdInstance, receiver, (void *)sender, senderCallBack);
}

// ─────────────────────────────────────
void Pd4Web::sendFloat(std::string s, float f) {
    Pd4WebSender *sender = new Pd4WebSender();
    sender->type = FLOAT;
    sender->f = f;
    t_pd *receiver = gensym(s.c_str())->s_thing;
    pd_queue_mess(m_NewPdInstance, receiver, (void *)sender, senderCallBack);
}

// ─────────────────────────────────────
void Pd4Web::sendSymbol(std::string s, std::string thing) {
    Pd4WebSender *sender = new Pd4WebSender();
    sender->type = SYMBOL;
    sender->m = thing.c_str();
    t_pd *receiver = gensym(s.c_str())->s_thing;
    pd_queue_mess(m_NewPdInstance, receiver, (void *)sender, senderCallBack);
}

// ─────────────────────────────────────
void Pd4Web::sendList(const std::string &r, emscripten::val jsArray) {
    const unsigned len = jsArray["length"].as<unsigned>();
    if (libpd_start_message(len)) {
        _JS_alert("Failed to start message for sendList");
        return;
    }

    for (unsigned i = 0; i < len; ++i) {
        emscripten::val v = jsArray[i];
        if (v.isNumber()) {
            libpd_add_float(v.as<double>());
        } else if (v.isString()) {
            libpd_add_symbol(v.as<std::string>().c_str());
        } else {
            std::cerr << "Unsupported type at index " << i << " for sendList\n";
        }
    }
    if (libpd_finish_list(r.c_str())) {
        _JS_alert("Failed to send message for sendList");
    }
}

// ─────────────────────────────────────
void Pd4Web::sendMessage(const std::string &r, const std::string &s, emscripten::val jsArray) {
    const unsigned len = jsArray["length"].as<unsigned>();
    t_atom atoms[len];

    for (unsigned i = 0; i < len; ++i) {
        emscripten::val v = jsArray[i];
        if (v.isNumber()) {
            libpd_set_float(&atoms[i], v.as<double>());
        } else if (v.isString()) {
            libpd_set_symbol(&atoms[i], v.as<std::string>().c_str());
        } else {
            std::cerr << "Unsupported type at index " << i << " for sendMessage\n";
        }
    }
    libpd_message(r.c_str(), s.c_str(), len, atoms);
}

// ─────────────────────────────────────
static std::unordered_map<std::string, std::string> bindedReceivers;
void Pd4Web::bind(std::string r, std::string func) {
    bindedReceivers[r] = func;
}

// ─────────────────────────────────────
void Pd4Web::unbind(std::string r) {
    auto it = bindedReceivers.find(r);
    if (it != bindedReceivers.end()) {
        bindedReceivers.erase(it);
    }
}

// ─────────────────────────────────────
void receivedBang(const char *r) {
    std::string functionName = bindedReceivers[r];
    if (functionName.empty()) {
        return;
    }

    // clang-format off
    EM_ASM(
        {
            const receiver = UTF8ToString($0);
            const func = UTF8ToString($1);
            if (typeof window[func] === 'function') {
                window[func](receiver);
            } else {
                console.warn("Function not found:", func);
            }
        },
        r, functionName.c_str());
    // clang-format on
}

// ─────────────────────────────────────
void Pd4Web::receivedFloat(const char *r, float f) {
    std::string functionName = bindedReceivers[r];
    if (functionName.empty()) {
        return;
    }
}

// ─────────────────────────────────────
void Pd4Web::receivedSymbol(const char *r, const char *s) {}
// ─────────────────────────────────────
void Pd4Web::receivedList(const char *r, int argc, t_atom *argv) {}
// ─────────────────────────────────────
void Pd4Web::receivedMessage(const char *r, const char *s, int argc, t_atom *argv) {}

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

EM_BOOL process(int numInputs, const AudioSampleFrame *In, int numOutputs, AudioSampleFrame *Out, int numParams,
                const AudioParamFrame *params, void *data) {

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
void audioWorkletProcessorCreated(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData) {

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
    Pd4WebUserData *ud = (Pd4WebUserData *)userData;
    libpd_set_instance(ud->libpd);

    // turn audio on
    libpd_start_message(1);
    libpd_add_float(1.0f);
    libpd_finish_message("pd", "dsp");
    libpd_init_audio(NInCh, NOutCh, SR);

    std::string id = "pd4web_" + std::to_string(libpd_num_instances());
    EMSCRIPTEN_AUDIO_WORKLET_NODE_T AudioWorkletNode =
        emscripten_create_wasm_audio_worklet_node(audioContext, id.c_str(), &options, process, userData);
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
void audioWorkletInit(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData) {
    // LOG("Pd4Web::audioWorkletInit");
    Pd4WebUserData *ud = (Pd4WebUserData *)userData;
    if (!success) {
        _JS_alert("WebAudio worklet thread initialization failed!\n");
        return;
    }

    std::string id = "pd4web_" + std::to_string(libpd_num_instances());
    WebAudioWorkletProcessorCreateOptions opts = {
        .name = id.c_str(),
    };

    libpd_set_instance(ud->libpd);
    emscripten_create_wasm_audio_worklet_processor_async(audioContext, &opts, audioWorkletProcessorCreated, userData);
}

// ─────────────────────────────────────
/**
 * Suspends the audio worklet using JavaScript.
 */
void Pd4Web::suspendAudio() {
    _JS_suspendAudioWorkLet(m_Context);
}

// ╭─────────────────────────────────────╮
// │             Midi Input              │
// ╰─────────────────────────────────────╯
void Pd4Web::startMidi() {
    EM_ASM(
        {
            function onMIDISuccess(midiAccess) {
                for (let input of midiAccess.inputs.values()) {
                    input.onmidimessage = function(message) {
                        const data = message.data;
                        instance._midiByte(data[0], data[1], data[2]);
                    };
                }
            }
            function onMIDIFailure() {
                alert("Failed to access MIDI devices.");
            }

            // request midi supprt
            if (navigator.requestMIDIAccess) {
                navigator.requestMIDIAccess().then(onMIDISuccess, onMIDIFailure);
            } else {
                alert("Web MIDI API is not supported.");
            }
        },
        this);
}

// ─────────────────────────────────────
void Pd4Web::midiByte(uint8_t byte1, uint8_t byte2, uint8_t byte3) {
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
EM_BOOL touch_listener(int eventType, const EmscriptenTouchEvent *e, void *userData) {
    Pd4WebUserData *data = (Pd4WebUserData *)userData;
    libpd_set_instance(data->libpd);

    t_canvas *canvas = pd_getcanvaslist();
    if (!canvas) {
        fprintf(stderr, "No pd canvas found\n");
        return EM_TRUE;
    }

    if (e->numTouches < 1) {
        return EM_TRUE;
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

    return EM_TRUE;
}

// ─────────────────────────────────────
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

    return EM_TRUE;
}

// ─────────────────────────────────────
EM_BOOL sound_toggle(int eventType, const EmscriptenMouseEvent *e, void *userData) {
    Pd4WebUserData *data = (Pd4WebUserData *)userData;
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
            emscripten_resume_audio_context_sync(data->pd4web->m_Context);
        } else {
            EM_ASM(
                {
                    const el = document.getElementById(UTF8ToString($1));
                    el.style.backgroundImage = "url(" + UTF8ToString($0) + ")";
                },
                ICON_SOUND_OFF, data->soundToggleSel.c_str());
            data->soundSuspended = true;
            _JS_suspendAudioWorkLet(data->pd4web->m_Context);
        }
    }

    // delete data;
    return EM_FALSE;
}

// ─────────────────────────────────────
void setAsyncMainLoop(void *userData) {
    Pd4WebUserData *ud = (Pd4WebUserData *)userData;
    int canvas_width, canvas_height;
    emscripten_get_canvas_element_size(ud->canvasSel.c_str(), &canvas_width, &canvas_height);
    ud->canvas_width = canvas_width;
    ud->canvas_height = canvas_height;
    ud->devicePixelRatio = emscripten_get_device_pixel_ratio();
    emscripten_set_main_loop_arg(loop, (void *)userData, 0, 0);
}

// ─────────────────────────────────────
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
void Pd4Web::openPatch(std::string PatchPath, std::string PatchCanvaId, std::string soundToggleId) {
    // init new instance of libpd
    m_NewPdInstance = libpd_new_instance();
    if (m_NewPdInstance == NULL) {
        _JS_alert("libpd_init() failed, please report!");
        return;
    }
    libpd_set_instance(m_NewPdInstance);

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
                el.classList.add("pulse-icon");
            },
            ICON_SOUND_OFF, soundToggleId.c_str());

        std::string selector = "#" + soundToggleId;
        m_SoundToggle = new Pd4WebUserData();
        m_SoundToggle->soundInit = false;
        m_SoundToggle->soundSuspended = false;
        m_SoundToggle->pd4web = this;
        m_SoundToggle->soundToggleSel = soundToggleId;
        emscripten_set_mousedown_callback(selector.c_str(), (void *)m_SoundToggle, EM_FALSE, sound_toggle);
    }

    // add paths
    libpd_set_queued_printhook(_JS_post);

    // TODO: messages
    // libpd_banghook
    // libpd_floathook
    // libpd_symbolhook
    // libpd_listhook
    // libpd_messagehook

    // TODO: midi
    // libpd_noteonhook
    // libpd_controlchangehook
    // libpd_programchangehook
    // libpd_pitchbendhook
    // libpd_aftertouchhook
    // libpd_polyaftertouchhook
    // libpd_midibytehook

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
        _JS_alert("Failed to open patch | Please Report!\n");
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
            // init Gui Interface
            int zoom = PD4WEB_PATCH_ZOOM;
            emscripten_set_canvas_element_size(sel, canvasWidth * zoom, canvasHeight * zoom);

            t_canvas *canvas = pd_getcanvaslist();
            for (t_gobj *obj = canvas->gl_list; obj; obj = obj->g_next) {
                gobj_vis(obj, canvas, 1);
            }

            m_MouseListener = new Pd4WebUserData(); // deleted on Pd4Web destructor
            m_MouseListener->libpd = m_NewPdInstance;
            m_MouseListener->mousedown = false;
            m_MouseListener->pd4web = this;

            // mouse
            emscripten_set_mousedown_callback(sel, (void *)m_MouseListener, EM_FALSE, mouse_listener);
            emscripten_set_mouseup_callback(sel, (void *)m_MouseListener, EM_FALSE, mouse_listener);
            emscripten_set_mousemove_callback(sel, (void *)m_MouseListener, EM_FALSE, mouse_listener);

            // touchscreen
            emscripten_set_touchstart_callback(sel, (void *)m_MouseListener, EM_FALSE, touch_listener);
            emscripten_set_touchend_callback(sel, (void *)m_MouseListener, EM_FALSE, touch_listener);
            emscripten_set_touchmove_callback(sel, (void *)m_MouseListener, EM_FALSE, touch_listener);
            emscripten_set_touchcancel_callback(sel, (void *)m_MouseListener, EM_FALSE, touch_listener);

            // TODO: key input (for nbx, floatatom, and similar)
        }

        m_MainLoop = new Pd4WebUserData(); // delete on Pd4Web destructor
        m_MainLoop->libpd = m_NewPdInstance;
        m_MainLoop->pd4web = this;
        m_MainLoop->canvasSel = PatchCanvaSel;
        create_webgl_context(m_MainLoop);
        if (m_MainLoop->vg == nullptr) {
            _JS_alert("NanoVG invalid Context, not rendering patch");
            return;
        }

        emscripten_async_call(setAsyncMainLoop, (void *)m_MainLoop, 0); // to avoid unwind error
    }
}

// ╭─────────────────────────────────────╮
// │            Gui Interface            │
// ╰─────────────────────────────────────╯
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
void create_webgl_context(Pd4WebUserData *ud) {
    if (ud->contextReady) {
        emscripten_webgl_make_context_current(ud->ctx);
        return;
    }

    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);

    attrs.alpha = false;
    attrs.depth = false;
    attrs.stencil = false;
    attrs.antialias = false;
    attrs.premultipliedAlpha = false;
    attrs.preserveDrawingBuffer = false;
    attrs.powerPreference = EM_WEBGL_POWER_PREFERENCE_HIGH_PERFORMANCE;
    attrs.failIfMajorPerformanceCaveat = false;
    attrs.majorVersion = 2;
    attrs.minorVersion = 2;
    attrs.enableExtensionsByDefault = false;
    attrs.explicitSwapControl = false;
    attrs.proxyContextToMainThread = EMSCRIPTEN_WEBGL_CONTEXT_PROXY_DISALLOW;
    attrs.renderViaOffscreenBackBuffer = false;

    ud->ctx = emscripten_webgl_create_context(ud->canvasSel.c_str(), &attrs);
    if (ud->ctx <= 0) {
        return;
    }

    emscripten_webgl_make_context_current(ud->ctx);
    ud->vg = nvgCreateContext(NVG_ANTIALIAS);
    if (!ud->vg) {
        _JS_alert("Failed to create NanoVG context");
        emscripten_webgl_destroy_context(ud->ctx);
        return;
    }

    ud->font_handler = nvgCreateFont(ud->vg, "roboto", "DejaVuSans.ttf");
    if (ud->font_handler == -1) {
        _JS_alert("Failed to create NanoVG font");
        return;
    }

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    ud->contextReady = true;
}

// ╭─────────────────────────────────────╮
// │            Gui Commands             │
// ╰─────────────────────────────────────╯
PdLuaObjsGui &get_libpd_instance_commands() {
    static PdInstanceGui GuiCommands;
    t_pdinstance *pd = libpd_this_instance();
    return GuiCommands[pd];
}

// ─────────────────────────────────────
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
void endpaint_layercommand(const char *obj_layer_id, int layer) {
    std::string layer_id(obj_layer_id);

    PdLuaObjsGui &pdlua_objs = get_libpd_instance_commands();
    PdLuaObjLayers &obj_layers = pdlua_objs[layer_id];
    PdLuaObjGuiLayer &obj_layer = obj_layers[layer];
    obj_layer.drawing = false;
}

// ─────────────────────────────────────
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
void pd4webdraw_command(Pd4WebUserData *ud, GuiCommand *cmd) {
    float r, g, b;
    hex_to_rgb_normalized(cmd->current_color, &r, &g, &b);
    nvgFillColor(ud->vg, nvgRGBAf(r, g, b, 1.0f));
    nvgStrokeColor(ud->vg, nvgRGBAf(r, g, b, 1.0f));

    switch (cmd->command) {
    case FILL_ALL: {
        nvgBeginPath(ud->vg);
        nvgRect(ud->vg, 0, 0, cmd->canvas_width, cmd->canvas_height);
        nvgFill(ud->vg);

        nvgStrokeColor(ud->vg, nvgRGBAf(0, 0, 0, 1.0f));
        nvgStrokeWidth(ud->vg, 1 * PD4WEB_PATCH_ZOOM);
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
            nvgTextBox(ud->vg, cmd->x1, cmd->y1, cmd->w, cmd->text, nullptr);
        }
        break;
    }
    }
}

// ─────────────────────────────────────
void loop(void *userData) {
    Pd4WebUserData *ud = static_cast<Pd4WebUserData *>(userData);
    libpd_set_instance(ud->libpd);

    create_webgl_context(ud);
    if (ud->vg == nullptr) {
        _JS_alert("NanoVG context invalid");
        return;
    }
    float zoom = PD4WEB_PATCH_ZOOM;

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

            int fbw = layer.objw * zoom;
            int fbh = layer.objh * zoom;

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
                pd4webdraw_command(ud, &cmd);
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

    glClearColor(1, 1, 1, 1);
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

    // TODO:
    libpd_queued_receive_pd_messages();
    libpd_queued_receive_midi_messages();
}

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

    Pd4WebUserData *userData = new Pd4WebUserData();
    userData->libpd = m_NewPdInstance;
    libpd_set_instance(m_NewPdInstance);

    // Start the audio context
    EMSCRIPTEN_WEBAUDIO_T AudioContext = emscripten_create_audio_context(&attrs);
    emscripten_start_wasm_audio_worklet_thread_async(AudioContext, WasmAudioWorkletStack, sizeof(WasmAudioWorkletStack),
                                                     audioWorkletInit, (void *)userData);
    m_Context = AudioContext;

    // TODO:
    // _JS_onReceived();

    m_audioSuspended = false;
    if (PD4WEB_MIDI) {
        // TODO: startMidi();
    }
}

// ╭─────────────────────────────────────╮
// │            Main Function            │
// ╰─────────────────────────────────────╯
int main() {
    emscripten_set_window_title(PD4WEB_PROJECT_NAME);
    libpd_set_printhook(libpd_print_concatenator);
    libpd_set_concatenated_printhook(_JS_post);
    printf("pd4web version %s.%s.%s\n", PD4WEB_VERSION_MAJOR, PD4WEB_VERSION_MINOR, PD4WEB_VERSION_PATCH);
    int ok = libpd_queued_init();
    if (ok) {
    }
    return 0;
}
