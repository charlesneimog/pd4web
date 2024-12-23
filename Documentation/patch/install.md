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
  To compile you first patch, first you need to install <code>pd4web</code> and <code>python</code>. You can use a PureData patch or a Python package.
</p>


 
---
## <h2 align="center">:fontawesome-solid-tv: Pd Patch (Graphical Interface)</h2>

Simple Gui Interface to use `pd4web`.


<div class="grid cards" markdown>

-   :material-gesture-double-tap:{ .lg .middle } __How to Install?__

    ---

    To use the `pd4web` object in PureData, follow these steps:

    * <p> Install <a href="https://puredata.info/downloads/pure-data" target="_blank">PureData</a> and 
      <a href="https://www.python.org/downloads/release/python-3130/" target="_blank">Python</a> 
      (bottom of the page).
    </p>
    
    * Open PureData.
    * Navigate to **Help** :material-arrow-right: **Find Externals**.
    * Search for `pd4web`.
    * Click the **Install** button.


-   :fontawesome-solid-tv:{ .lg .middle } __Gui Interface__

    ---
    <p align="center">
      <img src="../../assets/pd-pd4web.png" alt="pd4web" width="60%" style="border-radius: 2px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.3);">
    </p>
</div>


---
## <h2 align="center">:octicons-terminal-16: Python Package (Command Line)</h2>

!!! tip "Prefer the Command Line? Here's How"  
    If you're comfortable with the command line, you can use the `pd4web` package to compile your patches efficiently.

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
    usage: pd4web.py <PureData Patch>

    ```
</div>

--- 

Run the command `pd4web <PureData Patch>` to compile your patch. The options are:

- `--template <TEMPLATE_NUMBER>` : Specify the template number.
- `--memory <MEMORY_SIZE>` : Specify the memory size in megabytes (MB).
- `--clean` : Clean the output directory before compiling. 
- `--verbose` or `-v` : Show verbose output.
- `--nogui` : Do not show the GUI.
- `--patch-zoom <ZOOM>` : Specify the patch zoom level.
- `--debug` : Compile the website with debugging information.
- `--run-browser` : Run browser (this will not compile the patch).



    
