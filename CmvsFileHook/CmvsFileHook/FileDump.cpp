#include "FileDump.h"

LPCSTR g_lpDumpFolder = ".\\Dump\\";

struct BPICTURE
{
	PDWORD pfile;
	DWORD size;
	DWORD width;
	DWORD hight;
	DWORD bit;
	DWORD type;
};

BOOL DumpFileAPI(LPCSTR lpPath,PDWORD pFile,PDWORD dwSize)
{
	HANDLE hFile = CreateFileA(lpPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		WriteFile(hFile, pFile, *dwSize, NULL, NULL);
		FlushFileBuffers(hFile);
		CloseHandle(hFile);
		return TRUE;
	}
	return FALSE;
}

typedef PDWORD(__thiscall* pReadScript)(
	PVOID pTHIS,
	LPCSTR lpFileName,
	PDWORD pFileSize);
pReadScript rawReadScript = (pReadScript)0x0046CE10;

typedef BOOL(__thiscall* pReadPicture)(
	PVOID pTHIS,
	BPICTURE* BPICTURE,
	LPCSTR lpString,
	LPCSTR lpFileName,
	LPCSTR a5);
pReadPicture rawReadPicture = (pReadPicture)0x0043FDF0;

PDWORD __fastcall newReadScript(PVOID pTHIS, DWORD dwReserved, LPCSTR lpFileName, PDWORD pFileSize)
{
	CHAR filepath[MAX_PATH] = { 0 };
	lstrcatA(filepath, g_lpDumpFolder);
	lstrcatA(filepath, lpFileName);

	PDWORD pFile = rawReadScript(pTHIS, lpFileName, pFileSize);

	DumpFileAPI(filepath, pFile, pFileSize);
	return pFile;
}

BOOL __fastcall newReadPicture(PVOID pTHIS, DWORD dwReserved, BPICTURE* BPICTURE, LPCSTR lpString, LPCSTR lpFileName, LPCSTR a5)
{
	DWORD imageSize = 0;
	BOOL isRead = FALSE;
	CHAR filepath[MAX_PATH] = { 0 };
	lstrcatA(filepath, g_lpDumpFolder);
	lstrcatA(filepath, lpFileName);

	isRead = rawReadPicture(pTHIS, BPICTURE, lpString, lpFileName, a5);

	imageSize = (BPICTURE->size) + 0x36;

	DWORD alloc = (DWORD)VirtualAlloc(NULL, imageSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (alloc)
	{
		memcpy((LPVOID)(alloc + 0x36), BPICTURE->pfile, BPICTURE->size);

		tagBITMAPFILEHEADER header = { 0 };
		tagBITMAPINFOHEADER info = { 0 };

		header.bfType = 0x4D42;
		header.bfSize = imageSize;
		header.bfOffBits = sizeof(tagBITMAPFILEHEADER) + sizeof(tagBITMAPINFOHEADER);

		info.biSize = sizeof(tagBITMAPINFOHEADER);
		info.biWidth = BPICTURE->width;
		info.biHeight = BPICTURE->hight;
		info.biPlanes = 0x1;
		info.biBitCount = (WORD)BPICTURE->bit;

		memcpy((LPVOID)alloc, &header, sizeof(header));
		memcpy((LPVOID)(alloc + sizeof(header)), &info, sizeof(info));

		DumpFileAPI(filepath, (PDWORD)alloc, &imageSize);
		VirtualFree((LPVOID)alloc, NULL, MEM_RELEASE);
	}

	return isRead;
}

VOID DumpFile()
{
	DetourRestoreAfterWith();
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	//DetourAttach(&(PVOID&)rawReadPicture, newReadPicture);
	DetourAttach(&(PVOID&)rawReadScript, newReadScript);
	DetourTransactionCommit();
}