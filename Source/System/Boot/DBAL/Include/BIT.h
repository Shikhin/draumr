/* 
 * General BIT related definitions and structures.
 *
 * Copyright (c) 2012, Shikhin Sethi
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Draumr nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL SHIKHIN SETHI BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * - INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION - HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _BIT_H
#define _BIT_H

#include <Standard.h>
#include <Output.h>
#include <BootFiles.h>

// Hardware flags.
#define A20_DISABLED    (1 << 0)

// Video flags.
#define VGA_PRESENT     (1 << 0)
#define VBE_PRESENT     (1 << 1)
#define EDID_PRESENT    (1 << 2)

// Boot device flags.
#define BD_CD           0
#define BD_FLOPPY       1
#define BD_PXE          2

// The API codes for the file access API.
#define FILE_OPEN   0
#define FILE_READ   1
#define FILE_CLOSE  2

// The API codes for the video handling API.
#define VIDEO_VGA_SWITCH_MODE   0
#define VIDEO_VGA_PALETTE       1

#define VIDEO_VBE_SWITCH_MODE   10
#define VIDEO_VBE_PALETTE       11
#define VIDEO_VBE_GET_MODES     12

// The BIT structure.
struct BIT
{
    // The open, read and close file functions.
    uint32_t FileAPI;

    uint8_t HrdwreFlags;             // The "hardware" flags.
    uint8_t BDFlags;                 // The boot device flags.

    uint32_t ACPI;                   // The 32-bit address of the RSDP.
    uint32_t MPS;                    // The 32-bit address of the MPS tables.
    uint32_t SMBIOS;                 // The 32-bit address of the SMBIOS tables.

    uint32_t AbortBootServices;      // Abort all boot services given by DBAL + firmware.

    uint32_t MMap;                    // The 32-bit address of the Memory Map.  
    uint64_t HighestAddress;          // Highest accessible memory address.

    // Define the Video related things here.
    struct
    {
        uint8_t VideoFlags;                      // The video flags.
        uint32_t VBECntrlrInfo;            // The 32-bit adddress of the VBE Controller Mode Info block.
        uint32_t VBEModeInfo;              // The 32-bit address of the (allocated) VBE mode info block.
        uint32_t VBEModeInfoN;                   // The number of entries.

        EDIDInfo_t EDIDInfo;                            // The EDID information.

        uint32_t VideoAPI;                              // The video API function.
        uint32_t OutputRevert;                          // The function to revert back from the current level.

        VBEModeInfo_t ModeInfo;        // The mode we switched to's information.

        FILE_t BackgroundImg;                      // Pointer to the boot image.
        uint32_t AbortBoot;                        // The abort boot function - provided by the DBAL.
    }_PACKED Video;

    // Define the DBAL PMM related things here.
    struct
    {
        // AllocFrame, FreeFrame, AllocContigFrames, FreeContigFrames.
        uint32_t AllocFrame;
        uint32_t FreeFrame;
        uint32_t AllocContigFrames;
        uint32_t FreeContigFrames;
    }_PACKED DBALPMM;

    // The architecture of the kernel.
    uint32_t Arch;
}_PACKED;

typedef uint32_t (*FileAPIFunc_t)(uint32_t, ...);
typedef uint32_t (*VideoAPIFunc_t)(uint32_t, ...);
typedef void     (*EmptyFunc_t)(void);

typedef struct BIT BIT_t;

// Declare all the functions.
FileAPIFunc_t FileAPIFunc;
VideoAPIFunc_t VideoAPIFunc;

// The BIT structure defined in BIT.c - where we back this up.
extern BIT_t BIT;

/*
 * Initializes the BIT structure, copying it to somewhere appropriate.
 *     uint32_t *BITPointer -> the pointer to the BIT structure, as passed to us.
 */
void BITInit(uint32_t *BITPointer) _COLD;

#endif /* _BIT_H */
