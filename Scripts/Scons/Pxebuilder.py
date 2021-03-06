 # Draumr build system.
 #
 # Copyright (c) 2012, Shikhin Sethi
 # All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions are met:
 #     * Redistributions of source code must retain the above copyright
 #       notice, this list of conditions and the following disclaimer.
 #     * Redistributions in binary form must reproduce the above copyright
 #       notice, this list of conditions and the following disclaimer in the
 #       documentation and/or other materials provided with the distribution.
 #     * Neither the name of Draumr nor the
 #       names of its contributors may be used to endorse or promote products
 #       derived from this software without specific prior written permission.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 # ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 # WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 # DISCLAIMED. IN NO EVENT SHALL SHIKHIN SETHI BE LIABLE FOR ANY
 # DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 # (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 # LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 # ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 # SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import os
import shutil
import tempfile
import glob

from SCons.Builder import Builder
from SCons.Action import Action
from Isobuilder import Path

def _pxe_builder(target, source, env) :
    # Create a temporary directory to build the ISO image structure.
    if not os.path.exists('/tftpboot'):
        raise StopError("The /tftpboot directory, which would contain the PXE files, isn't present.")

    # Copy the kernel to the image.
    TftpBoot = "/tftpboot"
    
    Stage1 = str(env["PXE_STAGE1"][0])
    BIOS = str(env["BIOS"][0])
    DBAL = str(env["DBAL"][0])
    KL = str(env["KL"][0])
    Kernelx86 = str(env["Kernelx86"][0])
    KernelAMD64 = str(env["KernelAMD64"][0])
    PMMx86 = str(env["PMMx86"][0])
    PMMx86PAE = str(env["PMMx86PAE"][0])
    PMMAMD64 = str(env["PMMAMD64"][0])
    VMMx86 = str(env["VMMx86"][0])
    VMMx86PAE = str(env["VMMx86PAE"][0])
    VMMAMD64 = str(env["VMMAMD64"][0])
    
    shutil.copy(Stage1, TftpBoot)
    shutil.copy(BIOS, TftpBoot)
    shutil.copy(DBAL, TftpBoot)
    shutil.copy(KL, TftpBoot)
    shutil.copy(Kernelx86, TftpBoot)
    shutil.copy(KernelAMD64, TftpBoot)
    shutil.copy(PMMx86, TftpBoot)
    shutil.copy(PMMx86PAE, TftpBoot)
    shutil.copy(PMMAMD64, TftpBoot)
    shutil.copy(VMMx86, TftpBoot)
    shutil.copy(VMMx86PAE, TftpBoot)
    shutil.copy(VMMAMD64, TftpBoot)

    Background = str(env["BACK"])
    if env["BACK"] != 0:
        shutil.copy(Background, TftpBoot)
    
    print("  [PXE]   %s" % (target[0]))
    return 0

PXEBuilder = Builder(action = Action(_pxe_builder, None))

