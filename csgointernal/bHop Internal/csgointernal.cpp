// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include <iostream>
#include <cmath>
#include "trampoline.h"
#include "includes.h"
#include <sstream>
#include <string.h>

// Global Vars

bool bRadar = false, bFlash = false, bT = false, bGlow = false, bRecoil = false, bAimbot = false, bEsp = false, b2Dbox = false, b3Dbox = false, bSnap = false, bBars = false, bEspNums = false, bMenu = true;

double PI = 3.14159265358;

float ViewMatrix[16];

bool save1 = false, save2 = false, save3 = false, save4 = false, save5 = false;



// DirectX9 and Dummy Setups
void* d3d9Device[119]; // holds vtable 
BYTE EndSceneBytes[7]{ 0 };
tEndScene oEndScene = nullptr;
extern LPDIRECT3DDEVICE9 pDevice = nullptr;

// Structs

struct gameOffsets {
    DWORD flags = 0x104;
    DWORD lPlayer = 0xDB35DC;
    DWORD fJump = 0x5278DDC;
    DWORD m_bSpotted = 0x93D;
    DWORD entityList = 0x4DCEEAC;
    DWORD m_flFlashDuration = 0x10470;
    DWORD m_iCrosshairId = 0x11838;
    DWORD m_iTeamNum = 0xF4;
    DWORD m_iHealth = 0x100;
    DWORD dwForceAttack = 0x31FF3C0;
    DWORD m_vecOrigin = 0x138;
    DWORD dwGlowObjectManager = 0x5317308;
    DWORD m_iGlowIndex = 0x10488;
    DWORD m_bIsDefusing = 0x997C;
    DWORD dwClientState = 0x58CFC4;
    DWORD dwClientState_ViewAngles = 0x4D90;
    DWORD m_iShotsFired = 0x103E0;
    DWORD m_aimPunchAngle = 0x303C;
    DWORD m_vecViewOffset = 0x108;
    DWORD m_dwBoneMatrix = 0x26A8;
    DWORD dwClientState_MaxPlayer = 0x388;
    DWORD dwViewMatrix = 0x4DC07C4;
    DWORD m_ArmorValue = 0x117CC;
    DWORD m_angEyeAnglesX = 0x117D0;
    DWORD m_angEyeAnglesY = 0x117D4;
    DWORD m_iItemIDHigh = 0x2FD0;
    DWORD m_nFallbackPaintKit = 0x31D8;
    DWORD m_flFallbackWear = 0x31E0;
    DWORD m_nFallbackStatTrak = 0x31E4;
    DWORD m_nFallbackSeed = 0x31DC;
    DWORD m_iEntityQuality = 0x2FBC;
    DWORD m_hMyWeapons = 0x2E08;
    DWORD m_iItemDefinitionIndex = 0x2FBA;
    DWORD clientstate_delta_ticks = 0x174;
}offsets;

struct values {
    DWORD localPlayer;
    DWORD process;
    DWORD gameModule;
    DWORD engineModule;
    BYTE flag;
    int myTeam;
    int trigDelay;
}val;

struct vector {
    float x, y, z;

    vector operator+ (vector a) {
        return { x + a.x, y + a.y, z + a.z };
    }
    vector operator- (vector a) {
        return { x - a.x, y - a.y, z - a.z };
    }
    vector operator* (float b) {
        return { x * b, y * b, z * b };
    }
    void antiVAC() {
        while (y < -180) {y += 360;}
        while (y > 180) { y -= 360;}
        if (x > 89) { x = 89;}
        if (x < 89) { x = -89;}
    }
};



// Aimbot Functions

float aimbotDistance(vector* other) {
    vector localPlayerPos = *(vector*)(val.localPlayer + offsets.m_vecOrigin);
    vector delta;


    delta.x = (other->x - localPlayerPos.x);
    delta.y = (other->y - localPlayerPos.y);
    delta.z = (other->z - localPlayerPos.z);
    return sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
}

void lockOn(vector target) {
    vector* viewAngles = (vector*)(*(DWORD*)(val.engineModule + offsets.dwClientState) + offsets.dwClientState_ViewAngles);
    vector origin = *(vector*)(val.localPlayer + offsets.m_vecOrigin);
    vector viewOffset = *(vector*)(val.localPlayer + offsets.m_vecViewOffset);

    vector local = (origin + viewOffset);
    vector* localPos = &local;

    vector deltaVec;
    deltaVec.x = (target.x - localPos->x);
    deltaVec.y = (target.y - localPos->y);
    deltaVec.z = (target.z - localPos->z);

    float deltaVecLength = sqrt(deltaVec.x * deltaVec.x + deltaVec.y * deltaVec.y + deltaVec.z * deltaVec.z);

    // pitch = up/down
    // convert to degrees
    float pitch = -asin(deltaVec.z / deltaVecLength) * (180 / PI);
    // finding side to side change 
    float yaw = atan2(deltaVec.y, deltaVec.x) * (180 / PI);

    if (pitch > 89) {
        pitch = 89;
    }
    if (pitch < -89) {
        pitch = -89;
    }
    if (yaw > 179) {
        yaw = 179;
    }
    if (yaw < -179) {
        yaw = -179;
    }


    viewAngles->x = pitch;
    viewAngles->y = yaw;
}

vector getBoneLocation(DWORD entity, int boneId) {
    DWORD boneMatrix = *(DWORD*)(entity + offsets.m_dwBoneMatrix);
    vector bonePosition;
    bonePosition.x = *(float*)(boneMatrix + 0x30 * boneId + 0x0c); // 8 = Head Bone
    bonePosition.y = *(float*)(boneMatrix + 0x30 * boneId + 0x1c);
    bonePosition.z = *(float*)(boneMatrix + 0x30 * boneId + 0x2c);
    return bonePosition;
}

DWORD chooseTarget() {

    float closestDistance = 1000000;
    int closestDistanceIndex = -1;
    int localPlayerTeam = *(int*)(val.localPlayer + offsets.m_iTeamNum);
    int maxPlayer = *(int*)(*(DWORD*)(val.engineModule + offsets.dwClientState) + offsets.dwClientState_MaxPlayer);
    std::cout << maxPlayer << std::endl;
    for (int i = 1; i < maxPlayer; i++) {
        DWORD entity = *(DWORD*)(val.gameModule + offsets.entityList + i * 0x10);
        if (entity == NULL) {
            continue;
        }
        int entityTeam = *(int*)(entity + offsets.m_iTeamNum);
        if (localPlayerTeam == entityTeam) {
            continue;
        }
        int localPlayerHealth = *(int*)(val.localPlayer + offsets.m_iHealth);
        int entityHealth = *(int*)(entity + offsets.m_iHealth);
        if (localPlayerHealth < 1 || entityHealth < 1) {
            continue;
        }
        float distance = aimbotDistance((vector*)(entity + offsets.m_vecOrigin));
        if (distance < closestDistance) {
            closestDistance = distance;
            closestDistanceIndex = i;
        }
    }
    if (closestDistanceIndex == -1) {
        return NULL;
    }
    DWORD bestTarget = *(DWORD*)(val.gameModule + offsets.entityList + closestDistanceIndex * 0x10);
    return bestTarget;
}



// ESP

void updateViewMatrix() {
    memcpy_s(&ViewMatrix, sizeof(ViewMatrix), (PBYTE*)(val.gameModule + offsets.dwViewMatrix), sizeof(ViewMatrix));
}

bool checkEnt(DWORD entity) {
    if (entity == NULL) {
        return false;
    }
    if (entity == val.localPlayer) {
        return false;
    }
    int entityHealth = *(int*)(entity + offsets.m_iHealth);
    if (entityHealth <= 0) {
        return false;
    }
    return true;
}
// converts 3d image to 2D drawing for ESP
// fancy matrix math, tutorials on youtube !!!
bool WorldToScreen(Vec3 pos, Vec2& screen) {
    Vec4 clipcoords;
    clipcoords.x = pos.x * ViewMatrix[0] + pos.y * ViewMatrix[1] + pos.z * ViewMatrix[2] + ViewMatrix[3];
    clipcoords.y = pos.x * ViewMatrix[4] + pos.y * ViewMatrix[5] + pos.z * ViewMatrix[6] + ViewMatrix[7];
    clipcoords.z = pos.x * ViewMatrix[8] + pos.y * ViewMatrix[9] + pos.z * ViewMatrix[10] + ViewMatrix[11];
    clipcoords.w = pos.x * ViewMatrix[12] + pos.y * ViewMatrix[13] + pos.z * ViewMatrix[14] + ViewMatrix[15];

    if (clipcoords.w < 0.1f) { // means its behind us
        return false;
    }

    Vec3 NDC;
    NDC.x = clipcoords.x / clipcoords.w;
    NDC.y = clipcoords.y / clipcoords.w;
    NDC.z = clipcoords.z / clipcoords.w;

    screen.x = (windowWidth / 2 * NDC.x) + (NDC.x + windowWidth / 2);
    // - just for special case in csgo
    screen.y = -(windowHeight / 2 * NDC.y) + (NDC.y + windowHeight / 2);
    return true;
}

// Hook Function
void APIENTRY hkEndScene(LPDIRECT3DDEVICE9 o_pDevice) {
    if (!pDevice) {
        pDevice = o_pDevice;
    }
    // Drawing text
    if (bMenu) {
        D3DCOLOR purple = D3DCOLOR_ARGB(255, 135, 12, 80);
        D3DCOLOR green = D3DCOLOR_ARGB(255, 38, 255, 0);
        TextToScreen("By Cole Strickler aka 1337Master", (float)(windowWidth / 2), (float)(windowHeight - 20), purple, 20, 15);
        if (bRadar) {
            TextToScreen("[NUMPAD1] RADAR HACKS", windowWidth / 2 - 120, 70, green, 14, 0);
        }
        else {
            TextToScreen("[NUMPAD1] RADAR HACKS", windowWidth / 2 - 120, 70, purple, 14, 0);
        }
        if (bFlash) {
            TextToScreen("[NUMPAD2] Flash Protection", windowWidth / 2 - 120, 90, green, 14, 0);
        }
        else {
            TextToScreen("[NUMPAD2] Flash Protection", windowWidth / 2 - 120, 90, purple, 14, 0);
        }
        if (bT) {
            TextToScreen("[NUMPAD3] Triggerbot", windowWidth / 2 - 120, 110, green, 14, 0);
        }
        else {
            TextToScreen("[NUMPAD3] TriggerBot", windowWidth / 2 - 120, 110, purple, 14, 0);
        }
        if (bGlow) {
            TextToScreen("[NUMPAD4] GLOW HACK", windowWidth / 2 - 120, 130, green, 14, 0);
        }
        else {
            TextToScreen("[NUMPAD4] GLOW HACK", windowWidth / 2 - 120, 130, purple, 14, 0);
        }
        if (bRecoil) {
            TextToScreen("[NUMPAD5] Recoil Control", windowWidth / 2 - 120, 150, green, 14, 0);
        }
        else {
            TextToScreen("[NUMPAD5] Recoil Control", windowWidth / 2 - 120, 150, purple, 14, 0);
        }
        if (bAimbot) {
            TextToScreen("[NUMPAD6] Aimbot", windowWidth / 2 - 120, 170, green, 14, 0);
        }
        else {
            TextToScreen("[NUMPAD6] Aimbot", windowWidth / 2 - 120, 170, purple, 14, 0);
        }
        if (bEsp) {
            TextToScreen("[NUMPAD7] ESP", windowWidth / 2 - 120, 190, green, 14, 0);
            if (b2Dbox) {
                TextToScreen("[END] 2D Box", windowWidth / 2 + 120, 70, green, 14, 0);
            }
            else {
                TextToScreen("[END] 2D Box", windowWidth / 2 + 120, 70, purple, 14, 0);
            }
            if (bSnap) {
                TextToScreen("[DEL] Snap Line", windowWidth / 2 + 120, 90, green, 14, 0);
            }
            else {
                TextToScreen("[DEL] Snap Line", windowWidth / 2 + 120, 90, purple, 14, 0);
            }
            if (bBars) {
                TextToScreen("[INS] HP & Armor Bars", windowWidth / 2 + 120, 110, green, 14, 0);
            }
            else {
                TextToScreen("[INS] HP & Armor Bars", windowWidth / 2 + 120, 110, purple, 14, 0);
            }
            if (b3Dbox) {
                TextToScreen("[HOME] 3D BOX", windowWidth / 2 + 120, 130, green, 14, 0);
            }
            else {
                TextToScreen("[HOME] 3D BOX", windowWidth / 2 + 120, 130, purple, 14, 0);
            }
            if (bEspNums) {
                TextToScreen("[PgUp] HP & Armor #", windowWidth / 2 + 120, 150, green, 14, 0);
            }
            else {
                TextToScreen("[PgUp] HP & Armor #", windowWidth / 2 + 120, 150, purple, 14, 0);
            }
        }
        else {
            TextToScreen("[NUMPAD7] ESP", windowWidth / 2 - 120, 190, purple, 14, 0);
        }
        TextToScreen("[NUMPAD8] Hide Menu", windowWidth / 2, 210, D3DCOLOR_ARGB(255, 255, 255, 255), 14, 0);


    }
    else {
        TextToScreen("[NUMPAD8] Show Menu", windowWidth / 2, 70, D3DCOLOR_ARGB(255, 255, 255, 255), 14, 0);
    }


    for (int i = 1; i < 32; i++) {
        DWORD entity = *(DWORD*)(val.gameModule + offsets.entityList + i * 0x10);
        if (checkEnt(entity)) {
            std::cout << entity << std::endl;
            int entTeam = *(int*)(entity + offsets.m_iTeamNum);
            int localPlayerTeam = *(int*)(val.localPlayer + offsets.m_iTeamNum);
            D3DCOLOR color = D3DCOLOR_ARGB(255, 0, 0, 0);
            if (entTeam == localPlayerTeam) {
                color = D3DCOLOR_ARGB(255, 0, 255, 0);
            }
            else {
                color = D3DCOLOR_ARGB(255, 255, 0, 0);
            }

            vector entHead = getBoneLocation(entity, 8);
            Vec3 entHead3D;
            entHead3D.x = entHead.x;
            entHead3D.y = entHead.y;
            entHead3D.z = entHead.z + 8;
            Vec2 entPos2D, entHead2D;
            // snapline
            Vec3 currentEntVecOrigin = *(Vec3*)(entity + offsets.m_vecOrigin);
            if (WorldToScreen(currentEntVecOrigin, entPos2D)) {
                if (bSnap) {
                    DrawLine(entPos2D.x, entPos2D.y, windowWidth / 2, windowHeight, 2, color);
                }
                if (WorldToScreen(entHead3D, entHead2D)) {
                    if (b2Dbox) {
                        DrawEspBox2D(entPos2D, entHead2D, 2, color);
                    }
                    if (b3Dbox) {
                        float entEyeAngY = *(float*)(entity + offsets.m_angEyeAnglesY);
                        DrawEspBox3D(entHead3D, currentEntVecOrigin, entEyeAngY, 25, 2, color);
                    }

                    if (bBars) {
                        // set up health and armor bar ESP here
                        int height = ABS(entPos2D.y - entHead2D.y);
                        int dX = (entPos2D.x - entHead2D.x);
                        float healthPercent = (*(int*)(entity + offsets.m_iHealth)) / 100.f;
                        float armorPercent = (*(int*)(entity + offsets.m_ArmorValue)) / 100.f;
                        Vec2 botHealthBar, topHealthBar, botArmorBar, topArmorBar;
                        int healthBarHeight = height * healthPercent;
                        int armorBarHeight = height * armorPercent;
                        botHealthBar.y = botArmorBar.y = entPos2D.y;
                        botHealthBar.x = entPos2D.x - (height / 4) - 4;
                        botArmorBar.x = entPos2D.x + (height / 4) + 4;
                        topHealthBar.y = entHead2D.y + height - healthBarHeight;
                        topArmorBar.y = entHead2D.y + height - armorBarHeight;
                        topHealthBar.x = entPos2D.x - (height / 4) - 4 - (dX + healthPercent);
                        topArmorBar.x = entPos2D.x + (height / 4) + 4 - (dX * armorPercent);
                        DrawLine2(topHealthBar, botHealthBar, 6, D3DCOLOR_ARGB(255, 204, 255, 0));
                        DrawLine2(topArmorBar, botArmorBar, 6, D3DCOLOR_ARGB(255, 0, 26, 255));
                    }
                    if (bEspNums) {
                        std::stringstream s1, s2;
                        s1 << (*(int*)(entity + offsets.m_iHealth));
                        s2 << (*(int*)(entity + offsets.m_ArmorValue));
                        std::string healthstring = "HP: " + s1.str();
                        std::string armorstring = "AP: " + s2.str();
                        char* healthMessage = (char*)healthstring.c_str();
                        char* armorMessage = (char*)armorstring.c_str();

                        TextToScreen(healthMessage, entPos2D.x, entPos2D.y, D3DCOLOR_ARGB(255, 255, 255, 255), 12, 0);
                        TextToScreen(armorMessage, entPos2D.x, entPos2D.y + 15, D3DCOLOR_ARGB(255, 255, 255, 255), 12, 0);
                    }

                }
            }
        }
    }

    oEndScene(pDevice);
}



// Triggerbot Functions
void getDistance(DWORD entity) {
    vector localPlayerLoc = *(vector*)(val.localPlayer + offsets.m_vecOrigin);
    vector enemyLoc = *(vector*)(entity + offsets.m_vecOrigin);
    float distance = sqrt(pow(localPlayerLoc.x - enemyLoc.x, 2) + pow(localPlayerLoc.y - enemyLoc.y, 2) + pow(localPlayerLoc.z - enemyLoc.z, 2)) * 0.0254;
    val.trigDelay = distance * 2.55;

}
void fire() {

    *(int*)(val.gameModule + offsets.dwForceAttack) = 5;
    Sleep(val.trigDelay);
    Sleep(15);
    *(int*)(val.gameModule + offsets.dwForceAttack) = 4;
}
bool checkT() {
   
    int crosshair = *(int*)(val.localPlayer + offsets.m_iCrosshairId);
    if (crosshair != 0 && crosshair < 64) {
        DWORD entity = *(DWORD*)(val.gameModule + offsets.entityList + (crosshair - 1) * 0x10);
        int entTeam = *(int*)(entity + offsets.m_iTeamNum);
        int entHealth = *(int*)(entity + offsets.m_iHealth);

        if (entTeam != val.myTeam && entHealth > 0) {
            getDistance(entity);
            return true;
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
   
}
void handleTb() {
    if (checkT()) {
        fire();
    }
}

// GlowHack Functions

struct glowStruct {
    BYTE base[8]; 
    float red;    // 0x8
    float green;  // 0xC
    float blue;   // 0x10
    float alpha;  // 0x14
    BYTE pad[16]; 
    bool renderOccluded; // 0x28
    bool renderNotOccluded; // 0x29
    bool fullBloom;  // 0x2A
    BYTE pad2[5];  
    int glowStyle; // 0x2F
}glowStr;

glowStruct setColor(glowStruct glow, DWORD entity) {
    int health = *(int*)(entity + offsets.m_iHealth);
    bool defusing = *(bool*)(entity + offsets.m_bIsDefusing);
    if (defusing) {
        glow.red = 1.0f;
        glow.green = 1.0f;
        glow.blue = 1.0f;
        glow.alpha = 1.0f;
    }
    else {
        glow.red = health * -0.01 + 1;
        glow.green = health * 0.01;
        glow.alpha = 1.0f;
    }
    glow.renderOccluded = true;
    glow.renderNotOccluded = false;
    return glow;
}

void setTeamGlow(DWORD entity, DWORD glowObject, int glowIndex) {
    glowStruct teamGlow;
    teamGlow = *(glowStruct*)(glowObject + (glowIndex * 0x38));
    teamGlow.blue = 1.0f;
    teamGlow.alpha = 1.0f;
    teamGlow.renderOccluded = true;
    teamGlow.renderNotOccluded = false;
    *(glowStruct*)(glowObject + (glowIndex * 0x38)) = teamGlow;
}

void setEnemyGlow(DWORD entity, DWORD glowObject, int glowIndex) {
    glowStruct enemyGlow;
    enemyGlow = *(glowStruct*)(glowObject + (glowIndex * 0x38));
    int health = *(int*)(entity + offsets.m_iHealth);
    enemyGlow = setColor(enemyGlow, entity);
    
    *(glowStruct*)(glowObject + (glowIndex * 0x38)) = enemyGlow;
}

void setGlow(DWORD entity) {
    DWORD glowObject = *(DWORD*)(val.gameModule + offsets.dwGlowObjectManager);
    int localPlayerTeam = *(int*)(val.localPlayer + offsets.m_iTeamNum);
    if (entity != NULL) {
        int entTeam = *(int*)(entity + offsets.m_iTeamNum);
        int glowIndex = *(int*)(entity + offsets.m_iGlowIndex);
        if (localPlayerTeam == entTeam) {
            setTeamGlow(entity, glowObject, glowIndex);
        }
        else {
            setEnemyGlow(entity, glowObject, glowIndex);
        }
    }
    
    
}









void pwn() {
   

    val.gameModule = (DWORD)GetModuleHandle("client.dll");
    val.engineModule = (DWORD)GetModuleHandle("engine.dll");

    

    val.localPlayer = *(DWORD*)(val.gameModule + offsets.lPlayer);
    if (val.localPlayer == NULL) {
        while (val.localPlayer == NULL) {
            val.localPlayer = *(DWORD*)(val.gameModule + offsets.lPlayer);
        }
    }
    std::cout << "Player Address: " << std::hex << val.localPlayer << std::endl;

    // hook if true
    if (GetD3D9Device(d3d9Device, sizeof(d3d9Device))) {
        // steal bytes from the Vtable for the end scene function
        memcpy(EndSceneBytes, (char*)d3d9Device[42], 7);
        // replaces 42nd entry of Vtable with our end scene function
        oEndScene = (tEndScene)TrampHook((char*)d3d9Device[42], (char*)hkEndScene, 7);
        }
   

    
    vector viewAngle = *(vector*)(*(DWORD*)(val.engineModule + offsets.dwClientState) + offsets.dwClientState_ViewAngles);
    vector origPunch = { 0,0,0 };

    

    while (true) {
        {
            // bitwise flag for the bunnyhop
            val.flag = *(BYTE*)(val.localPlayer + offsets.flags);
            // update view matrix for the esp
            updateViewMatrix();
            
            // CHANGE BOOLEANS BASED ON KEYS PRESSED 

            
            if (GetAsyncKeyState(VK_SPACE) && val.flag & (1 << 0)) {
                *(DWORD*)(val.gameModule + offsets.fJump) = 2;
            }
            if (GetAsyncKeyState(VK_NUMPAD1) & 1) {
                bRadar = !bRadar;
                std::cout << "[RADAR ON]" << std::endl;
            }
            if (GetAsyncKeyState(VK_NUMPAD2) & 1) {
                bFlash = !bFlash;
                std::cout << "[FLASH PROTECT ON]" << std::endl;
            }
            if (GetAsyncKeyState(VK_NUMPAD3) & 1) {
                val.myTeam = *(int*)(val.localPlayer + offsets.m_iTeamNum);
                bT = !bT;
                std::cout << "[TRIGGERBOT ON]" << std::endl;
            }
            if (GetAsyncKeyState(VK_NUMPAD4) & 1) {
                bGlow = !bGlow;
                if (bRadar = false) {
                    bRadar = !bRadar;
                }
                std::cout << "[GLOWHACK ON]" << std::endl;
            }
            if (GetAsyncKeyState(VK_NUMPAD5) & 1) {
                bRecoil = !bRecoil;
                std::cout << "[RCS ON]" << std::endl;
            }
            if (GetAsyncKeyState(VK_NUMPAD6) & 1) {
                bAimbot = !bAimbot;
                std::cout << "[AIMBOT ON]" << std::endl;
            }
            if (GetAsyncKeyState(VK_NUMPAD7) & 1) {
                bEsp = !bEsp;
                if (!bEsp) {
                    save1 = b2Dbox;
                    save2 = bSnap;
                    save3 = bBars;
                    save4 = b3Dbox;
                    save5 = bEspNums;

                    b2Dbox = false;
                    bSnap = false;
                    bBars = false;
                    b3Dbox = false;
                    bEspNums = false;
                }
                if (bEsp) {
                    b2Dbox = save1;
                    bSnap = save2;
                    bBars = save3;
                    b3Dbox = save4;
                    bEspNums = save5;
                }
            }
            if (GetAsyncKeyState(VK_NUMPAD8) & 1) {
                bMenu = !bMenu;
            }
            if (GetAsyncKeyState(VK_END) & 1) {
                b2Dbox = !b2Dbox;
                if (b2Dbox && b3Dbox) {
                    b3Dbox = false;
                }
            }
            if (GetAsyncKeyState(VK_DELETE) & 1) {
                bSnap = !bSnap;
            }
            if (GetAsyncKeyState(VK_INSERT) & 1) {
                bBars = !bBars;
                if (bEspNums && bBars) {
                    bEspNums = !false;
                }
            }
            if (GetAsyncKeyState(VK_HOME) & 1) {
                b3Dbox = !b3Dbox;
                if (b3Dbox && b2Dbox) {
                    b2Dbox = false;
                }
            }
            if (GetAsyncKeyState(VK_PRIOR) & 1) {
                bEspNums = !bEspNums;
                if (bEspNums && bBars) {
                    bBars = !false;
                }
            }



            // One single for loop over entity list for efficiency
            if (bRadar || bGlow) {
                for (short int i = 0; i < 64; i++) {
                    DWORD entity = *(DWORD*)(val.gameModule + offsets.entityList + i * 0x10);
                    if (bRadar) {
                        if (entity != NULL) {
                            *(BOOL*)(entity + offsets.m_bSpotted) = true;
                        }
                    }
                    if (bGlow) {
                        setGlow(entity);
                    }
                }
            }

            // Continuous write base on booleans


            if (bFlash) {
                if (*(int*)(val.localPlayer + offsets.m_flFlashDuration) > 0) {
                    *(int*)(val.localPlayer + offsets.m_flFlashDuration) = 0;
                }
            }
            if (bT) {
                handleTb();
            }
            if (bRecoil) {
                vector punchAngle = (*(vector*)(val.localPlayer + offsets.m_aimPunchAngle)) * 2;
                if ((*(int*)(val.localPlayer + offsets.m_iShotsFired)) > 1) {
                    // calc recoil
                    vector newAngle = viewAngle + origPunch - punchAngle;
                    // normalize and set
                    viewAngle = newAngle;
                }
                origPunch = punchAngle;
            }
            if (bAimbot) {
                DWORD bestTarget = chooseTarget();
                if ((GetKeyState(VK_LBUTTON) & 0x8000) != 0) {
                    if (bestTarget) {
                        lockOn(getBoneLocation(bestTarget, 8));

                        // AimAt(closestenemy->GetBonePos(8);
                    }
                }
            }



        }


    }
}






BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call,  LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)pwn, NULL, NULL, NULL);
    }

    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

