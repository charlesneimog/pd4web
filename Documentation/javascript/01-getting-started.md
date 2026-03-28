# Initialization

The `pd4web` project includes a JavaScript module named `Pd4WebModule`. In the `index.html` file inside the `WebPatch` directory, `Pd4Web` is loaded as follows:

!!! warning "For those familiar with JavaScript"
    This section is for anyone with JavaScript knowledge who wants to explore interactions with the `pd4web` project. If you’re not comfortable with JavaScript, you can skip this section.

```javascript
var Pd4Web = null;
Pd4WebModule().then((module) => {
    // load method
    Pd4Web = new module.Pd4Web();
});
```

## Opening a Patch

### `Pd4Web.openPatch`

After you have loaded `Pd4Web`, you can open a patch using the following code:

!!! warning "Patches must be compiled!"
    Patch files **MUST** be compiled; simply placing a new patch file inside a folder will not work. `Pd4Web` has its own file system which is automatically built when you compile the patch. This means that files available on our website will not necessarily be available in the patch.

```javascript
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

* `canvasId` - The ID of the `<canvas>` element in your HTML where the patch will be drawn. This will be resized.
* `soundToggleId` - The ID of the `<span>` element in your HTML where `Pd4Web` will put a listener for click (to initialize audio).
* `patchZoom` - The zoom level of the patch.
* `projectName` - The name of the project, `Pd4Web` will define it as the title of the page.
* `channelCountIn` - The number of input channels.
* `channelCountOut` - The number of output channels.
* `sampleRate` - The sample rate of the patch.
* `renderGui` - Should `Pd4Web` render the GUI?
* `requestMidi` - Should `Pd4Web` request MIDI access?
* `fps` - The FPS of the patch. On `0` the browser will decide this.
