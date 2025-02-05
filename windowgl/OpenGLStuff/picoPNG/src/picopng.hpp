//-----------------------------LICENSE NOTICE------------------------------------
// picoPNG: UA GameDev Version 20191112, with minor modifications from original version 20101224
//  Copyright (c) 2005-2010 Lode Vandevenne
//  Copyright (C) 2019 Francisco J. Gallego-Durán (fjgallego@ua.es @FranGallegoBR)
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
//     1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required.
//     2. Altered source versions must be plainly marked as such, and must not be
//     misrepresented as being the original software.
//     3. This notice may not be removed or altered from any source distribution.
//------------------------------------------------------------------------------

#pragma once
#include <vector>

int decodePNG(std::vector<unsigned char>& out_image, unsigned long& image_width, 
               unsigned long& image_height, const unsigned char* in_png, 
               std::size_t in_size, bool convert_to_rgba32 = true);