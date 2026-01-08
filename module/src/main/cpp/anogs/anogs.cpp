#include "anogs.h"
#include <cstdlib>
#include <cstring>
#include <cinttypes>
#include <unordered_set>
#include "log.h"
#include "xdl.h"
#include "dobby.h"

int h_gettimeofday(struct timeval *tv, struct timezone *tz) {
    return 0;
}

int64_t (*o_AnoSDKIoctl)(int64_t a1, int64_t a2, int64_t *a3, int64_t a4, uint64_t a5, uint64_t a6) = nullptr;
int64_t h_AnoSDKIoctl(int64_t a1, int64_t a2, int64_t *a3, int64_t a4, uint64_t a5, uint64_t a6) {

    static const std::unordered_set<int> blocked_cases = {
            1,  // Monitoring/Observation
            // DO NOT BLOCK THESE
            // 16, // Often Connection/Heartbeat Checks
            // 35, // Integrity Check A
            // 36, // Integrity Check B
            // 37  // Fire Ban
    };

    if (blocked_cases.count((int)a1)) {
        // LOGI("BLOCKED CASE: %d - %s", (int)a1, (const char*)a2);
        return 0;
    }
    // LOGI("ALLOWED CASE: %d - %s", (int)a1, (const char*)a2);
    return o_AnoSDKIoctl(a1, a2, a3, a4, a5, a6);
}

void (*o_AnoSDKForExport)() = nullptr;
void h_AnoSDKForExport() {
    void* v0 = malloc(0x31u);
    memset(v0, 0, 0x31u);
    o_AnoSDKForExport();
    free(v0);
}

int64_t (*o_IsEnable)(int64_t a1, int64_t a2, bool a3, char a4) = nullptr;
int64_t h_IsEnable(int64_t a1, int64_t a2, bool a3, char a4) {
    if (strstr((const char*)a2, "scan")) {
        // LOGI("BLOCKED: %s", (const char*)a2);
        return 0;
    }
    return o_IsEnable(a1, a2, a3, a4);
}

int64_t h_scan(const char *str) {
    if (strstr(str, "scan")) {
        // LOGI("BLOCKED: %s", str);
        return 0;
    }
    int64_t length = 0;
    while (str[length] != '\0') length++;
    return length;
}

void *(*o_memcpy)(void *dest, const void *src, size_t len) = nullptr;
void *h___memcpy_chk(void *dest, const void *src, size_t len, size_t destlen) {
    return o_memcpy(dest, src, len);
}

int h_memcmp(const void *s1, const void *s2, size_t n) {
    return 0;
}

void anogs_init(void *handle) {
    xdl_info_t info;
    xdl_info(handle, XDL_DI_DLINFO, &info);
    uint64_t base = (uint64_t)info.dli_fbase;
    LOGI("anogs_base: %" PRIx64 "", base);

    DobbyHook((void*)(base + 0x54A550), (void*)h_gettimeofday, nullptr);                        // Bypassing Emulator Detection And Fixing Crashes (gettimeofday Wrapper)
    DobbyHook((void*)(base + 0x1CC2B0), (void*)h_AnoSDKIoctl, (void**)&o_AnoSDKIoctl);          // Gating AnoSDKIoctl (Trampoline Found In AnoSDKIoctl)
    DobbyHook((void*)(base + 0x1D9F48), (void*)h_AnoSDKForExport, (void**)&o_AnoSDKForExport);  // Blocking AnoSDKForExport (AnoSDKForExport)
    DobbyHook((void*)(base + 0x2391D4), (void*)h_IsEnable, (void**)&o_IsEnable);                // Gating Cases (Found With tdm_report)
    DobbyHook((void*)(base + 0x4E0A10), (void*)h_scan, nullptr);                                // Gating Scan (Found In AnoSDKIoctl Case 10)
    o_memcpy = (void*(*)(void*, const void*, size_t))(void*)(base + 0x54A450);                  // memcpy (memcpy Wrapper)
    DobbyHook((void*)(base + 0x54A460), (void*)h___memcpy_chk, nullptr);                        // __memcpy_chk (__memcpy_chk Wrapper)
    DobbyHook((void*)(base + 0x54A7C0), (void*)h_memcmp, nullptr);                              // memcmp (memcmp Wrapper)
}