#include "stdafx.h"
#include "stdio.h"
#include <windows.h>
#include "..\includes\injector\injector.hpp"
#include <cstdint>
#include "..\includes\IniReader.h"
#include <d3d9.h>

DWORD WINAPI Thing(LPVOID);

bool HDReflections, ImproveReflectionLOD, RealFrontEndReflections;
static int ResolutionX, ResolutionY;
static float Scale;
HWND windowHandle;

void Init()
{
	// Read values from .ini
	CIniReader iniReader("NFSPSHDReflections.ini");

	// Resolution
	ResolutionX = iniReader.ReadInteger("RESOLUTION", "ResolutionX", 1920);
	ResolutionY = iniReader.ReadInteger("RESOLUTION", "ResolutionY", 1080);
	Scale = iniReader.ReadFloat("RESOLUTION", "Scale", 1.0);

	// General
	HDReflections = iniReader.ReadInteger("GENERAL", "HDReflections", 1);
	ImproveReflectionLOD = iniReader.ReadInteger("GENERAL", "ImproveReflectionLOD", 1);
	RealFrontEndReflections = iniReader.ReadInteger("GENERAL", "RealFrontEndReflections", 0);

	if (HDReflections)
	{
		// Vehicle Reflection
		injector::WriteMemory<uint32_t>(0x4BD062, ResolutionY * Scale, true);
		injector::WriteMemory<uint32_t>(0x4BD24D, ResolutionY * Scale, true);
		injector::WriteMemory<uint32_t>(0x4BD283, ResolutionY * Scale, true);
		injector::WriteMemory<uint32_t>(0x4BD288, ResolutionY * Scale, true);
	}

	if (ImproveReflectionLOD)
	{
		// Vehicle Reflection LOD
		injector::WriteMemory<uint32_t>(0x74489F, 0x00008002, true);
		// Vehicle Reflection Skybox Quality
		injector::MakeNOP(0x777104, 6, true);
	}

	if (RealFrontEndReflections)
	{
		// Enables reflection mapping 
		injector::MakeNOP(0x4B8D1B, 2, true);
		injector::MakeNOP(0x4B8B8B, 2, true);
		injector::MakeNOP(0x4B8C66, 2, true);
	}
}
	

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD reason, LPVOID /*lpReserved*/)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		uintptr_t base = (uintptr_t)GetModuleHandleA(NULL);
		IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)(base);
		IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);

		if ((base + nt->OptionalHeader.AddressOfEntryPoint + (0x400000 - base)) == 0x00828C25) // Check if .exe file is compatible - Thanks to thelink2012 and MWisBest
			Init();

		else
		{
			MessageBoxA(NULL, "This .exe is not supported.\nPlease use v1.1 NFS.exe from BATTERY (27,4 MB (28.739.656 bytes)).", "NFSPS HD Reflections", MB_ICONERROR);
			return FALSE;
		}
	}
	return TRUE;

}