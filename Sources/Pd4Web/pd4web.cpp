#include "pd4web.hpp"

#include <emscripten/webaudio.h>
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
    // LOG("Pd4Web::receivedBang");
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
    // LOG("Pd4Web::receivedFloat");
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
    // LOG("Pd4Web::receivedSymbol");
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
    // LOG("Pd4Web::receivedList");
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
    // LOG("Pd4Web::receivedMessage");
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
    // LOG("Pd4Web::sendSymbol");

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
void Pd4Web::bindReceiver(std::string s) {
    // LOG("Pd4Web::bindReceiver");
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

    Pd4WebUserData *userData = (Pd4WebUserData *)data;
    libpd_set_instance(userData->libpd);

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
    Pd4WebUserData *ud = (Pd4WebUserData *)userData;
    libpd_set_instance(ud->libpd);

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
    // LOG("Pd4Web::audioWorkletInit");
    if (!success) {
        _JS_alert("WebAudio worklet thread initialization failed!\n");
        return;
    }

    WebAudioWorkletProcessorCreateOptions opts = {
        .name = "pd4web",
    };
    Pd4WebUserData *ud = (Pd4WebUserData *)userData;
    libpd_set_instance(ud->libpd);

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
            // _JS_setIcon("--sound-on", "");
        }
    } else {
        suspendAudio();
        m_audioSuspended = true;
        if (PD4WEB_GUI) {
            // _JS_setIcon("--sound-off", "");
        }
    }
}

// ╭─────────────────────────────────────╮
// │            Init Function            │
// ╰─────────────────────────────────────╯
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

// ──────────────────────────────────────────
void Pd4Web::init() {
    PD4WEB_INSTANCES++;
    // LOG("Pd4Web::init");

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
    emscripten_start_wasm_audio_worklet_thread_async(AudioContext, WasmAudioWorkletStack,
                                                     sizeof(WasmAudioWorkletStack),
                                                     Pd4Web::audioWorkletInit, (void *)userData);
    m_Context = AudioContext;

    // After load, it defines some extra functions
    _JS_sendList();
    _JS_onReceived();

    // Bind the receivers
    // bindGuiReceivers();

    if (PD4WEB_GUI) {
        // _JS_setIcon("--sound-on", "");
        // _JS_addSoundToggle();
    }
    m_audioSuspended = false;
    if (PD4WEB_MIDI) {
        _JS_post("Loading Midi");
        // clang-format off
        EM_ASM({
            function onMIDISuccess(midiAccess) {
                for (let input of midiAccess.inputs) {
                    input[1].onmidimessage = (message) => {
                        const data = message.data;
                        Pd4Web._midibyte(data[0], data[1], data[2]);
                    };
                }
            }

            function onMIDIFailure() {
                console.error("Failed to access MIDI devices.");
            }

            if (navigator.requestMIDIAccess) {
                console.log("requestMIDIAccess");
                navigator.requestMIDIAccess().then(onMIDISuccess, onMIDIFailure);
            } else {
                alert("Web MIDI API is not supported.");
            }
        });
        // clang-format on
    }
}

// ╭─────────────────────────────────────╮
// │             USER EVENTS             │
// ╰─────────────────────────────────────╯
void pd4web_queue_mouseclick(t_pd *obj, void *data) {
    Pd4WebUserData *d = (Pd4WebUserData *)data;
    (void)gobj_click(d->obj, d->canvas, d->xpos, d->ypos, 0, 0, 0, d->doit);
    delete d;
}

// ─────────────────────────────────────
EM_BOOL mouse_listener(int eventType, const EmscriptenMouseEvent *e, void *userData) {
    Pd4WebUserData *data = (Pd4WebUserData *)userData;
    libpd_set_instance(data->libpd);

    t_canvas *canvas = pd_getcanvaslist();
    if (!canvas)
        return EM_TRUE;

    int xpos = round((float)e->targetX / PD4WEB_PATCH_ZOOM);
    int ypos = round((float)e->targetY / PD4WEB_PATCH_ZOOM);
    // int shift = e->shiftKey;
    // int alt = e->altKey;

    data->xpos = xpos;
    data->ypos = ypos;
    data->canvas = canvas;
    data->doit = false;

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
        if (data->mousedown) {
            data->doit = true;
        } else {
            data->doit = false;
        }
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
            pd_queue_mess(data->libpd, &obj->g_pd, (void *)d, pd4web_queue_mouseclick);
            // pd4web_queue_mouseclick(&obj->g_pd, d);
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
    emscripten_set_main_loop_arg(mainLoop, (void *)userData, 0, 1);
}

// ─────────────────────────────────────
void Pd4Web::openPatch(std::string PatchPath, std::string PatchCanvaId, std::string soundToggleId) {
    m_NewPdInstance = libpd_new_instance();

    if (m_NewPdInstance == NULL) {
        _JS_alert("libpd_init() failed, please report!");
        return;
    }
    libpd_set_instance(m_NewPdInstance);

    if (soundToggleId != "") {
        EM_ASM(
            {
                const el = document.getElementById(UTF8ToString($1));
                el.style.backgroundImage = "url(" + UTF8ToString($0) + ")";
                el.classList.add("pulse-icon");
            },
            ICON_SOUND_OFF, soundToggleId.c_str());

        std::string selector = "#" + soundToggleId;
        Pd4WebUserData *sound_toggle_data = new Pd4WebUserData();
        sound_toggle_data->soundInit = false;
        sound_toggle_data->soundSuspended = false;
        sound_toggle_data->pd4web = this;
        sound_toggle_data->soundToggleSel = soundToggleId;
        emscripten_set_mousedown_callback(selector.c_str(), (void *)sound_toggle_data, EM_FALSE,
                                          sound_toggle);
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
    if (!libpd_openfile(PatchPath.c_str(), "./")) {
        _JS_alert("Failed to open patch | Please Report!\n");
        return;
    }

    // resize canvas
    t_canvas *canvas = pd_getcanvaslist();
    int canvasWidth = canvas->gl_pixwidth;
    int canvasHeight = canvas->gl_pixheight;
    if (canvasWidth == 0 && canvasHeight == 0) {
        canvasWidth = canvas->gl_screenx2 - canvas->gl_screenx1;
        canvasHeight = canvas->gl_screeny2 - canvas->gl_screeny1;
    }

    std::string PatchCanvaSel = "#" + PatchCanvaId;

    emscripten_set_canvas_element_size(PatchCanvaSel.c_str(), canvasWidth * PD4WEB_PATCH_ZOOM,
                                       canvasHeight * PD4WEB_PATCH_ZOOM);

    // BUG: init the first paint, for some reason, the object is nill on libpd_openfile, not
    // sure if this is a bug
    if (PD4WEB_GUI) {
        t_canvas *canvas = pd_getcanvaslist();
        Pd4WebUserData *mouse_data = new Pd4WebUserData();
        for (t_gobj *obj = canvas->gl_list; obj; obj = obj->g_next) {
            const struct _widgetbehavior *c_wb;
            c_wb = obj->g_pd->c_wb;
            t_visfn w_visfn = c_wb->w_visfn;
            if (w_visfn) {
                w_visfn(obj, canvas->gl_owner, 1);
            }
        }
        mouse_data->libpd = libpd_this_instance();
        mouse_data->mousedown = false;
        emscripten_set_mousedown_callback(PatchCanvaSel.c_str(), (void *)mouse_data, EM_FALSE,
                                          mouse_listener);
        emscripten_set_mouseup_callback(PatchCanvaSel.c_str(), (void *)mouse_data, EM_FALSE,
                                        mouse_listener);
        emscripten_set_mousemove_callback(PatchCanvaSel.c_str(), (void *)mouse_data, EM_FALSE,
                                          mouse_listener);
    }

    Pd4WebUserData *mainLoopData = new Pd4WebUserData();
    mainLoopData->libpd = libpd_this_instance();
    mainLoopData->pd4web = this;
    emscripten_async_call(setAsyncMainLoop, mainLoopData, 20);
}

// ─────────────────────────────────────
void mainLoop(void *userData) {
    Pd4WebUserData *ud = (Pd4WebUserData *)userData;
    libpd_set_instance(ud->libpd);

#if PD4WEB_LUA
    pd4weblua_draw();
#endif

    // for (auto &GuiReceiver : Pd4WebGuiReceivers) {
    //     if (GuiReceiver.Updated) {
    //         switch (GuiReceiver.Type) {
    //         case Pd4WebGuiConnector::BANG: {
    //             _JS_receiveBang(GuiReceiver.Receiver.c_str());
    //             break;
    //         }
    //         case Pd4WebGuiConnector::FLOAT: {
    //             _JS_receiveFloat(GuiReceiver.Receiver.c_str(), GuiReceiver.Float);
    //             break;
    //         }
    //         case Pd4WebGuiConnector::SYMBOL: {
    //             _JS_receiveSymbol(GuiReceiver.Receiver.c_str(), GuiReceiver.Symbol.c_str());
    //             break;
    //         }
    //         case Pd4WebGuiConnector::LIST: {
    //             _JS_receiveList(GuiReceiver.Receiver.c_str());
    //             break;
    //         }
    //         case Pd4WebGuiConnector::MESSAGE: {
    //             _JS_receiveMessage(GuiReceiver.Receiver.c_str());
    //         }
    //         }
    //         GuiReceiver.Updated = false;
    //     }
    // }
}

// ╭─────────────────────────────────────╮
// │            Main Function            │
// ╰─────────────────────────────────────╯
int main() {
    emscripten_set_window_title(PD4WEB_PROJECT_NAME);
    libpd_set_printhook(libpd_print_concatenator);
    libpd_set_concatenated_printhook(&Pd4Web::post);
    printf("pd4web version %s.%s.%s\n", PD4WEB_VERSION_MAJOR, PD4WEB_VERSION_MINOR,
           PD4WEB_VERSION_PATCH);
    libpd_init();
    return 0;
}
