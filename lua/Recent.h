#pragma once
#include <windows.h> //linux trolled
					 // lol

//functions
void AddToRecentScripts(char* path);
void BuildRecentScriptsMenu(HWND);
void RunRecentScript(WORD);
void ClearRecent(BOOL);
void EnableRecentScriptsMenu(HMENU hMenu, BOOL flag);
