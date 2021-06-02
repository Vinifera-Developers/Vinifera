/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          COMMANDEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended hotkey command class.
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

#include "extension.h"
#include "command.h"


class BuildingClass;
class HouseClass;


/**
 *  Based class for all new command classes.
 */
class ViniferaCommandClass : public CommandClass
{
    public:
        ViniferaCommandClass() : CommandClass(), IsDeveloper(false) {}
        virtual ~ViniferaCommandClass() {}

        virtual KeyNumType Default_Key() const = 0;

    public:
        /**
         *  Is this command only available in developer mode?
         */
        bool IsDeveloper;
};


/**
 *  
 */
class PNGScreenCaptureCommandClass : public ViniferaCommandClass
{
    public:
        PNGScreenCaptureCommandClass() : ViniferaCommandClass() { IsDeveloper = false; }
        virtual ~PNGScreenCaptureCommandClass() {}

        virtual const char *Get_Name() const override;
        virtual const char *Get_UI_Name() const override;
        virtual const char *Get_Category() const override;
        virtual const char *Get_Description() const override;
        virtual bool Process() override;

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
};


/**
 *  Produces a memory dump on request.
 */
class MemoryDumpCommandClass : public ViniferaCommandClass
{
    public:
        MemoryDumpCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
        virtual ~MemoryDumpCommandClass() {}

        virtual const char *Get_Name() const override;
        virtual const char *Get_UI_Name() const override;
        virtual const char *Get_Category() const override;
        virtual const char *Get_Description() const override;
        virtual bool Process() override;

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
};


/**
 *  Produces a log dump of all the game object CRC's
 */
class DumpHeapCRCCommandClass : public ViniferaCommandClass
{
    public:
        DumpHeapCRCCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
        virtual ~DumpHeapCRCCommandClass() {}

        virtual const char *Get_Name() const override;
        virtual const char *Get_UI_Name() const override;
        virtual const char *Get_Category() const override;
        virtual const char *Get_Description() const override;
        virtual bool Process() override;

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
};


#ifndef DEBUG
/**
 *  Based class for all new developer/debug command classes.
 */
class ViniferaDebugCommandClass : public ViniferaCommandClass
{
    public:
        ViniferaDebugCommandClass() : ViniferaCommandClass() {}
        virtual ~ViniferaDebugCommandClass() {}
};
#endif
