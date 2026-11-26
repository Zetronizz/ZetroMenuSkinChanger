#include "pch-il2cpp.h"
#include "menu.hpp"
#include "imgui/imgui.h"
#include "tabs/about_tab.h"
#include "tabs/doors_tab.h"
#include "tabs/game_tab.h"
#include "tabs/host_tab.h"
#include "tabs/players_tab.h"
#include "tabs/radar_tab.h"
#include "tabs/replay_tab.h"
#include "tabs/esp_tab.h"
#include "tabs/sabotage_tab.h"
#include "tabs/self_tab.h"
#include "tabs/settings_tab.h"
#include "tabs/tasks_tab.h"
#ifdef _DEBUG
#include "tabs/debug_tab.h"
#endif
#include "state.hpp"
#include "gui-helpers.hpp"

namespace Menu {
	static bool openAbout = false;
	static bool openSettings = false;
	static bool openGame = false;
	static bool openSelf = false;
	static bool openRadar = false;
	static bool openReplay = false;
	static bool openEsp = false;
	static bool openPlayers = false;
	static bool openTasks = false;
	static bool openSabotage = false;
	static bool openDoors = false;
	static bool openHost = false;
#ifdef _DEBUG
	static bool openDebug = false;
#endif

	struct SearchEntry {
		std::string Name;
		std::string SubGroup;
	};

	std::map<std::string, std::vector<SearchEntry>> categories = {
		// Поиск оставлен пустым, чтобы скрытые вкладки не открывались через него
	};

	void CloseAllOtherTabs(Tabs openTab) {
		openAbout = openTab == Tabs::About;
		openSettings = openTab == Tabs::Settings;
		openGame = openTab == Tabs::Game;
		openSelf = openTab == Tabs::Self;
		openRadar = openTab == Tabs::Radar;
		openReplay = openTab == Tabs::Replay;
		openEsp = openTab == Tabs::Esp;
		openPlayers = openTab == Tabs::Players;
		openTasks = openTab == Tabs::Tasks;
		openSabotage = openTab == Tabs::Sabotage;
		openDoors = openTab == Tabs::Doors;
		openHost = openTab == Tabs::Host;
#ifdef _DEBUG
		openDebug = openTab == Tabs::Debug;
#endif
	}

	void Init() {
		ImGui::SetNextWindowSize(ImVec2(600, 450) * State.dpiScale, ImGuiCond_None);
		ImGui::SetNextWindowBgAlpha(State.MenuThemeColor.w);
	}

	bool init = false;
	bool firstRender = true;
	bool isPanicWarning = false;

	std::string ToLower(const std::string& str) {
		std::string lowerStr = str;
		std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
		return lowerStr;
	}

	static std::string StrRev(std::string str) {
		std::string new_str = str;
		std::reverse(new_str.begin(), new_str.end());
		return new_str;
	}

	Tabs CategoryNameToTab(const std::string& name) {
		if (name == "Settings") return Tabs::Settings;
		if (name == "Game") return Tabs::Game;
		if (name == "Self") return Tabs::Self;
		if (name == "Radar") return Tabs::Radar;
		if (name == "Replay") return Tabs::Replay;
		if (name == "ESP") return Tabs::Esp;
		if (name == "Players") return Tabs::Players;
		if (name == "Tasks") return Tabs::Tasks;
		if (name == "Sabotage") return Tabs::Sabotage;
		if (name == "Doors") return Tabs::Doors;
		if (name == "Host") return Tabs::Host;
#ifdef _DEBUG
		if (name == "Debug") return Tabs::Debug;
#endif
		return Tabs::About;
	}

	bool IsTabUsable(Tabs tab) {
		return tab == Tabs::About;
	}

	void OpenTabSubGroup(const std::string& tabName, const std::string& subGroup) {
		if (subGroup.empty()) return;
		if (tabName == "Settings") SettingsTab::OpenSubGroup(subGroup);
		else if (tabName == "Game") GameTab::OpenSubGroup(subGroup);
		else if (tabName == "Self") SelfTab::OpenSubGroup(subGroup);
		else if (tabName == "Players") PlayersTab::OpenSubGroup(subGroup);
		else if (tabName == "Host") HostTab::OpenSubGroup(subGroup);
	}

	void RenderSearchResults() {
		if (State.searchQuery.size() == 0) return;
		std::string lowerQuery = ToLower(State.searchQuery);
		std::vector<std::pair<std::string, std::string>> searchResults = {};
		for (const auto& category : categories) {
			for (const auto& entry : category.second) {
#ifdef _DEBUG
				if (category.first == "Debug" && !State.showDebugTab) continue;
#endif
				if (ToLower(entry.Name).find(lowerQuery) != std::string::npos) {
					searchResults.push_back({ category.first, entry.SubGroup });
					break;
				}
			}
		}

		ImGui::TextColored(ImVec4(0.f, 0.f, 0.f, 0.f), "space");
		if (searchResults.size() == 0)
			BoldText("No results.");
		else {
			BoldText(("Search Result" + std::string(searchResults.size() == 1 ? "" : "s")).c_str());
			for (auto& [tabName, subGroup] : searchResults) {
				std::string label = tabName;
				if (ImGui::Selectable((label + "##searchresult").c_str())) {
					Tabs selectedTab = CategoryNameToTab(tabName);
					if (IsTabUsable(selectedTab)) {
						CloseAllOtherTabs(selectedTab);
						OpenTabSubGroup(tabName, subGroup);
					}
				}
			}
		}
	}

	void Render() {
		try {
			if (!init) Menu::Init();
			std::string modText = std::format("SickoMenu {}", State.SickoVersion);
			ImGui::Begin("SickoMenu", &State.ShowMenu, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollWithMouse);
			static ImVec4 titleCol = State.MenuThemeColor;
			if (State.RgbMenuTheme)
				titleCol = State.RgbColor;
			else
				titleCol = State.GradientMenuTheme ? State.MenuGradientColor : State.MenuThemeColor;
			titleCol.w = 1.f;
			ImGui::TextColored(titleCol, modText.c_str());

			ImGui::SameLine(ImGui::GetWindowWidth() - 19 * State.dpiScale);
			if (ImGui::Button("-")) State.ShowMenu = false;

			ImGui::BeginChild("###SickoMenu", ImVec2(90 * State.dpiScale, 0), true, ImGuiWindowFlags_NoBackground);

			// Search field
			ImGui::SetNextItemWidth(70 * State.dpiScale);
			InputStringWithHint("##Search", "Search...", &State.searchQuery);
			ImGui::SameLine(0.f, 2.f);
			if (ColoredButton(ImVec4(1.f, 0.f, 0.f, 1.f), "\u00D7")) {
				State.searchQuery = "";
			}

			// Оставлена только вкладка About
			if (ImGui::Selectable("About", openAbout)) {
				CloseAllOtherTabs(Tabs::About);
			}

			RenderSearchResults();

			ImVec4 PanicCol = ImVec4(1.f, 0.f, 0.f, 1.f);
			ImVec4 GreenCol = ImVec4(0.f, 1.f, 0.f, 1.f);
			if (!isPanicWarning) {
				ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - 90 * State.dpiScale, ImGui::GetWindowHeight() - 20 * State.dpiScale));
				if (ColoredButton(PanicCol, "Disable Menu")) {
					isPanicWarning = State.PanicWarning;
					if (!State.PanicWarning) {
						State.PanicMode = true;
						State.MIG_ThemeChanged = true;
						if (State.ControlPet) State.DisableControlPetHand = true;
						ReloadCurrentSceneIfNeeded();
					}
				}
			}
			else {
				bool panicKeybind = State.KeyBinds.Toggle_Sicko != 0x00;
				ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - 90 * State.dpiScale, ImGui::GetWindowHeight() - 65 * State.dpiScale));
				if (!panicKeybind) {
					ImGui::TextColored(PanicCol, "No Panic");
					ImGui::TextColored(PanicCol, "Keybind!");
				}
				else {
					ImGui::TextColored(PanicCol, ("Press " + (std::string)KeyBinds::ToString(State.KeyBinds.Toggle_Sicko)).c_str());
					ImGui::TextColored(PanicCol, ("to re-enable!"));
				}
				ImGui::TextColored(PanicCol, "Continue?");
				if (ColoredButton(PanicCol, "Yes")) {
					isPanicWarning = false;
					State.PanicMode = true;
					State.MIG_ThemeChanged = true;
					if (State.ControlPet) State.DisableControlPetHand = true;
					ReloadCurrentSceneIfNeeded();
				}
				ImGui::SameLine();
				if (ColoredButton(GreenCol, "No")) {
					isPanicWarning = false;
				}
			}

			if (firstRender) {
				firstRender = false;
				CloseAllOtherTabs(Tabs::About);
			}

			ImGui::EndChild();

			// Отрисовка содержимого вкладки About
			if (openAbout) AboutTab::Render();
			else {
				if (!State.HasOpenedMenuBefore) State.HasOpenedMenuBefore = true;
			}

			ImGui::End();
		}
		catch (...) {
			LOG_ERROR("Exception occurred when rendering menu");
		}
	}
}