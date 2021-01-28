﻿//_____________________________________________________________________________
//   ▄▄   ▄ ▄  ▄▄▄ ▄▄ ▄ ▄                                                      
//  ██ ▀ ██▀█ ▀█▄  ▀█▄▀ ▄  ▄█▄█ ▄▀██                                           
//  ▀█▄▀ ██ █ ▄▄█▀ ██ █ ██ ██ █  ▀██                                           
//_______________________________▀▀____________________________________________
//
// by Guillaume "Aoineko" Blanchard (aoineko@free.fr)
// available on GitHub (https://github.com/aoineko-fr/CMSXimg)
// under CC-BY-AS license (https://creativecommons.org/licenses/by-sa/2.0/)

#pragma once

// std
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <ctime>
// CMSXi
#include "CMSXi.h"
#include "types.h"
#include "color.h"
// CMSXi
#include "CMSXtk.h"

#define BUFFER_SIZE 1024

/// Format of the data
enum TableFormat
{
	TABLE_U8,					///< 8-bits unsigned interger
	TABLE_U16,					///< 16-bits unsigned interger
	TABLE_U32,					///< 32-bits unsigned interger
	TABLE_Header,				///< Header structure (@see CMSXi_Header)
};

/// Exporter parameters
struct ExportParameters
{
	std::string inFile;			///< Input filename
	std::string outFile;		///< Output filename
	std::string tabName;		///< Data table name
	i32 posX;					///< Start X position for data extracting
	i32 posY;					///< Start Y position for data extracting
	i32 sizeX;					///< Width of data block
	i32 sizeY;					///< Height of data block
	i32 gapX;					///< X gap between blocks
	i32 gapY;					///< Y gap between blocks
	i32 numX;					///< Number of columns of block to extract
	i32 numY;					///< Number of row of block to extract
	i32 bpc;					///< Bits Per Color (can be 1, 2, 4 or 8-bits)
	bool bUseTrans;				///< Use transparency color
	u32 transColor;				///< Transparency color (24-bits RGB)
	PaletteType palType;		///< Palette type (@see PaletteType)
	i32 palCount;				///< Number of colors in the palette
	CMSXi_Compressor comp;		///< Compressor to use (@see CMSXi_Compressor)
	CMSXtk_DataFormat format;	///< Data format to use for text export (@see CMSXtk_DataFormat)
	bool bSkipEmpty;			///< Skip empty block (be aware this option change the block index)
	DitheringMethod dither;		///< The dithering method to use (only for 1-bit BPC)
	bool bAddCopy;				///< Add copyright information from file
	std::string copyFile;		///< Copyright filename
	bool bAddHeader;			///< Add export header table
	bool bAddIndex;				///< Add index table (should be necessary if bSkipEmpty is True)
	bool bAddFont;				///< Add font header information data (@see CMSXi_Font)
	c8 fontFirst;				///< First character ASCII code
	c8 fontLast;				///< Last character ASCII code
	u8 fontX;					///< Font width (can be equal or greater than sizeX)
	u8 fontY;					///< Font height (can be equal or greater than sizeY)
	bool bDefine;				///< Add define block for C file that allow to add directive to table definition (to place data at a given address for e.g.)
	bool bTitle;				///< Display ASCII-art title on top of exported text file

	ExportParameters()
	{
		inFile = "";
		outFile = "";
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
		palCount = -1;
		comp = COMPRESS_None;
		format = DATA_Hexa;
		bSkipEmpty = false;
		dither = DITHER_None;
		bAddCopy = false;
		copyFile = "";
		bAddHeader = false;
		bAddIndex = false;
		bAddFont = false;
		fontFirst = 0;
		fontLast = 0;
		fontX = 0;
		fontY = 0;
		bDefine = false;
		bTitle = true;
	}
};

// Get the short/long name of a given compressor
const char* GetCompressorName(CMSXi_Compressor comp, bool bShort = false);

// Get table format C text
std::string GetTableCText(TableFormat format, std::string name);

// Check if a compressor if compatible with given import parameters
bool IsCompressorCompatible(CMSXi_Compressor comp, const ExportParameters& param);

/**
 * Exporter interface
 */
class ExporterInterface
{
protected:
	CMSXtk_DataFormat eFormat;
	ExportParameters* Param;
	u32 TotalBytes;

public:
	ExporterInterface(CMSXtk_DataFormat f, ExportParameters* p): eFormat(f), Param(p), TotalBytes(0) {}
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
	char strFormat[BUFFER_SIZE];
	char strData[BUFFER_SIZE];
	std::string outData;

public:
	ExporterText(CMSXtk_DataFormat f, ExportParameters* p) : ExporterInterface(f, p) {}
	virtual void WriteHeader()
	{
		// Add title
		if (Param->bTitle)
		{
			WriteCommentLine(u8"_____________________________________________________________________________");
			WriteCommentLine(u8"   ▄▄   ▄ ▄  ▄▄▄ ▄▄ ▄ ▄");
			WriteCommentLine(u8"  ██ ▀ ██▀█ ▀█▄  ▀█▄▀ ▄  ▄█▄█ ▄▀██");
			WriteCommentLine(u8"  ▀█▄▀ ██ █ ▄▄█▀ ██ █ ██ ██ █  ▀██");
			WriteCommentLine(u8"_______________________________▀▀____________________________________________");
		}

		// Add copyright information
		if (Param->bAddCopy)
		{
			std::ifstream file(Param->copyFile);
			std::string strLine;
			while (std::getline(file, strLine))
			{
				WriteCommentLine(strLine);
			}
			file.close();
			WriteCommentLine(u8"_____________________________________________________________________________");
		}

		// Add version & date
		std::time_t result = std::time(nullptr);
		char* ltime = std::asctime(std::localtime(&result));
		ltime[strlen(ltime) - 1] = 0; // remove final '\n'
		sprintf_s(strData, BUFFER_SIZE, "Data generated using CMSXimg %s on %s", CMSXi_VERSION, ltime);
		WriteCommentLine(strData);

		// Add author & license
		WriteCommentLine("by Guillaume \"Aoineko\" Blanchard (2021) under CC BY-SA free license");

		// Add generation parameters
		WriteCommentLine("Generation parameters:");
		sprintf_s(strData, BUFFER_SIZE, " - Input file:     %s", Param->inFile.c_str());
		WriteCommentLine(strData);
		sprintf_s(strData, BUFFER_SIZE, " - Start position: %i, %i", Param->posX, Param->posY);
		WriteCommentLine(strData);
		sprintf_s(strData, BUFFER_SIZE, " - Sprite size:    %i, %i (gap: %i, %i)", Param->sizeX, Param->sizeY, Param->gapX, Param->gapY);
		WriteCommentLine(strData);
		sprintf_s(strData, BUFFER_SIZE, " - Sprite count:   %i, %i", Param->numX, Param->numY);
		WriteCommentLine(strData);
		sprintf_s(strData, BUFFER_SIZE, " - Color count:    %i (Transparent: #%04X)", 1 << Param->bpc, Param->transColor);
		WriteCommentLine(strData);
		sprintf_s(strData, BUFFER_SIZE, " - Compressor:     %s", GetCompressorName(Param->comp));
		WriteCommentLine(strData);
		sprintf_s(strData, BUFFER_SIZE, " - Skip empty:     %s", Param->bSkipEmpty ? "TRUE" : "FALSE");
		WriteCommentLine(strData);
	}
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

	virtual const c8* GetNumberFormat(u8 bytes = 1)
	{
		return CMSXtk_GetDataFormat(eFormat, bytes);
	}

	virtual bool Export()
	{
		// Write header file
		FILE* file;
		if (fopen_s(&file, Param->outFile.c_str(), "wb") != 0)
		{
			printf("Error: Fail to create %s\n", Param->outFile.c_str());
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
	ExporterC(CMSXtk_DataFormat f, ExportParameters* p): ExporterText(f, p) {}

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
	ExporterASM(CMSXtk_DataFormat f, ExportParameters* p): ExporterText(f, p) {}

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
	ExporterBin(CMSXtk_DataFormat f, ExportParameters* p) : ExporterInterface(f, p) {}
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
		if (fopen_s(&file, Param->outFile.c_str(), "wb") != 0)
		{
			printf("Error: Fail to create %s\n", Param->outFile.c_str());
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
	ExporterDummy(CMSXtk_DataFormat f, ExportParameters* p) : ExporterInterface(f, p) {}
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

