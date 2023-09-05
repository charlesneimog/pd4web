---
hide:
  - navigation
  - toc
---

# Compiling your patch

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
Open the terminal from your project folder. Then run the follow steps (3, 4 and 5) in the same terminal window.

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

You need to have one patch called mypatch in the project root.

```
cd emsdk && source ./emsdk_env.sh && cd ..
make PATCH=./mypatch.pd
```
