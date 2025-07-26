<style>
    h2 {
        text-align: center;
        font-weight: bold;
    };

  .md-typeset h2,
  .md-content__button {
      text-align: center;
      font-weight: bold;
  }

</style>


# Pd4WebModule

!!! warning "For Those Familiar with JavaScript" 
    This section is for anyone with JavaScript knowledge who wants to explore interactions with the `pd4web` project. If you're not comfortable with JavaScript, you can skip this section.

The `pd4web` project includes a JavaScript module named `Pd4WebModule`. In the `index.html` file inside the `WebPatch` directory, `Pd4Web` is loaded as follows:

``` javascript
var Pd4Web = null;
Pd4WebModule().then((Pd4WebModulePromise) => {
    // load method
    Pd4Web = new Pd4WebModulePromise.Pd4Web();
});
```

## Opening a Patch

### `Pd4Web.openPatch`

After you have loaded `Pd4Web`, you can open a patch using the following code:

!!! warning "Patches must be compiled!"
    Patches files **MUST** be compiled; simply placing a new patch inside a folder will not work. `Pd4Web` has its file system, which is automatically built when you compile the patch. This means that files available on our website will not necessarily be available in the patch.



``` js
Pd4Web.openPatch("index.pd", {
    canvasId: "Pd4WebCanvas",
    soundToggleId: "Pd4WebAudioSwitch",
    patchZoom: 2,
    projectName: "MyProject",
    channelCountIn: 1,
    channelCountOut: 2,
    sampleRate: 48000,
    renderGui: true,
    requestMidi: false,
    fps: 0,
});
```

* `canvasId` - The ID of the canvas (must be a `<canvas>`) element in your HTML where the patch will be drawn. This will be resized.
* `soundToggleId` - The ID of the canvas (must be a `<span>`) element in your HTML where `Pd4Web` will put a listener for click (to initialize audio).
* `patchZoom` - The zoom level of the patch.
* `projectName` - The name of the project, `Pd4Web` will define it as title of the page.
* `channelCountIn` - The number of input channels.
* `channelCountOut` - The number of output channels.
* `sampleRate` - The sample rate of the patch.
* `renderGui` - Should `Pd4Web` render the GUI?
* `requestMidi` - Should `Pd4Web` request MIDI access?
* `fps` - The FPS of the patch. On `0` the browser will decide this.

## Initializing Audio Manually

### `Pd4Web.toggleAudio`

When you open the patch but does not define a `soundToggleId`, you must initialize audio manually. This must be done by click user event. For example: 

``` js
document.addEventListener(
    "click",
    async () => {
        Pd4Web.toggleAudio();
    },
    { once: true }, // Ensures this only runs once
);
```

!!! warning "Run `toggleAudio` outside of click event"
    The browser does not allow audio to play outside of a click event.



## Send data

If you want to send data to a Pd patch, you can use the following functions:

### `Pd4Web.sendBang`

Send number to Pd. 

``` js 
Pd4Web.sendBang("myreceiver")
```

Bang will be received by `[r myreceiver]`


### `Pd4Web.sendFloat`

Send number to Pd. 

``` js 
Pd4Web.sendFloat("myreceiver", 5)
```

To receive this use one object `[r myreceiver]` in Pd.

### `Pd4Web.sendSymbol`


Send symbol to Pd. 

``` js 
Pd4Web.sendSymbol("myreceiver", "mysymbol")
```

To receive this use one object `[r myreceiver]` in Pd.

### `Pd4Web.sendList`

Send list to Pd. 

``` js 
Pd4Web.sendList("myreceiver", [5, "mysymbol"])
```

To receive this use one object `[r myreceiver]` in Pd.

## Receive data

To receive data from Pd you can use `callback` functions. They are defined by PureData `selector` and must be defined precisily.

---

### `Pd4Web.onBangReceived`

Registers a callback function that is triggered when a `bang` is received.

```js
Pd4Web.onBangReceived("r-test", (r) => {
    console.log("Received bang from", r);
});
```

When the patch receives a `bang` at `[r-test]`, the callback function is called.

---

### `Pd4Web.onFloatReceived`

Registers a callback function that is triggered when a `float` is received.

```js
Pd4Web.onFloatReceived("r-test", (r, f) => {
    console.log("Received float:", f);
});
```

When the patch receives a `float` at `[r-test]`, the callback function is called.

---

### `Pd4Web.onSymbolReceived`

Registers a callback function that is triggered when a `symbol` is received.

```js
Pd4Web.onSymbolReceived("r-test", (r, s) => {
    console.log("Received symbol:", s);
});
```

When the patch receives a `symbol` at `[r-test]`, the callback function is called.

---

### `Pd4Web.onListReceived`

Registers a callback function that is triggered when a `list` is received.

```js
Pd4Web.onListReceived("r-test", (r, l) => {
    console.log("Received list:", l);
});
```

When the patch receives a `list` at `[r-test]`, the callback function is called.


