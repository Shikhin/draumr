 ; Entry point for CD Bootloader Stage 1
 ;
 ; Copyright (c) 2012, Shikhin Sethi
 ; All rights reserved.
 ;
 ; Redistribution and use in source and binary forms, with or without
 ; modification, are permitted provided that the following conditions are met:
 ;     * Redistributions of source code must retain the above copyright
 ;       notice, this list of conditions and the following disclaimer.
 ;     * Redistributions in binary form must reproduce the above copyright
 ;       notice, this list of conditions and the following disclaimer in the
 ;       documentation and/or other materials provided with the distribution.
 ;     * Neither the name of Draumr nor the
 ;       names of its contributors may be used to endorse or promote products
 ;       derived from this software without specific prior written permission.
 ;
 ; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ; ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 ; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 ; DISCLAIMED. IN NO EVENT SHALL SHIKHIN SETHI BE LIABLE FOR ANY
 ; DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 ; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 ; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 ; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 ; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

BITS 16
CPU 386

GLOBAL Startup

SECTION .data

%define BD_CD        0
%define BD_FLOPPY    1
%define BD_PXE       2

; End (of) line.
%define EL           0x0A, 0x0D

; The error slogan! (hehe)
ErrorMsg:
    db "ERROR! ERROR! ERROR!", EL, EL, 0

; Abort boot if can't open CBIOS file.
ErrorOpenCBIOSMsg:
    db "Unable to open the common BIOS file.", EL, 0

; Or file is incorrect.
ErrorCBIOSHeaderMsg:
    db "The common BIOS's header has been found to be corrupt.", EL, 0

; Or the CRC value is incorrect.
ErrorCBIOSCRCMsg:
    db "Incorrect CRC32 value of the common BIOS file.", EL, 0

SECTION .base

 ; Entry point where the BIOS hands control.
 ;     DL    -> Expects the drive number to be present in dl.
 ;     CS:IP -> Expects CS:IP to point to the linear address 0x7C00.
Startup:
    jmp 0x0000:Main                   ; Some BIOS' load the bootloader at 0x0000:0x7C00, while others
                                      ; load it at 0x07C0:0x0000. Do a far jump to reload this value
                                      ; to a standard 0x0000:0xIP.

SECTION .text
%include "Source/System/Boot/BIOS/Floppy/Src/Abort.asm"
%include "Source/System/Boot/BIOS/Floppy/Src/Screen.asm"
%include "Source/System/Boot/BIOS/Floppy/Src/Disk/Disk.asm"
%include "Source/Lib/CRC32/CRC32.asm"

SECTION .base
Main:
    xor ax, ax                        ; Set all the segment registers to 0x0000.

    mov ss, ax
    mov sp, Startup                   ; Set the stack to start from Startup (0x7C00) and continue below.

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    mov [BootDrive], dl               ; Save @dl which contains the Boot Drive number for future references.
   
    ; Set to mode 0x03, or 80*25 text mode.
    mov ax, 0x03
   
    ; SWITCH! 
    int 0x10

    call BootFileGet                  ; Get the complete boot file (us).
    jmp ExtMain

; Pad out the remaining bytes in the first 512 bytes, and then define the boot signature.
    times 510-($-$$) db 0

BIOSSignature:
    dw 0xAA55

BigVal1: dd 0xFFFFFFFF, 0x00000000
BigVal2: dd 0xFFFFFFFE, 0xE0000000
BigVal3: dd 0x00000000, 0x00000000

ExtMain:
    call ScreenInit                   ; Initialize the entire screen to blue, and disable the hardware cursor.		
    call BootFilesInit                ; Initialize boot file data - get the size currently.

.LoadCommonBIOS:
    xor ax, ax                        ; Open File 0, or common BIOS file.

    call FileOpen                     ; Open the File.
    jc .ErrorOpenCBIOS
    
    ; ECX contains size of file we are opening.
    push ecx

    mov ecx, 0x200                    ; Read only 0x200 bytes.
    mov edi, 0x9000
    
    call FileRead                     ; Read the entire file.

; Does all checks related to the first sector of the common BIOS file.
.CheckCBIOS1:
    cmp dword [0x9000], "BIOS"        ; Check the signature.
    jne .ErrorCBIOSHeader

    cmp dword [0x9008], 0x9000        ; If the starting of the file isn't 0x9000, abort.
    jne .ErrorCBIOSHeader

    mov ecx, [0x9000 + 12]            ; Get the end of the file in ECX.
    sub ecx, 0x9000                   ; Subtract 0x9000 from it to get it's size.
    add ecx, 0x1FF
    shr ecx, 9                        ; Here we have the number of sectors of the file (according to the header).
  
    mov edx, [esp]                    ; Get the saved ECX.

    add edx, 0x1FF
    shr edx, 9                        ; Here we have the number of sectors of the file (according to the fs).

    cmp ecx, edx
    jne .ErrorCBIOSHeader             ; If not equal, error.

.LoadRestFile:
    add edi, 0x200
    pop ecx
    
    cmp ecx, 0x200
    jbe .Finish

    sub ecx, 0x200                    ; Read the rest 0x200 bytes.
    
    call FileRead                     ; Read the rest of the file.

.Finish:
    call FileClose                    ; And then close the file.

.CheckCommonBIOS2:
    mov ecx, [0x9000 + 12]            ; Get the end of the file in ECX.
    mov esi, 0x9000 + 28              ; Calculate CRC from above byte 28.
     
    sub ecx, esi                      ; Subtract 0x9000 (address of start) + 28 (size of header) from it, to get the size.   
    mov eax, 0xFFFFFFFF               ; Put the seed in EAX.
    
    call CRC32
    
    not eax                           ; Inverse the bits to get the CRC value.
    cmp eax, [esi - 4]                ; Compare the has with the hash stored in the file.
    jne .ErrorCBIOSCRC                ; Not equal? ERROR: Abort boot.

.ZeroBSS:
    mov esi, 0x9000 
    movzx edi, word [esi + 16]        ; Move the start of BSS section into EDI.
   
    movzx ecx, word [esi + 20]
    sub ecx, edi                      ; Calculate the length, and store it in ECX.
    shr ecx, 2                        ; Shift the length right by 2, effectively dividing by 4.
    
    xor eax, eax                      ; Zero out EAX, since we want to clear the region.
    rep stosd                         ; Clear out the BSS section.
 
.JmpToBIOS:
    mov eax, FileOpen
    mov ebx, FileRead
    mov ecx, FileClose
    mov edx, BD_FLOPPY
       
    mov esp, 0x7C00
    xor ebp, ebp

    jmp [0x9004]

.ErrorOpenCBIOS:
    mov si, ErrorOpenCBIOSMsg
    jmp AbortBoot

.ErrorCBIOSHeader:
    mov si, ErrorCBIOSHeaderMsg
    jmp AbortBoot

.ErrorCBIOSCRC:
    mov si, ErrorCBIOSCRCMsg
    jmp AbortBoot

SECTION .pad
; Define DRAUMRSS - so that it can be used to check the sanity of the file.
db "DRAUMRSS"                         ; Define the boot signature - DRAUMRSS.
