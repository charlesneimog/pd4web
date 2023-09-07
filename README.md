# PdWebCompiler

**Introduction**

`PdWebCompiler` allows to run PureData patches in web browsers using Web Audio technologies. This tool stands out from other approaches like [hvcc](https://github.com/Wasted-Audio/hvcc) and [WebPd](https://github.com/sebpiq/WebPd) because it compiles the [libpd](https://github.com/libpd/libpd) source, this is possible mainly because of the work of [claudeha](https://github.com/claudeha). You can compile any PureData vanilla patch and most of the externals.

**Already Supported Library Externals**

| Library   |          Not Supported       | Number of Objects | 
|:---------:|:----------------------------:|:-----------------:|
| else      |  `sfont~`, `plaits~`, `sfz~` | 509               |
| cyclone   |               -              | ± 197             |
| convolve~ |               -              | 1                 |
| timbreIDLib |               -              | 109                |

So, for now, we have more than 800 supported externals objects.

* **Note**: Some objects, mainly due to dynamic libraries, are not supported yet. Feel free to submit a Pull Request to add support for them.

------------------

## Making a Pull Request

Contributions to this repository are welcome! Here are the main areas you can contribute to:

* The `pd2wasm/lib/` folder, which contains files with special steps for compiling certain `externals/libraries`.
* The `pd2wasm/src/template.c`, used to PdWebCompiler to build the `main.c` file.
* The `pd2wasm/resources/PdWebCompiler.py`, which configures the `main.c` file.

## Running Your Patch on the Internet

To make your patch available online, follow these steps:

#### 1. Install Python (first-time setup)

* **Linux**: `apt install python3.11`, `dnf install python3.11`, etc.
* **MacOS**: Download and install Python from the [Python website](https://www.python.org/downloads/release/python-3115/).
* **Windows**: `winget install -e --id Python.Python.3.11`.

#### 2. Then Install pd2wasm (first-time setup)

``` bash
  pip install pd2wasm
```
#### 3. Now, it is just to compile your patch

``` bash
      pd2wasm --patch ./YOUR_PATCH.pd
```


