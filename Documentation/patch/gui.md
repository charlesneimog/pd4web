<p align="center">
Here we list all the objects that are supported by the GUI module and how to use them inside your web patch.
</p>

<div class="grid cards" markdown>

-   :material-gesture-double-tap:{ .lg .middle } __Light theme__ 
    <p align="center">
        <img src="../gui-light.png" alt="gui" width="90%" style="border-radius: 10px" box-shadow: "0 0 10px rgba(0, 0, 0, 0.3)">
    </p>


-   :material-gesture-double-tap:{ .lg .middle } __Dark theme__
    <p align="center">
        <img src="../gui-dark.png" alt="gui" width="90%" style="border-radius: 10px" box-shadow: "0 0 10px rgba(0, 0, 0, 0.3)">
    </p>
</div>

---

!!! warning "`pd4web` just render GUI objects"
    `pd4web` don't render objects boxes, object connections or messages.

## <h3 align="center">Supported Gui Objects</h3>

- `bng`: Bang object;
- `tgl`: Toggle object;
- `nbx`: Number box object;
- `vsl`: Vertical slider object;
- `hsl`: Horizontal slider object;
- `vradio`: Vertical radio object;
- `hradio`: Horizontal radio object;
- `vu`: Vertical vu object;
- `text`: Comment object;
- `floatatom`: Number box object;
- `symbolatom`: Symbol box object;
- `listatom`: List box object;

---

## <h3 align="center">`pdlua` GUI objects</h3>

!!! tip "All objects created using `pdlua` *are supported*! :)"

`pd4web` supports any GUI object implemented with **pdlua**.
To use it, place the corresponding `*.pd_lua` file in the `Libs/` directory of your patch.

For example, the `show~` object by Ben Wes is implemented in `pdlua`. To use it with `pd4web`, download `show~.pd_lua` from the repository and copy it into the `Libs/` folder inside your patch directory. See the [show~](https://github.com/ben-wes/pdlua-show_tilde){:target="_blank" rel="noopener"} object here.

Also objects from [`pd-saf`](https://github.com/EL-LOCUS-SOLUS/pd-saf){:target="_blank" rel="noopener"}, [`pd-bhack`](https://github.com/EL-LOCUS-SOLUS/pd-bhack){:target="_blank" rel="noopener"}, and many others, no need to extra configurations.

<div class="grid cards" markdown>
-   :material-gesture-double-tap:{ .lg .middle } __`saf.panning`, `keyboard` and others objects__
    <p align="center">
        <img src="../../assets/gui.png" alt="gui" width="90%" style="border-radius: 10px" box-shadow: "0 0 10px rgba(0, 0, 0, 0.3)">
    </p>


-   :material-gesture-double-tap:{ .lg .middle } __`diamond` using Harry Partch__ 
    <p align="center">
        <img src="../../assets/gui2.png" alt="gui" width="90%" style="border-radius: 10px" box-shadow: "0 0 10px rgba(0, 0, 0, 0.3)">
    </p>
</div>

