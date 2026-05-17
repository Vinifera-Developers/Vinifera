/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  In-game ImGui debug overlay window.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "debug_overlay.h"

#include "aircraft.h"
#include "anim.h"
#include "armortype.h"
#include "building.h"
#include "bullet.h"
#include "event.h"
#include "factory.h"
#include "foot.h"
#include "house.h"
#include "housetype.h"
#include "infantry.h"
#include "mission.h"
#include "tag.h"
#include "tagtype.h"
#include "team.h"
#include "teamtype.h"
#include "triggertype.h"
#include "net/combuf.h"
#include "net/ipxconn.h"
#include "net/ipxmgr.h"
#include "object.h"
#include "objecttype.h"
#include "overlay.h"
#include "particle.h"
#include "queue.h"
#include "session.h"
#include "smudge.h"
#include "techno.h"
#include "technotype.h"
#include "terrain.h"
#include "tibsun_globals.h"
#include "trigger.h"
#include "unit.h"
#include "verses.h"
#include "veterancy.h"
#include "vinifera_globals.h"
#include "voxelanim.h"
#include "weapontype.h"

#include <imgui.h>

#include <cstdio>


bool DebugOverlay::IsVisible = false;


namespace
{
    /**
     *  Picks the house to display on the House tab: the owner of the first
     *  currently-selected object, falling back to the local player.
     */
    static HouseClass* Get_Display_House()
    {
        if (CurrentObjects.Count() > 0 && CurrentObjects[0] != nullptr) {
            HouseClass* owner = CurrentObjects[0]->Owner_HouseClass();
            if (owner != nullptr) {
                return owner;
            }
        }
        return PlayerPtr;
    }

    static void Format_Mission_Time(char* out, size_t out_size)
    {
        const int total_seconds = Frame / 15; // logic ticks: ~15 ticks/sec
        const int hours = total_seconds / 3600;
        const int minutes = (total_seconds / 60) % 60;
        const int seconds = total_seconds % 60;
        std::snprintf(out, out_size, "%d:%02d:%02d", hours, minutes, seconds);
    }

    /**
     *  Draws the always-on ts-patches INFO_PERFORMANCE block plus, in debug
     *  builds, the vanilla DMONO_STRESS-style heap / queue dump.
     *
     *  @author: ZivDero
     */
    static void Draw_Stats_Tab()
    {
        char timebuf[16];
        Format_Mission_Time(timebuf, sizeof(timebuf));

        ImGui::Text("Time : %s", timebuf);
        ImGui::Text("FPS  : %u", FramesPerSecond);

        if (!Vinifera_DeveloperMode) {
            return;
        }

        ImGui::Text("Frame      : %ld", Frame);

        /**
         *  Live instance heaps -- counts here change every frame and matter
         *  for runtime debug. Static rules-definition heaps (TeamTypes,
         *  TriggerTypes, etc) are intentionally omitted.
         */
        ImGui::SeparatorText("Heaps (combatants)");
        ImGui::Text("Units      : %d", Units.Count());
        ImGui::Text("Infantry   : %d", Infantry.Count());
        ImGui::Text("Aircraft   : %d", Aircrafts.Count());
        ImGui::Text("Buildings  : %d", Buildings.Count());

        ImGui::SeparatorText("Heaps (FX)");
        ImGui::Text("Bullets    : %d", Bullets.Count());
        ImGui::Text("Anims      : %d", Anims.Count());
        ImGui::Text("VoxelAnims : %d", VoxelAnims.Count());
        ImGui::Text("Particles  : %d", Particles.Count());

        ImGui::SeparatorText("Heaps (map)");
        ImGui::Text("Terrains   : %d", Terrains.Count());
        ImGui::Text("Smudges    : %d", Smudges.Count());
        ImGui::Text("Overlays   : %d", Overlays.Count());

        ImGui::SeparatorText("Heaps (state)");
        ImGui::Text("Factories  : %d", Factories.Count());
        ImGui::Text("Triggers   : %d", Triggers.Count());

        /**
         *  Real queues -- the multiplayer event lists used for sync.
         *  OutList = events leaving this client; DoList = events to apply
         *  this frame.
         */
        ImGui::SeparatorText("Queues");
        ImGui::Text("OutList    : %d", OutList.Count);
        ImGui::Text("DoList     : %d", DoList.Count);
    }

    static const char* Diff_To_String(DiffType d)
    {
        switch (d) {
        case DIFF_EASY:   return "Easy";
        case DIFF_NORMAL: return "Normal";
        case DIFF_HARD:   return "Hard";
        default:          return "?";
        }
    }

    static void Draw_Factory_Line(const char* label, FactoryClass* factory)
    {
        if (factory == nullptr) {
            ImGui::Text("%-12s: (idle)", label);
            return;
        }
        const TechnoClass* obj = factory->Get_Object();
        const char* name = obj && obj->Class_Of() ? obj->Class_Of()->Name() : "?";
        ImGui::Text("%-12s: %s", label, name);
    }

    /**
     *  Vanilla DMONO_HOUSE-style player-state dump, retargeted to the owner
     *  of the currently-selected object (falls back to PlayerPtr).
     */
    static void Draw_House_Tab()
    {
        HouseClass* h = Get_Display_House();
        if (h == nullptr) {
            ImGui::TextDisabled("(no house)");
            return;
        }

        ImGui::Text("House : %s (%s) [#%d]", h->IniName.c_str(), h->Class->IniName.c_str(), h->HeapID);
        ImGui::Separator();

        ImGui::Text("Credits  : %ld", h->Available_Money());
        ImGui::Text("Capacity : %ld", h->Capacity);

        const long power = h->Power_Output();
        const long drain = h->Power_Drain();
        const bool low_power = drain > power;
        if (low_power) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Power    : %ld / %ld (LOW)", power, drain);
        } else {
            ImGui::Text("Power    : %ld / %ld", power, drain);
        }

        ImGui::SeparatorText("Production");
        Draw_Factory_Line("Building", h->BuildingFactory);
        Draw_Factory_Line("Unit",     h->UnitFactory);
        Draw_Factory_Line("Infantry", h->InfantryFactory);
        Draw_Factory_Line("Aircraft", h->AircraftFactory);

        ImGui::SeparatorText("Profile");
        ImGui::Text("TechLevel  : %d", h->Control.TechLevel);
        ImGui::Text("IQ         : %d", h->IQ);
        ImGui::Text("Difficulty : %s", Diff_To_String(h->Difficulty));

        ImGui::SeparatorText("Biases");
        ImGui::Text("Firepower : x%.2f", h->FirepowerBias);
        ImGui::Text("Armor     : x%.2f", h->ArmorBias);
        ImGui::Text("ROF       : x%.2f", h->ROFBias);
        ImGui::Text("GroundSpd : x%.2f", h->GroundspeedBias);
        ImGui::Text("AirSpd    : x%.2f", h->AirspeedBias);
        ImGui::Text("Cost      : x%.2f", h->CostBias);
        ImGui::Text("BuildSpd  : x%.2f", h->BuildSpeedBias);

        ImGui::SeparatorText("Losses");
        ImGui::Text("Units lost      : %u", h->UnitsLost);
        ImGui::Text("Buildings lost  : %u", h->BuildingsLost);

        if (ImGui::CollapsingHeader("Flags")) {
            ImGui::Text("IsHuman         : %s", h->IsHuman ? "yes" : "no");
            ImGui::Text("IsPlayerControl : %s", h->IsPlayerControl ? "yes" : "no");
            ImGui::Text("IsAlerted       : %s", h->IsAlerted ? "yes" : "no");
            ImGui::Text("IsDefeated      : %s", h->IsDefeated ? "yes" : "no");
            ImGui::Text("IsToDie         : %s", h->IsToDie ? "yes" : "no");
            ImGui::Text("IsToWin         : %s", h->IsToWin ? "yes" : "no");
            ImGui::Text("IsToLose        : %s", h->IsToLose ? "yes" : "no");
        }
    }

    static const char* Rank_To_String(VeterancyRankType r)
    {
        switch (r) {
        case RANK_ROOKIE:  return "Rookie";
        case RANK_VETERAN: return "Veteran";
        case RANK_ELITE:   return "Elite";
        default:           return "?";
        }
    }

    static const char* Persistence_To_String(PersistantType p)
    {
        switch (p) {
        case VOLATILE:       return "Volatile";
        case SEMIPERSISTANT: return "Semi-persistent";
        case PERSISTANT:     return "Persistent";
        default:             return "?";
        }
    }

    /**
     *  Draws one weapon slot's stats. ROF in TS is ticks between shots, and
     *  there are 15 logic ticks/sec, so raw shots-per-second = 15/ROF and
     *  DPS = Attack * Burst * 15 / ROF.
     */
    static void Draw_Weapon_Block(const char* label, const WeaponTypeClass* w, double firepower_bias)
    {
        if (w == nullptr) {
            ImGui::Text("%s : (none)", label);
            return;
        }

        /**
         *  Scope ImGui IDs so the two weapon blocks on the same tab don't
         *  collide on identically-labelled children (e.g. the Verses
         *  TreeNode).
         */
        ImGui::PushID(label);

        ImGui::Text("%s : %s", label, w->Name());
        ImGui::Indent();

        ImGui::Text("Attack=%d  Burst=%d  ROF=%d", w->Attack, w->Burst, w->ROF);

        if (w->ROF > 0) {
            const double dps_raw  = static_cast<double>(w->Attack) * w->Burst * 15.0 / w->ROF;
            const double dps_bias = dps_raw * firepower_bias;
            ImGui::Text("DPS raw=%.1f  with-bias=%.1f", dps_raw, dps_bias);
        } else {
            ImGui::TextDisabled("DPS : (ROF=0)");
        }

        if (w->WarheadPtr != nullptr) {
            ImGui::Text("Warhead : %s", w->WarheadPtr->Name());

            if (ImGui::TreeNode("Verses (vs each armor)")) {
                for (ArmorType a = ARMOR_FIRST; a < ArmorTypes.Count(); ++a) {
                    const double mod = Verses::Get_Modifier(a, w->WarheadPtr);
                    ImGui::Text("vs %-12s x%.2f", ArmorTypes[a]->Name(), mod);
                }
                ImGui::TreePop();
            }
        }

        ImGui::Unindent();
        ImGui::PopID();
    }

    /**
     *  ts-patches INFO_UNIT-style selected-unit panel: identity, HP, armor,
     *  veterancy, speed (for FootClass), house biases, and per-weapon stats
     *  including DPS and the verses table for the weapon's warhead.
     */
    static void Draw_Unit_Tab()
    {
        if (CurrentObjects.Count() == 0) {
            ImGui::TextDisabled("Nothing Selected");
            return;
        }

        ObjectClass* obj = CurrentObjects[0];
        if (obj == nullptr || obj->Class_Of() == nullptr) {
            ImGui::TextDisabled("Nothing Selected");
            return;
        }

        const ObjectTypeClass* type = obj->Class_Of();
        HouseClass* owner = obj->Owner_HouseClass();
        const bool is_enemy = PlayerPtr != nullptr && owner != nullptr && !PlayerPtr->Is_Ally(owner);

        /**
         *  Outside of developer mode, refuse to leak details about enemy
         *  units -- mirrors the "Nothing Selected" placeholder.
         */
        if (is_enemy && !Vinifera_DeveloperMode) {
            ImGui::TextDisabled("Enemy Unit Selected");
            return;
        }

        ImGui::Text("Unit    : %s", type->Name());
        ImGui::Text("Owner   : %s%s",
            owner ? owner->IniName.c_str() : "(none)",
            is_enemy ? "  [enemy]" : "");

        ImGui::Text("HP      : %d / %d", obj->Strength, type->MaxStrength);

        const Cell c = obj->Get_Cell();
        ImGui::Text("Cell    : %d, %d", c.X, c.Y);

        const ArmorType armor = type->Armor;
        const char* armor_name = (armor >= ARMOR_FIRST && armor < ArmorTypes.Count())
            ? ArmorTypes[armor]->Name() : "?";
        ImGui::Text("Armor   : %s", armor_name);

        if (!obj->Is_Techno()) {
            return;
        }

        TechnoClass* techno = static_cast<TechnoClass*>(obj);
        ImGui::Text("Mission : %s", MissionClass::Mission_Name(techno->Get_Mission()));

        /**
         *  Control-group: 0-9 if the unit is part of a Ctrl+# group,
         *  otherwise -1 (0xFFFFFFFF when reinterpreted unsigned).
         */
        const int group = static_cast<int>(techno->Group);
        if (group >= 0 && group <= 9) {
            ImGui::Text("Group   : %d", group);
        } else {
            ImGui::Text("Group   : (none)");
        }

        ImGui::Text("Rank    : %s (xp=%.2f)", Rank_To_String(techno->Crew.Get_Rank()), techno->Crew.Get_Experience());

        /**
         *  Foot units carry their own speed bias on top of the house ground
         *  speed bias (Veterancy promotions, terrain, etc).
         */
        const bool is_foot = obj->RTTI == RTTI_UNIT || obj->RTTI == RTTI_INFANTRY || obj->RTTI == RTTI_AIRCRAFT;
        if (is_foot) {
            FootClass* foot = static_cast<FootClass*>(obj);
            ImGui::Text("Speed   : %.2f (bias x%.2f)", foot->Speed, foot->SpeedBias);
        }

        /**
         *  Scenario-script bindings: Team (foot units only) and Tag (any
         *  object). Only shown when something is actually attached so the
         *  panel stays uncluttered for plain units.
         */
        TeamClass* team = is_foot ? static_cast<FootClass*>(obj)->Team : nullptr;
        TagClass* tag = obj->Tag;
        if (team != nullptr || tag != nullptr) {
            ImGui::SeparatorText("Scenario");

            if (team != nullptr) {
                ImGui::Text("Team    : %s", team->Name());
                if (ImGui::TreeNode("Team details")) {
                    ImGui::Text("Members       : %d", team->Total);
                    ImGui::Text("Risk          : %d", team->Risk);
                    ImGui::Text("Forced active : %s", team->IsForcedActive ? "yes" : "no");
                    ImGui::Text("Full strength : %s", team->IsFullStrength ? "yes" : "no");
                    ImGui::Text("Under strength: %s", team->IsUnderStrength ? "yes" : "no");
                    ImGui::Text("Has been      : %s", team->IsHasBeen ? "yes" : "no");
                    ImGui::TreePop();
                }
            }

            if (tag != nullptr) {
                const TagTypeClass* tagtype = tag->Class;
                ImGui::Text("Tag     : %s", tagtype ? tagtype->Name() : "?");
                if (ImGui::TreeNode("Tag details")) {
                    if (tagtype != nullptr && tagtype->TriggerType != nullptr) {
                        ImGui::Text("Trigger     : %s", tagtype->TriggerType->Name());
                    }
                    if (tagtype != nullptr) {
                        ImGui::Text("Persistence : %s", Persistence_To_String(tagtype->Persistence));
                    }
                    ImGui::Text("AttachCount : %d", tag->AttachCount);
                    ImGui::Text("IsSprung    : %s", tag->IsSprung ? "yes" : "no");
                    ImGui::Text("IsToDie     : %s", tag->IsToDie ? "yes" : "no");
                    ImGui::TreePop();
                }
            }
        }

        ImGui::SeparatorText("Weapons");
        const double firepower_bias = owner ? owner->FirepowerBias : 1.0;
        const WeaponInfoStruct* primary   = techno->Get_Weapon(WEAPON_SLOT_PRIMARY);
        const WeaponInfoStruct* secondary = techno->Get_Weapon(WEAPON_SLOT_SECONDARY);
        Draw_Weapon_Block("Primary  ", primary   ? primary->Weapon   : nullptr, firepower_bias);
        Draw_Weapon_Block("Secondary", secondary ? secondary->Weapon : nullptr, firepower_bias);
    }

    /**
     *  TS encodes most network timings in 1/60-second units; vanilla's
     *  Multiplayer_Debug_Print divides by 60 and multiplies by 1000 to get
     *  milliseconds.
     */
    static unsigned long Ticks_To_Ms(unsigned long ticks)
    {
        return ticks * 1000UL / 60UL;
    }

    /**
     *  Looks up a peer's ProcessTime from Session.Players by matching the
     *  player ID (which doubles as a house ID).
     */
    static int Find_Peer_Process_Time(HousesType player_id)
    {
        for (int j = 0; j < Session.Players.Count(); ++j) {
            const NodeNameType* node = Session.Players[j];
            if (node != nullptr && node->Player.ID == player_id) {
                return node->Player.ProcessTime;
            }
        }
        return -1;
    }

    /**
     *  ts-patches INFO_NETWORK panel plus the contents of vanilla's
     *  Multiplayer_Debug_Print (mainloop.cpp + IPXManagerClass) -- the global
     *  sync/latency block on top, per-peer rtt/process below.
     */
    static void Draw_Network_Tab()
    {
        ImGui::SeparatorText("Sync");
        ImGui::Text("Frame       : %ld", Frame);
        ImGui::Text("FPS         : %u", FramesPerSecond);
        ImGui::Text("MaxAhead    : %d", Session.MaxAhead);
        ImGui::Text("Req FPS     : %d", Session.DesiredFrameRate);
        ImGui::Text("Resp Time   : %lu ms", Ticks_To_Ms(Ipx.Response_Time()));
        ImGui::Text("Lat Fudge   : %d", Session.LatencyFudge);
        ImGui::Text("Rtr delta   : %lu ms", Ticks_To_Ms(Ipx.RetryDelta));
        ImGui::Text("Rtr timeout : %lu ms", Ticks_To_Ms(Ipx.Timeout));

        /**
         *  Local player ProcessTime (Session.Players[0] in vanilla).
         */
        if (Session.Players.Count() > 0 && Session.Players[0] != nullptr) {
            ImGui::Text("Process     : %d", Session.Players[0]->Player.ProcessTime);
        }

        ImGui::SeparatorText("Peers");
        const int n = Ipx.Num_Connections();
        if (n <= 0) {
            ImGui::TextDisabled("(not networked)");
            return;
        }

        if (ImGui::BeginTable("##peers", 9,
            ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Avg rtt");
            ImGui::TableSetupColumn("Max rtt");
            ImGui::TableSetupColumn("Resends");
            ImGui::TableSetupColumn("Lost");
            ImGui::TableSetupColumn("Pcnt lost");
            ImGui::TableSetupColumn("Missed o/m");
            ImGui::TableSetupColumn("Queue s/r");
            ImGui::TableSetupColumn("Process");
            ImGui::TableHeadersRow();

            for (int i = 0; i < n; ++i) {
                const IPXConnClass* conn = Ipx.Connection[i];

                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                const char* name = Ipx.Connection_Name(i);
                ImGui::TextUnformatted(name ? name : "?");

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%lu ms", Ticks_To_Ms(Ipx.Avg_Response_Time(i)));

                /**
                 *  The remaining columns come from the per-connection
                 *  ConnectionClass / CommBufferClass. Skip the row's tail if
                 *  the connection slot is null.
                 */
                if (conn != nullptr) {
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%lu ms", Ticks_To_Ms(conn->Queue ? conn->Queue->Max_Response_Time() : 0));

                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%d", conn->Num_Resends());

                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("%d", conn->Num_Lost());

                    ImGui::TableSetColumnIndex(5);
                    ImGui::Text("%d%%", conn->Percent_Lost());

                    ImGui::TableSetColumnIndex(6);
                    ImGui::Text("%d / %d", conn->Missed_Overall(), conn->Missed_Magic());

                    ImGui::TableSetColumnIndex(7);
                    if (conn->Queue) {
                        ImGui::Text("%d / %d", conn->Queue->Num_Send(), conn->Queue->Num_Receive());
                    } else {
                        ImGui::TextDisabled("-");
                    }
                }

                ImGui::TableSetColumnIndex(8);
                const int pt = Find_Peer_Process_Time(static_cast<HousesType>(i));
                if (pt >= 0) {
                    ImGui::Text("%d", pt);
                } else {
                    ImGui::TextDisabled("-");
                }
            }
            ImGui::EndTable();
        }
    }
}


void DebugOverlay::Draw()
{
    if (!IsVisible) {
        return;
    }

    if (!ImGui::Begin("Vinifera Debug", &IsVisible, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    if (!GameActive) {
        ImGui::TextDisabled("(not in a game)");
        ImGui::End();
        return;
    }

    if (ImGui::BeginTabBar("##vinifera_debug_tabs")) {

        if (ImGui::BeginTabItem("Stats")) {
            Draw_Stats_Tab();
            ImGui::EndTabItem();
        }

        if (Vinifera_DeveloperMode) {
            if (ImGui::BeginTabItem("House")) {
                Draw_House_Tab();
                ImGui::EndTabItem();
            }
        }

        if (ImGui::BeginTabItem("Unit")) {
            Draw_Unit_Tab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Network")) {
            Draw_Network_Tab();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
