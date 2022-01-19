#pragma once

#include "Defines.h"

struct game;

// Application configuration.
typedef struct application_config {
    // Window starting position x axis, if applicable.
    s16 startPosX;
    // Window starting position y axis, if applicable.
    s16 startPosY;
    // Window starting width, if applicable.
    s16 startWidth;
    // Window starting height, if applicable.
    s16 startHeight;
    // The application name used in windowing, if applicable.
    char* name;
} application_config;


TAPI b8 ApplicationCreate(struct game* gameInst);
TAPI b8 ApplicationRun();
void ApplicationGetFramebufferSize(u32* width, u32* height);