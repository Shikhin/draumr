 ; Common definitions to get the Video Mode information list.
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

SECTION .bss
; Reserve space for the palette AND the VBE controller information, since we only do it one at a time - where we store it.
Palette:
ControllerInfo:
    resb (4 * 256)

SECTION .data

PaletteLookup3BITS    db 0,  9, 18, 27, 36, 45, 54, 63
PaletteLookup2BITS    db 0, 21, 42, 63

SECTION .text
BITS 16

 ; Performs a switch to a VGA mode, using the BIOS.
 ;     AX -> the mode to switch to.
VGASwitchMode:
    pushad

    int 0x10                          ; Since the mode is in AX, AH should be cleared. So, switch!

.Return:
    popad
    ret

 ; Performs a switch to a VBE mode, using the VBE.
 ;
 ; Returns:
 ;     AX -> the status return code.
VBESwitchMode:
    mov bx, ax
    mov ax, 0x4F02
    
    int 0x10
   
.Return:
    ret

 ; Set ups the palette for a 8bpp mode, using the VBE.
VBESetupPalette:
    pushad
    
    ; Clear the counter, and point edi at the output buffer.
    mov edi, Palette
    xor edx, edx

.PaletteLoop:
    ; Move the counter in EAX, EBX, ECX.
    mov eax, edx
    mov ebx, edx
    mov ecx, edx

    ; Get the red, green and blue bits in EAX, EBX, ECX.
    shr al, 5
    shr bl, 2
    and cl, 0x03
    and bl, 0x07

    ; Get the values from the lookup tables.
    mov al, [PaletteLookup3BITS + eax]
    mov ah, [PaletteLookup3BITS + ebx]
    mov bl, [PaletteLookup2BITS + ecx]
    
    ; And store them.
    mov [di], bl
    mov [di + 1], ax
       
    add di, 4
    add dl, 0x01
    jnc .PaletteLoop

    ; Start from register 0, set 256 registers, from the table 'Palette', using AX = 0x4F09.
    mov ax, 0x4F09
    xor dx, dx
    xor bl, bl
    mov cx, 256
    mov di, Palette

    int 0x10
    
    ; If successful, return.
    cmp ax, 0x4F
    je .Return
    
    ; Else, use VGA.
    ; Start from register 0, set 256 registers, from the table 'Palette', using AX = 0x1012.
    xor bx, bx
    mov ax, 0x1012
    mov cx, 256
    mov dx, Palette

    int 0x10
    
.Return:
    popad
    ret

 ; Set ups the palette for a 8bpp mode, using the BIOS.
VGASetupPalette:
    pushad

    ; Clear the counter, and point edi at the output buffer.
    mov edi, Palette
    xor edx, edx

.PaletteLoop:
    ; Move the counter in EAX, EBX, ECX.
    mov eax, edx
    mov ebx, edx
    mov ecx, edx

    ; Get the red, green and blue bits in EAX, EBX, ECX.
    shr al, 5
    shr bl, 2
    and cl, 0x03
    and bl, 0x07

    ; Get the values from the lookup tables.
    mov al, [PaletteLookup3BITS + eax]
    mov ah, [PaletteLookup3BITS + ebx]
    mov bl, [PaletteLookup2BITS + ecx]
    
    ; And store them.
    mov [di], ax
    mov [di + 2], bl
    
    add di, 3
    add dl, 0x01
    jnc .PaletteLoop

    ; Start from register 0, set 256 registers, from the table 'Palette', using AX = 0x1012.
    xor bx, bx
    mov ax, 0x1012
    mov cx, 256
    mov dx, Palette

    int 0x10

.Return:
    popad
    ret

 ; Checks for VGA, VBE, and if VBE supported, also gets the VBE information.
VideoInit:
    pushad

; Check for EDID support.
.EDIDCheck:
    ; AX=0x4F15, bl=0x00, int 0x10 is the installation check for EDID. If this fails,
    ; then move on the VGA check.
    mov ax, 0x4F15
    xor bl, bl
    int 0x10

    ; If function isn't supported, al wouldn't be 0x4F.
    cmp al, 0x4F
    jne .VGACheck

    ; Gives the status.
    test ah, ah
    jnz .VGACheck

; So EDID is supported - get the EDID information.
.EDIDGet:
    ; AX=0x4F15, bl=0x01, cx=0x0000, dx=0x0000, di=destination buffer, int 0x10 gives
    ; the EDID information block in the destination buffer.
    mov ax, 0x4F15
    mov di, BIT.EDIDInfo
    xor cx, cx
    xor dx, dx
    xor bl, bl
    inc bl
    int 0x10

    ; If function isn't supported, al wouldn't be 0x4F.
    cmp al, 0x4F
    jne .VGACheck

    ; Gives the status.
    test ah, ah
    jnz .VGACheck
    
    ; So EDID is present, check the flag.
    or byte [BIT.VideoFlags], EDID_PRESENT
    
; Check for VGA support.
.VGACheck:
    ; AX=0x1A00, int 0x10 returns the display combination code.
    mov ax, 0x1A00

    ; Quoting Ralf Brown's interrupt list:
    ; Notes: This function is commonly used to check for the presence of a VGA. 
    ; This function is supported on the ATI EGA Wonder with certain undocumented configuration switch settings,
    ; even though the EGA Wonder does not support VGA graphics; 
    ; to distinguish this case, call AX=1C00h with CX nonzero, which the EGA Wonder does not support.

    xor cx, cx
    not cx

    int 0x10

    ; If AL isn't 0x1A, it failed. Go the VBE check.
    cmp al, 0x1A
    jne .VBECheck
    
    ; 0xFF indicated unknown display type.
    cmp bl, 0xFF
    je .VGAPresent

    ; 0x07 indicates VGA w/ monochrome display
    cmp bl, 0x07
    je .VGAPresent
 
    ; 0x08 indicates VGA w/ color analogue display.
    cmp bl, 0x08
    je .VGAPresent

    ; No - no VGA; check for VBE.
    jmp .VBECheck

; So VGA is present - set the right flag.
.VGAPresent:
    or byte [BIT.VideoFlags], VGA_PRESENT

; Check whether VBE is present or not.
.VBECheck:
    ; Ask for VBE2 info, if it's present.
    mov [ControllerInfo], dword 'VBE2'
    mov di, ControllerInfo
    mov ax, 0x4F00

    ; Get VBE Controller Mode Info.
    int 0x10

    ; If 0x004F (successful) wasn't returned, return.
    cmp ax, 0x004F
    jne .Return

    ; It fills the signature with VESA.
    cmp [ControllerInfo], dword 'VESA'
    jne .Return

    push es
    ; If it is a dummy VBE installation, i.e. no entries, then return.
    mov ax, [ControllerInfo + 16]     ; Get the segment into AX.
    mov es, ax                        ; And then into ES.
    mov bx, [ControllerInfo + 14]     ; Get the offset into BX.
 
    mov ax, [es:bx]                   ; And the first entry into AX.
    
    pop es                            ; Restore ES.

    cmp ax, 0xFFFF
    je .Return                        ; If first entry is "end of list", return.

    ; So, we support VBE.
    or byte [BIT.VideoFlags], VBE_PRESENT  ; We do.
    
    ; And store the address of the Controler Info into the BIT.
    mov dword [BIT.VBECntrlrInfo], ControllerInfo
 
.Return:
    popad
    ret


 ; Get's video modes information from VBE and store it at VBEModeInfo.
 ;
 ; Returns:
 ;     EAX -> the number of entries.
VBEGetModeInfo:
    ; Save es and fs.
    push es
    push fs
    push edi
    push esi
    push ecx
    push ebp

    ; GET THE ADDRESS OF THE OUTPUT BUFFER.
    ; Get the offset into EDI.
    mov edi, [BIT.VBEModeInfo]

    ; Get the segment into EAX.
    mov eax, edi
    and eax, ~0xFFFF

    ; Shift it right, equivalent to dividing by 2^4, or 16 (0x10).
    shr eax, 4
    ; And get it into EAX.
    mov es, ax

    ; AND INPUT BUFFER.
    ; Get segment into FS, and offset into SI.
    mov ax, [ControllerInfo + 16]     ; Get the segment into AX.
    mov fs, ax                        ; And then into FS.
    mov si, [ControllerInfo + 14]     ; Get the offset into SI.

    ; Count the entries in EBP.
    xor ebp, ebp
 
; For every mode, get the information.
.LoopModeInfo:
    ; So, if we are on the end of the list, then exit gracefully.
    mov cx, [fs:si]
    cmp cx, 0xFFFF
    je .Return

    ; Get the mode information, for whatever mode is in CX.
    mov ax, 0x4F01
    int 0x10

    ; If 0x004F (successful) wasn't returned, then simply try for the next entry.
    cmp ax, 0x004F
    jne .MoveNextInputBuf

    ; So we got one more entry.
    inc ebp

.MoveNextOutputBuf:
    ; Write the mode over the "mode" field (reserved actually, at offset 48), if we passed.
    mov [es:di + 48], cx

    ; Compare DI with the beginning of the last entry in the segment.
    cmp di, (0x10000 - 256)
    jb .MoveNextOutputBufOff

    ; Increase the segment, if we need to.
    mov ax, es
    add ax, 0x10
    mov es, ax

; Bad naming - but it conveys the point.
; Increment the offset of the output buffer.
.MoveNextOutputBufOff:
    ; So, it could also "overflow", and we'd already be on the next segment if it was to overflow.
    add edi, 256

; Move to the next thingy in the input buffer.
.MoveNextInputBuf:
    ; Compare SI with the beginning of the last entry in the segment.
    cmp si, (0x10000 - 2)
    jb .MoveNextInputBufOff

    ; Increase the segment, if we need to.
    mov ax, fs
    add ax, 0x10
    mov fs, ax

; Bad naming - as always - but it conveys the point - increment offset in input buffer.
.MoveNextInputBufOff:
    ; So it could overflow, as descibed above.
    add esi, 2

    jmp .LoopModeInfo

.Return:
    ; Get the number of entries in EAX.
    mov eax, ebp

    ; Restore fs and es.
    pop ebp
    pop ecx
    pop esi
    pop edi
    pop fs
    pop es
    ret