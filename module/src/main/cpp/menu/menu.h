#pragma once

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_android.h"
#include "il2cpp-resolver.h"

#include "menu-config.h"
#include "menu-input.h"
#include "menu-draw.h"
#include "menu-esp.h"

#include "font.h"

static bool menu_inited = false;
static bool menu_show = false;
static float scaleFactor = 1.0f;
static float icon_size = 22.0f;
static ImVec2 menuPos = ImVec2(100, 100);

int screenWidth = 0;
int screenHeight = 0;

MenuConfig menu_config;
ESP ESPManager;

int32_t (*get_width)(const MethodInfo*) = nullptr;
int32_t (*get_height)(const MethodInfo*) = nullptr;

void menu_init(Il2CppDomain *domain) {
    auto UnityEngine = get_image(domain, "UnityEngine.CoreModule.dll");
    if (!UnityEngine) UnityEngine = get_image(domain, "UnityEngine.dll");
    auto Screen = il2cpp_class_from_name(UnityEngine, "UnityEngine", "Screen");

    get_width = (int32_t(*)(const MethodInfo*))(void*)get_method(Screen, "get_width")->methodPointer;
    get_height = (int32_t(*)(const MethodInfo*))(void*)get_method(Screen, "get_height")->methodPointer;

    ImGui::CreateContext();
    ImGuiStyle &style = ImGui::GetStyle();

    ImGui_ImplOpenGL3_Init("#version 300 es");

    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.16f, 0.16f, 0.21f, 1.0f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.74f, 0.58f, 0.98f, 1.0f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImColor(76, 76, 76, 150);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.19f, 0.2f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.16f, 0.16f, 0.21f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImColor(76, 76, 76, 150);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.2f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.16f, 0.16f, 0.21f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.74f, 0.58f, 0.98f, 1.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.1f, 0.1f, 0.13f, 0.92f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.44f, 0.37f, 0.61f, 0.54f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.74f, 0.58f, 0.98f, 0.54f);
    style.Colors[ImGuiCol_FrameBg] = ImColor(64, 64, 74, 150);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.2f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.16f, 0.21f, 1.0f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.16f, 0.21f, 1.0f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.24f, 0.24f, 0.32f, 1.0f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.2f, 0.22f, 0.27f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.16f, 0.16f, 0.21f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.16f, 0.16f, 0.21f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.16f, 0.16f, 0.21f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.21f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16f, 0.16f, 0.21f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1f, 0.1f, 0.13f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.16f, 0.16f, 0.21f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.19f, 0.2f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.24f, 0.24f, 0.32f, 1.0f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.44f, 0.37f, 0.61f, 1.0f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.74f, 0.58f, 0.98f, 1.0f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.84f, 0.58f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.44f, 0.37f, 0.61f, 0.29f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.74f, 0.58f, 0.98f, 0.29f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.84f, 0.58f, 1.0f, 0.29f);

    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->Clear();

    ImFontConfig cfg_base;
    cfg_base.FontDataOwnedByAtlas = false;
    cfg_base.PixelSnapH = false;
    cfg_base.OversampleH = 5;
    cfg_base.OversampleV = 5;
    cfg_base.RasterizerMultiply = 1.0f;

    io.FontDefault = io.Fonts->AddFontFromMemoryCompressedTTF(
            (void*)font_data,
            font_size,
            15.0f,
            &cfg_base,
            io.Fonts->GetGlyphRangesChineseFull()
    );
}

void menu_resize(ImGuiIO &io, ImGuiStyle &style) {
    io.DisplaySize = ImVec2((float)screenWidth, (float)screenHeight);

    float min_dim = std::min((float)screenWidth, (float)screenHeight);
    if (min_dim < 1.0f) min_dim = 1.0f;
    scaleFactor = min_dim / 400.0f;

    style.WindowPadding = ImVec2(8.0f * scaleFactor, 8.0f * scaleFactor);
    style.FramePadding = ImVec2(8.0f * scaleFactor, 6.0f * scaleFactor);
    style.ItemSpacing = ImVec2(8.0f * scaleFactor, 8.0f * scaleFactor);
    style.ItemInnerSpacing = ImVec2(4.0f * scaleFactor, 4.0f * scaleFactor);
    style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
    style.IndentSpacing = 21.0f * scaleFactor;
    style.ColumnsMinSpacing = 6.0f * scaleFactor;
    style.ScrollbarSize = 18.0f * scaleFactor;
    style.GrabMinSize = 12.0f * scaleFactor;
    style.WindowBorderSize = 1.5f * scaleFactor;
    style.ChildBorderSize = 1.0f * scaleFactor;
    style.PopupBorderSize = 1.0f * scaleFactor;
    style.FrameBorderSize = 0.0f * scaleFactor;
    style.TabBorderSize = 0.0f * scaleFactor;
    style.WindowRounding = 7.0f * scaleFactor;
    style.ChildRounding = 7.0f * scaleFactor;
    style.FrameRounding = 4.0f * scaleFactor;
    style.PopupRounding = 4.0f * scaleFactor;
    style.ScrollbarRounding = 9.0f * scaleFactor;
    style.GrabRounding = 3.0f * scaleFactor;
    style.TabRounding = 4.0f * scaleFactor;
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);

    io.FontGlobalScale = scaleFactor;
}

void menu_icon(ImGuiIO &io, ImDrawList *drawList) {
    static bool isDragging = false;
    static bool wasDragging = false;
    static auto dragOffset = ImVec2(0, 0);

    float radius = icon_size * scaleFactor;
    float fontSize = icon_size * scaleFactor * 0.7f;

    float dx = io.MousePos.x - menuPos.x;
    float dy = io.MousePos.y - menuPos.y;
    float distance = sqrtf(dx * dx + dy * dy);
    bool isHovered = (distance <= radius);

    if (ImGui::IsMouseClicked(0) && isHovered) {
        isDragging = true;
        wasDragging = false;
        dragOffset = ImVec2(io.MousePos.x - menuPos.x, io.MousePos.y - menuPos.y);
    }

    if (isDragging) {
        if (ImGui::IsMouseDown(0)) {
            if (io.MouseDragMaxDistanceSqr[0] > 10.0f) {
                wasDragging = true;
            }
            menuPos.x = io.MousePos.x - dragOffset.x;
            menuPos.y = io.MousePos.y - dragOffset.y;
        } else {
            if (!wasDragging) {
                menu_show = !menu_show;
            }
            isDragging = false;
        }
    }

    menuPos.x = ImClamp(menuPos.x, radius, (float)screenWidth - radius);
    menuPos.y = ImClamp(menuPos.y, radius, (float)screenHeight - radius);

    auto circleColor = ImColor(0.16f, 0.16f, 0.21f, 1.0f);
    if (isDragging) circleColor = ImColor(0.24f, 0.24f, 0.32f, 1.0f);
    else if (isHovered) circleColor = ImColor(0.19f, 0.2f, 0.25f, 1.0f);

    ImDraw::DrawCircle(drawList, true, menuPos, radius, circleColor);
    ImDraw::DrawCircle(drawList, false, menuPos, radius, ImColor(0.74f, 0.58f, 0.98f, 1.0f), 1.5f * scaleFactor);
    ImDraw::DrawText(drawList, nullptr, fontSize, ImVec2(menuPos.x, menuPos.y), ImColor(1.0f, 1.0f, 1.0f, 1.0f), "KINO");
}

void menu_draw(ImGuiIO &io, ImGuiStyle &style) {
    ImDrawList *drawList = ImGui::GetForegroundDrawList();
    ImDraw::DrawRect(drawList, true, ImVec2((float)screenWidth / 2.0f - 55.0f * scaleFactor, (float)screenHeight - 10.0f * scaleFactor), ImVec2((float)screenWidth / 2.0f + 55.0f * scaleFactor, (float)screenHeight + 10.0f * scaleFactor), ImColor(0.0f, 0.0f, 0.0f, 1.0f), 20.0f * scaleFactor);
    ImDraw::DrawText(drawList, nullptr, 8.0f * scaleFactor, ImVec2((float)screenWidth / 2.0f, (float)screenHeight - 5.0f * scaleFactor), ImColor(1.0f, 1.0f, 1.0f, 1.0f), "Powered By Zygisk");
    if (menu_show) {
        static bool isDragging = false;
        static bool dragLocked = false;
        static int count = 0;

        float min_dim = std::min((float)screenWidth, (float)screenHeight);
        auto menuSize = ImVec2(min_dim * 0.7f, min_dim * 0.7f);
        float radius = icon_size * scaleFactor;

        auto initPos = ImVec2(
                ImClamp(menuPos.x - radius, 0.0f, (float)screenWidth - menuSize.x),
                ImClamp(menuPos.y - radius, 0.0f, (float)screenHeight - menuSize.y)
        );

        ImGui::SetNextWindowPos(initPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(menuSize, ImGuiCond_Always);

        if (ImGui::Begin("Modded By Kinocrp", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove)) {
            if (ImGui::IsMouseDown(0)) {
                if (count < 2) {
                    if (ImGui::IsAnyItemHovered()) {
                        dragLocked = true;
                    } else {
                        dragLocked = false;
                    }
                } else if (!dragLocked && ImGui::IsMouseDragging(0) && ImGui::IsWindowHovered()) {
                    isDragging = true;
                }
                count++;
            } else {
                isDragging = false;
                dragLocked = false;
                count = 0;
            }

            if (isDragging) {
                menuPos.x += io.MouseDelta.x;
                menuPos.y += io.MouseDelta.y;
                menuPos.x = ImClamp(menuPos.x, 0.0f, (float)screenWidth - menuSize.x + radius);
                menuPos.y = ImClamp(menuPos.y, 0.0f, (float)screenHeight - menuSize.y + radius);
            }

            ImGui::Checkbox("ESP", &menu_config.IsESP);
            ImGui::Checkbox("Reach", &menu_config.IsReach);
            ImGui::Checkbox("Aimbot", &menu_config.IsAimbot);
            ImGui::Checkbox("Mod Skin", &menu_config.IsModSkin);
            ImGui::SliderFloat("Drone View", &menu_config.CameraHeight, 0.0f, 2.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

            ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetFrameHeight() - style.WindowPadding.y);
            if (ImGui::Button("Close Menu", ImVec2(-FLT_MIN, 0))) menu_show = false;
        }
        ImGui::End();
    } else {
        menu_icon(io, drawList);
    }
}

Vector3 GetMapLocation(Vector3 location, Vector2 MiniMapPosition, Vector2 MiniMapSize) {
    location.x += 54100.0f;
    location.z += 54100.0f;
    if (HostCampID == 2) {
        location.x = 108300.0f - location.x;
        location.z = 108300.0f - location.z;
    }
    location.x = MiniMapPosition.x - MiniMapSize.x / 2.0f + MiniMapSize.x * location.x / 108300.0f;
    location.z = MiniMapPosition.y + MiniMapSize.y / 2.0f - MiniMapSize.y * location.z / 108300.0f;
    return location;
}

void menu_esp() {
    if (!menu_config.IsESP || !IsFight) return;
    auto background = ImGui::GetBackgroundDrawList();
    auto foreground = ImGui::GetForegroundDrawList();

    Vector2 MiniMapPosition = ESPManager.get_MiniMapPosition();
    Vector2 MiniMapSize = ESPManager.get_MiniMapSize();
    MiniMapPosition.y = (float)screenHeight - MiniMapPosition.y;

    int aliveEnemies = 0;
    int totalEnemies = 0;
    for (auto &obj : ESPManager.Get(ESPType::Hero)) {
        if (obj.CampID == HostCampID) continue;
        totalEnemies++;
        if (obj.CurrentHP <= 0) continue;
        Vector3 screenPosition = obj.ScreenPosition;
        float espSize = 3.2f - CameraHeightTotal;
        if (espSize < 1.0f) espSize = 1.0f;
        espSize *= scaleFactor;
        ImDraw::DrawLine(background, ImVec2((float)screenWidth / 2.0f, 0.0f), ImVec2(screenPosition.x, screenPosition.y - 15.0f * espSize), ImColor(1.0f, 1.0f, 1.0f, 1.0f), 1.0f * scaleFactor);
        ImDraw::DrawRect(background, false, ImVec2(screenPosition.x - 7.0f * espSize, screenPosition.y - 15.0f * espSize), ImVec2(screenPosition.x + 7.0f * espSize, screenPosition.y + 15.0f * espSize), ImColor(1.0f, 1.0f, 1.0f, 1.0f), 0.0f, 1.0f * scaleFactor);

        ImDraw::DrawRect(background, true, ImVec2(screenPosition.x - 30.0f * scaleFactor, screenPosition.y + 15.0f * espSize + 5.0f * scaleFactor), ImVec2(screenPosition.x + 30.0f * scaleFactor, screenPosition.y + 15.0f * espSize + 20.0f * scaleFactor), ImColor(0.0f, 0.0f, 0.0f, 1.0f), 20.0f * scaleFactor);
        ImDraw::DrawRect(background, true, ImVec2(screenPosition.x - 22.0f * scaleFactor, screenPosition.y + 15.0f * espSize + 5.0f * scaleFactor), ImVec2(screenPosition.x + (-22.0f + 44.0f * ((float)obj.CurrentHP / (float)obj.TotalHP)) * scaleFactor, screenPosition.y + 15.0f * espSize + 6.0f * scaleFactor), ImColor(0.0f, 1.0f, 0.0f, 1.0f));
        ImDraw::DrawText(background, nullptr, 12.0f * scaleFactor, ImVec2(screenPosition.x, screenPosition.y + 15.0f * espSize + 12.5f * scaleFactor), ImColor(1.0f, 1.0f, 1.0f, 1.0f), obj.Name.c_str());

        ImDraw::DrawText(background, nullptr, 15.0f * scaleFactor, ImVec2(screenPosition.x + 40.0f * scaleFactor, screenPosition.y + 15.0f * espSize + 30.0f * scaleFactor), ImColor(1.0f, 1.0f, 1.0f, 1.0f), std::to_string(obj.Skill1).c_str());
        ImDraw::DrawText(background, nullptr, 15.0f * scaleFactor, ImVec2(screenPosition.x + 20.0f * scaleFactor, screenPosition.y + 15.0f * espSize + 30.0f * scaleFactor), ImColor(1.0f, 1.0f, 1.0f, 1.0f), std::to_string(obj.Skill2).c_str());
        ImDraw::DrawText(background, nullptr, 15.0f * scaleFactor, ImVec2(screenPosition.x, screenPosition.y + 15.0f * espSize + 30.0f * scaleFactor), ImColor(1.0f, 1.0f, 1.0f, 1.0f), std::to_string(obj.Skill3).c_str());
        ImDraw::DrawText(background, nullptr, 15.0f * scaleFactor, ImVec2(screenPosition.x - 20.0f * scaleFactor, screenPosition.y + 15.0f * espSize + 30.0f * scaleFactor), ImColor(0.0f, 1.0f, 0.0f, 1.0f), std::to_string(obj.Skill4).c_str());
        ImDraw::DrawText(background, nullptr, 15.0f * scaleFactor, ImVec2(screenPosition.x - 40.0f * scaleFactor, screenPosition.y + 15.0f * espSize + 30.0f * scaleFactor), ImColor(0.8f, 0.5f, 1.0f, 1.0f), std::to_string(obj.Skill5).c_str());
        aliveEnemies++;
    }
    ImDraw::DrawRect(foreground, true, ImVec2((float)screenWidth / 2.0f - 20.0f * scaleFactor, -10.0f * scaleFactor), ImVec2((float)screenWidth / 2.0f + 20.0f * scaleFactor, 10.0f * scaleFactor), ImColor(0.0f, 0.0f, 0.0f, 1.0f), 20.0f * scaleFactor);
    ImDraw::DrawText(foreground, nullptr, 8.0f * scaleFactor, ImVec2((float)screenWidth / 2.0f, 5.0f * scaleFactor), ImColor(1.0f, 1.0f, 1.0f, 1.0f), (std::to_string(aliveEnemies) + " / " + std::to_string(totalEnemies)).c_str());

    ImDraw::DrawRect(background, false, ImVec2(MiniMapPosition.x - MiniMapSize.x / 2, MiniMapPosition.y - MiniMapSize.y / 2), ImVec2(MiniMapPosition.x + MiniMapSize.x / 2, MiniMapPosition.y + MiniMapSize.y / 2), ImColor(1.0f, 1.0f, 0.0f, 1.0f), 0, 2.0f * scaleFactor);

    for (auto &obj : ESPManager.Get(ESPType::Monster)) {
        Vector3 location = GetMapLocation(obj.Location, MiniMapPosition, MiniMapSize);
        Vector3 screenPosition = obj.ScreenPosition;
        if (obj.CurrentHP <= 1 || (obj.Location.x == 0.0f && obj.Location.z == 0.0f)) continue;
        ImDraw::DrawCircle(foreground, true, ImVec2(location.x, location.z), 3.0f * scaleFactor, (obj.CurrentHP >= obj.TotalHP) ? ImColor(1.0f, 1.0f, 0.0f, 1.0f) : ImColor(1.0f, 0.5f, 0.0f, 1.0f));
        ImDraw::DrawRect(foreground, true, ImVec2(screenPosition.x - 35.0f * scaleFactor, screenPosition.y - 10.0f * scaleFactor), ImVec2(screenPosition.x + 35.0f * scaleFactor, screenPosition.y + 10.0f * scaleFactor), ImColor(0.0f, 0.0f, 0.0f, 1.0f), 15.0f * scaleFactor);
        ImDraw::DrawText(foreground, nullptr, 17.0f * scaleFactor, ImVec2(screenPosition.x, screenPosition.y), ImColor(1.0f, 1.0f, 1.0f, 1.0f), std::to_string(obj.CurrentHP).c_str());
    }

    for (auto &obj : ESPManager.Get(ESPType::Hero)) {
        if (obj.CampID == HostCampID || obj.CurrentHP <= 0) continue;
        Vector3 location = GetMapLocation(obj.Location, MiniMapPosition, MiniMapSize);
        if (!obj.Visibility) ImDraw::DrawCircle(background, true, ImVec2(location.x, location.z), 7.7f * scaleFactor, ImColor(0.7f, 0.0f, 0.0f, 1.0f));
        ImDraw::DrawCircle(background, false, ImVec2(location.x, location.z), 7.9f * scaleFactor, (obj.Skill3 == 0) ? ImColor(0.0f, 1.0f, 0.0f, 1.0f) : ImColor(1.0f, 0.0f, 0.0f, 1.0f), 1.5f * scaleFactor);
        ImDraw::DrawCircle(background, true, ImVec2(location.x - 5.0f * scaleFactor, location.z - 5.0f * scaleFactor), 5.0f * scaleFactor, ImColor(0.7f, 0.0f, 0.0f, 1.0f));
        ImDraw::DrawCircle(background, true, ImVec2(location.x - 5.0f * scaleFactor, location.z - 5.0f * scaleFactor), 3.5f * scaleFactor, (obj.Skill5 == 0) ? ImColor(0.0f, 1.0f, 0.0f, 1.0f) : ImColor(0.5f, 0.0f, 0.0f, 1.0f));
        ImDraw::DrawRightArcHP(background, ImVec2(location.x, location.z), 10.6f * scaleFactor, ((float)obj.CurrentHP / (float)obj.TotalHP), 1.5f * scaleFactor);
        ImDraw::DrawRect(foreground, true, ImVec2(location.x - 20.0f * scaleFactor, location.z + 10.0f * scaleFactor), ImVec2(location.x + 20.0f * scaleFactor, location.z + 22.0f * scaleFactor), ImColor(0.0f, 0.0f, 0.0f, 0.5f), 12.0f * scaleFactor);
        ImDraw::DrawText(foreground, nullptr, 8.0f * scaleFactor, ImVec2(location.x, location.z + 16.0f * scaleFactor), ImColor(1.0f, 1.0f, 1.0f, 1.0f), obj.Name.c_str());
    }
}

void menu_render() {
    if (!menu_inited) {
        auto domain = il2cpp_domain_get();
        il2cpp_thread_attach(domain);
        menu_init(domain);
        menu_input_init(domain);
        menu_inited = true;
    }
    screenWidth = get_width(nullptr);
    screenHeight = get_height(nullptr);

    ImGuiIO &io = ImGui::GetIO();
    ImGuiStyle &style = ImGui::GetStyle();

    menu_resize(io, style);
    menu_input(io, screenHeight);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    menu_esp();
    menu_draw(io, style);

    ImGui::EndFrame();
    ImGui::Render();

    GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
    GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);

    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glScissor(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glViewport(last_viewport[0], last_viewport[1], last_viewport[2], last_viewport[3]);
    glScissor(last_scissor_box[0], last_scissor_box[1], last_scissor_box[2], last_scissor_box[3]);
}
