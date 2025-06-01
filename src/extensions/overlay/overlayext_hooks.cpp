/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          OVERLAYEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended OverlayClass.
 *
 *  @license       Vinifera is free software: you can redistribute it and/or
 *                 modify it under the terms of the GNU General Public License
 *                 as published by the Free Software Foundation, either version
 *                 3 of the License, or (at your option) any later version.
 *
 *                 Vinifera is distributed in the hope that it will be
 *                 useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *                 PURPOSE. See the GNU General Public License for more details.
 *
 *                 You should have received a copy of the GNU General Public
 *                 License along with this program.
 *                 If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/
#include "overlayext_hooks.h"
#include "overlayext_init.h"
#include "overlayext.h"
#include "extension.h"
#include "bsurface.h"
#include "buffpipe.h"
#include "buffstraw.h"
#include "ccini.h"
#include "lcwpipe.h"
#include "lcwstraw.h"
#include "session.h"
#include "overlaytype.h"
#include "lcwpipe.h"
#include "lcwstraw.h"
#include "mouse.h"
#include "session.h"
#include "tracker.h"

#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"
#include "hooker.h"
#include "hooker_macros.h"


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

        int len = ini.Get_UUBlock("OverlayPack", temp_surface.Lock(), temp_surface.Get_Width() * temp_surface.Get_Height() * temp_surface.BytesPerPixel);

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

                    if (classid != OVERLAY_NONE && (OverlayTypes[classid]->Get_Image_Data() != nullptr || OverlayTypes[classid]->CellAnim)) {

                        /*
                        **	Don't allow placement of crates in the multiplayer scenarios.
                        */
#if false
                        if (Session.Type == GAME_NORMAL || !OverlayTypes[classid]->IsCrate) {
#endif

                            /*
                            **	Don't allow placement of overlays on the top or bottom rows of
                            **	the map.
                            */
                            if (Map.In_Radar(cell)) {
                                unsigned char old_overlay_data = Map[cell].OverlayData;
                                new OverlayClass(OverlayTypes[classid], cell);

                                if (static_cast<int>(classid) == OVERLAY_BRIDGE1 || static_cast<int>(classid) == OVERLAY_BRIDGE2 ||
                                    static_cast<int>(classid) == OVERLAY_RAIL_BRIDGE1 || static_cast<int>(classid) == OVERLAY_RAIL_BRIDGE2) {
                                    Map[cell].OverlayData = old_overlay_data;
                                }
                            }
#if false
                        }
#endif
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
    Remove_All_Inactive();
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
    **	First, clear out all existing unit data from the ini file.
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
