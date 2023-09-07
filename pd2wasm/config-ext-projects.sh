cd .. && git submodule init && git submodule update
cd libpd && git switch emscripten-pd54 && git submodule init && git submodule update
cd pure-data && git switch emscripten-pd54
cd ../ && mkdir build && cd build
emcmake cmake .. -DPD_UTILS:BOOL=OFF -DCMAKE_BUILD_TYPE=Release -Wno-dev
emmake make STATIC=true
cd .. && cd ..

