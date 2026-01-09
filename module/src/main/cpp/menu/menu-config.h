#pragma once

#include <jni.h>
#include "menu-esp.h"

struct MenuConfig {
    bool IsESP = true;
    bool IsReach = false;
    bool IsAimbot = true;
    bool IsModSkin = true;
    bool IsHideName = true;
    float CameraHeight = 0.4f;
};

extern int screenWidth;
extern int screenHeight;
extern MenuConfig menu_config;
extern ESP ESPManager;

extern int64_t HostUUID;
extern int32_t HostObjectID;
extern int32_t HostBillboardID;
extern int32_t HostPersonalButtonID;
extern int32_t HostHeroID;
extern int32_t HostSkinID;
extern int32_t HostCampID;
extern int32_t CurrentSkillSlot;
extern int32_t CurrentPingMS;
extern VInt3 HostLocation;
extern float CurrentHeight;
extern float CameraHeightTotal;
extern bool IsFight;
