
#include "pch.h"
#include "processutil.h"
#include "winuser.h"
#include <vector>
#include <chrono>
#include <string>
#include <memory>
#include <stdexcept>
#include <stdarg.h>
#include <errno.h>
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
int STATE_FLY = 15;
struct sonicSnapshot
{
    float xspeed;
    NJS_VECTOR surface_normal;
    
};
struct tailsSnapshot
{
    NJS_VECTOR speed;
    NJS_VECTOR speedoffset;
    float flight_time;
    double stickmagnitude;
    bool flybuttonheld;
};
std::vector<sonicSnapshot> snapshots = std::vector<sonicSnapshot>();
std::string data_fn = "\\sonicdata_lw_tap_aligned.csv";



//Lost world boulder hallway: 2197.35, -1323.05, 1353.87, angle 16384
//Mystic Ruins Airstrip: 1532.83, 190.79, 862.69, angle 9551    



extern "C" {
    __declspec(dllexport) void __cdecl Init(const char* path, const HelperFunctions& helperFunctions)
    {

        HelperFunctionsGlobal = helperFunctions; // Save the helper pointer for external use
        modpath = path;
        

    }
    double getStickMagnitude() {
        double stickx = Controllers[0].LeftStickX;
        double sticky = Controllers[0].LeftStickY;
        return sqrt(stickx * stickx + sticky * sticky);
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
        DisplayDebugStringFormatted(NJM_LOCATION(3, 2), "Speed: (%.4f, %.4f, %.4f)", CharObj2Ptrs[0]->Speed.x, CharObj2Ptrs[0]->Speed.y, CharObj2Ptrs[0]->Speed.z);
        
        DisplayDebugStringFormatted(NJM_LOCATION(3,3), "Surface Normal: (%.4f, %.4f, %.4f)", CharObj2Ptrs[0]->SurfaceNormal.x, CharObj2Ptrs[0]->SurfaceNormal.y, CharObj2Ptrs[0]->SurfaceNormal.z);
        
    }

    
    

    void snapshot_to_csv() {
        if (snapshots.size() == 0) {
            return;
        }
        
        float init_xspeed = snapshots[0].xspeed;
        std::string fn_fmt = modpath + "\\spindash_data_%.2f_%.2f_%d_frames.csv";
        float init_y_snorm = snapshots[4].surface_normal.y;
        char buf[500];
        snprintf(buf, sizeof(buf), fn_fmt.c_str(), init_xspeed, init_y_snorm, snapshots.size());
        std::string out_fn = buf;
        
        FILE* csvf;
        errno_t err = fopen_s(&csvf, out_fn.c_str(), "w");
        if (err != 0) {
            char ebuf[1024];
            fprintf(stderr, "cannot open file %s: %s\n", out_fn.c_str(), strerror_s(ebuf, sizeof(ebuf), err));
            return;
        }
        fprintf(csvf, "XSpeed,NormX,NormY,NormZ\n");
        for (sonicSnapshot s : snapshots) {
            fprintf(csvf, "%.8f,%.4f,%.4f,%.4f\n", s.xspeed, s.surface_normal.x, s.surface_normal.y, s.surface_normal.z);
        }
        fclose(csvf);
        snapshots = std::vector<sonicSnapshot>();
    }

    __declspec(dllexport) void __cdecl OnFrame()
    {
        UpdateOSD();
        if (EntityData1Ptrs[0] != nullptr && CharObj2Ptrs[0] != nullptr) {
            if (EntityData1Ptrs[0]->Action == STATE_BALL) {
                if (snapshots.size() == 0) {
                    printf("Collecting Data\n");
                }
                double stickmagnitude = getStickMagnitude();
                bool isJumpHeld = ((Controllers[0].HeldButtons & JumpButtons) != 0);
                struct sonicSnapshot snap = {CharObj2Ptrs[0]->Speed.x,CharObj2Ptrs[0]->SurfaceNormal};
                snapshots.push_back(snap);
            }
            else if (snapshots.size() > 0) {
                printf("Done Collecting Data\n");
                snapshot_to_csv();
            }
        }
    }

    __declspec(dllexport) ModInfo SADXModInfo = { ModLoaderVer };
}       