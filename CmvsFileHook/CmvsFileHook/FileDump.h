#pragma once
#include <Windows.h>
#include <detours.h>

BOOL DumpFileAPI(LPCSTR lpPath, PDWORD pFile, PDWORD dwSize);
VOID DumpFile();