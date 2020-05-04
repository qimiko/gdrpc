#include "pch.h"
#include "gdapi.h"

difficulty getDiffValue(int diff)
{
	switch (diff)
	{
	case 10:
		return difficulty::easy;
	case 20:
		return difficulty::normal;
	case 30:
		return difficulty::hard;
	case 40:
		return difficulty::harder;
	case 50:
		return difficulty::insane;
	case 0:
	default:
		return difficulty::na;
	}
}

demon_difficulty getDemonDiffValue(int diff)
{
	switch (diff)
	{
	case 3:
		return demon_difficulty::easy;
	case 4:
		return demon_difficulty::medium;
	case 5:
		return demon_difficulty::insane;
	case 6:
		return demon_difficulty::extreme;
	case 0:
	case 1:
	case 2:
	default:
		return demon_difficulty::hard;
	}
}

std::string getDifficultyName(GDlevel &level)
{
	// for some reason auto/demon levels don't have a proper difficulty
	if (level.isAuto)
	{
		return "auto";
	}

	// this is also messy
	if (level.isDemon)
	{
		switch (level.demonDifficulty)
		{
		case demon_difficulty::easy:
			return "easy_demon";
		case demon_difficulty::medium:
			return "medium_demon";
		case demon_difficulty::insane:
			return "insane_demon";
		case demon_difficulty::extreme:
			return "extreme_demon";
		default:
		case demon_difficulty::hard:
			return "hard_demon";
		}
	}

	// definitely a better way to do this
	switch (level.difficulty)
	{
	case difficulty::easy:
		return "easy";
	case difficulty::normal:
		return "normal";
	case difficulty::hard:
		return "hard";
	case difficulty::harder:
		return "harder";
	case difficulty::insane:
		return "insane";
	case difficulty::na:
	default:
		return "na";
	}
}

DWORD makeRequest(std::string data, LPCWSTR url, std::string &response)
{
	// this function has become magic dust for me lol

	static LPCWSTR hdrs = L"Content-Type: application/x-www-form-urlencoded";

	static LPCWSTR accept[2] = {L"*/*", NULL};

	HINTERNET hSession = InternetOpen(
			L"GDApi/WinInet", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0); // first part is user agent, change it to whatev
	HINTERNET hConnect = InternetConnectA(hSession, "boomlings.com", // i could explain how web works but no. boomlings.com is the host
																			 INTERNET_DEFAULT_HTTP_PORT, NULL, NULL,
																			 INTERNET_SERVICE_HTTP, 0, 1);
	HINTERNET hRequest = HttpOpenRequest(hConnect, L"POST", url, NULL, NULL, accept, 0, 1); // this starts the send something

	// send request
	if (!HttpSendRequest(hRequest, hdrs, (DWORD)wcslen(hdrs), (LPVOID)data.c_str(), (DWORD)strlen(data.c_str())))
	{
		DWORD dwErr = GetLastError(); // no error handling :o
		return dwErr;
	}
	std::stringstream responseStringStream;

	CHAR szBuffer[1025];
	DWORD dwRead = 0;

	while (::InternetReadFile(hRequest, szBuffer, sizeof(szBuffer) - 1, &dwRead) && dwRead)
	{ // chunk output i guess
		szBuffer[dwRead] = 0;
		OutputDebugStringA(szBuffer);
		responseStringStream << szBuffer;
		dwRead = 0;
	}

	::InternetCloseHandle(hRequest);
	::InternetCloseHandle(hSession); // we done

	response = responseStringStream.str();

	return 0;
}

bool getUserInfo(int &accID, GDuser &user)
{
	std::string frmdata = "gameVersion=21&secret=Wmfd2893gb7&targetAccountID=";
	frmdata.append(std::to_string(accID));

	std::string userString;
	DWORD responseCode = makeRequest(frmdata, L"/database/getGJUserInfo20.php", userString);
	if (responseCode != 0)
	{
		return false;
	}
	std::vector<std::string> splitlist = explode(userString, ':');

	if (splitlist.size() == 1 || splitlist.size() == 0)
	{
		user.name = "invalid";
		user.ID = -1;
		user.accID = -1;
		return false;
	}

	for (unsigned int i = 0; i < splitlist.size(); i++)
	{
		if (splitlist[i] == "1")
		{
			user.name = splitlist[i + 1];
		}
		if (splitlist[i] == "2")
		{
			user.ID = atoi(splitlist[i + 1].c_str());
		}
		if (splitlist[i] == "16")
		{
			user.accID = atoi(splitlist[i + 1].c_str());
		}
		i++;
	}

	return true;
}

bool getPlayerInfo(int &playerID, GDuser &user)
{
	std::string frmdata = "gameVersion=21&secret=Wmfd2893gb7&str=";
	frmdata.append(std::to_string(playerID));

	std::string playerString;
	DWORD responseCode = makeRequest(frmdata, L"/database/getGJUsers20.php", playerString);
	if (responseCode != 0)
	{
		return false;
	}
	std::vector<std::string> splitlist = explode(playerString, ':');

	if (splitlist.size() == 1 || splitlist.size() == 0)
	{
		user.name = "invalid";
		user.ID = -1;
		user.accID = -1;
		return false;
	}

	for (unsigned int i = 0; i < splitlist.size(); i++)
	{
		if (splitlist[i] == "1")
		{
			user.name = splitlist[i + 1];
		}
		if (splitlist[i] == "2")
		{
			user.ID = atoi(splitlist[i + 1].c_str());
		}
		if (splitlist[i] == "16")
		{
			user.accID = atoi(splitlist[i + 1].c_str());
		}
		i++;
	}

	return true;
}

bool getUserRank(GDuser &user)
{
	std::string frmdata = "gameVersion=21&secret=Wmfd2893gb7&type=relative&count=1&accountID=";
	frmdata.append(std::to_string(user.accID));

	std::string leaderboardString;
	DWORD responseCode = makeRequest(frmdata, L"/database/getGJScores20.php", leaderboardString);
	if (responseCode != 0)
	{
		return false;
	}

	if (leaderboardString == "-1")
	{
		return false;
	}

	std::vector<std::string> splitlist = explode(leaderboardString, '|');

	if (splitlist.size() == 1 || splitlist.size() == 0)
	{
		return false;
	}

	// testing shows its the 24st element, if its not valid there's going to be
	// some checks lol
	bool foundUser = false;
	std::vector<std::string> seglist;

	if (splitlist.size() >= 24)
	{
		seglist = explode(splitlist[24], ':');
		foundUser = (atoi(seglist[16].c_str()) == user.accID);
	}

	if (!foundUser)
	{ // hey look its the checks

		// so basically you look for
		// :16:<id>:
		std::string idLookup = ":16:" + std::to_string(user.accID) + ":";

		unsigned int i;

		for (i = 0; i < splitlist.size() - 1; i++)
		{
			if (splitlist[i].find(idLookup) != std::string::npos)
			{
				break;
			}
		}
		if (i == 0)
		{
			user.rank = -1;
			return false;
		}

		seglist = explode(splitlist[i], ':');
	}

	for (unsigned int i = 0; i < seglist.size(); i++)
	{
		/*if (seglist[i] == "1") {
			user.name = seglist[i + 1];
		}
		// else ifs seem to do weird things
		if (seglist[i] == "16") {
			user.accID = atoi(seglist[i + 1].c_str());
		}*/
		if ((seglist[i] == "6"))
		{
			user.rank = atoi(seglist[i + 1].c_str()); // this one doesn't like string to integer conversion for some reason
		}
		/*if (seglist[i] == "2") {
			user.ID = atoi(seglist[i + 1].c_str());
		}*/
		i++;
	}

	return true;
}

// reads string at address
// does proper length checks
std::string readString(int * addr) {
	int length = *(int *)((int)addr + 0x10);
	if (length >= 16) {
		return std::string((char *)(*addr));
	} else {
		return std::string((char *)addr);
	}
}

// i was going to make a joke about this being the better thing like the id parsing
// but this is _really_ messy
// no error handling either. good luck
bool parseGJGameLevel(int * gameLevel, GDlevel &level) {
	int newID = *(int *)((int)gameLevel + 0xF8);

	if (newID == level.levelID) { // don't calculate more than we have to
		return true;
	}

	level.levelID = newID;
	level.stars = *(int *)((int)gameLevel + 0x2AC);

	level.name = readString((int *)((int)gameLevel + 0xFC));

	int levelLocation = *(int *)((int)gameLevel + 0x364);
	if (levelLocation == 1) {
			level.author = "RobTop"; // author is "" on these
			int diffValue = *(int *)((int)gameLevel + 0x1BC)*10;
			if (diffValue == 60) {
				level.isDemon = true;
				level.difficulty = difficulty::insane;
				level.demonDifficulty = demon_difficulty::easy;
			} else {
				level.isDemon = false;
				level.difficulty = getDiffValue(diffValue);
			}
	} else {
		level.author = readString((int *)((int)gameLevel + 0x144));
		level.difficulty = getDiffValue(*(int *)((int)gameLevel + 0x1E4));

		// don't have the proper booleans mapped
		level.isDemon = (level.stars >= 10); // can't tell if this is more readable or not
		level.isAuto = (level.stars == 1);

		if (level.isDemon) {
			level.demonDifficulty = getDemonDiffValue(*(int *)((int)gameLevel + 0x2A0));
		}
	}
	return true;
}

//helper function
std::vector<std::string> explode(std::string &string, char separator)
{
	std::stringstream segmentstream(string);
	std::string segmented;
	std::vector<std::string> splitlist;

	while (std::getline(segmentstream, segmented, separator))
	{
		splitlist.push_back(segmented);
	}

	return splitlist;
}