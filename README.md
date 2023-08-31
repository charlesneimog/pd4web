# WebPd

This is one Repo where I try to merge all the repos related with run PureData in the browser, this work was done by [claudeha](https://github.com/claudeha/libpd.git) and already have on PR for PureData, that was not merged yet :(. 

## Make your patch run on internet

First install and activate the emscripten environment:



Then run these commands, (IMPORTANT: Download it from zip will not work).
    git clone https://github.com/charlesneimog/webpd.git
    cd webpd && git submodule init
    cd libpd && git switch emscripten-pd54
    cd pure-data && git switch emscripten-pd54
    cd ../ && mkdir build && cd build
    emcmake cmake .. -DPD_UTILS:BOOL=OFF -DCMAKE_BUILD_TYPE=Release -Wno-dev
    emmake make STATIC=true


## run

    python -m SimpleHTTPServer 8080 &
    firefox localhost:8080/pdtest.html   # plays automatically
    chromium localhost:8080/index.html   # click to play
