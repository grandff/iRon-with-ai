/*
MIT License

Copyright (c) 2021-2022 L. E. Spalt

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d2d1.lib")
#pragma comment(lib,"dcomp.lib")
#pragma comment(lib,"dwrite.lib")


#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <windows.h>
#include "util.h"
#include "iracing.h"
#include "Config.h"
#include "OverlayCover.h"
#include "OverlayRelative.h"
#include "OverlayInputs.h"
#include "OverlayStandings.h"
#include "OverlayDebug.h"
#include "OverlayDDU.h"
#include "OverlaySpotter.h"
#include "OverlayRadar.h"
#include "OverlayIncident.h"
#include "OverlayTraffic.h"
#include "OverlayPitHelper.h"
#include "OverlayTireDash.h"
#include "TelemetryLogger.h"

#define printf printf_to_log_and_console

// ANSI Color Codes
#define ANSI_RESET   "\x1b[0m"
#define ANSI_BOLD    "\x1b[1m"
#define ANSI_RED     "\x1b[31m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_BLUE    "\x1b[34m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_CYAN    "\x1b[36m"
#define ANSI_WHITE   "\x1b[37m"
#define ANSI_B_RED   "\x1b[91m"
#define ANSI_B_WHITE "\x1b[97m"
#define ANSI_BG_RED  "\x1b[41m"

#include <shlobj.h>

// Function to enable ANSI colors in Windows Console
void EnableANSIColors() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

TelemetryLogger* g_telemetryLogger = nullptr;


enum class Hotkey
{
    UiEdit,
    Standings,
    DDU,
    Inputs,
    Relative,
    Cover,
    Spotter,
    Radar,
    Incident,
    TireDash
};

static void registerHotkeys()
{
    UnregisterHotKey( NULL, (int)Hotkey::UiEdit );
    UnregisterHotKey( NULL, (int)Hotkey::Standings );
    UnregisterHotKey( NULL, (int)Hotkey::DDU );
    UnregisterHotKey( NULL, (int)Hotkey::Inputs );
    UnregisterHotKey( NULL, (int)Hotkey::Relative );
    UnregisterHotKey( NULL, (int)Hotkey::Cover );
    UnregisterHotKey( NULL, (int)Hotkey::Spotter );
    UnregisterHotKey( NULL, (int)Hotkey::Radar );
    UnregisterHotKey( NULL, (int)Hotkey::Incident );
    UnregisterHotKey( NULL, (int)Hotkey::TireDash );

    UINT vk, mod;

    if( parseHotkey( g_cfg.getString("General","ui_edit_hotkey","alt-j"),&mod,&vk) )
        RegisterHotKey( NULL, (int)Hotkey::UiEdit, mod, vk );

    if( parseHotkey( g_cfg.getString("OverlayStandings","toggle_hotkey","ctrl-space"),&mod,&vk) )
        RegisterHotKey( NULL, (int)Hotkey::Standings, mod, vk );

    if( parseHotkey( g_cfg.getString("OverlayDDU","toggle_hotkey","ctrl-1"),&mod,&vk) )
        RegisterHotKey( NULL, (int)Hotkey::DDU, mod, vk );

    if( parseHotkey( g_cfg.getString("OverlayInputs","toggle_hotkey","ctrl-2"),&mod,&vk) )
        RegisterHotKey( NULL, (int)Hotkey::Inputs, mod, vk );

    if( parseHotkey( g_cfg.getString("OverlayRelative","toggle_hotkey","ctrl-3"),&mod,&vk) )
        RegisterHotKey( NULL, (int)Hotkey::Relative, mod, vk );

    if( parseHotkey( g_cfg.getString("OverlayCover","toggle_hotkey","ctrl-4"),&mod,&vk) )
        RegisterHotKey( NULL, (int)Hotkey::Cover, mod, vk );

    if( parseHotkey( g_cfg.getString("OverlaySpotter","toggle_hotkey","ctrl-5"),&mod,&vk) )
        RegisterHotKey( NULL, (int)Hotkey::Spotter, mod, vk );

    if( parseHotkey( g_cfg.getString("OverlayRadar","toggle_hotkey","ctrl-6"),&mod,&vk) )
        RegisterHotKey( NULL, (int)Hotkey::Radar, mod, vk );

    if( parseHotkey( g_cfg.getString("OverlayIncident","toggle_hotkey","ctrl-7"),&mod,&vk) )
        RegisterHotKey( NULL, (int)Hotkey::Incident, mod, vk );

    if( parseHotkey( g_cfg.getString("OverlayTireDash","toggle_hotkey","ctrl-8"),&mod,&vk) )
        RegisterHotKey( NULL, (int)Hotkey::TireDash, mod, vk );
}

static void handleConfigChange( std::vector<Overlay*> overlays, ConnectionStatus status, bool uiEdit )
{
    registerHotkeys();

    ir_handleConfigChange();

    for( Overlay* o : overlays )
    {
        o->enable( g_cfg.getBool(o->getName(),"enabled",true) && (
            uiEdit ||
            status == ConnectionStatus::DRIVING ||
            status == ConnectionStatus::CONNECTED && o->canEnableWhileNotDriving() ||
            status == ConnectionStatus::DISCONNECTED && o->canEnableWhileDisconnected()
            ));
        o->configChanged();
    }
}

static void giveFocusToIracing()
{
    HWND hwnd = FindWindow( "SimWinClass", NULL );
    if( hwnd )
        SetForegroundWindow( hwnd );
}

static void prepopulateConfig(const std::vector<Overlay*>& overlays)
{
    // General settings
    g_cfg.getString("General", "ui_edit_hotkey", "alt-j");
    g_cfg.getBool("General", "performance_mode_30hz", false);

    // Overlay hotkeys and states
    g_cfg.getString("OverlayStandings", "toggle_hotkey", "ctrl-space");
    g_cfg.getString("OverlayDDU", "toggle_hotkey", "ctrl-1");
    g_cfg.getString("OverlayInputs", "toggle_hotkey", "ctrl-2");
    g_cfg.getString("OverlayRelative", "toggle_hotkey", "ctrl-3");
    g_cfg.getString("OverlayCover", "toggle_hotkey", "ctrl-4");
    g_cfg.getString("OverlaySpotter", "toggle_hotkey", "ctrl-5");
    g_cfg.getString("OverlayRadar", "toggle_hotkey", "ctrl-6");
    g_cfg.getString("OverlayIncident", "toggle_hotkey", "ctrl-7");
    g_cfg.getString("OverlayTireDash", "toggle_hotkey", "ctrl-8");

    for (Overlay* o : overlays)
    {
        std::string name = o->getName();
        g_cfg.getBool(name, "enabled", true);
        
        // Populate standard default positions using hash
        const unsigned hash = MurmurHash2(name.c_str(), (int)name.length(), 0x1234);
        const int defaultX = (hash % 100) * 15;
        const int defaultY = (hash % 80) * 10;
        const float2 defaultSize = o->getDefaultSize();
        
        g_cfg.getInt(name, "window_pos_x", defaultX);
        g_cfg.getInt(name, "window_pos_y", defaultY);
        g_cfg.getInt(name, "window_size_x", (int)defaultSize.x);
        g_cfg.getInt(name, "window_size_y", (int)defaultSize.y);
    }
}

int main()
{
    // Enable ANSI colors for beautiful console output
    EnableANSIColors();

    // Bump priority up so we get time from the sim
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

    // Get dynamic folder in My Documents and set it up
    std::string ronDir = getRonDir();
    if (!ronDir.empty()) {
        std::string logPath = ronDir + "app.log";
        FILE* errFp = nullptr;
        freopen_s(&errFp, logPath.c_str(), "a", stderr);
    }
    logMsg("INFO", "iRon-Advanced initialized. Settings directory: %s", ronDir.c_str());

    // Load the config and watch it for changes
    g_cfg.setFilename(ronDir + "config.json");
    g_cfg.load();
    logMsg("INFO", "Config files loaded successfully from: %s", (ronDir + "config.json").c_str());
    g_cfg.watchForChanges();

    // Start telemetry logging dynamically
    g_telemetryLogger = new TelemetryLogger(ronDir + "telemetry_debug.log");
    g_telemetryLogger->start();
    logMsg("INFO", "Telemetry logger successfully started at: %s", (ronDir + "telemetry_debug.log").c_str());

    // Register global hotkeys
    registerHotkeys();

    printf("\n" ANSI_B_RED " ====================================================================== " ANSI_RESET "\n\n");
    
    printf(ANSI_BOLD ANSI_B_WHITE "  iRon-Advanced " ANSI_RESET ANSI_B_RED " | " ANSI_RESET "Lightweight Overlays for iRacing\n\n");
    
    printf(ANSI_YELLOW "  [ INFO ] " ANSI_RESET "Most overlays are active ONLY when iRacing is running\n");
    printf("           and your car is on the track.\n\n");

    printf(ANSI_BOLD "  [ HOTKEYS ]" ANSI_RESET "\n");
    printf("    " ANSI_B_RED ">" ANSI_RESET " Move and resize overlays : " ANSI_CYAN "[ %s ]\n" ANSI_RESET, g_cfg.getString("General","ui_edit_hotkey","").c_str() );
    printf("    " ANSI_B_RED ">" ANSI_RESET " Toggle standings         : " ANSI_CYAN "[ %s ]\n" ANSI_RESET, g_cfg.getString("OverlayStandings","toggle_hotkey","").c_str() );
    printf("    " ANSI_B_RED ">" ANSI_RESET " Toggle DDU               : " ANSI_CYAN "[ %s ]\n" ANSI_RESET, g_cfg.getString("OverlayDDU","toggle_hotkey","").c_str() );
    printf("    " ANSI_B_RED ">" ANSI_RESET " Toggle inputs            : " ANSI_CYAN "[ %s ]\n" ANSI_RESET, g_cfg.getString("OverlayInputs","toggle_hotkey","").c_str() );
    printf("    " ANSI_B_RED ">" ANSI_RESET " Toggle relative          : " ANSI_CYAN "[ %s ]\n" ANSI_RESET, g_cfg.getString("OverlayRelative","toggle_hotkey","").c_str() );
    printf("    " ANSI_B_RED ">" ANSI_RESET " Toggle cover             : " ANSI_CYAN "[ %s ]\n" ANSI_RESET, g_cfg.getString("OverlayCover","toggle_hotkey","").c_str() );
    printf("    " ANSI_B_RED ">" ANSI_RESET " Toggle spotter           : " ANSI_CYAN "[ %s ]\n" ANSI_RESET, g_cfg.getString("OverlaySpotter","toggle_hotkey","").c_str() );
    printf("    " ANSI_B_RED ">" ANSI_RESET " Toggle radar             : " ANSI_CYAN "[ %s ]\n" ANSI_RESET, g_cfg.getString("OverlayRadar","toggle_hotkey","").c_str() );
    printf("    " ANSI_B_RED ">" ANSI_RESET " Toggle incident warning  : " ANSI_CYAN "[ %s ]\n" ANSI_RESET, g_cfg.getString("OverlayIncident","toggle_hotkey","").c_str() );
    printf("    " ANSI_B_RED ">" ANSI_RESET " Toggle tire dash         : " ANSI_CYAN "[ %s ]\n" ANSI_RESET, g_cfg.getString("OverlayTireDash","toggle_hotkey","").c_str() );
    
    printf("\n" ANSI_BOLD "  [ CONFIG ]" ANSI_RESET "\n");
    printf("    Settings are auto-saved to " ANSI_YELLOW "Documents\\iRon_Advanced\\config.json" ANSI_RESET ".\n");
    printf("    You can edit it manually at any time to customize aesthetics.\n");
    
    printf("\n    Close this window to exit.\n\n");
    
    printf(ANSI_B_RED " ====================================================================== " ANSI_RESET "\n\n");

    // Create overlays
    std::vector<Overlay*> overlays;
    overlays.push_back( new OverlayCover() );
    overlays.push_back( new OverlayRelative() );
    overlays.push_back( new OverlayInputs() );
    overlays.push_back( new OverlayStandings() );
    overlays.push_back( new OverlayDDU() );
    overlays.push_back( new OverlaySpotter() );
    overlays.push_back( new OverlayRadar() );
    overlays.push_back( new OverlayIncident() );
    overlays.push_back( new OverlayTraffic() );
    overlays.push_back( new OverlayPitHelper() );
    overlays.push_back( new OverlayTireDash() );
#ifdef _DEBUG
    overlays.push_back( new OverlayDebug() );
#endif

    // Prepopulate config keys and save immediately to guarantee config.json exists with all defaults on first boot
    prepopulateConfig(overlays);
    g_cfg.save();

    ConnectionStatus  status   = ConnectionStatus::UNKNOWN;
    bool              uiEdit   = false;
    unsigned          frameCnt = 0;

    while( true )
    {
        ConnectionStatus prevStatus       = status;
        SessionType      prevSessionType  = ir_session.sessionType;

        // Refresh connection and session info
        status = ir_tick();
        if( status != prevStatus )
        {
            if( status == ConnectionStatus::DISCONNECTED ) {
                logMsg("INFO", "iRacing disconnected. Waiting for connection...");
            }
            else {
                logMsg("INFO", "iRacing connected! State: %s", ConnectionStatusStr[(int)status]);
            }

            // Enable user-selected overlays, passing uiEdit state
            handleConfigChange( overlays, status, uiEdit );
        }

        // Live-updating single console status line (refreshes every 200ms)
        static DWORD lastPrintTime = 0;
        DWORD now = GetTickCount();
        if (now - lastPrintTime > 200)
        {
            lastPrintTime = now;
            if (status == ConnectionStatus::DISCONNECTED) {
                printf("\r" ANSI_BOLD ANSI_B_RED "  [ WAITING ] " ANSI_RESET "Waiting for iRacing connection...                       ");
            } else {
                int qSize = g_telemetryLogger ? (int)g_telemetryLogger->getQueueSize() : 0;
                printf("\r" ANSI_BOLD ANSI_GREEN "  [ ACTIVE ] " ANSI_RESET "iRacing connected (%s) | Telemetry Sending (Queue: %d)   ", ConnectionStatusStr[(int)status], qSize);
            }
            fflush(stdout);
        }

        if( ir_session.sessionType != prevSessionType )
        {
            for( Overlay* o : overlays )
                o->sessionChanged();
        }

        dbg( "connection status: %s, session type: %s, session state: %d, pace mode: %d, on track: %d, flags: 0x%X", ConnectionStatusStr[(int)status], SessionTypeStr[(int)ir_session.sessionType], ir_SessionState.getInt(), ir_PaceMode.getInt(), (int)ir_IsOnTrackCar.getBool(), ir_SessionFlags.getInt() );

        // Push telemetry snapshot to logger queue if driving/connected
        if (status == ConnectionStatus::DRIVING || status == ConnectionStatus::CONNECTED)
        {
            TelemetryData tdata;
            tdata.timestamp = ir_SessionTime.getDouble();
            tdata.lap = ir_Lap.getInt();
            tdata.lapCompleted = ir_LapCompleted.getInt();
            tdata.lapDistPct = ir_LapDistPct.getDouble();
            tdata.speed = ir_Speed.getDouble();
            tdata.rpm = ir_RPM.getDouble();
            tdata.gear = ir_Gear.getInt();
            tdata.throttle = ir_Throttle.getDouble();
            tdata.brake = ir_Brake.getDouble();
            tdata.steering = ir_SteeringWheelAngle.getDouble();
            tdata.fuelLevelPct = ir_FuelLevelPct.getDouble();
            tdata.fuelLevel = ir_FuelLevel.getDouble();
            tdata.waterTemp = ir_WaterTemp.getDouble();
            tdata.oilTemp = ir_OilTemp.getDouble();
            tdata.oilPress = ir_OilPress.getDouble();
            tdata.voltage = ir_Voltage.getDouble();
            tdata.trackTemp = ir_TrackTempCrew.getDouble();
            tdata.airTemp = ir_AirTemp.getDouble();
            tdata.sessionState = ir_SessionState.getInt();
            tdata.sessionFlags = ir_SessionFlags.getInt();
            tdata.driverCarIdx = ir_session.driverCarIdx;
            tdata.isOnTrackCar = ir_IsOnTrackCar.getBool();
            tdata.isInGarage = ir_IsInGarage.getBool();
            if (g_telemetryLogger) {
                g_telemetryLogger->push(tdata);
            }
        }

        // Update/render overlays
        {
            if( !g_cfg.getBool("General", "performance_mode_30hz", false) )
            {
                // Update everything every frame, roughly every 16ms (~60Hz)
                for( Overlay* o : overlays )
                    o->update();
            }
            else
            {
                // To save perf, update half of the (enabled) overlays on even frames and the other half on odd, for ~30Hz overall
                int cnt = 0;
                for( Overlay* o : overlays )
                {
                    if( o->isEnabled() )
                        cnt++;

                    if( (cnt & 1) == (frameCnt & 1) )
                        o->update();
                }
            }
        }

        // Watch for config change signal
        if( g_cfg.hasChanged() )
        {
            g_cfg.load();
            handleConfigChange( overlays, status, uiEdit );
        }

        // Message pump
        MSG msg = {};
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // Handle hotkeys
            if( msg.message == WM_HOTKEY )
            {
                if( msg.wParam == (int)Hotkey::UiEdit )
                {
                    uiEdit = !uiEdit;
                    for( Overlay* o : overlays )
                        o->enableUiEdit( uiEdit );

                    // Update overlays immediately on UI edit state changes
                    handleConfigChange( overlays, status, uiEdit );

                    // When we're exiting edit mode, attempt to make iRacing the foreground window again for best perf
                    // without the user having to manually click into iRacing.
                    if( !uiEdit )
                        giveFocusToIracing();
                }
                else
                {
                    switch( msg.wParam )
                    {
                    case (int)Hotkey::Standings:
                        g_cfg.setBool( "OverlayStandings", "enabled", !g_cfg.getBool("OverlayStandings","enabled",true) );
                        break;
                    case (int)Hotkey::DDU:
                        g_cfg.setBool( "OverlayDDU", "enabled", !g_cfg.getBool("OverlayDDU","enabled",true) );
                        break;
                    case (int)Hotkey::Inputs:
                        g_cfg.setBool( "OverlayInputs", "enabled", !g_cfg.getBool("OverlayInputs","enabled",true) );
                        break;
                    case (int)Hotkey::Relative:
                        g_cfg.setBool( "OverlayRelative", "enabled", !g_cfg.getBool("OverlayRelative","enabled",true) );
                        break;
                    case (int)Hotkey::Cover:
                        g_cfg.setBool( "OverlayCover", "enabled", !g_cfg.getBool("OverlayCover","enabled",true) );
                        break;
                    case (int)Hotkey::Spotter:
                        g_cfg.setBool( "OverlaySpotter", "enabled", !g_cfg.getBool("OverlaySpotter","enabled",true) );
                        break;
                    case (int)Hotkey::Radar:
                        g_cfg.setBool( "OverlayRadar", "enabled", !g_cfg.getBool("OverlayRadar","enabled",true) );
                        break;
                    case (int)Hotkey::Incident:
                        g_cfg.setBool( "OverlayIncident", "enabled", !g_cfg.getBool("OverlayIncident","enabled",true) );
                        break;
                    case (int)Hotkey::TireDash:
                        g_cfg.setBool( "OverlayTireDash", "enabled", !g_cfg.getBool("OverlayTireDash","enabled",true) );
                        break;
                    }
                    
                    g_cfg.save();
                    handleConfigChange( overlays, status, uiEdit );
                }
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);            
        }

        frameCnt++;
    }

    for( Overlay* o : overlays )
        delete o;

    if (g_telemetryLogger) {
        g_telemetryLogger->stop();
        delete g_telemetryLogger;
        g_telemetryLogger = nullptr;
    }
}
