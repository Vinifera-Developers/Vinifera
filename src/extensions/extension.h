/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          EXTENSION.H
 *
 *  @author        CCHyper
 *
 *  @brief         The file contains the functions required for the extension system.
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

#include "always.h"
#include "vinifera_defines.h"
#include "extension_globals.h"
#include "abstract.h"
#include "abstractext.h"
#include "swizzle.h"
#include "noinit.h"
#include "debughandler.h"
#include "asserthandler.h"

#include <unknwn.h> // for IStream
#include <typeinfo>
#include <string>


class EventClass;
class WWCRCEngine;


namespace Extension
{

/**
 *  Do not call these directly! Use the template functions below.
 */
namespace Private
{

AbstractClassExtension *Make_Internal(const AbstractClass *abstract);
bool Destroy_Internal(const AbstractClass *abstract);
AbstractClassExtension *Fetch_Internal(const AbstractClass *abstract);

}; // namespace "Extension::Private".

namespace Utility
{

/**
 *  Erase First Occurrence of given substring from main string.
 * 
 *  @author: CCHyper
 */
inline void Erase_Sub_String(std::string &str, const std::string &erase)
{
    /**
     *  Search for the substring in string.
     */
    size_t pos = str.find(erase);
    if (pos != std::string::npos) {

        /**
         *  If found then erase it from string.
         */
        str.erase(pos, erase.length());
    }
}

/**
 *  Wrapper for "typeid(T).name()", removes the "class" or "struct" prefix on the string.
 * 
 *  @author: CCHyper
 */
template<typename T>
std::string Get_TypeID_Name()
{
    std::string str = typeid(T).name();
    Erase_Sub_String(str, "class ");
    Erase_Sub_String(str, "struct ");
    return str;
}

static std::string Get_TypeID_Name(const AbstractClass *abstract)
{
    std::string str = typeid(*abstract).name();
    Erase_Sub_String(str, "class ");
    return str;
}

static std::string Get_TypeID_Name(const AbstractClassExtension *abstract_ext)
{
    std::string str = typeid(*abstract_ext).name();
    Erase_Sub_String(str, "class ");
    return str;
}

}; // namespace "Extension::Utility"

namespace Singleton
{

/**
 *  Create an isntance of the singleton class.
 * 
 *  @author: CCHyper
 */
template<class BASE_CLASS, class EXT_CLASS>
EXT_CLASS *Make(const BASE_CLASS *base)
{
    ASSERT(base != nullptr);

    EXT_CLASS *ext_ptr = new EXT_CLASS(base);
    ASSERT(ext_ptr != nullptr);

    EXT_DEBUG_INFO("Created \"%s\" extension.\n", Extension::Utility::Get_TypeID_Name<BASE_CLASS>().c_str());

    return ext_ptr;
}

/**
 *  Destroy an instance of the singleton class.
 * 
 *  @author: CCHyper
 */
template<class BASE_CLASS, class EXT_CLASS>
void Destroy(const EXT_CLASS *ext)
{
    ASSERT(ext != nullptr);

    delete ext;

    EXT_DEBUG_INFO("Destroyed \"%s\" extension.\n", Extension::Utility::Get_TypeID_Name<BASE_CLASS>().c_str());
}

}; // namespace "Extension::Singleton".

namespace List
{

/**
 *  Fetch an extension instance from a list whose extension pointer points to the base class.
 * 
 *  @author: CCHyper
 */
template<class BASE_CLASS, class EXT_CLASS>
EXT_CLASS *Fetch(const BASE_CLASS *base, DynamicVectorClass<EXT_CLASS *> &list)
{
    ASSERT(base != nullptr);

    for (int index = 0; index < list.Count(); ++index) {
        EXT_CLASS * ext = list[index];
        if (list[index]->This() == base) {
            EXT_DEBUG_INFO("Found \"%s\" extension.\n", Extension::Utility::Get_TypeID_Name<BASE_CLASS>().c_str());
            return ext;
        }
    }

    return nullptr;
}

/**
 *  Creation an instance of the extension class and add it to the list.
 * 
 *  @author: CCHyper
 */
template<class BASE_CLASS, class EXT_CLASS>
EXT_CLASS *Make(const BASE_CLASS *base, DynamicVectorClass<EXT_CLASS *> &list)
{
    ASSERT(base != nullptr);

    EXT_CLASS *ext_ptr = new EXT_CLASS(base);
    ASSERT(ext_ptr != nullptr);

    EXT_DEBUG_INFO("Created \"%s\" extension.\n", Extension::Utility::Get_TypeID_Name<BASE_CLASS>().c_str());

    list.Add(ext_ptr);

    return ext_ptr;
}

/**
 *  Destroy an instance of the extension and remove it from the list.
 * 
 *  @author: CCHyper
 */
template<class BASE_CLASS, class EXT_CLASS>
void Destroy(const BASE_CLASS *base, DynamicVectorClass<EXT_CLASS *> &list)
{
    ASSERT(base != nullptr);

    for (int index = 0; index < list.Count(); ++index) {
        EXT_CLASS * ext = list[index].This();
        if (ext->This() == base) {
            EXT_DEBUG_INFO("Found \"%s\" extension.\n", Extension::Utility::Get_TypeID_Name<BASE_CLASS>().c_str());
            delete ext;
            return;
        }
    }

    EXT_DEBUG_INFO("Destroyed \"%s\" extension.\n", Extension::Utility::Get_TypeID_Name<BASE_CLASS>().c_str());
}

}; // namespace "Extension::List".

/**
 *  Fetch the extension instance linked to this abstract object. 
 * 
 *  @author: CCHyper
 */
template<class EXT_CLASS>
EXT_CLASS *Fetch(const AbstractClass *abstract)
{
    ASSERT(abstract != nullptr);

    return (EXT_CLASS *)Extension::Private::Fetch_Internal(abstract);
}

/**
 *  Create an instance of the extension class and link it to the abstract object.
 * 
 *  @author: CCHyper
 */
template<class EXT_CLASS>
EXT_CLASS *Make(const AbstractClass *abstract)
{
    ASSERT(abstract != nullptr);

    return (EXT_CLASS *)Extension::Private::Make_Internal(abstract);
}

/**
 *  Destory an instance of the extension class linked to this abstract object.
 * 
 *  @author: CCHyper
 */
template<class EXT_CLASS>
void Destroy(const AbstractClass *abstract)
{
    ASSERT(abstract != nullptr);

    Extension::Private::Destroy_Internal(abstract);
}

/**
 *  Save and load interface.
 */
bool Save(IStream *pStm);
bool Load(IStream *pStm);
bool Request_Pointer_Remap();
unsigned Get_Save_Version_Number();

/**
 *  Tracking, announcement, and debugging functions.
 */
void Detach_This_From_All(TARGET target, bool all = true);
bool Register_Class_Factories();
void Free_Heaps();
void Print_CRCs(EventClass *ev);
void Print_CRCs(FILE *fp, EventClass *ev);

}; // namespace "Extension".


/**
 * 
 *  Base class for all global extension classes.
 * 
 */
template<class T>
class GlobalExtensionClass
{
    public:
        STDMETHOD(Load)(IStream *pStm);
        STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

    public:
        GlobalExtensionClass(const T *this_ptr = nullptr);
        GlobalExtensionClass(const NoInitClass &noinit);
        virtual ~GlobalExtensionClass();

        /**
         *  Return the raw size of class data for save/load purposes.
         *  
         *  @note: This must be overridden by the extended class!
         */
        virtual int Size_Of() const = 0;

        /**
         *  Removes the specified target from any targeting and reference trackers.
         *  
         *  @note: This must be overridden by the extended class!
         */
        virtual void Detach(TARGET target, bool all = true) = 0;

        /**
         *  Compute a unique crc value for this instance.
         *  
         *  @note: This must be overridden by the extended class!
         */
        virtual void Compute_CRC(WWCRCEngine &crc) const = 0;

        /**
         *  Access to the class instance we extend.
         */
        virtual T *This() const { return const_cast<T *>(ThisPtr); }
        virtual const T *This_Const() const { return ThisPtr; }

        /**
         *  Assign the class instance that we extend.
         */
        virtual void Assign_This(const T *this_ptr) { ASSERT(this_ptr != nullptr); ThisPtr = this_ptr; }

        /**
         *  Returns the name of this object type.
         *  
         *  @note: This must be overridden by the extended class!
         */
        virtual const char *Name() const = 0;

        /**
         *  Returns the full name of this object type.
         *  
         *  @note: This must be overridden by the extended class!
         */
        virtual const char *Full_Name() const = 0;

    private:
        /**
         *  Pointer to the class we are extending. This provides us with a way of
         *  quickly referencing the base class without doing a look-up each time.
         */
        const T *ThisPtr;

    protected:
        /**
         *  Has this extension already executed Read_INI?
         *  Set this to true at the end of Read_INI of the last extension
         *  in the inheritance hierarchy.
         */
        bool IsInitialized;

    private:
        GlobalExtensionClass(const GlobalExtensionClass &) = delete;
        void operator = (const GlobalExtensionClass &) = delete;

    public:
};


/**
 *  Class constructor
 * 
 *  @author: CCHyper
 */
template<class T>
GlobalExtensionClass<T>::GlobalExtensionClass(const T *this_ptr) :
    ThisPtr(this_ptr),
    IsInitialized(false)
{
    //if (this_ptr) EXT_DEBUG_TRACE("GlobalExtensionClass<%s>::GlobalExtensionClass - 0x%08X\n", typeid(T).name(), (uintptr_t)(ThisPtr));
    //ASSERT(ThisPtr != nullptr);      // NULL ThisPtr is valid when performing a Load state operation.
}


/**
 *  Class no-init constructor.
 * 
 *  @author: CCHyper
 */
template<class T>
GlobalExtensionClass<T>::GlobalExtensionClass(const NoInitClass &noinit)
{
    //EXT_DEBUG_TRACE("GlobalExtensionClass<%s>::GlobalExtensionClass(NoInitClass) - 0x%08X\n", typeid(T).name(), (uintptr_t)(ThisPtr));
}


/**
 *  Class destructor
 * 
 *  @author: CCHyper
 */
template<class T>
GlobalExtensionClass<T>::~GlobalExtensionClass()
{
    //EXT_DEBUG_TRACE("GlobalExtensionClass<%s>::~GlobalExtensionClass - 0x%08X\n", typeid(T).name(), (uintptr_t)(ThisPtr));

    ThisPtr = nullptr;
}


/**
 *  Loads the object from the stream and requests a new pointer to
 *  the class we extended post-load.
 * 
 *  As singleton is static data, we do not need to request
 *  pointer remap of "ThisPtr" after loading has finished.
 * 
 *  @author: CCHyper, tomsons26
 */
template<class T>
HRESULT GlobalExtensionClass<T>::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("GlobalExtensionClass<%s>::Load - 0x%08X\n", typeid(T).name(), (uintptr_t)(ThisPtr));

    if (!pStm) {
        return E_POINTER;
    }

    HRESULT hr;

    /**
     *  Read this class's binary blob data directly into this instance.
     */
    hr = pStm->Read(this, Size_Of(), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}


/**
 *  Saves the object to the stream.
 * 
 *  @author: CCHyper, tomsons26
 */
template<class T>
HRESULT GlobalExtensionClass<T>::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("GlobalExtensionClass<%s>::Save - 0x%08X\n", typeid(T).name(), (uintptr_t)(ThisPtr));

    if (!pStm) {
        return E_POINTER;
    }

    HRESULT hr;
    
    /**
     *  Write this class instance as a binary blob.
     */
    hr = pStm->Write(this, Size_Of(), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}
