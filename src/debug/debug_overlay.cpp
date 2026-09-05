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
#include "extension.h"

#include "aircraft.h"
#include "anim.h"
#include "armortype.h"
#include "building.h"
#include "bullet.h"
#include "event.h"
#include "extension.h"
#include "factory.h"
#include "foot.h"
#include "house.h"
#include "houseext.h"
#include "housetype.h"
#include "infantry.h"
#include "mission.h"
#include "mouse.h"
#include "net/combuf.h"
#include "net/ipxconn.h"
#include "net/ipxmgr.h"
#include "object.h"
#include "objecttype.h"
#include "overlay.h"
#include "particle.h"
#include "queue.h"
#include "rules.h"
#include "session.h"
#include "smudge.h"
#include "tactical.h"
#include "tag.h"
#include "tagtype.h"
#include "team.h"
#include "teamtype.h"
#include "techno.h"
#include "technoext.h"
#include "technotype.h"
#include "terrain.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "trigger.h"
#include "triggertype.h"
#include "unit.h"
#include "verses.h"
#include "veterancy.h"
#include "vinifera_defines.h"
#include "vinifera_globals.h"
#include "voxelanim.h"
#include "weapontype.h"
#include "xmouse.h"

#include <imgui.h>

#include <cfloat>
#include <cmath>
#include <cstdio>


bool DebugOverlay::IsVisible = false;


namespace
{
    /**
     *  Owner of the first selected object, or the local player.
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
        const int total_seconds = Frame / 15; // 15 logic ticks/sec
        const int hours = total_seconds / 3600;
        const int minutes = (total_seconds / 60) % 60;
        const int seconds = total_seconds % 60;
        std::snprintf(out, out_size, "%d:%02d:%02d", hours, minutes, seconds);
    }

    
    static void Cell_Goto_Button(Cell cell)
    {
        if (TacticalMap == nullptr) {
            ImGui::Text("(%d, %d)", cell.X, cell.Y);
            return;
        }
        char buf[64];
        std::snprintf(buf, sizeof(buf), "> (%d, %d)", cell.X, cell.Y);
        if (ImGui::SmallButton(buf)) {
            TacticalMap->Set_Tactical_Position(cell.As_Coord());
        }
    }

    /**
     *  Performance block; in dev mode adds heap / queue counts.
     *
     *  @author: ZivDero
     */
    static void Draw_General_Tab()
    {
        char timebuf[16];
        Format_Mission_Time(timebuf, sizeof(timebuf));

        ImGui::Text("Time  : %s", timebuf);
        ImGui::Text("FPS   : %u", FramesPerSecond);
        ImGui::Text("Frame : %ld", Frame);

        if (!Vinifera_DeveloperMode) {
            return;
        }

        /**
         *  Live instance heaps; static type heaps (TeamTypes etc.) are omitted.
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

        Cell cursor_map_cell = TacticalMap->Click_Cell_Calc(MouseCursor->Get_Mouse_Point());
        if (cursor_map_cell != CELL_NONE)
        {
            CellClass& cell = Map[cursor_map_cell];
            ImGui::SeparatorText("Cell");
            ImGui::Text("Owner      : %d (%s)", (int)cell.Owner, cell.Owner == HOUSE_NONE ? "<none>" : Houses[cell.Owner]->Class->IniName.c_str());
            ImGui::Text("Land       : %s", Name_From_Land(cell.Land));
            ImGui::Text("Position   : %d,%d", cursor_map_cell.X, cursor_map_cell.Y);
            
            auto cell_object = cell.Cell_Object();
            if (cell_object != nullptr) {
                ImGui::Text("Cell Object: %s (%s)", cell_object->Full_Name(), cell_object->Name());
            } else {
                ImGui::Text("Cell Object: <none>");
            }
        }

        /**
         *  Multiplayer sync queues. OutList = outbound events,
         *  DoList = events to apply this frame.
         */
        ImGui::SeparatorText("Queues");
        ImGui::Text("OutList    : %d", OutList.Count);
        ImGui::Text("DoList     : %d", DoList.Count);
    }

    static void Draw_Factory_Line(const char* label, FactoryClass* factory)
    {
        if (factory == nullptr) {
            ImGui::Text("%-12s: (idle)", label);
            return;
        }
        const TechnoClass* obj = factory->Get_Object();
        const char* name = obj && obj->Class_Of() ? obj->Class_Of()->Full_Name() : "Unknown object";
        int stage_percent = std::round(double(factory->Fetch_Stage()) / FactoryClass::FactoryClassEnum::STEP_COUNT * 100);
        ImGui::Text("%-12s: %s (%d%%)", label, name, stage_percent);
    }


    /**
     *  State dump for the owner of the selected object (falls back to PlayerPtr).
     */
    static void Draw_House_Tab()
    {
        HouseClass* house = Get_Display_House();
        if (house == nullptr) {
            ImGui::TextDisabled("(no house)");
            return;
        }

        ImGui::Text("House    : %s (%s) [#%d]", Session.Type == GAME_NORMAL && house == PlayerPtr 
            ? "You" : house->IniName.c_str(),
            house->Class->IniName.c_str(), house->HeapID);
        ImGui::Separator();

        ImGui::Text("Total Credits    : %ld", house->Available_Money());
        ImGui::Text("Cash Reserves    : %ld", house->Credits);
        ImGui::Text("Stored Tiberium  : %ld", house->Tiberium.Get_Total_Value());
        ImGui::Text("Storage Capacity : %ld / %ld", house->Capacity - house->Available_Storage(), house->Capacity);

        const long power = house->Power_Output();
        const long drain = house->Power_Drain();
        const bool low_power = drain > power;
        
        if (low_power) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Power            : %ld / %ld (LOW)", drain, power);
        } else {
            ImGui::Text("Power            : %ld / %ld", drain, power);
        }

        ImGui::SeparatorText("Production");
        Draw_Factory_Line("Building", house->BuildingFactory);
        Draw_Factory_Line("Infantry", house->InfantryFactory);
        Draw_Factory_Line("Unit",     house->UnitFactory);
        Draw_Factory_Line("Aircraft", house->AircraftFactory);

        FactoryClass* naval_factory = Extension::Fetch(house)->NavalFactory;
        Draw_Factory_Line("Naval", naval_factory);

        ImGui::SeparatorText("Profile");
        ImGui::Text("TechLevel  : %d", house->Control.TechLevel);
        ImGui::Text("IQ         : %d", house->IQ);
        ImGui::Text("Difficulty : %s", Difficulty_Name(house->Difficulty));

        ImGui::SeparatorText("Biases");
        ImGui::Text("Firepower    : x%.2f", house->FirepowerBias);
        ImGui::Text("Armor        : x%.2f", house->ArmorBias);
        ImGui::Text("ROF          : x%.2f", house->ROFBias);
        ImGui::Text("Ground Speed : x%.2f", house->GroundspeedBias);
        ImGui::Text("Air Speed    : x%.2f", house->AirspeedBias);
        ImGui::Text("Cost         : x%.2f", house->CostBias);
        ImGui::Text("Build Speed  : x%.2f", house->BuildSpeedBias);

        ImGui::SeparatorText("Losses");
        ImGui::Text("Units lost      : %u", house->UnitsLost);
        ImGui::Text("Buildings lost  : %u", house->BuildingsLost);

        if (ImGui::CollapsingHeader("Flags")) {
            ImGui::Text("IsHuman         : %s", house->IsHuman ? "yes" : "no");
            ImGui::Text("IsPlayerControl : %s", house->IsPlayerControl ? "yes" : "no");
            ImGui::Text("IsAlerted       : %s", house->IsAlerted ? "yes" : "no");
            ImGui::Text("IsDefeated      : %s", house->IsDefeated ? "yes" : "no");
            ImGui::Text("IsToDie         : %s", house->IsToDie ? "yes" : "no");
            ImGui::Text("IsToWin         : %s", house->IsToWin ? "yes" : "no");
            ImGui::Text("IsToLose        : %s", house->IsToLose ? "yes" : "no");
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
     *  Per-weapon stats. ROF is ticks between shots at 15 ticks/sec,
     *  so DPS = Attack * Burst * 15 / ROF.
     */
    static void Draw_Weapon_Block(const char* label, const WeaponTypeClass* w, double firepower_bias)
    {
        if (w == nullptr) {
            ImGui::Text("%s : (none)", label);
            return;
        }

        /**
         *  Scope IDs so Primary and Secondary blocks don't collide on shared child labels.
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
     *  Stats panel for the selected unit.
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
         *  Redact enemy unit details outside dev mode.
         */
        if (is_enemy && !Vinifera_DeveloperMode) {
            ImGui::TextDisabled("Enemy Unit Selected");
            return;
        }

        ImGui::Text("Object   : %s (%s)", type->Full_Name(), type->Name());

        /**
         *  Owner is shown as "<house type> (<who controls it>)". The control tag
         *  distinguishes the local player, other humans (by nickname in MP), and the AI
         *  (by alliance in SP). HouseClass::IniName is the player's nickname only in MP;
         *  in SP it stays the bogus "Computer" default, so it is only used for humans.
         */
        char owner_buf[64];
        if (owner == nullptr) {
            std::snprintf(owner_buf, sizeof(owner_buf), "(none)");
        } else {
            const char* house_name = owner->Class != nullptr
                ? (owner->Class->Full_Name()[0] ? owner->Class->Full_Name() : owner->Class->Name())
                : "?";

            const char* tag;
            if (owner->Is_Player_Control()) {
                tag = "You";
            } else if (owner->Is_Human_Player()) {
                tag = owner->IniName.c_str();
            } else if (Session.Singleplayer_Game()) {
                tag = (PlayerPtr != nullptr && PlayerPtr->Is_Ally(owner)) ? "Ally" : "Enemy";
            } else {
                tag = "Computer";
            }

            std::snprintf(owner_buf, sizeof(owner_buf), "%s (%s)", house_name, tag);
        }
        ImGui::Text("Owner    : %s", owner_buf);

        ImGui::Text("HP       : %d / %d", obj->Strength, type->MaxStrength);

        const Cell cell = obj->Get_Cell();
        ImGui::Text("Cell     :");
        ImGui::SameLine();
        Cell_Goto_Button(cell);        

        if (!obj->Is_Techno()) {
            return;
        }

        TechnoClass* techno = static_cast<TechnoClass*>(obj);
        auto techno_type_ext = Extension::Fetch(techno);

        ImGui::Text("Sight    : %d", techno_type_ext->Get_Sight_Range());

        ImGui::Text("Mission  : %s", MissionClass::Mission_Name(techno->Get_Mission()));

        /**
         *  Group is 0-9 for Ctrl+# groups, otherwise -1 (0xFF as unsigned).
         */
        const int group = static_cast<int>(techno->Group);
        if (group >= 0 && group <= 9) {
            ImGui::Text("Group    : %d", group == 9 ? 0 : group + 1); // align with actual group view rather than internal group number
            if (Vinifera_DeveloperMode) {
                ImGui::Text("Group ID : %d", group);
            }
        } else {
            ImGui::Text("Group    : (none)");

            if (Vinifera_DeveloperMode) {
                ImGui::Text("Group ID : (none)");
            }
        }

        /**
         *  Experience is normalized: [0,1) Rookie, [1,2) Veteran, >=2 Elite. One rank is
         *  worth Cost_Of(owner) * VeteranRatio of destroyed value, so the fractional part
         *  scaled by that gives a credits-style "XP toward next rank".
         */
        const double exp = techno->Crew.Get_Experience();
        const VeterancyRankType rank = techno->Crew.Get_Rank();
        ImGui::Text("Rank     : %s", Rank_To_String(rank));

        const TechnoTypeClass* ttype = techno->Techno_Type_Class();
        const double per_level = ttype != nullptr ? ttype->Cost_Of(owner) * Rule->VeteranRatio : 0.0;
        if (rank == RANK_ELITE || per_level <= 0.0) {
            ImGui::Text("XP       : (max rank)");
        } else {
            const double frac = exp - std::floor(exp);
            ImGui::Text("XP       : %d / %d (%d%%)",
                static_cast<int>(frac * per_level + 0.5), 
                static_cast<int>(per_level + 0.5), 
                static_cast<int>(std::round(frac * 100)));
        }

        /**
         *  FootClass speed bias stacks on top of the house ground bias.
         */
        const bool is_foot = obj->RTTI == RTTI_UNIT || obj->RTTI == RTTI_INFANTRY || obj->RTTI == RTTI_AIRCRAFT;
        if (is_foot) {
            FootClass* foot = static_cast<FootClass*>(obj);
            ImGui::Text("Speed    : %d (bias x%.2f)", foot->Locomotion->Apparent_Speed(), foot->SpeedBias);

            if (foot->NavCom == nullptr) {
                ImGui::Text("NavCom   : <none>");
            } else {
                Cell navcom_cell = foot->NavCom->Center_Coord().As_Cell();
                bool is_navcom_techno = Is_Techno(foot->NavCom);
                std::string text;
                if (is_navcom_techno) {
                    auto navcom_techno = static_cast<TechnoClass*>(foot->NavCom);
                    text = std::format("{} ({})", navcom_techno->Full_Name(), navcom_techno->Name());
                } else {
                    text = Name_From_RTTI(foot->NavCom->RTTI);
                }

                ImGui::Text("NavCom   : %s", text.c_str());
                ImGui::SameLine();
                Cell_Goto_Button(navcom_cell);
            }
        }

        if (techno->TarCom == nullptr) {
            ImGui::Text("TarCom   : <none>");
        } else {
            Cell tarcom_cell = techno->TarCom->Center_Coord().As_Cell();
            bool is_tarcom_techno = Is_Techno(techno->TarCom);
            std::string text;
            if (is_tarcom_techno) {
                auto tarcom_techno = static_cast<TechnoClass*>(techno->TarCom);
                text = std::format("{} ({})", tarcom_techno->Full_Name(), tarcom_techno->Name());
            } else {
                text = Name_From_RTTI(techno->TarCom->RTTI);
            }

            ImGui::Text("TarCom   : %s", text.c_str());
            ImGui::SameLine();
            Cell_Goto_Button(tarcom_cell);
        }

        /**
         *  Scenario bindings, shown only when attached.
         *  Team is foot-only; Tag is on any object.
         */
        TeamClass* team = is_foot ? static_cast<FootClass*>(obj)->Team : nullptr;
        TagClass* tag = obj->Tag;
        if (team != nullptr || tag != nullptr) {
            ImGui::SeparatorText("Scenario");

            if (team != nullptr) {
                ImGui::Text("Team    : %s", team->Name());
                if (ImGui::TreeNode("Team details")) {
                    ImGui::Text("Team Name     : %s", team->Class->Full_Name());
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
                        ImGui::Text("Trigger      : %s", tagtype->TriggerType->Name());
                        ImGui::Text("Trigger Name : %s", tagtype->TriggerType->Full_Name());
                    }

                    if (tagtype != nullptr) {
                        ImGui::Text("Tag Name     : %s", tagtype->Full_Name());
                        ImGui::Text("Persistence  : %s (%d)", Persistence_To_String(tagtype->Persistence), tagtype->Persistence);
                    }

                    ImGui::Text("AttachCount  : %d", tag->AttachCount);
                    ImGui::Text("IsSprung     : %s", tag->IsSprung ? "yes" : "no");
                    ImGui::Text("IsToDie      : %s", tag->IsToDie ? "yes" : "no");
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
        
        ImGui::SeparatorText("Armor");
        const ArmorType armor = type->Armor;
        bool is_valid_armor = armor >= ARMOR_FIRST && armor < ArmorTypes.Count();
        const char* armor_name = is_valid_armor ? ArmorTypes[armor]->Name() : "Unknown armor";
        ImGui::Text("Armor   : %s", armor_name);

        if (is_valid_armor) {
            if (ImGui::TreeNode("Default Armor Values")) {
                ImGui::Text("PassiveAcquire : %s", ArmorTypes[armor]->PassiveAcquire ? "yes" : "no");
                ImGui::Text("ForceFire      : %s", ArmorTypes[armor]->ForceFire ? "yes" : "no");
                ImGui::Text("Retaliate      : %s", ArmorTypes[armor]->Retaliate ? "yes" : "no");
                ImGui::TreePop();
            }
        }
    }


    /**
     *  TS network timings are 1/60-second ticks.
     */
    static unsigned long Ticks_To_Ms(unsigned long ticks)
    {
        return ticks * 1000UL / 60UL;
    }


    /**
     *  Looks up a peer's ProcessTime by player ID (== HousesType).
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
     *  Sync/latency on top, per-peer rtt/process below.
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
         *  Local player ProcessTime; Session.Players[0] in vanilla.
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
                 *  Remaining columns need a connection slot; skip them if null.
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

    // minimum width of 360, no max width/height
    ImGui::SetNextWindowSizeConstraints(ImVec2(360.0f, 0.0f), ImVec2(FLT_MAX, FLT_MAX));
    if (!ImGui::Begin("Game Info", &IsVisible, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    if (!GameActive) {
        ImGui::TextDisabled("(not in a game)");
        ImGui::End();
        return;
    }

    if (ImGui::BeginTabBar("##vinifera_debug_tabs")) {

        if (ImGui::BeginTabItem("General")) {
            Draw_General_Tab();
            ImGui::EndTabItem();
        }

        if (Vinifera_DeveloperMode) {
            if (ImGui::BeginTabItem("House")) {
                Draw_House_Tab();
                ImGui::EndTabItem();
            }
        }

        if (ImGui::BeginTabItem("Selected Object")) {
            Draw_Unit_Tab();
            ImGui::EndTabItem();
        }

        /**
         *  The Network tab only carries meaningful data in a networked game.
         */
        if (!Session.Singleplayer_Game()) {
            if (ImGui::BeginTabItem("Network")) {
                Draw_Network_Tab();
                ImGui::EndTabItem();
            }
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
