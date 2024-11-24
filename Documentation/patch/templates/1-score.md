---
hide:
 - toc
---

# <h1 align="center">Template <code>1</code>: Patch + Score</h1>

In this template you have access to a patch that receives image files (from scores) and renders it side by side with your patch. The result will be something like this:

!!! tip "Click in the image to open the example in a new tab" 

<p align="center">
  <img src="../assets/score.png" width="80%" style="border-radius: 10px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); cursor: pointer;" onclick="window.open('../templates/score', '_blank');">
</p>

<p align="center" markdown>
    :octicons-download-16: [Download Patch Template](../patches/template-1.pd)
</p>

!!! info "Compile using the template mode `1`"

---

## <h2 align="center">What should your patch include?</h2>


You make your patch and the score file can be set using the object `[s pd4web-score]`. I recomend to use svg files, but you can use any image file (.png, .jpeg). 

* **These files must be inside the `WebPatch` folder**. For example, if the score image is called `score-measure1.png`, you will send to `[s pd4web-score]` the message `symbol score-measure1.png`. 

!!! danger "Don't forget the `symbol` word before the file"

---
    
## <h3 align="center">Extra options</h3>


After compile the patch, you will see that, inside the folder `WebPatch`, will have a file called `bula.md`. Inside this file, you can write some description about your piece. The file uses markdown to render, you can check the complet syntax in https://www.markdownguide.org/basic-syntax/. Below the example of main command:

``` md
# Title 1
## Title 2
### Title 3

**bold text**
*italic text**

* this one item
* this is another item
```


