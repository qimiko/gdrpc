#include "presence_wrapper.hpp"
#include "pch.h"

Discord_Presence global_discord = Discord_Presence();

Discord_Presence *get_discord() {
  return &global_discord;
}

// this weird structure is necessary thanks to how discord-rpc handles this stuff
static void handleDiscordReady(const DiscordUser *connectedUser) {
  get_discord()->set_status(0); // success is code of 0
}

static void handleDiscordDisconnected(int errcode, const char *message) {
  get_discord()->set_status(errcode);
}

static void handleDiscordError(int errcode, const char *message) {
  get_discord()->set_status(errcode);
}

static void handleDiscordJoinGame(const char *secret) { return; }

static void handleDiscordSpectateGame(const char *secret) { return; }

static void handleDiscordJoinRequest(const DiscordUser *request) {
  Discord_Respond(request->userId, DISCORD_REPLY_NO);
}

Discord_Presence::Discord_Presence() {
  status = -1;
}

void Discord_Presence::initialize() {
  DiscordEventHandlers handlers;
  memset(&handlers, 0, sizeof(handlers));
  handlers.ready = handleDiscordReady;
  handlers.errored = handleDiscordError;
  handlers.disconnected = handleDiscordDisconnected;
  handlers.joinGame = handleDiscordJoinGame;
  handlers.spectateGame = handleDiscordSpectateGame;
  handlers.joinRequest = handleDiscordJoinRequest;

  Discord_Initialize(APPLICATION_ID, &handlers, 1, "322170");
}

int Discord_Presence::get_status() {
  return status;
}

void Discord_Presence::set_status(int n_status) {
  status = n_status;
}

void Discord_Presence::update(const char *details, const char *largeText,
                    const char *smallText, const char *statetext,
                    const char *smallImage, time_t timestamp) {
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

void Discord_Presence::run_callbacks() {
  Discord_RunCallbacks();
}

void Discord_Presence::shutdown() {
  Discord_Shutdown();
}