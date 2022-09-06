# gimp-colormap-shift

A gimp plugin for shifting the colormap of indexed images

![gimp-colormap-shift](gimp-colormap-shift.gif)

Often when working with indexed images (images whose color comes from the
colormap rather than stored directly), it is useful to be able to shift or
offset the colormap in order to view and edit the same image while using a
different set of colors.  This is especially useful when dealing with graphics
chips that rely on "palette offsets" for certain display modes, such as the
VERA chip used by the Commander X16.

## Compilation and Installation

To install this plugin, you need to make sure your system has `gimptool-2.0`
installed.  This GIMP-provided command line tool is used by the plugin's
`Makefile` in order to specify includes and library dependencies, as well as
facilitate installation of the plugin.  If all the appropriate GIMP development
dependencies are installed, you should be able to compile and install with:

```
$ make
$ make install
```

## Usage

The plugin defines both interactive and non-interactive run modes, so you can
use it through a user interface from within GIMP, or from the command line with
scripts.  In either case, the plugin requires that you set your image to
_Indexed_ mode (using a palette (aka colormap) to reference your colors).

### Interactive Mode

Open to the colormap shift dialog by navigating to Colors -> Map -> Shift
Colors.  This will open a dialog showing the entire palette.  Select any color
by double clicking (or using the cursor keys and hitting Enter or Space) and
the colormap will be shifted such that the selected color is not the first
color in the colormap.

Unlike the colormap remap plugin, this plugin does _not_ change the indexes of
the pixels in the drawing to match the existing color's new index in the color
map.  This forces the pixel to become whatever color its existing index now
points to in the shifted map.  This is very useful for designing graphics for
retro computers, where the video displays work on the same principle.  You can
now edit and view images with different colormaps, just as you display them on
old video chips.

### Non-Interactive Mode

TBD
