The `pd4web` project includes a WebAssembly module named `Pd4Web`. In the `index.html` file inside the `WebPatch` directory, `Pd4Web` is always loaded as follows:

``` javascript

var Pd4Web = null;

Pd4WebModule().then((Pd4WebModulePromise) => {
    Pd4Web = new Pd4WebModulePromise.Pd4Web();
    // after this you can use Pd4Web to interact with Pd
});
```

After loading, you can use various functions to enable interaction between `Pd` and your website.

--- 
<div class="grid cards" markdown>
-   :simple-javascript: [__Change Patch Parameters__](pd.new_object/methods.md)
-   :simple-javascript: [__Receive Data from Pd patch__](receive.md)

</div>

---

<div class="grid cards" markdown>
-   :simple-javascript: [__Gui Interface__](engine.md/#disabling-gui-interface)
-   :simple-javascript: [__MIDI__](midi.md)

</div
>

