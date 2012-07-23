/*
 * Contains VMM related definitions for KL.
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
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _VMM_H
#define _VMM_H

#include <Standard.h>
#include <BIT.h>

#ifdef VMMX86_PAGING

// Typedef page directory and page table to uint32_t.
typedef uint32_t PageDirEntry_t;
typedef uint32_t PageTableEntry_t;

// Define the mask for the index for the page directory, and the index for the page table.
#define PD_INDEX(a) ((a & 0xFFC00000) >> 22)
#define PT_INDEX(a) ((a & 0x003FF000) >> 12)

#endif /* VMMX86_PAGING */

#ifdef VMMPAE_PAGING

// Typedef page directory and page table to uint64_t.
typedef uint64_t PageDirPTEntry_t;
typedef uint64_t PageDirEntry_t;
typedef uint64_t PageTableEntry_t;

// Define the mask for the index for the page directory pointer table, the page dir, the page table.
#define PDPT_INDEX(a) ((a & 0xC0000000) >> 30)
#define PD_INDEX(a)   ((a & 0x3FE00000) >> 21)
#define PT_INDEX(a)   ((a & 0x001FF000) >> 12)

#endif /* VMMPAE_PAGING */

#ifdef VMMAMD64_PAGING

// Typedef page directory and page table to uint64_t.
typedef uint64_t PML4Entry_t;
typedef uint64_t PageDirPTEntry_t;
typedef uint64_t PageDirEntry_t;
typedef uint64_t PageTableEntry_t;

// Define the mask for the index for the relative indexes.
#define PML4_INDEX(a) ((a & 0xFF8000000000) >> 39)
#define PDPT_INDEX(a) ((a & 0x7FC0000000) >> 30)
#define PD_INDEX(a)   ((a & 0x3FE00000) >> 21)
#define PT_INDEX(a)   ((a & 0x001FF000) >> 12)

#endif /* VMMAMD64_PAGING */

#define PRESENT_BIT (1 << 0)

#define PAGE_MASK   (~0xFFF)

/*
 * Initializes x86 paging.
 */
_PROTOTYPE(void x86PagingInit, (void)) _COLD;

/*
 * Maps a page (x86).
 *     uint64_t VirtAddr -> the virtual address where to map the frame to.
 *     uint64_t PhysAddr -> the physical address of the frame to map to the page.
 */
_PROTOTYPE(void x86PagingMap, (uint64_t VirtAddr, uint64_t PhysAddr));

/*
 * Initializes PAE paging.
 */
_PROTOTYPE(void PAEPagingInit, (void)) _COLD;

/*
 * Maps a page (PAE).
 *     uint64_t VirtAddr -> the virtual address where to map the frame to.
 *     uint64_t PhysAddr -> the physical address of the frame to map to the page.
 */
_PROTOTYPE(void PAEPagingMap, (uint64_t VirtAddr, uint64_t PhysAddr));

/*
 * Initializes AMD64 paging.
 */
_PROTOTYPE(void AMD64PagingInit, (void)) _COLD;

/*
 * Maps a page (AMD64).
 *     uint64_t VirtAddr -> the virtual address where to map the frame to.
 *     uint64_t PhysAddr -> the physical address of the frame to map to the page.
 */
_PROTOTYPE(void AMD64PagingMap, (uint64_t VirtAddr, uint64_t PhysAddr));

#endif /* _VMM_H */