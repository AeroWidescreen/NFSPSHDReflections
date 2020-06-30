#include "stdafx.h"
#include "stdio.h"
#include <windows.h>
#include "..\includes\injector\injector.hpp"
#include <cstdint>
#include "..\includes\IniReader.h"
#include <d3d9.h>

bool HDReflections, ImproveReflectionLOD, ImproveReflectionSkybox, RealFrontEndReflections, GammaFix;
int ResolutionX, ResolutionY;
int ResX, ResY;
float Scale;
double VehicleReflectionBrightness;

float BrightnessMultiplier = 2.0f;
float BrightnessDivider = 100.0f;
DWORD BrightnessResult;
DWORD BrightnessFixCodeCaveExit = 0x4B3E91;

float FE_VehicleSkyboxBrightness = 0.75f;
DWORD VehicleSkyboxBrightnessCodeCaveExit = 0x4B209D;

float FE_VehicleWorldBrightness = 0.5f;
DWORD VehicleWorldBrightnessCodeCaveExit = 0x4CA487;

DWORD VehicleSkyboxQualityCodeCaveExit1 = 0x77710A;
DWORD VehicleSkyboxQualityCodeCaveExit2 = 0x7771E0;

DWORD VehicleReflectionLODCodeCaveExit = 0x7448A3;

void __declspec(naked) BrightnessFixCodeCave()
{
	__asm {
		fild dword ptr ds : [0xAC6F0C]              // Loads gamma integer
		fmul dword ptr ds : [BrightnessMultiplier]  // Multiplies by 2
		fstp dword ptr ds : [BrightnessResult]		// Stores result
		fld dword ptr ds : [BrightnessDivider]		// Loads a value of 100.0f
		fdiv dword ptr ds : [BrightnessResult]		// Divides by result
		fstp dword ptr ds : [0xAA9630]              // Stores new gamma float at 00AA9630
		movss xmm0, dword ptr ds : [0xAA9630]
		jmp BrightnessFixCodeCaveExit
	}
}

void __declspec(naked) VehicleSkyboxBrightnessCodeCave()
{
	__asm {
		movss xmm0, dword ptr ds : [0x9EEECC]
		cmp dword ptr ds : [0xABB510], 0x06 // checks if FrontEnd (0x03) or InGame (0x06)
		je VehicleSkyboxBrightnessCodeCaveInGame
		movss xmm0, dword ptr ds : [FE_VehicleSkyboxBrightness]

	VehicleSkyboxBrightnessCodeCaveInGame:
		jmp VehicleSkyboxBrightnessCodeCaveExit
	}
}

void __declspec(naked) VehicleWorldBrightnessCodeCave()
{
	__asm {
		fld dword ptr ds : [0x9EEECC]
		cmp dword ptr ds : [0xABB510], 0x06 // checks if FrontEnd (0x03) or InGame (0x06)
		je VehicleWorldBrightnessCodeCaveInGame
		fld dword ptr ds : [FE_VehicleWorldBrightness]

	VehicleWorldBrightnessCodeCaveInGame:
		jmp VehicleWorldBrightnessCodeCaveExit
	}
}

void __declspec(naked) VehicleSkyboxQualityCodeCave()
{
	__asm {
		cmp dword ptr ds : [0xABB510], 0x06 // checks if FrontEnd (0x03) or InGame (0x06)
		je VehicleSkyboxQualityCodeCaveInGame
		cmp ebx, 0x12
		jne VehicleSkyboxQualityCodeCaveFrontEnd

	VehicleSkyboxQualityCodeCaveInGame:
		jmp VehicleSkyboxQualityCodeCaveExit1

	VehicleSkyboxQualityCodeCaveFrontEnd:
		jmp VehicleSkyboxQualityCodeCaveExit2
	}
}

void __declspec(naked) VehicleReflectionLODCodeCave()
{
	__asm {
		cmp dword ptr ds : [0xABB510], 0x06 // checks if FrontEnd (0x03) or InGame (0x06)
		je VehicleReflectionLODCodeCaveInGame
		jmp VehicleReflectionLODCodeCaveFrontEnd

	VehicleReflectionLODCodeCaveInGame:
		cmp byte ptr ds : [ImproveReflectionLOD], 0x01 // checks if ImproveReflectionLOD is enabled
		jge VehicleReflectionLODCodeCaveEnable
		jmp VehicleReflectionLODCodeCaveDisable

	VehicleReflectionLODCodeCaveFrontEnd:
		cmp byte ptr ds : [RealFrontEndReflections], 0x01 // checks if RealFrontEndReflections is enabled
		jge VehicleReflectionLODCodeCaveEnable
		jmp VehicleReflectionLODCodeCaveDisable

	VehicleReflectionLODCodeCaveEnable:
		or edx, 0x00008002
		jmp VehicleReflectionLODCodeCaveExit
			
	VehicleReflectionLODCodeCaveDisable:
		or edx, 0x08002180
		jmp VehicleReflectionLODCodeCaveExit
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
	ImproveReflectionSkybox = iniReader.ReadInteger("GENERAL", "ImproveReflectionSkybox", 0);
	RealFrontEndReflections = iniReader.ReadInteger("GENERAL", "RealFrontEndReflections", 0);
	VehicleReflectionBrightness = iniReader.ReadFloat("GENERAL", "VehicleReflectionBrightness", 1.0);

	// Extra
	GammaFix = iniReader.ReadInteger("EXTRA", "GammaFix", 1);

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
		injector::MakeJMP(0x74489D, VehicleReflectionLODCodeCave, true);
		injector::MakeNOP(0x7448A2, 1, true);
	}

	if (ImproveReflectionSkybox)
	{
		// Vehicle Reflection Skybox Quality
		injector::MakeJMP(0x777104, VehicleSkyboxQualityCodeCave, true);
		injector::MakeNOP(0x777109, 1, true);
	}

	if (RealFrontEndReflections)
	{
		// Enables reflection mapping 
		injector::MakeNOP(0x4B8D1B, 2, true);
		injector::MakeNOP(0x4B8B8B, 2, true);
		injector::MakeNOP(0x4B8C66, 2, true);
		// Corrects brightness
		injector::WriteMemory<uint8_t>(0x701606, 0xEB, true);
		injector::MakeJMP(0x4B2095, VehicleSkyboxBrightnessCodeCave, true);
		injector::MakeNOP(0x4B209A, 3, true);
		injector::MakeJMP(0x4CA481, VehicleWorldBrightnessCodeCave, true);
		injector::MakeNOP(0x4CA486, 1, true);
		// Vehicle Reflection LOD
		injector::MakeJMP(0x74489D, VehicleReflectionLODCodeCave, true);
		injector::MakeNOP(0x7448A2, 1, true);
	}

	if (VehicleReflectionBrightness)
	{
		static double VehicleReflectionIntensity = (0.5f * VehicleReflectionBrightness);
		injector::WriteMemory(0x4B7AB3, &VehicleReflectionIntensity, true);
	}

	if (GammaFix)
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