In this section, I will explain some problems related with configuration of `pd4web` and how to solve then.

This problems are seem from browser console. To see the console you need to open the developer tools of your browser. You can do this pressing `F12` in your keyboard. Then check if the `console` tab is selected.

---

<h2 align="center">Memory problems</h2>

You need to define the size of memory of your patch, this is pretty simple. Big patches needs more memory, small patches needs less memory.

If you run a big patch with a small memory size you will get one error like this:

!!! danger "Aborted(Cannot enlarge memory arrays to size 134225920 bytes (OOM)."

If you see this, you need to increase the memory size. You can do this in the `pd4web` configuration, using the memory input on Pd patch or the `--memory` flag in the command line.

---

<h2 align="center">Compilation problems</h2>

If you have problems to compile, the easiear way to solve is to report what is the problem using the GitHub issues. 

Common problems:

- Object not found: _You must remove the object not found_.
- Pd4web can not process some object: _You must report the problem and the object_.
- Cmake configuration problems: _Try to delete the `Pd4Web`, `build` and `Webpatch`, then compile it again, on errors, report the issue._
- Cmake compilation problems;

<style>
    .problem {
        color: red;
    }
</style>

<table class="special-table">
    <thead>
        <tr>
          <th style="width: 30%">Problem</th>
          <th>Solution</th>
        </tr>
    </thead>
    <tbody>
        <tr>
          <td class="problem">Object not found</td>
          <td><em>Remove the object not found. If it is in a library report!</em></td> 
        </tr>
        <tr>
          <td class="problem">Pd4web can not process some object</td>
          <td><em>You must report the problem and the object.</em></td>
        </tr>
        <tr>
          <td class="problem">Cmake configuration problems</td>
          <td><em>Try to delete the <code>Pd4Web</code>, <code>build</code> and <code>Webpatch</code>, then compile it again, on errors, report the issue.</em></td>
        </tr>
        <tr>
          <td class="problem">Cmake compilation problems</td>
          <td><em>Report the issue, these errors should not happen!</em></td>
        </tr>
    </tbody>
</table>


---

<h2 align="center"><code>flutex.c</code> problems</h2>

These problems are common when using some set of objects. If this happens, probably is because you are using some object that I don't use in my patches. So, you need to report this problem to me and I will try to solve this. 

To report the problem you must share the patch that is causing the problem, without the patch is impossible to solve the problem. You can always reduce the patch to the minimum size that is causing the problem. 

Usually, these problems have an error related with `flutex.c` file.
