
#include "pch.h"
#include "processutil.h"
#include "winuser.h"
#include <vector>
#include <chrono>
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}



HelperFunctions HelperFunctionsGlobal;
std::string modpath;
int STATE_BALL = 5;
struct sonicSnapshot
{
    float xspeed;
    float normalx;
    float normaly;
    float normalz;
    
};

std::vector<sonicSnapshot> snapshots = std::vector<sonicSnapshot>();
std::string data_fn = "\\data.csv";




    



extern "C" {
    __declspec(dllexport) void __cdecl Init(const char* path, const HelperFunctions& helperFunctions)
    {

        HelperFunctionsGlobal = helperFunctions; // Save the helper pointer for external use
        modpath = path;
        

    }

    void UpdateOSD()
    {
        if (EntityData1Ptrs[0] == nullptr || CharObj2Ptrs[0] == nullptr) {
            SetDebugFontColor(0xFFFF0000);
            DisplayDebugString(NJM_LOCATION(3, 1), "NO CHARACTER LOADED");
            return;
        }
        SetDebugFontColor(0xFF00FFFF);
        DisplayDebugStringFormatted(NJM_LOCATION(3, 1), "Action: %d", EntityData1Ptrs[0]->Action);
        DisplayDebugStringFormatted(NJM_LOCATION(3, 2), "XSpeed: %.4f", CharObj2Ptrs[0]->Speed.x);
        NJS_VECTOR slope_normal = CharObj2Ptrs[0]->SurfaceNormal;
        DisplayDebugStringFormatted(NJM_LOCATION(3,3), "Slope Normal: (%.4f, %.4f, %.4f)", slope_normal.x, slope_normal.y, slope_normal.z);
        
    }

    __declspec(dllexport) void __cdecl OnFrame()
    {
        UpdateOSD();
        if (EntityData1Ptrs[0] != nullptr && CharObj2Ptrs[0] != nullptr) {
            if (EntityData1Ptrs[0]->Action == STATE_BALL) {
                if (snapshots.size() == 0) {
                    printf("Collecting Data\n");
                }
                struct sonicSnapshot snap = { CharObj2Ptrs[0]->Speed.x,CharObj2Ptrs[0]->SurfaceNormal.x,CharObj2Ptrs[0]->SurfaceNormal.y,CharObj2Ptrs[0]->SurfaceNormal.z };
                snapshots.push_back(snap);
            }
            else if (snapshots.size() > 0) {
                printf("Done Collecting Data\n");
                FILE* csvf;
                std::string out_path = modpath + data_fn;
                printf("Outputting to %s", out_path.c_str());
                fopen_s(&csvf, out_path.c_str(), "w");
                for (auto it = snapshots.begin(); it != snapshots.end(); it++) {
                    int index = it - snapshots.begin();
                    fprintf(csvf, "%d,%.4f,%.4f,%.4f,%.4f\n", index, it->xspeed, it->normalx, it->normaly, it->normalz);
                }
                fclose(csvf);
                snapshots = std::vector<sonicSnapshot>();
            }
        }
    }

    __declspec(dllexport) ModInfo SADXModInfo = { ModLoaderVer };
}   