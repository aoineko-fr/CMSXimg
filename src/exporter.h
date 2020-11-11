//     _____    _____________  ___ .___                               
//    /     \  /   _____/\   \/  / |   | _____ _____     ____   ____  
//   /  \ /  \ \_____  \  \     /  |   |/     \\__  \   / ___\_/ __ \ 
//  /    Y    \/        \ /     \  |   |  Y Y  \/ __ \_/ /_/  >  ___/ 
//  \____|__  /_______  //___/\  \ |___|__|_|  (____  /\___  / \___  >
//          \/        \/       \_/           \/     \//_____/      \/ 
//
// by Guillaume "Aoineko" Blanchard (aoineko@free.fr)
// under CC-BY-AS license (https://creativecommons.org/licenses/by-sa/2.0/)

#pragma once

// std
#include <string>
#include <vector>
// MSXImage
#include "types.h"
#include "color.h"

//-----------------------------------------------------------------------------
// Compression mode
enum Compressor
{
	COMPRESS_None          = 0b00000000, // No compression

	// Crop compression
	COMPRESS_Crop16        = 0b00000001, // Crop sprite to keep only the non-transparent area (max size 16x16)
	COMPRESS_Crop32        = 0b00000010, // Crop sprite to keep only the non-transparent area (max size 32x32)
	COMPRESS_Crop256       = 0b00000011, // Crop sprite to keep only the non-transparent area (max size 256x256)
	COMPRESS_CropLine_Mask = 0b00001000, // Bits mask
	COMPRESS_CropLine16    = COMPRESS_Crop16  | COMPRESS_CropLine_Mask, // Crop each sprite line (max size 16x16)
	COMPRESS_CropLine32    = COMPRESS_Crop32  | COMPRESS_CropLine_Mask, // Crop each sprite line (max size 32x32)
	COMPRESS_CropLine256   = COMPRESS_Crop256 | COMPRESS_CropLine_Mask, // Crop each sprite line (max size 256x256)
	COMPRESS_Crop_Mask     = 0b00001111, // Bits mask

	// RLE compression
	COMPRESS_RLE0          = 0b00010000, // Run-length encoding of transparent blocs (7-bits for block length)
	COMPRESS_RLE4          = 0b00100000, // Run-length encoding for all colors (4-bits for block length)
	COMPRESS_RLE8          = 0b00110000, // Run-length encoding for all colors (8-bits for block length)
	COMPRESS_RLE_Mask      = 0b00110000, // Bits mask
};

const char* GetCompressorName(Compressor comp);

/// Format of the data
enum DataFormat
{
	DATA_Decimal,
	DATA_Hexa,
	DATA_HexaC,      // 0x00, 0xD2, 0xFF
	DATA_HexaAsm,    // 00h, 0D2h, 0FFh
	DATA_HexaDollar, // $00, $D2, $FF
	DATA_HexaSharp,  // #00, #D2, #FF
	DATA_Binary,
};


struct ExportParameters
{
	const char* inFile;
	const char* outFile;
	const char* tabName;
	i32 posX;
	i32 posY;
	i32 sizeX;
	i32 sizeY;
	i32 numX;
	i32 numY;
	i32 bpc;
	bool bUseTrans;
	u32 transColor;
	PaletteType palType;
	i32 palCount;
	Compressor comp;
	DataFormat dataType;
	bool bSkipEmpty;
	DitheringMethod dither;

	ExportParameters()
	{
		inFile = NULL;
		outFile = NULL;
		tabName = "table";
		posX = 0;
		posY = 0;
		sizeX = 0;
		sizeY = 0;
		numX = 1;
		numY = 1;
		bpc = 8;
		bUseTrans = false;
		transColor = 0;
		palType = PALETTE_MSX1;
		palCount = 15;
		comp = COMPRESS_None;
		dataType = DATA_Hexa;
		bSkipEmpty = false;
		dither = DITHER_None;
	}
};

/**
 * Exporter interface
 */
class ExporterInterface
{
protected:
	DataFormat eFormat;
	ExportParameters* Param;

public:
	ExporterInterface(DataFormat f, ExportParameters* p): eFormat(f), Param(p) {}
	virtual i32 WriteHeader() = 0;
	virtual i32 WriteTableBegin(std::string name, std::string comment) = 0;
	virtual i32 WriteSpriteHeader(i32 number, i32 offset) = 0;
	virtual i32 Write1ByteLine(u8 a, std::string comment) = 0;
	virtual i32 Write2BytesLine(u8 a, u8 b, std::string comment) = 0;
	virtual i32 Write4BytesLine(u8 a, u8 b, u8 c, u8 d, std::string comment) = 0;
	virtual i32 WriteLineBegin() = 0;
	virtual i32 Write1ByteData(u8 data) = 0;
	virtual i32 Write8BitsData(u8 data) = 0;
	virtual i32 WriteLineEnd() = 0;
	virtual i32 WriteTableEnd(std::string comment) = 0;

	virtual const c8* GetNumberFormat() = 0;

	virtual bool Save() = 0;
};

/**
 * Text exporter interface
 */
class ExporterText : public ExporterInterface
{
protected:
#define BUFFER_SIZE 1024
	char strFormat[BUFFER_SIZE];
	char strData[BUFFER_SIZE];
	std::string outData;

public:
	ExporterText(DataFormat f, ExportParameters* p) : ExporterInterface(f, p) {}
	virtual i32 WriteHeader() = 0;
	virtual i32 WriteTableBegin(std::string name, std::string comment) = 0;
	virtual i32 WriteSpriteHeader(i32 number, i32 offset) = 0;
	virtual i32 Write1ByteLine(u8 a, std::string comment) = 0;
	virtual i32 Write2BytesLine(u8 a, u8 b, std::string comment) = 0;
	virtual i32 Write4BytesLine(u8 a, u8 b, u8 c, u8 d, std::string comment) = 0;
	virtual i32 WriteLineBegin() = 0;
	virtual i32 Write1ByteData(u8 data) = 0;
	virtual i32 Write8BitsData(u8 data) = 0;
	virtual i32 WriteLineEnd() = 0;
	virtual i32 WriteTableEnd(std::string comment) = 0;

	virtual const c8* GetNumberFormat() = 0;

	virtual bool Save()
	{
		// Write header file
		FILE* file;
		if (fopen_s(&file, Param->outFile, "wb") != 0)
		{
			printf("Error: Fail to create %s\n", Param->outFile);
			return false;
		}
		fwrite(outData.c_str(), 1, outData.size(), file);
		fclose(file);
		return true;
	}
};
	
/**
 * C++ langage exporter
 */
class ExporterC: public ExporterText
{
public:
	ExporterC(DataFormat f, ExportParameters* p): ExporterText(f, p) {}

	virtual const c8* GetNumberFormat()
	{
		switch(eFormat)
		{
		case DATA_Decimal:
			return "%3u";
		case DATA_Hexa:
		case DATA_HexaC:
		case DATA_HexaAsm:
		case DATA_HexaDollar:
		case DATA_HexaSharp:
			return "0x%02X";
		case DATA_Binary:
		default:
			return "0x%02X";
		}	
	}

	virtual i32 WriteHeader()
	{
		sprintf_s(strData, BUFFER_SIZE, 
			"// Sprite table generated by MSXImage (v%s)\n"
			"// - Input file:     %s\n"
			"// - Start position: %i, %i\n"
			"// - Sprite size:    %i, %i\n"
			"// - Sprite count:   %i, %i\n"
			"// - Color count:    %i (Transparent: #%04X)\n"
			"// - Compressor:     %s\n"
			"// - Skip empty:     %s\n",
			VERSION, Param->inFile, Param->posX, Param->posY, Param->sizeX, Param->sizeY, Param->numX, Param->numY, 1 << Param->bpc, Param->transColor,
			GetCompressorName(Param->comp), Param->bSkipEmpty ? "TRUE" : "FALSE");
		outData += strData; 
		return 0;
	}

	virtual i32 WriteTableBegin(std::string name, std::string comment)
	{
		sprintf_s(strData, BUFFER_SIZE,
			"\n"
			"// %s\n"
			"const unsigned char %s[] =\n"
			"{\n",
			comment.c_str(), name.c_str());
		outData += strData;
		return 0;
	}

	virtual i32 WriteSpriteHeader(i32 number, i32 offset)
	{ 
		sprintf_s(strData, BUFFER_SIZE,
			"// Sprite[%i] (offset:%i)\n", number, offset);
		outData += strData;
		return 0;
	}

	virtual i32 Write4BytesLine(u8 minX, u8 minY, u8 sizeX, u8 sizeY, std::string comment)
	{
		sprintf_s(strFormat, BUFFER_SIZE, 
			"\t%s, %s, %s, %s, // %s\n", GetNumberFormat(), GetNumberFormat(), GetNumberFormat(), GetNumberFormat(), comment.c_str());
		sprintf_s(strData, BUFFER_SIZE, strFormat, minX, minY, sizeX, sizeY);
		outData += strData;
		return 4;
	}

	virtual i32 Write2BytesLine(u8 minXY, u8 sizeXY, std::string comment)
	{
		sprintf_s(strFormat, BUFFER_SIZE, 
			"\t%s, %s, // %s\n", GetNumberFormat(), GetNumberFormat(), comment.c_str());
		sprintf_s(strData, BUFFER_SIZE, strFormat, minXY, sizeXY);
		outData += strData;
		return 2;
	}

	virtual i32 Write1ByteLine(u8 minSizeY, std::string comment)
	{
		sprintf_s(strFormat, BUFFER_SIZE, 
			"\t%s, // %s\n", GetNumberFormat(), comment.c_str());
		sprintf_s(strData, BUFFER_SIZE, strFormat, minSizeY);
		outData += strData;
		return 1;
	}

	virtual i32 WriteLineBegin()
	{ 
		outData += "\t";
		return 0;
	}

	virtual i32 Write1ByteData(u8 data)
	{
		sprintf_s(strFormat, BUFFER_SIZE, "%s, ", GetNumberFormat());
		sprintf_s(strData, BUFFER_SIZE, strFormat, data);
		outData += strData;
		return 1;
	}

	virtual i32 Write8BitsData(u8 data)
	{
		sprintf_s(strFormat, BUFFER_SIZE, 
			"%s, /* %%c%%c%%c%%c%%c%%c%%c%%c */ ", GetNumberFormat());
		sprintf_s(strData, BUFFER_SIZE, strFormat, data, 
			data & 0x80 ? '#' : '.', 
			data & 0x40 ? '#' : '.', 
			data & 0x20 ? '#' : '.', 
			data & 0x10 ? '#' : '.', 
			data & 0x08 ? '#' : '.', 
			data & 0x04 ? '#' : '.', 
			data & 0x02 ? '#' : '.', 
			data & 0x01 ? '#' : '.');
		outData += strData;
		return 1;
	}

	virtual i32 WriteLineEnd()
	{ 
		outData += "\n";
		return 0;
	}

	virtual i32 WriteTableEnd(std::string comment)
	{
		sprintf_s(strData, BUFFER_SIZE, 
			"};\n"
			"// %s\n", comment.c_str());
		outData += strData;
		return 0;
	}
};


/**
 * C++ langage exporter
 */
class ExporterASM: public ExporterText
{
public:
	ExporterASM(DataFormat f, ExportParameters* p): ExporterText(f, p) {}

	virtual const c8* GetNumberFormat()
	{
		switch(eFormat)
		{
		case DATA_Decimal:
			return "%3u";
		case DATA_Hexa:
		case DATA_HexaC:
			return "0x%02X";
		case DATA_HexaAsm:
			return "0%02Xh";
		case DATA_HexaDollar:
			return "$%02X";
		case DATA_HexaSharp:
			return "#%02X";
		case DATA_Binary:
		default:
			return "%3u";
		}	
	}

	virtual i32 WriteHeader()
	{
		sprintf_s(strData, BUFFER_SIZE,
			"; Sprite table generated by MSXImage (v%s)\n"
			"; - Input file:     %s\n"
			"; - Start position: %i, %i\n"
			"; - Sprite size:    %i, %i\n"
			"; - Sprite count:   %i, %i\n"
			"; - Color count:    %i (Transparent: #%04X)\n"
			"; - Compressor:     %s\n"
			"; - Skip empty:     %s\n",
			VERSION, Param->inFile, Param->posX, Param->posY, Param->sizeX, Param->sizeY, Param->numX, Param->numY, 1 << Param->bpc, Param->transColor,
			GetCompressorName(Param->comp), Param->bSkipEmpty ? "TRUE" : "FALSE");
		outData += strData;
		return 0;
	}

	virtual i32 WriteTableBegin(std::string name, std::string comment)
	{
		sprintf_s(strData, BUFFER_SIZE,
			"\n"
			"; %s\n"
			"%s:\n",
			comment.c_str(), name.c_str());
		outData += strData;
		return 0;
	}

	virtual i32 WriteSpriteHeader(i32 number, i32 offset)
	{ 
		sprintf_s(strData, BUFFER_SIZE, 
			"; Sprite[%i] (offset:%i)\n", number, offset);
		outData += strData;
		return 0;
	}

	virtual i32 Write4BytesLine(u8 minX, u8 minY, u8 sizeX, u8 sizeY, std::string comment)
	{
		sprintf_s(strFormat, BUFFER_SIZE, 
			"\t.db %s %s %s %s ; %s\n", GetNumberFormat(), GetNumberFormat(), GetNumberFormat(), GetNumberFormat(), comment.c_str());
		sprintf_s(strData, BUFFER_SIZE, strFormat, minX, minY, sizeX, sizeY);
		outData += strData;
		return 4;
	}

	virtual i32 Write2BytesLine(u8 minXY, u8 sizeXY, std::string comment)
	{
		sprintf_s(strFormat, BUFFER_SIZE, 
			"\t.db %s %s ; %s\n", GetNumberFormat(), GetNumberFormat(), comment.c_str());
		sprintf_s(strData, BUFFER_SIZE, strFormat, minXY, sizeXY);
		outData += strData;
		return 2;
	}

	virtual i32 Write1ByteLine(u8 minSizeY, std::string comment)
	{
		sprintf_s(strFormat, BUFFER_SIZE, 
			"\t.db %s ; %s\n", GetNumberFormat(), comment.c_str());
		sprintf_s(strData, BUFFER_SIZE, strFormat, minSizeY);
		outData += strData;
		return 1;
	}

	virtual i32 WriteLineBegin()
	{ 
		outData += "\t.db ";
		return 0;
	}

	virtual i32 Write1ByteData(u8 data)
	{
		sprintf_s(strFormat, BUFFER_SIZE, "%s ", GetNumberFormat());
		sprintf_s(strData, BUFFER_SIZE, strFormat, data);
		outData += strData;
		return 1;
	}

	virtual i32 Write8BitsData(u8 data)
	{
		sprintf_s(strFormat, BUFFER_SIZE, "%s ", GetNumberFormat());
		sprintf_s(strData, BUFFER_SIZE, strFormat, data);
		outData += strData;
		return 1;
	}

	virtual i32 WriteLineEnd()
	{ 
		outData += "\n";
		return 0;
	}

	virtual i32 WriteTableEnd(std::string comment)
	{
		sprintf_s(strData, BUFFER_SIZE, 
			"; %s", comment.c_str());
		outData += strData;
		return 0;
	}
};

/**
 * Exporter interface
 */
class ExporterBin: public ExporterInterface
{
protected:
#define BUFFER_SIZE 1024
	std::vector<u8> outData;

public:
	ExporterBin(DataFormat f, ExportParameters* p) : ExporterInterface(f, p) {}
	virtual i32 WriteHeader() { return 0; };
	virtual i32 WriteTableBegin(std::string name, std::string comment) { return 0; };
	virtual i32 WriteSpriteHeader(i32 number, i32 offset) { return 0; };
	virtual i32 Write1ByteLine(u8 a, std::string comment)
	{ 
		outData.push_back(a); 
		return 1;
	};
	virtual i32 Write2BytesLine(u8 a, u8 b, std::string comment)
	{ 
		outData.push_back(a); 
		outData.push_back(b); 
		return 2; 
	};
	virtual i32 Write4BytesLine(u8 a, u8 b, u8 c, u8 d, std::string comment)
	{ 
		outData.push_back(a); 
		outData.push_back(b); 
		outData.push_back(c); 
		outData.push_back(d); 
		return 4; 
	};
	virtual i32 WriteLineBegin() { return 0; };
	virtual i32 Write1ByteData(u8 data)
	{ 
		outData.push_back(data);
		return 1; 
	};
	virtual i32 Write8BitsData(u8 data)
	{ 
		outData.push_back(data);
		return 1; 
	};
	virtual i32 WriteLineEnd() { return 0; };
	virtual i32 WriteTableEnd(std::string comment) { return 0; };

	virtual const c8* GetNumberFormat() { return NULL; }

	virtual bool Save()
	{
		// Write header file
		FILE* file;
		if (fopen_s(&file, Param->outFile, "wb") != 0)
		{
			printf("Error: Fail to create %s\n", Param->outFile);
			return false;
		}
		fwrite(outData.data(), 1, outData.size(), file);
		fclose(file);
		return true;
	}
};

