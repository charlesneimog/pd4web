<p align="center">
Here we list all the objects that are supported by the GUI module and how to use them inside your web patch.
</p>

<div class="grid cards" markdown>

-   :material-gesture-double-tap:{ .lg .middle } __With `plaits~` from else__    
    <p align="center">
        <img src="../gui-light.png" alt="gui" width="90%" style="border-radius: 10px" box-shadow: "0 0 10px rgba(0, 0, 0, 0.3)">
    </p>
    <p align="center" markdown>
        :octicons-browser-16: [Check the website](../../tests/gui)
    </p>


-   :material-gesture-double-tap:{ .lg .middle } __All Gui Objects__
    <p align="center">
        <img src="../gui-dark.png" alt="gui" width="90%" style="border-radius: 10px" box-shadow: "0 0 10px rgba(0, 0, 0, 0.3)">
    </p>
    <p align="center" markdown>
        :octicons-browser-16: [Check the website](../../tests/gui2)
    </p>
</div>

---

!!! warning "pd4web just render GUI objects"
    `pd4web` don't render objects boxes, object connections, messages, or anything else. Just the GUI objects.

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

---
## <h2 align="center">How to use this objects?</h2>

Thanks to `libpd`, `pd4web` intercepts the messages sent by the objects and sends them to the web patch. Connections can't be intercepted, so you need to use `send`, `s` or `receive`, `r` objects to connect the GUI objects. This is how you can use the `toggle` object:

!!! danger "Don't use connections"
    The connections between objects are not intercepted by `pd4web`. You need to use `send` and `receive` objects to connect the GUI objects.

<p align="center">
    <img src="../Templates/assets/gui.png" alt="gui" width="80%" style="border-radius: 10px">
</p>

