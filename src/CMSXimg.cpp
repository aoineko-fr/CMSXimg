﻿//_____________________________________________________________________________
//   ▄▄   ▄ ▄  ▄▄▄ ▄▄ ▄ ▄                                                      
//  ██ ▀ ██▀█ ▀█▄  ▀█▄▀ ▄  ▄█▄█ ▄▀██                                           
//  ▀█▄▀ ██ █ ▄▄█▀ ██ █ ██ ██ █  ▀██                                           
//_______________________________▀▀____________________________________________
//
// by Guillaume "Aoineko" Blanchard (aoineko@free.fr)
// available on GitHub (https://github.com/aoineko-fr/CMSXimg)
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
// CMSXi
#include "CMSXi.h"
#include "types.h"
#include "color.h"
#include "exporter.h"
#include "image.h"
#include "parser.h"

/// Check if filename contains the given extension
bool HaveExt(const std::string& str, const std::string& ext)
{
	return str.find(ext) != std::string::npos;
}

/// Remove the filename extension (if any)
std::string RemoveExt(const std::string& str)
{
	size_t lastdot = str.find_last_of(".");
	if (lastdot == std::string::npos)
		return str;
	return str.substr(0, lastdot);
}

/// Check if a file exist
bool FileExists(const std::string& filename)
{
	FILE* file;
	if (fopen_s(&file, filename.c_str(), "r") == 0)
	{
		fclose(file);
		return true;
	}
	return false;
}

/// Check if 2 string are equal
//bool CMSX::StrEqual(const c8* str1, const c8* str2)
//{
//	return (_stricmp(str1, str2) == 0);
//}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

void PrintHelp()
{
	printf("CMSXimg (v%s)\n", CMSXi_VERSION);
	printf("Usage: CMSXimg <filename> [options]\n");
	printf("\n");
	printf("Options:\n");
	printf("   inputFile       Inuput file name. Can be 8/16/24/32 bits image\n");
	printf("                   Supported format: BMP, JPEG, PCX, PNG, TGA, PSD, GIF, etc.\n");
	printf("   -out outFile    Output file name\n");
	printf("   -format ?       Output format\n");
	printf("      auto         Auto-detected using output file extension (default)\n");
	printf("      c            C header file output\n");
	printf("      asm          Assembler header file output\n");
	printf("      bin          Raw binary data image\n");
	printf("   -name name      Name of the table to generate\n");
	printf("   -mode ?         Exporter mode\n");
	printf("      bmp          Export image as bitmap (default)\n");
	printf("      gm1          Generate all tables for Graphic mode 1 (Screen 1)\n");
	printf("      gm2          Generate all tables for Graphic mode 2 or 3 (Screen 2 or 4)\n");
	printf("      sprt         Export 16x16 sprites with specific block ordering\n");
	printf("   -pos x y        Start position in the input image\n");
	printf("   -size x y       Width/height of a block to export (if 0, use image size)\n");
	printf("   -gap x y        Gap between blocks in pixels\n");
	printf("   -num x y        Number of block to export (columns/rows number)\n");
	printf("   -l ? sx sy nx ny col1 (col2, col3, ...)\n");
	printf("                   Layer including the given color(s) (coordinate are relative to each expoted block)\n");
	printf("      i8           8x8 sprites layer with only provided colors\n");
	printf("      i16          16x16 sprites layer with only provided colors\n");
	printf("      e8           8x8 sprites layer with all colors but the provided ones\n");
	printf("      e16          16x16 sprites layer with all colors but the provided ones\n");
	printf("                   sx/sy is layer start position in pixel in a block\n");
	printf("                   nx/ny is layer size in sprite count (1 equal 8 or 16 according to sprite size)\n");
	printf("                   Colors are in RGB 24 bits format (0xFFFFFF)\n");
	printf("   -trans color    Transparency color (in RGB 24 bits format : 0xFFFFFF)\n");
	printf("   -opacity color  Opacity color (in RGB 24 bits format : 0xFFFFFF). All other colors are considered transparent\n");
	printf("   -bpc ?	       Number of bits per color for the output image (support 1, 4 and 8-bits)\n");
	printf("      1	           1-bit black & white (0: tranparency or black, 1: other colors)\n");
	printf("      2	           2-bit index in 4 colors palette\n");
	printf("      4	           4-bits index in 16 colors palette\n");
	printf("      8	           8 bits RGB 256 colors (format: [G:3|R:3|B2]; default)\n");
	printf("   -pal            Palette to use for 16 colors mode\n");
	printf("      msx1         Use default MSX1 palette\n");
	printf("      custom       Generate a custom palette and add it to the output file\n");
	printf("   --palcount n    Number of color in the custom palette to create (default: 15)\n");
	printf("   --paloff n      Index offset of the palette (default: 1)\n");
	printf("   --pal24	       Use 24-bits palette (for v9990; default: false)\n");
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
	printf("      rlep         Pattern based run-length encoding (6-bits for block length)\n");
	printf("      auto         Determine a good compression method according to parameters\n");
	printf("      best         Search for best compressor according to input parameters (smallest data)\n");
	printf("   -dither ?       Dithering method (for 1-bit color only)\n");
	printf("      none         No dithering (default)\n");
	printf("      floyd        Floyd & Steinberg error diffusion algorithm\n");
	printf("      bayer4       Bayer ordered dispersed dot dithering (order 2 – 4x4 - dithering matrix)\n");
	printf("      bayer8       Bayer ordered dispersed dot dithering (order 3 – 8x8 - dithering matrix)\n");
	printf("      bayer16      Bayer ordered dispersed dot dithering (order 4 – 16x16 dithering matrix)\n");
	printf("      cluster6     Ordered clustered dot dithering (order 3 - 6x6 matrix)\n");
	printf("      cluster8     Ordered clustered dot dithering (order 4 - 8x8 matrix)\n");
	printf("      cluster16    Ordered clustered dot dithering (order 8 - 16x16 matrix)\n");
	printf("   -data ?         Text format for numbers\n");
	printf("      dec          Decimal data (c & asm)\n");
	printf("      hexa         Default hexadecimal data (depend on langage; default)\n");
	printf("      hexa0x       Hexadecimal data (0xFF; c & asm)\n");
	printf("      hexaH        Hexadecimal data (0FFh; asm only)\n");
	printf("      hexa$        Hexadecimal data ($FF; asm only)\n");
	printf("      hexa#        Hexadecimal data (#FF; asm only)\n");
	printf("      bin          Binary data (11001100b; asm only)\n");
	printf("   -skip           Skip empty sprites (default: false)\n");
	printf("   -idx            Add images index table (default: false)\n");
	printf("   -copy (file)    Add copyright information from text file\n");
	printf("                   If file name is empty, search for <inputFile>.txt\n");
	printf("   -head           Add a header table contening input parameters (default: false)\n");
	printf("   -font x y f l   Add font header (default: false)\n");
	printf("                   x/y: Font width/heigt in pixels\n");
	printf("                   f/l: ASCII code of the first/last character to export\n");
	printf("                        Can be character (like: &) or hexadecimal value (0xFF format)\n");
	printf("   -offset x       Offset of layout index for GM1 et GM2 mode (default: 0)\n");
	printf("   -at x           Data starting address (can be decimal or hexadecimal starting with '0x')\n");
	printf("   -def            Add defines for each table\n");
	printf("   -notitle        Remove the ASCII-art title in top of exported text file\n");
	printf("   --gm2compnames  GM2 mode: Compress names/layout table (default: false)\n");
	printf("   --gm2unique     GM2 mode: Export all unique tiles (default: false)\n");
	printf("   --bload         Add header for BLOAD image (default: false)\n");
	printf("   -help           Display this help\n");
}

// Debug
//const char* ARGV[] = { "", "test/cars.png", "-out", "test/sprt_car_1.h", "-pos", "0", "0", "-size", "13", "11", "-num", "16", "1", "-name", "g_Car1", "-trans", "0xE300E3", "-compress", "cropline16", "-copy", "test/cmsx.txt" };
//const char* ARGV[] = { "", "test/cars.png", "-out", "test/sprt_car_1.h", "-pos", "0", "0", "-size", "13", "11", "-num", "16", "1", "-name", "g_Car1", "-trans", "0xE300E3", "-compress", "cropline32" };
//const char* ARGV[] = { "", "test/track_tiles.png", "-out", "test/sprt_track.h", "-pos", "0", "0", "-size", "32", "32", "-num", "8", "4", "-name", "g_TrackTiles", "-trans", "0xDA48AA", "-bpc", "1", "-compress", "crop256", "-dither", "cluster8" };
//const char* ARGV[] = { "", "test/test_sprt.png", "-out", "test/sprt_player.h", "-pos", "0", "0", "-size", "16", "16", "-num", "11", "8", "-name", "g_PlayerSprite", "-trans", "0x336600", "-bpc", "4", "-pal", "custom", "-compress", "best" };
//const char* ARGV[] = { "", "test/cmsx_9.png", "-out", "test/cmsx_9.h", "-pos", "0", "0", "-size", "8", "12", "-gap", "0", "4", "-num", "16", "6", "-name", "cmsx_9", "-trans", "0x000000", "-bpc", "1", "-skip", "-font", "8", "8", "!", "_" };
//const char* ARGV[] = { "", "test/Court.png", "-out", "test/court.h", "-name", "g_Court", "-mode", "g2" };
//const char* ARGV[] = { "", "test/players16.png", "-out", "test/players16.h", "-mode", "s16", "-pos", "0", "0", "-size", "16", "24", "-num", "13", "1", };
//const char* ARGV[] = { "", "test/players.png", "-out", "test/players.h", "-mode", "sprt", "-pos", "0", "0", "-size", "16", "32", "-num", "9", "3", "-name", "g_Player1", "-compress", "rlep", "-at", "0x0010",
//	"-l", "i16", "0", "0",  "1", "1", "0x010101",													// Black 1
//	"-l", "i16", "0", "0",  "1", "1", "0x010101", "0x5955E0", "0x3AA241", "0xCCCCCC", "0xDB6559",	// Black 2
//	"-l", "i16", "0", "8",  "1", "1", "0x8076F1", "0x3EB849", "0x5955E0", "0x3AA241",				// Cloth
//	"-l", "i16", "0", "0",  "1", "1", "0xFFFFFF", "0xCCCCCC",										// White
//	"-l", "i16", "0", "0",  "1", "1", "0xFF897D", "0xDB6559",										// Skin
//	"-l", "i8",  "0", "16", "2", "1", "0x010101",													// Black 1
//	"-l", "i8",  "0", "16", "2", "1", "0x010101", "0x5955E0", "0x3AA241", "0xCCCCCC", "0xDB6559",	// Black 2
//	"-l", "i8",  "0", "16", "2", "1", "0xFFFFFF", "0xCCCCCC",										// White
//	"-l", "i8",  "0", "16", "2", "1", "0xFF897D", "0xDB6559" };										// Skin
//const char* ARGV[] = { "", "test/score.png", "-out", "test/data_score.h", "-mode", "gm2", "-def", "-name", "g_DataScore", "-pos", "0", "0", "-size", "216", "80", "-compress", "rlep",
//	"-l", "gm2",   "0",  "88", "112", "96",
//	"-l", "gm2", "112",  "80",  "72", "24",
//	"-l", "gm2", "184",  "80",  "72", "24",
//	"-l", "gm2", "112", "104",  "72", "16",
//	"-l", "gm2", "184", "104",  "72", "16", };
//#define DEBUG_ARGS

/** Main entry point
	Usage: CMSXimg inFile -pos x y -size x y -num x y -out outFile -palette [16|256]
*/
int main(int argc, const char* argv[])
{
	// for debug purpose
#ifdef DEBUG_ARGS
	argc = sizeof(ARGV)/sizeof(ARGV[0]); argv = ARGV;
#endif

	FreeImage_Initialise();

	CMSX::FileFormat outFormat = CMSX::FILEFORMAT_Auto;
	ExportParameters param;
	i32 i;
	bool bAutoCompress = false;
	bool bBestCompress = false;

	if((argc < 2) || (CMSX::StrEqual(argv[1], "-help")))
	{
		PrintHelp();
		return 1;
	}
	param.inFile = argv[1];

	//-------------------------------------------------------------------------
	// Parse parameters
	for(i=2; i<argc; i++)
	{
		if (CMSX::StrEqual(argv[i], "-help")) // Display help
		{
			PrintHelp();
			return 0;
		}
		else if (CMSX::StrEqual(argv[i], "-out")) // Output filename
		{
			param.outFile = argv[++i];
		}
		else if (CMSX::StrEqual(argv[i], "-format")) // Output format
		{
			i++;
			if (CMSX::StrEqual(argv[i], "auto"))
				outFormat = CMSX::FILEFORMAT_Auto;
			else if (CMSX::StrEqual(argv[i], "c"))
				outFormat = CMSX::FILEFORMAT_C;
			else if (CMSX::StrEqual(argv[i], "asm"))
				outFormat = CMSX::FILEFORMAT_Asm;
			else if (CMSX::StrEqual(argv[i], "bin"))
				outFormat = CMSX::FILEFORMAT_Bin;
		}
		else if(CMSX::StrEqual(argv[i], "-pos")) // Extract start position
		{
			param.posX = atoi(argv[++i]);
			param.posY = atoi(argv[++i]);
		}
		else if(CMSX::StrEqual(argv[i], "-size")) // Block size
		{
			param.sizeX = atoi(argv[++i]);
			param.sizeY = atoi(argv[++i]);
		}
		else if (CMSX::StrEqual(argv[i], "-gap")) // Gap between blocks
		{
			param.gapX = atoi(argv[++i]);
			param.gapY = atoi(argv[++i]);
		}		
		else if(CMSX::StrEqual(argv[i], "-num")) // Column/rows blocks count
		{
			param.numX = atoi(argv[++i]);
			param.numY = atoi(argv[++i]);
		}
		else if (CMSX::StrEqual(argv[i], "-name")) // Data table name
		{
			param.tabName = argv[++i];
		}
		else if(CMSX::StrEqual(argv[i], "-bpc")) // Byte per color
		{
			param.bpc = atoi(argv[++i]);
		}
		else if(CMSX::StrEqual(argv[i], "-trans")) // Use transparency color
		{
			sscanf_s(argv[++i], "%i", &param.transColor);
			param.bUseTrans = true;
		}
		else if (CMSX::StrEqual(argv[i], "-opacity")) // Use opacity color
		{
			sscanf_s(argv[++i], "%i", &param.opacityColor);
			param.bUseOpacity = true;
		}
		else if (CMSX::StrEqual(argv[i], "-pal")) // Palette type
		{
			i++;
			if (CMSX::StrEqual(argv[i], "msx1"))
				param.palType = PALETTE_MSX1;
			else if (CMSX::StrEqual(argv[i], "custom"))
				param.palType = PALETTE_Custom;
		}
		else if (CMSX::StrEqual(argv[i], "--palcount") || CMSX::StrEqual(argv[i], "-palcount")) // Palette count
		{
			param.palCount = atoi(argv[++i]);
		}		
		else if (CMSX::StrEqual(argv[i], "--paloff") || CMSX::StrEqual(argv[i], "-paloff")) // Palette offset
		{
			param.palOffset = atoi(argv[++i]);
		}
		else if (CMSX::StrEqual(argv[i], "--pal24") || CMSX::StrEqual(argv[i], "-pal24")) // Palette 24b
		{
			param.pal24 = true;
		}
		else if(CMSX::StrEqual(argv[i], "-compress")) // Compression method
		{
			i++;
			if (CMSX::StrEqual(argv[i], "crop16"))
				param.comp = COMPRESS_Crop16;
			else if (CMSX::StrEqual(argv[i], "cropline16"))
				param.comp = COMPRESS_CropLine16;
			else if (CMSX::StrEqual(argv[i], "crop32"))
				param.comp = COMPRESS_Crop32;
			else if (CMSX::StrEqual(argv[i], "cropline32"))
				param.comp = COMPRESS_CropLine32;
			else if (CMSX::StrEqual(argv[i], "crop256"))
				param.comp = COMPRESS_Crop256;
			else if (CMSX::StrEqual(argv[i], "cropline256"))
				param.comp = COMPRESS_CropLine256;
			else if (CMSX::StrEqual(argv[i], "rle0"))
				param.comp = COMPRESS_RLE0;
			else if (CMSX::StrEqual(argv[i], "rle4"))
				param.comp = COMPRESS_RLE4;
			else if (CMSX::StrEqual(argv[i], "rle8"))
				param.comp = COMPRESS_RLE8;
			else if (CMSX::StrEqual(argv[i], "rlep"))
				param.comp = COMPRESS_RLEp;
			else if (CMSX::StrEqual(argv[i], "auto"))
				bAutoCompress = true;
			else if (CMSX::StrEqual(argv[i], "best"))
				bBestCompress = true;
			else
				param.comp = COMPRESS_None;
		}
		else if (CMSX::StrEqual(argv[i], "-dither")) // Dithering method
		{
			i++;
			if (CMSX::StrEqual(argv[i], "none"))
				param.dither = DITHER_None;
			else if (CMSX::StrEqual(argv[i], "floyd"))
				param.dither = DITHER_Floyd;
			else if (CMSX::StrEqual(argv[i], "bayer4"))
				param.dither = DITHER_Bayer4;
			else if (CMSX::StrEqual(argv[i], "bayer8"))
				param.dither = DITHER_Bayer8;
			else if (CMSX::StrEqual(argv[i], "bayer16"))
				param.dither = DITHER_Bayer16;
			else if (CMSX::StrEqual(argv[i], "cluster6"))
				param.dither = DITHER_Cluster6;
			else if (CMSX::StrEqual(argv[i], "cluster8"))
				param.dither = DITHER_Cluster8;
			else if (CMSX::StrEqual(argv[i], "cluster16"))
				param.dither = DITHER_Cluster16;
		}
		else if(CMSX::StrEqual(argv[i], "-data")) // Text data format
		{
			i++;
			if(CMSX::StrEqual(argv[i], "dec"))
				param.format = CMSX::DATAFORMAT_Decimal;
			else if(CMSX::StrEqual(argv[i], "hexa"))
				param.format = CMSX::DATAFORMAT_Hexa;
			else if(CMSX::StrEqual(argv[i], "hexa0x"))
				param.format = CMSX::DATAFORMAT_HexaC;
			else if(CMSX::StrEqual(argv[i], "hexaH"))
				param.format = CMSX::DATAFORMAT_HexaASM;
			else if(CMSX::StrEqual(argv[i], "hexa$"))
				param.format = CMSX::DATAFORMAT_HexaPascal;
			else if (CMSX::StrEqual(argv[i], "hexa&H"))
				param.format = CMSX::DATAFORMAT_HexaBasic;
			else if (CMSX::StrEqual(argv[i], "hexa&"))
				param.format = CMSX::DATAFORMAT_HexaAnd;
			else if (CMSX::StrEqual(argv[i], "hexa#"))
				param.format = CMSX::DATAFORMAT_HexaSharp;
			else if(CMSX::StrEqual(argv[i], "bin"))
				param.format = CMSX::DATAFORMAT_Binary;
			else if (CMSX::StrEqual(argv[i], "bin0b"))
				param.format = CMSX::DATAFORMAT_BinaryC;
			else if (CMSX::StrEqual(argv[i], "binB"))
				param.format = CMSX::DATAFORMAT_BinaryASM;
		}
		else if (CMSX::StrEqual(argv[i], "-mode")) // Exporter mode
		{
			i++;
			if (CMSX::StrEqual(argv[i], "bmp"))
				param.mode = MODE_Bitmap;
			else if (CMSX::StrEqual(argv[i], "gm1"))
				param.mode = MODE_GM1;
			else if (CMSX::StrEqual(argv[i], "gm2"))
				param.mode = MODE_GM2;
			else if (CMSX::StrEqual(argv[i], "sprt"))
				param.mode = MODE_Sprite;
		}
		else if (CMSX::StrEqual(argv[i], "-skip")) // Skip empty blocks
		{
			param.bSkipEmpty = true;
		}
		else if (CMSX::StrEqual(argv[i], "-idx")) // Index table
		{
			param.bAddIndex = true;
		}
		else if (CMSX::StrEqual(argv[i], "-copy")) // Copyright file
		{
			param.bAddCopy = true;
			if ((i < argc - 1) && *argv[i + 1] != '-')
			{
				param.copyFile = argv[++i];
			}
			else
			{
				param.copyFile = RemoveExt(param.inFile) + ".txt";
			}
		}
		else if (CMSX::StrEqual(argv[i], "-head")) // Add export data header
		{
			param.bAddHeader = true;
		}
		else if (CMSX::StrEqual(argv[i], "-font")) // Add font data header
		{
			param.bAddFont = true;
			param.fontX = atoi(argv[++i]);
			param.fontY = atoi(argv[++i]);
			i++;
			if(strlen(argv[i]) > 1) // is hexadecimal? (in '0xFF' format)
				param.fontFirst = (c8)strtol(argv[i], NULL, 16);
			else
				param.fontFirst = *argv[i];
			i++;
			if (strlen(argv[i]) > 1) // is hexadecimal? (in '0xFF' format)
				param.fontLast = (c8)strtol(argv[i], NULL, 16);
			else
				param.fontLast = *argv[i];
		}
		else if (CMSX::StrEqual(argv[i], "-at")) // Starting address
		{
			param.bStartAddr = true;
			i++;
			sscanf_s(argv[i], "%i", &param.startAddr);
		}
		else if (CMSX::StrEqual(argv[i], "-def")) // Add C define
		{
			param.bDefine= true;
		}
		else if (CMSX::StrEqual(argv[i], "-offset")) // Offset
		{
			param.offset = atoi(argv[++i]);
		}
		else if (CMSX::StrEqual(argv[i], "-notitle")) // Remove title
		{
			param.bTitle = false;
		}
		else if (CMSX::StrEqual(argv[i], "-l")) // Block layers
		{
			Layer l;
			i++;
			if (CMSX::StrEqual(argv[i], "i8"))
			{
				l.size16 = false;
				l.include = true;
			}
			else if (CMSX::StrEqual(argv[i], "i16"))
			{
				l.size16 = true;
				l.include = true;
			}
			else if (CMSX::StrEqual(argv[i], "e8"))
			{
				l.size16 = false;
				l.include = false;
			}
			else if (CMSX::StrEqual(argv[i], "e16"))
			{
				l.size16 = true;
				l.include = false;
			}
			l.posX = atoi(argv[++i]);
			l.posY = atoi(argv[++i]);
			l.numX = atoi(argv[++i]);
			l.numY = atoi(argv[++i]);
			while((i < argc - 1) && (argv[i+1][0] != '-'))
			{
				u32 c24;
				sscanf_s(argv[++i], "%i", &c24);
				l.colors.push_back(c24);
			}
			if (l.colors.size() == 0)
			{
				if (l.include)
					l.colors.push_back(0xFFFFFF);
				else // LAYER_Exclude
					l.colors.push_back(0x000000);
			}
			param.layers.push_back(l);
		}
		else if (CMSX::StrEqual(argv[i], "--gm2compnames")) // GM2 names compression
		{
			param.bGM2CompressNames = true;
		}
		else if (CMSX::StrEqual(argv[i], "--gm2unique")) // GM2 names compression
		{
			param.bGM2Unique = true;
		}
		else if (CMSX::StrEqual(argv[i], "--bload")) // Add header for BLOAD image
		{
			param.bBLOAD = true;
		}
	}

	//-------------------------------------------------------------------------
	if (param.palCount < 0) // Set default palette count
	{
		if (param.bpc == 2)
			param.palCount = 4 - param.palOffset;
		else if (param.bpc == 4)
			param.palCount = 16 - param.palOffset;
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
				if ((param.bpc == 1) || (param.bpc == 2))
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
		static const CMSXi_Compressor compTable[] =
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
		CMSXi_Compressor bestComp = COMPRESS_None;

		for (i32 i = 0; i < numberof(compTable); i++)
		{
			param.comp = compTable[i];
			printf("- Check %s... ", GetCompressorName(param.comp, true));
			if (IsCompressorCompatible(param.comp, param))
			{
				ExporterInterface* exp = new ExporterDummy(param.format, &param);
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

	//.........................................................................
	// Errors
	if (param.inFile == "")
	{
		printf("Error: Input file required!\n");
		return 1;
	}
	if (param.outFile == "")
	{
		switch (outFormat)
		{
		case CMSX::FILEFORMAT_C:
			param.outFile = RemoveExt(param.inFile) + ".h";
			break;
		case CMSX::FILEFORMAT_Asm:
			param.outFile = RemoveExt(param.inFile) + ".asm";
			break;
		case CMSX::FILEFORMAT_Bin:
			param.outFile = RemoveExt(param.inFile) + ".bin";
			break;
		case CMSX::FILEFORMAT_Auto:
		default:
			printf("Error: Output file is required if format is set to 'auto'!\n");
			return 1;
		}
	}
	if ((param.bpc != 1) && (param.bpc != 2) && (param.bpc != 4) && (param.bpc != 8))
	{
		printf("Error: Invalid bits-per-color value (%i). Only 1, 2, 4 or 8-bits colors are supported!\n", param.bpc);
		return 1;
	}
	if ((param.bAddCopy) && (!FileExists(param.copyFile)))
	{
		printf("Error: Copyright file not found (%s)!\n", param.copyFile.c_str());
		return 1;
	}
	if (param.bUseTrans && param.bUseOpacity)
	{
		printf("Error: Transparency and Opacity can't be use together!\n");
		return 1;
	}
	if (((param.bpc == 2) || (param.bpc == 4)) && (param.palCount < 1))
	{
		printf("Error: Palette count can't be less that 1 with 2-bits and 4-bits color mode!\n");
		return 1;
	}

	//.........................................................................
	// Warnings
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
	if (((param.bpc == 1) || (param.bpc == 2)) && (param.comp & COMPRESS_RLE_Mask))
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
	if ((param.bpc == 2) && (param.palOffset + param.palCount > 4))
	{
		printf("Warning: -paloffset is %i and -palcount is %i but total can't be more than 4 with 2-bits color (color index 0 is always transparent). Continue with 4 as value.\n", param.palOffset, param.palCount);
		param.palCount = 4 - param.palOffset;
	}
	if ((param.bpc == 4) && (param.palOffset + param.palCount > 16))
	{
		printf("Warning: -paloffset is %i and -palcount is %i but total can't be more than 16 with 4-bits color (color index 0 is always transparent). Continue with 16 as value.\n", param.palOffset, param.palCount);
		param.palCount = 16 - param.palOffset;
	}
	if ((param.dither != DITHER_None) && (param.bpc != 1))
	{
		printf("Warning: Dithering only work with 1-bit color format (current is %i-bits). Dithering value will be ignored.\n", param.bpc);
	}

	bool bSucceed = false;
	u32 size = 0;

	//-------------------------------------------------------------------------
	// Convert
	if((param.inFile != "") && (param.outFile != ""))
	{
		if((outFormat == CMSX::FILEFORMAT_C) || ((outFormat == CMSX::FILEFORMAT_Auto) && (HaveExt(param.outFile, ".h") || HaveExt(param.outFile, ".inc"))))
		{
			ExporterInterface* exp = new ExporterC(param.format, &param);
			bSucceed = ParseImage(&param, exp);
			size = exp->GetTotalBytes();
			delete exp;
		}
		else if((outFormat == CMSX::FILEFORMAT_Asm) || ((outFormat == CMSX::FILEFORMAT_Auto) && (HaveExt(param.outFile, ".s") || HaveExt(param.outFile, ".asm"))))
		{
			ExporterInterface* exp = new ExporterASM(param.format, &param);
			bSucceed = ParseImage(&param, exp);
			size = exp->GetTotalBytes();
			delete exp;
		}
		else if((outFormat == CMSX::FILEFORMAT_Bin) || ((outFormat == CMSX::FILEFORMAT_Auto) && (HaveExt(param.outFile, ".bin") || HaveExt(param.outFile, ".raw"))))
		{
			ExporterInterface* exp = new ExporterBin(param.format, &param);
			bSucceed = ParseImage(&param, exp);
			size = exp->GetTotalBytes();
			delete exp;
		}
		else
		{
			FIBITMAP *dib = LoadImage(param.inFile.c_str()); // open and load the file using the default load option
			if (dib == NULL)
			{
				printf("Error: Fail to load %s\n", param.inFile.c_str());
			}
			else
			{
				bSucceed = SaveImage(dib, param.outFile.c_str()); // save the file
				size = FreeImage_GetDIBSize(dib);
				FreeImage_Unload(dib); // free the dib
			}
		}
	}

	FreeImage_DeInitialise();
	if(bSucceed)
		printf("Succeed!\n");
	else
		printf("Error: Fatal error!\n");

	return bSucceed ? param.startAddr + size : 0;
}

