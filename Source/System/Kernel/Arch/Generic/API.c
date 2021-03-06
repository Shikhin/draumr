/*
 * API for accessing the boot services.
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

#include <Standard.h>
#include <API.h>
#include <BIT.h>

// Define old AbortBootServices.
void (*OldAbortBootServices)(void);

// AbortBootServicesFunc.
EmptyFunc_t AbortBootServicesFunc;

// Define header's for predefined modules.
PMMHeader_t *PMMModHeader;
PMMEntry_t  *PMMModEntry;

/*
 * Initializes the API for all the modules.
 */
static void ModAPIInit()
{
    PMMModHeader = (PMMHeader_t*)(BIT_ADDR + MODULE_SIZE);   
    PMMModEntry = (PMMEntry_t*)(uintptr_t)(PMMModHeader->EntryPoint);
}

/*
 * Initializes the API.
 */
void APIInit()
{
	// Save the old AbortBootServices - which we'd call via our own function.
    OldAbortBootServices = (EmptyFunc_t)(uintptr_t)BIT->AbortBootServices;
	
    // Redirect the AbortBootServices function to the interface.
    AbortBootServicesFunc = &AbortBootServicesInt;

    ModAPIInit();
}