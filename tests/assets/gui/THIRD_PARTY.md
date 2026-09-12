# Third-party material in this directory

Everything under `tests/assets/gui/` is used only by `tests/debug_gui.cpp`, the
optional debug board behind `CHESSO_BUILD_GUI` (OFF by default). None of it is
linked into `chesso` and none of it is part of the engine. It is redistributed
with this repository all the same, so it is inventoried here with its licence.

Written by S211, from `2026-09-10_adversarial-F03`: twelve of the files below
were being redistributed with no licence and no attribution, and three more had
no stated origin at all. The repository is MIT and stays MIT (DEC-104), so every
entry here is either under a licence MIT can carry or is gone.

## The chess pieces -- Cburnett, Wikimedia Commons, used under BSD

`Chess_bdt60.png`, `Chess_blt60.png`, `Chess_kdt60.png`, `Chess_klt60.png`,
`Chess_ndt60.png`, `Chess_nlt60.png`, `Chess_pdt60.png`, `Chess_plt60.png`,
`Chess_qdt60.png`, `Chess_qlt60.png`, `Chess_rdt60.png`, `Chess_rlt60.png`.

Twelve 60x60 PNG renders of the standard-transparent SVG chess set by the
Wikimedia Commons user **Cburnett**, whose originals are `Chess_bdt45.svg` and
its eleven siblings (own work, 2006-12-27). The naming here is that set's own:
piece letter, `d` or `l` for the dark or light piece, `t` for a transparent
square, then the pixel size.

The author published each file under four licences at once -- GFDL 1.2+,
CC BY-SA 3.0, BSD and GPL -- and the file page ends "You may select the license
of your choice." **This project selects the BSD licence**, which is the one of
the four that an MIT-licensed repository can redistribute without imposing a
share-alike or a GPL term on anything beside it. Verified against the page's own
wikitext on 2026-09-12: `{{self|GFDL|migration=relicense|BSD|GPL}}`, author
`{{U|Cburnett}}`.

The BSD licence requires the notice be retained, so here it is, as Commons
renders it on the file page, with the author named where that page prints the
placeholder "The author":

> Copyright (c) Cburnett
>
> Redistribution and use in source and binary forms, with or without
> modification, are permitted provided that the following conditions are met:
>
> * Redistributions of source code must retain the above copyright notice, this
>   list of conditions and the following disclaimer.
> * Redistributions in binary form must reproduce the above copyright notice,
>   this list of conditions and the following disclaimer in the documentation
>   and/or other materials provided with the distribution.
> * Neither the name of Cburnett nor the names of its contributors may be used
>   to endorse or promote products derived from this software without specific
>   prior written permission.
>
> This software is provided by Cburnett and contributors "as is" and any express
> or implied warranties, including, but not limited to, the implied warranties
> of merchantability and fitness for a particular purpose are disclaimed. In no
> event shall Cburnett and contributors be liable for any direct, indirect,
> incidental, special, exemplary, or consequential damages (including, but not
> limited to, procurement of substitute goods or services; loss of use, data, or
> profits; or business interruption) however caused and on any theory of
> liability, whether in contract, strict liability, or tort (including
> negligence or otherwise) arising in any way out of the use of this software,
> even if advised of the possibility of such damage.

Source: https://commons.wikimedia.org/wiki/File:Chess_bdt45.svg and the eleven
sibling pages, the licence text quoted from the BSD box those pages render
(https://opensource.org/licenses/bsd-license.php).

The PNGs in this directory carry `tEXtSoftware www.inkscape.org`, so they are
Inkscape renders of those SVGs rather than the SVGs themselves. Nothing about
that changes the licence: they are a copy in another resolution.

## The fonts -- already licensed, in files beside them

| file | font | licence | where the licence is |
|---|---|---|---|
| `PressStart2P.ttf`, `font/PressStart2P.ttf` | Press Start 2P, Cody "CodeMan38" Boisclair | SIL Open Font License 1.1 | `LICENSE.txt`, `FONTLOG.txt`, and the copies under `font/` |
| `font/ubuntu_mono/UbuntuMono-*.ttf`, `ubuntu_mono/UbuntuMono-*.ttf` | Ubuntu Mono | Ubuntu Font Licence 1.0 | `font/ubuntu_mono/UFL.txt`, `ubuntu_mono/UFL.txt` |

Both were already stated when S211 ran; they are listed so that this file is the
whole inventory and not only the part that was missing.

## The sounds -- origin not recorded, and that is the remaining gap

`sound/click.wav`, `sound/tick_1.wav` .. `sound/tick_5.wav`,
`sound/anime-wow-sound-effect.mp3`.

Committed on 2025-03-09 in `766465f` ("Start the test gui") with no source.
Their files carry no author, no copyright and no origin metadata -- the WAVs
carry only a RIFF header and the MP3 only its encoder's tags -- so nobody can
state where they came from, and this file will not guess. They are named here
rather than left silent. They sit outside `2026-09-10_adversarial-F03`, which
counted the images, so removing them is not S211's to do; the honest reading is
that this directory is fully licensed **except** these seven files.

## Deleted by S211, because their origin could not be stated

| file | what it was | why it went |
|---|---|---|
| `background_l.jpg` | a 4256x2832 photograph used as the window background | committed in `766465f` with no source; no author, copyright or origin in the file, only an sRGB profile. `debug_gui.cpp` now paints a flat colour instead |
| `flip_icon.png` | a 1000x846 icon on the flip-board button | same commit, same absence; the button now carries the text "Flip" like the two buttons beside it |
| `ChessPiecesArray.png` | a 360x120 sprite sheet, Adobe Photoshop CC 2015.5, created 2017-01-13 | same commit, same absence, and **nothing in the repository ever read it** -- it was dead weight as well as unsourced |

Deleting was the accepted outcome for these three (S211's `accepts`). They could
not be sourced: `git log --follow` reaches one commit each, and the metadata in
the files names a tool and a date, never a rights holder.
