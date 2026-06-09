/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Developer-mode scenario debug window.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "scenario_overlay.h"

#include "abstract.h"
#include "aitrigtype.h"
#include "base.h"
#include "buildingtype.h"
#include "extension_globals.h"
#include "house.h"
#include "housetype.h"
#include "scenario.h"
#include "scenarioext.h"
#include "script.h"
#include "scripttype.h"
#include "taction.h"
#include "tactionext.h"
#include "tactical.h"
#include "tag.h"
#include "tagtype.h"
#include "taskforce.h"
#include "team.h"
#include "teamtype.h"
#include "technotype.h"
#include "tevent.h"
#include "teventext.h"
#include "theatertype.h"
#include "tibsun_globals.h"
#include "trigger.h"
#include "triggertype.h"
#include "vinifera_defines.h"

#include <imgui.h>

#include <cstdio>
#include <cstring>
#include <vector>


bool ScenarioOverlay::IsVisible = false;


namespace
{
    /***************************************************************************
    **  Cross-reference navigation
    ***************************************************************************/


    /**
     *  Cross-reference target. Clicking "> <name>" posts a nav request;
     *  on the next frame the matching outer + inner tabs select it.
     */
    enum class NavTarget {
        None,

        // Types.
        TriggerType, TagType, TeamType, TaskForce, ScriptType, AITriggerType, HouseType,

        // Instances (no ScriptInst -- script progression lives on TeamInst).
        TriggerInst, TagInst, TeamInst, HouseInst,
    };

    struct NavRequest {
        NavTarget target = NavTarget::None;
        int index = -1;
    };
    static NavRequest PendingNavRequest;

    /***************************************************************************
    **  Back/forward history
    ***************************************************************************/


    /**
     *  Browser-style back/forward stacks of visited (pane, index) pairs.
     *  Each pane writes to CurrentNavLocation on draw; Goto/Back/Forward
     *  shuffle entries between it and the two stacks.
     */
    struct NavLocation {
        NavTarget target = NavTarget::None;
        int index = -1;
    };
    static NavLocation CurrentNavLocation;
    static std::vector<NavLocation> NavBackStack;
    static std::vector<NavLocation> NavForwardStack;

    static void Record_Current(NavTarget target, int selected)
    {
        CurrentNavLocation.target = target;
        CurrentNavLocation.index  = selected;
    }

    static void Push_Goto(NavTarget target, int index)
    {
        if (CurrentNavLocation.target != NavTarget::None) {
            NavBackStack.push_back(CurrentNavLocation);
        }
        NavForwardStack.clear();
        PendingNavRequest.target = target;
        PendingNavRequest.index  = index;
    }

    static void Go_Back()
    {
        if (NavBackStack.empty()) {
            return;
        }
        NavForwardStack.push_back(CurrentNavLocation);
        const NavLocation prev = NavBackStack.back();
        NavBackStack.pop_back();
        PendingNavRequest.target = prev.target;
        PendingNavRequest.index  = prev.index;
    }

    static void Go_Forward()
    {
        if (NavForwardStack.empty()) {
            return;
        }
        NavBackStack.push_back(CurrentNavLocation);
        const NavLocation next = NavForwardStack.back();
        NavForwardStack.pop_back();
        PendingNavRequest.target = next.target;
        PendingNavRequest.index  = next.index;
    }

    static bool Is_Type_Target(NavTarget t)
    {
        return t >= NavTarget::TriggerType && t <= NavTarget::HouseType;
    }
    static bool Is_Instance_Target(NavTarget t)
    {
        return t >= NavTarget::TriggerInst && t <= NavTarget::HouseInst;
    }

    static ImGuiTabItemFlags Tab_Flag(NavTarget want)
    {
        return PendingNavRequest.target == want ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
    }
    static ImGuiTabItemFlags Outer_Types_Flag()
    {
        return Is_Type_Target(PendingNavRequest.target)
            ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
    }
    static ImGuiTabItemFlags Outer_Instances_Flag()
    {
        return Is_Instance_Target(PendingNavRequest.target)
            ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
    }

    /**
     *  Called at the top of each pane's draw: consumes a matching nav
     *  request and records this pane as the current location.
     */
    static void Consume_Nav(NavTarget want, int& selected)
    {
        if (PendingNavRequest.target == want) {
            selected = PendingNavRequest.index;
            PendingNavRequest.target = NavTarget::None;
        }
        Record_Current(want, selected);
    }

    /**
     *  Detail row "Slot : > <name>" with a nav button, or "<none>"
     *  if `index < 0`. `width > 0` left-pads the label.
     */
    static void Goto_Row(const char* slot, const char* visible_name,
                         NavTarget target, int index, int width = 0)
    {
        /**
         *  Honor ImGui's "display##id" split: text before "##" is shown,
         *  the rest is only an ID. Full slot string goes to PushID.
         */
        char display[64];
        const char* sep = std::strstr(slot, "##");
        if (sep != nullptr) {
            const size_t n = static_cast<size_t>(sep - slot);
            const size_t copy_n = n < sizeof(display) - 1 ? n : sizeof(display) - 1;
            std::memcpy(display, slot, copy_n);
            display[copy_n] = '\0';
        } else {
            std::snprintf(display, sizeof(display), "%s", slot);
        }

        if (width > 0) {
            ImGui::Text("%-*s :", width, display);
        } else {
            ImGui::Text("%s :", display);
        }
        ImGui::SameLine();
        if (index < 0 || visible_name == nullptr) {
            ImGui::TextDisabled("<none>");
            return;
        }
        ImGui::PushID(slot);
        char buf[160];
        std::snprintf(buf, sizeof(buf), "> %s", visible_name);
        if (ImGui::SmallButton(buf)) {
            Push_Goto(target, index);
        }
        ImGui::PopID();
    }

    /**
     *  Read-only checkbox.
     */
    static void RO_Checkbox(const char* label, bool value)
    {
        bool v = value;
        ImGui::BeginDisabled();
        ImGui::Checkbox(label, &v);
        ImGui::EndDisabled();
    }


    /***************************************************************************
    **  Shared formatting helpers
    ***************************************************************************/


    static const char* Persistence_To_String(PersistantType p)
    {
        switch (p) {
        case VOLATILE:       return "Volatile";
        case SEMIPERSISTANT: return "Semi-persistent";
        case PERSISTANT:     return "Persistent";
        default:             return "?";
        }
    }

    static const char* HouseType_Name(HousesType h)
    {
        if (h < 0 || h >= HouseTypes.Count()) {
            return "<none>";
        }
        return HouseTypes[h]->Name();
    }

    static const char* StructType_Name(StructType s)
    {
        if (s < 0 || s >= BuildingTypes.Count()) {
            return "<none>";
        }
        return BuildingTypes[s]->Name();
    }

    /**
     *  "> (X, Y)" button that scrolls the tactical view to `cell`.
     *  Falls back to plain text outside a tactical map. Caller must
     *  push a unique ID if multiple buttons share coordinates in scope.
     */
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
     *  Formats a live house as "IniName (HouseType)", e.g. "Multi1 (GDI)".
     */
    static const char* House_Display_Name(const HouseClass* h, char* buf, size_t n)
    {
        if (h == nullptr) {
            std::snprintf(buf, n, "?");
        } else if (h->Class != nullptr) {
            std::snprintf(buf, n, "%s (%s)", h->IniName.c_str(), h->Class->Name());
        } else {
            std::snprintf(buf, n, "%s", h->IniName.c_str());
        }
        return buf;
    }

    /**
     *  Prefers the mapper-set Full_Name; falls back to the INI Name.
     *  Used where space is tight (listbox rows, goto buttons).
     */
    static const char* Display_Name(const AbstractTypeClass* t)
    {
        if (t == nullptr) {
            return "?";
        }
        const char* full = t->Full_Name();
        return (full != nullptr && full[0] != '\0') ? full : t->Name();
    }

    /**
     *  Two-line "INI Name / Name" header for type detail panes.
     *  `width` widens the labels to match sibling rows.
     *  Default 8 = max("INI Name", "Name", "HeapID").
     */
    static void Draw_Type_Names(const AbstractTypeClass* t, int width = 8)
    {
        ImGui::Text("%-*s : %s", width, "INI Name", t->Name());
        ImGui::Text("%-*s : %s", width, "Name",
            (t->Full_Name() && t->Full_Name()[0]) ? t->Full_Name() : "<unset>");
    }


    /***************************************************************************
    **  Master/detail helper
    ***************************************************************************/


    /**
     *  Two-column master/detail: scrolling listbox on the left,
     *  detail block for `selected` on the right.
     */
    template <typename LabelFn, typename DetailFn>
    static void Draw_List_Detail(const char* id, int count, int& selected,
                                 LabelFn label_fn, DetailFn detail_fn)
    {
        if (!ImGui::BeginTable(id, 2,
            ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV)) {
            return;
        }
        ImGui::TableSetupColumn("List",    ImGuiTableColumnFlags_WidthFixed, 220.0f);
        ImGui::TableSetupColumn("Details", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        if (ImGui::BeginListBox("##list", ImVec2(-1, -1))) {
            for (int i = 0; i < count; ++i) {
                char label[128];
                label_fn(i, label, sizeof(label));
                ImGui::PushID(i);
                if (ImGui::Selectable(label, selected == i)) {
                    selected = i;
                }
                ImGui::PopID();
            }
            ImGui::EndListBox();
        }

        ImGui::TableSetColumnIndex(1);
        if (selected >= 0 && selected < count) {
            detail_fn(selected);
        } else {
            ImGui::TextDisabled("(select an entry on the left)");
        }

        ImGui::EndTable();
    }


    /***************************************************************************
    **  Tab "Scenario"
    ***************************************************************************/


    static void Draw_Scenario_Tab()
    {
        if (Scen == nullptr) {
            ImGui::TextDisabled("(no scenario loaded)");
            return;
        }

        /* Width 13 = "Mission timer". */
        ImGui::Text("%-13s : %s", "Name",          Scen->ScenarioName);
        ImGui::Text("%-13s : %s", "Description",   Scen->Description);
        ImGui::Text("%-13s : %s", "Theater",       TheaterTypeClass::Name_From(Scen->Theater));
        ImGui::Text("%-13s : %s (computer: %s)", "Difficulty",
            Difficulty_Name(Scen->Difficulty), Difficulty_Name(Scen->CDifficulty));
        ImGui::Text("%-13s : %s", "Player house",  HouseType_Name(Scen->PlayerHouse));
        ImGui::Text("%-13s : %ld frames", "Mission timer", static_cast<long>(Scen->MissionTimer));

        /* Width 17 = "IsMultiplayerOnly". */
        ImGui::SeparatorText("Flags");
        ImGui::Text("%-17s : %s", "IsTrainCrate",      Scen->IsTrainCrate      ? "yes" : "no");
        ImGui::Text("%-17s : %s", "IsTibGrowth",       Scen->IsTibGrowth       ? "yes" : "no");
        ImGui::Text("%-17s : %s", "IsVeinGrowth",      Scen->IsVeinGrowth      ? "yes" : "no");
        ImGui::Text("%-17s : %s", "IsIceGrowth",       Scen->IsIceGrowth       ? "yes" : "no");
        ImGui::Text("%-17s : %s", "IsGlobalChanged",   Scen->IsGlobalChanged   ? "yes" : "no");
        ImGui::Text("%-17s : %s", "IsAmbientChanged",  Scen->IsAmbientChanged  ? "yes" : "no");
        ImGui::Text("%-17s : %s", "IsEndOfGame",       Scen->IsEndOfGame       ? "yes" : "no");
        ImGui::Text("%-17s : %s", "IsMultiplayerOnly", Scen->IsMultiplayerOnly ? "yes" : "no");
        ImGui::Text("%-17s : %s", "IsCratePickup",     Scen->IsCratePickup     ? "yes" : "no");

        if (ScenExtension != nullptr) {
            ImGui::Text("%-17s : %s", "IsIceDestruction", ScenExtension->IsIceDestruction ? "yes" : "no");
        }
    }


    /***************************************************************************
    **  Tab "Types" panes
    ***************************************************************************/


    /**
     *  Prints each event on a TriggerTypeClass.
     */
    static void Draw_TriggerType_Events(const TriggerTypeClass* t)
    {
        int n = 0;
        for (const TEventClass* e = t->Event; e != nullptr; e = e->Next, ++n) {
            ImGui::Text("[%d] %s  (data=%ld)", n,
                TEventClassExtension::Event_Name(e->Event), e->Data.Value);
        }
        if (n == 0) {
            ImGui::TextDisabled("(no events)");
        }
    }

    /**
     *  Prints each action with its cross-references (Tag/Trigger/Team) as goto buttons.
     */
    static void Draw_TriggerType_Actions(const TriggerTypeClass* t)
    {
        int n = 0;
        for (const TActionClass* a = t->Action; a != nullptr; a = a->Next, ++n) {
            ImGui::PushID(n);
            ImGui::Text("[%d] %s  (data=%ld)", n,
                TActionClassExtension::Action_Name(a->Action), a->Data.Value);
            ImGui::Indent();
            if (a->Tag != nullptr) {
                Goto_Row("Tag", Display_Name(a->Tag), NavTarget::TagType,
                    TagTypes.ID(a->Tag));
            }
            if (a->Trigger != nullptr) {
                Goto_Row("Trigger", Display_Name(a->Trigger), NavTarget::TriggerType,
                    TriggerTypes.ID(a->Trigger));
            }
            if (a->Team != nullptr) {
                Goto_Row("Team", Display_Name(a->Team), NavTarget::TeamType,
                    TeamTypes.ID(a->Team));
            }
            ImGui::Unindent();
            ImGui::PopID();
        }
        if (n == 0) {
            ImGui::TextDisabled("(no actions)");
        }
    }

    static void Draw_TriggerTypes_Pane()
    {
        static int selected = -1;
        Consume_Nav(NavTarget::TriggerType, selected);
        Draw_List_Detail("##trigtypes", TriggerTypes.Count(), selected,
            [](int i, char* buf, size_t n) {
                const TriggerTypeClass* t = TriggerTypes[i];
                std::snprintf(buf, n, "[%d] %s", i, Display_Name(t));
            },
            [](int i) {
                const TriggerTypeClass* t = TriggerTypes[i];
                if (!t) {
                    ImGui::TextDisabled("<null>");
                    return;
                }

                Draw_Type_Names(t);
                ImGui::Text("%-8s : %d", "HeapID", static_cast<int>(t->HeapID));
                ImGui::Separator();

                /* Width 7 = max("Enabled", "House", "Next"). */
                ImGui::Text("%-7s :", "Enabled");
                ImGui::SameLine(); RO_Checkbox("Base##e", t->Enabled);
                ImGui::SameLine(); RO_Checkbox("Easy##e", t->IsEnabledEasy);
                ImGui::SameLine(); RO_Checkbox("Med##e",  t->IsEnabledMedium);
                ImGui::SameLine(); RO_Checkbox("Hard##e", t->IsEnabledHard);

                Goto_Row("House", t->House ? t->House->Name() : nullptr,
                    NavTarget::HouseType, t->House ? HouseTypes.ID(t->House) : -1, 7);

                if (t->Next != nullptr) {
                    Goto_Row("Next", Display_Name(t->Next),
                        NavTarget::TriggerType, TriggerTypes.ID(t->Next), 7);
                }

                ImGui::SeparatorText("Events");
                Draw_TriggerType_Events(t);

                ImGui::SeparatorText("Actions");
                Draw_TriggerType_Actions(t);
            });
    }

    static void Draw_TagTypes_Pane()
    {
        static int selected = -1;
        Consume_Nav(NavTarget::TagType, selected);
        Draw_List_Detail("##tagtypes", TagTypes.Count(), selected,
            [](int i, char* buf, size_t n) {
                const TagTypeClass* t = TagTypes[i];
                std::snprintf(buf, n, "[%d] %s", i, Display_Name(t));
            },
            [](int i) {
                const TagTypeClass* t = TagTypes[i];
                if (!t) {
                    ImGui::TextDisabled("<null>");
                    return;
                }

                Draw_Type_Names(t);
                ImGui::Text("%-8s : %d", "HeapID", static_cast<int>(t->HeapID));
                ImGui::Separator();

                /* Width 11 = max("Persistence", "TriggerType"). */
                ImGui::Text("%-11s : %s (%d)", "Persistence",
                    Persistence_To_String(t->Persistence),
                    static_cast<int>(t->Persistence));
                Goto_Row("TriggerType",
                    t->TriggerType ? Display_Name(t->TriggerType) : nullptr,
                    NavTarget::TriggerType,
                    t->TriggerType ? TriggerTypes.ID(t->TriggerType) : -1, 11);
            });
    }

    static void Draw_TeamTypes_Pane()
    {
        static int selected = -1;
        Consume_Nav(NavTarget::TeamType, selected);
        Draw_List_Detail("##teamtypes", TeamTypes.Count(), selected,
            [](int i, char* buf, size_t n) {
                const TeamTypeClass* t = TeamTypes[i];
                std::snprintf(buf, n, "[%d] %s", i, Display_Name(t));
            },
            [](int i) {
                const TeamTypeClass* t = TeamTypes[i];
                if (!t) {
                    ImGui::TextDisabled("<null>");
                    return;
                }

                Draw_Type_Names(t);
                ImGui::Text("%-8s : %d", "HeapID", static_cast<int>(t->HeapID));
                ImGui::Separator();

                /**
                 *  Width 12 = max("House", "Script", "TaskForce", "Tag",
                 *  "VeteranLevel", "Group", "MaxAllowed").
                 *
                 *  TeamTypeClass.House is a *live* HouseClass, not HouseTypeClass.
                 */
                if (t->House != nullptr) {
                    char hbuf[64];
                    House_Display_Name(t->House, hbuf, sizeof(hbuf));
                    Goto_Row("House", hbuf,
                        NavTarget::HouseInst, Houses.ID(t->House), 12);
                } else {
                    ImGui::Text("%-12s : <none>", "House");
                }
                Goto_Row("Script",    t->Script    ? Display_Name(t->Script)    : nullptr,
                    NavTarget::ScriptType,
                    t->Script ? ScriptTypes.ID(t->Script) : -1, 12);
                Goto_Row("TaskForce", t->TaskForce ? Display_Name(t->TaskForce) : nullptr,
                    NavTarget::TaskForce,
                    t->TaskForce ? TaskForces.ID(t->TaskForce) : -1, 12);
                Goto_Row("Tag",       t->Tag       ? Display_Name(t->Tag)       : nullptr,
                    NavTarget::TagType,
                    t->Tag ? TagTypes.ID(t->Tag) : -1, 12);

                ImGui::Text("%-12s : %d", "VeteranLevel", t->VeteranLevel);
                ImGui::Text("%-12s : %d", "Group",        t->Group);
                ImGui::Text("%-12s : %d", "MaxAllowed",   t->MaxAllowed);

                ImGui::SeparatorText("Flags");
                RO_Checkbox("Autocreate##tt",    t->IsAutocreate);   ImGui::SameLine();
                RO_Checkbox("Reinforcable##tt",  t->IsReinforcable); ImGui::SameLine();
                RO_Checkbox("Aggressive##tt",    t->IsAggressive);
                RO_Checkbox("Suicide##tt",       t->IsSuicide);      ImGui::SameLine();
                RO_Checkbox("BaseDefense##tt",   t->IsBaseDefense);  ImGui::SameLine();
                RO_Checkbox("Prebuilt##tt",      t->IsPrebuilt);
                RO_Checkbox("Recruiter##tt",     t->IsRecruiter);    ImGui::SameLine();
                RO_Checkbox("Annoyance##tt",     t->IsAnnoyance);    ImGui::SameLine();
                RO_Checkbox("Whiner##tt",        t->IsWhiner);
            });
    }

    static void Draw_TaskForces_Pane()
    {
        static int selected = -1;
        Consume_Nav(NavTarget::TaskForce, selected);
        Draw_List_Detail("##taskforces", TaskForces.Count(), selected,
            [](int i, char* buf, size_t n) {
                const TaskForceClass* t = TaskForces[i];
                std::snprintf(buf, n, "[%d] %s", i, Display_Name(t));
            },
            [](int i) {
                const TaskForceClass* t = TaskForces[i];
                if (!t) {
                    ImGui::TextDisabled("<null>");
                    return;
                }

                Draw_Type_Names(t);
                ImGui::Separator();

                /* Width 10 = max("Group", "ClassCount"). */
                ImGui::Text("%-10s : %d", "Group",      t->Group);
                ImGui::Text("%-10s : %d", "ClassCount", t->ClassCount);
                ImGui::Separator();
                for (int m = 0; m < t->ClassCount && m < TaskForceClass::MAX_TEAM_CLASSCOUNT; ++m) {
                    const TaskForceMemberClass& mem = t->Members[m];
                    ImGui::Text("[%d] %dx %s", m, mem.Quantity,
                        mem.Class ? mem.Class->Name() : "<null>");
                }
            });
    }

    static void Draw_ScriptTypes_Pane()
    {
        static int selected = -1;
        Consume_Nav(NavTarget::ScriptType, selected);
        Draw_List_Detail("##scripttypes", ScriptTypes.Count(), selected,
            [](int i, char* buf, size_t n) {
                const ScriptTypeClass* s = ScriptTypes[i];
                std::snprintf(buf, n, "[%d] %s", i, Display_Name(s));
            },
            [](int i) {
                const ScriptTypeClass* s = ScriptTypes[i];
                if (!s) {
                    ImGui::TextDisabled("<null>");
                    return;
                }

                Draw_Type_Names(s);
                ImGui::Separator();

                /* Width 12 = max("HeapID", "MissionCount"). */
                ImGui::Text("%-12s : %d", "HeapID",       static_cast<int>(s->HeapID));
                ImGui::Text("%-12s : %d", "MissionCount", s->MissionCount);
                ImGui::Separator();
                const int show = s->MissionCount < ScriptTypeClass::MAX_SCRIPT_MISSIONS
                    ? s->MissionCount : ScriptTypeClass::MAX_SCRIPT_MISSIONS;
                for (int m = 0; m < show; ++m) {
                    const ScriptMissionClass& mc = s->MissionList[m];
                    const bool valid = mc.Mission >= SMISSION_FIRST && mc.Mission < SMISSION_COUNT;
                    ImGui::Text("[%d] %s  (data=%d)",
                        m,
                        valid ? ScriptMissionClass::Mission_Name(mc.Mission) : "<invalid>",
                        mc.Data.Value);
                }
            });
    }

    static void Draw_AITriggerTypes_Pane()
    {
        static int selected = -1;
        Consume_Nav(NavTarget::AITriggerType, selected);
        Draw_List_Detail("##aitrigtypes", AITriggerTypes.Count(), selected,
            [](int i, char* buf, size_t n) {
                const AITriggerTypeClass* a = AITriggerTypes[i];
                std::snprintf(buf, n, "[%d] %s", i, Display_Name(a));
            },
            [](int i) {
                const AITriggerTypeClass* a = AITriggerTypes[i];
                if (!a) {
                    ImGui::TextDisabled("<null>");
                    return;
                }

                Draw_Type_Names(a);
                ImGui::Separator();

                /**
                 *  Width 13 = max("Enabled", "Owner house", "TechLevel",
                 *  "Weight", "Condition obj", "Team one", "Team two",
                 *  "Roles", "Success/exec").
                 */
                ImGui::Text("%-13s :", "Enabled");
                ImGui::SameLine(); RO_Checkbox("Base##ai",  a->IsEnabled);
                ImGui::SameLine(); RO_Checkbox("Easy##ai",  a->IsEnabledInEasy);
                ImGui::SameLine(); RO_Checkbox("Med##ai",   a->IsEnabledInMedium);
                ImGui::SameLine(); RO_Checkbox("Hard##ai",  a->IsEnabledInHard);

                /**
                 *  AITriggerTypeClass.House is HousesType (== HouseTypes heap index).
                 */
                if (a->House >= 0 && a->House < HouseTypes.Count()) {
                    Goto_Row("Owner house",
                        HouseTypes[a->House]->Name(),
                        NavTarget::HouseType, a->House, 13);
                } else {
                    ImGui::Text("%-13s : <any>", "Owner house");
                }

                ImGui::Text("%-13s : %d", "TechLevel", a->TechLevel);
                ImGui::Text("%-13s : %.2f (min %.2f, max %.2f)", "Weight",
                    a->Weight, a->MinWeight, a->MaxWeight);
                ImGui::Text("%-13s : %s", "Condition obj",
                    a->ConditionObject ? a->ConditionObject->Name() : "<none>");

                Goto_Row("Team one", a->TeamTypeOne ? Display_Name(a->TeamTypeOne) : nullptr,
                    NavTarget::TeamType,
                    a->TeamTypeOne ? TeamTypes.ID(a->TeamTypeOne) : -1, 13);
                Goto_Row("Team two", a->TeamTypeTwo ? Display_Name(a->TeamTypeTwo) : nullptr,
                    NavTarget::TeamType,
                    a->TeamTypeTwo ? TeamTypes.ID(a->TeamTypeTwo) : -1, 13);

                ImGui::Text("%-13s :", "Roles");
                ImGui::SameLine(); RO_Checkbox("Skirmish##ai",    a->IsAvailableInSkirmish);
                ImGui::SameLine(); RO_Checkbox("BaseDefense##ai", a->IsForBaseDefense);

                ImGui::Text("%-13s : %d / %d", "Success/exec", a->SuccessCount, a->ExecutionCount);
            });
    }

    static void Draw_HouseTypes_Pane()
    {
        static int selected = -1;
        Consume_Nav(NavTarget::HouseType, selected);
        Draw_List_Detail("##housetypes", HouseTypes.Count(), selected,
            [](int i, char* buf, size_t n) {
                const HouseTypeClass* h = HouseTypes[i];
                std::snprintf(buf, n, "[%d] %s", i, h ? h->Name() : "<null>");
            },
            [](int i) {
                const HouseTypeClass* h = HouseTypes[i];
                if (!h) {
                    ImGui::TextDisabled("<null>");
                    return;
                }

                /* Width 6 = max("Name", "HeapID"). */
                ImGui::Text("%-6s : %s", "Name",   h->Name());
                ImGui::Text("%-6s : %d", "HeapID", static_cast<int>(h->HeapID));
                ImGui::Separator();

                /* Width 13 = max("Side", "Prefix", "FirepowerBias", "ArmorBias", "CostBias"). */
                ImGui::Text("%-13s : %d", "Side",   h->Side);
                ImGui::Text("%-13s : '%c'  Suffix: '%s'", "Prefix", h->Prefix, h->Suffix);

                RO_Checkbox("IsMultiplay##ht",        h->IsMultiplay);
                RO_Checkbox("IsMultiplayPassive##ht", h->IsMultiplayPassive);
                RO_Checkbox("IsWallOwner##ht",        h->IsWallOwner);
                RO_Checkbox("IsSmartAI##ht",          h->IsSmartAI);

                ImGui::Text("%-13s : x%.2f", "FirepowerBias", h->FirepowerBias);
                ImGui::Text("%-13s : x%.2f", "ArmorBias",     h->ArmorBias);
                ImGui::Text("%-13s : x%.2f", "CostBias",      h->CostBias);
            });
    }

    static void Draw_Types_Tab()
    {
        if (!ImGui::BeginTabBar("##type_cats")) {
            return;
        }

        if (ImGui::BeginTabItem("Triggers", nullptr, Tab_Flag(NavTarget::TriggerType))) {
            Draw_TriggerTypes_Pane();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Tags", nullptr, Tab_Flag(NavTarget::TagType))) {
            Draw_TagTypes_Pane();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Teams", nullptr, Tab_Flag(NavTarget::TeamType))) {
            Draw_TeamTypes_Pane();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("TaskForces", nullptr, Tab_Flag(NavTarget::TaskForce))) {
            Draw_TaskForces_Pane();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Scripts", nullptr, Tab_Flag(NavTarget::ScriptType))) {
            Draw_ScriptTypes_Pane();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("AITriggers", nullptr, Tab_Flag(NavTarget::AITriggerType))) {
            Draw_AITriggerTypes_Pane();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Houses", nullptr, Tab_Flag(NavTarget::HouseType))) {
            Draw_HouseTypes_Pane();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }


    /***************************************************************************
    **  Tab "Instances" panes
    ***************************************************************************/


    static void Draw_Triggers_Pane()
    {
        static int selected = -1;
        Consume_Nav(NavTarget::TriggerInst, selected);
        Draw_List_Detail("##triggers", Triggers.Count(), selected,
            [](int i, char* buf, size_t n) {
                const TriggerClass* t = Triggers[i];
                const char* nm = (t && t->Class) ? Display_Name(t->Class) : "?";
                std::snprintf(buf, n, "[%d] %s", i, nm);
            },
            [](int i) {
                const TriggerClass* t = Triggers[i];
                if (!t) {
                    ImGui::TextDisabled("<null>");
                    return;
                }

                /* Width 11 = max("INI Name", "Name", "TriggerType"). */
                if (t->Class != nullptr) {
                    Draw_Type_Names(t->Class, 11);
                }
                Goto_Row("TriggerType", t->Class ? Display_Name(t->Class) : nullptr,
                    NavTarget::TriggerType,
                    t->Class ? TriggerTypes.ID(t->Class) : -1, 11);

                ImGui::Separator();

                /* Width 18 = max("LinkedTo (trigger)", "Timer"). */
                RO_Checkbox("IsActive##trig", t->IsActive); ImGui::SameLine();
                RO_Checkbox("IsToDie##trig",  t->IsToDie);

                ImGui::Text("%-18s : %ld frames", "Timer", static_cast<long>(t->Timer));

                if (t->LinkedTo != nullptr && t->LinkedTo->Class != nullptr) {
                    Goto_Row("LinkedTo (trigger)", Display_Name(t->LinkedTo->Class),
                        NavTarget::TriggerInst, Triggers.ID(t->LinkedTo), 18);
                }

                /**
                 *  Per-event condition state, mirroring the engine's own springing
                 *  test (TriggerClass::Spring): an event counts as satisfied if it is
                 *  already latched in TrippedFlags (bit `n` = n-th event) or its live
                 *  op() currently evaluates true. Calling op() with TEVENT_ANY and null
                 *  object/source is side-effect-free and never dereferences them (every
                 *  object path is behind an `event == Event` guard), so this does not
                 *  mutate game state. Temporal events without memory (attacked, entered,
                 *  paralyzed, ...) can't be observed this way and read false except at
                 *  the instant they fire.
                 */
                ImGui::SeparatorText("Events");
                if (t->Class != nullptr) {
                    HouseClass* trig_house = t->Class->House != nullptr
                        ? House_From_HousesType(static_cast<HousesType>(t->Class->House->HeapID))
                        : nullptr;

                    int n = 0;
                    for (const TEventClass* e = t->Class->Event; e != nullptr; e = e->Next, ++n) {
                        bool fulfilled = (t->TrippedFlags & (1 << n)) != 0;
                        if (!fulfilled) {
                            bool is_perm = false;
                            CDTimerClass<FrameTimerClass> scratch = t->Timer;
                            fulfilled = const_cast<TEventClass*>(e)->operator()(
                                TEVENT_ANY, trig_house, nullptr, scratch, is_perm, nullptr);
                        }

                        char label[96];
                        std::snprintf(label, sizeof(label), "[%d] %s##evt",
                            n, TEventClassExtension::Event_Name(e->Event));
                        RO_Checkbox(label, fulfilled);
                    }
                    if (n == 0) {
                        ImGui::TextDisabled("(class has no events)");
                    }
                }
            });
    }

    static void Draw_Tags_Pane()
    {
        static int selected = -1;
        Consume_Nav(NavTarget::TagInst, selected);
        Draw_List_Detail("##tags", Tags.Count(), selected,
            [](int i, char* buf, size_t n) {
                const TagClass* t = Tags[i];
                std::snprintf(buf, n, "[%d] %s", i,
                    (t && t->Class) ? Display_Name(t->Class) : "?");
            },
            [](int i) {
                const TagClass* t = Tags[i];
                if (!t) {
                    ImGui::TextDisabled("<null>");
                    return;
                }

                /* Width 8 = max("INI Name", "Name", "TagType"). */
                if (t->Class != nullptr) {
                    Draw_Type_Names(t->Class);
                }
                Goto_Row("TagType",
                    t->Class ? Display_Name(t->Class) : nullptr,
                    NavTarget::TagType,
                    t->Class ? TagTypes.ID(t->Class) : -1, 8);

                ImGui::Separator();

                /**
                 *  Width 11 = max("Attached to", "AttachCount", "Persistence").
                 *
                 *  Get_Position() returns the cell when attached to terrain;
                 *  CELL_NONE (-1,-1) for non-cell attachments.
                 */
                const Cell pos = t->Get_Position();
                if (pos.X >= 0 && pos.Y >= 0) {
                    ImGui::Text("%-11s : cell", "Attached to");
                    ImGui::SameLine();
                    Cell_Goto_Button(pos);
                } else {
                    ImGui::Text("%-11s : object(s) (no cell)", "Attached to");
                }
                ImGui::Text("%-11s : %d", "AttachCount", t->AttachCount);

                if (t->Class != nullptr) {
                    ImGui::Text("%-11s : %s (%d)", "Persistence",
                        Persistence_To_String(t->Class->Persistence),
                        static_cast<int>(t->Class->Persistence));
                }

                RO_Checkbox("IsToDie##tag",  t->IsToDie);  ImGui::SameLine();
                RO_Checkbox("IsSprung##tag", t->IsSprung);

                /* Width 25 = max("Trigger", "TriggerType", "TriggerType (from class)"). */
                ImGui::SeparatorText("Linked trigger");
                if (t->Trigger != nullptr) {
                    Goto_Row("Trigger",
                        t->Trigger->Class ? Display_Name(t->Trigger->Class) : "?",
                        NavTarget::TriggerInst, Triggers.ID(t->Trigger), 25);

                    if (t->Trigger->Class != nullptr) {
                        Goto_Row("TriggerType", Display_Name(t->Trigger->Class),
                            NavTarget::TriggerType,
                            TriggerTypes.ID(t->Trigger->Class), 25);
                    }
                } else if (t->Class != nullptr && t->Class->TriggerType != nullptr) {
                    Goto_Row("TriggerType (from class)",
                        Display_Name(t->Class->TriggerType),
                        NavTarget::TriggerType,
                        TriggerTypes.ID(t->Class->TriggerType), 25);
                } else {
                    ImGui::TextDisabled("(no trigger linked)");
                }
            });
    }

    static void Draw_Teams_Pane()
    {
        static int selected = -1;
        Consume_Nav(NavTarget::TeamInst, selected);
        Draw_List_Detail("##teams", Teams.Count(), selected,
            [](int i, char* buf, size_t n) {
                const TeamClass* t = Teams[i];
                std::snprintf(buf, n, "[%d] %s", i,
                    (t && t->Class) ? Display_Name(t->Class) : "?");
            },
            [](int i) {
                const TeamClass* t = Teams[i];
                if (!t) {
                    ImGui::TextDisabled("<null>");
                    return;
                }

                /* Width 8 = max("INI Name", "Name", "TeamType"). */
                if (t->Class != nullptr) {
                    Draw_Type_Names(t->Class);
                }
                Goto_Row("TeamType",
                    t->Class ? Display_Name(t->Class) : nullptr,
                    NavTarget::TeamType,
                    t->Class ? TeamTypes.ID(t->Class) : -1, 8);

                ImGui::Separator();

                /* Width 8 = max("House", "Members", "Risk", "Flags", "Live tag", "Zone"). */
                char hbuf[64];
                if (t->House != nullptr) {
                    House_Display_Name(t->House, hbuf, sizeof(hbuf));
                }
                Goto_Row("House",
                    t->House ? hbuf : nullptr,
                    NavTarget::HouseInst,
                    t->House ? Houses.ID(t->House) : -1, 8);

                ImGui::Text("%-8s : %d", "Members", t->Total);
                ImGui::Text("%-8s : %d", "Risk",    t->Risk);

                ImGui::Text("%-8s :", "Flags");
                RO_Checkbox("ForcedActive##team",  t->IsForcedActive);  ImGui::SameLine();
                RO_Checkbox("FullStrength##team",  t->IsFullStrength);
                RO_Checkbox("UnderStrength##team", t->IsUnderStrength); ImGui::SameLine();
                RO_Checkbox("HasBeen##team",       t->IsHasBeen);

                if (t->Tag != nullptr) {
                    Goto_Row("Live tag",
                        t->Tag->Class ? Display_Name(t->Tag->Class) : "?",
                        NavTarget::TagInst, Tags.ID(t->Tag), 8);
                }

                /**
                 *  Zone is the AbstractClass the team centers on. Offer a jump-camera button.
                 */
                if (t->Zone != nullptr && TacticalMap != nullptr) {
                    ImGui::Text("%-8s :", "Zone");
                    ImGui::SameLine();
                    Cell_Goto_Button(t->Zone->Center_Coord().As_Cell());
                }

                /**
                 *  ScriptClass is owned by the team, so its progression
                 *  lives here rather than in a separate Scripts tab.
                 */
                if (t->Script != nullptr) {
                    ImGui::SeparatorText("Script progression");
                    /* Width 14 = max("Script type", "CurrentMission"). */
                    Goto_Row("Script type",
                        t->Script->Class ? Display_Name(t->Script->Class) : nullptr,
                        NavTarget::ScriptType,
                        t->Script->Class ? ScriptTypes.ID(t->Script->Class) : -1, 14);
                    ImGui::Text("%-14s : %d", "CurrentMission", t->Script->CurrentMission);

                    if (t->Script->Class != nullptr) {
                        const ScriptTypeClass* sc = t->Script->Class;
                        const int count = sc->MissionCount < ScriptTypeClass::MAX_SCRIPT_MISSIONS
                            ? sc->MissionCount : ScriptTypeClass::MAX_SCRIPT_MISSIONS;
                        for (int m = 0; m < count; ++m) {
                            const ScriptMissionClass& mc = sc->MissionList[m];
                            const bool current = (m == t->Script->CurrentMission);
                            if (current) {
                                ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f),
                                    "> [%d] %s  (data=%d)", m,
                                    ScriptMissionClass::Mission_Name(mc.Mission),
                                    mc.Data.Value);
                            } else {
                                ImGui::Text("  [%d] %s  (data=%d)", m,
                                    ScriptMissionClass::Mission_Name(mc.Mission),
                                    mc.Data.Value);
                            }
                        }
                    }
                }
            });
    }

    static void Draw_Houses_Pane()
    {
        static int selected = -1;
        Consume_Nav(NavTarget::HouseInst, selected);
        Draw_List_Detail("##houses", Houses.Count(), selected,
            [](int i, char* buf, size_t n) {
                const HouseClass* h = Houses[i];
                if (h == nullptr) {
                    std::snprintf(buf, n, "[%d] <null>", i);
                } else {
                    char nm[64];
                    House_Display_Name(h, nm, sizeof(nm));
                    std::snprintf(buf, n, "[%d] %s", i, nm);
                }
            },
            [](int i) {
                const HouseClass* h = Houses[i];
                if (!h) {
                    ImGui::TextDisabled("<null>");
                    return;
                }

                /* Width 9 = max("IniName", "HouseType"). */
                char name_buf[64];
                House_Display_Name(h, name_buf, sizeof(name_buf));
                ImGui::Text("%-9s : %s", "IniName", name_buf);
                Goto_Row("HouseType",
                    h->Class ? h->Class->Name() : nullptr,
                    NavTarget::HouseType,
                    h->Class ? HouseTypes.ID(h->Class) : -1, 9);

                ImGui::Separator();

                /* Width 10 = max("Credits", "Base nodes", "Allies"). */
                RO_Checkbox("IsHuman##h",         h->IsHuman);         ImGui::SameLine();
                RO_Checkbox("IsPlayerControl##h", h->IsPlayerControl);
                RO_Checkbox("IsDefeated##h",      h->IsDefeated);      ImGui::SameLine();
                RO_Checkbox("IsAlerted##h",       h->IsAlerted);

                ImGui::Text("%-10s : %ld", "Credits",    h->Credits);
                ImGui::Text("%-10s : %d",  "Base nodes", h->Base.Nodes.Count());

                /**
                 *  Allies bitmask: bit (1 << other->HeapID) set if `other` is an ally.
                 *  Decode to goto buttons; the raw mask stays above.
                 */
                ImGui::Text("%-10s : 0x%X", "Allies", h->Control.Allies);
                ImGui::Indent();
                int ally_count = 0;
                for (int oi = 0; oi < Houses.Count(); ++oi) {
                    HouseClass* other = Houses[oi];
                    if (other == nullptr) continue;
                    if ((h->Control.Allies & (1u << other->HeapID)) == 0) continue;
                    const bool self = (other == h);
                    char slot[32];
                    std::snprintf(slot, sizeof(slot), "%s##ally_%d",
                        self ? "(self)" : "ally", oi);
                    char vis[64];
                    House_Display_Name(other, vis, sizeof(vis));
                    Goto_Row(slot, vis, NavTarget::HouseInst, oi);
                    ++ally_count;
                }
                if (ally_count == 0) {
                    ImGui::TextDisabled("(no allies)");
                }
                ImGui::Unindent();
            });
    }

    static void Draw_Instances_Tab()
    {
        if (!ImGui::BeginTabBar("##inst_cats")) {
            return;
        }

        if (ImGui::BeginTabItem("Triggers", nullptr, Tab_Flag(NavTarget::TriggerInst))) {
            Draw_Triggers_Pane();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Tags", nullptr, Tab_Flag(NavTarget::TagInst))) {
            Draw_Tags_Pane();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Teams", nullptr, Tab_Flag(NavTarget::TeamInst))) {
            Draw_Teams_Pane();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Houses", nullptr, Tab_Flag(NavTarget::HouseInst))) {
            Draw_Houses_Pane();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }


    /***************************************************************************
    **  Tab "State"
    ***************************************************************************/


    static void Draw_Variables_Section()
    {
        ImGui::SeparatorText("Variables");
        if (ScenExtension == nullptr) {
            ImGui::TextDisabled("(no scenario extension)");
            return;
        }

        if (ImGui::CollapsingHeader("Locals")) {
            int shown = 0;
            for (int i = 0; i < 500; ++i) {
                const auto& v = ScenExtension->LocalFlags[i];
                if (v.VariableName[0] == '\0') {
                    continue;
                }
                ImGui::Text("[%3d] %-40s = %d", i, v.VariableName, v.Value);
                ++shown;
            }
            if (shown == 0) {
                ImGui::TextDisabled("(no locals declared)");
            }
        }

        if (ImGui::CollapsingHeader("Globals")) {
            int shown = 0;
            for (int i = 0; i < 500; ++i) {
                const auto& v = ScenExtension->GlobalFlags[i];
                if (v.VariableName[0] == '\0') {
                    continue;
                }
                ImGui::Text("[%3d] %-40s = %d", i, v.VariableName, v.Value);
                ++shown;
            }
            if (shown == 0) {
                ImGui::TextDisabled("(no globals declared)");
            }
        }
    }

    static void Draw_AI_Nodes_Section()
    {
        ImGui::SeparatorText("AI base nodes");
        for (int hi = 0; hi < Houses.Count(); ++hi) {
            HouseClass* h = Houses[hi];
            if (h == nullptr || h->Base.Nodes.Count() == 0) {
                continue;
            }

            ImGui::PushID(hi);
            char hbuf[64];
            House_Display_Name(h, hbuf, sizeof(hbuf));
            if (ImGui::TreeNode("##house", "%s (%d nodes)",
                hbuf, h->Base.Nodes.Count()))
            {
                for (int ni = 0; ni < h->Base.Nodes.Count(); ++ni) {
                    const BaseNodeClass& n = h->Base.Nodes[ni];
                    ImGui::PushID(ni);
                    ImGui::Text("[%d] %s @", ni, StructType_Name(n.Type));
                    ImGui::SameLine();
                    Cell_Goto_Button(n.CellID);
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
    }

    static void Draw_State_Tab()
    {
        Draw_Variables_Section();
        Draw_AI_Nodes_Section();
    }
}


void ScenarioOverlay::Draw()
{
    if (!IsVisible) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(720, 540), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Scenario", &IsVisible)) {
        ImGui::End();
        return;
    }

    if (!GameActive) {
        ImGui::TextDisabled("(not in a game)");
        ImGui::End();
        return;
    }

    ImGui::BeginDisabled(NavBackStack.empty());
    if (ImGui::Button("<- Back")) {
        Go_Back();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(NavForwardStack.empty());
    if (ImGui::Button("Forward ->")) {
        Go_Forward();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::TextDisabled("(%d back / %d fwd)",
        static_cast<int>(NavBackStack.size()), static_cast<int>(NavForwardStack.size()));

    if (ImGui::BeginTabBar("##vinifera_scenario_tabs")) {

        if (ImGui::BeginTabItem("Scenario")) {
            Draw_Scenario_Tab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Types", nullptr, Outer_Types_Flag())) {
            Draw_Types_Tab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Instances", nullptr, Outer_Instances_Flag())) {
            Draw_Instances_Tab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("State")) {
            Draw_State_Tab();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
