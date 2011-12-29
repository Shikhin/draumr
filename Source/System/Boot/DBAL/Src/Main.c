/* Entry point for DBAL file.
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
#include <BIT.h>
#include <PMM.h>
#include <Log.h>
#include <FPU.h>
#include <BootFiles.h>
#include <Output.h>
#include <OutputMod/OutputMod.h>

int Main(uint32_t *BITPointer)
{
    // Initialize the FPU, without which, we can't proceed.
    FPUInit();
    
    // Initialize the BIT - especially copy it to our side.
    BITInit(BITPointer);
    
    // Initialize the PMM.
    PMMInit();
    
    // Initialize the bouncer for the boot files.
    InitBootFiles();
        
    // Initialize 'output' thingy.
    OutputInit(); 
    
    // Just a temporary thingy to init the OutputMod.
    OutputModInit();

    for(;;)
        __asm__ __volatile__("hlt");
}
