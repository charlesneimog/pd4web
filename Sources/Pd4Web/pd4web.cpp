#include "pd4web.hpp"

// ╭─────────────────────────────────────╮
// │        JavaScript Functions         │
// ╰─────────────────────────────────────╯
// Functions written in JavaScript Language, this are used for the WebAudio API.
// Then we don't need to pass the WebAudio Context as in version 1.0.
// clang-format off
EM_JS(void, JS_CreateNumberInput, (const char *canvasId), {
    if (document.getElementById("_pd4web_number_input")) return;

    const canvasIdStr = UTF8ToString(canvasId);
    const canvas = document.getElementById(canvasIdStr);
    if (!canvas) {
        console.error("Canvas is undefined");
        return;
    }

    const el = document.createElement("input");

    el.id = "_pd4web_number_input";
    el.type = "text";
    el.inputMode = "decimal";
    el.autocomplete = "off";
    el.autocapitalize = "off";
    el.spellcheck = false;
    el.setAttribute("enterkeyhint", "done");
    el.tabIndex = -1;

    // Keep it effectively invisible while still focusable for mobile keyboards
    el.style.position = "fixed";
    el.style.top = "0";
    el.style.left = "0";
    el.style.width = "1px";        
    el.style.height = "1px";      
    el.style.opacity = "0.0001";      
    el.style.pointerEvents = "auto"; 

    el.style.zIndex = "0";
    document.body.appendChild(el);

    function dispatchFakeKey(key, code, keyCode) {
        const ev = new KeyboardEvent("keydown", {
            key,
            code,
            keyCode,
            which: keyCode,
            bubbles: true,
        });
        canvas.dispatchEvent(ev);
    }

    // digits (value change)
    el.addEventListener("input", () => {
        const value = el.value;
        if (!value) return;
        const digit = value[value.length - 1];
        el.value = "";
        dispatchFakeKey(
            digit,
            "Numpad" + digit,
            digit.charCodeAt(0)
        );
    });

    el.addEventListener("blur", (e) => {
        dispatchFakeKey("Enter", "Enter", 13); 
    });
});

// ─────────────────────────────────────
EM_JS(void, JS_CreateTextInput, (const char *canvasId), {
    if (document.getElementById("_pd4web_text_input")) return;
    const el = document.createElement("input");
    el.id = "_pd4web_text_input";
    el.type = "text";
    el.inputMode = "text";
    el.autocomplete = "off";
    el.autocapitalize = "off";
    el.spellcheck = false;
    el.setAttribute("enterkeyhint", "done");
    el.tabIndex = -1;

    const canvasIdStr = UTF8ToString(canvasId);
    const canvas = document.getElementById(canvasIdStr);
    if (!canvas) {
        console.error("Canvas is undefined");
        return;
    }

    // Keep it effectively invisible while still focusable for mobile keyboards
    el.style.position = "fixed";
    el.style.top = "0";
    el.style.left = "0";
    el.style.width = "1px";        // small but focusable
    el.style.height = "1px";       // enough to trigger key events
    el.style.opacity = "0.0001";      // effectively invisible
    el.style.pointerEvents = "auto"; // must be focusable

    el.style.zIndex = "0";
    document.body.appendChild(el);

    function dispatchFakeKey(key, code, keyCode) {
        const ev = new KeyboardEvent("keydown", {
            key,
            code,
            keyCode,
            which: keyCode,
            bubbles: true,
        });
        canvas.dispatchEvent(ev);
        el.focus();
    }

    // digits (value change)
    el.addEventListener("input", () => {
        const value = el.value;
        if (!value) return;
        const digit = value[value.length - 1];
        el.value = "";
        dispatchFakeKey(
            digit,
            "Numpad" + digit,
            digit.charCodeAt(0)
        );
    });

    el.addEventListener("blur", (e) => {
        dispatchFakeKey("Enter", "Enter", 13); 
    });
});

// ─────────────────────────────────────
EM_JS(void, JS_Pd4WebFocusNumberInput, (), {
    const el = document.getElementById("_pd4web_number_input");
    if (!el) return;
    el.focus({ preventScroll: true });
});

// ─────────────────────────────────────
EM_JS(void, JS_Pd4WebFocusTextInput, (), {
    const el = document.getElementById("_pd4web_text_input");
    if (!el) return;
    el.focus({ preventScroll: true });
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

// Return a client coordinate relative to the patch canvas. Mouse-up is listened for on the
// window, so Emscripten's targetX/targetY may refer to an element other than the canvas.
EM_JS(double, JS_CanvasRelativeClientX, (const char *canvasSel, double clientX), {
    const canvas = document.querySelector(UTF8ToString(canvasSel));
    return canvas ? clientX - canvas.getBoundingClientRect().left : clientX;
});

EM_JS(double, JS_CanvasRelativeClientY, (const char *canvasSel, double clientY), {
    const canvas = document.querySelector(UTF8ToString(canvasSel));
    return canvas ? clientY - canvas.getBoundingClientRect().top : clientY;
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
// Note: This callback is no longer used - messages are processed directly
// in the Process() function on the Audio Worklet thread
void SenderCallback(t_pd *obj, void *data) {
    // Deprecated
    return;
}

namespace {
template <std::size_t N> void CopyBounded(char (&destination)[N], const std::string &source) {
    const auto count = std::min(source.size(), N - 1);
    std::memcpy(destination, source.data(), count);
    destination[count] = '\0';
}
} // namespace

bool Pd4Web::EnqueueSender(const Pd4WebSender &sender) noexcept {
    auto *slot = m_ToSendQueue.beginPush();
    if (!slot) {
        m_DroppedSenders.fetch_add(1, std::memory_order_relaxed);
        return false;
    }
    *slot = sender;
    m_ToSendQueue.commitPush();
    return true;
}

// ─────────────────────────────────────
/**
 * Send a bang message to Pure Data (thread-safe).
 *
 * This function is called from the main thread and queues a bang message
 * to be processed on the Audio Worklet thread before the next audio block.
 *
 * @param s The receiver symbol in Pure Data.
 */
bool Pd4Web::SendBang(std::string s) {
    Pd4WebSender sender{};
    sender.type = BANG;
    CopyBounded(sender.receiver, s);
    return EnqueueSender(sender);
}

// ─────────────────────────────────────
/**
 * Send a float message to Pure Data (thread-safe).
 *
 * This function is called from the main thread and queues a float message
 * to be processed on the Audio Worklet thread before the next audio block.
 *
 * @param std::string The receiver symbol in Pure Data.
 * @param float The float value to send.
 */
bool Pd4Web::SendFloat(std::string s, float f) {
    Pd4WebSender sender{};
    sender.type = FLOAT;
    sender.f_value = f;
    CopyBounded(sender.receiver, s);
    return EnqueueSender(sender);
}

// ─────────────────────────────────────
/**
 * Send a symbol message to Pure Data (thread-safe).
 *
 * This function is called from the main thread and queues a symbol message
 * to be processed on the Audio Worklet thread before the next audio block.
 * String data is copied into the sender structure for thread safety.
 *
 * @param std::string The receiver symbol in Pure Data.
 * @param std::string The symbol string to send.
 */
bool Pd4Web::SendSymbol(std::string s, std::string thing) {
    Pd4WebSender sender{};
    sender.type = SYMBOL;
    CopyBounded(sender.receiver, s);
    CopyBounded(sender.s_value, thing);
    return EnqueueSender(sender);
}

// ─────────────────────────────────────
/**
 * Send a list message to Pure Data (thread-safe).
 *
 * This function is called from the main thread. It converts the JavaScript
 * array (emscripten::val) into a thread-safe vector of atoms on the main
 * thread, then queues the message for processing on the Audio Worklet thread.
 *
 * IMPORTANT: emscripten::val objects cannot be accessed from worker threads,
 * so all conversion must happen here on the main thread.
 *
 * @param std::string The receiver symbol in Pure Data.
 * @param emscripten::val JavaScript array containing numbers and/or strings.
 */
bool Pd4Web::SendList(std::string s, emscripten::val a) {
    if (!a.isArray()) {
        emscripten_log(EM_LOG_ERROR, "SendList: argument is not an array");
        return false;
    }

    size_t length = a["length"].as<size_t>();
    Pd4WebSender sender{};
    sender.type = LIST;
    CopyBounded(sender.receiver, s);

    for (size_t i = 0; i < length; ++i) {
        emscripten::val v = a[i];
        if (v.isNumber()) {
            if (sender.atomCount >= Pd4WebSender::MaxAtoms) {
                return false;
            }
            auto &atom = sender.list_data[sender.atomCount++];
            atom.type = Pd4WebAtom::FLOAT_TYPE;
            atom.f_value = v.as<float>();
        } else if (v.isString()) {
            if (sender.atomCount >= Pd4WebSender::MaxAtoms) {
                return false;
            }
            auto &atom = sender.list_data[sender.atomCount++];
            atom.type = Pd4WebAtom::SYMBOL_TYPE;
            CopyBounded(atom.s_value, v.as<std::string>());
        } else {
            emscripten_log(EM_LOG_WARN, "SendList: unsupported type at index %zu", i);
        }
    }

    return EnqueueSender(sender);
}

// ─────────────────────────────────────
/**
 * Send a typed message to Pure Data.
 *
 * This function is called from the main thread. It converts a JavaScript
 * array (emscripten::val) into a vector of Pd4WebAtom objects,
 * then queues the message (receiver + selector + atoms) for processing
 * on the Audio Worklet thread.
 *
 * IMPORTANT: emscripten::val objects must not be accessed from worker
 * threads, so all conversions are performed on the main thread.
 *
 * @param r std::string Receiver symbol in Pure Data
 * @param s std::string Message selector (e.g. "set", "connect")
 * @param a emscripten::val JavaScript array containing numbers and/or strings
 */
bool Pd4Web::SendMessage(std::string r, std::string s, emscripten::val a) {
    // Convert emscripten::val array to std::vector<Pd4WebAtom> on main thread
    if (!a.isArray()) {
        emscripten_log(EM_LOG_ERROR, "SendMessage: argument is not an array");
        return false;
    }

    size_t length = a["length"].as<size_t>();
    Pd4WebSender sender{};
    sender.type = MESSAGE;
    CopyBounded(sender.receiver, r);
    CopyBounded(sender.selector, s);

    for (size_t i = 0; i < length; ++i) {
        emscripten::val v = a[i];
        if (v.isNumber()) {
            if (sender.atomCount >= Pd4WebSender::MaxAtoms) {
                return false;
            }
            auto &atom = sender.list_data[sender.atomCount++];
            atom.type = Pd4WebAtom::FLOAT_TYPE;
            atom.f_value = v.as<float>();
        } else if (v.isString()) {
            if (sender.atomCount >= Pd4WebSender::MaxAtoms) {
                return false;
            }
            auto &atom = sender.list_data[sender.atomCount++];
            atom.type = Pd4WebAtom::SYMBOL_TYPE;
            CopyBounded(atom.s_value, v.as<std::string>());
        } else {
            emscripten_log(EM_LOG_WARN, "SendMessage: unsupported type at index %zu", i);
        }
    }

    return EnqueueSender(sender);
}

// ─────────────────────────────────────
/**
 * Send a file to pd4web (thread-safe).
 *
 * This function is called from the main thread. It converts a JavaScript
 * ArrayBuffer into a file stored in the pd4web virtual filesystem (FS).
 *
 * The pd4web filesystem is completely isolated from the host filesystem.
 * Any file that must be accessed by Pure Data inside pd4web must be
 * explicitly transferred using this function.
 *
 * @param emscripten::val JavaScript ArrayBuffer containing the file data
 * @param std::string Destination filename inside the pd4web filesystem
 */
bool Pd4Web::SendFile(emscripten::val jsArrayBuffer, std::string filename) {
    size_t length = jsArrayBuffer["byteLength"].as<size_t>();
    emscripten::val uint8Array = emscripten::val::global("Uint8Array").new_(jsArrayBuffer);
    std::vector<uint8_t> buffer(length);
    for (size_t i = 0; i < length; i++) {
        buffer[i] = uint8Array[i].as<uint8_t>();
    }
    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        emscripten_log(EM_LOG_ERROR, "Failed to open output file");
        return false;
    }
    out.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
    out.close();
    return true;
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
 * @param std::string     The receiver symbol in Pure Data to listen for bangs.
 * @param emscripten::val The JavaScript callback function to invoke on bang.
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
 * @param std::string     The receiver symbol in Pure Data to listen for floats.
 * @param emscripten::val The JavaScript callback function to invoke with the float value.
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
 * @param std::string     The receiver symbol in Pure Data to listen for symbols.
 * @param emscripten::val The JavaScript callback function to invoke with the symbol string.
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
 * @param std::string     The receiver symbol in Pure Data to listen for lists.
 * @param emscripten::val JavaScript callback function to invoke when list is received.
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
 * Registers a callback to be invoked when a message is received from Pure Data.
 *
 * Binds a JavaScript callback (`func`) to a receiver symbol (`r`), which will be
 * triggered when a message is sent to `r` from Pure Data.
 *
 * @param std::string     The receiver symbol in Pure Data to listen for lists.
 * @param emscripten::val JavaScript callback function to invoke when list is received.
 */
void Pd4Web::OnMessageReceived(std::string r, emscripten::val func) {
    libpd_set_instance(m_PdInstance);

    void *s = libpd_bind(r.c_str());
    m_BindSymbols.push_back(s);

    if (func.typeOf().as<std::string>() != "function") {
        fprintf(stderr, "Error: passed value is not a function\n");
        return;
    }

    int declaredArgCount = func["length"].as<int>();
    if (declaredArgCount != 3) {
        fprintf(stderr, "Callback for onMessageReceived must have 3 arguments, but has %d\n",
                declaredArgCount);
        return;
    }
    MessageReceiverListeners[m_PdInstance][r] = func;
}

// ─────────────────────────────────────
/**
 * Called when a print is received from Pure Data.
 *
 * @param const char * The print message.
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
 * @param const char* The receiver symbol of the bang message.
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
    emscripten_log(EM_LOG_ERROR, "Callback not found or not a function");
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
    emscripten_log(EM_LOG_ERROR, "Callback not found or not a function");
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
                        emscripten_log(EM_LOG_ERROR, "Only float and string are supported");
                    }
                }
                func(emscripten::val(r), jsArray);
                return;
            }
        }
    }
    emscripten_log(EM_LOG_ERROR, "Callback not found or not a function");
}

// ─────────────────────────────────────
void ReceivedMessage(const char *r, const char *s, int argc, t_atom *argv) {
    t_pdinstance *instance = libpd_this_instance();
    auto it = MessageReceiverListeners.find(instance);
    if (it != MessageReceiverListeners.end()) {
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
                        emscripten_log(EM_LOG_ERROR, "Only float and string are supported");
                    }
                }
                func(emscripten::val(r), emscripten::val(s), jsArray);
                return;
            }
        }
    }
    emscripten_log(EM_LOG_ERROR, "Callback not found or not a function");
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

// ─────────────────────────────────────
void ReceivedPitchBend(int channel, int value) {
    // typedef void (*t_libpd_pitchbendhook)(int channel, int value)
}

// ─────────────────────────────────────
void ReceivedAfterTouch(int channel, int value) {
    // typedef void (*t_libpd_aftertouchhook)(int channel, int value)
}

// ─────────────────────────────────────
void ReceivedPolyAfterTouch(int channel, int pitch, int value) {
    // typedef void (*t_libpd_polyaftertouchhook)(int channel, int pitch, int value)
}

// ─────────────────────────────────────
void ReceivedMIDIByte(int port, int byte) {
    // typedef void (*t_libpd_midibytehook)(int port, int byte)
}

// ╭─────────────────────────────────────╮
// │            WebAudioPatch            │
// ╰─────────────────────────────────────╯
/**
 * Process the audio block.
 *
 * This function runs on the Audio Worklet thread and is called for each audio block.
 * It processes all queued messages from the main thread BEFORE processing audio,
 * ensuring thread-safe communication with libpd.
 *
 * Thread Safety:
 * - Messages are queued on the main thread in bounded slots with owned data
 * - The SPSC queue is wait-free and preserves event order
 * - All libpd calls happen on this Audio Worklet thread only
 * - emscripten::val objects are converted to POD types on the main thread
 *
 * @param numInputs Number of input buffers.
 * @param In Array of input audio frames.
 * @param numOutputs Number of output buffers.
 * @param Out Array of output audio frames.
 * @param numParams Number of audio parameters.
 * @param params Array of audio parameters.
 * @param userData Pointer to Pd4WebUserData.
 * @return true if processing succeeded, false otherwise.
 */
EM_BOOL Process(int numInputs, const AudioSampleFrame *In, int numOutputs, AudioSampleFrame *Out,
                int numParams, const AudioParamFrame *params, void *userData) {

    auto *ud = static_cast<Pd4WebUserData *>(userData);
    libpd_set_instance(ud->libpd);
    pdlua_gfx_process_recovery();

    while (const auto *sender = ud->pd4web->BeginSender()) {
        switch (sender->type) {
        case BANG:
            libpd_bang(sender->receiver);
            break;

        case FLOAT:
            libpd_float(sender->receiver, sender->f_value);
            break;

        case SYMBOL:
            libpd_symbol(sender->receiver, sender->s_value);
            break;

        case LIST: {
            size_t len = sender->atomCount;
            if (len == 0) {
                libpd_bang(sender->receiver);
            } else {
                if (libpd_start_message(len) == 0) {
                    for (size_t i = 0; i < len; ++i) {
                        const auto &atom = sender->list_data[i];
                        if (atom.type == Pd4WebAtom::FLOAT_TYPE) {
                            libpd_add_float(atom.f_value);
                        } else if (atom.type == Pd4WebAtom::SYMBOL_TYPE) {
                            libpd_add_symbol(atom.s_value);
                        }
                    }
                    libpd_finish_list(sender->receiver);
                }
            }
            break;
        }

        case MESSAGE: {
            size_t len = sender->atomCount;
            if (len > 0) {
                t_atom atoms[Pd4WebSender::MaxAtoms];

                for (size_t i = 0; i < len; ++i) {
                    const auto &atom = sender->list_data[i];
                    if (atom.type == Pd4WebAtom::FLOAT_TYPE) {
                        SETFLOAT(&atoms[i], atom.f_value);
                    } else if (atom.type == Pd4WebAtom::SYMBOL_TYPE) {
                        SETSYMBOL(&atoms[i], gensym(atom.s_value));
                    }
                }
                libpd_message(sender->receiver, sender->selector, len, atoms);
            } else {
                libpd_message(sender->receiver, sender->selector, 0, nullptr);
            }
            break;
        }

        case MOUSE_EVENT:
            ProcessMouseEvent(ud, sender->mouse_data);
            break;

        case KEY_EVENT:
            ProcessKeyEvent(ud, sender->key_data);
            break;

        case TOUCH_EVENT:
            ProcessTouchEvent(ud, sender->touch_data);
            break;

        default:
            break;
        }
        ud->pd4web->EndSender();
    }

    // Process audio
    constexpr int MaxAudioChannels = 64;
    int ChCount = std::min(Out[0].numberOfChannels, MaxAudioChannels);
    float LibPdOuts[128 * MaxAudioChannels]{};
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
/**
 * Process mouse click event on the Audio Worklet thread.
 * This is called from the Process() function, not from pd_queue_mess.
 *
 * @param ud Pointer to Pd4WebUserData containing event information.
 * @param data Mouse event data with position and button info.
 */
void ProcessMouseEvent(Pd4WebUserData *ud, const MouseEventData &data) {
    libpd_set_instance(ud->libpd);

    t_canvas *canvas = pd_getcanvaslist();
    if (!canvas) {
        return;
    }

    // Store event data in user data for canvas hit testing
    ud->xpos = data.x;
    ud->ypos = data.y;
    ud->canvas = canvas;

    // pdlua GUI objects call glist_grab() from their click handler.  For a GOP,
    // Pd promotes that grab to the root canvas, so the editor contains the actual
    // child control even though the hit object below is the outer graph.  Mirror
    // Pd's canvas_motion()/canvas_mouseup() handling here instead of repeatedly
    // calling the graph's click handler: the latter never reaches mouse_drag and
    // can lose mouse_up as soon as the pointer leaves the child's hit box.
    auto dispatchGrab = [&](bool up) {
        t_editor *editor = canvas->gl_editor;
        if (!editor || editor->e_onmotion != MA_PASSOUT || !editor->e_grab || !editor->e_motionfn) {
            return false;
        }

        editor->e_motionfn(&editor->e_grab->g_pd, data.x - editor->e_xwas, data.y - editor->e_ywas,
                           up ? 1 : 0);
        if (up) {
            editor->e_onmotion = MA_NONE;
        } else {
            editor->e_xwas = data.x;
            editor->e_ywas = data.y;
        }
        return true;
    };

    if (data.event_type == MouseEventData::MOUSE_DOWN) {
        // Recover from a missing browser mouse-up before starting a new capture.
        if (ud->mousedown) {
            if (!dispatchGrab(true) && ud->obj) {
                (void)gobj_click(ud->obj, canvas, data.x, data.y, data.shift, data.alt, 0, 0);
            }
        }

        ud->mousedown = true;
        ud->doit = true;
        ud->obj = nullptr;

        // Capture the first object which accepts the mouse-down. Later events in this gesture
        // must be delivered to this object even when the pointer leaves its hitbox.
        for (t_gobj *obj = canvas->gl_list; obj != NULL; obj = obj->g_next) {
            int x1, y1, x2, y2;
            if (canvas_hitbox(canvas, obj, data.x, data.y, &x1, &y1, &x2, &y2, 0) &&
                gobj_click(obj, canvas, data.x, data.y, data.shift, data.alt, 0, 1)) {
                ud->obj = obj;
                break;
            }
        }
        return;
    }

    if (data.event_type == MouseEventData::MOUSE_UP) {
        ud->mousedown = false;
        ud->doit = false;

        if (ud->obj) {
            if (!dispatchGrab(true)) {
                (void)gobj_click(ud->obj, canvas, data.x, data.y, data.shift, data.alt, 0, 0);
            }
            ud->obj = nullptr;
        }
        return;
    }

    ud->doit = ud->mousedown;

    // While dragging, keep routing to the captured object. Otherwise dispatch hover movement
    // through normal hit testing.
    if (ud->mousedown && ud->obj) {
        if (!dispatchGrab(false)) {
            (void)gobj_click(ud->obj, canvas, data.x, data.y, data.shift, data.alt, 0, 1);
        }
        return;
    }

    for (t_gobj *obj = canvas->gl_list; obj != NULL; obj = obj->g_next) {
        int x1, y1, x2, y2;
        if (canvas_hitbox(canvas, obj, data.x, data.y, &x1, &y1, &x2, &y2, 0)) {
            (void)gobj_click(obj, canvas, data.x, data.y, data.shift, data.alt, 0, ud->doit);
        }
    }
}

// ─────────────────────────────────────
/**
 * Process key event on the Audio Worklet thread.
 * Calls key_down method on pdlua objects.
 *
 * @param ud Pointer to Pd4WebUserData.
 * @param data Key event data.
 */
void ProcessKeyEvent(Pd4WebUserData *ud, const KeyEventData &data) {
    libpd_set_instance(ud->libpd);

    lua_State *L = __L();
    lua_getglobal(L, "pd");
    lua_getfield(L, -1, "_objects");
    lua_remove(L, -2);
    lua_pushnil(L);

    while (lua_next(L, -2) != 0) {
        if (lua_istable(L, -1)) {
            lua_getfield(L, -1, "key_down");
            if (lua_isfunction(L, -1)) {
                lua_pushvalue(L, -2);
                lua_pushinteger(L, 20);
                lua_pushinteger(L, 20);
                lua_pushstring(L, data.key);
                if (lua_pcall(L, 4, 0, 0) != LUA_OK) {
                    // The callback runs in the AudioWorklet. Defer diagnostics rather than
                    // formatting or logging on the realtime thread.
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
 * Process touch event on the Audio Worklet thread.
 *
 * @param ud Pointer to Pd4WebUserData.
 * @param data Touch event data.
 */
void ProcessTouchEvent(Pd4WebUserData *ud, const TouchEventData &data) {
    MouseEventData mouseData{};
    mouseData.x = data.x;
    mouseData.y = data.y;
    switch (data.event_type) {
    case TouchEventData::TOUCH_START:
        mouseData.event_type = MouseEventData::MOUSE_DOWN;
        break;
    case TouchEventData::TOUCH_END:
    case TouchEventData::TOUCH_CANCEL:
        mouseData.event_type = MouseEventData::MOUSE_UP;
        break;
    case TouchEventData::TOUCH_MOVE:
        mouseData.event_type = MouseEventData::MOUSE_MOVE;
        break;
    }
    ProcessMouseEvent(ud, mouseData);
}

// ─────────────────────────────────────
/**
 * Keyboard event listener callback for Emscripten environment.
 *
 * Queues keyboard events to be processed on the Audio Worklet thread.
 *
 * @param eventType  The type of keyboard event (e.g., keydown, keyup).
 * @param e          Pointer to the EmscriptenKeyboardEvent containing event details.
 * @param userData   Pointer to user-defined data (typically Pd4WebUserData).
 * @return           Returns EM_TRUE if the event was handled, otherwise EM_FALSE.
 */
EM_BOOL KeyListener(int eventType, const EmscriptenKeyboardEvent *e, void *userData) {
    Pd4WebUserData *ud = (Pd4WebUserData *)userData;
    ud->pd4web->SendFloat("#key", e->keyCode);
    return EM_TRUE;
}

// ─────────────────────────────────────
EM_BOOL OrientationListener(int eventType, const EmscriptenOrientationChangeEvent *e,
                            void *userData) {
    emscripten_run_script("location.reload();");
    return EM_TRUE;
}

// ─────────────────────────────────────
/**
 * Touch event listener callback for Emscripten environment.
 *
 * Queues touch events to be processed on the Audio Worklet thread.
 *
 * @param eventType  The type of touch event (e.g., touchstart, touchend).
 * @param e          Pointer to the EmscriptenTouchEvent containing event details.
 * @param userData   Pointer to user-defined data (typically Pd4WebUserData).
 * @return           Returns EM_TRUE if the event was handled, otherwise EM_FALSE.
 */
EM_BOOL TouchListener(int eventType, const EmscriptenTouchEvent *e, void *userData) {
    Pd4WebUserData *ud = (Pd4WebUserData *)userData;

    if (e->numTouches < 1) {
        return EM_TRUE;
    }

    // Calculate position with zoom and margins
    int xpos = round((e->touches[0].targetX / ud->pd4web->GetPatchZoom()) + PD4WEB_PATCH_MARGINX);
    int ypos = round((e->touches[0].targetY / ud->pd4web->GetPatchZoom()) + PD4WEB_PATCH_MARGINY);

    // Create event data on main thread
    TouchEventData touchData;
    touchData.x = xpos;
    touchData.y = ypos;
    touchData.identifier = e->touches[0].identifier;

    switch (eventType) {
    case EMSCRIPTEN_EVENT_TOUCHSTART:
        touchData.event_type = TouchEventData::TOUCH_START;
        ud->pd4web->SetLastMousePosition(xpos, ypos);
        break;
    case EMSCRIPTEN_EVENT_TOUCHEND:
        touchData.event_type = TouchEventData::TOUCH_END;
        break;
    case EMSCRIPTEN_EVENT_TOUCHCANCEL:
        touchData.event_type = TouchEventData::TOUCH_CANCEL;
        break;
    case EMSCRIPTEN_EVENT_TOUCHMOVE:
        touchData.event_type = TouchEventData::TOUCH_MOVE;
        break;
    }

    // Queue event for processing on Audio Worklet thread
    Pd4WebSender sender{};
    sender.type = TOUCH_EVENT;
    sender.touch_data = touchData;
    ud->pd4web->EnqueueSender(sender);

    return EM_TRUE;
}

// ─────────────────────────────────────
/**
 * Mouse event listener callback for Emscripten environment.
 *
 * Queues mouse events to be processed on the Audio Worklet thread.
 *
 * @param eventType  The type of mouse event (e.g., click, mousemove).
 * @param e          Pointer to the EmscriptenMouseEvent containing event details.
 * @param userData   Pointer to user-defined data (typically Pd4WebUserData).
 * @return           Returns EM_TRUE if the event was handled, otherwise EM_FALSE.
 */
EM_BOOL MouseListener(int eventType, const EmscriptenMouseEvent *e, void *userData) {
    Pd4WebUserData *ud = (Pd4WebUserData *)userData;

    // Only capture browser data here. Pd state is exclusively accessed by the audio thread.
    double canvasX = JS_CanvasRelativeClientX(ud->canvasSel.c_str(), e->clientX);
    double canvasY = JS_CanvasRelativeClientY(ud->canvasSel.c_str(), e->clientY);
    int xpos = round((canvasX / ud->pd4web->GetPatchZoom()) + PD4WEB_PATCH_MARGINX);
    int ypos = round((canvasY / ud->pd4web->GetPatchZoom()) + PD4WEB_PATCH_MARGINY);

    // Create event data on main thread
    MouseEventData mouseData;
    mouseData.x = xpos;
    mouseData.y = ypos;
    mouseData.button = e->button;
    mouseData.shift = e->shiftKey;
    mouseData.ctrl = e->ctrlKey;
    mouseData.alt = e->altKey;

    switch (eventType) {
    case EMSCRIPTEN_EVENT_MOUSEDOWN:
        mouseData.event_type = MouseEventData::MOUSE_DOWN;
        break;
    case EMSCRIPTEN_EVENT_MOUSEUP:
        mouseData.event_type = MouseEventData::MOUSE_UP;
        ud->pd4web->SetLastMousePosition(xpos, ypos);
        break;
    case EMSCRIPTEN_EVENT_MOUSEMOVE:
        mouseData.event_type = MouseEventData::MOUSE_MOVE;
        break;
    default:
        return EM_FALSE;
    }

    // Queue event for processing on Audio Worklet thread
    Pd4WebSender sender{};
    sender.type = MOUSE_EVENT;
    sender.mouse_data = mouseData;
    ud->pd4web->EnqueueSender(sender);

    return EM_TRUE;
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
    return EM_TRUE;
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
    ud->renderer = std::make_unique<ThorVGRenderer>();
    if (!ud->renderer->initialize(ud->canvasSel, ud->pd4web->GetBGColor(), ud->pd4web->GetFGColor(),
                                  ud->pd4web->GetPatchZoom(), ud->canvas_marginx,
                                  ud->canvas_marginy)) {
        emscripten_log(EM_LOG_ERROR, "ThorVG WebGL initialization failed");
        return;
    }
    GetPatchComments(ud);
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
    m_ProjectName = PD4WEB_PROJECT_NAME;
    m_ChannelCountIn = PD4WEB_CHS_IN;
    m_ChannelCountOut = PD4WEB_CHS_OUT;
    m_SampleRate = PD4WEB_SR;
    m_RenderGui = PD4WEB_GUI;
    m_PatchZoom = PD4WEB_PATCH_ZOOM;
    m_UseMidi = PD4WEB_MIDI;

    m_Fps = 60;

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

    libpd_set_concatenated_printhook(&ReceivedPrintMsg);
    libpd_set_queued_printhook(libpd_print_concatenator);
    libpd_set_queued_banghook(ReceivedBang);
    libpd_set_queued_floathook(ReceivedFloat);
    libpd_set_queued_symbolhook(ReceivedSymbol);
    libpd_set_queued_listhook(ReceivedList);
    libpd_set_queued_messagehook(ReceivedMessage);

    // TODO: Midi messages
    /*
    libpd_set_queued_noteonhook(ReceivedNoteOn);
    libpd_set_queued_controlchangehook(ReceivedControlChange);
    libpd_set_queued_programchangehook(ReceivedProgramChange);
    libpd_set_queued_pitchbendhook(ReceivedPitchBend);
    libpd_set_queued_aftertouchhook(ReceivedAfterTouch);
    libpd_set_queued_polyaftertouchhook(ReceivedPolyAfterTouch);
    libpd_set_queued_midibytehook(ReceivedMIDIByte);
    */

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
    void *nKeyboard = libpd_bind("_pd4web_show_number_keyboard");

    if (!libpd_openfile(PatchPath.c_str(), "./")) {
        JS_Alert("Failed to open patch | Please Report!");
        return;
    }

    // resize canvas
    if (RenderGui() && PatchCanvaId != "") {
        JS_CreateNumberInput(PatchCanvaId.c_str());
        JS_CreateTextInput(PatchCanvaId.c_str());

        t_canvas *canvas = pd_getcanvaslist();
        int canvasWidth = PD4WEB_PATCH_WIDTH > 0 ? PD4WEB_PATCH_WIDTH : canvas->gl_pixwidth;
        int canvasHeight = PD4WEB_PATCH_HEIGHT > 0 ? PD4WEB_PATCH_HEIGHT : canvas->gl_pixheight;

        std::string PatchCanvaSel = "#" + PatchCanvaId;
        const char *sel = PatchCanvaSel.c_str();
        m_UserData->canvasSel = PatchCanvaSel;

        // set patch canvas size
        float zoom = GetPatchZoom();
        float dpr = emscripten_get_device_pixel_ratio();

        // Current viewport
        int winW = EM_ASM_INT({ return window.innerWidth; });
        int winH = EM_ASM_INT({ return window.innerHeight; });

        // If it doesn't fit, compute max zoom that fits
        float zoomW = (float)winW / (float)canvasWidth;
        float zoomH = (float)winH / (float)canvasHeight;
        float maxZoom = fminf(zoomW, zoomH);

        // If current zoom is too large, clamp it
        if (zoom > maxZoom) {
            zoom = maxZoom;
            m_PatchZoom = zoom;
        }

        // Now recompute css/backing after clamping
        float cssW = canvasWidth * zoom;
        float cssH = canvasHeight * zoom;
        int backingW = (int)(cssW * dpr);
        int backingH = (int)(cssH * dpr);

        // Apply
        emscripten_set_canvas_element_size(sel, backingW, backingH);
        emscripten_set_element_css_size(sel, cssW, cssH);

        // clang-format off
        EM_ASM(
            {
                const canvas = document.querySelector(UTF8ToString($0));
                if (canvas) {
                    canvas.style.width = $1 + "px";
                    canvas.style.height = $2 + "px";
                    canvas.addEventListener("mousedown", () => canvas.focus());
                }
            },
            sel, cssW, cssH);
        // clang-format on

        m_UserData->canvas_width = backingW;
        m_UserData->canvas_height = backingH;
        m_UserData->canvas_marginx = PD4WEB_PATCH_MARGINX;
        m_UserData->canvas_marginy = PD4WEB_PATCH_MARGINY;
        m_UserData->devicePixelRatio = dpr;

        for (t_gobj *obj = canvas->gl_list; obj; obj = obj->g_next) {
            gobj_vis(obj, canvas, 1);
            // pd4web guarantees that there will be only one main canvas per patch
            if (strcmp(obj->g_pd->c_name->s_name, "canvas") == 0) {
                t_canvas *child_canvas = (t_canvas *)obj;
                for (t_gobj *childobj = child_canvas->gl_list; childobj;
                     childobj = childobj->g_next) {

                    t_text *t = (t_text *)childobj;
                    int x = t->te_xpix;
                    int y = t->te_ypix;
                    gobj_vis(childobj, child_canvas, 1);
                }
            }
        }

        m_UserData->mousedown = false;
        m_UserData->obj = nullptr;
        emscripten_set_mousedown_callback(sel, m_UserData.get(), EM_TRUE, MouseListener);
        emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, m_UserData.get(), EM_TRUE,
                                        MouseListener);
        emscripten_set_mousemove_callback(sel, m_UserData.get(), EM_TRUE, MouseListener);

        // touchscreen
        emscripten_set_touchstart_callback(sel, m_UserData.get(), EM_TRUE, TouchListener);
        emscripten_set_touchend_callback(sel, m_UserData.get(), EM_TRUE, TouchListener);
        emscripten_set_touchmove_callback(sel, m_UserData.get(), EM_TRUE, TouchListener);
        emscripten_set_touchcancel_callback(sel, m_UserData.get(), EM_TRUE, TouchListener);

        // keydown (lua object must define obj::key_down)
        emscripten_set_keydown_callback(sel, m_UserData.get(), EM_FALSE, KeyListener);
        emscripten_set_keydown_callback("#_pd4web_number_input", m_UserData.get(), EM_FALSE,
                                        KeyListener);
        emscripten_set_keydown_callback("#_pd4web_text_input", m_UserData.get(), EM_FALSE,
                                        KeyListener);

        emscripten_set_orientationchange_callback(m_UserData.get(), EM_FALSE, OrientationListener);

        // TODO: When canvas is on focus
        // emscripten_set_focus_callback(sel, m_UserData.get(), EM_FALSE, FocusListener);

        m_UserData->libpd = m_PdInstance;
        m_UserData->pd4web = this;
        m_UserData->canvasSel = PatchCanvaSel;
        m_UserData->canvasId = soundToggleId;
        emscripten_async_call(setAsyncMainLoop, m_UserData.get(), 0);
    } else {
        m_UserData->libpd = m_PdInstance;
        m_UserData->pd4web = this;
    }
    m_MidiTickID = emscripten_set_interval(MidiTick, 1, m_UserData.get());
}

// ╭─────────────────────────────────────╮
// │                MIDI                 │
// ╰─────────────────────────────────────╯
void MidiTick(void *userData) {
    Pd4WebUserData *ud = static_cast<Pd4WebUserData *>(userData);
    libpd_set_instance(ud->libpd);
    libpd_queued_receive_pd_messages();
    libpd_queued_receive_midi_messages();
}

// ╭─────────────────────────────────────╮
// │            Gui Interface            │
// ╰─────────────────────────────────────╯
void RenderComments(Pd4WebUserData *ud, t_gobj *obj, int x, int y) {
    t_text *txt = (t_text *)obj;
    t_binbuf *bb = txt->te_binbuf;
    if (!bb) {
        return;
    }

    char *textbuf = nullptr;
    int textsize = 0;
    binbuf_gettext(bb, &textbuf, &textsize);

    if (!textbuf || textsize <= 0) {
        return;
    }

    char safe_buf[1025];
    int copy_len = (textsize < 1024) ? textsize : 1024;
    memcpy(safe_buf, textbuf, copy_len);
    safe_buf[copy_len] = '\0';

    GuiCommand cmd{};
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
    const float fontSize = PD4WEB_PATCH_FONTSIZE;
    const float estimatedCharWidth = fontSize * 0.6f;
    if (txt->te_width == 0) {
        cmd.w = std::max(estimatedCharWidth, copy_len * estimatedCharWidth);
    } else {
        cmd.w = txt->te_width * estimatedCharWidth;
    }
    const int estimatedLines = std::max(
        1, static_cast<int>(std::ceil((copy_len * estimatedCharWidth) / std::max(1.0f, cmd.w))));
    cmd.h = estimatedLines * fontSize * 1.2f;
    cmd.objw = cmd.w;
    cmd.objh = cmd.h;

    cmd.font_size = PD4WEB_PATCH_FONTSIZE;
    cmd.objx = txt->te_xpix - x;
    cmd.objy = txt->te_ypix - y;

    const ObjectId objectId = AllocateRenderObjectId();
    ClearLayerCommand(objectId, 0, cmd.objx, cmd.objy, cmd.w, cmd.h);
    AddNewCommand(objectId, 0, &cmd);
    EndPaintLayerCommand(objectId, 0);
}

// ─────────────────────────────────────
void GetPatchComments(Pd4WebUserData *ud) {
    t_canvas *canvas = pd_getcanvaslist();
    if (!canvas) {
        return;
    }

    for (t_gobj *obj = canvas->gl_list; obj; obj = obj->g_next) {
        if (obj->g_pd && obj->g_pd->c_name && strcmp(obj->g_pd->c_name->s_name, "text") == 0) {
            RenderComments(ud, obj, 0, 0);
        }
        if (strcmp(obj->g_pd->c_name->s_name, "canvas") == 0) {
            t_canvas *child_canvas = (t_canvas *)obj;
            int x = child_canvas->gl_xmargin;
            int y = child_canvas->gl_ymargin;
            for (t_gobj *childobj = child_canvas->gl_list; childobj; childobj = childobj->g_next) {
                t_text *t = (t_text *)childobj;
                if (childobj->g_pd && childobj->g_pd->c_name &&
                    strcmp(childobj->g_pd->c_name->s_name, "text") == 0) {
                    RenderComments(ud, childobj, x, y);
                }
            }
        }
    }
}

// ─────────────────────────────────────

extern "C" uint64_t AllocateRenderObjectIdC(void) {
    return AllocateRenderObjectId();
}

extern "C" void ClearLayerCommand(uint64_t objectId, int layer, int x, int y, int w, int h) {
    GetRenderTransport().beginLayer(objectId, layer, x, y, w, h);
}

extern "C" void AddNewCommand(uint64_t, int, GuiCommand *command) {
    if (!command) {
        return;
    }
    GetRenderTransport().append(*command);
}

extern "C" void EndPaintLayerCommand(uint64_t, int) {
    GetRenderTransport().publishLayer();
}

extern "C" void RemoveRenderLayer(uint64_t objectId, int layer) {
    GetRenderTransport().publishLifecycle(RenderMessageType::RemoveLayer, objectId, layer);
}

extern "C" void RemoveRenderObject(uint64_t objectId) {
    GetRenderTransport().publishLifecycle(RenderMessageType::RemoveObject, objectId);
}

extern "C" void UpdateRenderObject(uint64_t objectId, int x, int y, int w, int h) {
    GetRenderTransport().publishObjectUpdate(objectId, x, y, w, h);
}

extern "C" void ClearRenderPatch(void) {
    GetRenderTransport().publishLifecycle(RenderMessageType::ClearPatch, 0);
}

extern "C" int TakeRenderRecovery(uint64_t *objectId, int *layer) {
    if (!objectId || !layer) {
        return 0;
    }
    ObjectId id = 0;
    int layerIndex = -1;
    if (!GetRenderTransport().takeRecovery(id, layerIndex)) {
        return 0;
    }
    *objectId = id;
    *layer = layerIndex;
    return 1;
}

void Loop(void *userData) {
    auto *ud = static_cast<Pd4WebUserData *>(userData);
    libpd_set_instance(ud->libpd);
    libpd_queued_receive_pd_messages();
    libpd_queued_receive_midi_messages();

    if (!ud->soundInit) {
        const double now = emscripten_get_now();
        const double elapsed = now - ud->lastFrame;
        ud->lastFrame = now;
        const int ticks =
            static_cast<int>((elapsed / 1000.0) * (ud->pd4web->GetSampleRate() / 64.0f));
        if (ticks > 0) {
            libpd_process_float(ticks, nullptr, nullptr);
        }
    }

    if (ud->renderer) {
        ud->renderer->setZoom(ud->pd4web->GetPatchZoom());
        ud->renderer->poll();
    }
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
    if (UseMidi()) {
        SetupMIDI();
    }

    EmscriptenWebAudioCreateAttributes attrs = {
        .latencyHint = "interactive",
        .sampleRate = static_cast<uint32_t>(m_SampleRate),
    };

    // Midi tick
    int MidiTickId = emscripten_set_interval(MidiTick, 13, nullptr);

    // Start the audio context
    static uint8_t WasmAudioWorkletStack[1024 * 1024];
    m_AudioContext = emscripten_create_audio_context(&attrs);
    m_UserData->soundInit = true;

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

    libpd_set_printhook(libpd_print_concatenator);
    libpd_set_concatenated_printhook(&ReceivedPrintMsg);

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
