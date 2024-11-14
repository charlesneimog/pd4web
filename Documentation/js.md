# <h1 align="center"><code>Pd4Web</code> JavaScript Module</h1>

The `pd4web` project includes a WebAssembly module named `Pd4Web`. In the `index.html` file inside the `WebPatch` directory, `Pd4Web` is always loaded as follows:

``` javascript

var Pd4Web = null;

Pd4WebModule().then((Pd4WebModulePromise) => {
    Pd4Web = new Pd4WebModulePromise.Pd4Web();
    // after this you can use Pd4Web to interact with Pd
});
```

After loading, you can use various functions to enable interaction between `Pd` and your website.

## <h2 align="center">Requirements</h2>

`Pd4Web` uses the `WebAudio` API to communicate with `Pd`. This means that you need to have a browser and a server that supports some features.

### <h3 align="center">Cross-Origin Isolation</h3>

COI is a security feature that allows you to restrict how your website interacts with other websites. If COI is not enabled, you will see an error message related with `SharedArrayBuffer` being not defined.

`Pd4Web` uses the script `pd4web.threads.js` to enable COI if it's not enabled from the server. But there is some limitations, first of all you need to create a redirect `index.html` file from the root of the WebSite to an specific folder. This is necessary because the `pd4web.threads.js` script needs to be in the same folder as the `index.html` file. I am yet researching a better way to do this.

--- 

## <h2 align="center">Using Pd4Web Audio Backend</h2>

`Pd4Web` is mainly designed for Pd users, people that actually don't code. So here some options to use the `Pd4Web` audio backend.

### Disabling Gui Interface

To compile your patch without the GUI interface, you need to use the `-nogui` flag. This will disable the GUI interface and will use the `Pd4Web` audio backend.



## <h2 align="center">Send Data to Pd patch</h2>

These are pretty simple function;

### `Pd4Web.sendFloat`

Send number to Pd. 

``` js 
Pd4Web.sendFloat("myreceiver", 5)
```

To receive this use one object `[r myreceiver]` in Pd.

### `Pd4Web.sendSymbol`


Send symbol to Pd. 

``` js 
Pd4Web.sendSymbol("myreceiver", "mysymbol")
```

To receive this use one object `[r myreceiver]` in Pd.

### `Pd4Web.sendList`

Send list to Pd. 

``` js 
Pd4Web.sendList("myreceiver", [5, "mysymbol"])
```

To receive this use one object `[r myreceiver]` in Pd.

--- 

## <h2 align="center">Receive Data from Pd patch</h2>

You need to define something to be called when Pd receive the thing you want.

### `Pd4Web.onBangReceived`

``` js 
Pd4Web.onBangReceived("mybang", function () { 
    console.log("Received a bang");
});
```
To execute this you need to send a bang from Pd using `[s mybang]`.

### `Pd4Web.onFloatReceived`

``` js 
Pd4Web.onFloatReceived("myfloat", function (f) { 
    console.log("Received " + f)
});
```
To execute this you need to send a float from Pd using `[s myfloat]`.

### `Pd4Web.onSymbolReceived`

``` js 
Pd4Web.onSymbolReceived("mysymbol", function (s) { 
    console.log("Received " + s)
});
```
To execute this you need to send a symbol from Pd using `[s mysymbol]`.

### `Pd4Web.onListReceived`

``` js 
Pd4Web.onListReceived("mylist", function (mylist) { 
    console.log("Received list");
});
```
To execute this you need to send a list from Pd using `[s mylist]`.

--- 

On the Score Follower example in the main page, I use `Pd4Web.onFloatReceived`: 

``` js

Pd4Web.onFloatReceived("score-render", function (f) {
    var svgId = notes[f].getSVGId();
    var svgElement = document.getElementById("vf-" + svgId);
    var noteheadElement = svgElement.querySelector(".vf-notehead");
    var pathElement = noteheadElement.querySelector("path");
    pathElement.setAttribute("fill", "red");
});
```

In this example, always that the object `o.scofo~` receive a float from the `score-render` sender I change the color of the notehead of the note that is being detected from the `o.scofo~` object to red. Check the example [here](./tests/OScofo/), the [source code](https://github.com/charlesneimog/pd4web/blob/99658fb8d02a2427c9ac957915bd89188e641ba1/Documentation/tests/OScofo/WebPatch/index.html#L157), and the [Pd patch](./tests/OScofo/main.pd).

--- 
## Midi In/Out

- TODO

---
## Smartphones Sensors

- TODO

