#include "Interfaces.h"
#include <Windows.h>
#include "Utility.h"

CInterfaces * Interfaces = nullptr;

class InterfaceReg
{
private:

	using InstantiateInterfaceFn = void*(*)();

public:

	InstantiateInterfaceFn m_CreateFn;
	const char *m_pName;

	InterfaceReg *m_pNext;
};

template<typename T>
T* GetInterface(const char* modName, const char* ifaceName, bool exact = false)
{
	T* iface = nullptr;
	InterfaceReg* ifaceRegList;
	int partMatchLen = strlen(ifaceName);

	DWORD ifaceFn = reinterpret_cast<DWORD>(GetProcAddress(GetModuleHandleA(modName), "CreateInterface"));

	if (!ifaceFn)
		return nullptr;

	unsigned int jmpStart = (unsigned int)(ifaceFn)+4;
	unsigned int jmpTarget = jmpStart + *(unsigned int*)(jmpStart + 1) + 5;

	ifaceRegList = **reinterpret_cast<InterfaceReg * **>(jmpTarget + 6);

	for (InterfaceReg* cur = ifaceRegList; cur; cur = cur->m_pNext)
	{
		if (exact == true)
		{
			if (strcmp(cur->m_pName, ifaceName) == 0)
				iface = reinterpret_cast<T*>(cur->m_CreateFn());
		}
		else
		{
			if (!strncmp(cur->m_pName, ifaceName, partMatchLen) && std::atoi(cur->m_pName + partMatchLen) > 0)
				iface = reinterpret_cast<T*>(cur->m_CreateFn());
		}
	}
	return iface;
}

CInterfaces::CInterfaces()
{
	Client = GetInterface<IBaseClientDLL>("client.dll", "VClient0");
	Globals = **(CGlobalVarsBase***)(Util.FindPattern("client.dll", "B9 ? ? ? ? 8B 40 38 FF D0 84 C0 0F 85") + 0x1);
	ClientMode = **(IClientMode***)((*(DWORD**)Client)[10] + 0x5);
	Server = GetInterface<IServerGameDLL>("server.dll", "ServerGameDLL");
	Panel = GetInterface<IVPanel>("vgui2.dll", "VGUI_Panel");
	ClientEntityList = GetInterface<IClientEntityList>("client.dll", "VClientEntityList");
	EngineClient = GetInterface<IEngineClient>("engine.dll", "VEngineClient");
	InputSystem = GetInterface<IInputSystem>("inputsystem.dll", "InputSystemVersion");
	Input = *(CInput**)(Util.FindPattern("client.dll", "B9 ? ? ? ? 8B 40 38 FF D0 84 C0 0F 85") + 0x1);
	Surface = GetInterface<ISurface>("vguimatsurface.dll", "VGUI_Surface");
	EngineTrace = GetInterface<IEngineTrace>("engine.dll", "EngineTraceClient");
}
