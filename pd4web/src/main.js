var pdIsInitialized = false;
window.pd4webGuiValues = {};
window.subscribedData = {};
// let window.subscribedData = {};

function JS_AddUIButtons(audioContext, audioWorkletNode) {
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
      // TODO: choose audio out from browser
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
        channelCount: 1, //  TODO: This must be defined by pd4web.py
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

// ======================================
// ==== SEND DATA FROM WEBSITE TO PD ====
// ======================================
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

// ====================
function bindGuiReceiver(receiver) {
  console.log("bindGuiReceiver for " + receiver);
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
  if (Module._bindGuiReceiver(ptrReceiver) !== 0) {
    console.error("Error binding gui receiver to pd");
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

// ====================
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

// ====================
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
// =======================
// ==== SET VARIABLES ====
// =======================
function JS_setFloat(source, value) {
  if (source in window.subscribedData) {
    for (const data of window.subscribedData[source]) {
      switch (data.type) {
        case "bng":
          gui_bng_update_circle(data);
          break;
        case "tgl":
          data.value = value;
          gui_tgl_update_cross(data);
          break;
        case "vsl":
        case "hsl":
          gui_slider_set(data, value);
          gui_slider_bang(data);
          break;
        case "vradio":
        case "hradio":
          data.value = Math.min(
            Math.max(Math.floor(value), 0),
            data.number - 1,
          );
          gui_radio_update_button(data);
          sendFloat(data.send, data.value);
          break;
        case "vu":
          data.value = value;
          gui_vu_update_gain(data);
      }
    }
  } else {
    window.pd4webGuiValues[source] = value;
  }
}

// ====================
function JS_setSymbol(source, value) {
  // TODO: rename to receivedFloatFromPd
  if (value == "bang") {
    if (source in window.subscribedData) {
      for (const data of window.subscribedData[source]) {
        switch (data.type) {
          case "bng":
            gui_bng_update_circle(data);
            break;
          case "tgl":
            data.value = data.value ? 0 : data.default_value;
            gui_tgl_update_cross(data);
            break;
          case "vsl":
          case "hsl":
            gui_slider_bang(data);
            break;
          case "vradio":
          case "hradio":
            sendFloat(data.send, data.value);
            break;
        }
      }
    }
  } else {
    if (source in window.subscribedData) {
      for (const data of window.subscribedData[source]) {
        switch (data.type) {
          case "bng":
            gui_bng_update_circle(data);
            break;
        }
      }
    } else {
      window.pd4webGuiValues[source] = value;
    }
  }
}

// ====================
function JS_setList(source, value) {
  if (window.pd4webGuiValues[source] === undefined) {
    window.pd4webGuiValues[source] = [];
  }
  window.pd4webGuiValues[source].push(value);
}
