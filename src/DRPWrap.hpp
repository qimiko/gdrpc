#pragma once
#pragma comment(lib, "discord-rpc.lib")

#include "DiscordSecret.hpp"
#include <cstdint>
#include <cstdlib>
#include <discord_rpc.h>
#include <iostream>
#include <time.h>

#ifndef DRPWRAP
#define DRPWRAP

class Discord_Presence {
  private:
    int status;

  public:
    Discord_Presence();
    void initialize();
    int get_status();
    void set_status(int);
    void update(const char *details, const char *largeText, const char *smallText,
                const char *statetext, const char *smallImage, time_t timestamp);
    void run_callbacks();
    void shutdown();
};

Discord_Presence *get_discord();
#endif