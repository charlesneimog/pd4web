<p align="center">
  <h1 align="center">PdWebCompiler</h1>
  <p align="center">
    <a href="https://github.com/plugdata-team/plugdata/wiki">
      <img src="https://raw.githubusercontent.com/charlesneimog/PdWebCompiler/main/docs/assets/icon-light.svg" alt="Logo">
    </a>
  </p>
  <h4 align="center">PdWebCompiler: Running PureData Patches in Browsers with Web Audio</h4>
</p>

<p align="center">
  <a href="https://github.com/charlesneimog/PdWebCompiler/releases/latest"><img src="https://img.shields.io/github/release/charlesneimog/PdWebCompiler?include_prereleases" alt="Release"></a>
  <a href="https://pypistats.org/packages/pd2wasm"><img src="https://img.shields.io/pypi/dm/pd2wasm" alt="Downloads"></a>
  <a href="https://pypistats.org/packages/pd2wasm"><img src="https://img.shields.io/pypi/pyversions/pd2wasm" alt="Version"></a>
</p>

<p align="center">
  <a href="https://img.shields.io/pypi/pyversions/pd2wasm"><img src="https://img.shields.io/badge/platforms-macOS%20%7C%20Windows%20%7C%20Linux-green" alt="License"></a>
</p>

<p align="center">
  <a href="https://github.com/charlesneimog/PdWebCompiler/actions/workflows/Test.yml"><img src="https://github.com/charlesneimog/PdWebCompiler/actions/workflows/Test.yml/badge.svg" alt="License"></a>
</p>

<p align="center">
  <h2 align="center">Intro</h2>
  <br>
</p>

`PdWebCompiler` empowers you to execute PureData patches directly in web browsers using advanced Web Audio technologies. This tool distinguishes itself from alternative approaches like [hvcc](https://github.com/Wasted-Audio/hvcc) and [WebPd](https://github.com/sebpiq/WebPd) by compiling the source code of [libpd](https://github.com/libpd/libpd), a feat made possible largely due to the contributions of  [claudeha](https://github.com/claudeha). With `PdWebCompiler`, you can compile virtually any PureData vanilla patch, along with a wide list of externals.

##### ⚠️ Warning: Brace for Breaking Changes ⚠️

Given that this project is relatively new, it's important to note that there may be substantial breaking changes in the near future.

#### Details and Documentation

For comprehensive details and documentation, please visit the [Docs](charlesneimog.github.io/PdWebCompiler) section of this project.



Join us in contributing to this repository! Explore the primary areas for your valuable contributions:

* The `pd2wasm/lib/` folder, which contains files with special steps for compiling certain `externals/libraries`.
* The `pd2wasm/src/template.c`, used to PdWebCompiler to build the `main.c` file.
* The `pd2wasm/pd2wasm.py`, which configures the `main.c` file.
