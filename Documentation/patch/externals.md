External objects need to be explicitly detected by `pd4web`. This process is **not** fully automatic. You must provide enough information for `pd4web` to know which libraries your objects come from—especially when different libraries contain objects with the same name.

<h2>How detection works</h2> 

`pd4web` can detect externals in two ways:

- By using a library prefix, such as `else/knob` or `else/keyboard`.
- By declaring the libraries you use, for example:  
  `declare -lib else` or `declare -lib timbreIDLib`.

---

If you do not specify this information, external objects will fail to load, and you will likely see errors such as:  
`knob: object can't be found`.

!!! danger "`Pd4Web` does not recognize the libraries that you load on startup on PureData!"

!!! danger "`Plugdata` has external libraries"
    `Plugdata` uses externals libraries like, `else`, `cyclone`, `pdlua`, `Gem` and others.


