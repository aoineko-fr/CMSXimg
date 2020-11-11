//     _____    _____________  ___ .___                               
//    /     \  /   _____/\   \/  / |   | _____ _____     ____   ____  
//   /  \ /  \ \_____  \  \     /  |   |/     \\__  \   / ___\_/ __ \ 
//  /    Y    \/        \ /     \  |   |  Y Y  \/ __ \_/ /_/  >  ___/ 
//  \____|__  /_______  //___/\  \ |___|__|_|  (____  /\___  / \___  >
//          \/        \/       \_/           \/     \//_____/      \/ 
//
// by Guillaume "Aoineko" Blanchard (aoineko@free.fr)
// under CC-BY-AS license (https://creativecommons.org/licenses/by-sa/2.0/)

#include "color.h"

u32 PaletteMSX[16] = { 0x000000, 0x000000, 0x3EB849, 0x74D07D, 0x5955E0, 0x8076F1, 0xB95E51, 0x65DBEF, 0xDB6559, 0xFF897D, 0xCCC35E, 0xDED087, 0x3AA241, 0xB766B5, 0xCCCCCC, 0xFFFFFF };

// Convert RGB24 to GRB8
GRB8::GRB8(RGB24 color)
{
	i32 r, g, b;
	
	r = color.R * 7 / 255;
	g = color.G * 7 / 255;
	b = color.B * 3 / 255;

	RGB = u8((g << 5) + (r << 2) + b);
}

// Convert GRB8 to RGB24
RGB24::RGB24(GRB8 color)
{
	i32 r, g, b;

	r = color / 32;
	g = (color & 0x1C) / 4;
	b = color & 0x03;

	R = u8(r * 255 / 7);
	G = u8(g * 255 / 7);
	B = u8(b * 255 / 3);
}