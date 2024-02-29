var pdIsInitialized = false;
window.pd4webGuiValues = {};

function JS_AddUIButtons(audioContext, audioWorkletNode) {
  console.log("Latency: " + parseInt(audioContext.baseLatency * 1000));

  if (audioContext.state === "running") {
    audioContext.suspend();
  }
  const startButton = document.getElementById("Start-Audio-Button");
  if (startButton !== null) {
    startButton.onclick = () => {
      const iconElement = document.getElementById("SoundIcon");
      if (iconElement.classList.contains("fa-volume-xmark")) {
        for (var i = 0; i < iconElement.classList.length; i++) {
          iconElement.classList.remove(iconElement.classList[i]);
        }
        iconElement.className = "fa-solid fa-volume-high fa-2x";
        if (audioContext.state === "suspended") {
          audioContext.resume();
        }
        audioWorkletNode.connect(audioContext.destination);
      } else {
        // sound is on
        iconElement.className = "fa-solid fa-volume-xmark fa-2x";
        if (audioContext.state === "running") {
          audioContext.suspend();
        }
      }
    };
    audioWorkletNode.onprocessorerror = (event) => {
      alert(event);
      audioContext.suspend();
    };
  }
  async function init(stream) {
    if ("setSinkId" in AudioContext.prototype) {
      const devices = await navigator.mediaDevices.enumerateDevices();
      devices.forEach(function (device) {
        if (device.kind === "audiooutput" && device.deviceId !== "default") {
          var option = document.createElement("option");
          option.value = device.deviceId;
          option.text = device.label;
        }
      });
    } else {
      console.log(
        "Your browser not support AudioContext.setSinkId(), use Brave, Chrome or Edge instead.",
      );
    }

    if ("setSinkId" in AudioContext.prototype) {
      // const devices = await navigator.mediaDevices.enumerateDevices();
      // const buttom = document.getElementById("Input-Device-Select");
      // devices.forEach(function(device) {
      // if (device.kind === "audioinput" && device.deviceId !== "default") {
      // var option = document.createElement("option");
      // option.value = device.deviceId;
      // option.text = device.label;
      // buttom.appendChild(option);
      // }
      // });
    }
    const mic = audioContext.createMediaStreamSource(stream);
    const clickListenerMic = (_) => {
      if (audioContext.state !== "running") {
        mic.connect(audioWorkletNode);
        startButton.removeEventListener("click", clickListenerMic);
      } else {
        audioContext.suspend();
      }
    };
    if (startButton !== null) {
      startButton.addEventListener("click", clickListenerMic);
    }
  }

  navigator.mediaDevices
    .getUserMedia({
      video: false,
      audio: {
        echoCancellation: false,
        noiseSuppression: false,
        autoGainControl: false,
        // latency: 0,
        channelCount: 1,
      },
    })
    .then((stream) => init(stream));
}

// ====================
function JS_LoadFinished() {
  pdIsInitialized = true;
  var soundIcon = document.getElementById("SoundIcon");
  if (soundIcon === null) {
    console.log("SoundIcon not found!");
    return;
  }
  soundIcon.className = "fa-solid fa-volume-xmark fa-beat fa-2x";
}

// =======================
// ==== SET VARIABLES ====
// =======================
function JS_setFloat(symbol, value) {
  window.pd4webGuiValues[symbol] = value;
}

// ====================
function JS_setSymbol(symbol, value) {
  if (symbol.includes("pd4webscore")) {
    let img = document.getElementById(symbol);
    console.log(symbol, img);
    if (img === null) {
      console.error("Image with id " + symbol + " not found!");
      return;
    }
    img.src = value;
  }
  window.pd4webGuiValues[symbol] = value;
}

// ====================
function JS_setList(symbol, value) {
  if (window.pd4webGuiValues[symbol] === undefined) {
    window.pd4webGuiValues[symbol] = [];
  }
  window.pd4webGuiValues[symbol].push(value);
}

//╭─────────────────────────────────────╮
//│  JavaScript Functions to Send Data  │
//│                to Pd                │
//╰─────────────────────────────────────╯

function sendBang(receiver) {
  if (Module === undefined) {
    alert("Module is undefined!");
    return;
  }
  var str_rawReceiver = new TextEncoder().encode(receiver);
  var ptrReceiver = Module._webpd_malloc(str_rawReceiver.length + 1);
  var chunkReceiver = Module.HEAPU8.subarray(
    ptrReceiver,
    ptrReceiver + str_rawReceiver.length,
  );
  chunkReceiver.set(str_rawReceiver);
  Module.HEAPU8[ptrReceiver + str_rawReceiver.length] = 0; // Null-terminate the string
  if (Module._sendBangToPd(ptrReceiver) !== 0) {
    console.error("Error sending float to pd");
  }
  Module._webpd_free(ptrReceiver);
}

// ─────────────────────────────────────
function sendFloat(receiver, f) {
  if (Module === undefined) {
    alert("Module is undefined!");
    return;
  }
  var str_rawReceiver = new TextEncoder().encode(receiver);
  var ptrReceiver = Module._webpd_malloc(str_rawReceiver.length + 1);
  var chunkReceiver = Module.HEAPU8.subarray(
    ptrReceiver,
    ptrReceiver + str_rawReceiver.length,
  );
  chunkReceiver.set(str_rawReceiver);
  Module.HEAPU8[ptrReceiver + str_rawReceiver.length] = 0; // Null-terminate the string
  const result = Module._sendFloatToPd(ptrReceiver, f);
  if (result !== 0) {
    console.error("Error sending float to pd");
  }
  Module._webpd_free(ptrReceiver);
}

// ─────────────────────────────────────
function sendString(receiver, str) {
  if (pdIsInitialized === false) {
    console.log("Pd is not initialized yet!");
    return;
  }
  var str_rawReceiver = new TextEncoder().encode(receiver);
  var ptrReceiver = Module._webpd_malloc(str_rawReceiver.length + 1);
  var chunkReceiver = Module.HEAPU8.subarray(
    ptrReceiver,
    ptrReceiver + str_rawReceiver.length,
  );
  chunkReceiver.set(str_rawReceiver);
  Module.HEAPU8[ptrReceiver + str_rawReceiver.length] = 0; // Null-terminate the string

  var str_rawThing = new TextEncoder().encode(str);
  var ptrThing = Module._webpd_malloc(str_rawThing.length + 1);
  var chunkReceiver = Module.HEAPU8.subarray(
    ptrThing,
    ptrThing + str_rawThing.length,
  );
  chunkReceiver.set(str_rawThing);
  Module.HEAPU8[ptrThing + str_rawThing.length] = 0; // Null-terminate the string

  var result = Module._sendSymbolToPd(ptrReceiver, ptrThing);

  Module._webpd_free(ptrReceiver);
  Module._webpd_free(ptrThing);

  if (result !== 0) {
    console.error("Error sending float to pd");
  }
}

// ─────────────────────────────────────
function sendList(receiver, array) {
  if (pdIsInitialized === false) {
    console.log("Pd is not initialized yet!");
    return;
  }
  var str_rawReceiver = new TextEncoder().encode(receiver);
  var ptrReceiver = Module._webpd_malloc(str_rawReceiver.length + 1);
  var chunkReceiver = Module.HEAPU8.subarray(
    ptrReceiver,
    ptrReceiver + str_rawReceiver.length,
  );
  chunkReceiver.set(str_rawReceiver);
  Module.HEAPU8[ptrReceiver + str_rawReceiver.length] = 0; // Null-terminate the string

  var arrayLen = array.length;
  Module._startListMessage(arrayLen);

  // ───────── add things to list ─────────
  for (var i = 0; i < arrayLen; i++) {
    if (typeof array[i] === "number") {
      Module._addFloatToList(array[i]);
    } else if (typeof array[i] === "string") {
      var str_rawThing = new TextEncoder().encode(array[i]);
      var ptrThing = Module._webpd_malloc(str_rawThing.length + 1);
      var chunkReceiver = Module.HEAPU8.subarray(
        ptrThing,
        ptrThing + str_rawThing.length,
      );
      chunkReceiver.set(str_rawThing);
      Module.HEAPU8[ptrThing + str_rawThing.length] = 0; // Null-terminate the string
      Module._addSymbolToList(ptrThing);
      Module._webpd_free(ptrThing);
    } else {
      console.error("Type not supported yet!");
    }
  }

  // ───────────── Send list ──────────
  Module._FinishAndSendList(ptrReceiver);
  Module._webpd_free(ptrReceiver);
}

// ─────────────────────────────────────
function sendToPureData(receiver, thing) {
  if (typeof receiver !== "string") {
    console.error("Receiver is not a string!");
    return;
  }

  if (typeof thing === "number") {
    sendFloat(receiver, thing);
  } else if (typeof thing === "string") {
    sendString(receiver, thing);
  } else if (Array.isArray(thing)) {
    alert("Array is not supported yet!");
  }
}