<p align="center">
Here we list all the objects that are supported by the GUI module and how to use them inside your web patch.
</p>

---

!!! warning "pd4web just render GUI objects"
    `pd4web` just render the GUI objects. We don't aim to make Pd available in the browser. We aim to make projects that use Pd available in the browser, projects like pieces, algorithms, or audio engines.

## <h2 align="center">Supported Gui Objects</h2>

For now, just these objects are supported:

- `bng`: Bang object;
- `tgl`: Toggle object;
- `nbx`: Number box object;
- `vsl`: Vertical slider object;
- `hsl`: Horizontal slider object;
- `vradio`: Vertical radio object;
- `hradio`: Horizontal radio object;
- `vu`: Vertical vu object;
- `cnv`: Canvas object;
- `text`: Comment object;

From external libraries:

- `else/knob`: Knob object;
- `else/keyboard`: Keyboard object;

You can check these objects working in the [examples](../../../tests/gui) folder.

---
## <h2 align="center">How to use this objects?</h2>

`pd4web` intercepts the messages sent by the objects and sends them to the web patch. Connections can't be intercepted, so you need to use `send`, `s` or `receive`, `r` objects to connect the GUI objects. This is how you can use the `toggle` object:

!!! danger "Don't use connections"
    The connections between objects are not intercepted by `pd4web`. You need to use `send` and `receive` objects to connect the GUI objects.

<p align="center">
    <img src="../gui.png" alt="gui" width="100%" style="border-radius: 10px">
</p>

