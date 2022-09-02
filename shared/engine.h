#pragma once

#pragma pack(push, 4)

#define SYSTEM_MESSAGE_STACK_SIZE		256

class VFILE_SERVICE;
class CONTROLS;
class VSYSTEM_API;

class VMA
{
protected:
	VMA* pNext;
	long  nHash;
	long  nReference;
public:
	VMA()
	{
		nReference = 0;
		nHash = 0;
		//pNext = _pModuleClassRoot;
		//_pModuleClassRoot = this;
	};
	VMA* Next() { return pNext; }
	virtual ~VMA() {};
	long Build_Version() { return 1337; };
	void SetHash(long _hash) { nHash = _hash; }
	long GetHash() { return nHash; }
	void Set(VMA* _p) { pNext = _p; };
	virtual bool Service() { return false; }
	virtual const char* GetName() { return nullptr; }
	virtual void* CreateClass() { return nullptr; }
	virtual void RefDec() { nReference--; };
	virtual long GetReference() { return nReference; }
	virtual void Clear() { nReference = 0; };
	virtual bool ScriptLibriary() { return false; }
};

class VAPI
{
public:
	VAPI() = default;
	virtual ~VAPI() = default;

	VFILE_SERVICE* fio;
	CONTROLS* Controls;
};

typedef VMA* (__cdecl* DLLAPIFUNC)(VAPI*, VSYSTEM_API*);

struct MODULE_STATE
{
	HINSTANCE hInst;
	DLLAPIFUNC api_func_PTR;
};

struct MODULES_TABLE
{
	DWORD Paths_Count;
	char** Paths_Table;
	char gstring[MAX_PATH];
	DWORD nModulesNum;
	MODULE_STATE* pTable;
};

struct SYSTEM_MESSAGE
{
	UINT iMsg;
	WPARAM wParam;
	LPARAM lParam;
};

struct STRINGS_LIST
{
	DWORD List_size;
	DWORD Strings;
	char** String_Table_PTR;
	DWORD used_data_size;
	DWORD Cache[8];
	DWORD Cache_Pos;
};

struct FSDATA_LIST
{
	void* Data_PTR;
	DWORD structure_size;
	DWORD initiate_blocks;
	DWORD used_blocks;
};

struct SERVICE
{
	void* vtbl;
};

struct SERVICE_NODE
{
	DWORD reference;
	DWORD module_code;
	DWORD class_code;
	SERVICE* pointer;
	DWORD* linkL;
	DWORD* linkR;
};

struct SERVICES_LIST
{
	SERVICE_NODE* List;
	DWORD Objects;
	DWORD Search_module_code;
};

struct ENTITY_CREATION_TIME
{
	DWORD time[2];
};

struct CORE_STATE
{
	DWORD engine_version;
	DWORD Atoms_max_orbit;
	DWORD Atoms_min_free_orbit;
	DWORD Atoms_number;
	DWORD Atoms_space;
	ENTITY_CREATION_TIME Creation_Time;
};

class CORE : public VAPI
{
public:
	CORE() = default;
	~CORE() = default;

	bool bAppActive;
	bool Memory_Leak_flag;
	bool Reset_flag;
	bool Root_flag;
	bool Exit_flag;
	bool Initialized;
	bool bEngineIniProcessed;
	HWND App_Hwnd;
	char gstring[MAX_PATH];
	BYTE _unk0[4];
	MODULES_TABLE Modules_Table;
	SERVICES_LIST Services_List;
	HINSTANCE hInst;
	CORE_STATE CoreState;
	char* State_file_name;
	void** Atoms_PTR;
	DWORD Atom_Search_Position;
	DWORD Atom_Search_Class_Code;
	DWORD Atom_Get_Position;
	DWORD Scan_Layer_Code;
	DWORD Constructor_counter;
	FSDATA_LIST DeleteEntityList;
	FSDATA_LIST DeleteLayerList;
	FSDATA_LIST DeleteServicesList;
	STRINGS_LIST ClassesOff;
	BYTE _unk3[664];
	SYSTEM_MESSAGE MessageStack[SYSTEM_MESSAGE_STACK_SIZE];
	DWORD SystemMessagesNum;
};

#pragma pack(pop)

#define CHECK_OFFSET(_class, _field, _offset) static_assert(offsetof(_class, _field) == _offset, "Invalid " #_class "::" #_field " offset")

CHECK_OFFSET(CORE, App_Hwnd, 0x14);
CHECK_OFFSET(CORE, SystemMessagesNum, 0x1178);