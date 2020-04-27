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

bool getLevel(int levelid, GDlevel &level)
{
	// string splitting

	if (levelid == level.levelID)
	{ // don't redo request
		return true;
	}

	std::string frmdata =
			"gameVersion=21&type=0&secret=Wmfd2893gb7&str=";

	frmdata.append(std::to_string(levelid));

	std::string levelString;

	DWORD responseCode = makeRequest(frmdata, L"/database/getGJLevels21.php", levelString);
	if (responseCode != 0)
	{
		return false;
	}

	// reset
	level.levelID = -1;
	level.stars = 0;
	level.name = "removed";
	level.author = "player";

	level.authorID = -1;

	level.difficulty = difficulty::na;
	level.demonDifficulty = demon_difficulty::none;
	level.isAuto = false;
	level.isDemon = false;

	// this is inefficient me
	std::vector<std::string> splitlist = explode(levelString, '#');

	if (splitlist.size() == 1 || splitlist.size() == 0)
	{
		return false;
	}

	std::vector<std::string> seglist = explode(splitlist[0], ':');
	std::vector<std::string> seg2list = explode(splitlist[1], ':');

	// reading the string
	for (unsigned int i = 0; i < seglist.size(); i++)
	{
		// i tried to use switch statements it was a mistake
		if (seglist[i] == "1")
		{
			level.levelID = atoi(seglist[i + 1].c_str());
		}
		// else ifs seem to do weird things
		if (seglist[i] == "2")
		{
			level.name = seglist[i + 1];
		}
		if (seglist[i] == "9")
		{
			level.difficulty = getDiffValue(atoi(seglist[i + 1].c_str())); // messy but 9 is diff int
		}
		if (seglist[i] == "6")
		{
			level.authorID = atoi(seglist[i + 1].c_str());
		}
		if (seglist[i] == "18")
		{
			level.stars = atoi(seglist[i + 1].c_str()); // this is because as it turns out 1 star can be easy
		}
		if (seglist[i] == "17")
		{
			level.isDemon = (seglist[i + 1] == "1"); // 1 is true
		}
		if (seglist[i] == "25")
		{
			level.isAuto = (seglist[i + 1] == "1");
		}
		if (seglist[i] == "43")
		{
			if (level.isDemon)
			{
				level.demonDifficulty = getDemonDiffValue(atoi(seglist[i + 1].c_str()));
			}
		}
		i++;
	}
	if (seg2list.size() >
			0)
	{ // check to prevent any weird errors by id not existing
		level.author = seg2list[1].c_str();
	}

	return true;
}

void getOfficialInfo(int id, GDlevel &level)
{

	if (id == level.levelID)
	{ // don't redo this either
		return;
	}

	// default attributes
	level.levelID = id;
	level.authorID = -1;
	level.author = "RobTop";
	level.isAuto = 0;

	// attributes if level not found
	level.stars = id; // this is the default case for the first 12 levels
	level.name = "Invalid!";
	level.difficulty = difficulty::na;
	level.isDemon = 0;

	switch (id)
	{ //messy thing, i apologize
	case 1:
		level.name = "Stereo Madness";
		level.difficulty = difficulty::easy;
		break;
	case 2:
		level.name = "Back on Track";
		level.difficulty = difficulty::easy;
		break;
	case 3:
		level.name = "Polargeist";
		level.difficulty = difficulty::normal;
		break;
	case 4:
		level.name = "Dry Out";
		level.difficulty = difficulty::normal;
		break;
	case 5:
		level.name = "Base after Base";
		level.difficulty = difficulty::hard;
		break;
	case 6:
		level.name = "Can't Let Go";
		level.difficulty = difficulty::hard;
		break;
	case 7:
		level.name = "Jumper";
		level.difficulty = difficulty::harder;
		break;
	case 8:
		level.name = "Time Machine";
		level.difficulty = difficulty::harder;
		break;
	case 9:
		level.name = "Cycles";
		level.difficulty = difficulty::harder;
		break;
	case 10:
		level.name = "xStep";
		level.difficulty = difficulty::insane;
		break;
	case 11:
		level.name = "Clutterfunk";
		level.difficulty = difficulty::insane;
		break;
	case 12:
		level.name = "Theory of Everything";
		level.difficulty = difficulty::insane;
		break;
	case 13:
		level.name = "Electroman Adventures";
		level.difficulty = difficulty::insane;
		level.stars = 10;
		break;
	case 14:
		level.name = "Clubstep";
		level.difficulty = difficulty::insane;
		level.demonDifficulty = demon_difficulty::easy;
		level.isDemon = true;
		break;
	case 15:
		level.name = "Electrodynamix";
		level.difficulty = difficulty::insane;
		level.stars = 12;
		break;
	case 16:
		level.name = "Hexagon Force";
		level.difficulty = difficulty::insane;
		level.stars = 12;
		break;
	case 17:
		level.name = "Blast Processing";
		level.difficulty = difficulty::harder;
		level.stars = 10;
		break;
	case 18:
		level.name = "Theory of Everything 2";
		level.difficulty = difficulty::insane;
		level.demonDifficulty = demon_difficulty::easy;
		level.stars = 14;
		level.isDemon = true;
		break;
	case 19:
		level.name = "Geometrical Dominator";
		level.difficulty = difficulty::harder;
		level.stars = 10;
		break;
	case 20:
		level.name = "Deadlocked";
		level.difficulty = difficulty::insane;
		level.demonDifficulty = demon_difficulty::easy;
		level.stars = 15;
		level.isDemon = true;
		break;
	case 21:
		level.name = "Fingerdash";
		level.difficulty = difficulty::insane;
		level.stars = 12;
		break;
	case 3001:
		level.name = "The Challenge";
		level.difficulty = difficulty::normal;
		level.stars = 3;
	}
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