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
#include "color.h"
#include "exporter.h"
#include "image.h"
#include "parser.h"

struct RLEHash
{
	i32 length;
	u32 color;
	std::vector<u32> data;

	RLEHash() : length(0), color(0){}
};

//-----------------------------------------------------------------------------
// MSX interface
//-----------------------------------------------------------------------------

/***/
u8 GetNearestColorIndex(u32 color, u32* pal, i32 count)
{
	u8 bestIndex = 0;
	i32 bestWeight = 256 * 4;

	RGB24 c = RGB24(color);

	for (u8 i = 1; i <= count + 1; i++)
	{
		RGB24 p = RGB24(pal[i]);

		i32 weight = abs(p.R - c.R) + abs(p.G - c.G) + abs(p.B - c.B);
		if (weight < bestWeight)
		{
			bestWeight = weight;
			bestIndex = i;
		}
	}

	return bestIndex;
}

/***/
u8 GetGBR8(u32 color, u32 transRGB)
{
	u8 c8;

	if (color != transRGB)
	{
		RGB24 c24 = RGB24(color);
		c8 = GRB8(c24);
		if (c8 == 0)
		{
			if (c24.G > c24.R)
				c8 = 0x20;
			else
				c8 = 0x04;
		}
	}
	else
		c8 = 0;
	return c8;
}

/***/
bool ParseImage(ExportParameters* param, ExporterInterface* exp)
{
	FIBITMAP *dib, *dib32, *dib4, *dib1;
	i32 i, j, nx, ny, bit, minX, maxX, minY, maxY;
	RGB24 c24;
	GRB8 c8;
	u8 c4, byte = 0;
	char strData[256];
	u32 transRGB = 0x00FFFFFF & param->transColor;
	u32 headAddr = 0, palAddr = 0;
	std::vector<u16> sprtAddr;

	dib = LoadImage(param->inFile); // open and load the file using the default load option
	if (dib == NULL)
	{
		printf("Error: Fail to load %s\n", param->inFile);
		return false;
	}

	// Get 32 bits version
	dib32 = FreeImage_ConvertTo32Bits(dib);
	FreeImage_Unload(dib); // free the original dib
	i32 imageX = FreeImage_GetWidth(dib32);
	i32 imageY = FreeImage_GetHeight(dib32);
	i32 scanWidth = FreeImage_GetPitch(dib32);
	i32 bpp = FreeImage_GetBPP(dib32);
	BYTE* bits = new BYTE[scanWidth * imageY];
	FreeImage_ConvertToRawBits(bits, dib32, scanWidth, 32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE);

	// Get custom palette for 16 colors mode
	u32 customPalette[16];
	if ((param->bpc == 4) && (param->palType == PALETTE_Custom))
	{
		if (param->bUseTrans)
		{
			u32 black = 0;
			i32 res = FreeImage_ApplyColorMapping(dib32, (RGBQUAD*)&transRGB, (RGBQUAD*)&black, 1, true, false); // @warning: must be call AFTER retreving raw data!
		}
		dib4 = FreeImage_ColorQuantizeEx(dib32, FIQ_LFPQUANT, param->palCount, 0, NULL); // Try Lossless Fast Pseudo-Quantization algorithm (if there are 15 colors or less)
		if(dib4 == NULL)
			dib4 = FreeImage_ColorQuantizeEx(dib32, FIQ_WUQUANT, param->palCount, 0, NULL); // Else, use Efficient Statistical Computations for Optimal Color Quantization
		RGBQUAD* pal = FreeImage_GetPalette(dib4);
		customPalette[0] = 0;
		for (i32 c = 0; c < param->palCount; c++)
			customPalette[c + 1] = ((u32*)pal)[c];
		FreeImage_Unload(dib4);
	}
	// Apply dithering for 2 color mode
	else if ((param->bpc == 1) && (param->dither != DITHER_None))
	{
		dib1 = FreeImage_Dither(dib32, (FREE_IMAGE_DITHER)param->dither);
		FreeImage_ConvertToRawBits(bits, dib1, scanWidth, 32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE);
		FreeImage_Unload(dib1);
	}

	FreeImage_Unload(dib32);

	// Handle whole image case
	if ((param->sizeX == 0) || (param->sizeY == 0))
	{
		param->posX = param->posY = 0;
		param->sizeX = imageX;
		param->sizeY = imageY;
		param->numX = param->numY = 1;
	}

	// Build header file
	exp->WriteHeader();

	//-------------------------------------------------------------------------
	// HEADER TABLE

	if (param->bAddHeader)
	{
		sprintf_s(strData, 256, "%s_header", param->tabName);
		exp->WriteTableBegin(strData, "Header table");

		exp->Write2WordsLine((u16)param->sizeX, (u16)param->sizeY, "Sprite size (X Y)");
		exp->Write2WordsLine((u16)param->numX, (u16)param->numY, "Sprite count (X Y)");
		exp->Write1ByteLine((u8)param->bpc, "Bits per color");
		exp->Write1ByteLine((u8)param->comp, "Compressor");
		exp->Write1ByteLine(param->bSkipEmpty ? 1 : 0, "Skip empty");

		exp->WriteTableEnd("");
	}

	//-------------------------------------------------------------------------
	// FONT TABLE

	/*if (param->bAddFont)
	{
		sprintf_s(strData, 256, "%s_font", param->tabName);
		exp->WriteTableBegin(strData, "Font table");

		exp->Write1ByteLine((u8)((8 << 4) + (param->sizeY & 0x0F)), "Data size [x|y]");
		exp->Write1ByteLine((u8)(((param->fontX & 0x0F) << 4) + (param->fontY & 0x0F)), "Font size [x|y]");
		sprintf_s(strData, 256, "First character ASCII code (%c)", param->fontFirst);
		exp->Write1ByteLine((u8)param->fontFirst, strData);
		sprintf_s(strData, 256, "Last character ASCII code (%c)", param->fontLast);
		exp->Write1ByteLine((u8)param->fontLast, strData);

		exp->WriteTableEnd("");
	}*/

	//-------------------------------------------------------------------------
	// SPRITE TABLE

	sprtAddr.resize(param->numX * param->numY);
	exp->WriteTableBegin(param->tabName, "Data table");

	if (param->bAddFont)
	{
		exp->WriteCommentLine("Font header data");
		exp->Write1ByteLine((u8)((8 << 4) + (param->sizeY & 0x0F)), "Data size [x|y]");
		exp->Write1ByteLine((u8)(((param->fontX & 0x0F) << 4) + (param->fontY & 0x0F)), "Font size [x|y]");
		sprintf_s(strData, 256, "First character ASCII code (%c)", param->fontFirst);
		exp->Write1ByteLine((u8)param->fontFirst, strData);
		sprintf_s(strData, 256, "Last character ASCII code (%c)", param->fontLast);
		exp->Write1ByteLine((u8)param->fontLast, strData);
	}

	// Parse source image
	for(ny = 0; ny < param->numY; ny++)
	{
		for (nx = 0; nx < param->numX; nx++)
		{
			sprtAddr[nx + (ny * param->numX)] = (u16)exp->GetTotalBytes();

			// Print sprite header
			exp->WriteSpriteHeader(nx + (ny * param->numX));

			//-----------------------------------------------------------------
			//
			// RLE compression
			//
			//-----------------------------------------------------------------
			if (param->comp & COMPRESS_RLE_Mask)
			{
				i32 maxLength;
				switch (param->comp)
				{
				case COMPRESS_RLE0: maxLength = 0x7F; break;
				case COMPRESS_RLE4: maxLength = 0x0F; break;
				default:            maxLength = 0xFF; // COMPRESS_RLE8
				}

				// Hash sprite data
				std::vector<RLEHash> hashTable;
				for (j = 0; j < param->sizeY; j++)
				{
					for (i = 0; i < param->sizeX; i++)
					{
						i32 pixel = param->posX + i + (nx * (param->sizeX + param->gapX)) + ((param->posY + j + (ny * (param->sizeY + param->gapY))) * imageX);
						u32 rgb = 0xFFFFFF & ((u32*)bits)[pixel];

						if (param->comp == COMPRESS_RLE0) // Transparency color Run-length encoding
						{
							if ((hashTable.size() != 0) && (rgb == transRGB) && (hashTable.back().color == transRGB) && (hashTable.back().length < maxLength))
							{
								hashTable.back().length++;
							}
							else if ((hashTable.size() != 0) && (rgb != transRGB) && (hashTable.back().color != transRGB) && (hashTable.back().length < maxLength))
							{
								hashTable.back().length++;
								hashTable.back().data.push_back(rgb);
							}
							else
							{
								RLEHash hash;
								hash.color = rgb;
								hash.length = 1;
								hash.data.push_back(rgb);
								hashTable.push_back(hash);
							}
						}
						else if ((param->comp == COMPRESS_RLE4) || (param->comp == COMPRESS_RLE8)) // Full color Run-length encoding
						{
							if ((hashTable.size() != 0) && (rgb == hashTable.back().color) && (hashTable.back().length < maxLength))
							{
								hashTable.back().length++;
							}
							else
							{
								RLEHash hash;
								hash.color = rgb;
								hash.length = 1;
								hashTable.push_back(hash);
							}
						}
					}
				}

				// Write hash table
				for (u32 k = 0; k < hashTable.size(); k++)
				{
					exp->WriteLineBegin();
					if (param->comp == COMPRESS_RLE0) // Transparency color Run-length encoding
					{
						if (hashTable[k].color == transRGB)
						{
							exp->Write1ByteData(0x80 + (u8)hashTable[k].length);
						}
						else
						{
							exp->Write1ByteData((u8)hashTable[k].length);
							if (param->bpc == 4) // 4-bits index color palette
							{
								u8 byte;
								for (u32 l = 0; l < hashTable[k].data.size(); l++)
								{
									u32 rgb = hashTable[k].color;
									u32* pal = (param->palType == PALETTE_MSX1) ? PaletteMSX : customPalette;
									if (param->bUseTrans)
										c4 = (rgb == transRGB) ? 0x0 : GetNearestColorIndex(rgb, pal, param->palCount);
									else
										c4 = GetNearestColorIndex(rgb, pal, param->palCount);
									if (l & 0x1)
										byte |= c4; // Second pixel use lower bits
									else
										byte |= (c4 << 4); // First pixel use higher bits
									if ((l & 0x1) || (l == hashTable[k].data.size() - 1))
									{
										exp->Write1ByteData(byte);
										byte = 0;
									}
								}
							}
							else if (param->bpc == 8) // 8-bits GBR color
							{
								for (u32 l = 0; l < hashTable[k].data.size(); l++)
								{
									c8 = GetGBR8(hashTable[k].data[l], transRGB);
									exp->Write1ByteData(c8);
								}
							}
						}
					}
					else if (param->comp == COMPRESS_RLE4) // Full color 4bits Run-length encoding
					{
						if (param->bpc == 4) // 4-bits index color palette
						{
							u32 rgb = hashTable[k].color;
							u32* pal = (param->palType == PALETTE_MSX1) ? PaletteMSX : customPalette;
							if (param->bUseTrans)
								c4 = (rgb == transRGB) ? 0x0 : GetNearestColorIndex(rgb, pal, param->palCount);
							else
								c4 = GetNearestColorIndex(rgb, pal, param->palCount);
							u8 byte = ((0x0F & hashTable[k].length) << 4) + c4;
							exp->Write1ByteData(byte);
						}
					}
					else if (param->comp == COMPRESS_RLE8) // Full color 8bits Run-length encoding
					{
						if (param->bpc == 4) // 4-bits index color palette
						{
							exp->Write1ByteData((u8)hashTable[k].length);
							u32 rgb = hashTable[k].color;
							u32* pal = (param->palType == PALETTE_MSX1) ? PaletteMSX : customPalette;
							if (param->bUseTrans)
								c4 = (rgb == transRGB) ? 0x0 : GetNearestColorIndex(rgb, pal, param->palCount);
							else
								c4 = GetNearestColorIndex(rgb, pal, param->palCount);
							exp->Write1ByteData(c4);
						}
						else if (param->bpc == 8) // 8-bits GBR color
						{
							exp->Write1ByteData((u8)hashTable[k].length);
							c8 = GetGBR8(hashTable[k].color, transRGB);
							exp->Write1ByteData(c8);
						}
					}
					exp->WriteLineEnd();
				}
			}
			//-----------------------------------------------------------------
			//
			// Crop & No compression
			//
			//-----------------------------------------------------------------
			else
			{
				minX = 0;
				maxX = param->sizeX - 1;
				minY = 0;
				maxY = param->sizeY - 1;

				if (param->bUseTrans)
				{
					// Compute bound for crop compression and count non transparent pixels
					i32 count = 0;
					if (param->comp & COMPRESS_Crop_Mask)
					{
						minX = param->sizeX;
						maxX = 0;
						minY = param->sizeY;
						maxY = 0;
					}
					for (j = 0; j < param->sizeY; j++)
					{
						for (i = 0; i < param->sizeX; i++)
						{
							i32 pixel = param->posX + i + (nx * (param->sizeX + param->gapX)) + ((param->posY + j + (ny * (param->sizeY + param->gapY))) * imageX);
							u32 rgb = 0xFFFFFF & ((u32*)bits)[pixel];
							if (rgb != transRGB)
							{
								if (param->comp & COMPRESS_Crop_Mask)
								{
									if (i < minX)
										minX = i;
									if (i > maxX)
										maxX = i;
									if (j < minY)
										minY = j;
									if (j > maxY)
										maxY = j;
								}
								count++;
							}
						}
					}

					// Handle Empty
					if (count == 0)
					{
						if (param->bSkipEmpty)
						{
							sprtAddr[nx + (ny * param->numX)] = MSXi_NO_ENTRY;
							continue;
						}
						else if (param->comp & COMPRESS_Crop_Mask)
							minX = maxX = minY = maxY = 0;
					}

					// Sprite header
					if ((param->comp & COMPRESS_Crop_Mask))
					{
						if (param->comp == COMPRESS_Crop16)
						{
							minX &= 0x0F; // Clamp to 4bits (0-15)
							maxX &= 0x0F;
							minY &= 0x0F;
							maxY &= 0x0F;
							exp->Write2BytesLine(u8((minX << 4) + maxX), u8(((minY) << 4) + maxY), "[minX:4|maxX:4] [minY:4|maxY:4]");
						}
						else if (param->comp == COMPRESS_CropLine16)
						{
							minY &= 0x0F;
							maxY &= 0x0F;
							exp->Write1ByteLine(u8((minY << 4) + maxY), "[minY:4|maxY:4]");
						}
						else if (param->comp == COMPRESS_Crop32)
						{
							if (minX > 0x07)
								minX = 0x07; // Max to 3bits (0-7)
							maxX &= 0x1F;    // Clamp to 5bits (0-31)
							if (minY > 0x07)
								minY = 0x07; // Max to 3bits (0-7)
							maxY &= 0x1F;    // Clamp to 5bits (0-31)
							exp->Write2BytesLine(u8((minX << 5) + maxX), u8(((minY) << 5) + maxY), "[minX:3|maxX:5] [minY:3|maxY:5]");
						}
						else if (param->comp == COMPRESS_CropLine32)
						{
							if (minY > 0x07)
								minY = 0x07; // Max to 3bits (0-7)
							maxY &= 0x1F;    // Clamp to 5bits (0-31)
							exp->Write1ByteLine(u8(((minY) << 5) + maxY), "[minY:3|maxY:5]");
						}
						else if (param->comp == COMPRESS_Crop256)
						{
							exp->Write4BytesLine(u8(minX), u8(maxX), u8(minY), u8(maxY), "[minX] [maxX] [minY] [maxY]");
						}
						else if (param->comp == COMPRESS_CropLine256)
						{
							exp->Write2BytesLine(u8(minY), u8(maxY), "[minY] [maxY]");
						}
					}
				}

				// Print sprite content
				for (j = 0; j < param->sizeY; j++)
				{
					if ((j >= minY) && (j <= maxY))
					{
						// for line-crop, we need to recompute minX&maxX for each line
						if (param->comp & COMPRESS_CropLine_Mask)
						{
							minX = param->sizeX;
							maxX = 0;
							for (i = 0; i < param->sizeX; i++)
							{
								i32 pixel = param->posX + i + (nx * (param->sizeX + param->gapX)) + ((param->posY + j + (ny * (param->sizeY + param->gapY))) * imageX);
								u32 rgb = 0xFFFFFF & ((u32*)bits)[pixel];
								if (rgb  != transRGB)
								{
									if (i < minX)
										minX = i;
									if (i > maxX)
										maxX = i;
								}
							}
							if (param->bpc == 4) // 4-bits index color palette
							{
								minX &= 0xFE; // round 2
							}

							// Add row range info
							if (param->comp == COMPRESS_CropLine16)
							{
								minX &= 0x0F;	 // Clamp to 4bits (0-15)
								maxX &= 0x0F;	 // Clamp to 4bits (0-15)
								exp->Write1ByteLine(u8((minX << 4) + maxX), "[minX:4|maxX:4]");
							}
							else if (param->comp == COMPRESS_CropLine32)
							{
								if (minX > 0x07)
									minX = 0x07; // Clamp to 3bits (0-7)
								maxX &= 0x1F;	 // Clamp to 5bits (0-31)
								exp->Write1ByteLine(u8(((minX) << 5) + maxX), "[minX:3|maxX:5]");
							}
							else if (param->comp == COMPRESS_CropLine256)
							{
								exp->Write2BytesLine(u8(minX), u8(maxX), "[minX] [maxX]");
							}
						}

						// Add sprinte data
						exp->WriteLineBegin();
						byte = 0;
						for (i = 0; i < param->sizeX; i++)
						{
							if ((i >= minX) && (i <= maxX))
							{
								i32 pixel = param->posX + i + (nx * (param->sizeX + param->gapX)) + ((param->posY + j + (ny * (param->sizeY + param->gapY))) * imageX);
								u32 rgb = 0xFFFFFF & ((u32*)bits)[pixel];
								if (param->bpc == 8) // 8-bits GBR color
								{
									// convert to 8 bits GRB
									c8 = GetGBR8(rgb, transRGB);
									exp->Write1ByteData((u8)c8);
								}
								else if (param->bpc == 4) // 4-bits index color palette
								{
									u32* pal = (param->palType == PALETTE_MSX1) ? PaletteMSX : customPalette;
									if (param->bUseTrans)
										c4 = (rgb == transRGB) ? 0x0 : GetNearestColorIndex(rgb, pal, param->palCount);
									else
										c4 = GetNearestColorIndex(rgb, pal, param->palCount);

									if (i & 0x1)
										byte |= c4; // Second pixel use lower bits
									else
										byte |= (c4 << 4); // First pixel use higher bits
									if ((i & 0x1) || (i == maxX))
									{
										exp->Write1ByteData(byte);
										byte = 0;
									}
								}
								else if (param->bpc == 1) // Black & white
								{
									bit = pixel & 0x7;
									if (param->bUseTrans)
									{
										if (rgb != transRGB) // All non-transparent color are 1
											byte |= 1 << (7 - bit);
									}
									else
									{
										if (rgb != 0) // All non-black color are 1
											byte |= 1 << (7 - bit);
									}
									if (((pixel & 0x7) == 0x7) || (i == maxX))
									{
										exp->Write8BitsData(byte);
										byte = 0;
									}
								}
							}
						}
						exp->WriteLineEnd();
					}
				}
			}
		}
	}
	sprintf_s(strData, 256, "Total size : % i bytes", exp->GetTotalBytes());
	exp->WriteTableEnd(strData);

	delete bits;

	//-------------------------------------------------------------------------
	// INDEX TABLE

	if (param->bAddIndex)
	{
		sprintf_s(strData, 256, "%s_index", param->tabName);
		exp->WriteTableBegin(strData, "Images index");
		for (i32 i = 0; i < (i32)sprtAddr.size(); i++)
		{
			exp->Write1WordLine(sprtAddr[i], "");
		}
		exp->WriteTableEnd("");
	}

	//-------------------------------------------------------------------------
	// PALETTE TABLE

	if ((param->bpc == 4) && (param->palType == PALETTE_Custom))
	{
		sprintf_s(strData, 256, "%s_palette", param->tabName);
		exp->WriteTableBegin(strData, "Custom palette | Format: [X|R:3|X|B:3] [X:5|G:3]");
		for (i32 i = 1; i <= param->palCount; i++)
		{
			RGB24 color(customPalette[i]);
			u8 c1 = ((color.R >> 5) << 4) + (color.B >> 5);
			u8 c2 = (color.G >> 5);
			sprintf_s(strData, 256, "[%2i] #%06X", i, customPalette[i]);
			exp->Write2BytesLine(u8(c1), u8(c2), strData);
		}
		exp->WriteTableEnd("");
	}

	// Write file
	bool bSaved = exp->Export();

	return bSaved;
}

/** Build 256 colors palette */
void Create256ColorsPalette(const char* filename)
{
	RGB24 ColorTable[256];
	FILE* file;
	// Create table
	for(int i=0; i<256; i++)
	{
		ColorTable[i] = RGB24(GRB8(i));
	}
	// Save
	fopen_s(&file, filename, "wb");
	fwrite(ColorTable, sizeof(ColorTable), 1, file);
	fclose(file);
}

/** Build 16 colors palette */
void Create16ColorsPalette(const char* filename)
{
	RGB24 ColorTable[256];
	FILE* file;
	// Create table
	for(int i=0; i<256; i++)
	{
		switch(i)
		{
			case 2:  ColorTable[i] = RGB24(36,  218, 36); break;
			case 3:  ColorTable[i] = RGB24(109, 255, 109); break;
			case 4:  ColorTable[i] = RGB24(36,  255, 36); break;
			case 5:  ColorTable[i] = RGB24(72,  109, 255); break;
			case 6:  ColorTable[i] = RGB24(182, 36,  36); break;
			case 7:  ColorTable[i] = RGB24(72,  218, 255); break;
			case 8:  ColorTable[i] = RGB24(255, 36,  36); break;
			case 9:  ColorTable[i] = RGB24(255, 109, 109); break;
			case 10: ColorTable[i] = RGB24(218, 218, 36); break;
			case 11: ColorTable[i] = RGB24(218, 218, 145); break;
			case 12: ColorTable[i] = RGB24(36,  145, 36); break;
			case 13: ColorTable[i] = RGB24(218, 72,  182); break;
			case 14: ColorTable[i] = RGB24(182, 182, 182); break;
			case 15: ColorTable[i] = RGB24(255, 255, 255); break;
			default: ColorTable[i] = RGB24(0, 0, 0);
		}
	}
	// Save
	fopen_s(&file, filename, "wb");
	fwrite(ColorTable, sizeof(ColorTable), 1, file);
	fclose(file);
}