/* 
 * Contains common Output definitions.
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

#include <Output.h>
#include <String.h>
#include <BootFiles.h>
#include <BIT.h>
#include <PMM.h>
#include <Math.h>
#include <Abort.h>

/*
 * Switches to a video mode.
 *     uint32_t Mode          -> the identifier for the mode we are about to switch to.
 *     VBEModeInfo_t *VBEMode -> a pointer to the Mode Info structure.
 *     
 * Returns:
 *     uint16_t               -> the status of the switch, where non-zero indicates errors.
 */
static uint16_t SwitchToMode(uint32_t Mode, VBEModeInfo_t *ModeInfo)
{
    // The 8th bit specifies whether it's a VBE mode or not.
    if(Mode & (1 << 8))
    {
        // Switch to the mode, and save the status value.
        uint16_t Return = (uint16_t)VideoAPIFunc(VIDEO_VBE_SWITCH_MODE,
                Mode);

        // If the mode is 256 colors, then set up the palette.
        if((ModeInfo->BitsPerPixel == 8) && (Return == 0x4F))
        {
            // If the flag is zero, then it is VGA compatible.
            // Use the VGA function to setup the palette then.
            // Else the VBE one.
            if(! (ModeInfo->ModeAttributes & VGA_COMPATIBLE))
                VideoAPIFunc(VIDEO_VGA_PALETTE);

            else
                VideoAPIFunc(VIDEO_VBE_PALETTE);
        }

        // Get rid of 0x4F - signifying function exists.
        Return &= ~0x4F;
        return Return;
    }

    else
    {
        // Switch to the mode.
        VideoAPIFunc(VIDEO_VGA_SWITCH_MODE, Mode);

        // If the mode is 256 colors, then set up the palette.
        if(ModeInfo->BitsPerPixel == 8)
            // Setup the palette to a RGB thingy.
            VideoAPIFunc(VIDEO_VGA_PALETTE);

        return 0x0000;
    }
}

/*
 * Get the best mode from VBEModeInfo[] array using all factors neccessary.
 */
static VBEModeInfo_t FindBestVBEMode()
{
    VBEModeInfo_t *VBEModeInfo = (VBEModeInfo_t *)BIT.Video.VBEModeInfo;
    // Simply find the best mode on the basis of the XRes, Yres and the BitsPerPixel.
    // Also, keep in mind the Monitor Preference.
    VBEModeInfo_t Best = VBEModeInfo[0];

    // Find log 2 of Bits Per Pixel, and multiply it with the scaling factor.
    Best.Score = fyl2x(Best.BitsPerPixel, BPP_SCALING_FACTOR);

    // Find the "shortest distance" between the three scores.
    // NOTE: This is better than taking an average. For example, considering 400*600 as the resolution for one mode
    // and 500*500 for the other mode. For average, both are given same score. For "distance" the former is
    // preferred.
    Best.Score =
            sqrt((Best.Score * Best.Score) + (Best.XResolution * Best.XResolution)
                  + (Best.YResolution * Best.YResolution));

    // Multiply the score by the MonitorPreference.
    Best.Score = (int) ((float)Best.Score * Best.MonitorPreference);

    // Calculate the score for each mode - while side by side, finding the best mode.
    for(uint32_t i = 1; i < BIT.Video.VBEModeInfoN; i++)
    {
        // If the monitor preference is 0, then continue.
        if((int)VBEModeInfo[i].MonitorPreference == 0)
        {
            continue;
        }

        // Find log 2 of Bits Per Pixel and multiply it with the scaling factor.
        VBEModeInfo[i].Score = fyl2x(
                VBEModeInfo[i].BitsPerPixel, BPP_SCALING_FACTOR);

        // Find the "shortest distance" (read note above) between the three scores.
        VBEModeInfo[i].Score =
                sqrt((VBEModeInfo[i].Score
                        * VBEModeInfo[i].Score)
                     + (VBEModeInfo[i].XResolution
                        * VBEModeInfo[i].XResolution)
                     + (VBEModeInfo[i].YResolution
                        * VBEModeInfo[i].YResolution));

        VBEModeInfo[i].Score =
                 (int)((float)VBEModeInfo[i].Score
                        * VBEModeInfo[i].MonitorPreference);

        // If the score of this is greater than the best till now,
        // make it the best.                                     
        if(VBEModeInfo[i].Score > Best.Score)
        {
            Best = VBEModeInfo[i];
        }
    }

    return Best;
}

static EDIDModeInfo_t EDIDModeInfo[47];
static uint32_t EDIDModeInfoN;

/* 
 * Handles established timings.
 */
static void HandleEstablishedTimings()
{
    for(uint32_t i = 0; i < 2; i++)
    {
        for(uint32_t j = 0; j < 8; j++)
        {
            if(BIT.Video.EDIDInfo.EstablishedTimings[i] & (1 << j))
            {
                switch (i)
                {
                    // Handle established timings in byte 0 in EDIDInfo.EstablishedTimings.
                    case 0:
                    {
                        switch (j)
                        {
                            // Bit 1.
                            case 0:
                                EDIDModeInfo[EDIDModeInfoN].XRes = 800;
                                EDIDModeInfo[EDIDModeInfoN].YRes = 600;
                                EDIDModeInfo[EDIDModeInfoN].RefreshRate = 60;

                                EDIDModeInfoN++;
                                break;

                                // Bit 2.
                            case 1:
                                EDIDModeInfo[EDIDModeInfoN].XRes = 800;
                                EDIDModeInfo[EDIDModeInfoN].YRes = 600;
                                EDIDModeInfo[EDIDModeInfoN].RefreshRate = 56;

                                EDIDModeInfoN++;
                                break;

                                // Bit 3.
                            case 2:
                                EDIDModeInfo[EDIDModeInfoN].XRes = 640;
                                EDIDModeInfo[EDIDModeInfoN].YRes = 480;
                                EDIDModeInfo[EDIDModeInfoN].RefreshRate = 75;

                                EDIDModeInfoN++;
                                break;

                                // Bit 4.
                            case 3:
                                EDIDModeInfo[EDIDModeInfoN].XRes = 640;
                                EDIDModeInfo[EDIDModeInfoN].YRes = 480;
                                EDIDModeInfo[EDIDModeInfoN].RefreshRate = 72;

                                EDIDModeInfoN++;
                                break;

                                // Bit 5.
                            case 4:
                                EDIDModeInfo[EDIDModeInfoN].XRes = 640;
                                EDIDModeInfo[EDIDModeInfoN].YRes = 480;
                                EDIDModeInfo[EDIDModeInfoN].RefreshRate = 67;

                                EDIDModeInfoN++;
                                break;

                                // Bit 6.
                            case 5:
                                EDIDModeInfo[EDIDModeInfoN].XRes = 640;
                                EDIDModeInfo[EDIDModeInfoN].YRes = 480;
                                EDIDModeInfo[EDIDModeInfoN].RefreshRate = 60;

                                EDIDModeInfoN++;
                                break;

                                // Bit 7.
                            case 6:
                                EDIDModeInfo[EDIDModeInfoN].XRes = 720;
                                EDIDModeInfo[EDIDModeInfoN].YRes = 400;
                                EDIDModeInfo[EDIDModeInfoN].RefreshRate = 88;

                                EDIDModeInfoN++;
                                break;

                                // Bit 8.
                            case 7:
                                EDIDModeInfo[EDIDModeInfoN].XRes = 720;
                                EDIDModeInfo[EDIDModeInfoN].YRes = 400;
                                EDIDModeInfo[EDIDModeInfoN].RefreshRate = 70;

                                EDIDModeInfoN++;
                                break;
                        }

                        break;
                    }

                        // Handle established timings in byte 1 in EDIDInfo.EstablishedTimings.
                    case 1:
                    {
                        switch (j)
                        {
                            // Bit 1.
                            case 0:
                                EDIDModeInfo[EDIDModeInfoN].XRes = 1280;
                                EDIDModeInfo[EDIDModeInfoN].YRes = 1024;
                                EDIDModeInfo[EDIDModeInfoN].RefreshRate = 75;

                                EDIDModeInfoN++;
                                break;

                                // Bit 2.
                            case 1:
                                EDIDModeInfo[EDIDModeInfoN].XRes = 1024;
                                EDIDModeInfo[EDIDModeInfoN].YRes = 768;
                                EDIDModeInfo[EDIDModeInfoN].RefreshRate = 75;

                                EDIDModeInfoN++;
                                break;

                                // Bit 3.
                            case 2:
                                EDIDModeInfo[EDIDModeInfoN].XRes = 1024;
                                EDIDModeInfo[EDIDModeInfoN].YRes = 768;
                                EDIDModeInfo[EDIDModeInfoN].RefreshRate = 70;

                                EDIDModeInfoN++;
                                break;

                                // Bit 4.
                            case 3:
                                EDIDModeInfo[EDIDModeInfoN].XRes = 1024;
                                EDIDModeInfo[EDIDModeInfoN].YRes = 768;
                                EDIDModeInfo[EDIDModeInfoN].RefreshRate = 60;

                                EDIDModeInfoN++;
                                break;

                                // Bit 5.
                            case 4:
                                EDIDModeInfo[EDIDModeInfoN].XRes = 1024;
                                EDIDModeInfo[EDIDModeInfoN].YRes = 768;
                                EDIDModeInfo[EDIDModeInfoN].RefreshRate = 87;

                                EDIDModeInfoN++;
                                break;

                                // Bit 6.
                            case 5:
                                EDIDModeInfo[EDIDModeInfoN].XRes = 832;
                                EDIDModeInfo[EDIDModeInfoN].YRes = 624;
                                EDIDModeInfo[EDIDModeInfoN].RefreshRate = 75;

                                EDIDModeInfoN++;
                                break;

                                // Bit 7.
                            case 6:
                                EDIDModeInfo[EDIDModeInfoN].XRes = 800;
                                EDIDModeInfo[EDIDModeInfoN].YRes = 600;
                                EDIDModeInfo[EDIDModeInfoN].RefreshRate = 75;

                                EDIDModeInfoN++;
                                break;

                                // Bit 8.
                            case 7:
                                EDIDModeInfo[EDIDModeInfoN].XRes = 800;
                                EDIDModeInfo[EDIDModeInfoN].YRes = 600;
                                EDIDModeInfo[EDIDModeInfoN].RefreshRate = 72;

                                EDIDModeInfoN++;
                                break;
                        }

                        break;
                    }
                }
            }
        }
    }

    // Handle established timings in byte 2 in EDIDInfo.EstablishedTimings.
    if(BIT.Video.EDIDInfo.EstablishedTimings[2] & (1 << 7))
    {
        EDIDModeInfo[EDIDModeInfoN].XRes = 1152;
        EDIDModeInfo[EDIDModeInfoN].YRes = 870;
        EDIDModeInfo[EDIDModeInfoN].RefreshRate = 75;

        EDIDModeInfoN++;
    }

    // NOTE: The rest bits in byte 2 are reserved (for OEM use and other such stuff).
}

/*
 * Handles standard timings.
 *     uint8_t Byte0 -> the first byte of the standard timing identifier.
 *     uint8_t Byte1 -> the second byte of the standard timing identifier.
 */
static void HandleStandardTimings(uint8_t Byte0, uint8_t Byte1)
{
    // If both bytes are 0x01, then it is a unused standard timing descriptor.
    if((Byte0 == 0x01) && (Byte1 == 0x01))
    {
        return;
    }

    // Calculate the horizontal active pixels.
    // The first byte is defined as the "(horizontal active pixels / 8) - 31".
    EDIDModeInfo[EDIDModeInfoN].XRes = (Byte0 + 31) * 8;

    // Now, using the aspect ratio, calculate the vertical active pixels.
    uint32_t AspectRatio = Byte1 >> 6, VerticalRatio, HorizontalRatio;
    switch (AspectRatio)
    {
        // Case 0.
        case 0:
            // If the version is 1.3 or above, then the aspect ratio is 16:10.
            if((BIT.Video.EDIDInfo.Version >= 1)
               && (BIT.Video.EDIDInfo.Revision >= 3))
            {
                HorizontalRatio = 16;
                VerticalRatio = 10;
            }

            // Else, it is 1:1.
            else
            {
                HorizontalRatio = 1;
                VerticalRatio = 1;
            }

            break;

            // Case 1.
        case 1:
            HorizontalRatio = 4;
            VerticalRatio = 3;

            break;

            // Case 2.
        case 2:
            HorizontalRatio = 5;
            VerticalRatio = 4;

            break;

            // Case 3.
        case 3:
            HorizontalRatio = 16;
            VerticalRatio = 9;

            break;

        default:
            HorizontalRatio = 4;
            VerticalRatio = 3;

            break;
    }

    // Calculate the Y resolution - using the ratio.
    EDIDModeInfo[EDIDModeInfoN].YRes = (EDIDModeInfo[EDIDModeInfoN].XRes
            * VerticalRatio) / HorizontalRatio;

    // Calculate the Refresh Rate.
    // It is stored in the first 6 bits, with it being 60 lower than it's original value.
    EDIDModeInfo[EDIDModeInfoN].RefreshRate = (Byte1 & 0x3F) + 60;

    EDIDModeInfoN++;
}

/* 
 * Rounds off a refresh rate so that it becomes easier to calculate what all modes are "preferable".
 *     uint32_t RefreshRate -> the refresh rate which we would round off.
 *
 * Returns:
 *     uint32_t             -> the rounded off refresh rate.
 */
static uint32_t RoundOffRefreshRate(uint32_t RefreshRate)
{
    // Since the refresh rate we calculate isn't exactly accurate (might be off by +/-1),
    // and we want some round numbers so as to calculate what all modes are preferable,
    // let's round off the refresh rate.

    // The preferances are:
    //     a) multiples of 5.
    //     b) multiples of 2.

    // If it's already a multiple of 5, return.
    if(! (RefreshRate % 5))
    {
        return RefreshRate;
    }

    // If refresh rate + 1 is a multiple of 5, then return it + 1.
    else if(! ((RefreshRate + 1) % 5))
    {
        return RefreshRate + 1;
    }

    // If refres rate - 1 is a multiple of 5, then return it - 1.
    else if(! ((RefreshRate - 1) % 5))
    {
        return RefreshRate - 1;
    }

    // Else, look if it's a multiple of 2. If it is, return.
    else if(! (RefreshRate % 2))
    {
        return RefreshRate;
    }

    // Else, return + 1.
    return RefreshRate + 1;
}

/*
 * Parses the BIT.Video.EDIDInfo structure, cleaning it out, and making a usable
 * array out of it.
 */
static void ParseEDIDInfo()
{
    // Ok - so the most sensible thing to do would be to parse all the timings
    // specified in Established timings, Standard timings and Detailed timings
    // and put all the usable modes in a array consisting of the Horizontal resolution,
    // Vertical resolution and Refresh rate.

    // Later, this could be used to find out how many timings are supported of the 
    // "standard" (no - not the EDID standard, the standard standard) timings of 
    // a mode, adding to the score.

    // Currently, the number of EDIDModeInfo structures are 0.
    EDIDModeInfoN = 0;

    // Take care of the established modes.
    HandleEstablishedTimings();

    // Take care of the standard timings.
    for(uint32_t i = 0; i < 8; i++)
    {
        HandleStandardTimings(BIT.Video.EDIDInfo.StandardTimings[i][0],
                BIT.Video.EDIDInfo.StandardTimings[i][1]);
    }

    // Set the flag indicating it's the monitor's native mode.
    EDIDModeInfo[EDIDModeInfoN].Flags |= EDID_MONITOR_NATIVE;

    // Take care of the modes described in Detailed Timings.
    for(uint32_t i = 0; i < 4; i++)
    {
        if((i > 0) && (BIT.Video.EDIDInfo.DetailedTimings[i][0] == 0x00)
           && (BIT.Video.EDIDInfo.DetailedTimings[i][1] == 0x00))
        {
            // It's a monitor descriptor.

            // If it doesn't contain standard timings, then we don't have any use of it. Continue.
            if(BIT.Video.EDIDInfo.DetailedTimings[i][3] != 0xFA)
            {
                continue;
            }

            for(uint32_t j = 4; j < 16; j += 2)
            {
                HandleStandardTimings(BIT.Video.EDIDInfo.DetailedTimings[i][j],
                        BIT.Video.EDIDInfo.DetailedTimings[i][j + 1]);
            }

            continue;
        }

        // Get horizontal and vertical active pixels.
        uint16_t HorizontalActive = BIT.Video.EDIDInfo.DetailedTimings[i][2];
        HorizontalActive |= (BIT.Video.EDIDInfo.DetailedTimings[i][4] & 0xF0)
                << 4;

        uint16_t VerticalActive = BIT.Video.EDIDInfo.DetailedTimings[i][5];
        VerticalActive |= (BIT.Video.EDIDInfo.DetailedTimings[i][7] & 0xF0)
                << 4;

        // Get the pixel clock & the horizontal blanking in pixels to calculate the refresh rate.
        uint16_t HorizontalBlank = BIT.Video.EDIDInfo.DetailedTimings[i][3];
        HorizontalBlank |= (BIT.Video.EDIDInfo.DetailedTimings[i][4] & 0x0F)
                << 8;

        // Step 1. Get the pixel clock speed in hz.
        float RefreshRate = ((BIT.Video.EDIDInfo.DetailedTimings[i][1] << 8)
                | (BIT.Video.EDIDInfo.DetailedTimings[i][0])) * 10000;

        // Step 2. Divide it by the sum of (horizontal blanking in pixels and total horizontal active pixels) to get the horizontal frequency.
        RefreshRate /= (double) (HorizontalActive + HorizontalBlank);

        // Step 3. Divide it by total vertical pixels * 1.05 (to account for time spent moving the electron beam back to the top)
        RefreshRate /= ((double)VerticalActive * 1.05f);

        EDIDModeInfo[EDIDModeInfoN].XRes = HorizontalActive;
        EDIDModeInfo[EDIDModeInfoN].YRes = VerticalActive;
        EDIDModeInfo[EDIDModeInfoN].RefreshRate =
                RoundOffRefreshRate((uint32_t)RefreshRate);

        EDIDModeInfoN++;
    }
}

/*
 * Calculate the monitor preference, if EDID Info isn't present.
 */
static void MonitorPreferenceNoEDID()
{
    VBEModeInfo_t *VBEModeInfo = (VBEModeInfo_t*)BIT.Video.VBEModeInfo;
    // Safe resolutions which are supported on all modes.
    uint32_t SafeResolutions[8][2] =
    { { 720, 480 },
      { 640, 480 },
      { 360, 480 },
      { 320, 480 },
      { 720, 240 },
      { 640, 240 },
      { 360, 240 },
      { 320, 240 } };

    for(uint32_t i = 0; i < BIT.Video.VBEModeInfoN; i++)
    {
        VBEModeInfo[i].MonitorPreference = 0;

        for(uint32_t j = 0; j < 8; j++)
        {
            // If the horizontal & vertical resolutions match to something in SafeResolution, then MonitorPreference = 1.
            if((VBEModeInfo[i].XResolution == SafeResolutions[j][0])
                && (VBEModeInfo[i].YResolution
                    == SafeResolutions[j][1]))
            {
                VBEModeInfo[i].MonitorPreference = 1;

                // No need to continue in the loop.
                break;
            }
        }
    }
}

/*
 * Calculate the monitor preference for every mode based on the EDID Info.
 */
static void CalculateMonitorPreference()
{
    VBEModeInfo_t *VBEModeInfo = (VBEModeInfo_t*)BIT.Video.VBEModeInfo;
    if(EDIDModeInfoN)
    {
        // Make a bitmap for all refresh rates from 0 - 256.
        uint32_t RefreshRateBitmap[4] = { 0, 0, 0, 0 };

        // Counter of total refresh rates encountered.
        uint32_t TotalRefreshRates = 0;

        // Find out total refresh rates.
        for(uint32_t i = 0; i < EDIDModeInfoN; i++)
        {
            uint32_t RefreshRate = EDIDModeInfo[i].RefreshRate;

            // If the bit in the bitmap isn't already set, set it & increase the count.
            if(!(RefreshRateBitmap[RefreshRate / 32]
                 & (1 << (RefreshRate % 32))))
            {
                TotalRefreshRates++;
                RefreshRateBitmap[RefreshRate / 32] |=
                        (1 << (RefreshRate % 32));
            }
        }

        // For every mode in VBEModeInfo[] array, calculate it's MonitorPreference.
        for(uint32_t i = 0; i < BIT.Video.VBEModeInfoN; i++)
        {
            uint32_t SupportedModes = 0, TotalModes = TotalRefreshRates;

            for(uint32_t j = 0; j < EDIDModeInfoN; j++)
            {
                // If the horizontal & vertical resolution are same, then increase SupportedModes.
                if((VBEModeInfo[i].XResolution
                    == EDIDModeInfo[j].XRes)
                    && (VBEModeInfo[i].YResolution
                        == EDIDModeInfo[j].YRes))
                {
                    SupportedModes++;

                    // If this is the native mode, then +1.
                    if(EDIDModeInfo[j].Flags & EDID_MONITOR_NATIVE)
                    {
                        SupportedModes++;
                        TotalModes++;
                    }
                }
            }

            // The final score is supported/total.
            VBEModeInfo[i].MonitorPreference = (float)SupportedModes
                    / (float)TotalModes;
        }
    }

    else
    {
        // Calculate the monitor preference for the "no EDID" case.
        MonitorPreferenceNoEDID();
    }
}

/*
 * Check the VBE entry for it's sanity.
 *     VBEModeInfo_t *VBEModeInfo -> the pointer to the current vbe mode structure.
 *
 * Returns:
 *     uint8_t                    -> 1 means that the VBE entry isn't sane enough.
 */
static uint8_t CheckVBESanity(VBEModeInfo_t *VBEModeInfo)
{
    // The VBE entry isn't sane if:
    if(     // The current hardware configuration doesn't support the mode.
            !(VBEModeInfo->ModeAttributes & HARDWARE_INIT)
            ||

            // The mode is a text mode.
            ! (VBEModeInfo->ModeAttributes & GRAPHICAL_MODE)
            ||

            // If it takes more than a bank, and LFB isn't available.
            ((((VBEModeInfo->XResolution * VBEModeInfo->YResolution
                * VBEModeInfo->BitsPerPixel) / 8) > 0x10000)
                && ! (VBEModeInfo->ModeAttributes & LFB_AVAILABLE))
            ||

            // If the mode isn't 4-bpp, 8-bpp, 15-bpp, 16-bpp, 24-bpp or 32-bpp.
            ((VBEModeInfo->BitsPerPixel != 4)
              && (VBEModeInfo->BitsPerPixel != 8)
              && (VBEModeInfo->BitsPerPixel != 15)
              && (VBEModeInfo->BitsPerPixel != 16)
              && (VBEModeInfo->BitsPerPixel != 24)
              && (VBEModeInfo->BitsPerPixel != 32))
            ||

            // The mode if 4-bpp, and isn't VGA compatible.
            ((VBEModeInfo->BitsPerPixel == 4)
              && !(VBEModeInfo->ModeAttributes & VGA_COMPATIBLE))
            ||

            // The mode is smaller than 320*200.
            (((VBEModeInfo->XResolution < 320)
               && (VBEModeInfo->YResolution < 200))
               && (BIT.Video.VideoFlags & VGA_PRESENT))
            ||

            // The size of each scan line isn't dword divisible.
            (VBEModeInfo->BytesPerScanLine % 4)
            ||

            // The X resolution isn't divisible by 4.
            (VBEModeInfo->XResolution % 4)
            ||

            // The memory model isn't:
            //     Packed Pixel.
            //     Direct Color.
            ((VBEModeInfo->MemoryModel != 4) && (VBEModeInfo->MemoryModel != 6))
            ||

            // Number of planes isn't 1 or 4 (for 4-bpp)
            ((VBEModeInfo->NumberOfPlanes != 1)
              || ((VBEModeInfo->BitsPerPixel == 4)
              && (VBEModeInfo->NumberOfPlanes != 4)))
            ||

            ((VBEModeInfo->BitsPerPixel == 15)
              && VERIFY_RGB_MODE(1, 15, 5, 10, 5, 5, 5, 0))
            ||

            ((VBEModeInfo->BitsPerPixel == 16)
              && VERIFY_RGB_MODE(0, 0, 5, 11, 6, 5, 5, 0))
            ||

            ((VBEModeInfo->BitsPerPixel == 24)
              && VERIFY_RGB_MODE(0, 0, 8, 16, 8, 8, 8, 0))
            ||

            ((VBEModeInfo->BitsPerPixel == 32)
              && ((VERIFY_RGB_MODE(8, 24, 8, 16, 8, 8, 8, 0)
              && (VERIFY_RGB_MODE(0, 0, 8, 16, 8, 8, 8, 0))))))
    {
        return 1;
    }

    // Else, the entry is sane.
    return 0;
}

/*
 * Parse the VBEModeInfo[] array, and clean it out for usable modes.
 */
static void ParseVBEInfo()
{
    // For easier, accesses (rather than calculating [i] again and again). (though I think the compiler would optimize the previous one out anyway).
    VBEModeInfo_t *VBEModeInfo = (VBEModeInfo_t*)BIT.Video.VBEModeInfo;
    VBECntrlrInfo_t *VBECntrlrInfo = (VBECntrlrInfo_t*)BIT.Video.VBECntrlrInfo;

    // Check through each entry, removing unneccessary ones.
    for(uint32_t i = 0; i < BIT.Video.VBEModeInfoN; i++)
    {
        // Default fill the PhysBasePtr entry, in case it's not filled.
        if(!VBEModeInfo[i].PhysBasePtr)
        {
            VBEModeInfo[i].PhysBasePtr = 0xA0000;
        }

        // If version is greater than equal to 0x0300, then replace all banked fields with linear fields.
        if(VBECntrlrInfo->Version >= 0x0300)
        {
            // Replace all * fields with Lin* fields.
            VBEModeInfo[i].BytesPerScanLine = VBEModeInfo[i].LinBytesPerScanLine;
            memcpy(&VBEModeInfo[i].RedMaskSize, &VBEModeInfo[i].LinRedMaskSize,
                    sizeof(uint8_t) * 8);
        }

        // Remove the entry if:
        if(CheckVBESanity(&VBEModeInfo[i]))
        {
            // Move the required number of entries from i + 1 to i - effectively deleting the current entry.
            memmove(&VBEModeInfo[i], &VBEModeInfo[i + 1],
                    sizeof(VBEModeInfo_t) * (BIT.Video.VBEModeInfoN - (i + 1)));
            BIT.Video.VBEModeInfoN--;

            // Here, we don't know whether the next entry is usable or not. So, move to previous, and continue.
            i--;
            continue;
        }

        // I've found out that certain video cards for 32-bpp modes set the 
        // Rsvd* fields to 0. I'm not too sure why - but we'll just set them to
        // the default value we want them to be.
        switch (VBEModeInfo[i].BitsPerPixel)
        {
            case 15:
                VBEModeInfo[i].BytesBetweenLines -= VBEModeInfo->XResolution / 8;
                break;

            case 32:
                VBEModeInfo[i].RsvdFieldPosition = 24;
                VBEModeInfo[i].RsvdMaskSize = 8;
                break;
        }

        if(VBEModeInfo[i].ModeAttributes & LFB_AVAILABLE)
        {
            // Set the use LFB bit.
            VBEModeInfo[i].Mode |= (1 << 14);
        }

        VBEModeInfo[i].BytesBetweenLines = (VBEModeInfo[i].BytesPerScanLine)
                - ((VBEModeInfo[i].XResolution * VBEModeInfo[i].BitsPerPixel) / 8);
    }

    return;
}

// Some prototypes, for OutputRevert().

/*
 * Initializes VGA for a graphical color mode.
 */
static void VGAInit();

/*
 * Initializes VBE for a graphical color mode.
 */
static void VBEInit();

// To know what we are using right now - VBE, VGA.
#define LEVEL_VBE 0
#define LEVEL_VGA 1

static int CurrentLevel;

/*
 * Reverts back from whatever we are currently using to the below level.
 */
void OutputRevert()
{
    switch (CurrentLevel)
    {
        case LEVEL_VBE:
            // If we need to go down a level from VBE, and VGA is supported switch to it.
            if(BIT.Video.VideoFlags & VGA_PRESENT)
            {
                CurrentLevel = LEVEL_VGA;
                VGAInit();

                // And return.
                return;
            }

            break;

        case LEVEL_VGA:
            // Switch to mode 80*25 text, so that we can safely print something out (if it does print, but it causes no harm).
            VideoAPIFunc(VIDEO_VGA_SWITCH_MODE, MODE_80_25_TEXT);

            // If we need to go down a level from Serial, ABORT!
            AbortBoot(
                    "Unable to find any suitable display system (VGA, VBE).\n");
            break;
    }
}

/*
 * Initializes VGA for a graphical color mode.
 */
static void VGAInit()
{
    // Fill in some general details of the video mode.
    BIT.Video.ModeInfo.Mode = MODE_640_480_16;
    BIT.Video.ModeInfo.PhysBasePtr = 0xA0000;
    BIT.Video.ModeInfo.XResolution = 640;
    BIT.Video.ModeInfo.YResolution = 480;

    // Set the bits per pixel and the number of planes for the mode.
    BIT.Video.ModeInfo.BitsPerPixel = 4;
    BIT.Video.ModeInfo.NumberOfPlanes = 4;

    // The bytes between lines and the bytes per scan line.
    BIT.Video.ModeInfo.BytesBetweenLines = 0;
    BIT.Video.ModeInfo.BytesPerScanLine = (640 * 4) / 8;

    // And the mode is supported by hardware, is a graphical mode and is VGA compatible.
    BIT.Video.ModeInfo.ModeAttributes = HARDWARE_INIT | GRAPHICAL_MODE
            | VGA_COMPATIBLE;

    // Go to the 320*200*256 colors mode.
    SwitchToMode(MODE_640_480_16, &BIT.Video.ModeInfo);
}

/*  
 * Initializes VBE for a graphical mode.
 * If something fails, automatically reverts to VGA.
 */
static void VBEInit()
{
    VBECntrlrInfo_t *VBECntrlrInfo = (VBECntrlrInfo_t*)BIT.Video.VBECntrlrInfo;
    // If mode number is below 0x0102, then, revert. 
    if(VBECntrlrInfo->Version < 0x0102)
    {
        OutputRevert();
        return;
    }

    // Get the segment and the offset of the list of the modes.
    uint16_t Segment = VBECntrlrInfo->VideoModesFar & 0xFFFF0000;
    uint16_t Offset = VBECntrlrInfo->VideoModesFar & 0x0000FFFF;

    // Make flat pointer, from segment and offset = (segment * 0x10) + offset;
    uint16_t *VideoModesFlat = (uint16_t*) ((Segment * 0x10) + Offset);
    uint16_t Mode = *VideoModesFlat++;
    uint32_t Entries = 0;

    // Keep looping till we reach the End of Entries.
    do
    {
        // So we got one more entry.
        Entries++;
        // Get the mode into Mode, and move on to the next video mode.
        Mode = *VideoModesFlat++;
    } while(Mode != 0xFFFF);

    // Allocate some memory from the Base Bitmap to hold all the mode information.
    BIT.Video.VBEModeInfo = (uint32_t)PMMAllocContigFrames(BASE_BITMAP,
            ((sizeof(VBEModeInfo_t) * Entries) + 0xFFF) / 0x1000);

    // If we failed to allocate enough space, simply revert back.
    if(!BIT.Video.VBEModeInfo)
    {
        OutputRevert();
        return;
    }

    // Get mode information from VBE, and the number of entries in BIT.Video.VBEModeInfoN.
    BIT.Video.VBEModeInfoN = VideoAPIFunc(VIDEO_VBE_GET_MODES,
            BIT.Video.VBEModeInfo);

    // Parse the VBEModeInfo[] array, and clean it out for usable modes.
    ParseVBEInfo();

    // If no mode was defined "usable", then revert.
    if(!BIT.Video.VBEModeInfoN)
    {
        OutputRevert();
        return;
    }

    // Parse EDID Information.
    if(BIT.Video.VideoFlags & EDID_PRESENT)
    {
        // Do a checksum on the EDID structure.
        uint8_t *EDID = (uint8_t*)&BIT.Video.EDIDInfo, Checksum = 0;
        for(uint32_t i = 0; i < sizeof(EDIDInfo_t); i++)
        {
            Checksum += EDID[i];
        }

        // Only if checksum passes, use EDID information.
        if(!Checksum)
            ParseEDIDInfo();
    }

    // Calculate the montior's preference for every mode.
    CalculateMonitorPreference();

    // Get the best mode from VBEModeInfo[] array - and then switch to it.
    BIT.Video.ModeInfo = FindBestVBEMode();

    // If the best mode also has score 0, then revert.
    if(!BIT.Video.ModeInfo.Score)
    {
        OutputRevert();
        return;
    }

    // Switch to the mode.
    if(SwitchToMode(BIT.Video.ModeInfo.Mode, &BIT.Video.ModeInfo))
    {
        OutputRevert();
        return;
    }
}

/*
 * Intializes a proper video mode, which is supported by the OS, the video card and the monitor (and is beautiful).
 */
void OutputInit()
{
    // If VBE is present, initialize VBE for a graphical mode.
    if(BIT.Video.VideoFlags & VBE_PRESENT)
    {
        CurrentLevel = LEVEL_VBE;
        VBEInit();
    }

    // Else, if VGA is present, initialize VGA for a graphical mode.
    else if(BIT.Video.VideoFlags & VGA_PRESENT)
    {
        CurrentLevel = LEVEL_VGA;
        VGAInit();
    }

    else
    {
        AbortBoot("Unable to find any suitable display system (VGA, VBE).\n");
    }

    // Open the file, and get it's information.
    BIT.Video.BackgroundImg = BootFilesBGImg();
}
