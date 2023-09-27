function JS_AddUIButtons(audioContext, audioWorkletNode) {
    if (audioContext.state === "running") {
            audioContext.suspend();
        }
        const startButton = document.getElementById("Start-Audio-Button");
        startButton.onclick = () => {
            const iconElement = document.getElementById("SoundIcon");
            if (iconElement.classList.contains("fa-volume-xmark")) {
                iconElement.classList.remove("fa-volume-xmark");
                iconElement.classList.add("fa-volume-high");
                if (iconElement.classList.contains("fa-bounce")){
                    iconElement.classList.remove("fa-bounce");
                }

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
            iconElement.classList.add("fa-bounce");
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
                console.log("Your browser not support AudioContext.setSinkId(), use Brave, Chrome or Edge instead.");
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
}


// ====================
function JS_LoadFinished() {
    var soundIcon = document.getElementById("SoundIcon");
    soundIcon.className = "fa-solid fa-volume-xmark fa-2x";
}


// ====================
function sendFloat(receiver, f) {
    if (Module === undefined) {
        alert("Module is undefined!");
        return;
    }
    var str_rawReceiver = new TextEncoder().encode(receiver);
    var ptrReceiver = Module._webpd_malloc(str_rawReceiver.length + 1);
    var chunkReceiver = Module.HEAPU8.subarray(ptrReceiver, ptrReceiver + str_rawReceiver.length);
    chunkReceiver.set(str_rawReceiver);
    Module.HEAPU8[ptrReceiver + str_rawReceiver.length] = 0; // Null-terminate the string

    if(Module._sendFloatToPd(ptrReceiver, f) !== 0){
        console.error("Error sending float to pd");
    }
    Module._webpd_free(ptrReceiver);
}

// ====================
function sendBang(receiver) {
    if (Module === undefined) {
        alert("Module is undefined!");
        return;
    }
    var str_rawReceiver = new TextEncoder().encode(receiver);
    var ptrReceiver = Module._webpd_malloc(str_rawReceiver.length + 1);
    var chunkReceiver = Module.HEAPU8.subarray(ptrReceiver, ptrReceiver + str_rawReceiver.length);
    chunkReceiver.set(str_rawReceiver);
    Module.HEAPU8[ptrReceiver + str_rawReceiver.length] = 0; // Null-terminate the string
    if(Module._sendBangToPd(ptrReceiver) !== 0){
        console.error("Error sending float to pd");
    }
    Module._webpd_free(ptrReceiver);
}

// ====================
function sendString(receiver, str){
    if (pdIsInitialized === false) {
        console.log("Pd is not initialized yet!");
        return;
    }
    var str_rawReceiver = new TextEncoder().encode(receiver);
    var ptrReceiver = Module._webpd_malloc(str_rawReceiver.length + 1);
    var chunkReceiver = Module.HEAPU8.subarray(ptrReceiver, ptrReceiver + str_rawReceiver.length);
    chunkReceiver.set(str_rawReceiver);
    Module.HEAPU8[ptrReceiver + str_rawReceiver.length] = 0; // Null-terminate the string

    var str_rawThing = new TextEncoder().encode(str);
    var ptrThing = Module._webpd_malloc(str_rawThing.length + 1);
    var chunkReceiver = Module.HEAPU8.subarray(ptrThing, ptrThing + str_rawThing.length);
    chunkReceiver.set(str_rawThing);
    Module.HEAPU8[ptrThing + str_rawThing.length] = 0; // Null-terminate the string

    var result = Module._pd_sendSymbol(ptrReceiver, ptrThing);

    Module._webpd_free(ptrReceiver);
    Module._webpd_free(ptrThing);
    
    if (result !== 0) {
        console.error("Error sending float to pd");
    }
}



