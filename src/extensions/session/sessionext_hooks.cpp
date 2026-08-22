/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended SessionClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "sessionext_hooks.h"

#include "sessionext_init.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "house.h"
#include "session.h"
#include "tibsun_globals.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor.
 *
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
class SessionClassExt : public SessionClass
{
public:
    bool _Am_I_Master();
};


/**
 *  Replacement for SessionClass::Am_I_Master.
 *
 *  The vanilla implementation only consults MasterPlayerID/MasterPlayerName
 *  in GAME_INTERNET (WOL) sessions, falling back to "the first human house
 *  is the master" otherwise. Spawner multiplayer sessions are GAME_IPX, so
 *  extend the check to them; the spawner host announces itself via
 *  EXT_NET_HOST_ANNOUNCE (see SessionClassExtension::Announce_Master).
 *
 *  @author: ZivDero
 */
bool SessionClassExt::_Am_I_Master()
{
    if (PlayerPtr == nullptr) {
        return false;
    }

    if (Type == GAME_INTERNET || Type == GAME_IPX) {
        if (MasterPlayerID != -1) {
            return PlayerPtr->HeapID == MasterPlayerID;
        }
        if (stricmp(PlayerPtr->IniName.c_str(), MasterPlayerName) == 0) {
            return true;
        }
    }

    for (int i = 0; i < Houses.Count(); i++) {
        HouseClass* hptr = Houses[i];
        if (hptr->IsHuman) {
            return PlayerPtr == hptr;
        }
    }

    return false;
}


/**
 *  Main function for patching the hooks.
 */
void SessionClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    SessionClassExtension_Init();

    Patch_Jump(0x005ED6C0, &SessionClassExt::_Am_I_Master);

    /**
     *  #issue-218
     *
     *  Changes the default value of SessionClass 0x1D91 (IsGDI) from "1" to "0".. This is
     *  because we now use it as a HouseType index, and need it to default to the first index.
     */
    Patch_Byte(0x005ED06B+1, 0x85); // changes "dl" (1) to "al" (0)
}
