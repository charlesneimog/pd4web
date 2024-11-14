---
hide:
 - toc
---
<style>
  .md-typeset h1,
  .md-content__button {
    display: none;
  }
</style>

<h2 align="center">Template <code>2</code>: Hand detection</h2>

In this template you have access to use the library `ml5` from javascript. It is based on the library `tensorflow.js` and it is used to create machine learning models in the browser. In this case, we are going to use the `handpose` model to detect the hands in the live-video. You can read more about it on [ml5 website](https://docs.ml5js.org/#/reference/handpose) to understand how it works. 

The result will be something like this:

---

<p align="center">
  <img src="../assets/ml5.png" width="50%" style="border-radius: 10px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);">
</p>

<p align="center" markdown>
    :octicons-download-16: [Download Patch example](../patches/template-2.pd)
</p>

<p align="center" markdown><i>Ilustration, you will not see the green rects on the screen!</i></p>
    
<h2 align="center">How must be your patch</h2>


You patch must have some receivers using the `L-` and `R-` preffixes to receive the data from the hands followed by the number of the position you want, following the image below:

<p align="center">
  <img src="../assets/hand-pose.png" width="80%" style="border-radius: 10px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);">
</p>

<p align="center" markdown>
    :octicons-download-16: [Download Patch example](../template-2.pd)
</p>

For example, to receive the data from where is the `INDEX_FINGER_TIP` for the `LEFT` hand you must use the object `[r L-8]` in your patch.
