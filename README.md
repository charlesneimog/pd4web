# PdWebCompiler

This repository aims to compile PureData Patches for WebBrowser using Web Audio technologies. Differ itself from approaches like [hvcc](https://github.com/Wasted-Audio/hvcc) and [WebPd](https://github.com/sebpiq/WebPd), with PdWebCompiler you can compile any PureData vanilla patch with the great addition of compile most part of the externals.

This repository configures a `.c` file and then compile it for web, allowing running complete PureData patches **with externals** for the web. 

-------------------

| Library   |          Not Supported       |  
|:---------:|:----------------------------:|
| else      |  `sfont~`, `plaits~`, `sfz~` | 
| cyclone   |               -              | 
| earplug~  |               -              | 


* *Obs*.: Some objects, mainly because of dynamic libraries, are not supported yet (mainly because I don't use then, **please make one Pull Request**).

------------------

## Make your Pull Request

There is three main things in this repository that you can make Pull Requests:

* *Add externals support in `resources/lib/externals.py`*.
* Improve the `src/template.c`.
* Improve the `resources/webpd.py`.

If you see some way to improve the `resources/template.c` or all the Python inside `resources` please make an Pull Request.

 
## Make your Patch run on Internet

To put your patch online run these commands:

#### 1. Install Git

* `Linux`: `apt install Git`, `dnf install Git`, etc...

* `MacOS`: Go to [Git](https://git-scm.com/download/mac) website and Download/Install Binary installer.

* `Windows`: `winget install Git.Git`.


#### 2. Install for emscripten

``` bash
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    ./emsdk install latest
    ./emsdk activate latest
    source ./emsdk_env.sh

```

You need to install `emscripten`. 

#### 3. Compile your Patch

* **IMPORTANT**: Download it from zip will not work.


``` bash
    git clone https://github.com/charlesneimog/pdweb.git
    cd pdweb && git submodule init && git submodule update
    cd libpd && git switch emscripten-pd54 && git submodule init && git submodule update
    cd pure-data && git switch emscripten-pd54
    cd ../ && mkdir build && cd build
    emcmake cmake .. -DPD_UTILS:BOOL=OFF -DCMAKE_BUILD_TYPE=Release -Wno-dev
    emmake make STATIC=true
    cd .. && cd ..
    make PATCH=./mypatch.pd 
```

