# WebPd

This is one Repo where I try to merge all the repos related with run PureData in the browser, this work was done by [claudeha](https://github.com/claudeha/libpd.git) and already have on PR for PureData, that was not merged yet :(. 

## Make your patch run on internet

To put your patch online run these commands:

* **IMPORTANT**: Download it from zip will not work.

``` bash
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    ./emsdk install latest
    ./emsdk activate latest
    source ./emsdk_env.sh
    git clone https://github.com/charlesneimog/webpd.git
    cd webpd && git submodule init && git submodule update
    cd libpd && git switch emscripten-pd54 && git submodule update
    cd pure-data && git switch emscripten-pd54
    cd ../ && mkdir build && cd build
    emcmake cmake .. -DPD_UTILS:BOOL=OFF -DCMAKE_BUILD_TYPE=Release -Wno-dev
    emmake make STATIC=true

