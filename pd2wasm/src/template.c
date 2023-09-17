#include <emscripten/webaudio.h> // WebAudio API
#include <z_libpd.h> // libpd
#include <assert.h> // assert

// Externals Objects Declarations

// ====================

// =====================================
// ========== AUDIO CONFIG =============
// =====================================
uint8_t patchAudioInputs = 1;
uint8_t patchAudioOutputs = 2;
uint8_t wasmAudioWorkletStack[1024 * 1024];
int samplerate = 48000;

// =====================================
// ================ GUI ================
// =====================================
pthread_mutex_t WriteReadMutex = PTHREAD_MUTEX_INITIALIZER;

char* HTML_IDS[] = {}; // Add this to the WebPdAssembler
int HTML_IDS_SIZE = 0; // Add this to the WebPdAssembler

typedef struct pdItem{
    const char*     receiverID;
    float           f;
    const char*     s;
    int             type;
    int             changed;
} pdItem;

// ====================
typedef struct pdItemHash{
    pdItem** items;
    int size;
    int count;
} pdItemHash;

struct pdItemHash *receiverHash;
// IF THERE IS MORE THAN 32 VALUES, INCREASE THIS VALUE
int pdWebValueArraySize = 32;

// ====================
static pdItemHash* CreatePdItemHash(int size) { 
    pdItemHash* hash_table = (pdItemHash*)malloc(sizeof(pdItemHash));
    hash_table->size = size;
    hash_table->count = 0;
    hash_table->items = (pdItem**)calloc(size, sizeof(pdItem*));
    return hash_table;
}

// ====================
static unsigned int HashFunction(pdItemHash* hash_table, const char* key) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % hash_table->size;
}

// ====================
static void InsertFloat(pdItemHash* hash_table, const char* key, float f) { 
    unsigned int index = HashFunction(hash_table, key);
    pthread_mutex_lock(&WriteReadMutex); // Lock the mutex
    pdItem* item = hash_table->items[index];
    if (item == NULL &&  hash_table->count <= hash_table->size) {
        item = (pdItem*)malloc(sizeof(pdItem));
        item->receiverID = strdup(key);
        item->f = f;
        item->type = A_FLOAT;
        item->changed = 1;
        hash_table->items[index] = item;
        hash_table->count++; 
    }
    else if (hash_table->count > hash_table->size) {
        EM_ASM_({
            alert("Hash table is full");
        });
    }
    else if (item != NULL) {
        item->f = f;
        item->changed = 1;
        item->type = A_FLOAT;
    }
    pthread_mutex_unlock(&WriteReadMutex); // Unlock the mutex
    return;
}

// ====================
static void InsertSymbol(pdItemHash* hash_table, const char* key, const char* thing) { 
    unsigned int index = HashFunction(hash_table, key);
    pthread_mutex_lock(&WriteReadMutex); // Lock the mutex
    pdItem* item = hash_table->items[index];
    if (item == NULL &&  hash_table->count <= hash_table->size) {
        item = (pdItem*)malloc(sizeof(pdItem));
        item->receiverID = strdup(key);
        item->s = strdup(thing);
        item->type = A_SYMBOL;
        item->changed = 1;
        hash_table->items[index] = item;
        hash_table->count++; 
    }
    else if (hash_table->count > hash_table->size) {
        EM_ASM_({
            alert("Hash table is full");
        });
    }
    else if (item != NULL) {
        item->s = strdup(thing);
        item->changed = 1;
        item->type = A_SYMBOL;
    }
    pthread_mutex_unlock(&WriteReadMutex); // Unlock the mutex
    return;
}

// ====================
static pdItem *GetItem(pdItemHash* hash_table, char* key) {
    unsigned int index = HashFunction(hash_table, key);
    pthread_mutex_lock(&WriteReadMutex); // Lock the mutex

    pdItem* item = hash_table->items[index];
    if (item != NULL) {
        pthread_mutex_unlock(&WriteReadMutex); // Unlock the mutex
        return item;
    }
    pthread_mutex_unlock(&WriteReadMutex); // Unlock the mutex
    return NULL;
}

// =====================================
// ============= HELPERS ===============
// =====================================

EMSCRIPTEN_KEEPALIVE
void* webpd_malloc(int size) {
    return malloc(size);
}

// ======================================
EMSCRIPTEN_KEEPALIVE
void webpd_free(void *ptr) {
    free(ptr);
}

// ======================================
EMSCRIPTEN_KEEPALIVE
int sendFloatToPd(const char *receiver, float value) {
    return libpd_float(receiver, value);
}

// ======================================
EMSCRIPTEN_KEEPALIVE
int sendBangToPd(const char *receiver, float value) {
    return libpd_bang(receiver);
}

// ======================================
// ============= libpd HOOKS ============
// ======================================

void pdprint(const char *s) {
    if (s[0] == '\n') {
        return;
    }
    EM_ASM_({
        console.log(UTF8ToString($0));
    }, s);
}

// ========================================
void receiveFloatfromPd(const char *receiver, float value) {    
    InsertFloat(receiverHash, (char*)receiver, value);

}

// ========================================
static void receiveSymbolfromPd(const char *receiver, const char *thing) {
    InsertSymbol(receiverHash, (char*)receiver, (char*)thing);
}

// ========================================
// to remove warning about not defined
void sys_gui_midipreferences(void) {
    return;
}

// ========================================
// ============= WEB AUDIO ================
// ========================================
static EM_BOOL ProcessPdPatch(int numInputs, const AudioSampleFrame *inputs, int numOutputs, 
                AudioSampleFrame *outputs, int numParams, const AudioParamFrame *params, void *userData){

    int outCh = outputs[0].numberOfChannels;
    float tmpOutputs[128 * 2]; 
    libpd_process_float(2, inputs[0].data, tmpOutputs);
    int outputIndex = 0;
    for (int i = 0; i < outCh; i++) {
        for (int j = i; j < (128 * outCh); j += 2) {
            outputs[0].data[outputIndex] = tmpOutputs[j];
            outputIndex++;
        }
    }
	return EM_TRUE;
}

// ========================================
EM_JS(int, GetAudioSampleRate, (EMSCRIPTEN_WEBAUDIO_T audioContext), {
    return emscriptenGetAudioObject(audioContext).sampleRate;
});


// ========================================
EM_JS(void, LoadFinished, (void), {
    var soundIcon = document.getElementById("SoundIcon");
    soundIcon.className = "fa-solid fa-volume-xmark fa-2x";
});

// ========================================
EM_JS(void, AddUIButtons, (EMSCRIPTEN_WEBAUDIO_T audioContext, EMSCRIPTEN_AUDIO_WORKLET_NODE_T audioWorkletNode), {
    audioContext = emscriptenGetAudioObject(audioContext);
    audioWorkletNode = emscriptenGetAudioObject(audioWorkletNode);
    if (audioContext.state === "running") {
        audioContext.suspend();
    }
    const startButton = document.getElementById("Start-Audio-Button");
    startButton.onclick = () => {
        const iconElement = document.getElementById("SoundIcon");
        if (iconElement.classList.contains("fa-volume-xmark")) {
            iconElement.classList.remove("fa-volume-xmark");
            iconElement.classList.add("fa-volume-high");
            if (audioContext.state === "suspended")
                audioContext.resume();
            audioWorkletNode.connect(audioContext.destination);
        } 
        else {
            iconElement.classList.remove("fa-volume-high");
            iconElement.classList.add("fa-volume-xmark");
            if (audioContext.state === "running")
                audioContext.suspend();
        }
    };


    audioWorkletNode.onprocessorerror = (event) => {
        alert(event);
        audioContext.suspend();
    };


    const inputDeviceSelect = document.getElementById("Input-Device-Select");
    inputDeviceSelect.addEventListener("change", async () => {
        const iconElement = document.getElementById("SoundIcon");
        iconElement.classList.remove("fa-volume-high");
        iconElement.classList.add("fa-volume-xmark");
        if(outputDeviceSelect.value === "none" || outputDeviceSelect.value === "Default" 
        || outputDeviceSelect.value === "default") {
            if (audioContext.state === "running")
                audioContext.suspend();
        } 
        else {
            console.log("AudioContext state: " + audioContext.state);
            if (audioContext.state === "running")
                audioContext.suspend();
        }
    });
    const outputDeviceSelect = document.getElementById("Output-Device-Select");
    outputDeviceSelect.addEventListener("change", async () => {
        if(outputDeviceSelect.value === "none" || outputDeviceSelect.value === "Default" 
        || outputDeviceSelect.value === "default") {
            await audioContext.setSinkId({ type : "none" }).then(() => {
                console.log("Output device: " + outputDeviceSelect.value);
                console.log("AudioContext state: " + audioContext.state);
            });
            
        }
        else {
            await audioContext.setSinkId(outputDeviceSelect.value).then(() => {
                console.log("Output device: " + outputDeviceSelect.value);
                console.log("AudioContext state: " + audioContext.state);
            });
        }
    });
    var startButtonMic = document.getElementById("Start-Audio-Button");
    async function init(stream){
        if ("setSinkId" in AudioContext.prototype) {
            const devices = await navigator.mediaDevices.enumerateDevices();
            const buttom = document.getElementById("Output-Device-Select");
            devices.forEach(function(device) {
                if (device.kind === "audiooutput" && device.deviceId !== "default") {
                    var option = document.createElement("option");
                    option.value = device.deviceId;
                    option.text = device.label;
                    // buttom.appendChild(option);
                }
            });
        }
        else {
            alert("Your browser not support AudioContext.setSinkId(), use Brave, Chrome or Edge instead.");
        }

        if ("setSinkId" in AudioContext.prototype) {
            const devices = await navigator.mediaDevices.enumerateDevices();
            const buttom = document.getElementById("Input-Device-Select");
            devices.forEach(function(device) {
                if (device.kind === "audioinput" && device.deviceId !== "default") {
                        var option = document.createElement("option");
                        option.value = device.deviceId;
                        option.text = device.label;
                        // buttom.appendChild(option);
                }
            });
        }
        const mic = audioContext.createMediaStreamSource(stream);
        const clickListenerMic = (event) => {
            if (audioContext.state !== "running") {
                mic.connect(audioWorkletNode);
                startButtonMic.removeEventListener("click", clickListenerMic);
            }     
            else {
               audioContext.suspend();
            }
        };
        startButtonMic.addEventListener("click", clickListenerMic);
    }
    
    
    navigator.mediaDevices.getUserMedia({
                                        video: false,
                                        audio: 
                                            {
                                                echoCancellation: false, 
                                                noiseSuppression: false, 
                                                autoGainControl: false,
                                                channelCount: 1
                                            }
                                        })
        .then((stream) => init(stream));


});

// ========================================
void AudioWorkletProcessorCreated(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData){
    if (!success){
        return;
    }
    int outputChannelCounts[1] = {2}; // Stereo output
    EmscriptenAudioWorkletNodeCreateOptions options = {
        .numberOfInputs = 1,
        .numberOfOutputs = 1,
        .outputChannelCounts = outputChannelCounts,
    };

    EMSCRIPTEN_AUDIO_WORKLET_NODE_T wasmAudioWorklet = emscripten_create_wasm_audio_worklet_node(audioContext, 
                                                            "libpd-processor", &options, &ProcessPdPatch, 0); 
    AddUIButtons(audioContext, wasmAudioWorklet);
    printf("PdWebCompiler version 1.0.8");


    libpd_set_floathook(receiveFloatfromPd);
    libpd_set_symbolhook(receiveSymbolfromPd);


    libpd_set_printhook(pdprint);
    libpd_init();
    
    // WebPd Load Externals

    // ====================
    
    for (int i = 0; i < HTML_IDS_SIZE; i++){
        libpd_bind(HTML_IDS[i]);
    }

    // add webpatch/data to the search path
    libpd_add_to_search_path("webpatch/data");

    libpd_start_message(1); 
    libpd_add_float(1.0f);
    libpd_finish_message("pd", "dsp");

    libpd_init_audio(patchAudioInputs, patchAudioOutputs, GetAudioSampleRate(audioContext));
    if (!libpd_openfile("index.pd", "webpatch/data")){
        printf("Failed to open patch\n");
    }

    



    // Add sound off button
    LoadFinished();
}

// ========================================
void WebAudioWorkletThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData){
    if (!success){
        return;
    }
    WebAudioWorkletProcessorCreateOptions opts = {
        .name = "libpd-processor",
    };
    emscripten_create_wasm_audio_worklet_processor_async(audioContext, &opts, AudioWorkletProcessorCreated, 0);
}

// ========================================
EM_JS(void, setFloatValue, (const char* symbol, float value), {
    var symbolId = UTF8ToString(symbol);
    var element = document.getElementById(symbolId); // Find the element by ID

    if (element === null) {
        var myElement = document.createElement("input");
        myElement.id = symbolId;
        myElement.value = value;
        myElement.style.display = "none";
        document.body.appendChild(myElement);
    } 
    else {
        element.value = value;
    }
});

// ========================================
EM_JS(void, setSymbolValue, (const char* symbol, const char* value), {
    var symbolId = UTF8ToString(symbol);
    var element = document.getElementById(symbolId); // Find the element by ID

    if (element === null) {
        var myElement = document.createElement("input");
        myElement.id = symbolId;
        myElement.value = UTF8ToString(value);
        myElement.style.display = "none";
        document.body.appendChild(myElement);
    } 
    else {
        element.value = UTF8ToString(value);
    }
});

// ========================================
void PdWebCompiler_Loop(){
    for (int i = 0; i < HTML_IDS_SIZE; i++){
        pdItem* item = GetItem(receiverHash, HTML_IDS[i]);
        if (item == NULL) {
            continue;
        }
        
        if (item->changed){
            item->changed = 0;
            if (item->type == A_FLOAT){
                setFloatValue(HTML_IDS[i], item->f);
            }
            else if(item->type == A_SYMBOL){
                setSymbolValue(HTML_IDS[i], item->s); 
            }
            else{
                return;
            }
        }
    }
}

// ========================================
int main(){
    
    srand(time(NULL));
    assert(!emscripten_current_thread_is_audio_worklet());

    EmscriptenWebAudioCreateAttributes attrs = {
            .latencyHint = "interactive",
    };

    EMSCRIPTEN_WEBAUDIO_T context = emscripten_create_audio_context(&attrs);

    samplerate = GetAudioSampleRate(context);

    emscripten_start_wasm_audio_worklet_thread_async(context, wasmAudioWorkletStack, 
        sizeof(wasmAudioWorkletStack), WebAudioWorkletThreadInitialized, 0);


    receiverHash = CreatePdItemHash(pdWebValueArraySize);

    emscripten_set_main_loop(PdWebCompiler_Loop, 30, 1);


}
