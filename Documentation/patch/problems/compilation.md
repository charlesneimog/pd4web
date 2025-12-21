<style>
  .md-typeset h1,
  .md-content__button {
    display: none;
  }
</style>

<h2 align="center">Compilation problems</h2>

If you have problems compiling your patches the easiest way to solve them is to report the problem using the GitHub issues.

!!! bug "How to report?"
    <p style="font-size: 18px">Create a new issue on [Github](https://github.com/charlesneimog/pd4web/issues){target="_blank"} or use [Google Forms](https://forms.gle/qS7YX4QzrUKNXGkU7){target="_blank"}.</p>

---
<div class="grid cards" markdown>
-   :construction: __Object not found__:
    -   __Solution:__
        -   Check if the library of that object is supported [Supported libraries](../../../libraries).
        -   Check how `pd4web` detects objects in [Externals Objects](../../externals). If the library is not supported remove the object; if the library is supported, and you did what is in [Externals Objects](../../externals), report the issue.
-   :construction: __Object not supported__:
    -   __Solution:__ You must remove the object not supported.
</div>

<div class="grid cards" markdown>
-   :construction: __`pd4web` can not process some object__:
    -   __Solution:__ You must report the problem and the object. I probably never used this object.
-   :construction: __Cmake configuration problems__:
    -   __Solution:__ Try to delete the `Pd4Web`, `build` and `Webpatch`, then compile it again, on errors, report the issue.
</div>

<div class="grid cards" markdown>
-   :construction: __Cmake compilation problems__:
    -   __Solution:__ Report the issue, these errors should not happen!
</div>

---
