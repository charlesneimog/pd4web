`Pd4Web` is mainly designed for Pd users, people that actually don't code. So here some options to use the `Pd4Web` audio backend.

### <h3 align="center">Disabling Gui Interface</h3>

To compile your patch without the GUI interface, you need to use the `-nogui` flag. This will disable the GUI interface and will use the `Pd4Web` audio backend.

When disabling the Gui interface, you need to init `pd4web` for yourself. 

!!! important "Mic input use"
    Always that you use the Mic input (`adc~`), you must init `pd4web` from a user gesture (click).
    
For example, in this example below I init `pd4web` from a click anywhere in the main page.

``` javascript
var Pd4Web = null; // Pd4Web object must be declared in the global scope and the name must be Pd4Web
Pd4WebModule().then((Pd4WebModulePromise) => {
    Pd4Web = new Pd4WebModulePromise.Pd4Web();
});

document.addEventListener(
    "click",
    async () => {
        Pd4Web.init();
    },
    { once: true }, // Ensures this only runs once
);
```
