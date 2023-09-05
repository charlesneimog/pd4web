# PdWebCompiler

**Introduction**

`PdWebCompiler` allows bring PureData functionality to web browsers using Web Audio technologies. This tool stands out from other approaches like [hvcc](https://github.com/Wasted-Audio/hvcc) and [WebPd](https://github.com/sebpiq/WebPd) because it compiles the [libpd](https://github.com/libpd/libpd) source, this is possible mainly because of the work of [claudeha](https://github.com/claudeha). You can compile any PureData vanilla patch and most of the externals.

So, for now, we have more than 800 supported externals objects. One of the main problems is that, for some objects, mainly due to dynamic libraries, we need to make some work arround to compile it for `Wasm`. Feel free to submit a Pull Request to add support.

------------------

## Making a Pull Request

Contributions to this repository are welcome! Here are the main areas you can contribute to:

* The `resources/lib/` folder, which contains files with special steps for compiling and add support to dynamic libraries.
* The `resources/externals/` folder, which contains python scripts with extra steps for externals (externals with extra `.h` and dynamic libraries require it). 
* The `src/template.c`, where we load PureData and the patches, we are looking mainly for faster code.
* The `resources/PdWebCompiler.py`, which configures the `main.c` file. It allows to use externals.

## Running Your Patch on the Internet

To make your patch available online, follow these steps:

#### 1. Install Git (first-time setup)

* **Linux**: `apt install Git`, `dnf install Git`, etc.
* **MacOS**: Download and install the [Git Binary installer](https://git-scm.com/download/mac).
* **Windows**: `winget install Git.Git`.

#### 2. Install Python (first-time setup)

* **Linux**: `apt install python3.11`, `dnf install python3.11`, etc.
* **MacOS**: Download and install Python from the [Python website](https://www.python.org/downloads/release/python-3115/).
* **Windows**: `winget install -e --id Python.Python.3.11`.

#### 3. Install emscripten (first-time setup)

```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh && cd ..
```

#### 4. Configure the Environment (first-time setup)

```
git clone https://github.com/charlesneimog/PdWebCompiler.git
cd PdWebCompiler && git submodule init && git submodule update
cd libpd && git switch emscripten-pd54 && git submodule init && git submodule update
cd pure-data && git switch emscripten-pd54
cd ../ && mkdir build && cd build
emcmake cmake .. -DPD_UTILS:BOOL=OFF -DCMAKE_BUILD_TYPE=Release -Wno-dev
emmake make STATIC=true
cd .. && cd ..

```

#### 5. Compile Your Patch (Just these two lines for now)
```
cd emsdk && source ./emsdk_env.sh && cd ..
make PATCH=./mypatch.pd
```


