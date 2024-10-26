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
    virtual int Size_Of() const override;
    virtual void Detach(TARGET target, bool all = true) override;
    virtual void Compute_CRC(WWCRCEngine &crc) const override;

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

    /**
     *  Number of autosaves to make in skirmish.
     */
    int AutoSaveCount;

    /**
     *  The delay between autosaves in skirmish in frames.
     */
    int AutoSaveInterval;
};
