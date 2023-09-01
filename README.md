# PdWebCompiler

This repository aims to compile PureData Patches for WebBrowser using Web Audio technologies. It distinguishes itself from approaches such as [hvcc](https://github.com/Wasted-Audio/hvcc) and [WebPd](https://github.com/sebpiq/WebPd) because we compile the [libpd](https://github.com/libpd/libpd) source. You can compile any PureData vanilla patch and most of the externals.

This repository configures a `.c` file and then compile it for web, allowing running complete PureData patches **with externals** for the web. 

-------------------

| Library   |          Not Supported       |  
|:---------:|:----------------------------:|
| else      |  `sfont~`, `plaits~`, `sfz~` | 
| cyclone   |               -              | 
| earplug~  |               -              | 


* *Obs*.: Some objects, mainly because of dynamic libraries, are not supported yet (mainly because I don't use then, **please make a Pull Request**).

------------------

## Make your Pull Request

There is three main things in this repository:

* The `resources/lib/` folder, where are located some files with special steps to compile some `externals/libraries`.
* The `src/template.c`, where we load PureData and load the patches.
* The `resources/PdWebCompiler.py`, this make some configurations for the `main.c` file (because of it we can load externals).

## Make your Patch run on Internet

To put your patch online run these commands:

#### 1. Install Git (only the first time)

* `Linux`: `apt install Git`, `dnf install Git`, etc...

* `MacOS`: Go to [Git](https://git-scm.com/download/mac) website and Download/Install Binary installer.

* `Windows`: `winget install Git.Git`.

#### 2. Install Python (only the first time)

* `Linux`: `apt install python3.11`, `dnf install python3.11`, etc...

* `MacOS`: Go to [Python](https://www.python.org/downloads/release/python-3115/) website and Download/Install Binary installer.

* `Windows`: `winget install -e --id Python.Python.3.11`.

#### 2. Install emscripten (only the first time)

``` bash
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    ./emsdk install latest
    ./emsdk activate latest
    source ./emsdk_env.sh && cd ..
```

#### 3. Configure all the Enviroment (only the first time)

``` bash
    git clone https://github.com/charlesneimog/PdWebCompiler.git
    cd PdWebCompiler && git submodule init && git submodule update
    cd libpd && git switch emscripten-pd54 && git submodule init && git submodule update
    cd pure-data && git switch emscripten-pd54
    cd ../ && mkdir build && cd build
    emcmake cmake .. -DPD_UTILS:BOOL=OFF -DCMAKE_BUILD_TYPE=Release -Wno-dev
    emmake make STATIC=true
    cd .. && cd ..
```

#### 4. Compile your patch (Just this two lines for now)

```
    cd emsdk && source ./emsdk_env.sh && cd ..
    make PATCH=./mypatch.pd 
```

