#include "header.h"

#pragma comment(lib, "Rstrtmgr.lib")

#define GENERIC_READ			0x80000000L
#define FILE_SHARE_DELETE		0x00000004L
#define FILE_SHARE_READ			0x00000001L
#define FILE_SHARE_WRITE		0x00000002L
#define OPEN_EXISTING			3
#define FILE_ATTRIBUTE_NORMAL	0x00000080L

#define RM_SESSION_KEY_LEN		sizeof(GUID)
#define CCH_RM_SESSION_KEY		RM_SESSION_KEY_LEN * 2

void Main()
{
	// 背景の色を設定 | Set background color
	Scene::SetBackground(ColorF{ 0.8, 0.9, 1.0 });

	// 通常のフォントを作成 | Create a new font
	const Font font{ 60 };

	// 絵文字用フォントを作成 | Create a new emoji font
	const Font emojiFont{ 60, Typeface::ColorEmoji };

	// `font` が絵文字用フォントも使えるようにする | Set emojiFont as a fallback
	font.addFallback(emojiFont);

	// 画像ファイルからテクスチャを作成 | Create a texture from an image file
	const Texture texture{ U"example/windmill.png" };

	// 絵文字からテクスチャを作成 | Create a texture from an emoji
	const Texture emoji{ U"🐈"_emoji };

	// 絵文字を描画する座標 | Coordinates of the emoji
	Vec2 emojiPos{ 300, 150 };

	// テキストを画面にデバッグ出力 | Print a text
	Print << U"Push [A] key";



	//bool get = GetFileInformationByHandle()
	HANDLE hHandle = CreateEvent(NULL, FALSE, TRUE, L"sample");

	while (System::Update())
	{
		// ファイルがドラッグアンドドロップされたら
		if (DragDrop::HasNewFilePaths()) {
			// ファイルの総数
			Array<DroppedFilePath> files = DragDrop::GetDroppedFilePaths();
			int files_n = files.size();

			// ファイルリスト
			LPCWSTR files_list[256] = {};
			for (int i = 0; i < files_n; i++) {
				std::string str = files[i].path.toUTF8();
				str = std::regex_replace(str, std::regex("/"), "\\");
				std::wstring wstr = Unicode::FromUTF8(str).toWstr();

				files_list[i] = L"D:\\git\\team8_main\\trr.txt";//(LPCWSTR)wstr.c_str();
				Print << Unicode::FromWstring((std::wstring)files_list[i]);
			}

			// システム上のすべてのハンドルを取得
			DWORD dwPID = GetCurrentProcessId();
			PSYSTEM_HANDLE_INFORMATION_EX pSysHandleInformation = NULL;
			DWORD sys_hwnd_info_size = sizeof(pSysHandleInformation);
			DWORD needed = 0;
			NTSTATUS status;

			do {
				/*
				free(pSysHandleInformation);
				pSysHandleInformation = new SYSTEM_HANDLE_INFORMATION_EX;
				status = fpNtQuerySystemInformation((SYSTEM_INFORMATION_CLASS)SystemExtendedHandleInformation, pSysHandleInformation, sys_hwnd_info_size, &needed);
				sys_hwnd_info_size += sizeof(SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX) * 0x10000;
				Print << sys_hwnd_info_size;
				*/
				sys_hwnd_info_size += sizeof(SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX) * 0x10000;
				PSYSTEM_HANDLE_INFORMATION_EX newPtr = (PSYSTEM_HANDLE_INFORMATION_EX)realloc(pSysHandleInformation, sys_hwnd_info_size);
				if (NULL == newPtr)
					break;
				pSysHandleInformation = newPtr;
				ULONG returnLength = 0;
				status = fpNtQuerySystemInformation((SYSTEM_INFORMATION_CLASS)SystemExtendedHandleInformation, pSysHandleInformation, sys_hwnd_info_size, &returnLength);
			} while (status == STATUS_INFO_LENGTH_MISMATCH);

			if (NT_ERROR(status)) {
				Console << U"Error";
				delete(pSysHandleInformation);
				continue;
			}
			Print << U"TOTAL: " << pSysHandleInformation->HandleCount;
			for (ULONG i = 0; i < pSysHandleInformation->HandleCount; i++) {
				/*if (pSysHandleInformation->Handles[i].UniqueProcessId == dwPID && pSysHandleInformation->Handles[i].HandleValue == (USHORT)hHandle) {
					Print << pSysHandleInformation->Handles[i].Object;
				}*/

				// FileTypeでなければスキップ
				//if (pSysHandleInformation->Handles[i].ObjectTypeIndex)
				// タイプの取得
				
				HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, false, pSysHandleInformation->Handles[i].HandleValue);
				if (hProcess == INVALID_HANDLE_VALUE) {
					Console << U"Open Error: " << GetLastError();
					continue;
				}
				
				/*
				HANDLE hDup;
				if (!DuplicateHandle(hProcess, (HANDLE)pSysHandleInformation->Handles[i].Object, GetCurrentProcess(), &hDup, 0, false, DUPLICATE_SAME_ACCESS))
				{
					Console << U"DuplicateHandle failed!";
					continue;
				}
				*/
				ULONG handle_type_size = 0;
				fpNtQueryObject((HANDLE)pSysHandleInformation->Handles[i].HandleValue, ObjectTypeInformation, NULL, 0, &handle_type_size);
				PPUBLIC_OBJECT_TYPE_INFORMATION handle_type = (PPUBLIC_OBJECT_TYPE_INFORMATION)malloc(handle_type_size);
				
				NTSTATUS status = fpNtQueryObject((HANDLE)pSysHandleInformation->Handles[i].HandleValue, ObjectTypeInformation, handle_type, handle_type_size, &handle_type_size);
				
				if (NT_ERROR(status)) {
					delete[](handle_type);
					continue;
				}
				/*
				if (Unicode::FromWstring(handle_type->TypeName.Buffer) == U"Process") {
					delete[](handle_type);
					continue;
				}*/
				
				if (pSysHandleInformation->Handles[i].HandleValue != 23700) {
					continue;
				}
				Console << i << U" Object Type: |" << Unicode::FromWstring(handle_type->TypeName.Buffer) << U"|";
				delete[](handle_type);

				Console << i << U":" << pSysHandleInformation->Handles[i].HandleValue << U" " << pSysHandleInformation->Handles[i].UniqueProcessId << U" type: " << pSysHandleInformation->Handles[i].Object;
				Console << GetLastError() << U" " << hProcess;

				// ファイル名を取得
				TCHAR process_filename[1024] = { 0 };
				GetProcessImageFileName(hProcess, process_filename, 1024);
				Console << U"Name: " << Unicode::FromWstring(process_filename);

				//GetModuleFileNameEx(hProcess, )
			}
			Print << U"Done.";
		}
	}
}
