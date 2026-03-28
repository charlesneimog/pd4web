# Receive Data

To receive data from Pd you can use `callback` functions. They are defined by PureData `selector` and must be defined precisely.

### `Pd4Web.onBangReceived`

Registers a callback function that is triggered when a `bang` is received.

```javascript
Pd4Web.onBangReceived("r-test", (r) => {
    console.log("Received bang from", r);
});
```

When the patch receives a `bang` at `[r-test]`, the callback function is called.

### `Pd4Web.onFloatReceived`

Registers a callback function that is triggered when a `float` is received.

```javascript
Pd4Web.onFloatReceived("r-test", (r, f) => {
    console.log("Received float:", f);
});
```

When the patch receives a `float` at `[r-test]`, the callback function is called.

### `Pd4Web.onSymbolReceived`

Registers a callback function that is triggered when a `symbol` is received.

```javascript
Pd4Web.onSymbolReceived("r-test", (r, s) => {
    console.log("Received symbol:", s);
});
```

When the patch receives a `symbol` at `[r-test]`, the callback function is called.

### `Pd4Web.onListReceived`

Registers a callback function that is triggered when a `list` is received.

```javascript
Pd4Web.onListReceived("r-test", (r, l) => {
    console.log("Received list:", l);
});
```

When the patch receives a `list` at `[r-test]`, the callback function is called.

### `Pd4Web.onMessageReceived`

Registers a callback function that is triggered when a `message` is received. On Pd,

```javascript
Pd4Web.onMessageReceived("r-test", (r, selector, list) => {
    console.log("Received message from:", r);
});
```

When the patch receives a `message` at `[r-test]`, the callback function is called.
