---
hide:
  - toc
---

# PureData Online: Step-by-Step Guide
 
<p style="text-align: center"> This guide introduce step by step the process of compiling your PureData patches. </p>


## <h2 style="text-align: center"><b>Installing Dependencies</b></h2>

<p style="text-align: center"> To compile your patch, you need to install <code>Git</code>, <code>Python</code>, and <code>pd2wasm</code>. Below we have instructions for all the main plataforms, follow the steps for your plataform.</p>

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
    
    * Go to [Python.org](https://www.python.org/downloads/release/python-31011/),
    * Go to the bottom of the page and download: `macOS 64-bit universal2 installer`.
    * Install it as an ordinary program.
    
    Or, if you has install `brew`, you can just run:
    
    !!! bash-code
        ``` bash    
            brew install python@3.11
        ``` 
    
    
=== ":material-linux: Linux"

    On Fedora:
    
    !!! bash-code 
    
        ``` powershell
        sudo dnf install python3.11
        ``` 
       
    On Ubuntu/Debian Based:
    !!! bash-code 
    
        ``` powershell
        sudo apt install python3.11
        ``` 
        
    On Arch based:
    
    !!! bash-code 
    
        ``` powershell
        sudo pacman -S python3.11
        ``` 


--------------------------
### <h3 style="text-align: center"><b>pd2wasm</b></h3>
--------------------------


Close all the Terminals/Powershells/Cmds opened then open it again running `python3 -m pip install pd2wasm` or, for Windows, `python -m pip install pd2wasm`.

To test if it works you can run: `pd2wasm --help`. 

It must install a lot of things. Wait for it. Finnaly run `pd2wasm --help` again, you must see something like: 

!!! bash-code

    ``` 
    usage: pd2wasm [-h] --patch PATCH [--html HTML] [--confirm CONFIRM]
                   [--clearTmpFiles CLEARTMPFILES] [--server-port SERVER_PORT]
                   [--initial-memory INITIAL_MEMORY] [--gui GUI] [--version]

    Check the complete docs in https://www.charlesneimog.com/PdWebCompiler

        etc...

    ```

Now you can compile your patches! If it not work, you can buy support in <a href="https://ko-fi.com/s/13200c3cd6" target="_blank">Ko-Fi</a>.
 
 
-----------------------------------
## <h2 style="text-align: center"><b>Make your patch</b></h2>
-----------------------------------

Here, I will explain some considerations for starting a new Project using `PdWebCompiler`.


#### Folder Structure

I recommend using the file structure shown below. Be careful with upper and lower case letters.

!!! folder "Folder Organization"
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

---------------------
#### Rules to follow

There is some rules that you need to follow to `pd2wasm` work properly. 

---------------------
=== "Rule 1: Externals"

    !!! pd2wasm-rule "RULE #1"

        <h3 style="text-align: center">Always use the library name in the object. So, don't type `counter` object, type `cyclone/counter`. </h3>
        
    This is how, for now, `pd2wasm` find the objects that are externals or embbedded in PureData. There is some automatic work around externals.

=== "Rule 2: Browser Console"

    !!! pd2wasm-rule "RULE #2"

        <h3 style="text-align: center">Always check the console after compile.</h3>

    There is some errors that just can be noted when run the patch in the Web. So always check the console of your browser. You can do it pressing: Shift + ⌘ + J (on macOS) or Shift + CTRL + J (on Windows/Linux). 


 
 
 
-----------------------------------
## <h2 style="text-align: center"><b>Compiling the patch</b></h2>
-----------------------------------

Here I explain the steps to convert your `.pd` patch to `.wasm` file. The `.wasm` file will be loaded in the browser.

### <h3 style="text-align: center"><b>pd2wasm command line</b></h3>
---------------------


`pd2wasm` is a command line used to compile your PureData patch. 

* `--patch`: Define your patch name. For example, `--patch mypatch.pd`
* `--html`: Define where is the `index.html` page. If not provided, `pd2wasm` will use the default page. `--html index.html`
* `--confirm`: There is some automatic way check if the external is correct, but it is not always accurate. If you want to confirm if the external is correct, use this flag. For example, `--confirm True`.
* `--server-port`: If you want see your patch running in the web browser after the compilation process, you can use this. Normally we use the port number 8080, for example: `--server-port 8080`.
* `--initial-memory`: If you have a big patch, maybe you will need more that `32MB` of memory, to use more memory set it using `--initial-memory 64`, for example.


### <h3 style="text-align: center"><b>Common Browser Console Erros</b></h3>
---------------------

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
        
    To solve this, you must run `pd2wasm` with the flag `--initial-memory 64` or a bigger number.
    

=== "Files Not Found"

    To solve this, you must check all the paths used by PureData. It is important to say that, objects like `readsf~` are not able to search from paths declared by `declare -path myfolder`.






-----------------------------------
## <h2 style="text-align: center"><b>Put the patch online</b></h2>
-----------------------------------

Once you compile the patch successfully, the last step is to put it online. You have some options, and the easy and free way is to upload all files in a repository in GitHub and make it available. 

1. First, you must download VsCode.
2. If you don't have a GitHub account, make it on [Github Sign In](https://github.com/signup).
3. Enter in you Github account using VsCode. You can get help using this [Youtube Video](https://www.youtube.com/watch?v=4Q9PHRsfIvQ).

Once you finish to compile the patch sucefully. The final step is to put it online. You have a lot of options, the easy way, if you already have one website, is to upload all things inside the `webpatch` folder, created next to the `mypatch.pd` patch.

If you don't have any website, you can use the free github pages. It provides one website with the `<yourusername>.github.io`, for example, my user name is `charlesneimog` so my website will be `charlesneimog.github.io`. If you are one completly noob, you can use VsCode to do that. See the video below:

<h2 style="text-align: center"> I will upload one video soon!</h2>

I hope that is can be usefull.

-----------------------------------
## <h2 style="text-align: center"><b>Making a html GUI</b></h2>
-----------------------------------

This is the hard part of the process, and for now you must know how to use `html`. I will soon release some templates for 
