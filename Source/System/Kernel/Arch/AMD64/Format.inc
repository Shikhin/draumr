 ; Contains macros defined to "define" the Kernel Header.
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
 ; - INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 ; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION - HOWEVER CAUSED AND
 ; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 ; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 ; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

; Offset 0        Kernel Identifier.
; Offset 4        Address of entry point.
; Offset 12       Start of file.
; Offset 20       End of file.
; Offset 28       Starting address of BSS section.
; Offset 36       Ending address of BSS section.
; Offset 44       CRC32 remainder.

; This should just be at offset 0 of the file - identifies the Kernel.
%define KERN_DEF db "KE64"

; Defines the entry point of the Kernel.
%macro ENTRY_POINT 1
    dq %1
%endmacro 

; Defines the starting address of BSS section.
%macro BSS_START 1
    dq %1
%endmacro

; Defines the ending address of BSS section.
%macro BSS_END 1
    dq %1
%endmacro

; Defines the end of the file.
%macro FILE_END 1
    dq %1
%endmacro

; Defines the start of the file.
%macro FILE_START 0
    dq 0xFFFF800000000000
%endmacro

; The CRC32 remainder.
%define CRC32_DEFINE dd 0