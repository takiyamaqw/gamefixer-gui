#include "main.h"


auto __stdcall WndProcCallBack(SAMP::CallBacks::HookedStructs::stWndProcParams* params) -> LRESULT
{
	if (!bIsPluginInitialized)
		return 0;
	if (menu && params->uMsg == WM_KEYDOWN && params->wParam == VK_ESCAPE) {
		menu = false;
		Plugin::pauseScreen(false);
		return true;
	} ImGui_ImplWin32_WndProcHandler(params->hWnd, params->uMsg, params->wParam, params->lParam);

	return 0;
}

auto __stdcall D3DPresentHook(SAMP::CallBacks::HookedStructs::stPresentParams* params) -> HRESULT
{
	if (!bIsPluginInitialized)
		return D3D_OK;

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::GetIO().MouseDrawCursor = menu;
	if (menu) ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	if (menu) {
		ImGui::SetNextWindowPos({ ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f }, ImGuiCond_Once, { 0.5f, 0.5f });
		if (ImGui::Begin(u8"test menu", &menu, ImGuiWindowFlags_::ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar)) {
			Plugin::pauseScreen(true);

			ImGui::Checkbox(u8"Запретить серверу изменять время", &btime);
			if (btime) {
				unsigned long data = 0x000008C2; // if time changing is deny
				WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(SAMP::pSAMP->GetBase() + 0xA03A0), &data, 4, NULL); // r3
			}
			else {
				unsigned long data = 0x0824448B; // if time changing is allowed
				WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(SAMP::pSAMP->GetBase() + 0xA03A0), &data, 4, NULL); // r3
			}
			if (Save::bWeatChanger)
				CWeather::ForceWeatherNow(Save::weather); // plugin-sdk func

			if (Save::bTimeChanger)
				CClock::SetGameClock(Save::time, 0, 0); // plugin-sdk func
			ImGui::Checkbox(u8"Смена погоды", &Save::bWeatChanger); ImGui::SameLine(202); ImGui::Button(u8"Выбрать##Weather", { 70, 0 });
			if (ImGui::BeginPopupContextItem(0, 0))
			{
				if (ImGui::BeginMenuBar()) ImGui::TextUnformatted(u8"Смена погоды"), ImGui::EndMenuBar();
				ImGui::Checkbox(u8"Погода", &Save::bWeatChanger); ImGui::SameLine(100); ImGui::SameLine(150); ImGui::PushItemWidth(70); ImGui::SliderInt(u8"##Weather", &Save::weather, 0, 20);
				ImGui::Checkbox(u8"Время", &Save::bTimeChanger); ImGui::SameLine(150); ImGui::SliderInt(u8"##Time", &Save::time, 0, 24); ImGui::PopItemWidth();
				ImGui::EndPopup();

			}
			if (Save::bFogChanger)
				*(float*)(fogdist) = Save::fog;
			if (Save::bLodChanger) {
				unsigned long data = 0xCFFA11;
				int size = sizeof(Plugin::aWrites) / sizeof(Plugin::aWrites[0]);
				*(float*)(0xCFFA11) = Save::lod;
				for (int i = 0; i < size; i++) {
					WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(Plugin::aWrites[i]), &data, 4, NULL);
				}
				*(float*)(0xCFFA11) = Save::lod;
			}

			ImGui::Checkbox(u8"Дальность прорисовки", &Save::bFoggestChanger); ImGui::SameLine(202); ImGui::Button(u8"Выбрать##Fog", {70, 0});
			if (ImGui::BeginPopupContextItem(0, 0))
			{
				if (ImGui::BeginMenuBar()) ImGui::TextUnformatted(u8"Дальность прорисовки"), ImGui::EndMenuBar();
				ImGui::Checkbox(u8"Туман", &Save::bFogChanger); ImGui::SameLine(100); ImGui::SameLine(150); ImGui::PushItemWidth(70); ImGui::SliderInt(u8"##Fog", &Save::fog, 50, 3600);
				ImGui::Checkbox(u8"LOD's", &Save::bLodChanger); ImGui::SameLine(150); ImGui::SliderInt(u8"##Lod", &Save::lod, 1, 300); ImGui::PopItemWidth();
				if (ImGui::Button(u8"Восстановить по умолчанию")) {
					*(float*)(fogdist) = 300;
					Save::fog = 300;
					*(float*)(0xCFFA11) = 200;
					Save::lod = 200;
				}
				ImGui::EndPopup();

			}
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

			ImGui::Checkbox(u8"VehLods", &Save::lodveh); ImGui::SameLine(100); ImGui::SameLine(150); ImGui::PushItemWidth(70); ImGui::SliderInt(u8"##Vehlod", &Save::fVehlods, 0, 200);

			ImGui::Text("prod by takiyama");


			ImGui::End();
		}
		
	}
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	return D3D_OK;
}

auto __stdcall D3DResetHook(SAMP::CallBacks::HookedStructs::stResetParams* params) -> HRESULT
{
	if (!bIsPluginInitialized)
		return D3D_OK;

	ImGui_ImplDX9_InvalidateDeviceObjects();

	return D3D_OK;
}


auto __stdcall GameLoop(void) -> void
{
	if (!bIsPluginInitialized)
	{
		if (SAMP::pSAMP->LoadAPI())
		{
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			char font_dir[MAX_PATH] = { 0 };
			sprintf(font_dir, "%s\\Windows\\Fonts\\Arial.ttf", getenv("SystemDrive"));
			io.Fonts->AddFontFromFileTTF(font_dir, 16, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
			(void)io;
			ImGui_ImplWin32_Init(GetActiveWindow());
			ImGui_ImplDX9_Init(SAMP::CallBacks::pCallBackRegister->GetIDirect3DDevice9());
			ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
			ImGui::StyleColorsLight();



			SAMP::pSAMP->addClientCommand(".help", Plugin::help_cmd);
			SAMP::pSAMP->addClientCommand("st", Plugin::time_cmd);
			SAMP::pSAMP->addClientCommand("sw", Plugin::Weather_cmd);
			SAMP::pSAMP->addClientCommand("fd", Plugin::FogDist_cmd);
			SAMP::pSAMP->addClientCommand("ld", Plugin::LodDist_cmd);
			SAMP::pSAMP->addClientCommand("ss", Plugin::ChangeSkin_cmd);
			SAMP::pSAMP->addClientCommand(".menu", Plugin::gf_cmd);
			SAMP::pSAMP->addClientCommand("vehlods", Plugin::vehlods);
			SAMP::pSAMP->addMessageToChat(color, "[gamefixer] load^ /.help or /.menu");

			bIsPluginInitialized = true;
		}
	}
	else
	{
		//gameloop if plugin initialized

	}
}

int __stdcall DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
	{

		SAMP::Init();
		*SAMP::CallBacks::pCallBackRegister += GameLoop;
		*SAMP::CallBacks::pCallBackRegister += WndProcCallBack;
		*SAMP::CallBacks::pCallBackRegister += D3DPresentHook;
		*SAMP::CallBacks::pCallBackRegister += D3DResetHook;

		printf("\n -> Plugin loaded (%llu)\n", GetTickCount64());
		break;
	}
	case DLL_PROCESS_DETACH:
	{
		SAMP::ShutDown();

		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		printf("\n -> Plugin unloaded (%llu)\n", GetTickCount64());
		break;
	}
	}
	return TRUE;
}
