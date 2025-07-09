/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          HOUSEEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended HouseClass class.
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
#include "houseext.h"

#include <algorithm>
#include "house.h"
#include "ccini.h"
#include "extension.h"
#include "asserthandler.h"
#include "debughandler.h"
#include "factory.h"
#include "mouse.h"
#include "saveload.h"
#include "sidebarext.h"
#include "vinifera_saveload.h"
#include "storageext.h"
#include "tibsun_functions.h"
#include "utracker.h"
#include "building.h"
#include "factoryext.h"
#include "overlaytype.h"
#include "prerequisitegroup.h"
#include "rules.h"
#include "session.h"
#include "team.h"
#include "teamtype.h"
#include "unit.h"
#include "unittypeext.h"
#include "voc.h"
#include "vox.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
HouseClassExtension::HouseClassExtension(const HouseClass *this_ptr) :
    AbstractClassExtension(this_ptr),
    TiberiumStorage(Tiberiums.Count()),
    WeedStorage(Tiberiums.Count()),
    NavalFactories(0),
    NavalFactory(nullptr),
    BuildNavalUnit(UNIT_NONE)
{
    //if (this_ptr) EXT_DEBUG_TRACE("HouseClassExtension::HouseClassExtension - 0x%08X\n", (uintptr_t)(This()));

    for (int i = 0; i < Tiberiums.Count(); i++)
    {
        TiberiumStorage[i] = 0;
        WeedStorage[i] = 0;
    }

    if (this_ptr)
    {
        new ((StorageClassExt*)&this_ptr->Tiberium) StorageClassExt(&TiberiumStorage);
        new ((StorageClassExt*)&this_ptr->Weed) StorageClassExt(&WeedStorage);
    }

    HouseExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
HouseClassExtension::HouseClassExtension(const NoInitClass &noinit) :
    AbstractClassExtension(noinit),
    TiberiumStorage(noinit),
    WeedStorage(noinit)
{
    //EXT_DEBUG_TRACE("HouseClassExtension::HouseClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
HouseClassExtension::~HouseClassExtension()
{
    //EXT_DEBUG_TRACE("HouseClassExtension::~HouseClassExtension - 0x%08X\n", (uintptr_t)(This()));

    HouseExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT HouseClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("HouseClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (lpClassID == nullptr) {
        return E_POINTER;
    }

    *lpClassID = __uuidof(this);

    return S_OK;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT HouseClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("HouseClassExtension::Load - 0x%08X\n", (uintptr_t)(This()));

    HRESULT hr = AbstractClassExtension::Internal_Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    Load_Primitive_Vector(pStm, TiberiumStorage, "TiberiumStorage");
    Load_Primitive_Vector(pStm, WeedStorage, "WeedStorage");

    new (this) HouseClassExtension(NoInitClass());

    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(NavalFactory, "NavalFactory");
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT HouseClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("HouseClassExtension::Save - 0x%08X\n", (uintptr_t)(This()));

    HRESULT hr = AbstractClassExtension::Internal_Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    Save_Primitive_Vector(pStm, TiberiumStorage, "TiberiumStorage");
    Save_Primitive_Vector(pStm, WeedStorage, "WeedStorage");

    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: CCHyper
 */
int HouseClassExtension::Get_Object_Size() const
{
    //EXT_DEBUG_TRACE("HouseClassExtension::Get_Object_Size - 0x%08X\n", (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void HouseClassExtension::Detach(AbstractClass * target, bool all)
{
    //EXT_DEBUG_TRACE("HouseClassExtension::Detach - 0x%08X\n", (uintptr_t)(This()));

    if (NavalFactory == target) {
        NavalFactory = nullptr;
    }
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void HouseClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("HouseClassExtension::Object_CRC - 0x%08X\n", (uintptr_t)(This()));
}


/**
 *  Extended replacement of HouseClass::Fetch_Factory.
 *
 *  @author: ZivDero
 */
FactoryClass* HouseClassExtension::Fetch_Factory(RTTIType rtti, ProductionFlags flags) const
{
    FactoryClass* factory = nullptr;

    switch (rtti) {
    case RTTI_INFANTRY:
    case RTTI_INFANTRYTYPE:
        factory = This()->InfantryFactory;
        break;

    case RTTI_UNIT:
    case RTTI_UNITTYPE:
        if (flags & PRODFLAG_NAVAL) {
            factory = NavalFactory;
        } else {
            factory = This()->UnitFactory;
        }
        break;

    case RTTI_BUILDING:
    case RTTI_BUILDINGTYPE:
        factory = This()->BuildingFactory;
        break;

    case RTTI_AIRCRAFT:
    case RTTI_AIRCRAFTTYPE:
        factory = This()->AircraftFactory;
        break;

    default:
        factory = nullptr;
        break;
    }
    return factory;
}


/**
 *  Extended replacement of HouseClass::Set_Factory.
 *
 *  @author: ZivDero
 */
void HouseClassExtension::Set_Factory(RTTIType rtti, FactoryClass* factory, ProductionFlags flags)
{
    switch (rtti) {
    case RTTI_UNIT:
    case RTTI_UNITTYPE:
        if (flags & PRODFLAG_NAVAL) {
            NavalFactory = factory;
        } else {
            This()->UnitFactory = factory;
        }
        break;

    case RTTI_INFANTRY:
    case RTTI_INFANTRYTYPE:
        This()->InfantryFactory = factory;
        break;

    case RTTI_BUILDING:
    case RTTI_BUILDINGTYPE:
        This()->BuildingFactory = factory;
        break;

    case RTTI_AIRCRAFT:
    case RTTI_AIRCRAFTTYPE:
        This()->AircraftFactory = factory;
        break;
    }
}


/**
 *  Extended replacement of HouseClass::Factory_Counter.
 *
 *  @author: ZivDero
 */
int* HouseClassExtension::Factory_Counter(RTTIType rtti, ProductionFlags flags)
{
    switch (rtti) {
    case RTTI_UNITTYPE:
    case RTTI_UNIT:
        if (flags & PRODFLAG_NAVAL) {
            return &NavalFactories;
        } else {
            return &This()->UnitFactories;
        }

    case RTTI_AIRCRAFTTYPE:
    case RTTI_AIRCRAFT:
        return &This()->AircraftFactories;

    case RTTI_INFANTRYTYPE:
    case RTTI_INFANTRY:
        return &This()->InfantryFactories;

    case RTTI_BUILDINGTYPE:
    case RTTI_BUILDING:
        return &This()->BuildingFactories;

    default:
        break;
    }
    return nullptr;
}


/**
 *  Extended replacement of HouseClass::Factory_Count.
 *
 *  @author: ZivDero
 */
int HouseClassExtension::Factory_Count(RTTIType rtti, ProductionFlags flags) const
{
    int const* ptr = const_cast<HouseClassExtension*>(this)->Factory_Counter(rtti, flags);
    if (ptr != nullptr) {
        return *ptr;
    }
    return 0;
}


/**
 *  Extended replacement of HouseClass::Suspend_Production.
 *
 *  @author: ZivDero
 */
ProdFailType HouseClassExtension::Suspend_Production(RTTIType type, ProductionFlags flags)
{
    FactoryClass* fptr = Fetch_Factory(type, flags);

    /*
    **  If the house is already busy producing the requested object, then
    **  return with this failure code.
    */
    if (fptr == nullptr) return PROD_CANT;

    /*
    **  Actually suspend the production.
    */
    fptr->Suspend();

    /*
    **  Tell the sidebar that it needs to be redrawn because of this.
    */
    if (PlayerPtr == This()) {
        Map.SidebarClass::IsToRedraw = true;
        RedrawSidebar = true;
        Map.Flag_To_Redraw();
        Map.Column[0].Flag_To_Redraw();
        Map.Column[1].Flag_To_Redraw();
    }

    return PROD_OK;
}


/**
 *  Extended replacement of HouseClass::Begin_Production.
 *
 *  @author: ZivDero
 */
ProdFailType HouseClassExtension::Begin_Production(RTTIType type, int id, bool resume, ProductionFlags flags)
{
    int result = true;
    FactoryClass* fptr;
    const TechnoTypeClass* tech = Fetch_Techno_Type(type, id);

    BuildingClass* who = tech->Who_Can_Build_Me(false, true, true, This());
    bool onhold = false;

    if (who == nullptr) {
        if (resume) {
            who = tech->Who_Can_Build_Me(true, false, true, This());
        }
        if (who != nullptr) {
            onhold = true;
        } else {
            DEBUG_INFO("Request to Begin_Production of '%s' was rejected. No-one can build.\n", tech->FullName);
            return PROD_CANT;
        }
    }

    fptr = Fetch_Factory(type, flags);

    if (fptr == nullptr) {
        fptr = new FactoryClass;

        if (fptr == nullptr) {
            DEBUG_INFO("Request to Begin_Production of '%s' was rejected. Unable to create factory\n", tech->FullName);
            return PROD_CANT;
        }
    }

    /*
    **  If the house is already busy producing the requested object, then
    **  return with this failure code, unless we are restarting production.
    */
    if (fptr != nullptr) {
        if (fptr->Is_Building() && type == RTTI_BUILDINGTYPE) {
            DEBUG_INFO("Request to Begin_Production of '%s' was rejected. Cannot queue buildings.\n", tech->FullName);
            return PROD_CANT;
        }
    }

    Set_Factory(type, fptr, flags);

    /*
    **  Check if we have an object of this type currently suspended in production.
    */
    bool skipset = false;
    if (fptr->IsSuspended && !Extension::Fetch(fptr)->IsHoldingExit) {
        TechnoClass* object = fptr->Object;
        if (object != nullptr) {
            if (object->TClass == tech) {
                skipset = true;
            }
        }
    }

    if (!skipset) {
        result = fptr->Set(*tech, *This(), resume);
    }

    if (result) {
        if (fptr->QueuedObjects.Count() && !resume && !skipset) {
            SidebarExtension->Flag_Strip_To_Redraw(type, flags);
        } else {
            fptr->Start(onhold);

            /*
            **  Link this factory to the sidebar so that proper graphic feedback
            **  can take place.
            */
            if (PlayerPtr == This()) {
                Map.Factory_Link(fptr, type, id);
            }
        }

        return PROD_OK;
    }

    DEBUG_INFO("Request to Begin_Production of '%s' was rejected. Factory was unable to create the requested object\n", tech->FullName);

    /*
    **  Output debug information if production failed.
    */
    if (fptr->QueuedObjects.Count() == 0 && fptr->Object == nullptr) {
        DEBUG_INFO("type=%d\n", type);
        DEBUG_INFO("Frame == %d\n", Frame);
        DEBUG_INFO("fptr->QueuedObjects.Count() == %d\n", fptr->QueuedObjects.Count());
        DEBUG_INFO("Object->RTTI == %d\n", fptr->Object != nullptr ? fptr->Object->Fetch_RTTI() : -1);
        DEBUG_INFO("Object->HeapID == %d\n", fptr->Object != nullptr ? fptr->Object->Fetch_Heap_ID() : -1);
        DEBUG_INFO("IsSuspended\t= %d\n", fptr->IsSuspended);

        delete fptr;
        Set_Factory(type, nullptr, flags);
    }

    return PROD_CANT;
}


/**
 *  Extended replacement of HouseClass::Abandon_Production.
 *
 *  @author: ZivDero
 */
ProdFailType HouseClassExtension::Abandon_Production(RTTIType type, int id, ProductionFlags flags)
{
    FactoryClass* fptr = Fetch_Factory(type, flags);

    /*
    **  If there is no factory to abandon, then return with a failure code.
    */
    if (fptr == nullptr) {
        return PROD_CANT;
    }

    /*
    **  If we're just dequeuing a unit, redraw the strip.
    */
    if (fptr->Queued_Object_Count() > 0 && id >= 0) {
        const TechnoTypeClass* technotype = Fetch_Techno_Type(type, id);
        if (fptr->Remove_From_Queue(*technotype)) {
            SidebarExtension->Flag_Strip_To_Redraw(type, flags);
            return PROD_OK;
        }
    }

    if (id != -1) {
        TechnoClass* obj = fptr->Object;
        if (obj == nullptr || id != obj->Class_Of()->Fetch_Heap_ID()) {
            return PROD_OK;
        }
    }

    /*
    **  Tell the sidebar that it needs to be redrawn because of this.
    */
    if (PlayerPtr == This()) {
        SidebarExtension->Abandon_Production(type, fptr, flags);

        if (type == RTTI_BUILDINGTYPE || type == RTTI_BUILDING) {
            Map.PendingObjectPtr = nullptr;
            Map.PendingObject = nullptr;
            Map.PendingHouse = HOUSE_NONE;
            Map.Set_Cursor_Shape(nullptr);
        }
    }

    /*
    **  Abandon production of the object.
    */
    fptr->Abandon();
    if (fptr->QueuedObjects.Count() == 0) {
        Set_Factory(type, nullptr, flags);
        delete fptr;
    } else {
        fptr->Resume_Queue();
    }

    return PROD_OK;
}


/**
 *  Extended replacement of HouseClass::Place_Object.
 *
 *  @author: ZivDero
 */
bool HouseClassExtension::Place_Object(RTTIType type, Cell const& cell, ProductionFlags flags)
{
    bool placed = false;
    TechnoClass* tech = nullptr;
    FactoryClass* factory = Fetch_Factory(type, flags);

    /*
    **  Only if there is a factory active for this type, can it be "placed".
    **  In the case of a missing factory, then this request is completely bogus --
    **  ignore it. This might occur if, between two events to exit the same
    **  object, the mouse was clicked on the sidebar to start building again.
    **  The second placement event should NOT try to place the object that is
    **  just starting construction.
    */
    if (factory && factory->Has_Completed()) {
        auto factory_ext = Extension::Fetch(factory);
        tech = factory->Get_Object();

        if (tech != nullptr) {

            /*
            **  Announce that the object is ready.
            */
            if (!factory_ext->HasSpoken && factory->House == PlayerPtr) {
                if (tech->Is_Foot()) {
                    Speak(VOX_UNIT_READY);
                } else if (tech->RTTI == RTTI_BUILDING) {
                    Speak(VOX_CONSTRUCTION);
                }
                factory_ext->HasSpoken = true;
            }

            /*
            **  For units, make sure there's a factory that is not busy exiting a unit, otherwise just hold the unit for now.
            */
            if (tech->RTTI == RTTI_UNIT && Extension::Fetch(tech->Class_Of())->Who_Can_Build_Me(false, false, false, tech->Owner_HouseClass(), true) == nullptr) {
                factory_ext->IsHoldingExit = true;
                return placed;
            }

            if (cell == CELL_NONE || cell == Cell(-1, -1)) {

                /*
                **  Try to find a place for the object to appear from. For helicopters, it has the
                **  option of finding a nearby helipad if no helipads are free.
                */
                TechnoClass* builder = tech->Who_Can_Build_Me(false, true);
                if (builder == nullptr && tech->RTTI == RTTI_AIRCRAFT) {
                    builder = tech->Who_Can_Build_Me(true, true);
                }

                if (builder != nullptr) {
                    int exit = builder->Exit_Object(tech);
                    if (exit == 2 || (exit == 1 && builder->RTTI == RTTI_BUILDING && static_cast<BuildingClass*>(builder)->Factory != nullptr)) {

                        /*
                        **  Since the object has left the factory under its own power, delete
                        **  the production manager tied to this slot in the sidebar. Its job
                        **  has been completed.
                        */
                        LastRadarEventCell = builder->Center_Coord().As_Cell();
                        factory->Completed();
                        Abandon_Production(type, -1, flags);
                        placed = true;
                    } else {
                        /*
                        **  The object could not leave under it's own power. Just wait
                        **  until the player tries to place the object again.
                        */
                        if (This()->RTTI != RTTI_BUILDING) {
                            DEBUG_INFO("Failed to exit object from factory - refunding money\n");
                            Abandon_Production(type, -1, flags);
                        }
                        return placed;
                    }
                }

            } else {
                TechnoClass* builder = tech->Who_Can_Build_Me(false, false);
                if (builder) {

                    builder->Transmit_Message(RADIO_HELLO, tech);
                    if (tech->Unlimbo(cell.As_Coord())) {
                        if (tech->RTTI == RTTI_BUILDING) {
                            if (static_cast<BuildingClass*>(tech)->Class->IsFirestormWall) {
                                Map.Place_Firestorm_Wall(cell, This(), static_cast<BuildingClass*>(tech)->Class);
                            } else if (static_cast<BuildingClass*>(tech)->Class->ToOverlay != nullptr && static_cast<BuildingClass*>(tech)->Class->ToOverlay->IsWall) {
                                Map.Place_Wall(cell, This(), static_cast<BuildingClass*>(tech)->Class);
                            }
                        }
                        factory->Completed();
                        tech->Transmit_Message(RADIO_COMPLETE, builder);
                        Abandon_Production(type, -1, flags);
                        placed = true;

                        if (PlayerPtr == This()) {
                            if (tech->IsActive && !tech->IsDiscoveredByPlayer) {
                                tech->Revealed(This());
                            }
                            Sound_Effect(Rule->BuildingSlam);
                            Map.Set_Cursor_Shape(nullptr);
                            Map.PendingObjectPtr = nullptr;
                            Map.PendingObject = nullptr;
                            Map.PendingHouse = HOUSE_NONE;
                        }
                    } else {
                        placed = false;
                        if (This() == PlayerPtr) {
                            Speak(VOX_DEPLOY);
                        }
                    }
                    builder->Transmit_Message(RADIO_OVER_OUT);
                }
            }
        }

        if (placed) {

            /*
            **  Record the construction of the object.
            */
            This()->Just_Built(tech);

            /*
            **  If the factory still exists, we need to "reset" it.
            */
            if (factory != nullptr && Factories.Is_Present(factory)) {

                /*
                **  Mark that the factory isn't holding anything waiting to exit anymore.
                */
                Extension::Fetch(factory)->IsHoldingExit = false;

                /*
                **  Clear the flag for announcing unit production.
                */
                Extension::Fetch(factory)->HasSpoken = false;

            }

            /*
            **  For foot units, plays the unit ready sound when they exit.
            */
            if (tech->Is_Foot() && tech->House == PlayerPtr) {
                Speak(VOX_UNIT_READY);
            }
        }
    }

    return placed;
}


/**
 *  Extended replacement of HouseClass::Update_Factories.
 *
 *  @author: ZivDero
 */
void HouseClassExtension::Update_Factories(RTTIType rtti, ProductionFlags flags)
{
    FactoryClass* factory = Fetch_Factory(rtti, flags);

    if (factory != nullptr) {
        for (int i = factory->QueuedObjects.Count() - 1; i >= 0; i--) {
            TechnoTypeClass const* ttype = factory->QueuedObjects[i];
            if (ttype->Who_Can_Build_Me(true, false, true, This()) == nullptr) {
                factory->QueuedObjects.Delete(i);
            }
        }
        if (factory->Object != nullptr) {
            if (factory->Object->TClass->Who_Can_Build_Me(true, false, true, This()) == nullptr) {
                factory->Abandon();
                factory->Resume_Queue();
            } else {
                if (factory->Object->TClass->Who_Can_Build_Me(true, true, true, This()) == nullptr) {
                    factory->Suspend(false);
                    if (PlayerPtr == This()) {
                        Map.SidebarClass::IsToRedraw = true;
                        RedrawSidebar = true;
                        Map.Flag_To_Redraw();
                        Map.Column[0].Flag_To_Redraw();
                        Map.Column[1].Flag_To_Redraw();
                    }
                } else {
                    if (factory->IsSuspended && !factory->IsOnHold) {
                        factory->Start(false);
                    }
                }
            }
        }
        if (factory->Object == nullptr && factory->QueuedObjects.Count() == 0) {
            delete factory;
        }
    }
}


/**
 *  Extended replacement of HouseClass::Suggest_New_Object.
 *
 *  @author: ZivDero
 */
TechnoTypeClass const* HouseClassExtension::Suggest_New_Object(RTTIType objecttype, ProductionFlags flags) const
{
    TechnoTypeClass const* techno = nullptr;

    switch (objecttype) {
    case RTTI_AIRCRAFT:
    case RTTI_AIRCRAFTTYPE:
        if (This()->BuildAircraft != AIRCRAFT_NONE) {
            return AircraftTypes[This()->BuildAircraft];
        }
        return nullptr;

        /*
        **  Unit construction is based on the rule that up to twice the number required
        **  to fill all teams will be created.
        */
    case RTTI_UNIT:
    case RTTI_UNITTYPE:
        if (flags & PRODFLAG_NAVAL) {
            if (BuildNavalUnit != UNIT_NONE) {
                return UnitTypes[BuildNavalUnit];
            }
        } else {
            if (This()->BuildUnit != UNIT_NONE) {
                return UnitTypes[This()->BuildUnit];
            }
        }
        return nullptr;

        /*
        **  Infantry construction is based on the rule that up to twice the number required
        **  to fill all teams will be created.
        */
    case RTTI_INFANTRY:
    case RTTI_INFANTRYTYPE:
        if (This()->BuildInfantry != INFANTRY_NONE) {
            return InfantryTypes[This()->BuildInfantry];
        }
        return nullptr;

        /*
        **  Building construction is based upon the preconstruction list.
        */
    case RTTI_BUILDING:
    case RTTI_BUILDINGTYPE:
        if (This()->BuildStructure != STRUCT_NONE) {
            return BuildingTypes[This()->BuildStructure];
        }
        return nullptr;
    }
    return techno;
}


/**
 *  Reimplementation of HouseClass::AI_Unit.
 *
 *  @author: ZivDero
 */
int HouseClassExtension::AI_Unit()
{
    if (This()->BuildUnit != UNIT_NONE) return TICKS_PER_SECOND;

    int harv = This()->ActiveUQuantity.Count_Of(This()->Get_First_ActLike(Rule->HarvesterUnit)->HeapID);
    int ref = This()->ActiveBQuantity.Count_Of(This()->Get_First_ActLike(Rule->BuildRefinery)->HeapID);
    int mult;
    if (Session.Type == GAME_NORMAL || This()->Difficulty == DIFF_HARD) {
        mult = 1;
    } else {
        mult = 2;
    }

    /*
    **  A computer controlled house will try to build a replacement
    **  harvester if possible.
    */
    if (This()->IQ >= Rule->IQHarvester && !This()->IsTiberiumShort && !This()->Is_Human_Player() && ref * mult > harv) {
        if (This()->Get_First_ActLike(Rule->HarvesterUnit)->TechLevel <= This()->Control.TechLevel) {
            This()->BuildUnit = This()->Get_First_ActLike(Rule->HarvesterUnit)->HeapID;
            return TICKS_PER_SECOND;
        }
    }

    int counter[1000]; // size increased replicating ts-patches
    int value[std::size(counter)];
    memset(counter, 0x00, sizeof(counter));
    for (int& i : value) {
        i = 0x7FFFFFFF;
    }

    /*
    **  Build a list of the maximum of each type we wish to produce. This will be
    **  twice the number required to fill all teams.
    */
    for (TeamClass* tptr : Teams) {
        if (tptr != nullptr) {

            int val = tptr->field_40;

            if (((tptr->Class->IsReinforcable && !tptr->IsFullStrength) || (!tptr->IsForcedActive && !tptr->IsHasBeen)) && tptr->House == This()) {
                DynamicVectorClass<TechnoTypeClass const*> _members;
                tptr->Team_Members(_members);

                for (int subindex = 0; subindex < _members.Count(); subindex++) {
                    if (_members[subindex]->RTTI == RTTI_UNITTYPE) {
                        UnitTypeClass const* memtype = static_cast<UnitTypeClass const*>(_members[subindex]);
                        counter[memtype->HeapID]++;
                        value[memtype->HeapID] = std::min(val, value[memtype->HeapID]);
                    }
                }
            }
        }
    }

    /*
    **  Reduce the theoretical maximum by the actual number of objects currently
    **  in play.
    */
    for (UnitClass* obj : Units) {
        if (obj != nullptr && obj->Is_Recruitable(This()) && counter[obj->Class->HeapID] > 0) {
            counter[obj->Class->HeapID]--;
        }
    }

    /*
    **  Pick to build the most needed object but don't consider those object that
    **  can't be built because of scenario restrictions or insufficient cash.
    */
    int bestval = -1;
    int bestcount = 0;
    UnitType lasttype = UNIT_NONE;
    int lastval = 0x7FFFFFFF;
    UnitType bestlist[std::size(counter)];
    for (UnitType type = UNIT_FIRST; type < UnitTypes.Count(); type++) {
        if (counter[type] > 0 && This()->Can_Build(UnitTypes[type], false, false) && UnitTypes[type]->Cost_Of(This()) <= This()->Available_Money() && !Extension::Fetch(UnitTypes[type])->IsNaval) {
            if (bestval == -1 || bestval < counter[type]) {
                bestval = counter[type];
                bestcount = 0;
            }
            bestlist[bestcount++] = type;

            if (lasttype == UNIT_NONE || value[type] < lastval) {
                lasttype = type;
                lastval = value[type];
            }
        }
    }

    if (Random_Pick2(0, 0x7FFFFFFE) < Rule->FillEarliestTeamProbability[This()->Difficulty] / 100.0) {
        This()->BuildUnit = lasttype;
    } else {
        /*
        **  The object type to build is now known. Fetch a pointer to the techno type class.
        */
        if (bestcount) {
            This()->BuildUnit = bestlist[Random_Pick(0, bestcount - 1)];
        }
    }

    return TICKS_PER_SECOND;
}


/**
 *  A new AI naval unit production handler.
 *
 *  @author: ZivDero
 */
int HouseClassExtension::AI_Naval_Unit()
{
    if (BuildNavalUnit != UNIT_NONE) return TICKS_PER_SECOND;

    int counter[1000]; // size increased replicating ts-patches
    int value[std::size(counter)];
    memset(counter, 0x00, sizeof(counter));
    for (int& i : value) {
        i = 0x7FFFFFFF;
    }

    /*
    **  Build a list of the maximum of each type we wish to produce. This will be
    **  twice the number required to fill all teams.
    */
    for (TeamClass* tptr : Teams) {
        if (tptr != nullptr) {

            int val = tptr->field_40;

            if (((tptr->Class->IsReinforcable && !tptr->IsFullStrength) || (!tptr->IsForcedActive && !tptr->IsHasBeen)) && tptr->House == This()) {
                DynamicVectorClass<TechnoTypeClass const*> _members;
                tptr->Team_Members(_members);

                for (int subindex = 0; subindex < _members.Count(); subindex++) {

                    UnitTypeClass const* memtype = static_cast<UnitTypeClass const*>(_members[subindex]);

                    if (memtype->RTTI == RTTI_UNITTYPE) {
                        counter[memtype->HeapID]++;
                        value[memtype->HeapID] = std::min(val, value[memtype->HeapID]);
                    }
                }
            }
        }
    }

    /*
    **  Reduce the theoretical maximum by the actual number of objects currently
    **  in play.
    */
    for (UnitClass* obj : Units) {
        if (obj != nullptr && obj->Is_Recruitable(This()) && counter[obj->Class->HeapID] > 0) {
            counter[obj->Class->HeapID]--;
        }
    }

    /*
    **  Pick to build the most needed object but don't consider those object that
    **  can't be built because of scenario restrictions or insufficient cash.
    */
    int bestval = -1;
    int bestcount = 0;
    UnitType lasttype = UNIT_NONE;
    int lastval = 0x7FFFFFFF;
    UnitType bestlist[std::size(counter)];
    for (UnitType type = UNIT_FIRST; type < UnitTypes.Count(); type++) {
        if (counter[type] > 0 && This()->Can_Build(UnitTypes[type], false, false) && UnitTypes[type]->Cost_Of(This()) <= This()->Available_Money() && Extension::Fetch(UnitTypes[type])->IsNaval) {
            if (bestval == -1 || bestval < counter[type]) {
                bestval = counter[type];
                bestcount = 0;
            }
            bestlist[bestcount++] = type;

            if (lasttype == UNIT_NONE || value[type] < lastval) {
                lasttype = type;
                lastval = value[type];
            }
        }
    }

    if (Random_Pick2(0, 0x7FFFFFFE) < Rule->FillEarliestTeamProbability[This()->Difficulty] / 100.0) {
        BuildNavalUnit = lasttype;
    } else {
        /*
        **  The object type to build is now known. Fetch a pointer to the techno type class.
        */
        if (bestcount) {
            BuildNavalUnit = bestlist[Random_Pick(0, bestcount - 1)];
        }
    }

    return TICKS_PER_SECOND;
}


/**
 *  Checks if the house owns this prerequisite, as it appears in the Prerequisite= list.
 *
 *  @author: ZivDero
 */
bool HouseClassExtension::Has_Prerequisite(int prerequisite)
{
    if (prerequisite >= STRUCT_FIRST) {
        return Has_Prerequisite(static_cast<StructType>(prerequisite));
    } else {
        return Has_Prerequisite(PrerequisiteGroupClass::Decode(prerequisite));
    }
}


/**
 *  Checks if the house owns a building that satisfies this prerequisite group.
 *
 *  @author: ZivDero
 */
bool HouseClassExtension::Has_Prerequisite(PrerequisiteGroupType group)
{
    /*
    **  Invalid prerequisite groups can't be owned.
    */
    if (group < PREREQ_GROUP_FIRST || group > PrerequisiteGroups.Count()) {
        return false;
    }

    PrerequisiteGroupClass* group_ptr = PrerequisiteGroups[group];

    /*
    **  Check if we own at least one of the types in this group.
    */
    for (int building : group_ptr->Prerequisites) {
        if (Has_Prerequisite(static_cast<StructType>(building))) {
            return true;
        }
    }

    return false;
}


/**
 *  Checks if the house owns a specific building prerequisite.
 *
 *  @author: ZivDero
 */
bool HouseClassExtension::Has_Prerequisite(StructType building)
{
    /*
    **  Invalid buildings can't be owned.
    */
    if (building < STRUCT_FIRST || building > BuildingTypes.Count()) {
        return false;
    }

    BuildingTypeClass* btype = BuildingTypes[building];

    /*
    **  If this isn't an upgrade, just check the counter.
    */
    if (btype->PowersUpBuilding[0] == '\0') {
        return This()->ActiveBQuantity.Count_Of(building) > 0;
    }

    /*
    **  For upgrades, we have to scan all of the buildings on the map...
    */
    for (int i = 0; i < Buildings.Count(); i++) {
        BuildingClass* bptr = Buildings[i];
        if (!bptr->IsInLimbo && bptr->House == This() && bptr->IsPowerOn) {
            if (bptr->Mission != MISSION_DECONSTRUCTION && bptr->MissionQueue != MISSION_DECONSTRUCTION) {
                for (int j = 0; j < std::size(bptr->Upgrades); j++) {

                    /*
                    **  If this building is upgraded with our desired prerequisite, return true.
                    */
                    if (bptr->Upgrades[j] == btype) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}


/**
 *  Puts pointers to the storage extension into the storage class.
 *
 *  @author: ZivDero
 */
void HouseClassExtension::Put_Storage_Pointers()
{
    new (reinterpret_cast<StorageClassExt*>(&This()->Tiberium)) StorageClassExt(&TiberiumStorage);
    new (reinterpret_cast<StorageClassExt*>(&This()->Weed)) StorageClassExt(&WeedStorage);
}


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static class UnitTrackerClassExt : public UnitTrackerClass
{
public:
    HRESULT _Load(IStream* pStm);
    HRESULT _Save(IStream* pStm);
};


/**
 *  Saves a unit tracker's counts
 *
 *  @author: ZivDero
 */
HRESULT UnitTrackerClassExt::_Load(IStream* pStm)
{
    int count;
    HRESULT hr = pStm->Read(&count, sizeof(count), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    new (this) UnitTrackerClass(count);

    for (int i = 0; i < UnitCount; i++) {
        hr = pStm->Read(&UnitTotals[i], sizeof(UnitTotals[i]), nullptr);
        if (FAILED(hr)) {
            return hr;
        }

    }

    return S_OK;
}


/**
 *  Saves a unit tracker's counts.
 *
 *  @author: ZivDero
 */
HRESULT UnitTrackerClassExt::_Save(IStream* pStm)
{
    HRESULT hr = pStm->Write(&UnitCount, sizeof(UnitCount), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    for (int i = 0; i < UnitCount; i++) {
        HRESULT hr = pStm->Write(&UnitTotals[i], sizeof(UnitTotals[i]), nullptr);
        if (FAILED(hr)) {
            return hr;
        }

    }

    return S_OK;
}


/**
 *  Reads a house's unit trackers.
 *
 *  @author: ZivDero
 */
void HouseClassExtension::Load_Unit_Trackers(HouseClass* house, IStream* pStm)
{
    /**
     *  Trackers store their counts in a dynamically allocated array (AARGH WW!).
     *  Thus, we need to save/load them manually.
     *  But we can't do this in the extension because ThisPtr isn't remapped yet.
     */

    house->AircraftTotals = new UnitTrackerClass(0);
    house->InfantryTotals = new UnitTrackerClass(0);
    house->UnitTotals = new UnitTrackerClass(0);
    house->BuildingTotals = new UnitTrackerClass(0);
    house->DestroyedAircraft = new UnitTrackerClass(0);
    house->DestroyedInfantry = new UnitTrackerClass(0);
    house->DestroyedUnits = new UnitTrackerClass(0);
    house->DestroyedBuildings = new UnitTrackerClass(0);
    house->CapturedBuildings = new UnitTrackerClass(0);
    house->TotalCrates = new UnitTrackerClass(0);

    reinterpret_cast<UnitTrackerClassExt*>(house->AircraftTotals)->_Load(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(house->InfantryTotals)->_Load(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(house->UnitTotals)->_Load(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(house->BuildingTotals)->_Load(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(house->DestroyedAircraft)->_Load(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(house->DestroyedInfantry)->_Load(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(house->DestroyedUnits)->_Load(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(house->DestroyedBuildings)->_Load(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(house->CapturedBuildings)->_Load(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(house->TotalCrates)->_Load(pStm);
}


/**
 *  Saves a house's unit trackers.
 *
 *  @author: ZivDero
 */
void HouseClassExtension::Save_Unit_Trackers(HouseClass* house, IStream* pStm)
{
    reinterpret_cast<UnitTrackerClassExt*>(house->AircraftTotals)->_Save(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(house->InfantryTotals)->_Save(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(house->UnitTotals)->_Save(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(house->BuildingTotals)->_Save(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(house->DestroyedAircraft)->_Save(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(house->DestroyedInfantry)->_Save(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(house->DestroyedUnits)->_Save(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(house->DestroyedBuildings)->_Save(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(house->CapturedBuildings)->_Save(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(house->TotalCrates)->_Save(pStm);
}
