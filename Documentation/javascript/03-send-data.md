# Send Data

If you want to send data to a Pd patch, you can use the following functions:

### `Pd4Web.sendBang`

Send a `bang` to Pd.

```javascript
Pd4Web.sendBang("myreceiver")
```

The `bang` will be received by `[r myreceiver]`.

### `Pd4Web.sendFloat`

Send a number to Pd.

```javascript
Pd4Web.sendFloat("myreceiver", 5)
```

To receive this use one object `[r myreceiver]` in Pd.

### `Pd4Web.sendSymbol`

Send a symbol to Pd.

```javascript
Pd4Web.sendSymbol("myreceiver", "mysymbol")
```

To receive this use one object `[r myreceiver]` in Pd.

### `Pd4Web.sendList`

Send a list to Pd.

```javascript
Pd4Web.sendList("myreceiver", [5, "mysymbol"])
```

To receive this use one object `[r myreceiver]` in Pd.

### `Pd4Web.sendFile`

!!! warning "Alpha function, report on errors"

`Pd4Web` uses an internal file system within the [AudioWorklet](https://developer.mozilla.org/en-US/docs/Web/API/AudioWorklet). This is great because it keeps everything safe and secure, but it also means that to load audio files, text files, or anything else inside Pd, you first need to send those files into the `Pd4Web` file system. To do that, you must send the file’s binary data.

For example, suppose your HTML page includes:

```html
<input id="someAudioInput" type="file" accept="audio/*" />
```

You can then use this JavaScript code:

```javascript
document.getElementById("someAudioInput").addEventListener("change", async (e) => {
    const file = e.target.files[0];
    if (!file) return;
    const arrayBuffer = await file.arrayBuffer();
    Pd4Web.sendFile(arrayBuffer, file.name);
});
```

This is how you can use upload files in your PureData Patch.

## Pure Data arrays

The array must already exist in the open Pure Data patch, for example as
`[array define samples]`.

### `Pd4Web.readArray`

Read every value from a named Pure Data array. The returned value is a
`Float32Array`, so it can be used directly as audio sample data.

```javascript
const samples = Pd4Web.readArray("samples");

if (samples) {
    console.log(samples);
}
```

If no patch is open or the named array does not exist, `readArray` logs an
error and returns `undefined`.

### `Pd4Web.writeArray`

Write an array of numbers or a `Float32Array` into a named Pure Data array.
The Pure Data array is automatically resized to match the number of supplied
samples.

```javascript
const samples = new Float32Array([0, 0.25, 0.5, 0.25, 0]);
const written = Pd4Web.writeArray("samples", samples);

if (!written) {
    console.error("Could not write the samples");
}
```

Audio decoded with the Web Audio API can be written directly:

```javascript
const samples = audioBuffer.getChannelData(0);
Pd4Web.writeArray("samples", samples);
```

`writeArray` returns `true` on success. If no patch is open or the named array
does not exist, it logs an error and returns `false`. Pure Data clips a resize
of zero samples to an array size of one.
