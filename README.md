<p align="center">
  <h1 align="center">PdWebCompiler</h1>
  <p align="center">
    <a href="https://github.com/plugdata-team/plugdata/wiki">
      <img src="https://raw.githubusercontent.com/charlesneimog/PdWebCompiler/main/docs/assets/icon-light.svg" alt="Logo">
    </a>
  </p>
  <h4 align="center">Running Pd Patches in Your Web Browser.</h4>
</p>

<p align="center">
  <a href="https://github.com/charlesneimog/PdWebCompiler/releases/latest"><img src="https://img.shields.io/github/release/charlesneimog/PdWebCompiler?include_prereleases" alt="Release"></a>
  <a href="https://pypistats.org/packages/pd2wasm"><img src="https://img.shields.io/pypi/dm/pd2wasm" alt="Downloads"></a>
  <a href="https://pypistats.org/packages/pd2wasm"><img src="https://img.shields.io/pypi/pyversions/pd2wasm" alt="Version"></a>
</p>

<p align="center">
  <a href="https://img.shields.io/pypi/pyversions/pd2wasm"><img src="https://img.shields.io/badge/platforms-macOS%20%7C%20Windows%20%7C%20Linux-green" alt="License"></a>
</p>


<p align="center">
  <h2 align="center">Intro</h2>
  <br>
</p>

`PdWebCompiler` allows to run PureData patches in web browsers using Web Audio technologies. This tool stands out from other approaches like [hvcc](https://github.com/Wasted-Audio/hvcc) and [WebPd](https://github.com/sebpiq/WebPd) because it compiles the [libpd](https://github.com/libpd/libpd) source, this is possible mainly because of the work of [claudeha](https://github.com/claudeha). You can compile any PureData vanilla patch and most of the externals.

#### ⚠️ Warning: Breaking Changes Ahead ⚠️

The project is very recent, so can be very hard breaking changes for sometime.

<p align="center">
  <h2 align="center">Already Supported Library Externals</h2>
  <hr>
</p>


| Library   |          Not Supported       | Number of Objects | 
|:---------:|:----------------------------:|:-----------------:|
| else      |  `sfont~`, `plaits~`, `sfz~` | 509               |
| cyclone   |               -              | ± 197             |
| convolve~ |               -              | 1                 |
| timbreIDLib |               -              | 109                |

So, for now, we have more than 800 supported externals objects.

* **Note**: Some objects, mainly due to dynamic libraries, are not supported yet. Feel free to submit a Pull Request to add support for them.

<p align="center">
  <h2 align="center">Running Your Patch on the Internet</h2>
  <br>
</p>

To make your patch available online, follow these steps:

#### 1. Install Python (first-time setup)

* **Linux**: `apt install python3.11`, `dnf install python3.11`, etc.
* **MacOS**: Download and install Python from the [Python website](https://www.python.org/downloads/release/python-3115/).
* **Windows**: `winget install -e --id Python.Python.3.11` or go to [Python website](https://www.python.org/downloads/release/python-3115/)

#### 2. Then Install pd2wasm (first-time setup)

``` bash
pip install pd2wasm
```
#### 3. Now, it is just to compile your patch

``` bash
pd2wasm --patch ./YOUR_PATCH.pd --server-port 8080
```

<p align="center">
  <h2 align="center">Making a Pull Request</h2>
  <br>
</p>

Join us in contributing to this repository! Explore the primary areas for your valuable contributions:

* The `pd2wasm/lib/` folder, which contains files with special steps for compiling certain `externals/libraries`.
* The `pd2wasm/src/template.c`, used to PdWebCompiler to build the `main.c` file.
* The `pd2wasm/resources/PdWebCompiler.py`, which configures the `main.c` file.
