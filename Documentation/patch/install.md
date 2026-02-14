---
hide:
  - toc
---
 <style>
  .md-typeset h1,
  .md-content__button {
    display: none;
  }
</style>


`pd4web` can be installed as a Pure Data object or as a Python Package. 

If you are a Pure Data user, use the __Pd Patch (Graphical Interface)__. If you know how to do Python code (write code), them use the Python Package.

---
<a name="pd4web-pdpatch"></a>
## <h2 align="center">:fontawesome-solid-tv: Pd Patch (Graphical Interface)</h2>

On Pure Data, `pd4web`provides a simple GUI. To use the `pd4web` object in Pure Data, follow these steps:

- 1) Open Pure Data.
- 2) Go to **Tools** → **Find Externals**.
- 3) Search for `pd4web`.
- 4) Click **Install**.
- 5) A patch will open automatically. Keep it open until you see the message `[pd4web] Pd4Web initialized successfully`.
- 6) After you see the message, close Pd, open it again, them create a `pd4web` object. You will see a Graphical Interface similar to the image below.

<p align="center">
  <img src="../../assets/pd-pd4web.png" alt="pd4web" width="40%" style="border-radius: 2px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.3);">
</p>


---
<a name="pd4web-pdpython"></a>
## <h2 align="center">:octicons-terminal-16: Python Package (Command Line)</h2>

If you're comfortable with the command line, you can use the `pd4web` package to compile your patches.

<div class="grid cards" markdown>

-   :material-clock-fast:{ .lg .middle } __Set up in 10 seconds__

    ---
    Use pip to install the `pd4web` package:

    ```
    pip install pd4web
    ```

-   :octicons-terminal-16:{ .lg .middle } __Command Line Use__

    ---
    You will get a executable script called `pd4web`.

    ```bash
    usage: pd4web <PureData Patch Path>

    ```
</div>

---
The options of the CLI are:

| Flag | Description | Default |
|------|-------------|---------|
| `--help` | Print usage information | - |
| `--pd4web-folder <PATH>` | Pd4Web folder (with libraries, sources, etc.) | - |
| `-m, --initial-memory <MB>` | Initial memory size in megabytes | `32` |
| `-z, --patch-zoom <ZOOM>` | Set patch zoom level | `1` |
| `-o, --output-folder <PATH>` | Output folder | Same as the patch being compiled |
| `-c, --clear-before-compile` | Clear `WebPatch` and `Pd4Web` folders before compilation | `false` |
| `-t, --template-id <ID>` | Set template ID. See [templates](https://charlesneimog.github.io/pd4web/patch/templates/) | `0` |
| `-s, --server` | Start server for the given patch or current folder | - |
| `--evnk <LIST>` | Register extra PdLua objects for a virtual number keyboard on touch devices | `"nbx,floatatom"` |
| `--evtk <LIST>` | Register extra PdLua objects for a virtual text keyboard on touch devices | `"listatom,symbolatom"` |
| `--export-es6-module` | Export Pd4WebModule as an ES6 module for native import/export and TypeScript support | `false` |
| `--nogui` | Disable GUI interface | `false` |
| `--debug` | Activate debug compilation (faster compilation, slower execution, more error info) | `false` |
| `--devdebug` | Activate development debug compilation | `false` |
| `--failfast` | Fail on first error message | `true` |
