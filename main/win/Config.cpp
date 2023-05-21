/***************************************************************************
						  config.c  -  description
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

#include <Windows.h>
#include <winuser.h>
#include <stdio.h>
#include "rombrowser.h"
#include "commandline.h"
#include "../../winproject/resource.h"
#include "config.h"

#include "../lua/Recent.h"
#include <vcr.h>
#include "translation.h"

CONFIG Config;
std::vector<t_hotkey*> hotkeys;

void hotkey_to_string(t_hotkey* hotkeys, char* buf)
{
	int k = hotkeys->key;
	buf[0] = 0;

	if (!hotkeys->ctrl && !hotkeys->shift && !hotkeys->alt && !hotkeys->key)
	{
		strcpy(buf, "(nothing)");
		return;
	}

	if (hotkeys->ctrl)
		strcat(buf, "Ctrl ");
	if (hotkeys->shift)
		strcat(buf, "Shift ");
	if (hotkeys->alt)
		strcat(buf, "Alt ");
	if (k)
	{
		char buf2[32];
		if ((k >= '0' && k <= '9') || (k >= 'A' && k <= 'Z'))
			sprintf(buf2, "%c", (char)k);
		else if ((k >= VK_F1 && k <= VK_F24))
			sprintf(buf2, "F%d", k - (VK_F1 - 1));
		else if ((k >= VK_NUMPAD0 && k <= VK_NUMPAD9))
			sprintf(buf2, "Num%d", k - VK_NUMPAD0);
		else switch (k)
		{
		case VK_SPACE: strcpy(buf2, "Space"); break;
		case VK_BACK: strcpy(buf2, "Backspace"); break;
		case VK_TAB: strcpy(buf2, "Tab"); break;
		case VK_CLEAR: strcpy(buf2, "Clear"); break;
		case VK_RETURN: strcpy(buf2, "Enter"); break;
		case VK_PAUSE: strcpy(buf2, "Pause"); break;
		case VK_CAPITAL: strcpy(buf2, "Caps"); break;
		case VK_PRIOR: strcpy(buf2, "PageUp"); break;
		case VK_NEXT: strcpy(buf2, "PageDn"); break;
		case VK_END: strcpy(buf2, "End"); break;
		case VK_HOME: strcpy(buf2, "Home"); break;
		case VK_LEFT: strcpy(buf2, "Left"); break;
		case VK_UP: strcpy(buf2, "Up"); break;
		case VK_RIGHT: strcpy(buf2, "Right"); break;
		case VK_DOWN: strcpy(buf2, "Down"); break;
		case VK_SELECT: strcpy(buf2, "Select"); break;
		case VK_PRINT: strcpy(buf2, "Print"); break;
		case VK_SNAPSHOT: strcpy(buf2, "PrintScrn"); break;
		case VK_INSERT: strcpy(buf2, "Insert"); break;
		case VK_DELETE: strcpy(buf2, "Delete"); break;
		case VK_HELP: strcpy(buf2, "Help"); break;
		case VK_MULTIPLY: strcpy(buf2, "Num*"); break;
		case VK_ADD: strcpy(buf2, "Num+"); break;
		case VK_SUBTRACT: strcpy(buf2, "Num-"); break;
		case VK_DECIMAL: strcpy(buf2, "Num."); break;
		case VK_DIVIDE: strcpy(buf2, "Num/"); break;
		case VK_NUMLOCK: strcpy(buf2, "NumLock"); break;
		case VK_SCROLL: strcpy(buf2, "ScrollLock"); break;
		case /*VK_OEM_PLUS*/0xBB: strcpy(buf2, "=+"); break;
		case /*VK_OEM_MINUS*/0xBD: strcpy(buf2, "-_"); break;
		case /*VK_OEM_COMMA*/0xBC: strcpy(buf2, ","); break;
		case /*VK_OEM_PERIOD*/0xBE: strcpy(buf2, "."); break;
		case VK_OEM_7: strcpy(buf2, "'\""); break;
		case VK_OEM_6: strcpy(buf2, "]}"); break;
		case VK_OEM_5: strcpy(buf2, "\\|"); break;
		case VK_OEM_4: strcpy(buf2, "[{"); break;
		case VK_OEM_3: strcpy(buf2, "`~"); break;
		case VK_OEM_2: strcpy(buf2, "/?"); break;
		case VK_OEM_1: strcpy(buf2, ";:"); break;
		default:
			sprintf(buf2, "(%d)", k);
			break;
		}
		strcat(buf, buf2);
	}
}

#include "../ini.h"


CONFIG get_default_config()
{
	CONFIG config = { };

	config.fast_forward_hotkey = {
		.name = "Fast-forward",
		.key = VK_TAB,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = 0,
	};

	config.speed_up_hotkey = {
		.name = "Speed up",
		.key = VK_OEM_PLUS,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = IDC_INCREASE_MODIFIER,
	};

	config.speed_down_hotkey = {
		.name = "Speed down",
		.key = VK_OEM_MINUS,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = IDC_DECREASE_MODIFIER,
	};

	config.frame_advance_hotkey = {
		.name = "Frame advance",
		.key = VK_OEM_5,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = EMU_FRAMEADVANCE,
	};

	config.pause_hotkey = {
		.name = "Pause",
		.key = VK_PAUSE,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = EMU_PAUSE,
	};

	config.toggle_read_only_hotkey = {
		.name = "Toggle read-only",
		.key = 0x38 /* 8 */,
		.ctrl = 1,
		.shift = 0,
		.alt = 0,
		.command = EMU_VCRTOGGLEREADONLY,
	};

	config.start_movie_playback_hotkey = {
		.name = "Start movie playback",
		.key = 0x50 /* P */,
		.ctrl = 1,
		.shift = 0,
		.alt = 0,
		.command = ID_START_PLAYBACK,
	};

	config.stop_movie_playback_hotkey = {
		.name = "Stop movie playback",
		.key = 0x53 /* S */,
		.ctrl = 1,
		.shift = 0,
		.alt = 0,
		.command = ID_STOP_PLAYBACK,
	};

	config.start_movie_recording_hotkey = {
		.name = "Start movie recording",
		.key = 0x52 /* R */,
		.ctrl = 1,
		.shift = 0,
		.alt = 0,
		.command = ID_START_RECORD,
	};

	config.stop_movie_recording_hotkey = {
		.name = "Stop movie recording",
		.key = 0x53 /* S */,
		.ctrl = 1,
		.shift = 0,
		.alt = 0,
		.command = ID_STOP_RECORD,
	};

	config.take_screenshot_hotkey = {
		.name = "Take screenshot",
		.key = VK_F12,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = GENERATE_BITMAP,
	};

	config.save_to_current_slot_hotkey = {
		.name = "Save to current slot",
		.key = 0x49 /* I */,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = STATE_SAVE,
	};

	config.load_from_current_slot_hotkey = {
		.name = "Load from current slot",
		.key = 0x50 /* P */,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = STATE_RESTORE,
	};

	config.restart_movie_hotkey = {
		.name = "Restart playing movie",
		.key = 0x52 /* R */,
		.ctrl = 1,
		.shift = 1,
		.alt = 0,
		.command = ID_RESTART_MOVIE,
	};

	config.play_latest_movie_hotkey = {
		.name = "Play latest movie",
		.key = 0x50 /* P */,
		.ctrl = 1,
		.shift = 1,
		.alt = 0,
		.command = ID_REPLAY_LATEST,
	};

	config.save_to_slot_1_hotkey = {
		.name = "Save to slot 1",
		.key = 0x31 /* 1 */,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = (ID_SAVE_1 - 1) + 1,
	};

	config.save_to_slot_2_hotkey = {
		.name = "Save to slot 2",
		.key = 0x32 /* 2 */,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = (ID_SAVE_1 - 1) + 2,
	};

	config.save_to_slot_3_hotkey = {
		.name = "Save to slot 3",
		.key = 0x33 /* 3 */,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = (ID_SAVE_1 - 1) + 3,
	};

	config.save_to_slot_4_hotkey = {
		.name = "Save to slot 4",
		.key = 0x34 /* 4 */,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = (ID_SAVE_1 - 1) + 4,
	};

	config.save_to_slot_5_hotkey = {
		.name = "Save to slot 5",
		.key = 0x35 /* 5 */,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = (ID_SAVE_1 - 1) + 5,
	};

	config.save_to_slot_6_hotkey = {
		.name = "Save to slot 6",
		.key = 0x36 /* 6 */,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = (ID_SAVE_1 - 1) + 6,
	};

	config.save_to_slot_7_hotkey = {
		.name = "Save to slot 7",
		.key = 0x37 /* 7 */,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = (ID_SAVE_1 - 1) + 7,
	};

	config.save_to_slot_8_hotkey = {
		.name = "Save to slot 8",
		.key = 0x38 /* 8 */,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = (ID_SAVE_1 - 1) + 8,
	};

	config.save_to_slot_9_hotkey = {
		.name = "Save to slot 9",
		.key = 0x39 /* 9 */,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = (ID_SAVE_1 - 1) + 9,
	};

	config.load_from_slot_1_hotkey = {
	  .name = "Load from slot 1",
	  .key = VK_F1,
	  .ctrl = 0,
	  .shift = 0,
	  .alt = 0,
	  .command = (ID_LOAD_1 - 1) + 1,
	};

	config.load_from_slot_2_hotkey = {
		.name = "Load from slot 2",
		.key = VK_F2,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = (ID_LOAD_1 - 1) + 2,
	};

	config.load_from_slot_3_hotkey = {
		.name = "Load from slot 3",
		.key = VK_F3,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = (ID_LOAD_1 - 1) + 3,
	};

	config.load_from_slot_4_hotkey = {
		.name = "Load from slot 4",
		.key = VK_F4,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = (ID_LOAD_1 - 1) + 4,
	};

	config.load_from_slot_5_hotkey = {
		.name = "Load from slot 5",
		.key = VK_F5,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = (ID_LOAD_1 - 1) + 5,
	};

	config.load_from_slot_6_hotkey = {
		.name = "Load from slot 6",
		.key = VK_F6,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = (ID_LOAD_1 - 1) + 6,
	};

	config.load_from_slot_7_hotkey = {
		.name = "Load from slot 7",
		.key = VK_F7,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = (ID_LOAD_1 - 1) + 7,
	};

	config.load_from_slot_8_hotkey = {
		.name = "Load from slot 8",
		.key = VK_F8,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = (ID_LOAD_1 - 1) + 8,
	};

	config.load_from_slot_9_hotkey = {
		.name = "Load from slot 9",
		.key = VK_F9,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = (ID_LOAD_1 - 1) + 9,
	};

	config.select_slot_1_hotkey = {
	  .name = "Select slot 1",
	  .key = 0x31 /* 1 */,
	  .ctrl = 0,
	  .shift = 0,
	  .alt = 0,
	  .command = (ID_CURRENTSAVE_1 - 1) + 1,
	};

	config.select_slot_2_hotkey = {
		.name = "Select slot 2",
		.key = 0x32 /* 2 */,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = (ID_CURRENTSAVE_1 - 1) + 2,
	};

	config.select_slot_3_hotkey = {
		.name = "Select slot 3",
		.key = 0x33 /* 3 */,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = (ID_CURRENTSAVE_1 - 1) + 3,
	};

	config.select_slot_4_hotkey = {
		.name = "Select slot 4",
		.key = 0x34 /* 4 */,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = (ID_CURRENTSAVE_1 - 1) + 4,
	};

	config.select_slot_5_hotkey = {
		.name = "Select slot 5",
		.key = 0x35 /* 5 */,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = (ID_CURRENTSAVE_1 - 1) + 5,
	};

	config.select_slot_6_hotkey = {
		.name = "Select slot 6",
		.key = 0x36 /* 6 */,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = (ID_CURRENTSAVE_1 - 1) + 6,
	};

	config.select_slot_7_hotkey = {
		.name = "Select slot 7",
		.key = 0x37 /* 7 */,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = (ID_CURRENTSAVE_1 - 1) + 7,
	};

	config.select_slot_8_hotkey = {
		.name = "Select slot 8",
		.key = 0x38 /* 8 */,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = (ID_CURRENTSAVE_1 - 1) + 8,
	};

	config.select_slot_9_hotkey = {
		.name = "Select slot 9",
		.key = 0x39 /* 9 */,
		.ctrl = 0,
		.shift = 0,
		.alt = 0,
		.command = (ID_CURRENTSAVE_1 - 1) + 9,
	};


	config.language = "English";
	config.show_fps = 1;
	config.show_vis_per_second = 1;
	config.prevent_suspicious_rom_loading = 0;
	config.is_savestate_warning_enabled = 1;
	config.is_rom_movie_compatibility_check_enabled = 1;
	config.is_fps_limited = 1;
	config.is_ini_compressed = 0;
	config.core_type = 1;
	config.is_fps_modifier_enabled = 0;
	config.fps_modifier = 100;
	config.frame_skip_frequency = 1;
	config.is_movie_loop_enabled = 0;
	config.is_frame_count_visual_zero_index = 0;
	config.is_movie_backup_enabled = 0;
	config.movie_backup_level = 0;
	config.cpu_clock_speed_multiplier = 1;
	config.is_fullscreen_start_enabled = 0;
	config.is_unfocused_pause_enabled = 0;
	config.use_global_plugins = 1;
	config.is_toolbar_enabled = 1;
	config.is_statusbar_enabled = 1;
	config.is_slot_autoincrement_enabled = 0;
	config.is_state_independent_state_loading_allowed = 0;
	config.is_good_name_column_enabled = 1;
	config.is_internal_name_column_enabled = 0;
	config.is_internal_name_column_enabled = 0;
	config.is_country_column_enabled = 1;
	config.is_size_column_enabled = 0;
	config.is_comments_column_enabled = 0;
	config.is_filename_column_enabled = 1;
	config.is_md5_column_enabled = 0;
	config.is_default_plugins_directory_used = 1;
	config.is_default_saves_directory_used = 1;
	config.is_default_screenshots_directory_used = 1;
	config.plugins_directory = "";
	config.saves_directory = "";
	config.screenshots_directory = "";
	config.states_path = "";
	config.recent_rom_paths = std::vector<std::string>(10);
	config.is_recent_movie_paths_frozen = 0;
	config.rom_browser_sorted_column = 0;
	config.rom_browser_sort_method = "ASC";
	config.is_rom_browser_recursion_enabled = 0;
	config.is_reset_recording_disabled = 1;
	config.is_internal_capture_forced = 0;
	config.is_capture_cropped_screen_dc = 0;
	config.is_unknown_hotkey_selection_allowed = 1;
	config.avi_capture_path = "";
	config.synchronization_mode = 1;
	config.lua_script_path = "";
	config.recent_lua_script_paths = std::vector<std::string>(10);
	config.is_recent_scripts_frozen = 0;
	config.is_lua_simple_dialog_enabled = 0;
	config.is_lua_exit_confirm_enabled = 0;
	config.is_statusbar_frequent_refresh_enabled = 0;
	config.is_round_towards_zero_enabled = 0;
	config.is_float_exception_propagation_enabled = 0;
	config.is_input_delay_enabled = 0;
	config.is_lua_double_buffered = 1;

	return config;
}

mINI::INIStructure handle_config_ini(bool is_reading, mINI::INIStructure ini) {
#define HANDLE_INT_VALUE(x, y) if(is_reading) { y = std::stoi(ini["Config"][x]); } else { ini["Config"][x] = std::to_string(y); }
#define HANDLE_STRING_VALUE(x, y) if(is_reading) { y = ini["Config"][x]; } else { ini["Config"][x] = y; }
#define HANDLE_STRING_VECTOR_VALUE(x, y)\
	if (is_reading)\
	{\
		int vector_length = 0;\
		for (size_t i = 0; i < INT32_MAX; i++)\
		{\
			if (!ini["Config"].has(std::string(x) + "_" + std::to_string(i)))\
			{\
				vector_length = i;\
				break;\
			}\
		}\
		y.clear();\
		for (size_t i = 0; i < vector_length; i++)\
		{\
			y.push_back(ini["Config"][std::string(x) + "_" + std::to_string(i)]);\
		}\
	}\
	else {\
		for (size_t i = 0; i < y.size(); i++)\
		{\
			ini["Config"][std::string(x) + "_" + std::to_string(i)] = y[i];\
		}\
	}\
	
	// TODO: 
	// consider trade-off: 
	// - cleverer macros { HANDLE_STRING_VALUE(language); }
	// or
	// - intellisense

	HANDLE_STRING_VALUE("language", Config.language);
	HANDLE_INT_VALUE("show_fps", Config.show_fps);
	HANDLE_INT_VALUE("show_vis_per_second", Config.show_vis_per_second);
	HANDLE_INT_VALUE("prevent_suspicious_rom_loading", Config.prevent_suspicious_rom_loading);
	HANDLE_INT_VALUE("is_savestate_warning_enabled", Config.is_savestate_warning_enabled);
	HANDLE_INT_VALUE("is_rom_movie_compatibility_check_enabled", Config.is_rom_movie_compatibility_check_enabled);
	HANDLE_INT_VALUE("is_fps_limited", Config.is_fps_limited);
	HANDLE_INT_VALUE("is_ini_compressed", Config.is_ini_compressed);
	HANDLE_INT_VALUE("core_type", Config.core_type);
	HANDLE_INT_VALUE("is_fps_modifier_enabled", Config.is_fps_modifier_enabled);
	HANDLE_INT_VALUE("fps_modifier", Config.fps_modifier);
	HANDLE_INT_VALUE("frame_skip_frequency", Config.frame_skip_frequency);
	HANDLE_INT_VALUE("is_movie_loop_enabled", Config.is_movie_loop_enabled);
	HANDLE_INT_VALUE("is_frame_count_visual_zero_index", Config.is_frame_count_visual_zero_index);
	HANDLE_INT_VALUE("is_movie_backup_enabled", Config.is_movie_backup_enabled);
	HANDLE_INT_VALUE("movie_backup_level", Config.movie_backup_level);
	HANDLE_INT_VALUE("cpu_clock_speed_multiplier", Config.cpu_clock_speed_multiplier);
	HANDLE_INT_VALUE("is_fullscreen_start_enabled", Config.is_fullscreen_start_enabled);
	HANDLE_INT_VALUE("is_unfocused_pause_enabled", Config.is_unfocused_pause_enabled);
	HANDLE_INT_VALUE("use_global_plugins", Config.use_global_plugins);
	HANDLE_INT_VALUE("is_toolbar_enabled", Config.is_toolbar_enabled);
	HANDLE_INT_VALUE("is_statusbar_enabled", Config.is_statusbar_enabled);
	HANDLE_INT_VALUE("is_slot_autoincrement_enabled", Config.is_slot_autoincrement_enabled);
	HANDLE_INT_VALUE("is_state_independent_state_loading_allowed", Config.is_state_independent_state_loading_allowed);
	HANDLE_INT_VALUE("is_good_name_column_enabled", Config.is_good_name_column_enabled);
	HANDLE_INT_VALUE("is_internal_name_column_enabled", Config.is_internal_name_column_enabled);
	HANDLE_INT_VALUE("is_country_column_enabled", Config.is_country_column_enabled);
	HANDLE_INT_VALUE("is_size_column_enabled", Config.is_size_column_enabled);
	HANDLE_INT_VALUE("is_filename_column_enabled", Config.is_filename_column_enabled);
	HANDLE_INT_VALUE("is_md5_column_enabled", Config.is_md5_column_enabled);
	HANDLE_INT_VALUE("is_default_plugins_directory_used", Config.is_default_plugins_directory_used);
	HANDLE_INT_VALUE("is_default_saves_directory_used", Config.is_default_saves_directory_used);
	HANDLE_INT_VALUE("is_default_screenshots_directory_used", Config.is_default_screenshots_directory_used);
	HANDLE_STRING_VALUE("plugins_directory", Config.plugins_directory);
	HANDLE_STRING_VALUE("saves_directory", Config.saves_directory);
	HANDLE_STRING_VALUE("screenshots_directory", Config.screenshots_directory);
	HANDLE_STRING_VALUE("states_path", Config.states_path);
	HANDLE_STRING_VECTOR_VALUE("recent_rom_paths", Config.recent_rom_paths);
	HANDLE_INT_VALUE("is_recent_rom_paths_frozen", Config.is_recent_rom_paths_frozen);
	HANDLE_STRING_VECTOR_VALUE("recent_movie_paths", Config.recent_movie_paths);
	HANDLE_INT_VALUE("is_recent_movie_paths_frozen", Config.is_recent_movie_paths_frozen);
	HANDLE_INT_VALUE("rom_browser_sorted_column", Config.rom_browser_sorted_column);
	HANDLE_STRING_VALUE("rom_browser_sort_method", Config.rom_browser_sort_method);
	HANDLE_INT_VALUE("is_rom_browser_recursion_enabled", Config.is_rom_browser_recursion_enabled);
	HANDLE_INT_VALUE("is_reset_recording_disabled", Config.is_reset_recording_disabled);
	HANDLE_INT_VALUE("is_unknown_hotkey_selection_allowed", Config.is_unknown_hotkey_selection_allowed);
	HANDLE_STRING_VALUE("avi_capture_path", Config.avi_capture_path);
	HANDLE_INT_VALUE("synchronization_mode", Config.synchronization_mode);
	HANDLE_STRING_VALUE("lua_script_path", Config.lua_script_path);
	HANDLE_STRING_VECTOR_VALUE("recent_lua_script_paths", Config.recent_lua_script_paths);
	HANDLE_INT_VALUE("is_recent_scripts_frozen", Config.is_recent_scripts_frozen);
	HANDLE_INT_VALUE("is_lua_simple_dialog_enabled", Config.is_lua_simple_dialog_enabled);
	HANDLE_INT_VALUE("is_lua_exit_confirm_enabled", Config.is_lua_exit_confirm_enabled);
	HANDLE_INT_VALUE("is_statusbar_frequent_refresh_enabled", Config.is_statusbar_frequent_refresh_enabled);
	HANDLE_INT_VALUE("is_round_towards_zero_enabled", Config.is_round_towards_zero_enabled);
	HANDLE_INT_VALUE("is_float_exception_propagation_enabled", Config.is_float_exception_propagation_enabled);
	HANDLE_INT_VALUE("is_input_delay_enabled", Config.is_input_delay_enabled);
	HANDLE_INT_VALUE("is_lua_double_buffered", Config.is_lua_double_buffered);

	return ini;
}

void save_config()
{
	mINI::INIFile file("config.ini");
	mINI::INIStructure ini;

	ini = handle_config_ini(false, ini);

	file.write(ini, true);
}

void load_config()
{
	hotkeys.clear();
	size_t first_offset = offsetof(CONFIG, fast_forward_hotkey);
	// NOTE:
	// last_offset should contain the offset of the last hotkey
	// this also requires that the hotkeys are laid out contiguously, or else the pointer arithmetic fails
	// i recommend inserting your new hotkeys before the savestate hotkeys... pretty please
	size_t last_offset = offsetof(CONFIG, select_slot_9_hotkey);
	for (size_t offset = first_offset; offset <= last_offset; offset += sizeof(t_hotkey)) {
		hotkeys.push_back((t_hotkey*)((char*)&Config + offset));
	}

	mINI::INIFile file("config.ini");
	mINI::INIStructure ini;
	file.read(ini);

	struct stat buf;
	printf("STAT: %d\n", stat("config.ini", &buf));

	if (stat("config.ini", &buf) != 0)
	{
		// save default config if it doesn't exist
		Config = get_default_config();
		save_config();
		printf("Generated default config file\n");
	}

	ini = handle_config_ini(false, ini);
}

void SetDlgItemHotkey(HWND hwnd, int idc, t_hotkey* hotkeys)
{
	char buf[MAX_PATH];
	hotkey_to_string(hotkeys, buf);
	SetDlgItemText(hwnd, idc, buf);
}

void SetDlgItemHotkeyAndMenu(HWND hwnd, int idc, t_hotkey* hotkeys, HMENU hmenu, int menuItemID)
{
	char buf[MAX_PATH];
	hotkey_to_string(hotkeys, buf);
	SetDlgItemText(hwnd, idc, buf);

	if (hmenu && menuItemID >= 0)
	{
		if (strcmp(buf, "(nothing)"))
			SetMenuAccelerator(hmenu, menuItemID, buf);
		else
			SetMenuAccelerator(hmenu, menuItemID, "");
	}
}

void ApplyHotkeys() {

	// TODO: get rid of this garbage and find another way

	extern HWND mainHWND;
	SetDlgItemHotkey(mainHWND, IDC_HOT_FASTFORWARD, &Config.fast_forward_hotkey);
	SetDlgItemHotkey(mainHWND, IDC_HOT_SPEEDUP, &Config.speed_up_hotkey);
	SetDlgItemHotkey(mainHWND, IDC_HOT_SPEEDDOWN, &Config.speed_down_hotkey);
	SetDlgItemHotkeyAndMenu(mainHWND, IDC_HOT_FRAMEADVANCE, &Config.frame_advance_hotkey, GetSubMenu(GetMenu(mainHWND), 1), 1);
	SetDlgItemHotkeyAndMenu(mainHWND, IDC_HOT_PAUSE, &Config.pause_hotkey, GetSubMenu(GetMenu(mainHWND), 1), 0);
	SetDlgItemHotkeyAndMenu(mainHWND, IDC_HOT_READONLY, &Config.toggle_read_only_hotkey, GetSubMenu(GetMenu(mainHWND), 3), 15);
	SetDlgItemHotkeyAndMenu(mainHWND, IDC_HOT_PLAY, &Config.start_movie_playback_hotkey, GetSubMenu(GetMenu(mainHWND), 3), 3);
	SetDlgItemHotkeyAndMenu(mainHWND, IDC_HOT_PLAYSTOP, &Config.stop_movie_playback_hotkey, GetSubMenu(GetMenu(mainHWND), 3), 4);
	SetDlgItemHotkeyAndMenu(mainHWND, IDC_HOT_RECORD, &Config.start_movie_recording_hotkey, GetSubMenu(GetMenu(mainHWND), 3), 0);
	SetDlgItemHotkeyAndMenu(mainHWND, IDC_HOT_RECORDSTOP, &Config.stop_movie_recording_hotkey, GetSubMenu(GetMenu(mainHWND), 3), 1);
	SetDlgItemHotkeyAndMenu(mainHWND, IDC_HOT_SCREENSHOT, &Config.take_screenshot_hotkey, GetSubMenu(GetMenu(mainHWND), 1), 2);
	SetDlgItemHotkeyAndMenu(mainHWND, IDC_CSAVE, &Config.save_to_current_slot_hotkey, GetSubMenu(GetMenu(mainHWND), 1), 4);
	SetDlgItemHotkeyAndMenu(mainHWND, IDC_CLOAD, &Config.load_from_current_slot_hotkey, GetSubMenu(GetMenu(mainHWND), 1), 6);

	SetDlgItemHotkey(mainHWND, IDC_1SAVE, &Config.save_to_slot_1_hotkey);
	SetDlgItemHotkey(mainHWND, IDC_2SAVE, &Config.save_to_slot_2_hotkey);
	SetDlgItemHotkey(mainHWND, IDC_3SAVE, &Config.save_to_slot_3_hotkey);
	SetDlgItemHotkey(mainHWND, IDC_4SAVE, &Config.save_to_slot_4_hotkey);
	SetDlgItemHotkey(mainHWND, IDC_5SAVE, &Config.save_to_slot_5_hotkey);
	SetDlgItemHotkey(mainHWND, IDC_6SAVE, &Config.save_to_slot_6_hotkey);
	SetDlgItemHotkey(mainHWND, IDC_7SAVE, &Config.save_to_slot_7_hotkey);
	SetDlgItemHotkey(mainHWND, IDC_8SAVE, &Config.save_to_slot_8_hotkey);
	SetDlgItemHotkey(mainHWND, IDC_9SAVE, &Config.save_to_slot_9_hotkey);

	SetDlgItemHotkey(mainHWND, IDC_1LOAD, &Config.load_from_slot_1_hotkey);
	SetDlgItemHotkey(mainHWND, IDC_2LOAD, &Config.load_from_slot_2_hotkey);
	SetDlgItemHotkey(mainHWND, IDC_3LOAD, &Config.load_from_slot_3_hotkey);
	SetDlgItemHotkey(mainHWND, IDC_4LOAD, &Config.load_from_slot_4_hotkey);
	SetDlgItemHotkey(mainHWND, IDC_5LOAD, &Config.load_from_slot_5_hotkey);
	SetDlgItemHotkey(mainHWND, IDC_6LOAD, &Config.load_from_slot_6_hotkey);
	SetDlgItemHotkey(mainHWND, IDC_7LOAD, &Config.load_from_slot_7_hotkey);
	SetDlgItemHotkey(mainHWND, IDC_8LOAD, &Config.load_from_slot_8_hotkey);
	SetDlgItemHotkey(mainHWND, IDC_9LOAD, &Config.load_from_slot_9_hotkey);

	SetDlgItemHotkeyAndMenu(mainHWND, IDC_1SEL, &Config.select_slot_1_hotkey, GetSubMenu(GetSubMenu(GetMenu(mainHWND), 1), 9), 0);
	SetDlgItemHotkeyAndMenu(mainHWND, IDC_2SEL, &Config.select_slot_2_hotkey, GetSubMenu(GetSubMenu(GetMenu(mainHWND), 1), 9), 1);
	SetDlgItemHotkeyAndMenu(mainHWND, IDC_3SEL, &Config.select_slot_3_hotkey, GetSubMenu(GetSubMenu(GetMenu(mainHWND), 1), 9), 2);
	SetDlgItemHotkeyAndMenu(mainHWND, IDC_4SEL, &Config.select_slot_4_hotkey, GetSubMenu(GetSubMenu(GetMenu(mainHWND), 1), 9), 3);
	SetDlgItemHotkeyAndMenu(mainHWND, IDC_5SEL, &Config.select_slot_5_hotkey, GetSubMenu(GetSubMenu(GetMenu(mainHWND), 1), 9), 4);
	SetDlgItemHotkeyAndMenu(mainHWND, IDC_6SEL, &Config.select_slot_6_hotkey, GetSubMenu(GetSubMenu(GetMenu(mainHWND), 1), 9), 5);
	SetDlgItemHotkeyAndMenu(mainHWND, IDC_7SEL, &Config.select_slot_7_hotkey, GetSubMenu(GetSubMenu(GetMenu(mainHWND), 1), 9), 6);
	SetDlgItemHotkeyAndMenu(mainHWND, IDC_8SEL, &Config.select_slot_8_hotkey, GetSubMenu(GetSubMenu(GetMenu(mainHWND), 1), 9), 7);
	SetDlgItemHotkeyAndMenu(mainHWND, IDC_9SEL, &Config.select_slot_9_hotkey, GetSubMenu(GetSubMenu(GetMenu(mainHWND), 1), 9), 8);

}

