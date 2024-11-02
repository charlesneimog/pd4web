# JavaScript Module

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

## Send data to Pd

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

## Receive Data

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


## Midi

- TODO

## Smartphones Sensors

- TODO

