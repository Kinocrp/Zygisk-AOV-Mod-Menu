#include "il2cpp-hook.h"
#include "il2cpp-resolver.h"
#include <chrono>
#include <unordered_map>
#include "dobby.h"
#include "log.h"

#include "menu-config.h"
#include "menu-esp.h"

// Fields
static size_t LActorRoot_TheActorMeta = 0;
static size_t LActorRoot_ValueComponent = 0;
static size_t LActorRoot_SkillControl = 0;
static size_t ActorMeta_ActorType = 0;
static size_t ActorMeta_PlayerId = 0;
static size_t ActorMeta_SkinID = 0;
static size_t ActorMeta_ConfigId = 0;
static size_t ActorMeta_ActorCamp = 0;
static size_t LSkillComponent_SkillSlotArray = 0;

// Structs
struct ActorSmoothState {
    VInt3 lastLogicPos;
    float lastUpdateTime;
    bool wasAlive;
};

// Values
int64_t HostUUID = 0;
int32_t HostObjectID = 0;
int32_t HostBillboardID = 28;
int32_t HostPersonalButtonID = 26;
int32_t HostHeroID = 0;
int32_t HostSkinID = 0;
int32_t HostCampID = 0;
int32_t CurrentSkillSlot = 0;
int32_t CurrentPingMS = 16;
VInt3 HostLocation = { 0 };
float CurrentHeight = 0.0f;
float CameraHeightTotal = 0.0f;
bool IsFight = false;

std::unordered_map<uint32_t, ActorSmoothState> ActorSmoothMap;

// Functions
void (*o_SetMaterUUID)(Il2CppObject*, uint64_t, const MethodInfo*) = nullptr;
void h_SetMaterUUID(Il2CppObject *_this, uint64_t InMaterUUID, const MethodInfo *method) {
    HostUUID = (int64_t)InMaterUUID;
    o_SetMaterUUID(_this, InMaterUUID, method);
}

bool h_IsPrivaceOn(uint64_t data, int32_t mask, const MethodInfo *method) {
    return false;
}

void (*o_HandlePlayerInfo)(Il2CppObject*, const MethodInfo*) = nullptr;
void h_HandlePlayerInfo(Il2CppObject *playerInfo, const MethodInfo *method) {
    IsFight = true;
    ESPManager.ClearAll();
    ActorSmoothMap.clear();

    auto COMDT_PLAYERINFO = il2cpp_object_get_class(playerInfo);
    auto bObjType = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(COMDT_PLAYERINFO, "bObjType"));
    if (get_field_value<uint8_t>(playerInfo, bObjType) != 1) return o_HandlePlayerInfo(playerInfo, method);

    auto stDetail = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(COMDT_PLAYERINFO, "stDetail"));
    auto stDetailObj = get_field_value<Il2CppObject*>(playerInfo, stDetail);
    if (!stDetailObj) return o_HandlePlayerInfo(playerInfo, method);

    auto COMDT_PLAYERINFO_DETAIL = il2cpp_object_get_class(stDetailObj);
    auto get_stPlayerOfAcnt = get_method(COMDT_PLAYERINFO_DETAIL, "get_stPlayerOfAcnt");
    auto AcntObj = il2cpp_runtime_invoke(get_stPlayerOfAcnt, stDetailObj, nullptr, nullptr);
    if (!AcntObj) return o_HandlePlayerInfo(playerInfo, method);

    auto COMDT_PLAYERINFO_OF_ACNT = il2cpp_object_get_class(AcntObj);
    auto ullUid = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(COMDT_PLAYERINFO_OF_ACNT, "ullUid"));
    if (get_field_value<uint64_t>(AcntObj, ullUid) != HostUUID) return o_HandlePlayerInfo(playerInfo, method);

    auto dwObjId = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(COMDT_PLAYERINFO, "dwObjId"));
    auto astChoiceHero = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(COMDT_PLAYERINFO, "astChoiceHero"));
    auto astChoiceHeroArr = get_field_value<Il2CppArray*>(playerInfo, astChoiceHero);
    auto astChoiceHeroObj = (Il2CppObject*)astChoiceHeroArr->vector[0];
    HostObjectID = (int32_t)get_field_value<uint32_t>(playerInfo, dwObjId);

    auto COMDT_CHOICEHERO = il2cpp_object_get_class(astChoiceHeroObj);
    auto stBaseInfo = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(COMDT_CHOICEHERO, "stBaseInfo"));
    auto iBillboardID = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(COMDT_CHOICEHERO, "iBillboardID"));
    auto iPersonalButtonID = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(COMDT_CHOICEHERO, "iPersonalButtonID"));
    auto stBaseInfoObj = get_field_value<Il2CppObject*>(astChoiceHeroObj, stBaseInfo);

    set_field_value(astChoiceHeroObj, iBillboardID, HostBillboardID);
    set_field_value(astChoiceHeroObj, iPersonalButtonID, HostPersonalButtonID);

    auto COMDT_HEROINFO = il2cpp_object_get_class(stBaseInfoObj);
    auto stCommonInfo = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(COMDT_HEROINFO, "stCommonInfo"));
    auto stCommonInfoObj = get_field_value<Il2CppObject*>(stBaseInfoObj, stCommonInfo);
    auto COMDT_HERO_COMMON_INFO = il2cpp_object_get_class(stCommonInfoObj);
    auto dwHeroID = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(COMDT_HERO_COMMON_INFO, "dwHeroID"));
    auto wSkinID = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(COMDT_HERO_COMMON_INFO, "wSkinID"));

    HostHeroID = (int32_t)get_field_value<uint32_t>(stCommonInfoObj, dwHeroID);
    if (HostSkinID && menu_config.IsModSkin) {
        set_field_value(stCommonInfoObj, wSkinID, HostSkinID);
        HostSkinID = 0;
    }

    o_HandlePlayerInfo(playerInfo, method);
}

void (*o_normal_HeroSelect_OnSkinSelect)(Il2CppObject*, uint32_t, const MethodInfo*) = nullptr;
void h_normal_HeroSelect_OnSkinSelect(Il2CppObject *_this, uint32_t skinID, const MethodInfo *method) {
    if (menu_config.IsModSkin) {
        HostSkinID = (int32_t)skinID;
        skinID = 0;
    }
    o_normal_HeroSelect_OnSkinSelect(_this, skinID, method);
}

void (*o_rank_HeroSelect_OnSkinSelect)(Il2CppObject*, Il2CppObject*, const MethodInfo*) = nullptr;
void h_rank_HeroSelect_OnSkinSelect(Il2CppObject *_this, Il2CppObject *uiEvent, const MethodInfo *method) {
    if (menu_config.IsModSkin) {
        auto CUIEvent = il2cpp_object_get_class(uiEvent);
        auto m_eventParamsField = il2cpp_class_get_field_from_name(CUIEvent, "m_eventParams");
        auto m_eventParamsType = il2cpp_field_get_type(m_eventParamsField);
        auto m_eventParams = il2cpp_field_get_offset(m_eventParamsField);
        auto stUIEventParams = il2cpp_class_from_type(m_eventParamsType);
        auto skinId = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(stUIEventParams, "skinId"));
        auto m_eventParamsObj = get_field_pointer<void>(uiEvent, m_eventParams);

        HostSkinID = (int32_t)get_field_value<uint32_t>(m_eventParamsObj, skinId, true);
        set_field_value(m_eventParamsObj, skinId, 0, true);
    }
    o_rank_HeroSelect_OnSkinSelect(_this, uiEvent, method);
}

void CalculateESP(Il2CppObject*);
void (*OnCameraHeightChanged)(Il2CppObject*, const MethodInfo*) = nullptr;
float (*o_GetZoomRate)(Il2CppObject*, const MethodInfo*) = nullptr;
float h_GetZoomRate(Il2CppObject *_this, const MethodInfo *method) {
    CameraHeightTotal = o_GetZoomRate(_this, method) + menu_config.CameraHeight;
    return CameraHeightTotal;
}
Il2CppObject *(*o_get_CurCamera)(Il2CppObject*, const MethodInfo*) = nullptr;
Il2CppObject *h_get_CurCamera(Il2CppObject *_this, const MethodInfo *method) {
    auto ret = o_get_CurCamera(_this, method);
    if (CurrentHeight != menu_config.CameraHeight) {
        CurrentHeight = menu_config.CameraHeight;
        OnCameraHeightChanged(_this, nullptr);
    }
    if (!menu_config.IsESP || !IsFight) return ret;
    CalculateESP(ret);
    return ret;
}

Vector2 (*GetMMFianlScreenPos)(Il2CppObject*, const MethodInfo*) = nullptr;
Vector2 (*get_mmFinalScreenSize)(Il2CppObject*, const MethodInfo*) = nullptr;
void (*o_MiniMapUpdate)(Il2CppObject*, const MethodInfo*) = nullptr;
void h_MiniMapUpdate(Il2CppObject *_this, const MethodInfo *method) {
    ESPManager.set_MiniMapPosition(GetMMFianlScreenPos(_this, nullptr));
    ESPManager.set_MiniMapSize(get_mmFinalScreenSize(_this, nullptr));
    o_MiniMapUpdate(_this, method);
}

int32_t (*get_objCamp)(Il2CppObject*, const MethodInfo*) = nullptr;
uint32_t (*get_ConfigId)(Il2CppObject*, const MethodInfo*) = nullptr;
VInt3 (*kyrios_get_location)(Il2CppObject*, const MethodInfo*) = nullptr;
bool (*o_IsHostPlayer)(Il2CppObject*, const MethodInfo*) = nullptr;
bool h_IsHostPlayer(Il2CppObject *_this, const MethodInfo *method) {
    auto ret = o_IsHostPlayer(_this, method);
    if (ret) {
        HostCampID = get_objCamp(_this, nullptr);
        HostHeroID = (int32_t)get_ConfigId(_this, nullptr);
        HostLocation = kyrios_get_location(_this, nullptr);
    }
    return ret;
}

bool IsMobValid(int32_t ConfigId) {
    if (ConfigId >= 7300 && ConfigId <= 7303) return false;
    if (ConfigId >= 7310 && ConfigId <= 7313) return false;
    return true;
}

Il2CppString *(*GetHeroName)(uint32_t, const MethodInfo*) = nullptr;
void (*o_Spawned)(Il2CppObject*, Il2CppObject*, VInt3, VInt3, bool, bool, const MethodInfo*) = nullptr;
void h_Spawned(Il2CppObject *_this, Il2CppObject *battle, VInt3 pos, VInt3 dir, bool bWithSight, bool bShowControl, const MethodInfo *method) {
    auto meta = get_field_pointer<void>(_this, LActorRoot_TheActorMeta);
    auto ActorType = get_field_value<int32_t>(meta, ActorMeta_ActorType, true);
    auto ActorCamp = get_field_value<int32_t>(meta, ActorMeta_ActorCamp, true);
    if (ActorType == 0) {
        auto PlayerId = get_field_value<int32_t>(meta, ActorMeta_PlayerId, true);
        if (PlayerId == HostObjectID && menu_config.IsModSkin) set_field_value(meta, ActorMeta_SkinID, 0, true);
        auto ConfigId = get_field_value<int32_t>(meta, ActorMeta_ConfigId, true);
        size_t gchandle = il2cpp_gchandle_new(_this, false);
        ESPManager.Add(ESPType::Hero, (int)gchandle, nullptr, gchandle, il2cpp_string_to_std_string(GetHeroName(ConfigId, nullptr)));
    } else if (ActorType == 1 && ActorCamp == 0) {
        auto ConfigId = get_field_value<int32_t>(meta, ActorMeta_ConfigId, true);
        if (IsMobValid(ConfigId)) {
            size_t gchandle = il2cpp_gchandle_new(_this, false);
            ESPManager.Add(ESPType::Monster, (int)gchandle, nullptr, gchandle, std::to_string(ConfigId));
        }
    }
    o_Spawned(_this, battle, pos, dir, bWithSight, bShowControl, method);
}

VInt3 (*root_get_location)(Il2CppObject*, const MethodInfo*) = nullptr;
VInt3 (*root_get_preLocation)(Il2CppObject*, const MethodInfo*) = nullptr;
bool (*IsVisibleForEnermyCamp)(Il2CppObject*, const MethodInfo*) = nullptr;
int32_t (*get_actorHp)(Il2CppObject*, const MethodInfo*) = nullptr;
int32_t (*get_actorHpTotal)(Il2CppObject*, const MethodInfo*) = nullptr;
void (*WorldToScreenPoint_Injected)(Il2CppObject*, Vector3*, int32_t, Vector3*, const MethodInfo*) = nullptr;
bool (*IsUnLock)(Il2CppObject*, const MethodInfo*) = nullptr;
VInt2 (*get_CurSkillCD)(Il2CppObject*, const MethodInfo*) = nullptr;

float GetCurrentTimeSec() {
    using namespace std::chrono;
    static auto start = high_resolution_clock::now();
    auto now = high_resolution_clock::now();
    duration<float> elapsed = now - start;
    return elapsed.count();
}

int GetSkillCD(Il2CppObject *SkillSlotArrayObject, int slot) {
    auto SkillObject = get_array_element<Il2CppObject*>(SkillSlotArrayObject, slot);
    if (!SkillObject) return -1;
    if (!IsUnLock(SkillObject, nullptr)) return -1;
    VInt2 CD = get_CurSkillCD(SkillObject, nullptr);
    return (CD.x ^ CD.y) / 1000;
}

bool AimbotConfig(float &Range, float &bulletSpeed) {
    if (HostHeroID == 196 && CurrentSkillSlot == 2) {
        Range = 25000.0f;
        bulletSpeed = 55000.0f;
        return true;
    }
    return false;
}

void CalculateESP(Il2CppObject *_this) {
    float currentTime = GetCurrentTimeSec();

    const float TICK_RATE = 15.0f;
    const float TICK_DURATION = 1.0f / TICK_RATE;
    const float VELOCITY_TIMEOUT = 0.07f;

    float Range = 0.0f;
    float bulletSpeed = 0.0f;
    float lowestHP = 9999999.0f;
    bool foundTarget = false;

    Vector3 hostLocation = { (float)HostLocation.x, (float)HostLocation.y, (float)HostLocation.z };
    Vector2 bestAimLocal = { 0.0f, 0.0f };

    auto ProcessEntity = [&](ESPObject &obj, bool isMob) {
        Il2CppObject *ActorObject = obj.ObjectPtr ? obj.ObjectPtr : il2cpp_gchandle_get_target(obj.GCHandle);
        if (!ActorObject) return;

        auto meta = get_field_pointer<void>(ActorObject, LActorRoot_TheActorMeta);
        obj.CampID = get_field_value<int32_t>(meta, ActorMeta_ActorCamp, true);
        if (obj.CampID == HostCampID && !isMob) return;

        auto ValueComponentObject = get_field_value<Il2CppObject*>(ActorObject, LActorRoot_ValueComponent);
        if (!ValueComponentObject) return;

        VInt3 location = root_get_location(ActorObject, nullptr);
        VInt3 preLocation = root_get_preLocation(ActorObject, nullptr);

        Vector3 velocity = { 0 };
        Vector3 smoothLocation = { 0 };
        ActorSmoothState &state = ActorSmoothMap[obj.ObjectID];

        bool isLogicMoving = (location.x != state.lastLogicPos.x || location.y != state.lastLogicPos.y || location.z != state.lastLogicPos.z);
        if (isLogicMoving) {
            state.lastLogicPos = location;
            state.lastUpdateTime = currentTime;
        }

        bool isAlive = get_actorHp(ValueComponentObject, nullptr);
        if (state.wasAlive != isAlive) {
            state.lastUpdateTime = currentTime - TICK_DURATION;
            state.wasAlive = isAlive;
        }

        float timeSinceUpdate = currentTime - state.lastUpdateTime;
        float progress = timeSinceUpdate / TICK_DURATION;
        float visualProgress = (progress > 1.0f) ? 1.0f : progress;

        bool hasValidPrePos = !(preLocation.x == 0 && preLocation.y == 0 && preLocation.z == 0);
        auto deltaX = (float)(location.x - preLocation.x);
        auto deltaY = (float)(location.y - preLocation.y);
        auto deltaZ = (float)(location.z - preLocation.z);

        if (hasValidPrePos) {
            smoothLocation.x = (float)preLocation.x + (deltaX * visualProgress);
            smoothLocation.y = (float)preLocation.y + (deltaY * visualProgress);
            smoothLocation.z = (float)preLocation.z + (deltaZ * visualProgress);
        } else {
            smoothLocation = { (float)location.x, (float)location.y, (float)location.z };
        }

        if (hasValidPrePos && timeSinceUpdate <= (TICK_DURATION + VELOCITY_TIMEOUT)) {
            velocity.x = deltaX * TICK_RATE;
            velocity.y = deltaY * TICK_RATE;
            velocity.z = deltaZ * TICK_RATE;
        } else {
            velocity = { 0, 0, 0 };
        }

        Vector3 screenPosition = { 0 };
        Vector3 worldLocation = { smoothLocation.x / 1000.0f, (smoothLocation.y + 800.0f) / 1000.0f, smoothLocation.z / 1000.0f };

        WorldToScreenPoint_Injected(_this, &worldLocation, 2, &screenPosition, nullptr);
        screenPosition.y = (float)screenHeight - screenPosition.y;

        obj.Location = smoothLocation;
        obj.ScreenPosition = screenPosition;
        obj.Velocity = velocity;
        obj.CurrentHP = get_actorHp(ValueComponentObject, nullptr);
        obj.TotalHP = get_actorHpTotal(ValueComponentObject, nullptr);

        if (!isMob) {
            obj.Visibility = IsVisibleForEnermyCamp(ActorObject, nullptr);
            auto SkillControlObject = get_field_value<Il2CppObject*>(ActorObject, LActorRoot_SkillControl);

            if (SkillControlObject) {
                auto SkillSlotArrayObject = get_field_value<Il2CppObject*>(SkillControlObject, LSkillComponent_SkillSlotArray);
                if (SkillSlotArrayObject) {
                    obj.Skill1 = GetSkillCD(SkillSlotArrayObject, 1);
                    obj.Skill2 = GetSkillCD(SkillSlotArrayObject, 2);
                    obj.Skill3 = GetSkillCD(SkillSlotArrayObject, 3);
                    obj.Skill4 = GetSkillCD(SkillSlotArrayObject, 4);
                    obj.Skill5 = GetSkillCD(SkillSlotArrayObject, 5);
                }
            }
        }

        if (!isMob && obj.CurrentHP > 0 && obj.CampID != HostCampID && AimbotConfig(Range, bulletSpeed)) {
            float dx = smoothLocation.x - hostLocation.x;
            float dz = smoothLocation.z - hostLocation.z;
            float currentDist = sqrt(dx * dx + dz * dz);

            if (currentDist <= Range) {
                float speedVal = sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
                float predX, predZ;

                if (speedVal > 21000.0f) {
                    float vX = (speedVal > 0) ? (velocity.x / speedVal) : 0;
                    float vZ = (speedVal > 0) ? (velocity.z / speedVal) : 0;
                    predX = smoothLocation.x + (vX * 500.0f);
                    predZ = smoothLocation.z + (vZ * 500.0f);
                } else {
                    float tTravel = (bulletSpeed > 0) ? (currentDist / bulletSpeed) : 0;
                    float tPing = (float)CurrentPingMS / 1000.0f;
                    float tTotal = tTravel + tPing;
                    predX = smoothLocation.x + (velocity.x * tTotal);
                    predZ = smoothLocation.z + (velocity.z * tTotal);
                }

                float aimX = predX - hostLocation.x;
                float aimZ = predZ - hostLocation.z;
                float predDist = sqrt(aimX * aimX + aimZ * aimZ);

                if (predDist <= Range) {
                    if ((float)obj.CurrentHP < lowestHP) {
                        lowestHP = (float)obj.CurrentHP;
                        foundTarget = true;
                        if (predDist > 0) {
                            bestAimLocal.x = aimX / predDist;
                            bestAimLocal.y = aimZ / predDist;
                        }
                    }
                }
            }
        }
    };

    for (auto &obj : ESPManager.Get(ESPType::Hero)) {
        ProcessEntity(obj, false);
    }

    for (auto &obj : ESPManager.Get(ESPType::Monster)) {
        ProcessEntity(obj, true);
    }

    if (foundTarget) {
        if (HostCampID == 2) {
            ESPManager.AimVector = { -bestAimLocal.x, -bestAimLocal.y };
        } else {
            ESPManager.AimVector = bestAimLocal;
        }
    } else {
        ESPManager.AimVector = { 0.0f, 0.0f };
    }
}

void (*o_DoFightOver)(Il2CppObject*, bool, const MethodInfo*) = nullptr;
void h_DoFightOver(Il2CppObject *_this, bool bNormalEnd, const MethodInfo *methodInfo) {
    IsFight = false;
    ESPManager.ClearAll();
    ActorSmoothMap.clear();
    o_DoFightOver(_this, bNormalEnd, methodInfo);
}

bool bReach(int heroID, int skillID, int inSlotType) {
    if (HostHeroID == heroID && skillID == inSlotType) return true;
    return false;
}

int32_t (*o_GetIncedSkillAttackRange)(Il2CppObject*, int32_t, const MethodInfo*) = nullptr;
int32_t h_GetIncedSkillAttackRange(Il2CppObject *_this, int32_t inSlotType, const MethodInfo *method) {
    auto ret = o_GetIncedSkillAttackRange(_this, inSlotType, method);
    if (!menu_config.IsReach) return ret;
    if (bReach(150, 1, inSlotType)) return 1000;    // Nakroth
    if (bReach(531, 1, inSlotType)) return 1500;    // Keera
    if (bReach(503, 3, inSlotType)) return 1500;    // Zuka
    if (bReach(135, 2, inSlotType)) return 3500;    // Thane
    if (bReach(144, 1, inSlotType)) return 1000;    // Taara
    if (bReach(106, 2, inSlotType)) return 1000;    // Krixi
    return ret;
}

void (*o_SelectSkillTarget)(Il2CppObject*, int32_t, Vector2, bool, float, Vector2, bool, const MethodInfo*) = nullptr;
void h_SelectSkillTarget(Il2CppObject *_this, int32_t slot, Vector2 axis, bool isSkillCursorInCancelArea, float cursorRadiusRatio, Vector2 cursorMoveDelta, bool isControlMove, const MethodInfo *method) {
    if (menu_config.IsAimbot) {
        CurrentSkillSlot = slot;
        auto predict = ESPManager.AimVector;
        if (predict.x != 0.0f && predict.y != 0.0f) axis = predict;
    }
    o_SelectSkillTarget(_this, slot, axis, isSkillCursorInCancelArea, cursorRadiusRatio, cursorMoveDelta, isControlMove, method);
}

void (*o_set_GameSvrPing)(Il2CppObject*, int32_t value, const MethodInfo*) = nullptr;
void h_set_GameSvrPing(Il2CppObject *_this, int32_t value, const MethodInfo *method) {
    CurrentPingMS = value;
    o_set_GameSvrPing(_this, value, method);
}

Il2CppObject *(*o_GetPlayer)(Il2CppObject*, uint32_t, const MethodInfo*) = nullptr;
Il2CppObject *h_GetPlayer(Il2CppObject *_this, uint32_t inPlayerID, const MethodInfo *method) {
    auto ret = o_GetPlayer(_this, inPlayerID, method);

    if (ret) {
        auto PlayerBase = il2cpp_object_get_class(ret);
        auto Computer = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(PlayerBase, "Computer"));
        auto IsServerAIAgent = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(PlayerBase, "IsServerAIAgent"));
        auto bServerAI = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(PlayerBase, "bServerAI"));

        auto Name = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(PlayerBase, "Name"));
        auto mFullNickName = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(PlayerBase, "mFullNickName"));
        if (get_field_value<bool>(ret, IsServerAIAgent) || get_field_value<bool>(ret, bServerAI)) {
            set_field_value(ret, Name, il2cpp_string_new("AI (Server)"));
            set_field_value(ret, mFullNickName, il2cpp_string_new("AI (Server)"));
        } else if (get_field_value<bool>(ret, Computer)) {
            set_field_value(ret, Name, il2cpp_string_new("AI (Local)"));
            set_field_value(ret, mFullNickName, il2cpp_string_new("AI (Local)"));
        } else {
            if (menu_config.IsHideName) {
                auto PlayerId = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(PlayerBase, "PlayerId"));
                auto PlayerUId = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(PlayerBase, "PlayerUId"));
                if (get_field_value<uint32_t>(ret, PlayerId) == HostObjectID || get_field_value<uint64_t>(ret, PlayerUId) == HostUUID) {
                    set_field_value(ret, Name, il2cpp_string_new("Kinocrp"));
                    set_field_value(ret, mFullNickName, il2cpp_string_new("Kinocrp"));
                } else {
                    set_field_value(ret, Name, il2cpp_string_new("Player"));
                    set_field_value(ret, mFullNickName, il2cpp_string_new("Player"));
                }
            }
        }
    }

    return ret;
}

void il2cpp_hook() {
    // Domain
    auto domain = il2cpp_domain_get();

    // Images
    auto Project_d = get_image(domain, "Project_d.dll");
    auto Project_Plugins_d = get_image(domain, "Project.Plugins_d.dll");
    auto UnityEngine_CoreModule = get_image(domain, "UnityEngine.CoreModule.dll");

    // Classes
    auto CRoleInfoManager = il2cpp_class_from_name(Project_d, "Assets.Scripts.GameSystem", "CRoleInfoManager");
    auto SkinResourceHelper = il2cpp_class_from_name(Project_d, "Assets.Scripts.GameLogic", "SkinResourceHelper");
    auto HeroSelectNormalWindow = il2cpp_class_from_name(Project_d, "Assets.Scripts.GameSystem", "HeroSelectNormalWindow");
    auto HeroSelectBanPickWindow = il2cpp_class_from_name(Project_d, "Assets.Scripts.GameSystem", "HeroSelectBanPickWindow");
    auto CameraSystem = il2cpp_class_from_name(Project_d, "", "CameraSystem");
    auto MinimapSys = il2cpp_class_from_name(Project_d, "Assets.Scripts.GameSystem", "MinimapSys");
    auto ActorLinker = il2cpp_class_from_name(Project_d, "Kyrios.Actor", "ActorLinker");
    auto CHeroInfo = il2cpp_class_from_name(Project_d, "Assets.Scripts.GameSystem", "CHeroInfo");
    auto LActorRoot = il2cpp_class_from_name(Project_Plugins_d, "NucleusDrive.Logic", "LActorRoot");
    auto ActorMeta = il2cpp_class_from_name(Project_Plugins_d, "", "ActorMeta");
    auto ValuePropertyComponent = il2cpp_class_from_name(Project_Plugins_d, "NucleusDrive.Logic", "ValuePropertyComponent");
    auto Camera = il2cpp_class_from_name(UnityEngine_CoreModule, "UnityEngine", "Camera");
    auto LSkillComponent = il2cpp_class_from_name(Project_Plugins_d, "NucleusDrive.Logic", "LSkillComponent");
    auto SkillSlot = il2cpp_class_from_name(Project_Plugins_d, "NucleusDrive.Logic", "SkillSlot");
    auto BattleLogic = il2cpp_class_from_name(Project_d, "Assets.Scripts.GameLogic", "BattleLogic");
    auto SkillComponent = il2cpp_class_from_name(Project_d, "Assets.Scripts.GameLogic", "SkillComponent");
    auto LFrameSynchr = il2cpp_class_from_name(Project_Plugins_d, "NucleusDrive.Logic", "LFrameSynchr");
    auto GamePlayerCenter = il2cpp_class_from_name(Project_Plugins_d, "NucleusDrive.Logic.GameKernal", "GamePlayerCenter");

    // Fields
    LActorRoot_TheActorMeta = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(LActorRoot, "TheActorMeta"));
    LActorRoot_ValueComponent = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(LActorRoot, "ValueComponent"));
    LActorRoot_SkillControl = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(LActorRoot, "SkillControl"));
    ActorMeta_ActorType = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(ActorMeta, "ActorType"));
    ActorMeta_PlayerId = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(ActorMeta, "PlayerId"));
    ActorMeta_SkinID = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(ActorMeta, "SkinID"));
    ActorMeta_ConfigId = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(ActorMeta, "ConfigId"));
    ActorMeta_ActorCamp = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(ActorMeta, "ActorCamp"));
    LSkillComponent_SkillSlotArray = il2cpp_field_get_offset(il2cpp_class_get_field_from_name(LSkillComponent, "SkillSlotArray"));

    // Functions
    OnCameraHeightChanged = (void(*)(Il2CppObject*, const MethodInfo*))(void*)get_method(CameraSystem, "OnCameraHeightChanged")->methodPointer;
    GetMMFianlScreenPos = (Vector2(*)(Il2CppObject*, const MethodInfo*))(void*)get_method(MinimapSys, "GetMMFianlScreenPos")->methodPointer;
    get_mmFinalScreenSize = (Vector2(*)(Il2CppObject*, const MethodInfo*))(void*)get_method(MinimapSys, "get_mmFinalScreenSize")->methodPointer;
    get_objCamp = (int32_t(*)(Il2CppObject*, const MethodInfo*))(void*)get_method(ActorLinker, "get_objCamp")->methodPointer;
    get_ConfigId = (uint32_t(*)(Il2CppObject*, const MethodInfo*))(void*)get_method(ActorLinker, "get_ConfigId")->methodPointer;
    kyrios_get_location = (VInt3(*)(Il2CppObject*, const MethodInfo*))(void*)get_method(ActorLinker, "get_location")->methodPointer;
    GetHeroName = (Il2CppString*(*)(uint32_t, const MethodInfo*))(void*)get_method(CHeroInfo, "GetHeroName")->methodPointer;
    root_get_location = (VInt3(*)(Il2CppObject*, const MethodInfo*))(void*)get_method(LActorRoot, "get_location")->methodPointer;
    root_get_preLocation = (VInt3(*)(Il2CppObject*, const MethodInfo*))(void*)get_method(LActorRoot, "get_preLocation")->methodPointer;
    IsVisibleForEnermyCamp = (bool(*)(Il2CppObject*, const MethodInfo*))(void*)get_method(LActorRoot, "IsVisibleForEnermyCamp")->methodPointer;
    get_actorHp = (int32_t(*)(Il2CppObject*, const MethodInfo*))(void*)get_method(ValuePropertyComponent, "get_actorHp")->methodPointer;
    get_actorHpTotal = (int32_t(*)(Il2CppObject*, const MethodInfo*))(void*)get_method(ValuePropertyComponent, "get_actorHpTotal")->methodPointer;
    WorldToScreenPoint_Injected = (void(*)(Il2CppObject*, Vector3*, int32_t, Vector3*, const MethodInfo*))(void*)get_method(Camera, "WorldToScreenPoint_Injected")->methodPointer;
    IsUnLock = (bool(*)(Il2CppObject*, const MethodInfo*))(void*)get_method(SkillSlot, "IsUnLock")->methodPointer;
    get_CurSkillCD = (VInt2(*)(Il2CppObject*, const MethodInfo*))(void*)get_method(SkillSlot, "get_CurSkillCD")->methodPointer;

    // Trampolines
    DobbyHook((void*)get_method(CRoleInfoManager, "SetMaterUUID")->methodPointer, (void*)h_SetMaterUUID, (void**)&o_SetMaterUUID);
    DobbyHook((void*)get_method(CRoleInfoManager, "IsPrivaceOn", 1, nullptr, "COM_USER_PRIVACY_MASK")->methodPointer, (void*)h_IsPrivaceOn, nullptr);
    DobbyHook((void*)get_method(SkinResourceHelper, "HandlePlayerInfo")->methodPointer, (void*)h_HandlePlayerInfo, (void**)&o_HandlePlayerInfo);
    DobbyHook((void*)get_method(HeroSelectNormalWindow, "HeroSelect_OnSkinSelect", 0, "skinID")->methodPointer, (void*)h_normal_HeroSelect_OnSkinSelect, (void**)&o_normal_HeroSelect_OnSkinSelect);
    DobbyHook((void*)get_method(HeroSelectBanPickWindow, "HeroSelect_OnSkinSelect")->methodPointer, (void*)h_rank_HeroSelect_OnSkinSelect, (void**)&o_rank_HeroSelect_OnSkinSelect);
    DobbyHook((void*)get_method(CameraSystem, "GetZoomRate")->methodPointer, (void*)h_GetZoomRate, (void**)&o_GetZoomRate);
    DobbyHook((void*)get_method(CameraSystem, "get_CurCamera")->methodPointer, (void*)h_get_CurCamera, (void**)&o_get_CurCamera);
    DobbyHook((void*)get_method(MinimapSys, "Update")->methodPointer, (void*)h_MiniMapUpdate, (void**)&o_MiniMapUpdate);
    DobbyHook((void*)get_method(ActorLinker, "IsHostPlayer")->methodPointer, (void*)h_IsHostPlayer, (void**)&o_IsHostPlayer);
    DobbyHook((void*)get_method(LActorRoot, "Spawned")->methodPointer, (void*)h_Spawned, (void**)&o_Spawned);
    DobbyHook((void*)get_method(BattleLogic, "DoFightOver")->methodPointer, (void*)h_DoFightOver, (void**)&o_DoFightOver);
    DobbyHook((void*)get_method(SkillComponent, "GetIncedSkillAttackRange")->methodPointer, (void*)h_GetIncedSkillAttackRange, (void**)&o_GetIncedSkillAttackRange);
    DobbyHook((void*)get_method(SkillComponent, "SelectSkillTarget")->methodPointer, (void*)h_SelectSkillTarget, (void**)&o_SelectSkillTarget);
    DobbyHook((void*)get_method(LFrameSynchr, "set_GameSvrPing")->methodPointer, (void*)h_set_GameSvrPing, (void**)&o_set_GameSvrPing);
    DobbyHook((void*)get_method(GamePlayerCenter, "GetPlayer")->methodPointer, (void*)h_GetPlayer, (void**)&o_GetPlayer);

    // Test
}
