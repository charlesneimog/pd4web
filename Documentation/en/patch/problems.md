In this section, I will explain some problems related with configuration of `pd4web` and how to solve then.

This problems are seem from browser console. You need to open your browser, 

### Memory problems

You need to define the size of memory of your patch, this is pretty simple. Big patches needs more memory, small patches needs less memory.

If you run a big patch with a small memory size you will get one error like this:

```
Aborted(Cannot enlarge memory arrays to size 134225920 bytes (OOM).
```

If you see this, you need to increase the memory size. You can do this in the `pd4web` configuration file. 

### Compilation problems

If you have problems to compile, the easiear way to solve is to report what is the problem using the GitHub issues. 

### `flutex.c` problems

These problems are common when using some set of objects. If this happens, probably is because you are using some object that I don't use in my patches. So, you need to report this problem to me and I will try to solve this. 

To report the problem you must share the patch that is causing the problem, without the patch is impossible to solve the problem. You can always reduce the patch to the minimum size that is causing the problem. 
