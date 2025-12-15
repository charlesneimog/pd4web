<p align="center">
Here we list all the objects that are supported by the GUI module and how to use them inside your web patch.
 
</p>

!!! danger "Set Senders and Receivers for GUI Objects"
    Again, to use **any** GUI object (except for `vu`), you **must** configure senders and receivers. This ensures all objects function properly and saves you from spending excessive time debugging your patch.
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

!!! warning "`pd4web` just render GUI objects"
    `pd4web` don't render objects boxes, object connections, messages, or **anything else**. 

## <h3 align="center">Supported Gui Objects</h3>

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
- `floatatom`: Number box object;
- `symbolatom`: Symbol box object;
- `listatom`: List box object;

From external libraries:

- `else/knob`: Knob object;
- `else/keyboard`: Keyboard object;

---

## <h3 align="center">Pdlua + pd4web</h3>



`pd4web` supports any GUI object implemented with **pdlua**.
To use one, place the corresponding `*.pd_lua` file in the `Libs/` directory of your patch.

For example, the `show~` object by Ben Wes is implemented in pdlua. To use it with `pd4web`, download `show~.pd_lua` from the repository and copy it into the `Libs/` folder inside your patch directory. You can download it here [show~](Here is a clearer and tighter version:

For example, the `show~` object by Ben Wes is implemented in pdlua. To use it with `pd4web`, download `show~.pd_lua` from the repository and copy it into the `Libs/` folder inside your patch directory. 

See the [show~](https://github.com/ben-wes/pdlua-show_tilde) object here.




