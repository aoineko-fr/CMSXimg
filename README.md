```` 
_____________________________________________________________________________									 
   ▄ ▄  ▄▄▄ ▄▄   ▄           
  ██▀█ ██   ██   ▄  ▄█▄█ ▄▀██
  ██ █ ▀█▄█ ██▄▄ ██ ██ █  ▀██
__________________________▀▀_________________________________________________

 by Guillaume "Aoineko" Blanchard (aoineko@free.fr)
 available on GitHub (https://github.com/aoineko-fr/MGLimg)
 under CC-BY-AS free license (https://creativecommons.org/licenses/by-sa/2.0/)

Command line tool to create images table to add to MSX programs (C/ASM/Bin)

Usage: MGLimg <filename> [options]

Options:
   inputFile       Inuput file name. Can be 8/16/24/32 bits image
                   Supported format: BMP, JPEG, PCX, PNG, TGA, PSD, GIF, etc.
   -out outFile    Output file name
   -format ?       Output format
      auto         Auto-detected using output file extension (default)
      c            C header file output
      asm          Assembler header file output
      bin          Raw binary data image
   -name name      Name of the table to generate
   -pos x y        Start position in the input image
   -size x y       Width/height of a block to export (if 0, use image size)
   -gap x y        Gap between blocks in pixels
   -num x y        Number of block to export (columns/rows number)
   -trans color    Transparency color (in RGB 24 bits format : 0xFFFFFF)
   -bpc ?	       Number of bits per color for the output image (support 1, 4 and 8-bits)
      1	           1-bit black & white (0: tranparency or black, 1: other colors)
      2	           2-bit index in 4 colors palette
      4	           4-bits index in 16 colors palette
      8	           8 bits RGB 256 colors (format: [G:3|R:3|B2]; default)
   -pal            Palette to use for 16 colors mode
      msx1         Use default MSX1 palette
      custom       Generate a custom palette and add it to the output file
   -palcount n     Number of color in the custom palette to create (default: 15)
   -compress ?
      none         No compression (default)
      crop16       Crop image to non transparent area (4-bits, max size 16x16)
      cropline16   Crop image to non transparent area (4-bits per line, max size 16x16)
      crop32       Crop image to non transparent area (5-bits, max size 32x32)
      cropline32   Crop image to non transparent area (5-bits per line, max size 32x32)
      crop256      Crop image to non transparent area (8-bits, max size 256x256)
      cropline256  Crop image to non transparent area (8-bits per line, max size 256x256)
      rle0         Run-length encoding of transparent blocs (7-bits for block length)
      rle4         Run-length encoding for all colors (4-bits for block length)
      rle8         Run-length encoding for all colors (8-bits for block length)
      auto         Determine a good compression method according to parameters
      best         Search for best compressor according to input parameters (smallest data)
   -dither ?       Dithering method (for 1-bit color only)
      none         No dithering (default)
      floyd        Floyd & Steinberg error diffusion algorithm
      bayer4       Bayer ordered dispersed dot dithering (order 2 – 4x4 - dithering matrix)
      bayer8       Bayer ordered dispersed dot dithering (order 3 – 8x8 - dithering matrix)
      bayer16      Bayer ordered dispersed dot dithering (order 4 – 16x16 dithering matrix)
      cluster6     Ordered clustered dot dithering (order 3 - 6x6 matrix)
      cluster8     Ordered clustered dot dithering (order 4 - 8x8 matrix)
      cluster16    Ordered clustered dot dithering (order 8 - 16x16 matrix)
   -data ?         Text format for numbers
      dec          Decimal data (c & asm)
      hexa         Default hexadecimal data (depend on langage; default)
      hexa0x       Hexadecimal data (0xFF; c & asm)
      hexaH        Hexadecimal data (0FFh; asm only)
      hexa$        Hexadecimal data ($FF; asm only)
      hexa#        Hexadecimal data (#FF; asm only)
      bin          Binary data (11001100b; asm only)
   -skip           Skip empty sprites (default: false)
   -idx            Add images index table (default: false)
   -copy (file)    Add copyright information from text file
                   If file name is empty, search for <inputFile>.txt
   -head           Add a header table contening input parameters (default: false)
   -font x y f l   Add font header (default: false)
                   x/y: Font width/heigt in pixels
                   f/l: ASCII code of the first/last character to export
                        Can be character (like: &) or hexadecimal value (0xFF format)
   -def            Add defines for each table (default: false)
   -notitle        Remove the ASCII-art title in top of exported text file
   -help           Display this help
	
Example:

> MGLimg.exe cars.png -out test_4_rle0.h -pos 0 0 -size 13 11 -num 16 4 -name g_Cars -trans 0xE300E3 -bpc 4 -pal custom -compress rle0

Load "cars.png" file and export 16x4 blocks of 13x11 pixels from upper-left corner of the image (0x0) in 4-bits index (16 colors) using a custom palette of 15 colors. Table name is "g_Cars" and RLE-transparency method is use for loose-less compression.   

````