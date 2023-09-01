#include <emscripten/webaudio.h> // WebAudio API
#include <z_libpd.h> // libpd
#include <assert.h> // assert

// Externals Objects Declarations

// ====================

uint8_t patchAudioInputs = 1;
uint8_t patchAudioOutputs = 2;
uint8_t wasmAudioWorkletStack[1024 * 1024];
int samplerate = 48000;


// ==============================================
EMSCRIPTEN_KEEPALIVE
void* webpd_malloc(int size) {
    return malloc(size);
}

EMSCRIPTEN_KEEPALIVE
void webpd_free(void *ptr) {
    free(ptr);
}


// Send data to PureData
// ==============================================
EMSCRIPTEN_KEEPALIVE
int sendFloatToPd(const char *receiver, float value) {
    return libpd_float(receiver, value);
}

// ==============================================
EMSCRIPTEN_KEEPALIVE
int sendBangToPd(const char *receiver, float value) {
    return libpd_bang(receiver);
}



// ==============================================
void pdprint(const char *s) {
    printf("%s", s);
}

// ==============================================
void sys_gui_midipreferences(void) {
    return;
}

// ==============================================
EM_JS(void, JS_print, (const char *s, double d), {
    console.log(UTF8ToString(s) + " " + d);
});

// ========================================
EM_BOOL ProcessPdPatch(int numInputs, const AudioSampleFrame *inputs, int numOutputs, 
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

    // create button to print the audio context state
    const printButton = document.getElementById("Print-Audio-Context-Button");
    printButton.onclick = () => {
        console.log("AudioContext state: " + audioContext.state);
    };

    const inputDeviceSelect = document.getElementById("Input-Device-Select");
    inputDeviceSelect.addEventListener("change", async () => {
        const iconElement = document.getElementById("SoundIcon");
        iconElement.classList.remove("fa-volume-high");
        iconElement.classList.add("fa-volume-xmark");
        if(outputDeviceSelect.value === "none" || outputDeviceSelect.value === "Default" || outputDeviceSelect.value === "default") {
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
        if(outputDeviceSelect.value === "none" || outputDeviceSelect.value === "Default" || outputDeviceSelect.value === "default") {
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
        const AudioNoteLatency = document.getElementById("Latency-AudioNode");
        const Latency_Output = document.getElementById("Latency-Output");
        AudioNoteLatency.innerHTML = "Latency for AudioProcessing " + Math.floor(audioContext.baseLatency * 1000) + " ms";
        Latency_Output.innerHTML = "Latency for Output " + Math.floor(audioContext.outputLatency * 1000) + " ms";
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
void pdnoteon(int ch, int pitch, int vel) {
  printf("noteon: %d %d %d\n", ch, pitch, vel);
}


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

    
    libpd_set_printhook(pdprint);
    libpd_set_noteonhook(pdnoteon);
    libpd_init();
    
    // WebPd Load Externals

    // ====================

    libpd_start_message(1); 
    libpd_add_float(1.0f);
    libpd_finish_message("pd", "dsp");

    libpd_init_audio(patchAudioInputs, patchAudioOutputs, GetAudioSampleRate(audioContext));
    if (!libpd_openfile("index.pd", "webpatch/data")){
        printf("Failed to open patch\n");
    }
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

}
