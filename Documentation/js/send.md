If you want to send data to a Pd patch, you can use the following functions:

### `Pd4Web.sendFloat`

Send number to Pd. 

``` js 
Pd4Web.sendFloat("myreceiver", 5)
```

To receive this use one object `[r myreceiver]` in Pd.

### `Pd4Web.sendSymbol`


Send symbol to Pd. 

``` js 
Pd4Web.sendSymbol("myreceiver", "mysymbol")
```

To receive this use one object `[r myreceiver]` in Pd.

### `Pd4Web.sendList`

Send list to Pd. 

``` js 
Pd4Web.sendList("myreceiver", [5, "mysymbol"])
```

To receive this use one object `[r myreceiver]` in Pd.

--- 