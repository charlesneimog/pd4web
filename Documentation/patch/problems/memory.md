<style>
  .md-typeset h1,
  .md-content__button {
    display: none;
  }
</style>

<h2 align="center">Memory problems</h2>

!!! bug "How to report?"
    <p style="font-size: 18px">Create a new issue on [Github](https://github.com/charlesneimog/pd4web/issues){target="_blank}" or use [Google Forms](https://forms.gle/qS7YX4QzrUKNXGkU7){target="_blank"}.</p>


You need to define the size of memory of your patch, this is pretty simple. Big patches needs more memory, small patches needs less memory.

If you run a big patch with a small memory size you will get one error like this:

!!! danger "Aborted(Cannot enlarge memory arrays to size 134225920 bytes (OOM)."

If you see this, you need to increase the memory size. You can do this in the `pd4web` configuration, using the memory input on Pd patch or the `--memory` flag in the command line.