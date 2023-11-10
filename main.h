#pragma once
#include <Windows.h>
#include <string>

#include "SAMP/SAMP.hpp"

#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_dx9.h"
#include "ImGUI/imgui_impl_win32.h"

bool menu = false;
bool bIsPluginInitialized = false;
bool btime = false;

#define fogdist 0x00B7C4F0
#define color 0x1FD5EDFF
namespace Save {
	int skin;
	int weather;
	int time;
	int lod;
	int fog = 50;
	bool bTimeChanger = false;
	bool bWeatChanger = false;
	bool bFogChanger = false;
	bool bFoggestChanger = false;
	bool bLodChanger = false;
	bool lodveh = false;
	int fVehlods = 200;
}
namespace Plugin {

	int aWrites[] = {
	0x555172 + 2, 0x555198 + 2, 0x5551BB + 2, 0x55522E + 2, 0x555238 + 2,
	0x555242 + 2, 0x5552F4 + 2, 0x5552FE + 2, 0x555308 + 2, 0x555362 + 2,
	0x55537A + 2, 0x555388 + 2, 0x555A95 + 2, 0x555AB1 + 2, 0x555AFB + 2,
	0x555B05 + 2, 0x555B1C + 2, 0x555B2A + 2, 0x555B38 + 2, 0x555B82 + 2,
	0x555B8C + 2, 0x555B9A + 2, 0x5545E6 + 2, 0x554600 + 2, 0x55462A + 2,
	0x5B527A + 2
	};


	void help_cmd(char* params) {
		SAMP::pSAMP->addMessageToChat(color, "[gamefixer] SetTime -> /st[0-23], SetWeather -> /sw[1-45], FogDist -> /fd[1-3600], LodDist -> /ld[10-999], Change Skin -> /ss[0-2892]");
	}
	void SetTime(int bHour, int bMinute) {
		BitStream bs;
		bs.Write<UINT8>(bHour);
		bs.Write<UINT8>(bMinute);
		//rakhook::send_rpc(29, &bs, PacketPriority::HIGH_PRIORITY, PacketReliability::RELIABLE_ORDERED, 0u, false);
		SAMP::pSAMP->getRakNet()->EmulRPC(29, &bs);
	}
	void time_cmd(char* szParams) {
		if (strlen(szParams) < 1)
			return SAMP::pSAMP->addMessageToChat(0x1FD5EDFF, "[gamefixer] set time -> /st [0-23]");
		Save::time = atoi(szParams);
		if (Save::time > 23 || Save::time < 0)
			return SAMP::pSAMP->addMessageToChat(color, "[gamefixer] usage /st [0-23]");
		SetTime(Save::time, 0);
		CClock::SetGameClock(Save::time, 0, 0);
		SAMP::pSAMP->addMessageToChat(color, "[gamefixer] time changed: %d", Save::time);
	}

	void Weather_cmd(char* szParams) {
		if (strlen(szParams) < 1)
			return SAMP::pSAMP->addMessageToChat(0x1FD5EDFF, "[gamefixer] set weather -> /sw [1-45]");
		Save::weather = atoi(szParams);
		if (Save::weather > 45 || Save::weather < 1) {
			return SAMP::pSAMP->addMessageToChat(0x1FD5EDFF, "[gamefixer] usage /sw [1-45]");
		}
		else {
			char szMessage[64];
			sprintf_s(szMessage, "[gamefixer] weather changed: %d", Save::weather);
			SAMP::pSAMP->addMessageToChat(0x1FD5EDFF, szMessage);
			CWeather::ForceWeatherNow(Save::weather);
		}
	}

	void LodDist_cmd(char* szParams) {

		unsigned long data = 0xCFFA11;
		int size = sizeof(Plugin::aWrites) / sizeof(Plugin::aWrites[0]);
		if (strlen(szParams) < 1)
			return SAMP::pSAMP->addMessageToChat(0x1FD5EDFF, "[gamefixer] change lod distance -> /ld[1-999], set default -> /ld 0");
		Save::lod = atoi(szParams);
		if (Save::lod <= 0) {
			*(float*)(0xCFFA11) = 300;
			for (int i = 0; i < size; i++) {
				WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(Plugin::aWrites[i]), &data, 4, NULL);
			}
			*(float*)(0xCFFA11) = 300;
			SAMP::pSAMP->addMessageToChat(color, "[gamefixer] lod distance: default");
		}
		if (Save::lod >= 1) {
			if (Save::lod >= 999)
				Save::lod = 999;
			char szMessage[64];
			sprintf_s(szMessage, "[gamefixer] lod distance:%d", Save::lod);
			SAMP::pSAMP->addMessageToChat(color, szMessage);
			*(float*)(0xCFFA11) = Save::lod;
			for (int i = 0; i < size; i++) {
				WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(Plugin::aWrites[i]), &data, 4, NULL);
			}
			*(float*)(0xCFFA11) = Save::lod;
		}
	}
	void FogDist_cmd(char* szParams) {
		if (strlen(szParams) < 1)
			return SAMP::pSAMP->addMessageToChat(color, "[gamefixer] change fog distance -> /fd [1-3600]");
		Save::fog = atoi(szParams);
		if (Save::fog > 3600 || Save::fog < 1) {
			return SAMP::pSAMP->addMessageToChat(color, "[gamefixer] usage /fd[1-3600]");
		}
		else {
			char szMessage[64];
			sprintf_s(szMessage, "[gamefixer] fogdist changed: %d", Save::fog);
			SAMP::pSAMP->addMessageToChat(color, szMessage);
			*(float*)(fogdist) = Save::fog;

		}
	}
	void ChangeSkin_cmd(char* szParams) {
		if (strlen(szParams) < 1)
			return SAMP::pSAMP->addMessageToChat(color, "[gamefixer] usage /ss [id]");
		Save::skin = atoi(szParams);
		BitStream bs;
		bs.Write<UINT32>(SAMP::pSAMP->getPlayers()->sLocalPlayerID);
		bs.Write<UINT32>(Save::skin);
		SAMP::pSAMP->getRakNet()->EmulRPC(RPC_ScrSetPlayerSkin, &bs);
		SAMP::pSAMP->addMessageToChat(color, "[gamefixer] skin changed: %d", Save::skin);
	}
	inline auto pauseScreen(bool state) {
		static DWORD
			updateMouseProtection,
			rsMouseSetPosProtFirst,
			rsMouseSetPosProtSecond;

		if (state) {
			::VirtualProtect(reinterpret_cast<void*>(0x53F3C6U), 5U, PAGE_EXECUTE_READWRITE, &updateMouseProtection);
			::VirtualProtect(reinterpret_cast<void*>(0x53E9F1U), 5U, PAGE_EXECUTE_READWRITE, &rsMouseSetPosProtFirst);
			::VirtualProtect(reinterpret_cast<void*>(0x748A1BU), 5U, PAGE_EXECUTE_READWRITE, &rsMouseSetPosProtSecond);

			*reinterpret_cast<uint8_t*>(0x53F3C6U) = 0xE9U;
			*reinterpret_cast<uint32_t*>(0x53F3C6U + 1U) = 0x15BU;

			memset(reinterpret_cast<void*>(0x53E9F1U), 0x90, 5U);
			memset(reinterpret_cast<void*>(0x748A1BU), 0x90, 5U);
		}
		else {
			memcpy(reinterpret_cast<void*>(0x53F3C6U), "\xE8\x95\x6C\x20\x00", 5U);
			memcpy(reinterpret_cast<void*>(0x53E9F1U), "\xE8\xAA\xAA\x0D\x00", 5U);
			memcpy(reinterpret_cast<void*>(0x748A1BU), "\xE8\x80\x0A\xED\xFF", 5U);

			using CPad_ClearMouseHistory_t = void(__cdecl*)();
			using CPad_UpdatePads_t = void(__cdecl*)();
			reinterpret_cast<CPad_ClearMouseHistory_t>(0x541BD0U)();
			reinterpret_cast<CPad_UpdatePads_t>(0x541DD0U)();

			::VirtualProtect(reinterpret_cast<void*>(0x53F3C6U), 5U, updateMouseProtection, &updateMouseProtection);
			::VirtualProtect(reinterpret_cast<void*>(0x53E9F1U), 5U, rsMouseSetPosProtFirst, &rsMouseSetPosProtFirst);
			::VirtualProtect(reinterpret_cast<void*>(0x748A1BU), 5U, rsMouseSetPosProtSecond, &rsMouseSetPosProtSecond);
		} return;
	}
	void gf_cmd(char* szParams) {
		menu ^= true;
		pauseScreen(menu);
	}
	void vehlods(char* szParams) {
		Save::lodveh = !Save::lodveh;
		if (Save::lodveh) {
			unsigned long data = 0x732924 + 2;
			unsigned long address = 0xCB0900;
			*(float*)(address) = Save::fVehlods;
			WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(data), &address, 4, NULL);
		}
		else {
			unsigned long address = 0xCB0900;
			*(float*)(address) = 200;

		}

	}

}





extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);