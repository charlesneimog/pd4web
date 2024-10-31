---
hide:
  - navigation
  - toc
---

# Introduction

<p align="center"> Welcome to the <code>pd4web</code> documentation! </p>

<p align="center" markdown>
  ![Image title](./assets/icon-light.svg#only-light){ width="6%" }
  ![Image title](./assets/icon-dark.svg#only-dark){ width="6%" }
</p>

## <h2 align="center"> **What is pd4web?** </h2>

---

`pd4web` allows bring PureData functionality to browsers. This tool stands out from other approaches like [hvcc](https://github.com/Wasted-Audio/hvcc) (Command line that allows to compile to `wasm` and use it in JavaScript code) and [WebPd](https://github.com/sebpiq/WebPd) (WebSite that compiles PureData patches to `wasm`) because it compiles the [libpd](https://github.com/libpd/libpd) source, this is possible mainly because of the work of [claudeha](https://github.com/claudeha). With `pd4web`, command line provided by pd4web, you can run your patch **with externals** on Web Pages.


In addition to the evident advantages inherent in streamlining the development of entirely online audio applications through a fully visual approach, my personal scholarly interest as a composer centers on the creation of electroacoustic and live electronics works. This focus is underpinned by the aspiration to facilitate performer access to compositions without the need for configuring intricate PureData patches, replete with numerous libraries requirements and susceptible to errors arising from platform disparities. 

Additionally, a significant aspect of my research delves into the capacity of WebAudioApps to serve as an effective means for preserving works within the realm of live electronic music, because `pd4web` is restritive with the version used of the main libraries and repositories used in the compilation process.

-------------------------
### <h3 align="center"> **Examples** </h3>
-------------------------

<div class="container">
  <div class="card">
    <h3 class="card-title">Scofo Follower</h3>
    <img
      src="./tests/OScofo/patch.png"
      onclick="window.open('./tests/OScofo', '_blank')"
      class="card-img"
    />
    <p style="width: 80%; text-align: center">
      Show the use of the object <code>o.scofo~</code> with pd4web. Personal
      project for live-electronics.
    </p>
  </div>
  
  <div class="card">
    <h3 class="card-title">Didatical patches</h3>
    <img
      src="./tests/assets/didaticos.png"
      onclick="window.open('https://charlesneimog.github.io/Tese/Pages/Local-Maxima/', '_blank')"
      class="card-img"
    />
    <p style="width: 80%; text-align: center">
      Used to exemplify the concept of Local Maxima used on Partial Tracking.
    </p>
  </div>
</div>

---

<div class="container">
  <div class="card">
    <h3 class="card-title"><code>p5js</code> with <code>pd4web</code></h3>
    <img
      src="./tests/assets/patch3.png"
      onclick="window.open('https://charlesneimog.github.io/Improviso-I/', '_blank')"
      class="card-img"
    />
    <p style="width: 80%; text-align: center"><code>p5js</code> and <code>pd4web</code> used together.</p>
  </div>

  <div class="card">
    <h3 class="card-title">Physical Modelling with <code>pmpd</code></h3>
    <img
      src="./tests/assets/patch1.png"
      onclick="window.open('https://charlesneimog.github.io/Algorithm-Music/Piece-I/', '_blank')"
      class="card-img"
    />
    <p style="width: 80%; text-align: center">
        Using Physical Modelling for Synthesis.
    </p>
  </div>
</div>

---
