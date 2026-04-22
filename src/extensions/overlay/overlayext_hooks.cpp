/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended OverlayClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "overlayext_hooks.h"
#include <unordered_set>

#include "bsurface.h"
#include "buffpipe.h"
#include "buffstraw.h"
#include "ccini.h"
#include "extension.h"
#include "hooker.h"
#include "lcwpipe.h"
#include "lcwstraw.h"
#include "mouse.h"
#include "overlayext.h"
#include "overlayext_init.h"
#include "overlaytype.h"
#include "session.h"
#include "tracker.h"
#include "vinifera_util.h"

/**
 *  Full reimplacement for OverlayClass::Read_INI.
 *  Implements support for NewINIFormat == 5, with 2 bytes for OverlayTypes.
 *
 *  @author: ZivDero
 */
void Read_INI(CCINIClass const& ini)
{
    if (NewINIFormat > 1) {

        BSurface temp_surface(512, 512, 2);
        temp_surface.Fill(0);

        int len = ini.Get_UUBlock("OverlayPack", temp_surface.Lock(), temp_surface.Get_Width() * temp_surface.Get_Height() * temp_surface.Bytes_Per_Pixel());

        std::unordered_set<OverlayType> error_overlaytypes;

        if (len > 0) {
            BufferStraw bpipe(temp_surface.Lock(), len);
            LCWStraw uncomp(LCWStraw::DECOMPRESS);
            uncomp.Get_From(&bpipe);

            for (int y = 0; y < MAP_CELL_H; y++) {
                for (int x = 0; x < MAP_CELL_W; x++) {
                    Cell cell(x, y);

                    OverlayType classid = OVERLAY_NONE;

                    if (NewINIFormat >= 5) {
                        uncomp.Get(&reinterpret_cast<short&>(classid), sizeof(short));
                    } else {
                        uncomp.Get(&reinterpret_cast<char&>(classid), sizeof(char));
                    }
                    
                    if (classid != OVERLAY_NONE) {
                        if (NewINIFormat >= 5) {
                            classid = static_cast<OverlayType>(classid & 0xFFFF);
                        } else {
                            classid = static_cast<OverlayType>(classid & 0x00FF);
                        }
                    }

                    if (classid != OVERLAY_NONE) {

                        if (OverlayTypes[classid]->Get_Image_Data() != nullptr || OverlayTypes[classid]->CellAnim) {
#if false
                            /*
                            **  Don't allow placement of crates in the multiplayer scenarios.
                            */
                            if (Session.Type == GAME_NORMAL || !OverlayTypes[classid]->IsCrate) {
#endif

                                /*
                                **  Don't allow placement of overlays on the top or bottom rows of
                                **  the map.
                                */
                                if (Map.In_Radar(cell)) {
                                    unsigned char old_overlay_data = Map[cell].OverlayData;
                                    new OverlayClass(OverlayTypes[classid], cell);

                                    if (static_cast<int>(classid) == OVERLAY_BRIDGE1 || static_cast<int>(classid) == OVERLAY_BRIDGE2 || static_cast<int>(classid) == OVERLAY_RAIL_BRIDGE1 || static_cast<int>(classid) == OVERLAY_RAIL_BRIDGE2) {
                                        Map[cell].OverlayData = old_overlay_data;
                                    }
                                }
#if false
                            }
#endif
                        } else {
                            if (!OverlayTypes[classid]->IsVeins && !error_overlaytypes.contains(classid)) { // Veinhole dummies intentionally have no valid image
                                error_overlaytypes.insert(classid);
                                Vinifera_Log_And_Show_WWMessageBox("Overlay type %s (%d) has no image!", OverlayTypes[classid]->IniName.c_str(), classid);
                            }
                        }
                    }
                }
            }
            temp_surface.Unlock();
        }
        temp_surface.Unlock();

        len = ini.Get_UUBlock("OverlayDataPack", temp_surface.Lock(), temp_surface.Get_Width() * temp_surface.Get_Height());

        if (len > 0) {
            BufferStraw bpipe(temp_surface.Lock(), len);
            LCWStraw uncomp(LCWStraw::DECOMPRESS);
            uncomp.Get_From(&bpipe);

            for (int y = 0; y < MAP_CELL_H; y++) {
                for (int x = 0; x < MAP_CELL_W; x++) {
                    Cell cell(x, y);
                    unsigned char overlay_data = 0;
                    uncomp.Get(&reinterpret_cast<char&>(overlay_data), sizeof(char));

                    if (Map.In_Radar(cell)) {
                        CellClass* cellptr = &Map[cell];
                        cellptr->OverlayData = overlay_data;
                    }
                }
            }
            temp_surface.Unlock();
        }
        temp_surface.Unlock();
    }
    Delete_Marked();
}


/**
 *  Full reimplacement for OverlayClass::Write_INI.
 *  Implements support for NewINIFormat == 5, with 2 bytes for OverlayTypes.
 *
 *  @author: ZivDero
 */
void Write_INI(CCINIClass& ini)
{
    /*
    **  First, clear out all existing unit data from the ini file.
    */
    ini.Clear("OVERLAY");
    ini.Clear("OverlayPack");

    BSurface temp_surface(512, 512, 2);
    temp_surface.Fill(0);

    BufferPipe bpipe(temp_surface.Lock(), temp_surface.Get_Width() * temp_surface.Get_Height());
    LCWPipe comppipe(LCWPipe::COMPRESS);

    comppipe.Put_To(&bpipe);

    int total = 0;
    for (int y = 0; y < MAP_CELL_H; y++) {
        for (int x = 0; x < MAP_CELL_W; x++) {
            if (NewINIFormat >= 5) {
                total += comppipe.Put(&reinterpret_cast<short&>(Map[Cell(x, y)].Overlay), sizeof(short));
            } else {
                total += comppipe.Put(&reinterpret_cast<char&>(Map[Cell(x, y)].Overlay), sizeof(char));
            }
        }
    }
    if (total) {
        ini.Put_UUBlock("OverlayPack", temp_surface.Lock(), total);
        temp_surface.Unlock();
    }
    temp_surface.Unlock();

    ini.Clear("OverlayDataPack");

    BufferPipe bpipe2(temp_surface.Lock(), temp_surface.Get_Width() * temp_surface.Get_Height());
    LCWPipe comppipe2(LCWPipe::COMPRESS);

    comppipe2.Put_To(&bpipe2);

    total = 0;
    for (int y = 0; y < MAP_CELL_H; y++) {
        for (int x = 0; x < MAP_CELL_W; x++) {
            total += comppipe2.Put(&reinterpret_cast<char&>(Map[Cell(x, y)].OverlayData), sizeof(char));
        }
    }
    if (total) {
        ini.Put_UUBlock("OverlayDataPack", temp_surface.Lock(), total);
        temp_surface.Unlock();
    }
    temp_surface.Unlock();
}


/**
 *  Main function for patching the hooks.
 */
void OverlayClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    OverlayClassExtension_Init();

    Patch_Jump(0x0058BE30, &Read_INI);
    Patch_Jump(0x0058C280, &Write_INI);
}
