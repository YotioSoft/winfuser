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

// プロセス一覧を取得
PSYSTEM_HANDLE_INFORMATION_EX getHandleList() {
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
		return NULL;
	}

	return pSysHandleInformation;
}

// ハンドルのタイプを取得
String getHandleType(HANDLE handle) {
	ULONG handle_type_size = 0;
	fpNtQueryObject(handle, ObjectTypeInformation, NULL, 0, &handle_type_size);
	Console << handle;
	PPUBLIC_OBJECT_TYPE_INFORMATION handle_type = (PPUBLIC_OBJECT_TYPE_INFORMATION)malloc(handle_type_size);

	NTSTATUS status = fpNtQueryObject(handle, ObjectTypeInformation, handle_type, handle_type_size, &handle_type_size);

	if (NT_ERROR(status)) {
		free(handle_type);
		Console << U"Typegeterror";
		return U"";
	}

	String ret = Unicode::FromWstring(handle_type->TypeName.Buffer);
	free(handle_type);

	return ret;
}

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
			PSYSTEM_HANDLE_INFORMATION_EX pSysHandleInformation = getHandleList();
			if (pSysHandleInformation == NULL) {
				Console << U"Can't get handles";
				continue;
			}
			Print << U"TOTAL: " << pSysHandleInformation->HandleCount;

			// それぞれのハンドルについて調査
			int before_handle = 0;
			for (ULONG i = 0; i < pSysHandleInformation->HandleCount; i++) {
				// ハンドルのタイプを取得
				String handle_type = getHandleType((HANDLE)pSysHandleInformation->Handles[i].HandleValue);
				/*
				if (before_handle != pSysHandleInformation->Handles[i].HandleValue) {
					before_handle = pSysHandleInformation->Handles[i].HandleValue;

					Console << U"Handle: " << before_handle << U" Type: " << handle_type;
				}*/
				Console << U"Handle: " << pSysHandleInformation->Handles[i].HandleValue << U" Type: " << handle_type;
				// もしファイルでなければスキップ
				if (handle_type != U"Process") {
					continue;
				}

				// プロセスを開く
				HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, false, pSysHandleInformation->Handles[i].UniqueProcessId);
				if (hProcess == INVALID_HANDLE_VALUE) {
					Console << U"Open Error: " << GetLastError();
					continue;
				}

				// ファイル名を取得
				ULONG handle_name_size = 0;
				fpNtQueryObject((HANDLE)pSysHandleInformation->Handles[i].HandleValue, ObjectNameInformation, NULL, 0, &handle_name_size);
				POBJECT_NAME_INFORMATION handle_name = (POBJECT_NAME_INFORMATION)malloc(handle_name_size);

				NTSTATUS status = fpNtQueryObject((HANDLE)pSysHandleInformation->Handles[i].HandleValue, ObjectNameInformation, handle_name, handle_name_size, &handle_name_size);

				if (NT_ERROR(status)) {
					delete[](handle_name);
					continue;
				}

				Console << U"Handle Name: " << Unicode::FromWstring(handle_name->NAME.Buffer);


				TCHAR process_filename[1024] = { 0 };
				GetProcessImageFileName(hProcess, process_filename, 1024);
				Console << U"Name: " << Unicode::FromWstring(process_filename);

				//GetModuleFileNameEx(hProcess, )
				
			}
			Print << U"Done.";
		}
	}
}
