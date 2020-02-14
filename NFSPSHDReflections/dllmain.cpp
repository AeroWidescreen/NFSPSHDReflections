#include "stdafx.h"
#include "stdio.h"
#include <windows.h>
#include "..\includes\injector\injector.hpp"
#include <cstdint>
#include "..\includes\IniReader.h"
#include <d3d9.h>

bool HDReflections, ImproveReflectionLOD, RealFrontEndReflections, BrightnessFix;
int ResolutionX, ResolutionY;
int ResX, ResY;
float Scale;

float BrightnessMultiplier = 2.0f;
float BrightnessDivider = 100.0f;
DWORD BrightnessResult;
DWORD BrightnessFixCodeCaveExit = 0x4B3E91;

void __declspec(naked) BrightnessFixCodeCave()
{
	__asm {
		fild dword ptr ds : [0xAC6F0C]				// Loads brightness integer
		fmul dword ptr ds : [BrightnessMultiplier]	// Multiplies by 2
		fstp dword ptr ds : [BrightnessResult]		// Stores result
		fld dword ptr ds : [BrightnessDivider]		// Loads a value of 100
		fdiv dword ptr ds : [BrightnessResult]		// Divides by result
		fstp dword ptr ds : [0xAA9630]				// Stores new gamma float at 00AA9630
		movss xmm0, dword ptr ds : [0xAA9630]
		jmp BrightnessFixCodeCaveExit
	}
}

void Init()
{
	// Read values from .ini
	CIniReader iniReader("NFSPSHDReflections.ini");

	// Resolution
	ResX = iniReader.ReadInteger("RESOLUTION", "ResolutionX", 0);
	ResY = iniReader.ReadInteger("RESOLUTION", "ResolutionY", 0);
	Scale = iniReader.ReadFloat("RESOLUTION", "Scale", 1.0);

	// General
	HDReflections = iniReader.ReadInteger("GENERAL", "HDReflections", 1);
	ImproveReflectionLOD = iniReader.ReadInteger("GENERAL", "ImproveReflectionLOD", 1);
	RealFrontEndReflections = iniReader.ReadInteger("GENERAL", "RealFrontEndReflections", 0);

	// Extra
	BrightnessFix = iniReader.ReadInteger("EXTRA", "BrightnessFix", 1);

	if (ResX <= 0 || ResY <= 0)
	{
		ResX = ::GetSystemMetrics(SM_CXSCREEN);
		ResY = ::GetSystemMetrics(SM_CYSCREEN);
	}

	if (HDReflections)
	{
		// Vehicle Reflection
		injector::WriteMemory<uint32_t>(0x4BD062, ResY * Scale, true);
		injector::WriteMemory<uint32_t>(0x4BD24D, ResY * Scale, true);
		injector::WriteMemory<uint32_t>(0x4BD283, ResY * Scale, true);
		injector::WriteMemory<uint32_t>(0x4BD288, ResY * Scale, true);
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
		//injector::WriteMemory<uint8_t>(0x701606, 0xEB, true);
	}

	if (BrightnessFix)
	{
		// Sets default brightness to 50%
		injector::WriteMemory<float>(0xAA9630, 1.0f, true);
		// Custom brightness code
		injector::MakeJMP(0x4B3E89, BrightnessFixCodeCave, true);
		injector::MakeNOP(0x4B3E8E, 1, true);
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