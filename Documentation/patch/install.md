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
 
<p align="center">
  To compile you to install <code>pd4web</code>. You can use a PureData patch or a Python package.
</p>


 
---
## <h2 align="center">:fontawesome-solid-tv: Pd Patch (Graphical Interface)</h2>

On PureData, `pd4web`provides a simple GUI.


<div class="grid cards" markdown>

-   :material-gesture-double-tap:{ .lg .middle } __How to Install?__

    ---

    To use the `pd4web` object in PureData, follow these steps:
    
    * Open PureData.
    * Navigate to **Tools** :material-arrow-right: **Find Externals**.
    * Search for `pd4web`.
    * Click the **Install** button.
    * Wait for download and create a new `pd4web` object.


-   :fontawesome-solid-tv:{ .lg .middle } __Gui Interface__

    ---
    <p align="center">
      <img src="../../assets/pd-pd4web.png" alt="pd4web" width="60%" style="border-radius: 2px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.3);">
    </p>
</div>


---
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

* `--help`: Print this help message.
* `--pd4web-folder <PATH>`: Path to the Pd4Web folder (libraries, sources, etc.).
* `-m, --initial-memory <MB>`: Initial memory size in megabytes. **Default:** `32`
* `-z, --patch-zoom <ZOOM>`: Patch zoom level. **Default:** `1`
* `-o, --output-folder <PATH>`: Output folder.
* `-t, --template-id <ID>`: Template ID. See available templates <ID> at: [templates](../templates) section.
* `--nogui`: Disable the Patch GUI interface rendering on the Website.
* `--debug`: Enable debug compilation, (faster compilation, slower execution, more runtime error information).
* `--devdebug`: Enable development debug compilation (to debug the compiler).
* `--failfast`: Stop compilation on the first error.
