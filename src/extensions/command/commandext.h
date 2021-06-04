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


/**
 *  Toggles the instant build cheat for the player.
 */
class InstantBuildCommandClass : public ViniferaCommandClass
{
    public:
        InstantBuildCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
        virtual ~InstantBuildCommandClass() {}

        virtual const char *Get_Name() const override;
        virtual const char *Get_UI_Name() const override;
        virtual const char *Get_Category() const override;
        virtual const char *Get_Description() const override;
        virtual bool Process() override;

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
};


/**
 *  Toggles the instant build cheat for the AI.
 */
class AIInstantBuildCommandClass : public ViniferaCommandClass
{
    public:
        AIInstantBuildCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
        virtual ~AIInstantBuildCommandClass() {}

        virtual const char *Get_Name() const override;
        virtual const char *Get_UI_Name() const override;
        virtual const char *Get_Category() const override;
        virtual const char *Get_Description() const override;
        virtual bool Process() override;

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
};


/**
 *  Forces the player to win the current game session.
 */
class ForceWinCommandClass : public ViniferaCommandClass
{
    public:
        ForceWinCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
        virtual ~ForceWinCommandClass() {}

        virtual const char *Get_Name() const override;
        virtual const char *Get_UI_Name() const override;
        virtual const char *Get_Category() const override;
        virtual const char *Get_Description() const override;
        virtual bool Process() override;

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
};


/**
 *  Forces the player to lose the current game session.
 */
class ForceLoseCommandClass : public ViniferaCommandClass
{
    public:
        ForceLoseCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
        virtual ~ForceLoseCommandClass() {}

        virtual const char *Get_Name() const override;
        virtual const char *Get_UI_Name() const override;
        virtual const char *Get_Category() const override;
        virtual const char *Get_Description() const override;
        virtual bool Process() override;

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
};


/**
 *  Forces the player to blowup and lose the current game session.
 */
class ForceDieCommandClass : public ViniferaCommandClass
{
    public:
        ForceDieCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
        virtual ~ForceDieCommandClass() {}

        virtual const char *Get_Name() const override;
        virtual const char *Get_UI_Name() const override;
        virtual const char *Get_Category() const override;
        virtual const char *Get_Description() const override;
        virtual bool Process() override;

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
};


/**
 *  Take ownership of any selected objects.
 */
class CaptureObjectCommandClass : public ViniferaCommandClass
{
    public:
        CaptureObjectCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
        virtual ~CaptureObjectCommandClass() {}

        virtual const char *Get_Name() const override;
        virtual const char *Get_UI_Name() const override;
        virtual const char *Get_Category() const override;
        virtual const char *Get_Description() const override;
        virtual bool Process() override;

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
};


/**
 *  Grants all available special weapons to the player.
 */
class SpecialWeaponsCommandClass : public ViniferaCommandClass
{
    public:
        SpecialWeaponsCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
        virtual ~SpecialWeaponsCommandClass() {}

        virtual const char *Get_Name() const override;
        virtual const char *Get_UI_Name() const override;
        virtual const char *Get_Category() const override;
        virtual const char *Get_Description() const override;
        virtual bool Process() override;

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
};


/**
 *  Hands out free money to the player.
 */
class FreeMoneyCommandClass : public ViniferaCommandClass
{
    public:
        FreeMoneyCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
        virtual ~FreeMoneyCommandClass() {}

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
