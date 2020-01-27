// eventual future plans
// 1. make the base pointer not coded in 12 different spots
// 2. fix the mess that is the status determiner
// 3. recode into rust 
// 4. maybe make it not rely on the api for level info, i have most of the pointers

#include "pch.h"
#include "gdapi.h"
#include <ctime>
#include "DRPWrap.h"
#include <algorithm>

#pragma message("gg!")

// future funk
constexpr int GDBaseP = 0x3222D0;

void printDebug(std::string title, std::string message) {
#ifdef _DEBUG
  std::cout << "[" << title << "] " << message << std::endl;
#endif
}

int* getBase(int pointer) {
  return (int *)((int)GetModuleHandle(L"GeometryDash.exe") + pointer); // why the heck did i make this
}

bool isInLevel() {
	int* part = (int*)(*getBase(GDBaseP) + 0x164);
	return !(IsBadReadPtr((DWORD*)(*part + 0x22C), 4) != 0);
}

bool isInEditor() {
	int* part = (int*)(*getBase(GDBaseP) + 0x168);
	return !(IsBadReadPtr((DWORD*)(*part + 0x354), 4) != 0);
}

/* 
// this is literally never used except for debugging
bool isOnMenu() {
	return (!isInLevel() && !isInEditor());
}
*/

// if i made this into a  struct or something hmmm
int *getCurrentLevelP() {
	if (!isInLevel()) {
		return 0;
	}
	int* part = (int*)(*getBase(GDBaseP) + 0x164);
	part = (int*)(*part + 0x22C);
	part = (int*)(*part + 0x114);
	return part;
}

/* this is a superior check, beats all the checks
 why is this superior, you may ask
 as opposed to my last method of getting id, this gets the id of any level
 weekly, official level, etc.
*/
int getCurrentID() {
	if (!isInLevel()) {
		return -1;
	}
	int* part = getCurrentLevelP();
	part = (int*)(*part + 0xF8);
	return *part;
}

int getBestPercent() {
	if (!isInLevel()) {
		return 0;
	}
	int* part = getCurrentLevelP();
	part = (int*)(*part + 0x248);
	return *part;
}

// we only check for value 2, level in CCLocalLevels
// and val 1, official level (val 3 is CCGameManager or saved)
int getLevelLocation() {
	if (!isInLevel()) {
		return 0;
	}
	int* part = getCurrentLevelP();
	part = (int*)(*part + 0x364);
	return *part;
}

//insane_demon to Insane Demon
std::string getTextFromKey(std::string key) {
	key[0] = toupper(key[0]); // uppercase first letter
	int index = 1; // start from 1 because we start string from 1, and also because you can't get an index at -1 like it'll try to do..
	std::for_each(key.begin() + 1, key.end(), [&index, &key](char& letter) {
		// in any case, this checks if it's a space before
		if (key[index-1] == ' ') {
			letter = toupper(letter); // then capitalizes
		}
		else if (letter == '_') { // if underscore it goes and remoes it
			letter = ' ';
		}
		index++; // then we use this to count
		});
	return key;
}

DWORD WINAPI actualMain(LPVOID lpParam) {
   	#ifdef _DEBUG
		AllocConsole();  // attaches console if ya want to do some debug stuff
		SetConsoleTitleA("rpc");
		FILE* fp;
		freopen_s(&fp, "CONOUT$", "w", stdout);
	#endif

	printDebug("Compiled", std::string(__DATE__) + " " + std::string(__TIME__));
	printDebug("Info", "Please wait...");

	DRP::InitDiscord();
	Discord_RunCallbacks();

	// guess this just waits for discord
	Sleep(5000);

	// this +8 behavior persists from 1.9, for some reason
	int* accountID = (int *)(*getBase(GDBaseP+0x8) + 0x120);

	GDuser user;
	bool userInfoSuccess = getUserInfo(*accountID, user);
	if (userInfoSuccess) {
		getUserRank(user);
	}
	#ifdef _DEBUG
	else {
		printDebug("User", "Failure getting user rank!");
	}
	#endif

	std::string details;
	std::string state;
	std::string smallText;
	std::string smallImage;
	std::string largeText = "Invalid user...";
	
	if (user.rank != -1) {
		largeText = user.name + " [Rank #" + std::to_string(user.rank) + "]";
	} else {
		largeText = user.name + " [Leaderboard Banned]";
	}

	printDebug("largeText", largeText);

	// small protection in case of thread stays open sometimes
	HWND gd = FindWindow(0, L"Geometry Dash");
	// get process
	DWORD gdpid;
	GetWindowThreadProcessId(gd, &gdpid);
	HANDLE gdproc =
		OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, gdpid);
	DWORD appStatus;

	GDlevel currentLevel;

	std::string oldDetails;
	std::string oldState;

	time_t currentTimestamp = time(0);

	int lastID = -1;

	while (true) {

		Discord_RunCallbacks();

		if (DRP::getPresenceStatus() != -1 && DRP::getPresenceStatus() != 0) {
			printDebug("Discord", std::to_string(DRP::getPresenceStatus()));
			if (DRP::getPresenceStatus() != 1) {
				MessageBoxA(0, "Discord broke lol...",
					"rpcrpcrpcrcp", MB_ICONERROR | MB_OK);
			}
			return 0;
		};

		bool getAppStatus = GetExitCodeProcess(gdproc, &appStatus);
		if (!getAppStatus ||
			appStatus != STILL_ACTIVE) {  // my guess is the dll never fully shuts down, it crashes so hopefully this helps in cases of dll never stopping
			printDebug("Info", "DLL Shutdown...");
			return 0;
		}

		if (isInLevel()) {
			if (lastID != getCurrentID()) { // avoid redoing string stuff every time the loop goes through
				switch (getLevelLocation()) {
				case 1:
					details = "Playing an official level";
					getOfficialInfo(getCurrentID(), currentLevel);
					state = currentLevel.name + " (Best: " + std::to_string(getBestPercent()) + "%)";
					smallImage = getDifficultyName(currentLevel);
					smallText = std::to_string(currentLevel.stars) + "* " + getTextFromKey(getDifficultyName(currentLevel));
					break;
				case 2: // editing level but playing it
					details = "Editing a level";
					state = "";
					smallImage = "creator_point";
					smallText = "";
					break;
				default:
				case 3:
					details = "Playing a level";
					bool levelStatus = getLevel(getCurrentID(), currentLevel);
					if (!levelStatus) {
						smallImage = "";
						smallText = "";
						state = "Best: " + std::to_string(getBestPercent()) + "%";
					}
					else {
						smallImage = getDifficultyName(currentLevel);
						smallText = std::to_string(currentLevel.stars) + "* " + getTextFromKey(getDifficultyName(currentLevel));
						state = currentLevel.author + " - " + currentLevel.name + " (Best: " + std::to_string(getBestPercent()) + "%)";
					}
					break;
				}
				lastID = getCurrentID();
			}
		}
		else if(isInEditor()) {
			if (details != "Editing a level") {
				details = "Editing a level";
				state = "";
				smallImage = "creator_point";
				smallText = "";
			}
		}
		else {
			if (details != "Idle") {
				details = "Idle";
				state = "";
				smallImage = "";
				smallText = "";
			}
		}
		
		if (oldDetails != details || oldState != state) { //update if details change
			#ifdef _DEBUG
				printDebug("Details", details);
				printDebug("State", state);
				printDebug("Small image", smallImage);
				printDebug("Small text", smallText);
			#endif
			if (oldDetails != details) {
				currentTimestamp = time(0);
				printDebug("Timestamp", std::to_string(currentTimestamp));
			}
			DRP::UpdatePresence(details.c_str(), largeText.c_str(), smallText.c_str(),
                     state.c_str(), smallImage.c_str(), currentTimestamp);
			oldDetails = details;
			oldState = state;
		}

		Sleep(1500);
	}

  return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
		DisableThreadLibraryCalls(hModule);
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)actualMain, NULL, NULL, NULL);
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

