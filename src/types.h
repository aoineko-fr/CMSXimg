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

#define VERSION "1.7.1"

//-----------------------------------------------------------------------------
// integer types
typedef char               i8;  ///<  8 bits integer
typedef short              i16; ///< 16 bits integer
typedef long               i32; ///< 32 bits integer
typedef _int64             i64; ///< 64 bits integer

//-----------------------------------------------------------------------------
// unsigned integer types
typedef unsigned char      u8;  ///<  8 bits unsigned integer
typedef unsigned short     u16; ///< 16 bits unsigned integer
typedef unsigned long      u32; ///< 32 bits unsigned integer
typedef unsigned _int64    u64; ///< 64 bits unsigned integer

//-----------------------------------------------------------------------------
// signed integer types
typedef signed char        s8;	///<  8 bits signed integer
typedef signed short       s16;	///< 16 bits signed integer
typedef signed long        s32;	///< 32 bits signed integer
typedef signed _int64      s64;	///< 64 bits signed integer

//-----------------------------------------------------------------------------
// float types
typedef float              f32; ///< 32 bits floating-point
typedef double             f64; ///< 64 bits floating-point

//-----------------------------------------------------------------------------
// character types
typedef char               c8;	///<  8 bits character
typedef wchar_t            c16;	///< 16 bits character

#define numberof(tab) sizeof(tab) / sizeof(tab[0])
