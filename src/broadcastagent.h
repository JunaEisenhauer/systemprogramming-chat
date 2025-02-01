#include <stdbool.h>
#ifndef BROADCASTAGENT_H
#define BROADCASTAGENT_H

int broadcastAgentInit();
void broadcastAgentCleanup();

bool isPaused();
void broadcastPause();
void broadcastResume();

#endif
