#pragma once

#include <vector>
#include <cinttypes>
#include "il2cpp-resolver.h"
#include "il2cpp-structs.h"

enum class ESPType : int {
    Hero = 0,
    Monster,
    Count
};

struct ESPObject {
    ESPType Type;
    int ObjectID;
    Il2CppObject *ObjectPtr;
    uint32_t GCHandle;
    std::string Name;

    int CampID = 0;
    Vector3 Location = { 0, 0, 0 };
    Vector3 ScreenPosition = { 0, 0, 0 };
    Vector3 Velocity = { 0, 0, 0 };
    bool Visibility = false;
    int CurrentHP = 0, TotalHP = 0;
    int Skill1 = 0, Skill2 = 0, Skill3 = 0, Skill4 = 0, Skill5 = 0;

    ESPObject(ESPType type, int id, Il2CppObject *ptr, uint32_t handle, std::string name)
            : Type(type), ObjectID(id), ObjectPtr(ptr), GCHandle(handle), Name(name) {
    }
};

class ESP {
private:
    Vector2 MiniMapPosition;
    Vector2 MiniMapSize;

    std::vector<ESPObject> AllEntities;
    std::vector<ESPObject> Buckets[(int)ESPType::Count];

    void FreeHandlesInList(const std::vector<ESPObject> &list) {
        for (const auto &obj : list) {
            if (obj.GCHandle != 0) il2cpp_gchandle_free(obj.GCHandle);
        }
    }

public:
    ~ESP() { ClearAll(); }

    Vector2 AimVector = { 0 };

    Vector2 get_MiniMapPosition() { return MiniMapPosition; }
    void set_MiniMapPosition(Vector2 position) { MiniMapPosition = position; }

    Vector2 get_MiniMapSize() { return MiniMapSize; }
    void set_MiniMapSize(Vector2 size) { MiniMapSize = size; }

    void Add(ESPType type, int id, Il2CppObject *ptr, uint32_t handle, std::string name) {
        int idx = (int)type;
        if (idx >= 0 && idx < (int)ESPType::Count) Buckets[idx].emplace_back(type, id, ptr, handle, name);
    }

    void Remove(ESPType type, int id) {
        int typeIndex = (int)type;
        if (typeIndex < 0 || typeIndex >= (int)ESPType::Count) return;

        auto &bucket = Buckets[typeIndex];

        for (size_t i = 0; i < bucket.size(); ++i) {
            if (bucket[i].ObjectID == id) {
                if (bucket[i].GCHandle != 0) il2cpp_gchandle_free(bucket[i].GCHandle);
                if (i != bucket.size() - 1) bucket[i] = std::move(bucket.back());
                bucket.pop_back();
                return;
            }
        }
    }

    std::vector<ESPObject> &Get(ESPType type) {
        return Buckets[(int)type];
    }

    void ClearType(ESPType type) {
        int typeIndex = (int)type;
        if (typeIndex >= 0 && typeIndex < (int)ESPType::Count) {
            FreeHandlesInList(Buckets[typeIndex]);
            Buckets[typeIndex].clear();
        }
    }

    void ClearAll() {
        for (int i = 0; i < (int)ESPType::Count; i++) {
            FreeHandlesInList(Buckets[i]);
            Buckets[i].clear();
        }
    }
};
