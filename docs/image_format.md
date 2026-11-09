# Image format
The image format is based on a color palette with *n* bits per pixel to represent 2^n different colors. Colors are 12 bits, 4 for each channel.

It has the following structure:
| Name | Size (bits) | Description |
| - | - | - |
| Width | 16 | *w*, number of pixels the picture is wide (little endian) |
| Height | 16 | *h*, number of pixels the picture is wide (little endian) |
| Bits per color | 8 | *n*, the number of bits to represent all colors |
| Colors | 12 * 2^n | List of colors in palette |
| Pixels | w * g * n + p | Palette color of each pixel. *p* is added padding of 0s for byte alignment |

The pixel number goes from first at (0,0), then continues horizontally until (w - 1, 0), after which it wraps to the next horizontal line at (0,1) and so on.
