# Image format
The image format is based on a color palette with *n* bits per pixel to represent 2^n - 1 different colors. The color index represented by all 0s is fully transparent. Colors are 12 bits, 4 for each channel.

It has the following structure:
| Name | Size (bits) | Description |
| - | - | - |
| Width | 16 | *w*, number of pixels the picture is wide |
| Height | 16 | *h*, number of pixels the picture is wide |
| Bits per color | 8 | *n*, the number of bits to represent all colors |
| Colors | 12 * (2^n - 1) + 4 | List of colors in palette. Last 4 bits are all 0s to align to byte size |
| Pixels | w * g * n | Palette color of pixel |

The pixel number goes from first at (0,0), then continues horizontally until (w - 1, 0), after which it wraps to the next horizontal line at (0,1) and so on.
