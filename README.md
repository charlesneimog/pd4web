<p align="center">
  <h1 align="center">pd4web</h1>
  <p align="center">
    <a href="https://github.com/plugdata-team/plugdata/wiki">
      <img src="https://raw.githubusercontent.com/charlesneimog/pd4web/main/docs/assets/favicon.png" alt="Logo">
    </a>
  </p>
  <h4 align="center">pd4web: Running PureData Patches in Browsers with Web Audio</h4>
</p>

<p align="center">
  <a href="https://github.com/charlesneimog/pd4web/releases/latest"><img src="https://img.shields.io/github/release/charlesneimog/pd4web?include_prereleases" alt="Release"></a>
  <a href="https://pypistats.org/packages/pd4web"><img src="https://img.shields.io/pypi/dm/pd4web" alt="Downloads"></a>
  <a href="https://pypistats.org/packages/pd4web"><img src="https://img.shields.io/pypi/pyversions/pd4web" alt="Version"></a>
</p>

<p align="center">
  <a href="https://img.shields.io/pypi/pyversions/pd4web"><img src="https://img.shields.io/badge/platforms-macOS%20%7C%20Windows%20%7C%20Linux-green" alt="License"></a>
</p>

<p align="center">
  <a href="https://github.com/charlesneimog/pd4web/actions/workflows/pd4web-tests.yml"><img src="https://github.com/charlesneimog/pd4web/actions/workflows/pd4web-tests.yml/badge.svg" alt="Tests"></a>
</p>

`pd4web` empowers you to execute PureData patches directly in web browsers using advanced Web Audio technologies. This tool distinguishes itself from alternative approaches like [hvcc](https://github.com/Wasted-Audio/hvcc) and [WebPd](https://github.com/sebpiq/WebPd) by compiling the source code of [libpd](https://github.com/libpd/libpd), a feat made possible largely due to the contributions of  [claudeha](https://github.com/claudeha). With `pd4web`, you can compile virtually any PureData vanilla patch, along with a wide list of externals.

#### Details and Documentation

For comprehensive details and documentation, please visit the [Docs](https://charlesneimog.github.io/pd4web/) section of this project.



Join us in contributing to this repository! Explore the primary areas for your valuable contributions:

* The `pd4web/lib/` folder, which contains files with special steps for compiling certain `externals/libraries`.
* The `pd4web/src/template.c`, used to pd4web to build the `main.c` file.
* The `pd4web/pd4web.py`, which configures the `main.c` file.
