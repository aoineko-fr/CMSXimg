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

// CMSXi
#include "types.h"
#include "exporter.h"

//
bool ParseImage(ExportParameters* param, ExporterInterface* exp);

// Build 256 colors palette
void Create256ColorsPalette(const char* filename);

// Build 16 colors palette
void Create16ColorsPalette(const char* filename);