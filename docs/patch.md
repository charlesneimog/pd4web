---
hide:
  - toc
---

# PureData Online: Step-by-Step Guide
 
<p style="text-align: center"> This guide introduce step by step the process of compiling your PureData patches. </p>


## <h2 style="text-align: center"><b>Installing Dependencies</b></h2>

<p style="text-align: center"> To compile your patch, you need to install <code>Git</code>, <code>Python</code>, and <code>pd4web</code>. Below we have instructions for all the main plataforms, follow the steps for your plataform.</p>

--------------------------
### <h3 style="text-align: center"><b>Git</b></h3>
--------------------------

=== ":fontawesome-brands-windows: Windows"
    

    * **Windows 11**: If you are on Windows 11, you can easily install Git using the `winget` command. Open a Command Prompt or PowerShell window and run the following command:

        !!! bash-code 
            ``` powershell
            winget install Git.Git
            ``` 

    * **Windows 10**: For Windows 10 you can install `winget` using the Windows Store [Install Winget](https://apps.microsoft.com/store/detail/instalador-de-aplicativo/9NBLGGH4NNS1?hl=pt-br&gl=br&rtc=1). After install `winget`, open a Command Prompt or PowerShell window and run `winget install Git.Git`.
	    
	    !!! Warning
	        <p style="text-align: center">Minor versions of Windows not support `winget` and are not supported by this documentation.</p>
	    
=== ":material-apple: macOS"

    To install `Git` in Mac Os you have two options:
    
    * **installer**: To install Git using normal installer go to [Git](https://git-scm.com/download/mac), search for *Binary Installer*, download and install it. 
    
    * **brew**: To install Git using `brew`, visit the [Homebrew](https://brew.sh/) website, and follow their installation instructions for macOS. After installing Homebrew, open a Terminal and run the following command to install Git `brew install git`.
    !!! bash-code 
            ``` powershell
            brew install git
            ``` 

=== ":material-linux: Linux"
    
    On Fedora:
    
    !!! bash-code 
    
        ``` powershell
        sudo dnf install git
        ``` 
       
    On Ubuntu/Debian Based:
    !!! bash-code 
    
        ``` powershell
        sudo apt install git
        ``` 
        
    On Arch based:
    
    !!! bash-code 
    
        ``` powershell
        sudo pacman -S git
        ``` 
    
--------------------------
### <h3 style="text-align: center"><b>Python</b></h3>
--------------------------

=== ":fontawesome-brands-windows: Windows"
    
    On `Windows` you can install Python like and ordirary software.

    1. Go to [Python.org](https://www.python.org/downloads/release/python-31011/),
    2. Go to the bottom of the page and download: `Windows installer (64-bit)`.
    3. Install it as an ordinary program.

    **or easily run**.
    
    !!! bash-code
        ``` bash    
            winget install -e --id Python.Python.3.11
        ``` 
    
=== ":material-apple: macOS"
    
    On `MacOS` you can install Python like and ordirary software.
    
    1. Go to [Python.org](https://www.python.org/downloads/release/python-31011/),
    2. Go to the bottom of the page and download: `macOS 64-bit universal2 installer`.
    3. Install it as an ordinary program.
    
    Or, if you has install `brew`, you can just run:
    
    !!! bash-code
        ``` bash    
            brew install python@3.11
        ``` 
    
    
=== ":material-linux: Linux"
    
    ??? bash-code "Fedora"
    
        ``` powershell
        sudo dnf install python3.11 
        ``` 
        Python should be installed if you use Fedora.
       
    ??? bash-code "Ubuntu/Debian"
    
        ``` powershell
        sudo apt install python3.11
        ``` 
            
    ??? bash-code "Arch"
    
        ``` powershell
        sudo pacman -S python3.11
        ``` 


--------------------------
### <h3 style="text-align: center"><b>pd4web</b></h3>
--------------------------


Close all the Terminals/Powershells/Cmds opened then open it again running `python3 -m pip install pd4web` or, for Windows, `python -m pip install pd4web`.

To test if it works you can run: `pd4web --help`. 

It must install a lot of things. Wait for it. Finnaly run `pd4web --help` again, you must see something like: 

!!! bash-code

    ``` 
    usage: pd4web [-h] --patch PATCH [--html HTML] [--confirm CONFIRM]
                   [--clearTmpFiles CLEARTMPFILES] [--server-port SERVER_PORT]
                   [--initial-memory INITIAL_MEMORY] [--gui GUI] [--version]

    Check the complete docs in https://www.charlesneimog.com/pd4web

        etc...

    ```

Now you can compile your patches! If it not work, you can buy support in <a href="https://ko-fi.com/s/13200c3cd6" target="_blank">Ko-Fi</a>.
 
 
-----------------------------------
## <h2 style="text-align: center"><b>Make your patch for Web</b></h2>
-----------------------------------

<p style="text-align: center"> Here, I will explain some considerations for starting a new Project using <code>pd4web</code>. </p>


#### Folder Structure

I recommend using the file structure shown below. Be careful with upper and lower case letters.


```
├─ PROJECT_FOLDER
└── Audios/
    ├── AllMyAudioFiles.wav
    └── AllMyAudioFiles.aif
└── Libs/
    ├── pdAbstraction1.pd
    └── pdAbstraction2.pd
└── Extras/
    ├── extrathings.png
    └── mygesture.svg
└── MY_MAIN_PATCH.pd
```

* In the `Audios` folder, you should place audio files. 

* In the `Libs` folder you store abstractions, text files, or any other relevant items.

* In the `Extras` folder, you should place items that are not intended for PureData but will be utilized to enhance the website's appearance. For instance, I use this folder to store `.svg` files of my scores, which I then display in the piece work in progress <a href="charlesneimog.github.io/Compiled-I" target="_blank">Compiled I.</a>

After you compile your patch, will be created in the ROOT of the project a file `index.html`, another file `YOUR_PATCH_NAME.cfg`, and a new folder called `webpatch`. The `index.html` will just redirect to the right `html` file that is inside `webpatch` folder. The `YOUR_PATCH_NAME.cfg` will save the versions of the libraries that you are using, the version of PureData, the version of `libpd` and the version of `emscripten`. And inside `webpatch` will be all the files used by the version of your patch.


```
├─ PROJECT_FOLDER
├── Audios/
    ├── AllMyAudioFiles.wav
    └── AllMyAudioFiles.aif
├── Libs/
    ├── pdAbstraction1.pd
    └── pdAbstraction2.pd
├── Extras/
    ├── extrathings.png
    └── mygesture.svg
├── webpatch/
    ├── libpd.data
    ├── ...
    └── libpd.wasm
├── MY_MAIN_PATCH.pd
├── MY_MAIN_PATCH.cfg
└── index.html

```

---------------------
#### Rules to follow when making your patch

There is some rules that you need to follow to `pd4web` work properly. 

---------------------
=== "Rule 1: Externals"

    !!! pd4web-rule "RULE #1"

        <h3 style="text-align: center">Always use the library name in the object. So, don't type `counter` object, type `cyclone/counter`. You can also use `declare -lib else` for example. But all libraries declared in the PureData configuration will not be recognized. </h3>
        
    This is how, for now, `pd4web` find the objects that are externals or embbedded in PureData. There is some automatic work around externals.

=== "Rule 2: Avoid Visual Objects"

    !!! pd4web-rule "RULE #2"

        <h3 style="text-align: center">Avoid the use of Visual Objects.</h3>

    Always avoid the use of visual objects. Visual arrays, for example will broke your patch. Because of many abstractions of `else` uses visual array, we replace then using `array define myarray` automatically, but I suspect that another visual objects should not work. So, don't use then.
 
-----------------------------------
## <h2 style="text-align: center"><b>Compiling the patch</b></h2>
-----------------------------------

Here I explain the steps to convert your `.pd` patch to `.wasm` file. The `.wasm` file will be loaded in the browser.

### <h3 style="text-align: center"><b>pd4web command line</b></h3>
---------------------

To convert your patch you must use `pd4web` in the terminal. To set configurations for `pd4web` must use some of the flags descrited below:  

`--patch`

:   Define your patch name. For example, `--patch mypatch.pd`


`--html` 
:   Define where is the `index.html` page. If not provided, `pd4web` will use the default page. `--html index.html`.

`--confirm`
:   There is some automatic way check if the external is correct, but it is not always accurate. If you want to confirm if the external is correct, use this flag. For example, `--confirm True`.

`--server-port`
:   If you want see your patch running in the web browser after the compilation process, you can use this. Normally we use the port number 8080, for example: `--server-port 8080`.

`--initial-memory`
:   If you have a big patch, maybe you will need more that `32MB` of memory, to use more memory set it using `--initial-memory 64`, for example.

`--replace-helper`
:   Replace the `helpers.js` file, where `pd4web` defines functions that are called after the load of `PureData` is finished. Replace the icons for sound in/off.


`--version`
:   Show the version of `pd4web`.

For example, to compile a big patch called `mygreatpiece.pd` you must run `pd4web --patch mygreatpiece.pd --initial-memory 64`. 


### <h3 style="text-align: center"><b>Common Browser Console Erros</b></h3>
---------------------

After you compile your patch, you should check if there is some error in the browser console. Below we show some of the common erros that we find when compiling our patches.

=== "Memory"

    !!! bash-code ERROR
        ```
        Uncaught RuntimeError: Aborted(OOM). Build with -sASSERTIONS for more info.
        at abort (libpd.js:1:12855)
        at abortOnCannotGrowMemory (libpd.js:1:116480)
        at _emscripten_resize_heap (libpd.js:1:116583)
        at libpd.wasm:0x2c8e6
        at libpd.wasm:0x3d5f
        at libpd.wasm:0x1f204
        at libpd.wasm:0x3044
        at libpd.wasm:0x3071
        at libpd.wasm:0xae4dd
        at libpd.wasm:0x18244
        ```
        
    To solve this, you must run `pd4web` with the flag `--initial-memory 64` or a bigger number.
    

=== "Files Not Found"

    To solve this, you must check all the paths used by PureData. It is important to say that, objects like `readsf~` are not able to search from paths declared by `declare -path myfolder`. So, if your audio files are inside Extra, you can't use `declare -path Extra` to open the files. You must send a message with `open Extra/myaudiofile.wav`. Something like `open myaudiofile.wav` will not work.

=== "Shared Array not defined"
    
    This is a big problem, it means that you are not using a `https` server. Because of security, we can use some features of browser without being in a secure enviroment. So if you are putting you website in a `http` server, Shared Array will be not defined. To solve this search how to use `https` to host your website.

-----------------------------------
## <h2 style="text-align: center"><b>Put the patch online</b></h2>
-----------------------------------

Once you compile the patch successfully, the last step is to put it online. You have some options, and the easy and free way is to upload all files in a repository in GitHub and make it available. 

1. First, you must download VsCode.
2. If you don't have a GitHub account, make it on [Github Sign In](https://github.com/signup).
3. Enter in you Github account using VsCode. You can get help using this [Youtube Video](https://www.youtube.com/watch?v=4Q9PHRsfIvQ).

The final step is to put it online. You have a lot of options, the easy way, if you already have a website, is to upload `index.html` and the folder `webpatch` in the ROOT where are your patch. 

If you don't have any website, you can use the free github pages. It provides a website with the `<yourusername>.github.io`, for example, my user name is `charlesneimog` so my website will be `charlesneimog.github.io`. If you are one completly noob, you can use VsCode to do that. See the video below:

<h2 style="text-align: center"> I will upload one video soon!</h2>

I hope that is can be usefull.

-----------------------------------
## <h2 style="text-align: center"><b>Making a <code>html</code> GUI</b></h2>
-----------------------------------

This is the hard part of the process, and for now you must know how to use `html` and maybe `css`. I will try to release some templates for `html`, but for now, you should make them by yourself. When you build your `index.html`, you must know that is possible to make your web GUI comunitate with PureData and Vice-versa. For that you should include the `helpers.js` file in your `index.html`. 

### <h3 style="text-align: center"><b>Receiving data from PureData</b></h3>

To get some data from PureData you must first send the data using `send` or `s` object. To differentiate the data that you send inside your own patch and the things that you want to send to web GUI we use the `ui_` identifier. For example, if you use `s mygain` it will not work, you can't get this data from the `html`. You must use `s ui_mygain` or `send ui_mygain`.

To get this data you use these JavaScript snippet.

``` js
const forceValue7 = document.getElementById('ui_mygain');
```

It is important to note that it will be a string, so if you need a number you must use `parseInt` or `parseFloat`.

### <h3 style="text-align: center"><b>Sending data to PureData</b></h3>

To make some trigger or change some parameter like `gain` for your patch you must use the `sendToPureData`. They are part of the `helpers.js` script provide with `pd4web`. To receive the data you must use `receive` or `r` with the same name specified with function `sendToPureData`.

For example, to send `0.7` to PureData you must use the `js` code below:

``` js
sendToPureData("mygain", 0.7);
```

to receive this data you must create one `r mygain`. 

!!! warning
    It is important to note that `libpd.js` must be loaded to use this function.


