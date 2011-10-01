/* Contains common PMM definitions.
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
#include <PMM.h>
#include <BIT.h>

// Define the pointers to the headers and entries.
MMapHeader_t *MMapHeader;
MMapEntry_t  *MMapEntries;

// PMMFixMMap fixes the memory map - only overlapping entries.
static void PMMFixMMap()
{
    // Loop till all the entries.
    for(uint32_t i = 0; i < MMapHeader->Entries; i++)
    {
        // And fix all the shit here.
    }
}

// Initializes the physical memory manager for ourselves.
void PMMInit()
{
    // Get the addresses into the right variables.
    MMapHeader = (MMapHeader_t*)BIT.MMap;
    MMapEntries = (MMapEntry_t*)MMapHeader->Address;
    
    PMMFixMMap();                     // Fix overlapping entries.
}
