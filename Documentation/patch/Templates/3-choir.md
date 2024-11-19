---
hide:
 - toc
---

# <h1 align="center">Template <code>3</code>: Choir and Phones</h1>


This template is just a reuse of the template used in the pieces `Moteto (2023)` and `Pandemônico (2024)`. Pieces for choir and smartphone. Where you can do some patch with guide notes (or just the score) for the singer.

The result will be something like this:

---

<p align="center">
  <img src="../assets/choir.jpeg" width="35%" style="border-radius: 10px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); cursor: pointer;" onclick="window.open('https://charlesneimog.github.io/Pandemonio/', '_blank');">
</p>

<p align="center" markdown>
    :octicons-download-16: [Download Patch example](../patches/template-3.pd)
</p>

## <h2 align="center">What should your patch include?</h2>

In your patch, you have these options to set using **senders**, for example, `[s composer`], `[s title]`, `[s duration-bar]`, `[s piece-duration-bar]`, `[s poem-phrase]` and `[s pitch-syl]`:

- `composer`: The composer name;
- `title`: Title of the piece;
- `duration-bar`: A number between 0 and 100 for the bar line (control the bar above the score);
- `piece-duration-bar`: A number between 0 and 100 for the bar line (control the bar below the phrase);
- `poem-phrase`: The complete phrase of the poem (or anything else), for example `minha frase`.
- `pitch-syl`: A list with the pitchname and the syllable, for example `C4 fra`.

In your patch, you have these options to set using **receivers**, for example, `[r naipe]`:

- `naipe`. Receive the naipe choosen by the singer, 1 for Baixo, 2 for Tenor, 3 for Contralto and 4 for Soprano.

--- 
Check the website of [Pandemonio](https://charlesneimog.github.io/Pandemonio/WebPatch/index.html) to see how it works.
