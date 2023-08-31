# WebPd

This is one Repo where I try to merge all the repos related with run PureData in the browser, this work was done by [claudeha](https://github.com/claudeha/libpd.git) and already have on PR for PureData, that was not merged yet :(. 

The main addition to this repository is the webpatch module, located within the 'resources' directory. `webpatch.py` is a Python script that configures a `.c` file, making it possible to use various external ones. Below is the list of supported external libraries.

* *Obs*.: Some objects, mainly because of dynamic libraries, are not supported (mainly because I don't use then).

| Library | NOT Supported Objects       | 
|---------|-----------------------------|
| `else`  | `sfont~`, `plaits~`, `sfz~` |
| `cyclone`  | - |
| `earplug~`  | - |



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

