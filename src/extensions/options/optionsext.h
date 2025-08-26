/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          OPTIONSEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended OptionsClass class.
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
#pragma once

#include "always.h"
#include "extension.h"
#include "options.h"
#include <vector>


class CCINIClass;


class OptionsClassExtension final : public GlobalExtensionClass<OptionsClass>
{
    public:
        IFACEMETHOD(Load)(IStream *pStm);
        IFACEMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

    public:
        OptionsClassExtension(const OptionsClass *this_ptr);
        OptionsClassExtension(const NoInitClass &noinit);
        virtual ~OptionsClassExtension();

        /**
         *  OptionsClass extension does not require these to be used, but we
         *  implement them for completeness.
         */
        virtual int Get_Object_Size() const override;
        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual const char *Name() const override { return "Options"; }
        virtual const char *Full_Name() const override { return "Options"; }

        void Load_Settings();
        void Load_Init_Settings();
        void Save_Settings();

        void Set();

    public:

        /**
         *  Should cameos of defenses (including walls and gates) be sorted to the bottom of the sidebar?
         */
        bool SortDefensesAsLast;

        /**
         *  Are harvesters and MCVs excluded from a band-box selection that includes combat units?
         */
        bool FilterBandBoxSelection;

        struct {

            enum {
                MAX_TABS = 6
            };

            char MixLetter = '\0';

            bool IsTabs = false;
            int Tabs = 0;
            int Columns = 2;

            int TabHeight = 16;

            int CameoWidth = 64;
            int CameoHeight = 48;
            int CameoXSpacing = 3;
            int CameoYSpacing = 3;
            Point2D CameoNameOffset = Point2D(0, 41);
            Point2D CameoQueueCountOffset = Point2D(61, 2);
            Point2D CameoStateOffset = Point2D(33, 2);
            Point2D CameoQueueStateOffset = Point2D(0, 2);

            int StripXLeftSpace = 24;
            int StripXRightSpace = 13;
            int StripYOffset = 26;

            int ScrollRate = 51;

            int Get_Sidebar_Width() const { return (CameoWidth * Columns) + (CameoXSpacing * (Columns - 1)) + StripXRightSpace + StripXLeftSpace; }
            __declspec(property(get = Get_Sidebar_Width)) int SidebarWidth;

            int Get_Strip1_X() const { return StripXLeftSpace; }
            __declspec(property(get = Get_Strip1_X)) int Strip1X;

            int Get_Strip2_X() const { return Strip1X + CameoWidth + CameoXSpacing; }
            __declspec(property(get = Get_Strip2_X)) int Strip2X;

            int Get_Strip3_X() const { return Strip2X + CameoWidth + CameoXSpacing; }
            __declspec(property(get = Get_Strip3_X)) int Strip3X;

            int Get_Object_Width() const { return CameoWidth + CameoXSpacing; }
            __declspec(property(get = Get_Object_Width)) int ObjectWidth;

            int Get_Object_Height() const { return CameoHeight + CameoYSpacing; }
            __declspec(property(get = Get_Object_Height)) int ObjectHeight;

            Point2D PowerPosition = Point2D(8, 25);
            int PowerWidth = 12;
            int PowerHeightFudge = 1;
            int PowerPipHeight = 4;

            int RadarHeight = 134;
            Rect RadarMapRect = Rect(15, 12, 140, 108);

            Point2D RepairButtonPosition = Point2D(31, -9);
            Point2D SellButtonPosition = Point2D(58, -9);
            Point2D PowerButtonPosition = Point2D(85, -9);
            Point2D WaypointButtonPosition = Point2D(112, -9);

            std::vector<Point2D> TabButtonOffset;

            Point2D UpButtonOffset = Point2D(2, -1);
            Point2D DownButtonOffset = Point2D(31, -1);

            std::string StateColor = "LightGrey";
            std::string OnHoldColor = "LightBlue";

            int BuildingsTab = -1;
            int DefensesTab = -1;
            int SpecialTab = -1;
            int InfantryTab = -1;
            int UnitsTab = -1;
            int NavalTab = -1;
            int AircraftTab = -1;

            std::vector<std::string> TabName;

        } SidebarControls;
};
