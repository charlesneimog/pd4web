declare namespace pd4web {
  export type Pd4WebModule = () => Promise<{
    Pd4Web: new () => Pd4Web;
  }>;

  export type Pd4Web = {
    /**
     * Open a _previously compiled_ patch.
     *
     * Note that patch files **must** be compiled; simply placing a new patch
     * file inside a folder will not work. _Pd4Web_ has its own file-system
     * which is automatically built when you compile the patch. This means that
     * files available on your website will not necessarily be available in the patch.
     *
     * @see {@link https://charlesneimog.github.io/pd4web/patch/install/ | Compiling patches}
     */
    openPatch: (patchFile: string, options?: OpenPatchOptions) => void;

    /**
     * When opening a patch that does not explicitly define a `soundToggleId`
     * the audio must be initialized manually via this method.
     *
     * @example
     *
     * ```js
     * document.addEventListener("click", () => { Pd4Web.init(); }, { once: true });
     * ```
     *
     * This is typically done inside a `click` user event because of
     * [how browsers handle autoplaying](https://developer.mozilla.org/en-US/docs/Web/Media/Guides/Autoplay).
     */
    init: () => void;

    /**
     * `toggleAudio` is similar to {@link Pd4Web.init|init} but it can also
     * pause the audio processing.
     *
     * If not using `Pd4Web.init()`, the first call to `Pd4Web.toggleAudio` must
     * be triggered inside a `click` event.
     */
    toggleAudio: () => void;

    /**
     * Send a `bang` to Pure Data.
     */
    sendBang: (name: string) => boolean;

    /**
     * Send a `number` to Pure Data.
     */
    sendFloat: (name: string, n: number) => boolean;

    /**
     * Send a `symbol` to Pure Data.
     */
    sendSymbol: (name: string, symbol: string) => boolean;

    /**
     * Send a `list` of symbols/numbers to Pure Data.
     *
     * @see {@link Pd4Web.sendFloat}
     * @see {@link Pd4Web.sendSymbol}
     */
    sendList: (name: string, list: Array<string | number>) => boolean;

    /**
     * Send a file’s binary data to Pure Data.
     *
     * _Pd4Web_ uses an internal file system within an
     * [AudioWorklet](https://developer.mozilla.org/en-US/docs/Web/API/AudioWorklet).
     *
     * This is great because it keeps everything safe and secure, but it also
     * means that to load audio files, text files, or anything else inside
     * Pure Data, you first need to send those files' binary data into the
     * _Pd4Web_ file-system.
     */
    sendFile: (data: ArrayBuffer, fileName: string) => void;

    /**
     * Register a callback function that is triggered when a `bang` is received.
     */
    onBangReceived: (name: string, callback: (name: string) => void) => void;

    /**
     * Register a callback function that is triggered when a `number` is received.
     */
    onFloatReceived: (name: string, callback: (name: string, n: number) => void) => void;

    /**
     * Register a callback function that is triggered when a `symbol` is received.
     */
    onSymbolReceived: (name: string, callback: (name: string, symbol: string) => void) => void;

    /**
     * Register a callback function that is triggered when a `list` is received.
     */
    onListReceived: (name: string, callback: (name: string, list: Array<string | number>) => void) => void;
  };

  export type OpenPatchOptions = {
    /**
     * The ID of the `<canvas>` element in your HTML where the patch will be drawn.
     *
     * The element will be resized.
     */
    canvasId?: string,

    /**
     * The ID of the `<canvas>` element in your HTML where _Pd4Web_ will add a
     * listener for `click` (to initialize audio).
     */
    soundToggleId?: string,

    /**
     * The zoom level of the patch.
     */
    patchZoom?: number,

    /**
     * The name of the project.
     *
     * _Pd4Web_ will use this value as the title of the page.
     */
    projectName?: string,

    /**
     * The number of input channels.
     */
    channelCountIn?: number,

    /**
     * The number of output channels.
     */
    channelCountOut?: number,

    /**
     * The sample rate of the patch.
     */
    sampleRate?: number,

    /**
     * Should _Pd4Web_ render the GUI?
     */
    renderGui?: boolean,

    /**
     * Should _Pd4Web_ request MIDI access?
     */
    requestMidi?: boolean,

    /**
     * The FPS of the patch.
     *
     * On `0` the browser will decide this.
     */
    fps?: number,
  };
}

declare global {
  /**
   * The _Pd4Web_ instance, exposed as a global.
   *
   * ```js
   * // Pd4Web object must be declared in the global scope
   * var Pd4Web = null;
   *
   * Pd4WebModule().then((module) => {
   *    Pd4Web = new module.Pd4Web();
   * });
   * ```
   */
  export let Pd4Web: pd4web.Pd4Web;

  export const Pd4WebModule: pd4web.Pd4WebModule;
}

export {}
