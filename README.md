<p align="center">
  <h1 align="center">pd2web</h1>
  <p align="center">
    <a href="https://github.com/plugdata-team/plugdata/wiki">
      <img src="https://raw.githubusercontent.com/charlesneimog/pd2web/main/docs/assets/icon-light.svg" alt="Logo">
    </a>
  </p>
  <h4 align="center">pd2web: Running PureData Patches in Browsers with Web Audio</h4>
</p>

<p align="center">
  <a href="https://github.com/charlesneimog/pd2web/releases/latest"><img src="https://img.shields.io/github/release/charlesneimog/pd2web?include_prereleases" alt="Release"></a>
  <a href="https://pypistats.org/packages/pd2web"><img src="https://img.shields.io/pypi/dm/pd2web" alt="Downloads"></a>
  <a href="https://pypistats.org/packages/pd2web"><img src="https://img.shields.io/pypi/pyversions/pd2web" alt="Version"></a>
</p>

<p align="center">
  <a href="https://img.shields.io/pypi/pyversions/pd2web"><img src="https://img.shields.io/badge/platforms-macOS%20%7C%20Windows%20%7C%20Linux-green" alt="License"></a>
</p>

<p align="center">
  <a href="https://github.com/charlesneimog/pd2web/actions/workflows/Test.yml"><img src="https://github.com/charlesneimog/pd2web/actions/workflows/Test.yml/badge.svg" alt="License"></a>
</p>

`pd2web` empowers you to execute PureData patches directly in web browsers using advanced Web Audio technologies. This tool distinguishes itself from alternative approaches like [hvcc](https://github.com/Wasted-Audio/hvcc) and [WebPd](https://github.com/sebpiq/WebPd) by compiling the source code of [libpd](https://github.com/libpd/libpd), a feat made possible largely due to the contributions of  [claudeha](https://github.com/claudeha). With `pd2web`, you can compile virtually any PureData vanilla patch, along with a wide list of externals.

##### ⚠️ Warning: Brace for Breaking Changes ⚠️

Given that this project is relatively new, it's important to note that there may be substantial breaking changes in the near future.

#### Details and Documentation

For comprehensive details and documentation, please visit the [Docs](https://charlesneimog.github.io/pd2web/) section of this project.



Join us in contributing to this repository! Explore the primary areas for your valuable contributions:

* The `pd2web/lib/` folder, which contains files with special steps for compiling certain `externals/libraries`.
* The `pd2web/src/template.c`, used to pd2web to build the `main.c` file.
* The `pd2web/pd2web.py`, which configures the `main.c` file.
