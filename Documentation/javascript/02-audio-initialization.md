# Initializing Audio Manually

## `Pd4Web.init`

When you open the patch but the patch does not explicitly define a `soundToggleId`, you must initialize audio manually. This must be done by click user event. For example:

```javascript
document.addEventListener(
    "click",
    async () => {
        Pd4Web.init();
    },
    { once: true }, // Ensures this only runs once
);
```

!!! warning "Running `init` outside of click event"
    [The browser does not allow audio to play](https://developer.mozilla.org/en-US/docs/Web/Media/Guides/Autoplay#autoplay_availability) outside of a click event.

## `Pd4Web.toggleAudio`

`toggleAudio` is similar to `init`, but it can also pause the audio processing. If not using `Pd4Web.init()`, the first `Pd4Web.toggleAudio` must be triggered inside a click event.

```javascript
document.addEventListener(
    "click",
    async () => {
        Pd4Web.toggleAudio();
    },
);
```
