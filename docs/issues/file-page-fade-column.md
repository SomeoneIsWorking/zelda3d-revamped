# File select's file page fades with a dark column in the middle

User, 2026-11-27 (Fedora, 3840x2160, OoT3D): bug report "fade not widened"
(`<user>/bug-reports/20261127-124057-fade-not-widened`).

On a file's page (File 2, Start / Copy / Erase) while it fades, a column about the touch screen's
width in the middle is far darker than the rest of the widened page.

## Investigation
`inputtest file-fade` shows it in `02-fading.png` (the test passes: it checks no pixels). The column
spans about x 108 to 239 of 400 and lies under the page's UI (Start draws over it), with textures on
or off. The two queued fades (a 320x240 touch quad and a 400x240 top quad, fill strength at +0xFC
0.91 to 0.98) are not it: zeroing either quad's width before the frame leaves the column.
