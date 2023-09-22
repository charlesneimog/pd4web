# Documentation for Devs

<p align="center"> Thank you to look this page! </p>

## <h2 align="center"> **pd2wasm** </h2>

As you must noted, `PdWebCompiler` is a simple python script, but it has some boring details. We can split the `pd2wasm` package in three parts.

1. The main script, `PdWebCompiler.py`.
2. The folder `externals` with the main file `ExternalClass.py` and the files for each library.
3. The folder `libs` with the main file `main.py` and files for each library.

The idea is simple: 

* **To add a new simple PureData Library**: You must add a new entry in the `Externals.yaml`.
* **To add a new complex PureData Library**: You must add a new entry in the `Externals.yaml` and a newfile in `externals` folder that define the ExtraFunction 



* **To add a new simple PureData Library**: You must add a new entry in the `Externals.yaml`.
* **Add new Dynamic Library**: You must add a new file with all steps to the compilation 


