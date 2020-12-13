#include "stdafx.h"
#include "stdio.h"
#include <windows.h>
#include "..\includes\injector\injector.hpp"
#include <cstdint>
#include "..\includes\IniReader.h"
#include <d3d9.h>

bool HDReflections, OldGPUCompatibility, ImproveReflectionLOD, ExtendRenderDistance, RealFrontEndReflections, GammaFix, RealisticChrome;
int ImproveReflectionSkybox;
int VehicleRes = 256;
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
DWORD VehicleHorizonQualityCodeCaveExit = 0x754706;
DWORD VehicleSkyboxQualityCodeCaveExit1 = 0x77710A;
DWORD VehicleSkyboxQualityCodeCaveExit2 = 0x7771E0;
DWORD VehicleReflectionLODCodeCaveExit = 0x7448A3;
DWORD RealisticChromeCodeCaveExit = 0x4B7939;
DWORD ExtendRenderDistanceCodeCaveExit = 0x4BDEA8;

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

void __declspec(naked) VehicleHorizonQualityCodeCave()
{
	__asm {
		sub esp, 0x84
		push eax
		push ebx
		cmp dword ptr ds : [0xA83BCC] , 0x00
		je VehicleHorizonQualityCodeCave_None // jumps if Skybox Pointer is null
		mov eax, dword ptr ds : [0xA83BCC]
		mov eax, dword ptr ds : [eax + 0x2C]
		mov eax, dword ptr ds : [eax + 0x08] // Writes "XX_PAN_360_01_D" Hash to EAX
		cmp dword ptr ds : [0xA83BE4], 0x00
		je VehicleHorizonQualityCodeCave_None // jumps if ENVMAP Skybox Pointer is null
		mov ebx, dword ptr ds : [0xA83BE4]
		mov ebx, dword ptr ds : [ebx + 0x2C]
		mov dword ptr ds : [ebx + 0x08], eax // Overwrites "XX_PAN_CAR360_01_D" Hash

	VehicleHorizonQualityCodeCave_None:
		pop ebx
		pop eax
		jmp VehicleHorizonQualityCodeCaveExit
	}
}

void __declspec(naked) VehicleSkyboxQualityCodeCave()
{
	__asm 
	{
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
	__asm 
	{
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

void __declspec(naked) RealisticChromeCodeCave()
{
	__asm 
	{
		mov esi, dword ptr ds : [0xA9E578]
		mov dword ptr ds : [esi + 0x2E9C], 0x3F800000 // chrome materiel reflectivity (1.0)
		mov dword ptr ds : [esi + 0x2E8C], 0x3F800000 // chrome materiel reflectivity (1.0)
		mov dword ptr ds : [esi + 0x2E10], 0x3E800000 // chrome materiel brightness (0.25)
		mov dword ptr ds : [esi + 0x2E00], 0x00000000 // chrome materiel brightness (0.0)
		mov esi, dword ptr ds : [ecx + 0x08]
		fld dword ptr ds : [esi + 0x64]
		jmp RealisticChromeCodeCaveExit
	}
}

void __declspec(naked) ExtendRenderDistanceCodeCave()
{
	__asm 
	{
		mov dword ptr ds : [esi + 0x164], 0x461C4000 // 10000.0f
		jmp ExtendRenderDistanceCodeCaveExit
	}
}

void Init()
{
	// Read values from .ini
	CIniReader iniReader("NFSPSHDReflections.ini");

	// Resolution
	HDReflections = iniReader.ReadInteger("RESOLUTION", "HDReflections", 1);
	OldGPUCompatibility = iniReader.ReadInteger("RESOLUTION", "OldGPUCompatibility", 0);
	Scale = iniReader.ReadFloat("RESOLUTION", "Scale", 1.0f);

	// General
	ImproveReflectionLOD = iniReader.ReadInteger("GENERAL", "ImproveReflectionLOD", 1);
	ImproveReflectionSkybox = iniReader.ReadInteger("GENERAL", "ImproveReflectionSkybox", 1);
	ExtendRenderDistance = iniReader.ReadInteger("GENERAL", "ExtendRenderDistance", 0);
	RealFrontEndReflections = iniReader.ReadInteger("GENERAL", "RealFrontEndReflections", 0);
	VehicleReflectionBrightness = iniReader.ReadFloat("GENERAL", "VehicleReflectionBrightness", 1.0f);

	// Extra
	GammaFix = iniReader.ReadInteger("EXTRA", "GammaFix", 1);
	RealisticChrome = iniReader.ReadInteger("EXTRA", "RealisticChrome", 0);

	if (HDReflections)
	{
		VehicleRes = ::GetSystemMetrics(SM_CYSCREEN);
	}

	// Writes Resolution Values
	{
		// Vehicle Reflection
		injector::WriteMemory<uint32_t>(0x4BD062, VehicleRes * Scale, true);
		injector::WriteMemory<uint32_t>(0x4BD24D, VehicleRes * Scale, true);
		injector::WriteMemory<uint32_t>(0x4BD283, VehicleRes * Scale, true);
		injector::WriteMemory<uint32_t>(0x4BD288, VehicleRes * Scale, true);

		if (OldGPUCompatibility)
		{
			// Rounds vehicle resolution down to the nearest power of two
			static int VehicleRes_POT = (VehicleRes * Scale);
			VehicleRes_POT--;
			VehicleRes_POT |= VehicleRes_POT >> 1;
			VehicleRes_POT |= VehicleRes_POT >> 2;
			VehicleRes_POT |= VehicleRes_POT >> 4;
			VehicleRes_POT |= VehicleRes_POT >> 8;
			VehicleRes_POT |= VehicleRes_POT >> 16;
			VehicleRes_POT++;
			injector::WriteMemory<uint32_t>(0x4BD062, VehicleRes_POT / 2, true);
			injector::WriteMemory<uint32_t>(0x4BD24D, VehicleRes_POT / 2, true);
			injector::WriteMemory<uint32_t>(0x4BD283, VehicleRes_POT / 2, true);
			injector::WriteMemory<uint32_t>(0x4BD288, VehicleRes_POT / 2, true);
		}
	}

	if (ImproveReflectionLOD)
	{
		// Vehicle Reflection LOD
		injector::MakeJMP(0x74489D, VehicleReflectionLODCodeCave, true);
		injector::MakeNOP(0x7448A2, 1, true);
	}

	if (ImproveReflectionSkybox)
	{
		// Vehicle Reflection Horizon Quality
		injector::MakeJMP(0x754700, VehicleHorizonQualityCodeCave, true);
		injector::MakeNOP(0x754705, 1, true);

		if (ImproveReflectionSkybox >= 2)
		{
			// Vehicle Reflection Skybox Quality
			injector::MakeJMP(0x777104, VehicleSkyboxQualityCodeCave, true);
			injector::MakeNOP(0x777109, 1, true);
		}
	}

	if (ExtendRenderDistance)
	{
		// Vehicle Reflection Render Distance
		injector::MakeJMP(0x4BDEA0, ExtendRenderDistanceCodeCave, true);
		injector::MakeNOP(0x4BDEA5, 3, true);
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

	if (RealisticChrome)
	{
		// Chrome materiel
		injector::MakeJMP(0x4B7933, RealisticChromeCodeCave, true);
		injector::MakeNOP(0x4B7938, 1, true);
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