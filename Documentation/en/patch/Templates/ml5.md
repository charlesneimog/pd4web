In this template you have access to use the library `ml5` from javascript. It is based on the library `tensorflow.js` and it is used to create machine learning models in the browser. In this case, we are going to use the `handpose` model to detect the hands in the video. Check the video from the [ml5 website](https://docs.ml5js.org/#/reference/handpose) to understand how it works. 

You patch must have some receivers using the `L-` and `R-` preffixes to receive the data from the hands followed by the number of the position you want, following the image below:

<p align="center">
  <img src="../hand-pose.png" width="80%" style="border-radius: 10px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);">
</p>

For example, to receive the data from where is the `INDEX_FINGER_TIP` for the `LEFT` hand you must use the object `[r L-8]` in your patch.
