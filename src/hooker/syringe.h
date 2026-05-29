/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Holds macros, structures, classes that are necessary to interact
 *          with Syringe correctly.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include <bit>
#include <cstdint>
#include <type_traits>
#include <windows.h>

/**
 *  Models a 32-bit CPU register with unified access to its full DWORD
 *  and its low 16-bit half (AX-style) via typed getters/setters.
 */
class LimitedRegister
{
protected:
    DWORD data;
    WORD* wordData() { return reinterpret_cast<WORD*>(&this->data); }
    BYTE* byteData() { return reinterpret_cast<BYTE*>(&this->data); }

public:
    WORD Get16() {
        return *this->wordData();
    }

    template<typename T>
    T Get() {
        return *reinterpret_cast<T*>(&this->data);
    }

    template<typename T>
    void Set(T value) {
        this->data = (DWORD)value;
    }

    void Set16(WORD value) {
        *this->wordData() = value;
    }
};

/**
 *  Extends LimitedRegister with 8-bit high/low access (AH/AL),
 *  providing a full x86-style register family abstraction.
 */
class ExtendedRegister : public LimitedRegister
{
public:
    BYTE Get8Hi() { return this->byteData()[1]; }
    BYTE Get8Lo() { return this->byteData()[0]; }
    void Set8Hi(BYTE value) { this->byteData()[1] = value; }
    void Set8Lo(BYTE value) { this->byteData()[0] = value; }
};

/**
 *  Extends ExtendedRegister with stack/memory addressing helpers
 *  (lea and At), emulating ESP/EBP-style base-pointer operations.
 */
class StackRegister : public ExtendedRegister
{
public:
    template<typename T>
    T* lea(int byteOffset) {
        return reinterpret_cast<T*>(this->data + static_cast<DWORD>(byteOffset));
    }

    DWORD lea(int byteOffset) {
        return this->data + static_cast<DWORD>(byteOffset);
    }

    template<typename T>
    T At(int byteOffset) {
        return *reinterpret_cast<T*>(this->data + static_cast<DWORD>(byteOffset));
    }

    template<typename T>
    void At(int byteOffset, T value) {
        *reinterpret_cast<T*>(this->data + static_cast<DWORD>(byteOffset)) = value;
    }
};

/**
 *  Macros to make the following a lot easier
 */
#define REG_SHORTCUTS(reg) \
    DWORD reg() \
        { return this->_ ## reg.Get<DWORD>(); } \
    template<typename T> T reg() \
        { return this->_ ## reg.Get<T>(); } \
    template<typename T> void reg(T value) \
        { this->_ ## reg.Set(value); } \

#define REG_SHORTCUTS_X(r) \
    DWORD r ## X() \
        { return this->_E ## r ## X.Get16(); } \
    void r ## X(WORD value) \
        { this->_E ## r ## X.Set16(value); } \

#define REG_SHORTCUTS_HL(r) \
    DWORD r ## H() \
        { return this->_E ## r ## X.Get8Hi(); } \
    void r ## H(BYTE value) \
        { this->_E ## r ## X.Set8Hi(value); } \
    DWORD r ## L() \
        { return this->_E ## r ## X.Get8Lo(); } \
    void r ## L(BYTE value) \
        { this->_E ## r ## X.Set8Lo(value); } \

#define REG_SHORTCUTS_XHL(r) \
    REG_SHORTCUTS_X(r); \
    REG_SHORTCUTS_HL(r); \

/**
 *  A pointer to this class is passed as an argument to EXPORT functions
 */
class REGISTERS
{
private:
    template<typename T>
    using StackValueType = std::remove_cv_t<std::remove_reference_t<T>>;

    template<typename T>
    static constexpr bool IsPushPopType =
        std::is_pointer_v<T>
        || std::is_enum_v<T>
        || std::is_integral_v<T>
        || (sizeof(T) == sizeof(DWORD)
            && std::is_trivially_copyable_v<T>
            && !std::is_floating_point_v<T>);

    template<typename T>
    static DWORD PushPopToDWORD(T value) {
        using ValueType = StackValueType<T>;

        static_assert(IsPushPopType<ValueType>,
            "REGISTERS::Push/Pop only support pointers, enums, integral values, and trivially-copyable DWORD-sized values.");

        if constexpr (std::is_pointer_v<ValueType>) {
            return static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(value));
        } else if constexpr (std::is_enum_v<ValueType> || std::is_integral_v<ValueType>) {
            return static_cast<DWORD>(value);
        } else {
            return std::bit_cast<DWORD>(value);
        }
    }

    template<typename T>
    static T DWORDToPushPop(DWORD value) {
        using ValueType = StackValueType<T>;

        static_assert(IsPushPopType<ValueType>,
            "REGISTERS::Push/Pop only support pointers, enums, integral values, and trivially-copyable DWORD-sized values.");

        if constexpr (std::is_pointer_v<ValueType>) {
            return reinterpret_cast<T>(static_cast<std::uintptr_t>(value));
        } else if constexpr (std::is_enum_v<ValueType> || std::is_integral_v<ValueType>) {
            return static_cast<T>(value);
        } else {
            return static_cast<T>(std::bit_cast<ValueType>(value));
        }
    }

    DWORD origin;
    DWORD flags;

    LimitedRegister _EDI;
    LimitedRegister _ESI;
    StackRegister _EBP;
    StackRegister _ESP;
    ExtendedRegister _EBX;
    ExtendedRegister _EDX;
    ExtendedRegister _ECX;
    ExtendedRegister _EAX;

public:
    DWORD Origin() { return this->origin; }
    DWORD EFLAGS() { return this->flags; }
    void EFLAGS(DWORD value) { this->flags = value; }

    REG_SHORTCUTS(EAX);
    REG_SHORTCUTS(EBX);
    REG_SHORTCUTS(ECX);
    REG_SHORTCUTS(EDX);
    REG_SHORTCUTS(ESI);
    REG_SHORTCUTS(EDI);
    REG_SHORTCUTS(ESP);
    REG_SHORTCUTS(EBP);

    REG_SHORTCUTS_XHL(A);
    REG_SHORTCUTS_XHL(B);
    REG_SHORTCUTS_XHL(C);
    REG_SHORTCUTS_XHL(D);

    template<typename T>
    T lea_Stack(int offset) {
        return reinterpret_cast<T>(this->_ESP.lea(offset));
    }

    template<>
    DWORD lea_Stack(int offset) {
        return this->_ESP.lea(offset);
    }

    template<>
    int lea_Stack(int offset) {
        return static_cast<int>(this->_ESP.lea(offset));
    }

    template<typename T>
    T& ref_Stack(int offset) {
        return *this->lea_Stack<T*>(offset);
    }

    template<typename T>
    T Stack(int offset) {
        return this->_ESP.At<T>(offset);
    }

    DWORD Stack32(int offset) {
        return this->_ESP.At<DWORD>(offset);
    }

    WORD Stack16(int offset) {
        return this->_ESP.At<WORD>(offset);
    }

    BYTE Stack8(int offset) {
        return this->_ESP.At<BYTE>(offset);
    }

    /**
     *  Simulate x86 32-bit push/pop instructions on a single stack slot.
     *  These helpers are not generic stack-resizing primitives.
     */
    template<typename T>
    void Push(T value) {
        this->ESP(this->ESP() - 4);
        this->Stack(0, PushPopToDWORD(value));
    }

    template<typename T = DWORD>
    T Pop() {
        const DWORD value = this->Stack32(0);
        this->ESP(this->ESP() + 4);
        return DWORDToPushPop<T>(value);
    }

    template<typename T>
    T Base(int offset) {
        return this->_EBP.At<T>(offset);
    }

    template<typename T>
    void Stack(int offset, T value) {
        this->_ESP.At(offset, value);
    }

    void Stack16(int offset, WORD value) {
        this->_ESP.At(offset, value);
    }

    void Stack8(int offset, BYTE value) {
        this->_ESP.At(offset, value);
    }

    template<typename T>
    void Base(int offset, T value) {
        this->_EBP.At(offset, value);
    }
};

/**
 *  Use this for DLL export functions, e.g. EXPORT FunctionName(REGISTERS* R)
 */
#define EXPORT extern "C" __declspec(dllexport) DWORD __cdecl
#define EXPORT_FUNC(name) extern "C" __declspec(dllexport) DWORD __cdecl name (REGISTERS *R)

/**
 *  Handshake definitions
 */
struct SyringeHandshakeInfo
{
    int cbSize;
    int num_hooks;
    unsigned int checksum;
    DWORD exeFilesize;
    DWORD exeTimestamp;
    unsigned int exeCRC;
    int cchMessage;
    char* Message;
};

#define SYRINGE_HANDSHAKE(pInfo) extern "C" __declspec(dllexport) HRESULT __cdecl SyringeHandshake(SyringeHandshakeInfo* pInfo)

#pragma pack(push, 16)
#pragma warning(push)
#pragma warning( disable : 4324)
__declspec(align(16)) struct hookdecl {
    unsigned int hookAddr;
    unsigned int hookSize;
    const char * hookName;
};

__declspec(align(16)) struct hostdecl {
    unsigned int hostChecksum;
    const char * hostName;
};
#pragma warning(pop)
#pragma pack(pop)

#pragma section(".syhks00", read, write)
#pragma section(".syexe00", read, write)
namespace SyringeData
{
namespace Hooks
{

}
namespace Hosts
{

}
inline void* LastHookOrigin = nullptr;
} // namespace SyringeData

#define declhost(exename, checksum) \
namespace SyringeData { namespace Hosts { __declspec(allocate(".syexe00")) hostdecl _hst__##exename {checksum, #exename}; }; };

#define _declhook(address, funcname, size) \
namespace SyringeData { namespace Hooks { __declspec(allocate(".syhks00")) hookdecl _hk__##address##funcname {address, size, #funcname}; }; };

/**
 *  If HOOK_LOGGING is defined, a logging stub is created that logs when the hook is entered.
 *  If HOOK_LOGGING_VERBOSE is also defined, the logging stub will print detailed information.
 */
#define HOOK_LOGGING
// #define HOOK_LOGGING_VERBOSE

#ifdef HOOK_LOGGING

    // Verbose version
    #ifdef HOOK_LOGGING_VERBOSE
        #define declhook(address, funcname, size) \
            EXPORT_FUNC(funcname##_##address##_LogStub) \
            { \
                DEBUG_INFO("[Syringe] Hook {} entered at {:08X}.\n", #funcname, address); \
                SyringeData::LastHookOrigin = reinterpret_cast<void*>(R->Origin()); \
                return 0; \
            } \
            _declhook(address, funcname##_##address##_LogStub, size) \
            _declhook(address, funcname, size) \

    #else
        // Non-verbose version
        #define declhook(address, funcname, size) \
            EXPORT_FUNC(funcname##_##address##_LogStub) \
            { \
                SyringeData::LastHookOrigin = reinterpret_cast<void*>(R->Origin()); \
                return 0; \
            } \
            _declhook(address, funcname##_##address##_LogStub, size) \
            _declhook(address, funcname, size) \

    #endif // HOOK_LOGGING_VERBOSE

#else

    // No logging: only one hookdecl
    #define declhook(address, funcname, size) _declhook(address, funcname, size)

#endif // HOOK_LOGGING

/**
 *  DEFINE_HOOK defines a hook at the specified address with the specified name and saving
 *  the specified amount of instruction bytes to be restored if return to the same address is used.
 *  In addition to the injgen-declaration, also includes the function opening.
 */
#define DEFINE_HOOK(address, funcname, size) declhook(address, funcname, size) EXPORT_FUNC(funcname)

/**
 *  DEFINE_HOOK_AGAIN does the same as DEFINE_HOOK but no function opening,
 *  use for injgen-declaration when repeating the same hook at multiple addresses.
 *
 *  CAUTION: funcname must be the same as in DEFINE_HOOK.
 */
#define DEFINE_HOOK_AGAIN(address, funcname, size) declhook(address, funcname, size)

/**
 *  Convenient macros for getting register and stack values.
 */
#define GET(clsname, var, reg) clsname var = R->reg<clsname>();
#define LEA_STACK(clsname, var, offset) clsname var = R->lea_Stack<clsname>(offset);
#define REF_STACK(clsname, var, offset) clsname& var = R->ref_Stack<clsname>(offset);
#define GET_STACK(clsname, var, offset) clsname var = R->Stack<clsname>(offset);
#define GET_BASE(clsname, var, offset) clsname var = R->Base<clsname>(offset);
#define STACK_OFFSET(cur_offset, wanted_offset) (cur_offset + wanted_offset)

/**
 *  Feature flags set by Syringe at injection time.
 *  DLLs can check these at runtime to verify that the running Syringe
 *  version supports the features they rely on. Each flag defaults to false
 *  and is set to true by Syringe after the DLL is loaded into the target
 *  process. If a DLL is loaded by an older Syringe that does not know about
 *  feature flags, the values will remain false.
 */
namespace SyringeFeatures
{
extern "C" __declspec(dllexport) inline bool ESPModification = false;
extern "C" __declspec(dllexport) inline bool ZFPreservation = false;
extern "C" __declspec(dllexport) inline bool ReladdrInstructionFixup = false;
} // namespace SyringeFeatures
