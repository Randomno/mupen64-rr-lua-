/***************************************************************************
						  configdialog.c  -  description
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
  /*
  #if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0500)
  #undef _WIN32_WINNT
  #define _WIN32_WINNT 0x0500
  #endif
  */

#include <shlobj.h>
#include <stdio.h>
#include "../lua/LuaConsole.h"
#include "main_win.h"
#include "../../winproject/resource.h"
#include "../plugin.h"
#include "rombrowser.h"
#include "../guifuncs.h"
#include "../md5.h"
#include "timers.h"
#include "translation.h"
#include "Config.h"
#include "../rom.h"
#include "inifunctions.h"
  // its a hpp header
#include "../main/win/wrapper/ReassociatingFileDialog.h"
#include "../main/helpers/string_helpers.h"


#include "configdialog.h"
#include <vcr.h>
// ughh msvc-only code once again
#pragma comment(lib,"comctl32.lib") 
#pragma comment(lib,"propsys.lib")
#pragma comment(lib,"shlwapi.lib") 

#ifdef _MSC_VER
#define snprintf	_snprintf
#define strcasecmp	_stricmp
#define strncasecmp	_strnicmp
#endif

ReassociatingFileDialog fdSelectRomFolder;
ReassociatingFileDialog fdSelectPluginsFolder;
ReassociatingFileDialog fdSelectSavesFolder;
ReassociatingFileDialog fdSelectScreenshotsFolder;

//HWND romInfoHWND;
static DWORD dwExitCode;
static DWORD Id;
BOOL stopScan = FALSE;
//HWND __stdcall CreateTrackbar(HWND hwndDlg, HMENU hMenu, UINT iMin, UINT iMax, UINT iSelMin, UINT iSelMax, UINT x, UINT y, UINT w); // winapi macro was very confusing

HWND hwndTrackMovieBackup;

extern int no_audio_delay;
extern int no_compiled_jump;

void SwitchMovieBackupModifier(HWND hwnd) {
	if (ReadCheckBoxValue(hwnd, IDC_MOVIEBACKUPS)) {
		EnableWindow(hwndTrackMovieBackup, TRUE);
	}
	else {
		EnableWindow(hwndTrackMovieBackup, FALSE);
	}
}

BOOL CALLBACK OtherOptionsProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	int index;
	switch (Message)
	{
	case WM_INITDIALOG: {
		WriteCheckBoxValue(hwnd, IDC_MOVIEBACKUPS, Config.is_movie_backup_enabled);
		WriteCheckBoxValue(hwnd, IDC_ALERTMOVIESERRORS, Config.is_rom_movie_compatibility_check_enabled);

		static const char* clockSpeedMultiplierNames[] = { "1 - Legacy Mupen Lag Emulation", "2 - 'Lagless'", "3", "4", "5", "6" };

		// Populate CPU Clock Speed Multiplier Dropdown Menu
		for (int i = 0; i < sizeof(clockSpeedMultiplierNames) / sizeof(clockSpeedMultiplierNames[0]); i++) {
			SendDlgItemMessage(hwnd, IDC_COMBO_CLOCK_SPD_MULT, CB_ADDSTRING, 0, (LPARAM)clockSpeedMultiplierNames[i]);
		}
		index = SendDlgItemMessage(hwnd, IDC_COMBO_CLOCK_SPD_MULT, CB_FINDSTRINGEXACT, -1, (LPARAM)clockSpeedMultiplierNames[Config.cpu_clock_speed_multiplier - 1]);
		SendDlgItemMessage(hwnd, IDC_COMBO_CLOCK_SPD_MULT, CB_SETCURSEL, index, 0);

		// todo
		//hwndTrackMovieBackup = CreateTrackbar(hwnd, (HMENU)ID_MOVIEBACKUP_TRACKBAR, 1, 3, Config.movieBackupsLevel, 3, 200, 55, 100);
		SwitchMovieBackupModifier(hwnd);

		switch (Config.synchronization_mode)
		{
		case VCR_SYNC_AUDIO_DUPL:
			CheckDlgButton(hwnd, IDC_AV_AUDIOSYNC, BST_CHECKED);
			break;
		case VCR_SYNC_VIDEO_SNDROP:
			CheckDlgButton(hwnd, IDC_AV_VIDEOSYNC, BST_CHECKED);
			break;
		case VCR_SYNC_NONE:
			CheckDlgButton(hwnd, IDC_AV_NOSYNC, BST_CHECKED);
			break;
		}


		return TRUE;
	}
	case WM_COMMAND: {
		char buf[50];
		// dame tu xorita mamacita
		switch (LOWORD(wParam))
		{
		case IDC_AV_AUDIOSYNC:
			if (!VCR_isCapturing())
				Config.synchronization_mode = VCR_SYNC_AUDIO_DUPL;
			break;
		case IDC_AV_VIDEOSYNC:
			if (!VCR_isCapturing())
				Config.synchronization_mode = VCR_SYNC_VIDEO_SNDROP;
			break;
		case IDC_AV_NOSYNC:
			if (!VCR_isCapturing())
				Config.synchronization_mode = VCR_SYNC_NONE;
			break;
		}
	case IDC_COMBO_CLOCK_SPD_MULT:
		ReadComboBoxValue(hwnd, IDC_COMBO_CLOCK_SPD_MULT, buf);
		Config.cpu_clock_speed_multiplier = atoi(&buf[0]);
		break;
	case IDC_MOVIEBACKUPS:
		SwitchMovieBackupModifier(hwnd);
	}

				   break;


	case WM_NOTIFY:
	{
		if (((NMHDR FAR*) lParam)->code == NM_RELEASEDCAPTURE) {
			// could potentially show value here, if necessary
		}

		if (((NMHDR FAR*) lParam)->code == PSN_APPLY) {
			Config.is_movie_backup_enabled = ReadCheckBoxValue(hwnd, IDC_MOVIEBACKUPS);
			Config.is_rom_movie_compatibility_check_enabled = ReadCheckBoxValue(hwnd, IDC_ALERTMOVIESERRORS);
			EnableToolbar();
			EnableStatusbar();
			FastRefreshBrowser();
			LoadConfigExternals();

		}

	}
	break;

	default:
		return FALSE;
	}
	return TRUE;
}


void WriteCheckBoxValue(HWND hwnd, int resourceID, int value)
{
	if (value) {
		SendMessage(GetDlgItem(hwnd, resourceID), BM_SETCHECK, BST_CHECKED, 0);
	}
}

int ReadCheckBoxValue(HWND hwnd, int resourceID)
{
	return SendDlgItemMessage(hwnd, resourceID, BM_GETCHECK, 0, 0) == BST_CHECKED ? TRUE : FALSE;
}

void WriteComboBoxValue(HWND hwnd, int ResourceID, char* PrimaryVal, char* DefaultVal)
{
	int index;
	index = SendDlgItemMessage(hwnd, ResourceID, CB_FINDSTRINGEXACT, 0, (LPARAM)PrimaryVal);
	if (index != CB_ERR) {
		SendDlgItemMessage(hwnd, ResourceID, CB_SETCURSEL, index, 0);
		return;
	}
	index = SendDlgItemMessage(hwnd, ResourceID, CB_FINDSTRINGEXACT, 0, (LPARAM)DefaultVal);
	if (index != CB_ERR) {
		SendDlgItemMessage(hwnd, ResourceID, CB_SETCURSEL, index, 0);
		return;
	}
	SendDlgItemMessage(hwnd, ResourceID, CB_SETCURSEL, 0, 0);

}

void ReadComboBoxValue(HWND hwnd, int ResourceID, char* ret)
{
	int index;
	index = SendDlgItemMessage(hwnd, ResourceID, CB_GETCURSEL, 0, 0);
	SendDlgItemMessage(hwnd, ResourceID, CB_GETLBTEXT, index, (LPARAM)ret);
}

void ChangeSettings(HWND hwndOwner) {
	PROPSHEETPAGE psp[6]{};
	PROPSHEETHEADER psh{};
	char ConfigStr[200], DirectoriesStr[200], titleStr[200], settingsStr[200];
	char AdvSettingsStr[200], HotkeysStr[200], OtherStr[200];
	psp[0].dwSize = sizeof(PROPSHEETPAGE);
	psp[0].dwFlags = PSP_USETITLE;
	psp[0].hInstance = app_hInstance;
	psp[0].pszTemplate = MAKEINTRESOURCE(IDD_MAIN);
	psp[0].pfnDlgProc = PluginsCfg;
	TranslateDefault("Plugins", "Plugins", ConfigStr);
	psp[0].pszTitle = ConfigStr;
	psp[0].lParam = 0;
	psp[0].pfnCallback = NULL;

	psp[1].dwSize = sizeof(PROPSHEETPAGE);
	psp[1].dwFlags = PSP_USETITLE;
	psp[1].hInstance = app_hInstance;
	psp[1].pszTemplate = MAKEINTRESOURCE(IDD_DIRECTORIES);
	psp[1].pfnDlgProc = DirectoriesCfg;
	TranslateDefault("Directories", "Directories", DirectoriesStr);
	psp[1].pszTitle = DirectoriesStr;
	psp[1].lParam = 0;
	psp[1].pfnCallback = NULL;

	psp[2].dwSize = sizeof(PROPSHEETPAGE);
	psp[2].dwFlags = PSP_USETITLE;
	psp[2].hInstance = app_hInstance;
	psp[2].pszTemplate = MAKEINTRESOURCE(IDD_MESSAGES);
	psp[2].pfnDlgProc = GeneralCfg;
	TranslateDefault("General", "General", settingsStr);
	psp[2].pszTitle = settingsStr;
	psp[2].lParam = 0;
	psp[2].pfnCallback = NULL;

	psp[3].dwSize = sizeof(PROPSHEETPAGE);
	psp[3].dwFlags = PSP_USETITLE;
	psp[3].hInstance = app_hInstance;
	psp[3].pszTemplate = MAKEINTRESOURCE(IDD_ADVANCED_OPTIONS);
	psp[3].pfnDlgProc = AdvancedSettingsProc;
	TranslateDefault("Advanced", "Advanced", AdvSettingsStr);
	psp[3].pszTitle = AdvSettingsStr;
	psp[3].lParam = 0;
	psp[3].pfnCallback = NULL;

	psp[4].dwSize = sizeof(PROPSHEETPAGE);
	psp[4].dwFlags = PSP_USETITLE;
	psp[4].hInstance = app_hInstance;
	psp[4].pszTemplate = MAKEINTRESOURCE(IDD_NEW_HOTKEY_DIALOG);
	psp[4].pfnDlgProc = HotkeysProc;
	TranslateDefault("Hotkeys", "Hotkeys", HotkeysStr);
	psp[4].pszTitle = HotkeysStr;
	psp[4].lParam = 0;
	psp[4].pfnCallback = NULL;

	psp[5].dwSize = sizeof(PROPSHEETPAGE);
	psp[5].dwFlags = PSP_USETITLE;
	psp[5].hInstance = app_hInstance;
	psp[5].pszTemplate = MAKEINTRESOURCE(IDD_OTHER_OPTIONS_DIALOG);
	psp[5].pfnDlgProc = OtherOptionsProc;
	TranslateDefault("Other", "Other", OtherStr);
	psp[5].pszTitle = OtherStr;
	psp[5].lParam = 0;
	psp[5].pfnCallback = NULL;

	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
	psh.hwndParent = hwndOwner;
	psh.hInstance = app_hInstance;
	TranslateDefault("Settings", "Settings", titleStr);
	psh.pszCaption = (LPTSTR)titleStr;
	psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
	psh.nStartPage = 0;
	psh.ppsp = (LPCPROPSHEETPAGE)&psp;
	psh.pfnCallback = NULL;

	if (PropertySheet(&psh)) save_config();
	return;
}

BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
		SendDlgItemMessage(hwnd, IDB_LOGO, STM_SETIMAGE, IMAGE_BITMAP,
			(LPARAM)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_LOGO),
				IMAGE_BITMAP, 0, 0, 0));

		//            MessageBox( hwnd, "", "", MB_OK );
		return TRUE;

	case WM_CLOSE:
		EndDialog(hwnd, IDOK);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwnd, IDOK);
			break;

		case IDC_WEBSITE:
			ShellExecute(0, 0, "http://mupen64.emulation64.com", 0, 0, SW_SHOW);
			break;
		case IDC_GITREPO:
			ShellExecute(0, 0, "https://github.com/mkdasher/mupen64-rr-lua-/", 0, 0, SW_SHOW);
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}



BOOL CALLBACK DirectoriesCfg(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	char Directory[MAX_PATH];
	LPITEMIDLIST pidl{};
	BROWSEINFO bi{};
	char RomBrowserDir[_MAX_PATH];
	HWND RomBrowserDirListBox;
	int count;
	switch (Message)
	{
	case WM_INITDIALOG:
		FillRomBrowserDirBox(hwnd);
		TranslateDirectoriesConfig(hwnd);
		if (Config.is_rom_browser_recursion_enabled) {
			SendMessage(GetDlgItem(hwnd, IDC_RECURSION), BM_SETCHECK, BST_CHECKED, 0);
		}

		if (Config.is_default_plugins_directory_used)
		{
			SendMessage(GetDlgItem(hwnd, IDC_DEFAULT_PLUGINS_CHECK), BM_SETCHECK, BST_CHECKED, 0);
			EnableWindow(GetDlgItem(hwnd, IDC_PLUGINS_DIR), FALSE);
			EnableWindow(GetDlgItem(hwnd, IDC_CHOOSE_PLUGINS_DIR), FALSE);
		}
		if (Config.is_default_saves_directory_used)
		{
			SendMessage(GetDlgItem(hwnd, IDC_DEFAULT_SAVES_CHECK), BM_SETCHECK, BST_CHECKED, 0);
			EnableWindow(GetDlgItem(hwnd, IDC_SAVES_DIR), FALSE);
			EnableWindow(GetDlgItem(hwnd, IDC_CHOOSE_SAVES_DIR), FALSE);
		}
		if (Config.is_default_screenshots_directory_used)
		{
			SendMessage(GetDlgItem(hwnd, IDC_DEFAULT_SCREENSHOTS_CHECK), BM_SETCHECK, BST_CHECKED, 0);
			EnableWindow(GetDlgItem(hwnd, IDC_SCREENSHOTS_DIR), FALSE);
			EnableWindow(GetDlgItem(hwnd, IDC_CHOOSE_SCREENSHOTS_DIR), FALSE);
		}

		SetDlgItemText(hwnd, IDC_PLUGINS_DIR, Config.plugins_directory.c_str());
		SetDlgItemText(hwnd, IDC_SAVES_DIR, Config.saves_directory.c_str());
		SetDlgItemText(hwnd, IDC_SCREENSHOTS_DIR, Config.screenshots_directory.c_str());

		break;
	case WM_NOTIFY:
		if (((NMHDR FAR*) lParam)->code == PSN_APPLY) {
			SaveRomBrowserDirs();
			int selected = SendDlgItemMessage(hwnd, IDC_DEFAULT_PLUGINS_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED ? TRUE : FALSE;
			GetDlgItemText(hwnd, IDC_PLUGINS_DIR, TempMessage, 200);
			if (strcasecmp(TempMessage, Config.plugins_directory.c_str()) != 0 || Config.is_default_plugins_directory_used != selected)
				//if plugin dir changed,search for plugins in new dir
			{
				Config.plugins_directory = std::string(TempMessage);
				Config.is_default_plugins_directory_used = selected;
				search_plugins();
			}
			else {
				Config.plugins_directory = std::string(TempMessage);
				Config.is_default_plugins_directory_used = selected;
			}

			selected = SendDlgItemMessage(hwnd, IDC_DEFAULT_SAVES_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED ? TRUE : FALSE;

			char str[MAX_PATH];
			GetDlgItemText(hwnd, IDC_SAVES_DIR, str, MAX_PATH);
			Config.saves_directory = std::string(str);
			Config.is_default_saves_directory_used = selected;

			selected = SendDlgItemMessage(hwnd, IDC_DEFAULT_SCREENSHOTS_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED ? TRUE : FALSE;
			GetDlgItemText(hwnd, IDC_SCREENSHOTS_DIR, str, MAX_PATH);
			Config.screenshots_directory = std::string(str);
			Config.is_default_screenshots_directory_used = selected;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_RECURSION:
			Config.is_rom_browser_recursion_enabled = SendDlgItemMessage(hwnd, IDC_RECURSION, BM_GETCHECK, 0, 0) == BST_CHECKED ? TRUE : FALSE;
			break;
		case IDC_ADD_BROWSER_DIR: {
			if (fdSelectRomFolder.ShowFolderDialog(Directory, sizeof(Directory) / sizeof(char), hwnd))
			{
				int len = strlen(Directory);
				if (addDirectoryToLinkedList(Directory)) {
					SendDlgItemMessage(hwnd, IDC_ROMBROWSER_DIR_LIST, LB_ADDSTRING, 0, (LPARAM)Directory);
					AddDirToList(Directory, TRUE);
				}
			}
			break;
		}
		case IDC_REMOVE_BROWSER_DIR:
		{
			RomBrowserDirListBox = GetDlgItem(hwnd, IDC_ROMBROWSER_DIR_LIST);

			int selected = SendMessage(RomBrowserDirListBox, LB_GETCURSEL, 0, 0);
			if (selected != -1)
			{
				SendMessage(RomBrowserDirListBox, LB_GETTEXT, selected, (LPARAM)RomBrowserDir);
				removeDirectoryFromLinkedList(RomBrowserDir);
				SendMessage(RomBrowserDirListBox, LB_DELETESTRING, selected, 0);
				RefreshRomBrowser();
			}


			break; }

		case IDC_REMOVE_BROWSER_ALL:
			SendDlgItemMessage(hwnd, IDC_ROMBROWSER_DIR_LIST, LB_RESETCONTENT, 0, 0);
			freeRomDirList();
			RefreshRomBrowser();
			break;

		case IDC_DEFAULT_PLUGINS_CHECK:
		{
			int selected = SendMessage(GetDlgItem(hwnd, IDC_DEFAULT_PLUGINS_CHECK), BM_GETCHECK, 0, 0);
			EnableWindow(GetDlgItem(hwnd, IDC_PLUGINS_DIR), !selected);
			EnableWindow(GetDlgItem(hwnd, IDC_CHOOSE_PLUGINS_DIR), !selected);
		}
		break;
		case IDC_PLUGIN_DIRECTORY_HELP:
		{
			MessageBox(hwnd, "Changing the plugin directory may introduce bugs to some plugins.", "Info", MB_ICONINFORMATION | MB_OK);
		}
		break;
		case IDC_CHOOSE_PLUGINS_DIR:
		{
			// Lol why is vs indenting this one block down
			//folderDiag(Directory, sizeof(Directory)/sizeof(char), "");
			if (fdSelectPluginsFolder.ShowFolderDialog(Directory, sizeof(Directory) / sizeof(char), hwnd))
			{
				int len = strlen(Directory);
				if (Directory[len - 1] != '\\')
					strcat(Directory, "\\");
				SetDlgItemText(hwnd, IDC_PLUGINS_DIR, Directory);
			}


		}


		break;
		case IDC_DEFAULT_SAVES_CHECK:
		{
			int selected = SendMessage(GetDlgItem(hwnd, IDC_DEFAULT_SAVES_CHECK), BM_GETCHECK, 0, 0);
			if (!selected)
			{
				EnableWindow(GetDlgItem(hwnd, IDC_SAVES_DIR), TRUE);
				EnableWindow(GetDlgItem(hwnd, IDC_CHOOSE_SAVES_DIR), TRUE);
			}
			else
			{
				EnableWindow(GetDlgItem(hwnd, IDC_SAVES_DIR), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_CHOOSE_SAVES_DIR), FALSE);
			}
		}
		break;
		case IDC_CHOOSE_SAVES_DIR:
		{
			if (fdSelectSavesFolder.ShowFolderDialog(Directory, sizeof(Directory) / sizeof(char), hwnd))
			{
				if (Directory[strlen(Directory) - 1] != '\\')
				{
					strcat(Directory, "\\");
					SetDlgItemText(hwnd, IDC_SAVES_DIR, Directory);
				}
			}
		}
		break;
		case IDC_DEFAULT_SCREENSHOTS_CHECK:
		{
			int selected = SendMessage(GetDlgItem(hwnd, IDC_DEFAULT_SCREENSHOTS_CHECK), BM_GETCHECK, 0, 0);
			if (!selected)
			{
				EnableWindow(GetDlgItem(hwnd, IDC_SCREENSHOTS_DIR), TRUE);
				EnableWindow(GetDlgItem(hwnd, IDC_CHOOSE_SCREENSHOTS_DIR), TRUE);
			}
			else
			{
				EnableWindow(GetDlgItem(hwnd, IDC_SCREENSHOTS_DIR), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_CHOOSE_SCREENSHOTS_DIR), FALSE);
			}
		}
		break;
		case IDC_CHOOSE_SCREENSHOTS_DIR:
		{
			if (fdSelectScreenshotsFolder.ShowFolderDialog(Directory, sizeof(Directory) / sizeof(char), hwnd))
			{
				int len = strlen(Directory);
				if (Directory[len - 1] != '\\')
					strcat(Directory, "\\");
				SetDlgItemText(hwnd, IDC_SCREENSHOTS_DIR, Directory);
			}
		}
		break;
		}
		break;

	}
	return FALSE;
}

BOOL CALLBACK PluginsCfg(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	char path_buffer[_MAX_PATH];
	int index;

	switch (Message)
	{
	case WM_CLOSE:
		EndDialog(hwnd, IDOK);
		break;
	case WM_INITDIALOG:
		rewind_plugin();
		while (get_plugin_type() != -1) {
			switch (get_plugin_type())
			{
			case PLUGIN_TYPE_GFX:
				SendDlgItemMessage(hwnd, IDC_COMBO_GFX, CB_ADDSTRING, 0, (LPARAM)next_plugin());
				break;
			case PLUGIN_TYPE_CONTROLLER:
				SendDlgItemMessage(hwnd, IDC_COMBO_INPUT, CB_ADDSTRING, 0, (LPARAM)next_plugin());
				break;
			case PLUGIN_TYPE_AUDIO:
				SendDlgItemMessage(hwnd, IDC_COMBO_SOUND, CB_ADDSTRING, 0, (LPARAM)next_plugin());
				break;
			case PLUGIN_TYPE_RSP:
				SendDlgItemMessage(hwnd, IDC_COMBO_RSP, CB_ADDSTRING, 0, (LPARAM)next_plugin());
				break;
			default:
				next_plugin();
			}
		}
		// Set gfx plugin
		index = SendDlgItemMessage(hwnd, IDC_COMBO_GFX, CB_FINDSTRINGEXACT, 0, (LPARAM)gfx_name);
		if (index != CB_ERR) {
			SendDlgItemMessage(hwnd, IDC_COMBO_GFX, CB_SETCURSEL, index, 0);
		}
		else {
			SendDlgItemMessage(hwnd, IDC_COMBO_GFX, CB_SETCURSEL, 0, 0);
			SendDlgItemMessage(hwnd, IDC_COMBO_GFX, CB_GETLBTEXT, 0, (LPARAM)gfx_name);
		}
		// Set input plugin
		index = SendDlgItemMessage(hwnd, IDC_COMBO_INPUT, CB_FINDSTRINGEXACT, 0, (LPARAM)input_name);
		if (index != CB_ERR) {
			SendDlgItemMessage(hwnd, IDC_COMBO_INPUT, CB_SETCURSEL, index, 0);
		}
		else {
			SendDlgItemMessage(hwnd, IDC_COMBO_INPUT, CB_SETCURSEL, 0, 0);
			SendDlgItemMessage(hwnd, IDC_COMBO_INPUT, CB_GETLBTEXT, 0, (LPARAM)input_name);
		}
		// Set sound plugin
		index = SendDlgItemMessage(hwnd, IDC_COMBO_SOUND, CB_FINDSTRINGEXACT, 0, (LPARAM)sound_name);
		if (index != CB_ERR) {
			SendDlgItemMessage(hwnd, IDC_COMBO_SOUND, CB_SETCURSEL, index, 0);
		}
		else {
			SendDlgItemMessage(hwnd, IDC_COMBO_SOUND, CB_SETCURSEL, 0, 0);
			SendDlgItemMessage(hwnd, IDC_COMBO_SOUND, CB_GETLBTEXT, 0, (LPARAM)sound_name);
		}
		// Set RSP plugin
		index = SendDlgItemMessage(hwnd, IDC_COMBO_RSP, CB_FINDSTRINGEXACT, 0, (LPARAM)rsp_name);
		if (index != CB_ERR) {
			SendDlgItemMessage(hwnd, IDC_COMBO_RSP, CB_SETCURSEL, index, 0);
		}
		else {
			SendDlgItemMessage(hwnd, IDC_COMBO_RSP, CB_SETCURSEL, 0, 0);
			SendDlgItemMessage(hwnd, IDC_COMBO_RSP, CB_GETLBTEXT, 0, (LPARAM)rsp_name);
		}


		TranslateConfigDialog(hwnd);
		if (emu_launched) {
			EnableWindow(GetDlgItem(hwnd, IDC_COMBO_GFX), FALSE);
			EnableWindow(GetDlgItem(hwnd, IDC_COMBO_INPUT), FALSE);
			EnableWindow(GetDlgItem(hwnd, IDC_COMBO_SOUND), FALSE);
			EnableWindow(GetDlgItem(hwnd, IDC_COMBO_RSP), FALSE);
		}

		//Show the images
		SendDlgItemMessage(hwnd, IDB_DISPLAY, STM_SETIMAGE, IMAGE_BITMAP,
			(LPARAM)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_DISPLAY),
				IMAGE_BITMAP, 0, 0, 0));
		SendDlgItemMessage(hwnd, IDB_CONTROL, STM_SETIMAGE, IMAGE_BITMAP,
			(LPARAM)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_CONTROL),
				IMAGE_BITMAP, 0, 0, 0));
		SendDlgItemMessage(hwnd, IDB_SOUND, STM_SETIMAGE, IMAGE_BITMAP,
			(LPARAM)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_SOUND),
				IMAGE_BITMAP, 0, 0, 0));
		SendDlgItemMessage(hwnd, IDB_RSP, STM_SETIMAGE, IMAGE_BITMAP,
			(LPARAM)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_RSP),
				IMAGE_BITMAP, 0, 0, 0));
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDGFXCONFIG:
			hwnd_plug = hwnd;
			ReadComboBoxValue(hwnd, IDC_COMBO_GFX, path_buffer);
			exec_config(path_buffer);
			break;
		case IDGFXTEST:
			hwnd_plug = hwnd;
			ReadComboBoxValue(hwnd, IDC_COMBO_GFX, path_buffer);
			exec_test(path_buffer);
			break;
		case IDGFXABOUT:
			hwnd_plug = hwnd;
			ReadComboBoxValue(hwnd, IDC_COMBO_GFX, path_buffer);
			exec_about(path_buffer);
			break;
		case IDINPUTCONFIG:
			hwnd_plug = hwnd;
			ReadComboBoxValue(hwnd, IDC_COMBO_INPUT, path_buffer);
			exec_config(path_buffer);
			break;
		case IDINPUTTEST:
			hwnd_plug = hwnd;
			ReadComboBoxValue(hwnd, IDC_COMBO_INPUT, path_buffer);
			exec_test(path_buffer);
			break;
		case IDINPUTABOUT:
			hwnd_plug = hwnd;
			ReadComboBoxValue(hwnd, IDC_COMBO_INPUT, path_buffer);
			exec_about(path_buffer);
			break;
		case IDSOUNDCONFIG:
			hwnd_plug = hwnd;
			ReadComboBoxValue(hwnd, IDC_COMBO_SOUND, path_buffer);
			exec_config(path_buffer);
			break;
		case IDSOUNDTEST:
			hwnd_plug = hwnd;
			ReadComboBoxValue(hwnd, IDC_COMBO_SOUND, path_buffer);
			exec_test(path_buffer);
			break;
		case IDSOUNDABOUT:
			hwnd_plug = hwnd;
			ReadComboBoxValue(hwnd, IDC_COMBO_SOUND, path_buffer);
			exec_about(path_buffer);
			break;
		case IDRSPCONFIG:
			hwnd_plug = hwnd;
			ReadComboBoxValue(hwnd, IDC_COMBO_RSP, path_buffer);
			exec_config(path_buffer);
			break;
		case IDRSPTEST:
			hwnd_plug = hwnd;
			ReadComboBoxValue(hwnd, IDC_COMBO_RSP, path_buffer);
			exec_test(path_buffer);
			break;
		case IDRSPABOUT:
			hwnd_plug = hwnd;
			ReadComboBoxValue(hwnd, IDC_COMBO_RSP, path_buffer);
			exec_about(path_buffer);
			break;
		}
		break;
	case WM_NOTIFY:
		if (((NMHDR FAR*) lParam)->code == PSN_APPLY) {

			/*ReadComboBoxValue( hwnd, IDC_COMBO_GFX, gfx_name);
			WriteCfgString("Plugins","Graphics",gfx_name);

			ReadComboBoxValue( hwnd, IDC_COMBO_INPUT, input_name);
			WriteCfgString("Plugins","Input",input_name);

			ReadComboBoxValue( hwnd, IDC_COMBO_SOUND, sound_name);
			WriteCfgString("Plugins","Sound",sound_name);

			ReadComboBoxValue( hwnd, IDC_COMBO_RSP, rsp_name);
			WriteCfgString("Plugins","RSP",rsp_name);*/
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}


void SwitchModifier(HWND hwnd) {
	if (ReadCheckBoxValue(hwnd, IDC_SPEEDMODIFIER)) {
		EnableWindow(GetDlgItem(hwnd, IDC_FPSTRACKBAR), TRUE);
	}
	else {
		EnableWindow(GetDlgItem(hwnd, IDC_FPSTRACKBAR), FALSE);
	}
}

void SwitchLimitFPS(HWND hwnd) {
	if (ReadCheckBoxValue(hwnd, IDC_LIMITFPS)) {
		EnableWindow(GetDlgItem(hwnd, IDC_SPEEDMODIFIER), TRUE);
		SwitchModifier(hwnd);
	}
	else {
		EnableWindow(GetDlgItem(hwnd, IDC_SPEEDMODIFIER), FALSE);
		EnableWindow(GetDlgItem(hwnd, IDC_FPSTRACKBAR), FALSE);
	}
}

void FillModifierValue(HWND hwnd, int value) {
	char temp[10];
	sprintf(temp, "%d%%", value);
	SetDlgItemText(hwnd, IDC_SPEEDMODIFIER_VALUE, temp);
}


BOOL CALLBACK GeneralCfg(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{

	switch (Message) {
	case WM_INITDIALOG:
		WriteCheckBoxValue(hwnd, IDC_SHOWFPS, Config.show_fps);
		WriteCheckBoxValue(hwnd, IDC_SHOWVIS, Config.show_vis_per_second);
		WriteCheckBoxValue(hwnd, IDC_MANAGEBADROM, Config.prevent_suspicious_rom_loading);
		WriteCheckBoxValue(hwnd, IDC_ALERTSAVESTATEWARNINGS, Config.is_savestate_warning_enabled);
		WriteCheckBoxValue(hwnd, IDC_LIMITFPS, Config.is_fps_limited);
		WriteCheckBoxValue(hwnd, IDC_0INDEX, Config.is_frame_count_visual_zero_index);
		SendMessage(GetDlgItem(hwnd, IDC_SPEED_MODIFIER_SLIDER), TBM_SETRANGEMIN, TRUE, 0);
		SendMessage(GetDlgItem(hwnd, IDC_SPEED_MODIFIER_SLIDER), TBM_SETRANGEMAX, TRUE, 200);
		SendMessage(GetDlgItem(hwnd, IDC_SPEED_MODIFIER_SLIDER), TBM_SETPOS, TRUE, Config.fps_modifier);
		SetDlgItemInt(hwnd, IDC_SKIPFREQ, Config.frame_skip_frequency, 0);
		WriteCheckBoxValue(hwnd, IDC_ALLOW_ARBITRARY_SAVESTATE_LOADING, Config.is_state_independent_state_loading_allowed);


		switch (Config.core_type)
		{
		case 0:
			CheckDlgButton(hwnd, IDC_INTERP, BST_CHECKED);
			break;
		case 1:
			CheckDlgButton(hwnd, IDC_RECOMP, BST_CHECKED);
			break;
		case 2:
			CheckDlgButton(hwnd, IDC_PURE_INTERP, BST_CHECKED);
			break;
		}

		if (emu_launched) {
			EnableWindow(GetDlgItem(hwnd, IDC_INTERP), FALSE);
			EnableWindow(GetDlgItem(hwnd, IDC_RECOMP), FALSE);
			EnableWindow(GetDlgItem(hwnd, IDC_PURE_INTERP), FALSE);
		}

		SwitchLimitFPS(hwnd);
		FillModifierValue(hwnd, Config.fps_modifier);
		TranslateGeneralDialog(hwnd);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_SKIPFREQUENCY_HELP:
			MessageBox(hwnd, "0 = Skip all frames, 1 = Show all frames, n = show every nth frame", "Info", MB_OK | MB_ICONINFORMATION);
			break;
		case IDC_INTERP:
			if (!emu_launched) {
				Config.core_type = 0;
			}
			break;
		case IDC_RECOMP:
			if (!emu_launched) {
				Config.core_type = 1;
			}
			break;
		case IDC_PURE_INTERP:
			if (!emu_launched) {
				Config.core_type = 2;
			}
			break;
		case IDC_LIMITFPS:
			SwitchLimitFPS(hwnd);
			break;
		case  IDC_SPEEDMODIFIER:
			SwitchModifier(hwnd);
			break;
		case IDC_SPEED_MODIFIER_RESET:
			SendMessage(GetDlgItem(hwnd, IDC_SPEED_MODIFIER_SLIDER), TBM_SETPOS, TRUE, default_config.fps_modifier);
			break;

		}
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
			Config.show_fps = ReadCheckBoxValue(hwnd, IDC_SHOWFPS);
			Config.show_vis_per_second = ReadCheckBoxValue(hwnd, IDC_SHOWVIS);
			Config.prevent_suspicious_rom_loading = ReadCheckBoxValue(hwnd, IDC_MANAGEBADROM);
			Config.is_savestate_warning_enabled = ReadCheckBoxValue(hwnd, IDC_ALERTSAVESTATEWARNINGS);
			Config.is_fps_limited = ReadCheckBoxValue(hwnd, IDC_LIMITFPS);
			Config.fps_modifier = SendMessage(GetDlgItem(hwnd, IDC_SPEED_MODIFIER_SLIDER), TBM_GETPOS, 0, 0);
			Config.frame_skip_frequency = GetDlgItemInt(hwnd, IDC_SKIPFREQ, 0, 0);
			Config.is_frame_count_visual_zero_index = ReadCheckBoxValue(hwnd, IDC_0INDEX);
			Config.is_state_independent_state_loading_allowed = ReadCheckBoxValue(hwnd, IDC_ALLOW_ARBITRARY_SAVESTATE_LOADING);

			if (emu_launched) SetStatusMode(2);
			else SetStatusMode(0);
			InitTimer();

			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}


BOOL CALLBACK LangInfoProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch (Message) {
	case WM_INITDIALOG:
		TranslateLangInfoDialog(hwnd);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			EndDialog(hwnd, IDOK);
			break;
		}
	default:
		return FALSE;
	}
}

BOOL CALLBACK AdvancedSettingsProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{

	switch (Message) {

	case WM_INITDIALOG:
		WriteCheckBoxValue(hwnd, IDC_PAUSENOTACTIVE, Config.is_unfocused_pause_enabled);
		WriteCheckBoxValue(hwnd, IDC_GUI_TOOLBAR, Config.is_toolbar_enabled);
		WriteCheckBoxValue(hwnd, IDC_GUI_STATUSBAR, Config.is_statusbar_enabled);
		WriteCheckBoxValue(hwnd, IDC_ROUNDTOZERO, Config.is_round_towards_zero_enabled);
		WriteCheckBoxValue(hwnd, IDC_EMULATEFLOATCRASHES, Config.is_float_exception_propagation_enabled);
		WriteCheckBoxValue(hwnd, IDC_CLUADOUBLEBUFFER, Config.is_lua_double_buffered);
		WriteCheckBoxValue(hwnd, IDC_NO_AUDIO_DELAY, no_audio_delay);
		WriteCheckBoxValue(hwnd, IDC_NO_COMPILED_JUMP, no_compiled_jump);

		WriteCheckBoxValue(hwnd, IDC_COLUMN_GOODNAME, Config.is_good_name_column_enabled);
		WriteCheckBoxValue(hwnd, IDC_COLUMN_INTERNALNAME, Config.is_internal_name_column_enabled);
		WriteCheckBoxValue(hwnd, IDC_COLUMN_COUNTRY, Config.is_country_column_enabled);
		WriteCheckBoxValue(hwnd, IDC_COLUMN_SIZE, Config.is_size_column_enabled);
		WriteCheckBoxValue(hwnd, IDC_COLUMN_COMMENTS, Config.is_comments_column_enabled);
		WriteCheckBoxValue(hwnd, IDC_COLUMN_FILENAME, Config.is_filename_column_enabled);
		WriteCheckBoxValue(hwnd, IDC_COLUMN_MD5, Config.is_md5_column_enabled);

		WriteCheckBoxValue(hwnd, IDC_NORESET, !Config.is_reset_recording_disabled);

		WriteCheckBoxValue(hwnd, IDC_FORCEINTERNAL, Config.is_internal_capture_forced);

		TranslateAdvancedDialog(hwnd);
		return TRUE;



	case WM_NOTIFY:
		if (((NMHDR FAR*) lParam)->code == PSN_APPLY) {
			Config.is_unfocused_pause_enabled = ReadCheckBoxValue(hwnd, IDC_PAUSENOTACTIVE);
			Config.is_toolbar_enabled = ReadCheckBoxValue(hwnd, IDC_GUI_TOOLBAR);
			Config.is_statusbar_enabled = ReadCheckBoxValue(hwnd, IDC_GUI_STATUSBAR);
			Config.is_round_towards_zero_enabled = ReadCheckBoxValue(hwnd, IDC_ROUNDTOZERO);
			Config.is_float_exception_propagation_enabled = ReadCheckBoxValue(hwnd, IDC_EMULATEFLOATCRASHES);

			Config.is_lua_double_buffered = ReadCheckBoxValue(hwnd, IDC_CLUADOUBLEBUFFER);
			no_audio_delay = ReadCheckBoxValue(hwnd, IDC_NO_AUDIO_DELAY);
			no_compiled_jump = ReadCheckBoxValue(hwnd, IDC_NO_COMPILED_JUMP);

			Config.is_good_name_column_enabled = ReadCheckBoxValue(hwnd, IDC_COLUMN_GOODNAME);
			Config.is_internal_name_column_enabled = ReadCheckBoxValue(hwnd, IDC_COLUMN_INTERNALNAME);
			Config.is_country_column_enabled = ReadCheckBoxValue(hwnd, IDC_COLUMN_COUNTRY);
			Config.is_size_column_enabled = ReadCheckBoxValue(hwnd, IDC_COLUMN_SIZE);
			Config.is_comments_column_enabled = ReadCheckBoxValue(hwnd, IDC_COLUMN_COMMENTS);
			Config.is_filename_column_enabled = ReadCheckBoxValue(hwnd, IDC_COLUMN_FILENAME);
			Config.is_md5_column_enabled = ReadCheckBoxValue(hwnd, IDC_COLUMN_MD5);

			Config.is_reset_recording_disabled = !ReadCheckBoxValue(hwnd, IDC_NORESET);
			Config.is_internal_capture_forced = ReadCheckBoxValue(hwnd, IDC_FORCEINTERNAL);

			EnableToolbar();
			EnableStatusbar();
			FastRefreshBrowser();
			LoadConfigExternals();
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}




static void KillMessages()
{
	MSG Msg;
	int i = 0;
	while (GetMessage(&Msg, NULL, 0, 0) > 0 && i < 20)
	{
		//		TranslateMessage(&Msg);
		//		DispatchMessage(&Msg);
		i++;
	}
}

static void GetUserHotkey(t_hotkey* hotkeys)
{
	int i, j;
	int lc = 0, ls = 0, la = 0;
	for (i = 0; i < 500; i++)
	{
		SleepEx(10, TRUE);
		for (j = 8; j < 254; j++)
		{
			if (j == VK_LCONTROL || j == VK_RCONTROL || j == VK_LMENU || j == VK_RMENU || j == VK_LSHIFT || j == VK_RSHIFT)
				continue;

			if (GetAsyncKeyState(j) & 0x8000)
			{
				// HACK to avoid exiting all the way out of the dialog on pressing escape to clear a hotkeys
				// or continually re-activating the button on trying to assign space as a hotkeys
				if (j == VK_ESCAPE || j == VK_SPACE)
					KillMessages();

				if (j == VK_CONTROL)
				{
					lc = 1;
					continue;
				}
				else if (j == VK_SHIFT)
				{
					ls = 1;
					continue;
				}
				else if (j == VK_MENU)
				{
					la = 1;
					continue;
				}
				else if (j != VK_ESCAPE)
				{
					hotkeys->key = j;
					hotkeys->shift = GetAsyncKeyState(VK_SHIFT) ? 1 : 0;
					hotkeys->ctrl = GetAsyncKeyState(VK_CONTROL) ? 1 : 0;
					hotkeys->alt = GetAsyncKeyState(VK_MENU) ? 1 : 0;
					return;
				}
				memset(hotkeys, 0, sizeof(t_hotkey)); // clear key on escape
				return;
			}
			else
			{
				if (j == VK_CONTROL && lc)
				{
					hotkeys->key = 0;
					hotkeys->shift = 0;
					hotkeys->ctrl = 1;
					hotkeys->alt = 0;
					return;
				}
				else if (j == VK_SHIFT && ls)
				{
					hotkeys->key = 0;
					hotkeys->shift = 1;
					hotkeys->ctrl = 0;
					hotkeys->alt = 0;
					return;
				}
				else if (j == VK_MENU && la)
				{
					hotkeys->key = 0;
					hotkeys->shift = 0;
					hotkeys->ctrl = 0;
					hotkeys->alt = 1;
					return;
				}
			}
		}
	}
}

int get_hotkey_array_index_from_selected_identifier(std::string selected_identifier) {
	int indexInHotkeyArray = -1;
	for (size_t i = 0; i < hotkeys.size(); i++)
	{
		if (selected_identifier == hotkeys[i]->identifier)
		{
			indexInHotkeyArray = i;
			break;
		}
	}
	return indexInHotkeyArray;
}

void UpdateSelectedHotkeyTextBox(HWND dialogHwnd) {
	HWND listHwnd = GetDlgItem(dialogHwnd, IDC_HOTKEY_LIST);
	HWND selectedHotkeyEditHwnd = GetDlgItem(dialogHwnd, IDC_SELECTED_HOTKEY_TEXT);


	char selected_identifier[MAX_PATH];
	SendMessage(listHwnd, LB_GETTEXT, SendMessage(listHwnd, LB_GETCURSEL, 0, 0), (LPARAM)selected_identifier);

	int indexInHotkeyArray = get_hotkey_array_index_from_selected_identifier(std::string(selected_identifier));

	if (indexInHotkeyArray >= 0 && indexInHotkeyArray < hotkeys.size())
	{
		char hotkeyText[MAX_PATH];

		hotkey_to_string(hotkeys[indexInHotkeyArray], hotkeyText);

		SetWindowText(selectedHotkeyEditHwnd, hotkeyText);
	}
}



void build_hotkey_list(HWND hwnd, char* searchQuery) {
	SendMessage(hwnd, LB_RESETCONTENT, 0, 0);

	for (size_t i = 0; i < hotkeys.size(); i++)
	{
		if (strlen(searchQuery) != 0)
		{
			char hotkeyString[MAX_PATH];
			hotkey_to_string(hotkeys[i], hotkeyString);

			if (!stristr(hotkeyString, searchQuery) && !stristr(hotkeys[i]->identifier.c_str(), searchQuery))
			{
				continue;
			}
		}
		SendMessage(hwnd, LB_ADDSTRING, 0,
			(LPARAM)hotkeys[i]->identifier.c_str());
	}
}

BOOL CALLBACK HotkeysProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message) {
	case WM_INITDIALOG:
	{

		SetDlgItemText(hwnd, IDC_HOTKEY_SEARCH, "");

		return TRUE;
	}
	case WM_COMMAND: {
		int id = LOWORD(wParam);
		int event = HIWORD(wParam);

		if (id == IDC_HOTKEY_LIST && event == LBN_SELCHANGE)
		{
			UpdateSelectedHotkeyTextBox(hwnd);
		}

		if (id == IDC_HOTKEY_ASSIGN_SELECTED)
		{
			HWND listHwnd = GetDlgItem(hwnd, IDC_HOTKEY_LIST);
			char selected_identifier[MAX_PATH];
			SendMessage(listHwnd, LB_GETTEXT, SendMessage(listHwnd, LB_GETCURSEL, 0, 0), (LPARAM)selected_identifier);

			int index = get_hotkey_array_index_from_selected_identifier(std::string(selected_identifier));

			if (index >= 0 && index < hotkeys.size())
			{
				char buttonText[MAX_PATH];
				GetDlgItemText(hwnd, id, buttonText, MAX_PATH);
				SetDlgItemText(hwnd, id, "...");

				GetUserHotkey(hotkeys[index]);
				UpdateSelectedHotkeyTextBox(hwnd);

				SetDlgItemText(hwnd, id, buttonText);
			}
		}

		if (id == IDC_HOTKEY_SEARCH)
		{
			static char hotkeySearchQuery[MAX_PATH] = { 0 };
			GetDlgItemText(hwnd, IDC_HOTKEY_SEARCH, hotkeySearchQuery, sizeof(hotkeySearchQuery) / sizeof(hotkeySearchQuery[0]));
			build_hotkey_list(GetDlgItem(hwnd, IDC_HOTKEY_LIST), hotkeySearchQuery);
		}
	}
				   break;
	default:
		return FALSE;
	}
	return TRUE;
}


HWND WINAPI CreateTrackbar(
	HWND hwndDlg,  // handle of dialog box (parent window)
	HMENU hMenu,   // handle of trackbar to use as identifier
	UINT iMin,     // minimum value in trackbar range 
	UINT iMax,     // maximum value in trackbar range 
	UINT iSelMin,  // minimum value in trackbar selection 
	UINT iSelMax, // maximum value in trackbar selection 
	UINT x, // x pos
	UINT y, // y pos
	UINT w
)// width
{


	HWND hwndTrack = CreateWindowEx(
		0,                             // no extended styles 
		TRACKBAR_CLASS,                // class name 
		"Trackbar Control",            // title (caption) 
		WS_CHILD | WS_VISIBLE |
		/*TBS_TOOLTIPS |*/ TBS_FIXEDLENGTH | TBS_TOOLTIPS,  // style 
		x, y,                        // position 
		w, 30,                       // size 
		hwndDlg,                       // parent window 
		hMenu,               // control identifier 
		app_hInstance,                 // instance 
		NULL);                          // no WM_CREATE parameter 


	SendMessage(hwndTrack, TBM_SETRANGE,
		(WPARAM)TRUE,                   // redraw flag 
		(LPARAM)MAKELONG(iMin, iMax));  // min. & max. positions 

	LPARAM pageSize;
	if (iMax < 10) {
		pageSize = 1;
		SendMessage(hwndTrack, TBM_SETTIC,
			0, 2);
	}
	else {
		pageSize = 10;
		SendMessage(hwndTrack, TBM_SETTIC,
			0, 100);
	}
	SendMessage(hwndTrack, TBM_SETPAGESIZE,
		0, pageSize);                  // new page size 
	SendMessage(hwndTrack, TBM_SETLINESIZE,
		0, pageSize);
	/*
		SendMessage(hwndTrack, TBM_SETSEL,
			(WPARAM) FALSE,                  // redraw flag
			(LPARAM) MAKELONG(iSelMin, iSelMax));
	*/
	SendMessage(hwndTrack, TBM_SETPOS,
		(WPARAM)TRUE,                   // redraw flag 
		(LPARAM)iSelMin);

	SetFocus(hwndTrack);

	return hwndTrack;
}

