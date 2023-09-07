
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



