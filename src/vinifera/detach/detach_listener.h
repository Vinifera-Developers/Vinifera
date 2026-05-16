/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Type-indexed detach listener registry.
 *
 *          When an object is destroyed, only listeners that have opted in to
 *          its concrete type (and its hierarchy ancestors) get notified.
 *          Replaces the legacy O(N-extensions) Extension::Detach_This_From_All
 *          sweep with a per-target-type lookup.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/
#pragma once

#include "noinit.h"
#include "vector.h"


class AbstractClass;


namespace Vinifera::Detach {


template<typename T> class Listener;


template<typename T>
class Registry
{
public:
    static void Add(Listener<T>* listener)
    {
        List().Add(listener);
    }

    static void Remove(Listener<T>* listener)
    {
        List().Delete(listener);
    }

    /**
     *  Iterates backwards so a listener may safely unregister itself
     *  (i.e. be destroyed) during its own On_Detach callback. A listener
     *  must not destroy other listeners of the same target type T.
     */
    static void Notify(T* target, bool all);

private:
    static DynamicVectorClass<Listener<T>*>& List()
    {
        static DynamicVectorClass<Listener<T>*> g_list;
        return g_list;
    }
};


template<typename T>
class Listener
{
public:
    Listener()                            { Registry<T>::Add(this); }

    /**
     *  No-op on purpose. NoInit is used for in-place reconstruction during
     *  IPersistStream::Load (the derived class does `new (this) Klass(NoInit())`
     *  to reset the vtable after streaming the parent fields). The original
     *  ctor already added us to the registry at the same address, so re-adding
     *  here would leave a stale duplicate that survives our destructor.
     */
    Listener(const NoInitClass&)          {}

    virtual ~Listener()                   { Registry<T>::Remove(this); }

    virtual void On_Detach(T* target, bool all) = 0;

private:
    Listener(const Listener&) = delete;
    Listener& operator=(const Listener&) = delete;
};


template<typename T>
void Registry<T>::Notify(T* target, bool all)
{
    auto& list = List();
    for (int i = list.Count() - 1; i >= 0; --i) {
        list[i]->On_Detach(target, all);
    }
}


/**
 *  Entry point called from the engine intercept. Dispatches the given
 *  AbstractClass-derived target to all relevant concrete-type registries
 *  plus the generic Registry<AbstractClass>.
 *
 *  IMPORTANT: When adding a new Listener<T> for an AbstractClass-derived T,
 *  ensure Notify_Abstract dispatches to Registry<T>::Notify from every RTTI
 *  case whose runtime type IS-A T. Failure mode is observable: the listener
 *  compiles and registers but never fires.
 */
void Notify_Abstract(AbstractClass* target, bool all);


} // namespace Vinifera::Detach
