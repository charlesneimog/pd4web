var PdModule = undefined;
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
    console.log(audioContext);
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
        channelCount: 1, // TODO: Change this
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

//╭─────────────────────────────────────╮
//│   Save Variables from [s ui_...]    │
//╰─────────────────────────────────────╯
function JS_setFloat(source, value) {
  if (source in subscribedData) {
    for (const data of subscribedData[source]) {
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
      }
    }
  } else {
    window.pd4webGuiValues[source] = value;
  }
}

// ─────────────────────────────────────
function JS_setSymbol(source, value) {
  if (value == "bang") {
    if (source in subscribedData) {
      for (const data of subscribedData[source]) {
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
    if (source in subscribedData) {
      for (const data of subscribedData[source]) {
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

// ─────────────────────────────────────
function JS_setList(source, value) {
  if (window.pd4webGuiValues[source] === undefined) {
    window.pd4webGuiValues[source] = [];
  }
  window.pd4webGuiValues[source].push(value);
}

// ─────────────────────────────────────
//╭─────────────────────────────────────╮
//│  JavaScript Functions to Send Data  │
//│                to Pd                │
//╰─────────────────────────────────────╯
function sendBang(receiver) {
  if (PdModule === undefined) {
    alert("Module is undefined!");
    return;
  }
  var str_rawReceiver = new TextEncoder().encode(receiver);
  var ptrReceiver = PdModule._webpd_malloc(str_rawReceiver.length + 1);
  var chunkReceiver = PdModule.HEAPU8.subarray(
    ptrReceiver,
    ptrReceiver + str_rawReceiver.length,
  );
  chunkReceiver.set(str_rawReceiver);
  PdModule.HEAPU8[ptrReceiver + str_rawReceiver.length] = 0; // Null-terminate the string
  if (PdModule._sendBangToPd(ptrReceiver) !== 0) {
    console.error("Error sending float to pd");
  }
  PdModule._webpd_free(ptrReceiver);
}

// ─────────────────────────────────────
function sendFloat(receiver, f) {
  if (PdModule === undefined) {
    alert("Module is undefined!");
    return;
  }
  var str_rawReceiver = new TextEncoder().encode(receiver);
  var ptrReceiver = PdModule._webpd_malloc(str_rawReceiver.length + 1);
  var chunkReceiver = PdModule.HEAPU8.subarray(
    ptrReceiver,
    ptrReceiver + str_rawReceiver.length,
  );
  chunkReceiver.set(str_rawReceiver);
  PdModule.HEAPU8[ptrReceiver + str_rawReceiver.length] = 0; // Null-terminate the string
  const result = PdModule._sendFloatToPd(ptrReceiver, f);
  if (result !== 0) {
    console.error("Error sending float to pd");
  }
  PdModule._webpd_free(ptrReceiver);
}

// ─────────────────────────────────────
function sendString(receiver, str) {
  if (pdIsInitialized === false) {
    console.log("Pd is not initialized yet!");
    return;
  }
  var str_rawReceiver = new TextEncoder().encode(receiver);
  var ptrReceiver = PdModule._webpd_malloc(str_rawReceiver.length + 1);
  var chunkReceiver = PdModule.HEAPU8.subarray(
    ptrReceiver,
    ptrReceiver + str_rawReceiver.length,
  );
  chunkReceiver.set(str_rawReceiver);
  PdModule.HEAPU8[ptrReceiver + str_rawReceiver.length] = 0; // Null-terminate the string

  var str_rawThing = new TextEncoder().encode(str);
  var ptrThing = PdModule._webpd_malloc(str_rawThing.length + 1);
  var chunkReceiver = PdModule.HEAPU8.subarray(
    ptrThing,
    ptrThing + str_rawThing.length,
  );
  chunkReceiver.set(str_rawThing);
  PdModule.HEAPU8[ptrThing + str_rawThing.length] = 0; // Null-terminate the string

  var result = PdModule._sendSymbolToPd(ptrReceiver, ptrThing);

  PdModule._webpd_free(ptrReceiver);
  PdModule._webpd_free(ptrThing);

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
  var ptrReceiver = PdModule._webpd_malloc(str_rawReceiver.length + 1);
  var chunkReceiver = PdModule.HEAPU8.subarray(
    ptrReceiver,
    ptrReceiver + str_rawReceiver.length,
  );
  chunkReceiver.set(str_rawReceiver);
  PdModule.HEAPU8[ptrReceiver + str_rawReceiver.length] = 0; // Null-terminate the string

  var arrayLen = array.length;
  PdModule._startListMessage(arrayLen);

  // ───────── add things to list ─────────
  for (var i = 0; i < arrayLen; i++) {
    if (typeof array[i] === "number") {
      PdModule._addFloatToList(array[i]);
    } else if (typeof array[i] === "string") {
      var str_rawThing = new TextEncoder().encode(array[i]);
      var ptrThing = PdModule._webpd_malloc(str_rawThing.length + 1);
      var chunkReceiver = PdModule.HEAPU8.subarray(
        ptrThing,
        ptrThing + str_rawThing.length,
      );
      chunkReceiver.set(str_rawThing);
      PdModule.HEAPU8[ptrThing + str_rawThing.length] = 0; // Null-terminate the string
      PdModule._addSymbolToList(ptrThing);
      PdModule._webpd_free(ptrThing);
    } else {
      console.error("Type not supported yet!");
    }
  }

  // ───────────── Send list ──────────
  PdModule._FinishAndSendList(ptrReceiver);
  PdModule._webpd_free(ptrReceiver);
}

// ─────────────────────────────────────
function bindGuiReceiver(receiver) {
  console.log("bindGuiReceiver for " + receiver);
  if (PdModule === undefined) {
    alert("Module is undefined!");
    return;
  }
  var str_rawReceiver = new TextEncoder().encode(receiver);
  var ptrReceiver = PdModule._webpd_malloc(str_rawReceiver.length + 1);
  var chunkReceiver = PdModule.HEAPU8.subarray(
    ptrReceiver,
    ptrReceiver + str_rawReceiver.length,
  );
  chunkReceiver.set(str_rawReceiver);
  PdModule.HEAPU8[ptrReceiver + str_rawReceiver.length] = 0; // Null-terminate the string
  if (PdModule._bindGuiReceiver(ptrReceiver) !== 0) {
    console.error("Error binding gui receiver to pd");
  }
  PdModule._webpd_free(ptrReceiver);
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
    sendList(receiver, thing);
  } else {
    alert("You is trying to send to PureData a type not supported yet!");
  }
}

//╭─────────────────────────────────────╮
//│            Load PureData            │
//╰─────────────────────────────────────╯
function loadScript(url, callback) {
  var script = document.createElement("script");
  script.type = "text/javascript";
  script.src = url;

  if (script.readyState) {
    script.onreadystatechange = function () {
      if (script.readyState === "loaded" || script.readyState === "complete") {
        script.onreadystatechange = null;
        if (callback) callback();
      }
    };
  } else {
    script.onload = function () {
      if (callback) callback();
    };
  }
  document.head.appendChild(script);
}

// ─────────────────────────────────────
loadScript("./libpd.js", function () {
  PureData().then((module) => {
    PdModule = module;
  });

  loadScript("./gui.js", function () {
    // initGui();
  });
});
