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
#include "abstract.h"
#include "taction.h"
#include "tactical.h"
#include "tag.h"
#include "tagtype.h"
#include "taskforce.h"
#include "team.h"
#include "teamtype.h"
#include "technotype.h"
#include "tevent.h"
#include "tibsun_globals.h"
#include "trigger.h"
#include "triggertype.h"

#include <imgui.h>

#include <cstdio>
#include <cstring>


bool ScenarioOverlay::IsVisible = false;


namespace
{
    /*
    ** ---------------- cross-reference navigation ----------------
    **
    ** Detail panes show many references (a TeamType's Script, a Tag's
    ** Trigger, etc.). Clicking the "› <name>" button next to such a
    ** reference posts a nav request; on the next frame, the matching
    ** outer-tab + inner-tab + pane select the requested entry.
    */
    enum class NavTarget {
        None,
        // Types
        TriggerType, TagType, TeamType, TaskForce, ScriptType, AITriggerType, HouseType,
        // Instances (no ScriptInst -- script progression lives on TeamInst)
        TriggerInst, TagInst, TeamInst, HouseInst,
    };

    struct NavRequest {
        NavTarget target = NavTarget::None;
        int index = -1;
    };
    static NavRequest g_nav;

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
        return g_nav.target == want ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
    }
    static ImGuiTabItemFlags Outer_Types_Flag()
    {
        return Is_Type_Target(g_nav.target)
            ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
    }
    static ImGuiTabItemFlags Outer_Instances_Flag()
    {
        return Is_Instance_Target(g_nav.target)
            ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
    }

    /**
     *  Each pane calls this at the top to consume a matching nav request.
     */
    static void Consume_Nav(NavTarget want, int& selected)
    {
        if (g_nav.target == want) {
            selected = g_nav.index;
            g_nav.target = NavTarget::None;
        }
    }

    /**
     *  Renders one row of a detail panel showing "Slot: › <name>" with a
     *  navigation button. If `index < 0` (the referenced entry doesn't
     *  exist), shows "<none>" instead.
     */
    static void Goto_Row(const char* slot, const char* visible_name,
                         NavTarget target, int index)
    {
        /*
        ** Honor the ImGui "display##id" split: everything before "##" is the
        ** visible label, the rest is just an ID disambiguator. The full slot
        ** string is fed to PushID so identical visible labels stay unique.
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

        ImGui::Text("%s:", display);
        ImGui::SameLine();
        if (index < 0 || visible_name == nullptr) {
            ImGui::TextDisabled("<none>");
            return;
        }
        ImGui::PushID(slot);
        char buf[160];
        std::snprintf(buf, sizeof(buf), "› %s", visible_name);
        if (ImGui::SmallButton(buf)) {
            g_nav.target = target;
            g_nav.index = index;
        }
        ImGui::PopID();
    }

    /**
     *  Read-only checkbox: shows the bool state but can't be toggled.
     */
    static void RO_Checkbox(const char* label, bool value)
    {
        bool v = value;
        ImGui::BeginDisabled();
        ImGui::Checkbox(label, &v);
        ImGui::EndDisabled();
    }


    /*
    ** ---------------- vector lookup helpers ----------------
    */
    template <typename V, typename T>
    static int Find_Index(const V& vec, const T* needle)
    {
        if (needle == nullptr) return -1;
        for (int i = 0; i < vec.Count(); ++i) {
            if (vec[i] == needle) return i;
        }
        return -1;
    }


    /*
    ** ---------------- shared formatting helpers ----------------
    */

    static const char* Theater_To_String(TheaterType t)
    {
        switch (t) {
        case THEATER_NONE:      return "None";
        case THEATER_TEMPERATE: return "Temperate";
        case THEATER_SNOW:      return "Snow";
        default:                return "?";
        }
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


    /*
    ** ---------------- master/detail helper ----------------
    **
    ** Renders a two-column layout: a scrolling list of entries on the left,
    ** and the detail block for the selected entry on the right. The owner
    ** keeps a `selected` index in a static so the selection persists between
    ** frames.
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


    /*
    ** ---------------- Tab "Scenario" ----------------
    */
    static void Draw_Scenario_Tab()
    {
        if (Scen == nullptr) {
            ImGui::TextDisabled("(no scenario loaded)");
            return;
        }

        ImGui::Text("Name        : %s", Scen->ScenarioName);
        ImGui::Text("Description : %s", Scen->Description);
        ImGui::Text("Theater     : %s", Theater_To_String(Scen->Theater));
        ImGui::Text("Difficulty  : %s (computer: %s)",
            Diff_To_String(Scen->Difficulty), Diff_To_String(Scen->CDifficulty));
        ImGui::Text("Player house: %s", HouseType_Name(Scen->PlayerHouse));
        ImGui::Text("Mission timer: %ld frames", static_cast<long>(Scen->MissionTimer));

        ImGui::SeparatorText("Flags");
        ImGui::Text("IsTrainCrate     : %s", Scen->IsTrainCrate ? "yes" : "no");
        ImGui::Text("IsTibGrowth      : %s", Scen->IsTibGrowth ? "yes" : "no");
        ImGui::Text("IsVeinGrowth     : %s", Scen->IsVeinGrowth ? "yes" : "no");
        ImGui::Text("IsIceGrowth      : %s", Scen->IsIceGrowth ? "yes" : "no");
        ImGui::Text("IsGlobalChanged  : %s", Scen->IsGlobalChanged ? "yes" : "no");
        ImGui::Text("IsAmbientChanged : %s", Scen->IsAmbientChanged ? "yes" : "no");
        ImGui::Text("IsEndOfGame      : %s", Scen->IsEndOfGame ? "yes" : "no");
        ImGui::Text("IsMultiplayerOnly: %s", Scen->IsMultiplayerOnly ? "yes" : "no");
        ImGui::Text("IsCratePickup    : %s", Scen->IsCratePickup ? "yes" : "no");

        if (ScenExtension != nullptr) {
            ImGui::Text("IsIceDestruction : %s", ScenExtension->IsIceDestruction ? "yes" : "no");
        }
    }


    /*
    ** ---------------- Tab "Types" panes ----------------
    */
    /**
     *  Walks the linked-list of events on a TriggerTypeClass and prints each.
     *  Returns the number of events listed (caller uses this to decode the
     *  TrippedFlags bitmask on TriggerClass instances).
     */
    static int Draw_TriggerType_Events(const TriggerTypeClass* t)
    {
        int n = 0;
        for (const TEventClass* e = t->Event; e != nullptr; e = e->Next, ++n) {
            ImGui::Text("[%d] %s  (data=%ld)", n,
                TEventClass::Event_Name(e->Event), e->Data.Value);
        }
        if (n == 0) {
            ImGui::TextDisabled("(no events)");
        }
        return n;
    }

    /**
     *  Walks the linked-list of actions and prints each, including any
     *  cross-references (Tag, Trigger, Team) as goto buttons.
     */
    static void Draw_TriggerType_Actions(const TriggerTypeClass* t)
    {
        int n = 0;
        for (const TActionClass* a = t->Action; a != nullptr; a = a->Next, ++n) {
            ImGui::PushID(n);
            ImGui::Text("[%d] %s  (data=%ld)", n,
                TActionClass::Action_Name(a->Action), a->Data.Value);
            ImGui::Indent();
            if (a->Tag != nullptr) {
                Goto_Row("Tag", a->Tag->Name(), NavTarget::TagType,
                    Find_Index(TagTypes, a->Tag));
            }
            if (a->Trigger != nullptr) {
                Goto_Row("Trigger", a->Trigger->Name(), NavTarget::TriggerType,
                    Find_Index(TriggerTypes, a->Trigger));
            }
            if (a->Team != nullptr) {
                Goto_Row("Team", a->Team->Name(), NavTarget::TeamType,
                    Find_Index(TeamTypes, a->Team));
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
                std::snprintf(buf, n, "[%d] %s", i, t ? t->Name() : "<null>");
            },
            [](int i) {
                const TriggerTypeClass* t = TriggerTypes[i];
                if (!t) { ImGui::TextDisabled("<null>"); return; }

                ImGui::Text("Name   : %s", t->Name());
                ImGui::Text("HeapID : %d", static_cast<int>(t->HeapID));
                ImGui::Separator();

                ImGui::Text("Enabled:");
                ImGui::SameLine(); RO_Checkbox("Base##e", t->Enabled);
                ImGui::SameLine(); RO_Checkbox("Easy##e", t->IsEnabledEasy);
                ImGui::SameLine(); RO_Checkbox("Med##e",  t->IsEnabledMedium);
                ImGui::SameLine(); RO_Checkbox("Hard##e", t->IsEnabledHard);

                Goto_Row("House", t->House ? t->House->Name() : nullptr,
                    NavTarget::HouseType, t->House ? Find_Index(HouseTypes, t->House) : -1);

                if (t->Next != nullptr) {
                    Goto_Row("Next", t->Next->Name(),
                        NavTarget::TriggerType, Find_Index(TriggerTypes, t->Next));
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
                std::snprintf(buf, n, "[%d] %s", i, t ? t->Name() : "<null>");
            },
            [](int i) {
                const TagTypeClass* t = TagTypes[i];
                if (!t) { ImGui::TextDisabled("<null>"); return; }

                ImGui::Text("Name        : %s", t->Name());
                ImGui::Text("HeapID      : %d", static_cast<int>(t->HeapID));
                ImGui::Separator();
                ImGui::Text("Persistence : %s (%d)",
                    Persistence_To_String(t->Persistence),
                    static_cast<int>(t->Persistence));
                Goto_Row("TriggerType", t->TriggerType ? t->TriggerType->Name() : nullptr,
                    NavTarget::TriggerType,
                    t->TriggerType ? Find_Index(TriggerTypes, t->TriggerType) : -1);
            });
    }

    static void Draw_TeamTypes_Pane()
    {
        static int selected = -1;
        Consume_Nav(NavTarget::TeamType, selected);
        Draw_List_Detail("##teamtypes", TeamTypes.Count(), selected,
            [](int i, char* buf, size_t n) {
                const TeamTypeClass* t = TeamTypes[i];
                std::snprintf(buf, n, "[%d] %s", i, t ? t->Name() : "<null>");
            },
            [](int i) {
                const TeamTypeClass* t = TeamTypes[i];
                if (!t) { ImGui::TextDisabled("<null>"); return; }

                ImGui::Text("Name         : %s", t->Name());
                ImGui::Text("HeapID       : %d", static_cast<int>(t->HeapID));
                ImGui::Separator();

                /*
                ** HouseClass references on TeamTypeClass point at the *live*
                ** house instance, not the HouseTypeClass -- use the
                ** instance-side nav target.
                */
                if (t->House != nullptr) {
                    Goto_Row("House",     t->House->IniName.c_str(),
                        NavTarget::HouseInst, Find_Index(Houses, t->House));
                } else {
                    ImGui::TextDisabled("House: <none>");
                }
                Goto_Row("Script",    t->Script    ? t->Script->Name()    : nullptr,
                    NavTarget::ScriptType,
                    t->Script ? Find_Index(ScriptTypes, t->Script) : -1);
                Goto_Row("TaskForce", t->TaskForce ? t->TaskForce->Name() : nullptr,
                    NavTarget::TaskForce,
                    t->TaskForce ? Find_Index(TaskForces, t->TaskForce) : -1);
                Goto_Row("Tag",       t->Tag       ? t->Tag->Name()       : nullptr,
                    NavTarget::TagType,
                    t->Tag ? Find_Index(TagTypes, t->Tag) : -1);

                ImGui::Text("VeteranLevel : %d", t->VeteranLevel);
                ImGui::Text("Group        : %d", t->Group);
                ImGui::Text("MaxAllowed   : %d", t->MaxAllowed);

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
                std::snprintf(buf, n, "[%d] %s", i, t ? t->Name() : "<null>");
            },
            [](int i) {
                const TaskForceClass* t = TaskForces[i];
                if (!t) { ImGui::TextDisabled("<null>"); return; }

                ImGui::Text("Name       : %s", t->Name());
                ImGui::Text("Group      : %d", t->Group);
                ImGui::Text("ClassCount : %d", t->ClassCount);
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
                std::snprintf(buf, n, "[%d] %s", i, s ? s->Name() : "<null>");
            },
            [](int i) {
                const ScriptTypeClass* s = ScriptTypes[i];
                if (!s) { ImGui::TextDisabled("<null>"); return; }

                ImGui::Text("Name         : %s", s->Name());
                ImGui::Text("HeapID       : %d", static_cast<int>(s->HeapID));
                ImGui::Text("MissionCount : %d", s->MissionCount);
                ImGui::Separator();
                const int show = s->MissionCount < ScriptTypeClass::MAX_SCRIPT_MISSIONS
                    ? s->MissionCount : ScriptTypeClass::MAX_SCRIPT_MISSIONS;
                for (int m = 0; m < show; ++m) {
                    const ScriptMissionClass& mc = s->MissionList[m];
                    ImGui::Text("[%d] %s  (data=%d)",
                        m,
                        ScriptMissionClass::Mission_Name(mc.Mission),
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
                std::snprintf(buf, n, "[%d] %s", i, a ? a->Name() : "<null>");
            },
            [](int i) {
                const AITriggerTypeClass* a = AITriggerTypes[i];
                if (!a) { ImGui::TextDisabled("<null>"); return; }

                ImGui::Text("Name           : %s", a->Name());
                ImGui::Separator();

                ImGui::Text("Enabled:");
                ImGui::SameLine(); RO_Checkbox("Base##ai",  a->IsEnabled);
                ImGui::SameLine(); RO_Checkbox("Easy##ai",  a->EnabledInEasy);
                ImGui::SameLine(); RO_Checkbox("Med##ai",   a->EnabledInMedium);
                ImGui::SameLine(); RO_Checkbox("Hard##ai",  a->EnabledInHard);

                /*
                ** AITriggerTypeClass.House is the HousesType enum (which is
                ** also the HouseTypes heap index).
                */
                if (a->House >= 0 && a->House < HouseTypes.Count()) {
                    Goto_Row("Owner house",
                        HouseTypes[a->House]->Name(),
                        NavTarget::HouseType, a->House);
                } else {
                    ImGui::TextDisabled("Owner house: <any>");
                }

                ImGui::Text("TechLevel      : %d", a->TechLevel);
                ImGui::Text("Weight         : %.2f (min %.2f, max %.2f)",
                    a->Weight, a->MinWeight, a->MaxWeight);
                ImGui::Text("Condition obj  : %s",
                    a->ConditionObject ? a->ConditionObject->Name() : "<none>");

                Goto_Row("Team one", a->TeamOne ? a->TeamOne->Name() : nullptr,
                    NavTarget::TeamType,
                    a->TeamOne ? Find_Index(TeamTypes, a->TeamOne) : -1);
                Goto_Row("Team two", a->TeamTwo ? a->TeamTwo->Name() : nullptr,
                    NavTarget::TeamType,
                    a->TeamTwo ? Find_Index(TeamTypes, a->TeamTwo) : -1);

                ImGui::Text("Roles:");
                ImGui::SameLine(); RO_Checkbox("Skirmish##ai",    a->IsForSkirmish);
                ImGui::SameLine(); RO_Checkbox("BaseDefense##ai", a->IsForBaseDefense);

                ImGui::Text("Success/exec   : %d / %d", a->SuccessCount, a->ExecutionCount);
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
                if (!h) { ImGui::TextDisabled("<null>"); return; }

                ImGui::Text("Name          : %s", h->Name());
                ImGui::Text("HeapID        : %d", static_cast<int>(h->HeapID));
                ImGui::Separator();
                ImGui::Text("Side          : %d", h->Side);
                ImGui::Text("Prefix        : '%c'  Suffix: '%s'", h->Prefix, h->Suffix);

                RO_Checkbox("IsMultiplay##ht",        h->IsMultiplay);
                RO_Checkbox("IsMultiplayPassive##ht", h->IsMultiplayPassive);
                RO_Checkbox("IsWallOwner##ht",        h->IsWallOwner);
                RO_Checkbox("IsSmartAI##ht",          h->IsSmartAI);

                ImGui::Text("FirepowerBias : x%.2f", h->FirepowerBias);
                ImGui::Text("ArmorBias     : x%.2f", h->ArmorBias);
                ImGui::Text("CostBias      : x%.2f", h->CostBias);
            });
    }

    static void Draw_Types_Tab()
    {
        if (!ImGui::BeginTabBar("##type_cats")) {
            return;
        }
        if (ImGui::BeginTabItem("Triggers", nullptr, Tab_Flag(NavTarget::TriggerType)))   { Draw_TriggerTypes_Pane();   ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Tags",     nullptr, Tab_Flag(NavTarget::TagType)))       { Draw_TagTypes_Pane();       ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Teams",    nullptr, Tab_Flag(NavTarget::TeamType)))      { Draw_TeamTypes_Pane();      ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("TaskForces", nullptr, Tab_Flag(NavTarget::TaskForce)))   { Draw_TaskForces_Pane();     ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Scripts",  nullptr, Tab_Flag(NavTarget::ScriptType)))    { Draw_ScriptTypes_Pane();    ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("AITriggers", nullptr, Tab_Flag(NavTarget::AITriggerType))) { Draw_AITriggerTypes_Pane(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Houses",   nullptr, Tab_Flag(NavTarget::HouseType)))     { Draw_HouseTypes_Pane();     ImGui::EndTabItem(); }
        ImGui::EndTabBar();
    }


    /*
    ** ---------------- Tab "Instances" panes ----------------
    */
    static void Draw_Triggers_Pane()
    {
        static int selected = -1;
        Consume_Nav(NavTarget::TriggerInst, selected);
        Draw_List_Detail("##triggers", Triggers.Count(), selected,
            [](int i, char* buf, size_t n) {
                const TriggerClass* t = Triggers[i];
                const char* nm = (t && t->Class) ? t->Class->Name() : "?";
                std::snprintf(buf, n, "[%d] %s", i, nm);
            },
            [](int i) {
                const TriggerClass* t = Triggers[i];
                if (!t) { ImGui::TextDisabled("<null>"); return; }

                Goto_Row("Class", t->Class ? t->Class->Name() : nullptr,
                    NavTarget::TriggerType,
                    t->Class ? Find_Index(TriggerTypes, t->Class) : -1);

                ImGui::Separator();

                RO_Checkbox("IsActive##trig", t->IsActive); ImGui::SameLine();
                RO_Checkbox("IsToDie##trig",  t->IsToDie);

                ImGui::Text("Timer        : %ld frames", static_cast<long>(t->Timer));

                if (t->LinkedTo != nullptr && t->LinkedTo->Class != nullptr) {
                    Goto_Row("LinkedTo (trigger)", t->LinkedTo->Class->Name(),
                        NavTarget::TriggerInst, Find_Index(Triggers, t->LinkedTo));
                }

                /*
                ** TrippedFlags is a bitmask, bit `n` set => the n-th event
                ** in the trigger type's event list has been tripped.
                */
                ImGui::SeparatorText("TrippedFlags");
                ImGui::Text("Raw : 0x%X", t->TrippedFlags);
                if (t->Class != nullptr) {
                    int n = 0;
                    for (const TEventClass* e = t->Class->Event; e != nullptr; e = e->Next, ++n) {
                        const bool tripped = (t->TrippedFlags & (1 << n)) != 0;
                        char label[96];
                        std::snprintf(label, sizeof(label), "[%d] %s##trip",
                            n, TEventClass::Event_Name(e->Event));
                        RO_Checkbox(label, tripped);
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
                const char* nm = (t && t->Class) ? t->Class->Name() : "?";
                std::snprintf(buf, n, "[%d] %s", i, nm);
            },
            [](int i) {
                const TagClass* t = Tags[i];
                if (!t) { ImGui::TextDisabled("<null>"); return; }

                Goto_Row("Class (TagType)",
                    t->Class ? t->Class->Name() : nullptr,
                    NavTarget::TagType,
                    t->Class ? Find_Index(TagTypes, t->Class) : -1);

                ImGui::Separator();

                /*
                ** Attached-to: vanilla supports attachment to cell, building
                ** or houses. Get_Position() returns the cell when attached to
                ** terrain/cell; CELL_NONE (-1,-1) when attached elsewhere.
                */
                const Cell pos = t->Get_Position();
                if (pos.X >= 0 && pos.Y >= 0) {
                    ImGui::Text("Attached to  : cell %d, %d", pos.X, pos.Y);
                } else {
                    ImGui::Text("Attached to  : object(s) (no cell)");
                }
                ImGui::Text("AttachCount  : %d", t->AttachCount);

                if (t->Class != nullptr) {
                    ImGui::Text("Persistence  : %s (%d)",
                        Persistence_To_String(t->Class->Persistence),
                        static_cast<int>(t->Class->Persistence));
                }

                RO_Checkbox("IsToDie##tag",  t->IsToDie);  ImGui::SameLine();
                RO_Checkbox("IsSprung##tag", t->IsSprung);

                ImGui::SeparatorText("Linked trigger");
                if (t->Trigger != nullptr) {
                    Goto_Row("Trigger",
                        t->Trigger->Class ? t->Trigger->Class->Name() : "?",
                        NavTarget::TriggerInst, Find_Index(Triggers, t->Trigger));

                    if (t->Trigger->Class != nullptr) {
                        Goto_Row("TriggerType", t->Trigger->Class->Name(),
                            NavTarget::TriggerType,
                            Find_Index(TriggerTypes, t->Trigger->Class));
                    }
                } else if (t->Class != nullptr && t->Class->TriggerType != nullptr) {
                    Goto_Row("TriggerType (from class)",
                        t->Class->TriggerType->Name(),
                        NavTarget::TriggerType,
                        Find_Index(TriggerTypes, t->Class->TriggerType));
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
                const char* nm = t ? t->Name() : "?";
                std::snprintf(buf, n, "[%d] %s", i, nm ? nm : "?");
            },
            [](int i) {
                const TeamClass* t = Teams[i];
                if (!t) { ImGui::TextDisabled("<null>"); return; }

                Goto_Row("Class (TeamType)",
                    t->Class ? t->Class->Name() : nullptr,
                    NavTarget::TeamType,
                    t->Class ? Find_Index(TeamTypes, t->Class) : -1);

                ImGui::Separator();

                Goto_Row("House",
                    t->House ? t->House->IniName.c_str() : nullptr,
                    NavTarget::HouseInst,
                    t->House ? Find_Index(Houses, t->House) : -1);

                ImGui::Text("Members      : %d", t->Total);
                ImGui::Text("Risk         : %d", t->Risk);

                ImGui::Text("Flags:");
                RO_Checkbox("ForcedActive##team",  t->IsForcedActive);  ImGui::SameLine();
                RO_Checkbox("FullStrength##team",  t->IsFullStrength);
                RO_Checkbox("UnderStrength##team", t->IsUnderStrength); ImGui::SameLine();
                RO_Checkbox("HasBeen##team",       t->IsHasBeen);

                if (t->Tag != nullptr) {
                    Goto_Row("Live tag",
                        t->Tag->Class ? t->Tag->Class->Name() : "?",
                        NavTarget::TagInst, Find_Index(Tags, t->Tag));
                }

                /*
                ** Zone is the AbstractClass the team is centered around (a
                ** cell, building, etc). Offer a one-shot "jump camera there"
                ** button.
                */
                if (t->Zone != nullptr && TacticalMap != nullptr) {
                    const Coord c = t->Zone->Center_Coord();
                    const Cell cell = c.As_Cell();
                    ImGui::Text("Zone        :");
                    ImGui::SameLine();
                    char buf[64];
                    std::snprintf(buf, sizeof(buf), "› Center (%d, %d)##zone", cell.X, cell.Y);
                    if (ImGui::SmallButton(buf)) {
                        TacticalMap->Set_Tactical_Position(c);
                    }
                }

                /*
                ** Script progression: ScriptClass is owned by the team and
                ** has no meaningful life outside of it, so its progression
                ** lives here rather than in a separate Scripts tab.
                */
                if (t->Script != nullptr) {
                    ImGui::SeparatorText("Script progression");
                    Goto_Row("Script type",
                        t->Script->Class ? t->Script->Class->Name() : nullptr,
                        NavTarget::ScriptType,
                        t->Script->Class ? Find_Index(ScriptTypes, t->Script->Class) : -1);
                    ImGui::Text("CurrentMission : %d", t->Script->CurrentMission);

                    if (t->Script->Class != nullptr) {
                        const ScriptTypeClass* sc = t->Script->Class;
                        const int count = sc->MissionCount < ScriptTypeClass::MAX_SCRIPT_MISSIONS
                            ? sc->MissionCount : ScriptTypeClass::MAX_SCRIPT_MISSIONS;
                        for (int m = 0; m < count; ++m) {
                            const ScriptMissionClass& mc = sc->MissionList[m];
                            const bool current = (m == t->Script->CurrentMission);
                            if (current) {
                                /* Highlight the currently-executing line. */
                                ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f),
                                    "▶ [%d] %s  (data=%d)", m,
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
                std::snprintf(buf, n, "[%d] %s", i, h ? h->IniName.c_str() : "<null>");
            },
            [](int i) {
                const HouseClass* h = Houses[i];
                if (!h) { ImGui::TextDisabled("<null>"); return; }

                ImGui::Text("IniName         : %s", h->IniName.c_str());
                Goto_Row("Class (HouseType)",
                    h->Class ? h->Class->Name() : nullptr,
                    NavTarget::HouseType,
                    h->Class ? Find_Index(HouseTypes, h->Class) : -1);

                ImGui::Separator();

                RO_Checkbox("IsHuman##h",         h->IsHuman);         ImGui::SameLine();
                RO_Checkbox("IsPlayerControl##h", h->IsPlayerControl);
                RO_Checkbox("IsDefeated##h",      h->IsDefeated);      ImGui::SameLine();
                RO_Checkbox("IsAlerted##h",       h->IsAlerted);

                ImGui::Text("Credits         : %ld", h->Credits);
                ImGui::Text("Base nodes      : %d", h->Base.Nodes.Count());

                /*
                ** Allies is a bitmask of HousesType: bit (1 << other->HeapID)
                ** is set if `h` considers `other` an ally. Decode it into the
                ** actual house list with goto buttons; the raw mask stays
                ** alongside for sanity.
                */
                ImGui::Text("Allies          : 0x%X", h->Control.Allies);
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
                    Goto_Row(slot, other->IniName.c_str(),
                        NavTarget::HouseInst, oi);
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
        if (ImGui::BeginTabItem("Triggers", nullptr, Tab_Flag(NavTarget::TriggerInst))) { Draw_Triggers_Pane(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Tags",     nullptr, Tab_Flag(NavTarget::TagInst)))     { Draw_Tags_Pane();     ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Teams",    nullptr, Tab_Flag(NavTarget::TeamInst)))    { Draw_Teams_Pane();    ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Houses",   nullptr, Tab_Flag(NavTarget::HouseInst)))   { Draw_Houses_Pane();   ImGui::EndTabItem(); }
        ImGui::EndTabBar();
    }


    /*
    ** ---------------- Tab "State" ----------------
    */
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
            if (ImGui::TreeNode("##house", "%s (%d nodes)",
                h->IniName.c_str(), h->Base.Nodes.Count()))
            {
                for (int ni = 0; ni < h->Base.Nodes.Count(); ++ni) {
                    const BaseNodeClass& n = h->Base.Nodes[ni];
                    ImGui::Text("[%d] %s @ (%d, %d)", ni,
                        StructType_Name(n.Type), n.CellID.X, n.CellID.Y);
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
