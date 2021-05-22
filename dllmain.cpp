// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
// #define _DEBUG
const char* APPLICATION_ID = "844315251460931585";

uintptr_t getPointerToAddress(uintptr_t pointer, std::vector<DWORD> offsets);

uintptr_t WINAPI MainThread(HMODULE hModule)
{
    MessageBoxA(NULL, "This game is running LRPC. Made by Larko", "LRPC", MB_OK);

#ifdef _DEBUG
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
#endif

    const uintptr_t moduleBase = (uintptr_t) GetModuleHandle(L"DaysGone.exe");

    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));

    Discord_Initialize(APPLICATION_ID, &handlers, 1, NULL);

    DiscordRichPresence presence;
    memset(&presence, 0, sizeof(presence));

    presence.startTimestamp = time(0);
    presence.details = "Loading into the game";
    presence.largeImageKey = "logo";

    Discord_UpdatePresence(&presence);

    const DWORD daysGoneOffset = 0x410;
    const DWORD gameTimerOffset = 0x38;
    const DWORD moneyOffset = 0x74;

    int gameTimer = 0;

    while (true)
    {     
        const uintptr_t gameManager = getPointerToAddress(moduleBase + 0x4259800, { 0x7B0, 0xD0, 0x530, 0x550 });
        if (!gameManager) continue;

        const uintptr_t currency = getPointerToAddress(moduleBase + 0x4258338, { 0x20, 0xbe8, 0xe0, 0x10 });

#ifdef _DEBUG
        std::cout << "Got GameManager pointer = " << std::hex << gameManager << std::endl;
#endif

        // get the address gamemanager is pointing too, add the daysGoneOffset, cast them to an int pointer
        // and dereference it
        const int daysGone = *(int*)(*(uintptr_t*)gameManager + daysGoneOffset);
#ifdef _DEBUG
        std::cout << "Got days gone with offset 0x410 on GameManager = " << std::dec << daysGone << std::endl;
#endif

        const std::string sDaysGone = std::to_string(daysGone) + " days gone";

        switch (daysGone)
        {
            case 0:
                presence.details = "Loading into the game";
                presence.largeImageText = "Loading into the game";
                presence.state = "";
                break;
            case 730:
                presence.details = "On the main menu";
                presence.largeImageText = "On the main menu";
                presence.state = "";
                break;
            default:
                presence.details = *(int*)(*(uintptr_t*)gameManager + gameTimerOffset) == gameTimer ? "On the pause menu" : "Exploring the map";
                presence.state = sDaysGone.c_str();
                presence.largeImageText = sDaysGone.c_str();
        }
        
        gameTimer = *(int*)(*(uintptr_t*)gameManager + gameTimerOffset);

        Discord_UpdatePresence(&presence);
        Sleep(4 * 1000);
    }

#ifdef _DEBUG
    fclose(f);
    FreeConsole();
#endif
    FreeLibraryAndExitThread(hModule, 0);
    
    return 0;
}

uintptr_t getPointerToAddress(uintptr_t pointer, std::vector<DWORD> offsets)
{
    uintptr_t node = pointer;
    for (size_t i = 0; i < offsets.size(); i++)
    {
        // safety
        if (i != 0 && (uintptr_t)(uintptr_t*)node == offsets[i-1])
        {
            return NULL;
        };
        // get the pointer the pointer is referencing
        node = *(uintptr_t*)node;
        node += offsets[i];
    }
    return node;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       uintptr_t  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr));
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

