# PdWebCompiler

This repository serves as a consolidation point for various repositories focused on running PureData in web browsers. The initial work on this project was undertaken by [claudeha](https://github.com/claudeha/pure-data/tree/emscripten), who has also submitted a pull request for PureData. Unfortunately, the pull request has not been merged yet :(.

The main addition to this repository is the webpatch module, located within the 'resources' directory. `webpatch.py` is a Python script that configures a `.c` file, making it possible to use various external ones. Below is the list of supported external libraries.

* *Obs*.: Some objects, mainly because of dynamic libraries, are not supported (mainly because I don't use then).

<p style="margin-left: auto; margin-right: auto">

| Library   |          Not Supported       |  
|:---------:|:----------------------------:|
| else      |  `sfont~`, `plaits~`, `sfz~` | 
| cyclone   |               -              |  
| earplug~  |               -              | 
| piro      |               -              |

</p>




## Make your patch run on internet

To put your patch online run these commands:

* **IMPORTANT**: Download it from zip will not work.

``` bash
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    ./emsdk install latest
    ./emsdk activate latest
    source ./emsdk_env.sh
    git clone https://github.com/charlesneimog/pdweb.git
    cd pdweb && git submodule init && git submodule update
    cd libpd && git switch emscripten-pd54 && git submodule init && git submodule update
    cd pure-data && git switch emscripten-pd54
    cd ../ && mkdir build && cd build
    emcmake cmake .. -DPD_UTILS:BOOL=OFF -DCMAKE_BUILD_TYPE=Release -Wno-dev
    emmake make STATIC=true
    cd .. && cd ..
    make PATCH=./mypatch.pd 

