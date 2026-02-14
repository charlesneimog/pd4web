In this section I assume that `pd4web` is already installed in your machine. If this is not the case, check the [install](install.md) section first than come back here.

In this section I will explain how to organize your project to compile your patch. It's not much different from what you're probably already doing, but you need to take special care with external files.

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

Abstractions files must be inside `Libs` folder and be called as `Libs/myabs`. You can also use `declare -lib Libs` them call you abstraction using `myabs`. 

## <h2 align="center">How to compile your patch?</h2>

To compile your `patch` using Pd you need to use the `pd4web` object. To access it you can create a new patch, create the `pd4web` object.

<p align="center" style="border-radius: 10px;">
        <img src="../../assets/pd-pd4web.png" width="40%" loading="lazy"  style="border-radius: 10px; box-shadow: 0px 4px 8px rgba(0, 0, 0, 0.2);">
</p>

After creation, you need to follow this step by step.

- 1) First choose your patch clicking in the green button.
- 2) In the yellow/orange section choose the memory size and the zoom level:
    - Memory: The number for the memory depends on the size of your patch, 32 for example is able to run very small patches, with few `osc~` on it, but if you have big objects like `else/plaits~` this will be a low number, use for example 512 if you have a lot of `else/plaits~` copies. 
    - Zoom: If you render your patch, this will control the size of it on the Web. Normally without zoom, the patch is very small, in most my use I put 2 here.

- 3) After configuration, compile your patch clicking in the `red` button. This will take some time, and sometimes could appear to be freeze, do not close Pd, wait until you see the message `[pd4web] Build completed successfully!`.
- 4) After build the patch, run the server using the `blue` button. This will open a website in the browser to you test you site before upload it on some public page (check [GitHub Upload](../upload/github) and [CloudFare Upload](../upload/cloudflare) to see how to do this).

You can also check the Template section to understand how to use [templates](templates/index.md).
