# Setup in the app

The player's set (the mods built from their own dump and downloads) is made by the app itself, in
C++, on every platform. Packages carry no Python: the app is the whole product, which Android
needs and every desktop package gets for free.

## What moves

What `z3dmods.packed` runs today (`folders`, `setup`), about 6,600 lines of Python:

- Containers and formats: CCI/NCCH/RomFS and `.code` LZ (`ctr`), LzS (`lzs`), GAR, CTXB, ZSI rooms
  and scene collision (`zsi`), CMB models (`cmb`), BPS and IPS patches, QBF and GZFX fonts, QM and
  MM3D messages, CityHash64, PICA textures with ETC1/ETC1A4.
- The set's steps: source checks of the player's downloads (archives unpacked), the patch build
  and the code patches, the stairs (`stairs`, `stairs_stone`), the HUD art and edits (`hud`,
  `hud_icons`), the PC text and fonts (`text`, `text_font`, `outlined_font`), the texture-pack
  lookups, the slot links and the Azahar settings the set carries.
- Image work Pillow does: PNG read and write, resampling (Lanczos, box, bilinear, nearest),
  compositing, a max filter and outline-font drawing. In C++: SDL3_image for PNG, FreeType (already
  linked) for outlines, and our own resampling owner.

What stays Python: the maintainer tools (builds, packages, the harness, the input test, the scans).

## Rules for the move

- One implementation. Each module moves whole and its Python copy is deleted in the same change; the
  maintainer tools that need a moved step call the app's `--setup` mode, never a second copy.
- Proof per module: the C++ output matches the Python output byte for byte on both dumps, for every
  non-image file. Resampled images are checked by eye on the screen that shows them, since filters
  differ in rounding.
- The picker runs setup in-process on a worker thread and shows the same stage lines it shows now.

## Order

1. Formats with byte-exact differentials (ctr, lzs, gar, ctxb, bps, ips, cityhash, etc1, pica).
2. Models and rooms (cmb, zsi), then the stairs.
3. Fonts and messages (qbf, gzf, qm, text, text_font, outlined_font).
4. HUD art and edits, texture packs, source checks with archive unpacking.
5. The install step order and settings; the picker calls it in-process; `packed`, the bundled
   CPython and its licences leave every package.

Android starts once step 5 lands, with no Python to carry.
