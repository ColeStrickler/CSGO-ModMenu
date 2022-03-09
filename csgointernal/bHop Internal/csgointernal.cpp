// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include <iostream>
#include <cmath>

bool bRadar = false, bFlash = false, bT = false, bGlow = false;

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

}offsets;


struct values {
    DWORD localPlayer;
    DWORD process;
    DWORD gameModule;
    BYTE flag;
    int myTeam;
    int trigDelay;
}val;

struct vector {
    float x, y, z;
};


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









void HackThread() {
    AllocConsole();
    freopen("CONOUT$", "w", stdout);



    std::cout << "[NUMPAD1] TOGGLE RADAR HACKS" << std::endl;
    std::cout << "[NUMPAD2] TOGGLE FLASH PROTECT" << std::endl;
    std::cout << "[NUMPAD3] TOGGLE TRIGGERBOT" << std::endl;
    std::cout << "[NUMPAD4] TOGGLE WALLHACKS" << std::endl;



    val.gameModule = (DWORD)GetModuleHandle("client.dll");
    val.localPlayer = *(DWORD*)(val.gameModule + offsets.lPlayer);
    if (val.localPlayer == NULL) {
        while (val.localPlayer == NULL) {
            val.localPlayer = *(DWORD*)(val.gameModule + offsets.lPlayer);
        }
    }
    std::cout << "Player Address: " << std::hex << val.localPlayer << std::endl;



    while (true) {
        {
            // bitwise flag for the bunnyhop
            val.flag = *(BYTE*)(val.localPlayer + offsets.flags);


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
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)HackThread, NULL, NULL, NULL);
    }

    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

