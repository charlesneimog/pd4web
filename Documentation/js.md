# JavaScript Module

`pd4web` has a WebAsssembly module called `Pd4Web`. If you see the `index.html` page inside `WebPatch` you will see that is it always loaded as: 

``` js

let Pd4Web = null;

Pd4WebModule().then((Pd4WebModulePromise) => {
    Pd4Web = new Pd4WebModulePromise.Pd4Web();
});


document
    .getElementById("Pd4WebAudioSwitch")
    .addEventListener("click", async () => {
        var clickableSpan = document.getElementById("Pd4WebAudioSwitch");
        if (clickableSpan.classList.contains("pd4web-sound-on")) {
            clickableSpan.classList.remove("pd4web-sound-on");
            Pd4Web.suspendAudio();
            clickableSpan.classList.add("pd4web-sound-off");
            return;
        }
        clickableSpan.classList.remove("pd4web-sound-off");
        clickableSpan.classList.add("pd4web-sound-loading");
        Pd4Web.init(); 
        clickableSpan.classList.remove("pd4web-sound-loading");
        clickableSpan.classList.add("pd4web-sound-on");
    });
```

After loaded you can you some functions to interact between `Pd` and your website.

--- 

### Send data to Pd

These are pretty simple function;

#### `Pd4Web.sendFloat`

Send number to Pd. 

``` js 
Pd4Web.sendFloat("myreceiver", 5)
```

#### `Pd4Web.sendSymbol`


Send symbol to Pd. 

``` js 
Pd4Web.sendSymbol("myreceiver", "mysymbol")
```

#### `Pd4Web.sendList`

Send list to Pd. 

``` js 
Pd4Web.sendList("myreceiver", [5, "mysymbol"])
```

--- 

### Receive Data

You need to define something to be called when Pd receive the thing you want.

#### `Pd4Web.onBangReceived`

``` js 
Pd4Web.onBangReceived("mybang", function () { 
    console.log("Received a bang");
});
```

#### `Pd4Web.onFloatReceived`

``` js 
Pd4Web.onFloatReceived("myfloat", function (f) { 
    console.log("Received " + f)
});
```

#### `Pd4Web.onSymbolReceived`

``` js 
Pd4Web.onSymbolReceived("mysymbol", function (s) { 
    console.log("Received " + s)
});
```


#### `Pd4Web.onListReceived`

``` js 
Pd4Web.onListReceived("mylist", function (mylist) { 
    console.log("Received list");
});
```

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

In this example, always that I receive a float from the `score-render` sender I change the color of the notehead of the note that is being detected from the `o.scofo~` object.


### Midi

No docs yet!

