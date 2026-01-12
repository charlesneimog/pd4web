In this section, I will explain how to organize your project to compile your patch. It’s not much different from what you’re probably already doing, but you need to take special care with external files.

## <h2 align="center">Folder Structure</h2>

`pd4web` runs inside a container, which means it has no knowledge of you, your browser, the website you are using, or the files on that site. For `pd4web` to access anything, such as your files, it automatically creates a **Virtual File System (VFS)**. 

The VFS works like a USB drive: it contains only the files you explicitly include, and nothing else from your environment. 

`pd4web` generates this virtual "pendrive" automatically, but it makes some assumptions about where your files are located. The organization of the VFS follows this path structure:

!!! warning
    Be careful with upper and lower case letters.

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

* The `Audios` folder is for all audio files.  
* The `Libs` folder stores abstractions, text files, or any other relevant resources.  
* The `Extras` folder contains any additional files or miscellaneous items.

After compiling your patch, an `index.html` file will be created in the **root** of the project, along with a new folder called `WebPatch`.  

All essential content of your patch — **Audios**, **Libs**, and **Extras** — is stored inside the `WebPatch` folder. Your files are packed into a single file called `pd4web.data`, which acts as a virtual "pendrive" containing everything your patch needs.  

In addition, the `WebPatch` folder includes other files required to run Pd, such as `pd4web.js`, `pd4web.threads.js`, and more.

## <h2 align="center">How to use Abstractions?</h2>

Abstractions files must be inside `Libs` folder and be called as `Libs/myabs`.

## <h2 align="center">Compile your patch</h2>

To compile your `patch` using Pd you need to use the `pd4web` object and its help patch. To access it you can create a new patch, create the `pd4web` object.

<p align="center" style="border-radius: 10px;">
        <img src="../../assets/pd-pd4web.png" width="40%" loading="lazy"  style="border-radius: 10px; box-shadow: 0px 4px 8px rgba(0, 0, 0, 0.2);">
</p>

### Options

Follow the numbers to compile your patch:


- Choose your patch file using the `green` button.

* From the `yellow` section, choose the memory size and the zoom level.
* Compile your patch using the `red` button.
* Run the server using the `blue` button.

You can also check the Template section to understand how to use [templates](templates/index.md).
