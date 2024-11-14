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
