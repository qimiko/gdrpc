#include "pch.h"
#include "DRPWrap.h"

int rpcStatus = -1;  // -1 , not start

static void handleDiscordReady(const DiscordUser* connectedUser) {
  rpcStatus = 0;  // success is code of 0
  std::cout << "\nDiscord: connected to user " << connectedUser->username << "#"
			<< connectedUser->discriminator << " (" << connectedUser->userId
			<< ")";
}

static void handleDiscordDisconnected(int errcode, const char* message) {
  rpcStatus = errcode;
  std::cout << "\nDiscord: disconnected (" << errcode << " : " << message
			<< ")\n";
}

static void handleDiscordError(int errcode, const char* message) {
  rpcStatus = errcode;
  std::cout << "\nDiscord: error (" << errcode << " : " << message << ")\n";
}

static void handleDiscordJoinGame(const char* secret) { return; }

static void handleDiscordSpectateGame(const char* secret) { return; }

static void handleDiscordJoinRequest(const DiscordUser* request) {
  Discord_Respond(request->userId, DISCORD_REPLY_NO);
}
namespace DRP {

	int getPresenceStatus() { return rpcStatus; }

	void UpdatePresence(const char* details, const char* largeText,
		const char* smallText, const char* statetext,
		const char* smallImage, time_t timestamp) {
		DiscordRichPresence discordPresence;
		memset(&discordPresence, 0, sizeof(discordPresence));

		if (statetext != "") {
			discordPresence.state = statetext;
		}

		discordPresence.details = details;
		discordPresence.startTimestamp = timestamp;
		discordPresence.largeImageKey = "logo";
		discordPresence.largeImageText = largeText;

		if (smallImage != "none") {
			discordPresence.smallImageKey = smallImage;
			discordPresence.smallImageText = smallText;
		}
		Discord_UpdatePresence(&discordPresence);
	}

	void InitDiscord() {
		DiscordEventHandlers handlers;
		memset(&handlers, 0, sizeof(handlers));
		handlers.ready = handleDiscordReady;
		handlers.errored = handleDiscordError;
		handlers.disconnected = handleDiscordDisconnected;
		handlers.joinGame = handleDiscordJoinGame;
		handlers.spectateGame = handleDiscordSpectateGame;
		handlers.joinRequest = handleDiscordJoinRequest;

		// Discord_Initialize(const char* applicationId, DiscordEventHandlers*
		// handlers, int autoRegister, const char* optionalSteamId)
		Discord_Initialize(APPLICATION_ID, &handlers, 1, "322170");
	}
}