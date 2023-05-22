/***************************************************************************
                          guifuncs.c  -  description
                             -------------------
    copyright            : (C) 2003 by ShadowPrince
    email                : shadow@emulation64.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "../../lua/LuaConsole.h"

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "guifuncs.hpp"
#include "main_win.hpp"
#include "translation.hpp"
#include "rombrowser.h"
#include "../../winproject/resource.h"
#include "../vcr.hpp"
#include "Config.hpp"
#include <string>

char *get_currentpath()
{
    return AppPath;
}

std::string get_savespath()
{
    if (Config.is_default_saves_directory_used) {
        return std::string(AppPath) + "Save\\";
    }
    else return Config.saves_directory;
}

void display_loading_progress(int p)
{
   sprintf(TempMessage,"%d%%",p);
   //SendMessage( hStatus, SB_SETTEXT, 1, (LPARAM)TempMessage );  
   SendMessage( hStatusProgress, PBM_SETPOS, p+1, 0 );
}

int warn_recording() {

    int res = 0, warnings = res; // red eared slider
    int rec = VCR_isCapturing() || recording;
    //printf("\nRecording info:\nVCR_isRecording: %i\nVCR_isCapturing: %i", VCR_isRecording(), VCR_isCapturing());
    if (continue_vcr_on_restart_mode == 0) {
        std::string finalMessage = "";
        if (VCR_isRecording()) {
            finalMessage.append("Movie recording ");
            warnings++;
        }
        if (rec) {
            if (warnings > 0) { finalMessage.append(","); }
            finalMessage.append(" AVI capture ");
            warnings++;
        }
        if (enableTraceLog) {
            if(warnings > 0){ finalMessage.append(","); }
            finalMessage.append(" Trace logging ");
            warnings++;
        }
        finalMessage.append("is running. Are you sure you want to stop emulation?");
        //if(rec) SetWindowPos(mainHWND, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        if(warnings > 0) res = MessageBox(NULL, finalMessage.c_str(), "Stop emulation?", MB_YESNO | MB_TOPMOST | MB_ICONWARNING);
        //if (rec) SetWindowPos(mainHWND, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }

    return res == 7;
}
void internal_warnsavestate(const char* messageCaption, const char* message, bool modal) {
    if (!Config.is_savestate_warning_enabled) return;
    TranslateDefault(message, messageCaption, TempMessage);
    if (modal) 
        MessageBox(mainHWND, TempMessage, messageCaption, MB_ICONERROR);
    else 
        display_status(TempMessage);
}
void warn_savestate(const char* messageCaption, const char* message){internal_warnsavestate(messageCaption, message, false);}
void warn_savestate(const char* messageCaption, const char* message, bool modal){internal_warnsavestate(messageCaption, message, modal);}

//Shows text in the first sector of statusbar
void display_status(const char* status)
{
   SendMessage( hStatus, SB_SETTEXT, 0, (LPARAM)status ); 
}

void ShowInfo(const char* str, ...)
{
    printf("ShowInfo stub %s", str);
}
