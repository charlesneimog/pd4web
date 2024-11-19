---
hide:
 - toc
---


# <h1 align="center">Template <code>2</code>: Hand detection</h1>

In this template you have access to the library `ml5.js`. It is based on the library `tensorflow.js` and it is used to use machine learning models inside the browser. In this case, we are going to use the `handpose` model to detect the hands in the live-video. You can read more about it on [ml5 website](https://docs.ml5js.org/#/reference/handpose).

The result will be something like this:

---

<p align="center">
  <img src="../assets/ml5.png" width="50%" style="border-radius: 10px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); cursor: pointer;" onclick="window.open('../templates/ml5', '_blank');">
</p>

<p align="center" markdown>
    :octicons-download-16: [Download Patch example](../patches/template-2.pd)
</p>
    
## <h2 align="center">What should your patch include?</h2>

You patch must have some receivers using the `L-` and `R-` preffixes to receive the data from the hands followed by the number of the position you want, following the image below:

<p align="center">
  <img src="../assets/hand-pose.png" width="80%" style="border-radius: 10px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);">
</p>

<p align="center" markdown>
    :octicons-download-16: [Download Patch example](../patches/template-2.pd)
</p>

For example, to receive the data from where is the `INDEX_FINGER_TIP` for the `LEFT` hand you must use the object `[r L-8]` in your patch.

---

From these receivers, you will get two numbers, `x` and `y`, inside a list. These numbers represent the position of your hand relative to your real-time video.

<div class="grid cards" markdown>
  - __X Position__: `0` is the top and `1` is the bottom.
  - __Y Position__: `0` is the left and `1` is the right.
</div>

In summary, `[x, y]` defines the hand's position within a coordinate system where `(0,0)` is the top-left corner and `(1,1)` is the bottom-right corner of the image.

---

If you download the example patch and use the information above, you'll see that we can use the following object to detect if your finger is in the **top-left** corner of the image:

```
[expr if($f1 < 0.2 && $f2 < 0.2, 1, 0)]
```

In this expression:
- `$f1` represents the `x` coordinate (vertical position).
- `$f2` represents the `y` coordinate (horizontal position).

This expression checks if:
- `$f1` is less than `0.2` (meaning your hand is near the **top** of the image).
- `$f2` is less than `0.2` (meaning your hand is near the **left** side of the image).

If both conditions are true, the output will be `1` (indicating your finger is in the top-left corner). Otherwise, the output will be `0`.
