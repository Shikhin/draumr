/* Contains common Video definitions.
* 
*  Copyright (c) 2011 Shikhin Sethi
* 
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation;  either version 3 of the License, or
*  (at your option) any later version.
* 
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*  GNU General Public License for more details.
* 
*  You should have received a copy of the GNU General Public License along
*  with this program; if not, write to the Free Software Foundation, Inc.,
*  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <stdint.h>
#include <String.h>
#include <Video.h>
#include <BIT.h>
#include <PMM.h>
#include <Log.h>

// Switches to a video mode.
// uint32_t X                         The X resolution.
// uint32_t Y                         The Y resolution.
// uint32_t BPP                       Bits per pixel.
static void SwitchToMode(uint32_t X, uint32_t Y, uint32_t BPP)
{
    // If the mode is 640 * 480 * 16 colors, then switch to mode 0x12 defined by VGA.
    if((X == 640) &&
       (Y == 480) &&
       (BPP == 4))
    {
        BIT.Video.SwitchVGA(0x12);
        // Set the bit mask - 0xFF - we except everything!
	outb(0x03CE, 0x08);
	outb(0x03CF, 0xFF);
	
        // Set the address register to a) The Write Map Select Register b) The Read Map Select Register.
        outb(0x03C4, 0x02);
        outb(0x03CE, 0x04);
    }
    
    // If the mode is 320 * 200 * 256 colors, then switch to mode 0x13 defined by VGA.
    else if((X == 320) &&
            (Y == 200) &&
            (BPP == 8))
    {
        BIT.Video.SwitchVGA(0x13); 
       
	// Setup the palette to a RGB thingy.
	BIT.Video.SetupPaletteVGA();
    }
}

// Intializes a proper video mode, which is supported by the OS, the video card and the monitor (and is beautiful).
void VideoInit()
{
    SwitchToMode(320, 200, 8);
    
    // Fill in some general details of the video mode.
    BIT.Video.Address = (uint32_t*)0xA0000;
    BIT.Video.XRes = 320;
    BIT.Video.YRes = 200;
    BIT.Video.BPP = 8;
    BIT.Video.BytesBetweenLines = 0;
}