//     _____    _____________  ___ .___                               
//    /     \  /   _____/\   \/  / |   | _____ _____     ____   ____  
//   /  \ /  \ \_____  \  \     /  |   |/     \\__  \   / ___\_/ __ \ 
//  /    Y    \/        \ /     \  |   |  Y Y  \/ __ \_/ /_/  >  ___/ 
//  \____|__  /_______  //___/\  \ |___|__|_|  (____  /\___  / \___  >
//          \/        \/       \_/           \/     \//_____/      \/ 
//
// by Guillaume "Aoineko" Blanchard (aoineko@free.fr)
// under CC-BY-AS license (https://creativecommons.org/licenses/by-sa/2.0/)

// std
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <string>
#include <vector>
// FreeImage
#include "FreeImage.h"
// MSXImage
#include "types.h"
#include "color.h"
#include "exporter.h"
#include "image.h"
#include "parser.h"

// Output format
enum Format
{
	FORMAT_Auto, // Auto-detection
	FORMAT_C,    // C header file (text)
	FORMAT_Asm,  // Assembler header file (text)
	FORMAT_Bin,  // Binary data file
};

// Debug
//const char* ARGV[] = { "", "-in", "test/cars.png", "-out", "test/sprt_car_1.h", "-pos", "0", "0", "-size", "13", "11", "-num", "16", "1", "-name", "g_Car1", "-trans", "0xE300E3", "-compress", "cropline16", "-format", "asm", "-data", "dec" };
//const char* ARGV[] = { "", "-in", "test/cars.png", "-out", "test/sprt_car_1.h", "-pos", "0", "0", "-size", "13", "11", "-num", "16", "1", "-name", "g_Car1", "-trans", "0xE300E3", "-compress", "cropline32" };
//const char* ARGV[] = { "", "-in", "test/track_tiles.png", "-out", "test/sprt_track.h", "-pos", "0", "0", "-size", "32", "32", "-num", "8", "4", "-name", "g_TrackTiles", "-trans", "0xDA48AA", "-bpc", "1", "-compress", "crop256", "-dither", "cluster8" };
//const char* ARGV[] = { "", "-in", "test/test_sprt.png", "-out", "test/sprt_player.h", "-pos", "0", "0", "-size", "16", "16", "-num", "11", "8", "-name", "g_PlayerSprite", "-trans", "0x336600", "-bpc", "4", "-pal", "custom", "-compress", "best" };
//#define DEBUG_ARGS

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

void PrintHelp()
{
	printf("MSXImage (v%s)\n", VERSION);
	printf("Usage: MSXImage [options]\n");
	printf("Options:\n");
	printf("   -in fileName    Inuput file name. Can be 8/16/24/32 bits image\n");
	printf("                   Supported format: BMP, JPEG, PCX, PNG, TGA, PSD, GIF, etc.\n");
	printf("   -out fileName   Output file name\n");
	printf("   -format ?       Output format\n");
	printf("      auto         Auto-detected using output file extension (default)\n");
	printf("      c            C header file output\n");
	printf("      asm          Assembler header file output\n");
	printf("      bin          Raw binary data image\n");
	printf("   -name name      Name of the table to generate\n");
	printf("   -pos x y        Start position in the input image\n");
	printf("   -size x y       Width/height of a block to export (if 0, use image size)\n");
	printf("   -num x y        Number of block to export (columns/rows number)\n");
	printf("   -trans color    Transparency color (in RGB 24 bits format : 0xFFFFFF)\n");
	printf("   -bpc ?	       Number of bits per color for the output image (support 1, 4 and 8-bits)\n");
	printf("      1	           1-bit black & white (0: tranparency or black, 1: other colors)\n");
	printf("      4	           4-bits index in 16 colors palette\n");
	printf("      8	           8 bits RGB 256 colors (format: [G:3|R:3|B2]; default)\n");
	printf("   -pal            Palette to use for 16 colors mode\n");
	printf("      msx1         Use default MSX1 palette\n");
	printf("      custom       Generate a custom palette and add it to the output file\n");
	printf("   -palcount n     Number of color in the custom palette to create (default: 15)\n");
	printf("   -compress ?\n");
	printf("      none         No compression (default)\n");
	printf("      crop16       Crop image to non transparent area (4-bits, max size 16x16)\n");
	printf("      cropline16   Crop image to non transparent area (4-bits per line, max size 16x16)\n");
	printf("      crop32       Crop image to non transparent area (5-bits, max size 32x32)\n");
	printf("      cropline32   Crop image to non transparent area (5-bits per line, max size 32x32)\n");
	printf("      crop256      Crop image to non transparent area (8-bits, max size 256x256)\n");
	printf("      cropline256  Crop image to non transparent area (8-bits per line, max size 256x256)\n");
	printf("      rle0         Run-length encoding of transparent blocs (7-bits for block length)\n");
	printf("      rle4         Run-length encoding for all colors (4-bits for block length)\n");
	printf("      rle8         Run-length encoding for all colors (8-bits for block length)\n");
	printf("      auto         Determine a good compression method according to parameters\n");
	printf("      best         Search for best compressor according to input parameters (smallest data)\n");
	printf("   -dither ?       Dithering method (for 1-bit color only)\n");
	printf("      none         No dithering (default)\n");
	printf("      floyd        Floyd & Steinberg error diffusion algorithm\n");
	printf("      bayer4       Bayer ordered dispersed dot dithering(order 2 – 4x4 - dithering matrix)\n");
	printf("      bayer8       Bayer ordered dispersed dot dithering(order 3 – 8x8 - dithering matrix)\n");
	printf("      bayer16      Bayer ordered dispersed dot dithering(order 4 – 16x16 dithering matrix)\n");
	printf("      cluster6     Ordered clustered dot dithering(order 3 - 6x6 matrix)\n");
	printf("      cluster8     Ordered clustered dot dithering(order 4 - 8x8 matrix)\n");
	printf("      cluster16    Ordered clustered dot dithering(order 8 - 16x16 matrix)\n");
	printf("   -data ?         Text format for numbers\n");
	printf("      dec          Decimal data (c & asm)\n");
	printf("      hexa         Default hexadecimal data (depend on langage; default)\n");
	printf("      hexa0x       Hexadecimal data (0xFF; c & asm)\n");
	printf("      hexaH        Hexadecimal data (0FFh; asm only)\n");
	printf("      hexa$        Hexadecimal data ($FF; asm only)\n");
	printf("      hexa#        Hexadecimal data (#FF; asm only)\n");
	printf("      bin          Binary data (11001100b; asm only)\n");
	printf("   -skip           Skip empty sprites (default: false)\n");
	printf("   -head           Add a header table contening input parameters (default: false)\n");
	printf("   -help           Display this help\n");
}

/** Main entry point
	Usage: MSXImage -in inFile -pos x y -size x y -num x y -out outFile -palette [16|256]
*/
int main(int argc, const char* argv[])
{
	// for debug purpose
#ifdef DEBUG_ARGS
	argc = sizeof(ARGV)/sizeof(ARGV[0]); argv = ARGV;
#endif

	FreeImage_Initialise();

	Format outFormat = FORMAT_Auto;
	ExportParameters param;
	i32 i;
	bool bAutoCompress = false;
	bool bBestCompress = false;

	if(argc < 2)
	{
		PrintHelp();
		return 1;
	}

	//-------------------------------------------------------------------------
	// Parse parameters
	for(i=1; i<argc; i++)
	{
		if (_stricmp(argv[i], "-help") == 0)
		{
			PrintHelp();
			return 0;
		}
		else if(_stricmp(argv[i], "-in") == 0)
		{
			param.inFile = argv[++i];
		}
		else if (_stricmp(argv[i], "-out") == 0)
		{
			param.outFile = argv[++i];
		}
		else if (_stricmp(argv[i], "-format") == 0)
		{
			i++;
			if (_stricmp(argv[i], "auto") == 0)
				outFormat = FORMAT_Auto;
			else if (_stricmp(argv[i], "c") == 0)
				outFormat = FORMAT_C;
			else if (_stricmp(argv[i], "asm") == 0)
				outFormat = FORMAT_Asm;
			else if (_stricmp(argv[i], "bin") == 0)
				outFormat = FORMAT_Bin;
		}
		else if(_stricmp(argv[i], "-pos") == 0)
		{
			param.posX = atoi(argv[++i]);
			param.posY = atoi(argv[++i]);
		}
		else if(_stricmp(argv[i], "-size") == 0)
		{
			param.sizeX = atoi(argv[++i]);
			param.sizeY = atoi(argv[++i]);
		}
		else if(_stricmp(argv[i], "-num") == 0)
		{
			param.numX = atoi(argv[++i]);
			param.numY = atoi(argv[++i]);
		}
		else if (_stricmp(argv[i], "-name") == 0)
		{
			param.tabName = argv[++i];
		}
		else if(_stricmp(argv[i], "-bpc") == 0)
		{
			param.bpc = atoi(argv[++i]);
		}
		else if(_stricmp(argv[i], "-trans") == 0)
		{
			sscanf_s(argv[++i], "%i", &param.transColor);
			param.bUseTrans = true;
		}
		else if (_stricmp(argv[i], "-pal") == 0)
		{
			i++;
			if (_stricmp(argv[i], "msx1") == 0)
				param.palType = PALETTE_MSX1;
			else if (_stricmp(argv[i], "custom") == 0)
				param.palType = PALETTE_Custom;
		}
		else if (_stricmp(argv[i], "-palcount") == 0)
		{
			param.palCount = atoi(argv[++i]);
		}		
		else if(_stricmp(argv[i], "-compress") == 0)
		{
			i++;
			if (_stricmp(argv[i], "crop16") == 0)
				param.comp = COMPRESS_Crop16;
			else if (_stricmp(argv[i], "cropline16") == 0)
				param.comp = COMPRESS_CropLine16;
			else if (_stricmp(argv[i], "crop32") == 0)
				param.comp = COMPRESS_Crop32;
			else if (_stricmp(argv[i], "cropline32") == 0)
				param.comp = COMPRESS_CropLine32;
			else if (_stricmp(argv[i], "crop256") == 0)
				param.comp = COMPRESS_Crop256;
			else if (_stricmp(argv[i], "cropline256") == 0)
				param.comp = COMPRESS_CropLine256;
			else if (_stricmp(argv[i], "rle0") == 0)
				param.comp = COMPRESS_RLE0;
			else if (_stricmp(argv[i], "rle4") == 0)
				param.comp = COMPRESS_RLE4;
			else if (_stricmp(argv[i], "rle8") == 0)
				param.comp = COMPRESS_RLE8;
			else if (_stricmp(argv[i], "auto") == 0)
				bAutoCompress = true;
			else if (_stricmp(argv[i], "best") == 0)
				bBestCompress = true;
			else
				param.comp = COMPRESS_None;
		}
		else if (_stricmp(argv[i], "-dither") == 0)
		{
			i++;
			if (_stricmp(argv[i], "none") == 0)
				param.dither = DITHER_None;
			else if (_stricmp(argv[i], "floyd") == 0)
				param.dither = DITHER_Floyd;
			else if (_stricmp(argv[i], "bayer4") == 0)
				param.dither = DITHER_Bayer4;
			else if (_stricmp(argv[i], "bayer8") == 0)
				param.dither = DITHER_Bayer8;
			else if (_stricmp(argv[i], "bayer16") == 0)
				param.dither = DITHER_Bayer16;
			else if (_stricmp(argv[i], "cluster6") == 0)
				param.dither = DITHER_Cluster6;
			else if (_stricmp(argv[i], "cluster8") == 0)
				param.dither = DITHER_Cluster8;
			else if (_stricmp(argv[i], "cluster16") == 0)
				param.dither = DITHER_Cluster16;
		}
		else if(_stricmp(argv[i], "-data") == 0)
		{
			i++;
			if(_stricmp(argv[i], "dec") == 0)
				param.dataType = DATA_Decimal;
			else if(_stricmp(argv[i], "hexa") == 0)
				param.dataType = DATA_Hexa;
			else if(_stricmp(argv[i], "hexa0x") == 0)
				param.dataType = DATA_HexaC;
			else if(_stricmp(argv[i], "hexaH") == 0)
				param.dataType = DATA_HexaAsm;
			else if(_stricmp(argv[i], "hexa$") == 0)
				param.dataType = DATA_HexaDollar;
			else if(_stricmp(argv[i], "hexa#") == 0)
				param.dataType = DATA_HexaSharp;
			else if(_stricmp(argv[i], "bin") == 0)
				param.dataType = DATA_Binary;
		}
		else if (_stricmp(argv[i], "-skip") == 0)
		{
			param.bSkipEmpty = true;
		}
		else if (_stricmp(argv[i], "-head") == 0)
		{
			param.bAddHeader = true;
		}
	}

	//-------------------------------------------------------------------------
	// Determine a valid compression method according to input parameters
	if (bAutoCompress)
	{
		param.comp = COMPRESS_None;
		if ((param.sizeX != 0) && (param.sizeY != 0))
		{
			if (param.bUseTrans)
			{
				if (param.bpc == 1)
				{
					if ((param.sizeX <= 16) && (param.sizeY <= 16))
						param.comp = COMPRESS_Crop16;
					else if ((param.sizeX <= 32) && (param.sizeY <= 32))
						param.comp = COMPRESS_Crop32;
					else if ((param.sizeX <= 256) && (param.sizeY <= 256))
						param.comp = COMPRESS_Crop256;
				}
				else // bpc == 4 or 8
				{
					if ((param.sizeX <= 16) && (param.sizeY <= 16))
						param.comp = COMPRESS_CropLine16;
					else if ((param.sizeX <= 32) && (param.sizeY <= 32))
						param.comp = COMPRESS_CropLine32;
					else if ((param.sizeX <= 256) && (param.sizeY <= 256))
						param.comp = COMPRESS_CropLine256;
				}
			}
			else
			{
				if (param.bpc == 4)
					param.comp = COMPRESS_RLE4;
			}
		}
		printf("Auto compress: %s method selected\n", GetCompressorName(param.comp));
	}
	
	//-------------------------------------------------------------------------
	// Search for best compressor according to input parameters
	if (bBestCompress)
	{
		printf("Start benchmark to find the best compressor\n");
		static const Compressor compTable[] =
		{
			COMPRESS_None,
			COMPRESS_Crop16,
			COMPRESS_CropLine16,
			COMPRESS_Crop32,
			COMPRESS_CropLine32,
			COMPRESS_Crop256,
			COMPRESS_CropLine256,
			COMPRESS_RLE0,
			COMPRESS_RLE4,
			COMPRESS_RLE8
		};

		u32 bestSize = 0;
		Compressor bestComp = COMPRESS_None;

		for (i32 i = 0; i < numberof(compTable); i++)
		{
			param.comp = compTable[i];
			printf("- Check %s... ", GetCompressorName(param.comp, true));
			if (IsCompressorCompatible(param.comp, param))
			{
				ExporterInterface* exp = new ExporterDummy(param.dataType, &param);
				bool bSucceed = ParseImage(&param, exp);
				if (bSucceed)
				{
					printf("Generated data: %i bytes\n", exp->GetTotalBytes());
					if ((bestSize == 0) || (exp->GetTotalBytes() < bestSize))
					{
						bestSize = exp->GetTotalBytes();
						bestComp = param.comp;
					}
				}
				else
				{
					printf("Parse error!\n");
				}
				delete exp;
			}
			else
			{
				printf("Incompatible!\n");
			}
		}

		printf("- Best compressor selected: %s\n", GetCompressorName(bestComp));
		param.comp = bestComp;
	}

	//-------------------------------------------------------------------------
	// Validate parameters
	if (param.inFile == NULL)
	{
		printf("Error: Input file required!\n");
		return 1;
	}
	if (param.outFile == NULL)
	{
		printf("Error: Output file required!\n");
		return 1;
	}
	if ((param.bpc != 1) && (param.bpc != 4) && (param.bpc != 8))
	{
		printf("Error: Invalid bits-per-color value (%i). Only 1, 4 or 8-bits colors are supported!\n", param.bpc);
		return 1;
	}
	if ((param.sizeX == 0) || (param.sizeY == 0))
	{
		printf("Warning: sizeX or sizeY is 0. The whole image will be exported.\n");
	}
	if (!param.bUseTrans && (param.comp & COMPRESS_Crop_Mask))
	{
		printf("Warning: Crop compressor can't be use without transparency color. Crop compressor removed.\n");
		param.comp = COMPRESS_None;
	}
	if (!param.bUseTrans && (param.comp == COMPRESS_RLE0))
	{
		printf("Warning: RLE0 compressor can't be use without transparency color. RLE0 compressor removed.\n");
		param.comp = COMPRESS_None;
	}
	if ((param.bpc == 1) && (param.comp & COMPRESS_RLE_Mask))
	{
		printf("Warning: RLE compressor can be use only with 4 and 8-bits color format. RLE compressor removed.\n");
		param.comp = COMPRESS_None;
	}
	if ((param.bpc == 8) && (param.comp == COMPRESS_RLE4))
	{
		printf("Warning: RLE4 compressor have no advantage with 8-bits color format. RLE8 compressor will be use instead.\n");
		param.comp = COMPRESS_RLE8;
	}
	if (!param.bUseTrans && param.bSkipEmpty)
	{
		printf("Warning: -skip as no effect without transparency color.\n");
	}
	if (param.palCount > 15)
	{
		printf("Warning: -palcount is %i but can't be more than 15 (color index 0 is always transparent). Continue with 15 as value.\n", param.palCount);
		param.palCount = 15;
	}
	if ((param.dither != DITHER_None) && (param.bpc != 1))
	{
		printf("Warning: Dithering only work with 1-bit color format (current is %i-bits). Dithering value will be ignored.\n", param.bpc);
	}

	bool bSucceed = false;

	//-------------------------------------------------------------------------
	// Convert
	if(param.inFile && param.outFile)
	{
		if((outFormat == FORMAT_C) || ((outFormat == FORMAT_Auto) && (strstr(param.outFile, ".h") || strstr(param.outFile, ".inc"))))
		{
			ExporterInterface* exp = new ExporterC(param.dataType, &param);
			bSucceed = ParseImage(&param, exp);
			delete exp;
		}
		else if((outFormat == FORMAT_Asm) || ((outFormat == FORMAT_Auto) && (strstr(param.outFile, ".s") || strstr(param.outFile, ".asm"))))
		{
			ExporterInterface* exp = new ExporterASM(param.dataType, &param);
			bSucceed = ParseImage(&param, exp);
			delete exp;
		}
		else if((outFormat == FORMAT_Bin) || ((outFormat == FORMAT_Auto) && (strstr(param.outFile, ".bin") || strstr(param.outFile, ".raw"))))
		{
			ExporterInterface* exp = new ExporterBin(param.dataType, &param);
			bSucceed = ParseImage(&param, exp);
			delete exp;
		}
		else
		{
			FIBITMAP *dib = LoadImage(param.inFile); // open and load the file using the default load option
			if (dib == NULL)
			{
				printf("Error: Fail to load %s\n", param.inFile);
			}
			else
			{
				bSucceed = SaveImage(dib, param.outFile); // save the file
				FreeImage_Unload(dib); // free the dib
			}
		}
	}

	FreeImage_DeInitialise();
	if(bSucceed)
		printf("Succeed!\n");
	else
		printf("Error: Fatal error!\n");
	return bSucceed ? 0 : 1;
}

