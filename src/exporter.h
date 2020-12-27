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
#include "msxi.h"
#include "types.h"
#include "color.h"


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

/// Format of the data
enum TableFormat
{
	TABLE_U8,
	TABLE_U16,
	TABLE_U32,
	TABLE_Header,
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
	i32 gapX;
	i32 gapY;
	i32 numX;
	i32 numY;
	i32 bpc;
	bool bUseTrans;
	u32 transColor;
	PaletteType palType;
	i32 palCount;
	MSXi_Compressor comp;
	DataFormat dataType;
	bool bSkipEmpty;
	DitheringMethod dither;
	bool bAddHeader;
	bool bAddIndex;
	bool bAddFont;
	c8 fontFirst;
	c8 fontLast;
	u8 fontX;
	u8 fontY;
	bool bDefine;

	ExportParameters()
	{
		inFile = NULL;
		outFile = NULL;
		tabName = "table";
		posX = 0;
		posY = 0;
		sizeX = 0;
		sizeY = 0;
		gapX = 0;
		gapY = 0;
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
		bAddHeader = false;
		bAddIndex = false;
		bAddFont = false;
		fontFirst = 0;
		fontLast = 0;
		fontX = 0;
		fontY = 0;
		bDefine = false;
	}
};

// Get the short/long name of a given compressor
const char* GetCompressorName(MSXi_Compressor comp, bool bShort = false);

// Get table format C text
std::string GetTableCText(TableFormat format, std::string name);

// Check if a compressor if compatible with given import parameters
bool IsCompressorCompatible(MSXi_Compressor comp, const ExportParameters& param);

/**
 * Exporter interface
 */
class ExporterInterface
{
protected:
	DataFormat eFormat;
	ExportParameters* Param;
	u32 TotalBytes;

public:
	ExporterInterface(DataFormat f, ExportParameters* p): eFormat(f), Param(p), TotalBytes(0) {}
	virtual void WriteHeader() = 0;
	virtual void WriteTableBegin(TableFormat format, std::string name, std::string comment) = 0;
	virtual void WriteSpriteHeader(i32 number) = 0;
	virtual void WriteCommentLine(std::string comment) = 0;
	virtual void Write1ByteLine(u8 a, std::string comment) = 0;
	virtual void Write2BytesLine(u8 a, u8 b, std::string comment) = 0;
	virtual void Write4BytesLine(u8 a, u8 b, u8 c, u8 d, std::string comment) = 0;
	virtual void Write1WordLine(u16 a, std::string comment) = 0;
	virtual void Write2WordsLine(u16 a, u16 b, std::string comment) = 0;
	virtual void WriteLineBegin() = 0;
	virtual void Write1ByteData(u8 data) = 0;
	virtual void Write8BitsData(u8 data) = 0;
	virtual void WriteLineEnd() = 0;
	virtual void WriteTableEnd(std::string comment) = 0;

	virtual const c8* GetNumberFormat(u8 bytes = 1) = 0;

	virtual u32 GetTotalBytes() { return TotalBytes; }
	virtual bool Export() = 0;
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
	virtual void WriteHeader() = 0;
	virtual void WriteTableBegin(TableFormat format, std::string name, std::string comment) = 0;
	virtual void WriteSpriteHeader(i32 number) = 0;
	virtual void WriteCommentLine(std::string comment) = 0;
	virtual void Write1ByteLine(u8 a, std::string comment) = 0;
	virtual void Write2BytesLine(u8 a, u8 b, std::string comment) = 0;
	virtual void Write4BytesLine(u8 a, u8 b, u8 c, u8 d, std::string comment) = 0;
	virtual void Write1WordLine(u16 a, std::string comment) = 0;
	virtual void Write2WordsLine(u16 a, u16 b, std::string comment) = 0;
	virtual void WriteLineBegin() = 0;
	virtual void Write1ByteData(u8 data) = 0;
	virtual void Write8BitsData(u8 data) = 0;
	virtual void WriteLineEnd() = 0;
	virtual void WriteTableEnd(std::string comment) = 0;

	virtual const c8* GetNumberFormat(u8 bytes = 1) = 0;

	virtual bool Export()
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
 * C langage exporter
 */
class ExporterC: public ExporterText
{
public:
	ExporterC(DataFormat f, ExportParameters* p): ExporterText(f, p) {}

	virtual const c8* GetNumberFormat(u8 bytes = 1)
	{
		switch(eFormat)
		{
		case DATA_Decimal:
			return "%3u";
		case DATA_Binary:
			return (bytes == 1) ? "0x%02X" : "0x%04X";
		case DATA_Hexa:
		case DATA_HexaC:
		case DATA_HexaAsm:
		case DATA_HexaDollar:
		case DATA_HexaSharp:
		default:
			return (bytes == 1) ? "0x%02X" : "0x%04X";
		}	
	}

	virtual void WriteHeader()
	{
		sprintf_s(strData, BUFFER_SIZE,
			"// Sprite table generated by MSXImage (v%s)\n"
			"// - Input file:     %s\n"
			"// - Start position: %i, %i\n"
			"// - Sprite size:    %i, %i (gap: %i, %i)\n"
			"// - Sprite count:   %i, %i\n"
			"// - Color count:    %i (Transparent: #%04X)\n"
			"// - Compressor:     %s\n"
			"// - Skip empty:     %s\n",
			MSXi_VERSION, Param->inFile, Param->posX, Param->posY, Param->sizeX, Param->sizeY, Param->gapX, Param->gapY, Param->numX, Param->numY,
			1 << Param->bpc, Param->transColor, GetCompressorName(Param->comp), Param->bSkipEmpty ? "TRUE" : "FALSE");
		outData += strData;
	}

	virtual void WriteTableBegin(TableFormat format, std::string name, std::string comment)
	{
		if (Param->bDefine)
		{
			sprintf_s(strData, BUFFER_SIZE,
				"\n"
				"#ifndef D_%s\n"
				"\t#define D_%s\n"
				"#endif\n"
				"// %s\n"
				"D_%s %s =\n"
				"{\n",
				name.c_str(), name.c_str(), comment.c_str(), name.c_str(), GetTableCText(format, name).c_str());
		}
		else
		{
			sprintf_s(strData, BUFFER_SIZE,
				"\n"
				"// %s\n"
				"%s =\n"
				"{\n",
				comment.c_str(), GetTableCText(format, name).c_str());
		}
		outData += strData;
	}

	virtual void WriteSpriteHeader(i32 number)
	{ 
		sprintf_s(strData, BUFFER_SIZE,
			"// Sprite[%i] (offset:%i)\n", number, TotalBytes);
		outData += strData;
	}

	virtual void WriteCommentLine(std::string comment)
	{
		sprintf_s(strData, BUFFER_SIZE,	"// %s\n", comment.c_str());
		outData += strData;
	}

	virtual void Write4BytesLine(u8 a, u8 b, u8 c, u8 d, std::string comment)
	{
		sprintf_s(strFormat, BUFFER_SIZE, 
			"\t%s, %s, %s, %s, // %s\n", GetNumberFormat(), GetNumberFormat(), GetNumberFormat(), GetNumberFormat(), comment.c_str());
		sprintf_s(strData, BUFFER_SIZE, strFormat, a, b, c, d);
		outData += strData;
		TotalBytes += 4;
	}

	virtual void Write2BytesLine(u8 a, u8 b, std::string comment)
	{
		sprintf_s(strFormat, BUFFER_SIZE, 
			"\t%s, %s, // %s\n", GetNumberFormat(), GetNumberFormat(), comment.c_str());
		sprintf_s(strData, BUFFER_SIZE, strFormat, a, b);
		outData += strData;
		TotalBytes += 2;
	}

	virtual void Write1ByteLine(u8 a, std::string comment)
	{
		sprintf_s(strFormat, BUFFER_SIZE, 
			"\t%s, // %s\n", GetNumberFormat(), comment.c_str());
		sprintf_s(strData, BUFFER_SIZE, strFormat, a);
		outData += strData;
		TotalBytes += 1;
	}

	virtual void Write1WordLine(u16 a, std::string comment)
	{ 
		sprintf_s(strFormat, BUFFER_SIZE,
			"\t%s, // %s\n", GetNumberFormat(2), comment.c_str());
		sprintf_s(strData, BUFFER_SIZE, strFormat, a);
		outData += strData;
		TotalBytes += 2;
	}

	virtual void Write2WordsLine(u16 a, u16 b, std::string comment)
	{
		sprintf_s(strFormat, BUFFER_SIZE,
			"\t%s, %s, // %s\n", GetNumberFormat(2), GetNumberFormat(2), comment.c_str());
		sprintf_s(strData, BUFFER_SIZE, strFormat, a, b);
		outData += strData;
		TotalBytes += 4;
	}

	virtual void WriteLineBegin()
	{ 
		outData += "\t";
	}

	virtual void Write1ByteData(u8 data)
	{
		sprintf_s(strFormat, BUFFER_SIZE, "%s, ", GetNumberFormat());
		sprintf_s(strData, BUFFER_SIZE, strFormat, data);
		outData += strData;
		TotalBytes += 1;
	}

	virtual void Write8BitsData(u8 data)
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
		TotalBytes += 1;
	}

	virtual void WriteLineEnd()
	{ 
		outData += "\n";
	}

	virtual void WriteTableEnd(std::string comment)
	{
		outData += "};\n";
		if (comment != "")
		{
			sprintf_s(strData, BUFFER_SIZE,
				"// %s\n", comment.c_str());
			outData += strData;
		}
	}
};


/**
 * Assembler langage exporter
 */
class ExporterASM: public ExporterText
{
public:
	ExporterASM(DataFormat f, ExportParameters* p): ExporterText(f, p) {}

	virtual const c8* GetNumberFormat(u8 bytes = 1)
	{
		switch(eFormat)
		{
		case DATA_Hexa:
		case DATA_HexaC:
			return (bytes == 1) ? "0x%02X" : "0x%04X";
		case DATA_HexaAsm:
			return (bytes == 1) ? "0%02Xh" : "0%04Xh";
		case DATA_HexaDollar:
			return (bytes == 1) ? "$%02X" : "$%04X";
		case DATA_HexaSharp:
			return (bytes == 1) ? "#%02X" : "#%02X";
		case DATA_Decimal:
		case DATA_Binary:
		default:
			return "%3u";
		}	
	}

	virtual void WriteHeader()
	{
		sprintf_s(strData, BUFFER_SIZE,
			"; Sprite table generated by MSXImage (v%s)\n"
			"; - Input file:     %s\n"
			"; - Start position: %i, %i\n"
			"; - Sprite size:    %i, %i (gap: %i, %i)\n"
			"; - Sprite count:   %i, %i\n"
			"; - Color count:    %i (Transparent: #%04X)\n"
			"; - Compressor:     %s\n"
			"; - Skip empty:     %s\n",
			MSXi_VERSION, Param->inFile, Param->posX, Param->posY, Param->sizeX, Param->sizeY, Param->gapX, Param->gapY, Param->numX, Param->numY, 
			1 << Param->bpc, Param->transColor,	GetCompressorName(Param->comp), Param->bSkipEmpty ? "TRUE" : "FALSE");
		outData += strData;
	}

	virtual void WriteTableBegin(TableFormat format, std::string name, std::string comment)
	{
		sprintf_s(strData, BUFFER_SIZE,
			"\n"
			"; %s\n"
			"%s:\n",
			comment.c_str(), name.c_str());
		outData += strData;
	}

	virtual void WriteSpriteHeader(i32 number)
	{ 
		sprintf_s(strData, BUFFER_SIZE, 
			"; Sprite[%i] (offset:%i)\n", number, TotalBytes);
		outData += strData;
	}

	virtual void WriteCommentLine(std::string comment)
	{
		sprintf_s(strData, BUFFER_SIZE, "; %s\n", comment.c_str());
		outData += strData;
	}

	virtual void Write4BytesLine(u8 a, u8 b, u8 c, u8 d, std::string comment)
	{
		sprintf_s(strFormat, BUFFER_SIZE, 
			"\t.db %s %s %s %s ; %s\n", GetNumberFormat(), GetNumberFormat(), GetNumberFormat(), GetNumberFormat(), comment.c_str());
		sprintf_s(strData, BUFFER_SIZE, strFormat, a, b, c, d);
		outData += strData;
		TotalBytes += 4;
	}

	virtual void Write2BytesLine(u8 a, u8 b, std::string comment)
	{
		sprintf_s(strFormat, BUFFER_SIZE, 
			"\t.db %s %s ; %s\n", GetNumberFormat(), GetNumberFormat(), comment.c_str());
		sprintf_s(strData, BUFFER_SIZE, strFormat, a, b);
		outData += strData;
		TotalBytes += 2;
	}

	virtual void Write1ByteLine(u8 a, std::string comment)
	{
		sprintf_s(strFormat, BUFFER_SIZE, 
			"\t.db %s ; %s\n", GetNumberFormat(), comment.c_str());
		sprintf_s(strData, BUFFER_SIZE, strFormat, a);
		outData += strData;
		TotalBytes += 1;
	}

	virtual void Write1WordLine(u16 a, std::string comment)
	{
		sprintf_s(strFormat, BUFFER_SIZE,
			"\t.dw %s ; %s\n", GetNumberFormat(2), comment.c_str());
		sprintf_s(strData, BUFFER_SIZE, strFormat, a);
		outData += strData;
		TotalBytes += 2;
	}

	virtual void Write2WordsLine(u16 a, u16 b, std::string comment)
	{
		sprintf_s(strFormat, BUFFER_SIZE,
			"\t.dw %s %s ; %s\n", GetNumberFormat(), GetNumberFormat(), comment.c_str());
		sprintf_s(strData, BUFFER_SIZE, strFormat, a, b);
		outData += strData;
		TotalBytes += 4;
	}

	virtual void WriteLineBegin()
	{ 
		outData += "\t.db ";
	}

	virtual void Write1ByteData(u8 data)
	{
		sprintf_s(strFormat, BUFFER_SIZE, "%s ", GetNumberFormat());
		sprintf_s(strData, BUFFER_SIZE, strFormat, data);
		outData += strData;
		TotalBytes += 1;
	}

	virtual void Write8BitsData(u8 data)
	{
		sprintf_s(strFormat, BUFFER_SIZE, "%s ", GetNumberFormat());
		sprintf_s(strData, BUFFER_SIZE, strFormat, data);
		outData += strData;
		TotalBytes += 1;
	}

	virtual void WriteLineEnd()
	{ 
		outData += "\n";
	}

	virtual void WriteTableEnd(std::string comment)
	{
		if (comment != "")
		{
			sprintf_s(strData, BUFFER_SIZE,
				"; %s\n", comment.c_str());
			outData += strData;
		}
	}
};

/**
 * Binary exporter
 */
class ExporterBin: public ExporterInterface
{
protected:
#define BUFFER_SIZE 1024
	std::vector<u8> outData;

public:
	ExporterBin(DataFormat f, ExportParameters* p) : ExporterInterface(f, p) {}
	virtual void WriteHeader() {}
	virtual void WriteTableBegin(TableFormat format, std::string name, std::string comment) {}
	virtual void WriteSpriteHeader(i32 number) {}
	virtual void WriteCommentLine(std::string comment) {}
	virtual void Write1ByteLine(u8 a, std::string comment)
	{ 
		outData.push_back(a); 
		TotalBytes += 1;
	}
	virtual void Write2BytesLine(u8 a, u8 b, std::string comment)
	{ 
		outData.push_back(a); 
		outData.push_back(b); 
		TotalBytes += 2;
	}
	virtual void Write4BytesLine(u8 a, u8 b, u8 c, u8 d, std::string comment)
	{ 
		outData.push_back(a); 
		outData.push_back(b); 
		outData.push_back(c); 
		outData.push_back(d); 
		TotalBytes += 4;
	}
	virtual void Write1WordLine(u16 a, std::string comment)
	{
		outData.push_back(a & 0x00FF);
		outData.push_back(a >> 8);
		TotalBytes += 2;
	}
	virtual void Write2WordsLine(u16 a, u16 b, std::string comment)
	{
		outData.push_back(a & 0x00FF);
		outData.push_back(a >> 8);
		outData.push_back(b & 0x00FF);
		outData.push_back(b >> 8);
		TotalBytes += 4;
	}
	virtual void WriteLineBegin() {}
	virtual void Write1ByteData(u8 data)
	{ 
		outData.push_back(data);
		TotalBytes += 1;
	}
	virtual void Write8BitsData(u8 data)
	{ 
		outData.push_back(data);
		TotalBytes += 1;
	}
	virtual void WriteLineEnd() {}
	virtual void WriteTableEnd(std::string comment) {}

	virtual const c8* GetNumberFormat(u8 bytes = 1) { return NULL; }

	virtual bool Export()
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


/**
 * Dummy exporter
 */
class ExporterDummy : public ExporterInterface
{
public:
	ExporterDummy(DataFormat f, ExportParameters* p) : ExporterInterface(f, p) {}
	virtual void WriteHeader() {}
	virtual void WriteTableBegin(TableFormat format, std::string name, std::string comment) {}
	virtual void WriteSpriteHeader(i32 number) {}
	virtual void WriteCommentLine(std::string comment) {}
	virtual void Write1ByteLine(u8 a, std::string comment) { TotalBytes += 1; }
	virtual void Write2BytesLine(u8 a, u8 b, std::string comment) { TotalBytes += 2; }
	virtual void Write4BytesLine(u8 a, u8 b, u8 c, u8 d, std::string comment) { TotalBytes += 4; }
	virtual void Write1WordLine(u16 a, std::string comment) { TotalBytes += 2; }
	virtual void Write2WordsLine(u16 a, u16 b, std::string comment) { TotalBytes += 4; }
	virtual void WriteLineBegin() {}
	virtual void Write1ByteData(u8 data) { TotalBytes += 1; }
	virtual void Write8BitsData(u8 data) { TotalBytes += 1;	}
	virtual void WriteLineEnd() {}
	virtual void WriteTableEnd(std::string comment) {}
	virtual const c8* GetNumberFormat(u8 bytes = 1) { return NULL; }
	virtual bool Export() { return true; }
};

