/*
	SYRINGE.H
	---------
	Holds macros, structures, classes that are necessary to interact with Syringe correctly.
																						-pd
*/

#pragma once

#include <windows.h>

class LimitedRegister {
protected:
	DWORD data;

	WORD* wordData() {
		return reinterpret_cast<WORD*>(&this->data);
	}

	BYTE* byteData() {
		return reinterpret_cast<BYTE*>(&this->data);
	}

public:
	WORD Get16() {
		return *this->wordData();
	}

	template<typename T>
	inline T Get() {
		return *reinterpret_cast<T*>(&this->data);
	}

	template<typename T>
	inline void Set(T value) {
		this->data = DWORD(value);
	}

	void Set16(WORD value) {
		*this->wordData() = value;
	}
};

class ExtendedRegister : public LimitedRegister {
public:
	BYTE Get8Hi() {
		return this->byteData()[1];
	}

	BYTE Get8Lo() {
		return this->byteData()[0];
	}

	void Set8Hi(BYTE value) {
		this->byteData()[1] = value;
	}

	void Set8Lo(BYTE value) {
		this->byteData()[0] = value;
	}
};

class StackRegister : public ExtendedRegister {
public:
	template<typename T>
	inline T* lea(int byteOffset) {
		return reinterpret_cast<T*>(static_cast<DWORD>(this->data + static_cast<DWORD>(byteOffset)));
	}

	inline DWORD lea(int byteOffset) {
		return static_cast<DWORD>(this->data + static_cast<DWORD>(byteOffset));
	}

	template<typename T>
	inline T At(int byteOffset) {
		return *reinterpret_cast<T*>(this->data + static_cast<DWORD>(byteOffset));
	}

	template<typename T>
	inline void At(int byteOffset, T value) {
		*reinterpret_cast<T*>(this->data + static_cast<DWORD>(byteOffset)) = value;
	}
};

//Macros to make the following a lot easier
#define REG_SHORTCUTS(reg) \
	inline DWORD reg() \
		{ return this->_ ## reg.Get<DWORD>(); } \
	template<typename T> inline T reg() \
		{ return this->_ ## reg.Get<T>(); } \
	template<typename T> inline void reg(T value) \
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

//A pointer to this class is passed as an argument to EXPORT functions
class REGISTERS
{
private:
	DWORD	origin;
	DWORD	flags;

	LimitedRegister _EDI;
	LimitedRegister _ESI;
	StackRegister _EBP;
	StackRegister _ESP;
	ExtendedRegister _EBX;
	ExtendedRegister _EDX;
	ExtendedRegister _ECX;
	ExtendedRegister _EAX;

public:
	DWORD Origin() {
		return this->origin;
	}

	DWORD EFLAGS() {
		return this->flags;
	}

	void EFLAGS(DWORD value) {
		this->flags = value;
	}

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
	inline T lea_Stack(int offset) {
		return reinterpret_cast<T>(this->_ESP.lea(offset));
	}

	template<>
	inline DWORD lea_Stack(int offset) {
		return this->_ESP.lea(offset);
	}

	template<>
	inline int lea_Stack(int offset) {
		return static_cast<int>(this->_ESP.lea(offset));
	}

	template<typename T>
	inline T& ref_Stack(int offset) {
		return *this->lea_Stack<T*>(offset);
	}

	template<typename T>
	inline T Stack(int offset) {
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

	template<typename T>
	inline T Base(int offset) {
		return this->_EBP.At<T>(offset);
	}

	template<typename T>
	inline void Stack(int offset, T value) {
		this->_ESP.At(offset, value);
	}

	void Stack16(int offset, WORD value) {
		this->_ESP.At(offset, value);
	}

	void Stack8(int offset, BYTE value) {
		this->_ESP.At(offset, value);
	}

	template<typename T>
	inline void Base(int offset, T value) {
		this->_EBP.At(offset, value);
	}
};

//Use this for DLL export functions
//e.g. EXPORT FunctionName(REGISTERS* R)
#define EXPORT extern "C" __declspec(dllexport) DWORD __cdecl
#define EXPORT_FUNC(name) extern "C" __declspec(dllexport) DWORD __cdecl name (REGISTERS *R)


//Handshake definitions
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


#if SYR_VER == 2

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
namespace SyringeData {
	namespace Hooks {

	};
	namespace Hosts {

	};
};

#define declhost(exename, checksum) \
namespace SyringeData { namespace Hosts { __declspec(allocate(".syexe00")) hostdecl _hst__ ## exename  { checksum, #exename }; }; };

#define declhook(hook, funcname, size) \
namespace SyringeData { namespace Hooks { __declspec(allocate(".syhks00")) hookdecl _hk__ ## hook ## funcname  {  hook, size, #funcname }; }; };

#endif // SYR_VER == 2


// create empty macros
#ifndef declhost
#define declhost(exename, checksum)
#endif // declhost

#ifndef declhook
#define declhook(hook, funcname, size)
#endif // declhook

// Defines a hook at the specified address with the specified name and saving the specified amount of instruction bytes to be restored if return to the same address is used. In addition to the injgen-declaration, also includes the function opening.
#define DEFINE_HOOK(hook, funcname, size) \
declhook(hook, funcname, size) \
EXPORT_FUNC(funcname)
// Does the same as DEFINE_HOOK but no function opening, use for injgen-declaration when repeating the same hook at multiple addresses.
// CAUTION: funcname must be the same as in DEFINE_HOOK.
#define DEFINE_HOOK_AGAIN(hook, funcname, size) \
declhook(hook, funcname, size)
