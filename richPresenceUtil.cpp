#include "pch.h"
#include "richPresenceUtil.h"

playerState currentPlayerState = playerState::menu;
int *currentGameLevel;
bool updatePresence;
bool updateTimestamp;
time_t currentTimestamp = time(0);

int *getBase(int pointer)
{
	return (int *)((int)GetModuleHandle(L"GeometryDash.exe") + pointer);
}

void updatePresenceS(std::string &details, std::string &largeText, std::string &smallText,
											std::string &state, std::string &smallImage)
{
	if (updateTimestamp)
	{
		currentTimestamp = time(0);
		updateTimestamp = false;
	}
	#ifdef _DEBUG
		std::stringstream ss;
		ss << "d: " << details << " s: " << state << "\nst: "<< smallText << " lt: " << largeText;
		#ifdef __MINGW32__
			MessageBoxA(0, ss.str().c_str(), "b", MB_OK);
		#else
			std::cout << ss.str();
		#endif
	#endif
	DRP::UpdatePresence(details.c_str(), largeText.c_str(), smallText.c_str(),
											state.c_str(), smallImage.c_str(), currentTimestamp);
}

//insane_demon to Insane Demon
std::string getTextFromKey(std::string key)
{
	key[0] = toupper(key[0]); // uppercase first letter
	int index = 1;						// start from 1 because we start string from 1, and also because you can't get an index at -1 like it'll try to do..
	std::for_each(key.begin() + 1, key.end(), [&index, &key](char &letter) {
		// in any case, this checks if it's a space before
		if (key[index - 1] == ' ')
		{
			letter = toupper(letter); // then capitalizes
		}
		else if (letter == '_')
		{ // if underscore it goes and remoes it
			letter = ' ';
		}
		index++; // then we use this to count
	});
	return key;
}

DWORD WINAPI mainThread(LPVOID lpParam)
{
	std::cout << "Discord init...";
	DRP::InitDiscord();

	updatePresence = false;
	currentPlayerState = playerState::menu;

	int *accountID = (int *)(*getBase(0x3222D0 + 0x8) + 0x120);

	GDuser user;
	bool userInfoSuccess = getUserInfo(*accountID, user);
	if (userInfoSuccess)
	{
		getUserRank(user);
	}

	std::string details;
	std::string state;
	std::string smallText;
	std::string smallImage;
	std::string largeText = "Invalid user...";

	if (user.rank != -1)
	{
		largeText = user.name + " [Rank #" + std::to_string(user.rank) + "]";
	}
	else
	{
		largeText = user.name + " [Leaderboard Banned]";
	}

	int levelLocation, levelID, currentBest;
	GDlevel currentLevel;

	updatePresence = true;

	while (true) {
		// run discord calls
		Discord_RunCallbacks();
		if (updatePresence) {
			switch (currentPlayerState) {
				case playerState::level:
					levelLocation = *(int*)((int)currentGameLevel + 0x364);
					levelID = *(int *)((int)currentGameLevel + 0xF8);
					currentBest = *(int *)((int)currentGameLevel + 0x248);
					switch (levelLocation)
					{
					case 1:
						details = "Playing an official level";
						getOfficialInfo(levelID, currentLevel);
						state = currentLevel.name + " (Best: " + std::to_string(currentBest) + "%)";
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
						bool levelStatus = getLevel(levelID, currentLevel);
						if (!levelStatus)
						{
							smallImage = "";
							smallText = "";
							state = "Best: " + std::to_string(currentBest) + "%";
						}
						else
						{
							smallImage = getDifficultyName(currentLevel);
							smallText = std::to_string(currentLevel.stars) + "* " + getTextFromKey(getDifficultyName(currentLevel));
							state = currentLevel.author + " - " + currentLevel.name + " (Best: " + std::to_string(currentBest) + "%)";
						}
						break;
					}
					break;
				case playerState::editor:
					details = "Editing a level";
					state = "";
					smallImage = "creator_point";
					smallText = "";
					break;
				case playerState::menu:
					details = "Idle";
					state = "";
					smallImage = "";
					smallText = "";
					break;
			}
			updatePresenceS(details, largeText, smallText, state, smallImage);
			updatePresence = false;
		}
		Sleep(1000);
	}

	return 0;
}

