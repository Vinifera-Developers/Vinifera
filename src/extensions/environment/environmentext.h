/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the extended EnvironmentClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/
 
#include "environment.h"
#include "environmentext_hooks.h"

/**
*  Replacement class for EnvironmentClass.
*
*  @author: ZivDero, tomsons26
*/
class ExtEnvironmentClass
{
    friend void EnvironmentExtension_Hooks();

public:
    ExtEnvironmentClass();
    ~ExtEnvironmentClass() = default;

    void Snapshot_Game_State();
    void Apply_To_Game_State();
    void Apply_Difficulty() const;
    void Apply_Globals();

    HRESULT Load(IStream* stream);
    HRESULT Save(IStream* stream);

private:
    ExtEnvironmentClass* Hook_Ctor() { return new (this) ExtEnvironmentClass; }
    void Hook_Dtor() { this->~ExtEnvironmentClass(); }

    static int __cdecl Static_Init();
    static void __cdecl Static_Deinit();

private:
    // Used to be Globals[50], available for re-use.
    DiffType CDifficulty;
    char __Padding[46]; 

public:
    int CarryOverMoney;
    int MissionTimer;
    DiffType Difficulty;
    unsigned short Stage;
};

/**
 *  Since we're not making a new instance, ensure that the size is the same
 */
static_assert(sizeof(ExtEnvironmentClass) == sizeof(EnvironmentClass), "sizeof(ExtEnvironmentClass) != sizeof(EnvironmentClass)!");