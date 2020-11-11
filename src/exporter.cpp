//     _____    _____________  ___ .___                               
//    /     \  /   _____/\   \/  / |   | _____ _____     ____   ____  
//   /  \ /  \ \_____  \  \     /  |   |/     \\__  \   / ___\_/ __ \ 
//  /    Y    \/        \ /     \  |   |  Y Y  \/ __ \_/ /_/  >  ___/ 
//  \____|__  /_______  //___/\  \ |___|__|_|  (____  /\___  / \___  >
//          \/        \/       \_/           \/     \//_____/      \/ 
//
// by Guillaume "Aoineko" Blanchard (aoineko@free.fr)
// under CC-BY-AS license (https://creativecommons.org/licenses/by-sa/2.0/)

#include "exporter.h"

const char* GetCompressorName(Compressor comp)
{
	switch (comp)
	{
	case COMPRESS_None:        return "None";
	case COMPRESS_Crop16:      return "Crop16 (4-bits, max 16x16)";
	case COMPRESS_CropLine16:  return "CropLine16 (4-bits per line, max 16x16)";
	case COMPRESS_Crop32:      return "Crop32 (5-bits, max 32x32)";
	case COMPRESS_CropLine32:  return "CropLine32 (5-bits per line, max 32x32)";
	case COMPRESS_Crop256:     return "Crop256 (8-bits, max 256x256)";
	case COMPRESS_CropLine256: return "CropLine256 (8-bits per line, max 256x256)";
	case COMPRESS_RLE0:        return "RLE0 (7-bits Run-length encoding for transparency)";
	case COMPRESS_RLE4:        return "RLE4 (4-bits Run-length encoding)";
	case COMPRESS_RLE8:        return "RLE8 (8-bits Run-length encoding)";
	}
	return "Unknow";
}