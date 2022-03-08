// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include <iostream>
#include <cmath>

bool bRadar = false, bFlash = false, bT = false;

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












void HackThread() {
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    
    
    
    std::cout << "[NUMPAD1] TOGGLE RADAR HACKS" << std::endl;
    std::cout << "[NUMPAD2] TOGGLE FLASH PROTECT" << std::endl;
    std::cout << "[NUMPAD3] TOGGLE TRIGGERBOT" << std::endl;
    
    
    
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
            val.flag = *(BYTE*)(val.localPlayer + offsets.flags);

            if (GetAsyncKeyState(VK_SPACE) && val.flag & (1 << 0)) {
                *(DWORD*)(val.gameModule + offsets.fJump) = 2;
            }
            if (GetAsyncKeyState(VK_NUMPAD1) & 1) {
                bRadar = !bRadar;
                if (!bRadar) {
                    std::cout << "[RADAR OFF]" << std::endl;
                }
            }
            if (GetAsyncKeyState(VK_NUMPAD2) & 1) {
                bFlash = !bFlash;
            }
            if (GetAsyncKeyState(VK_NUMPAD3) & 1) {
                val.myTeam = *(int*)(val.localPlayer + offsets.m_iTeamNum);
                bT = !bT;
            }


            // Continuous write base on booleans

            if (bRadar) {

                for (short int i = 0; i < 64; i++) {
                    DWORD entity = *(DWORD*)(val.gameModule + offsets.entityList + i * 0x10);
                    
                    if (entity != NULL) {
                        *(BOOL*)(entity + offsets.m_bSpotted) = true;
                                             
                    }
                }
            }
            if (bFlash) {
                if (*(int*)(val.localPlayer + offsets.m_flFlashDuration) > 0) {
                    std::cout << "entered loop" << std::endl;
                    *(int*)(val.localPlayer + offsets.m_flFlashDuration) = 0;

                }
            }
            if (bT) {
                handleTb();
            }
            
            
        } }

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

