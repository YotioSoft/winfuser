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

// ハンドル一覧の取得
PSYSTEM_HANDLE_INFORMATION_EX getHandleList() {
	PSYSTEM_HANDLE_INFORMATION_EX pSysHandleInformation = NULL;
	DWORD sys_hwnd_info_size = sizeof(pSysHandleInformation);
	DWORD needed = 0;
	NTSTATUS status;

	do {
		sys_hwnd_info_size += sizeof(SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX) * 0x10000;
		PSYSTEM_HANDLE_INFORMATION_EX newPtr = (PSYSTEM_HANDLE_INFORMATION_EX)realloc(pSysHandleInformation, sys_hwnd_info_size);
		if (NULL == newPtr)
			break;
		pSysHandleInformation = newPtr;
		ULONG returnLength = 0;
		status = fpNtQuerySystemInformation((SYSTEM_INFORMATION_CLASS)SystemExtendedHandleInformation, pSysHandleInformation, sys_hwnd_info_size, &returnLength);
	} while (status == STATUS_INFO_LENGTH_MISMATCH);

	if (NT_ERROR(status)) {
		free(pSysHandleInformation);
		return NULL;
	}

	return pSysHandleInformation;
}

// ハンドルのタイプを取得
String getHandleType(HANDLE handle) {
	ULONG handle_type_size = 0;
	fpNtQueryObject(handle, ObjectTypeInformation, NULL, 0, &handle_type_size);
	PPUBLIC_OBJECT_TYPE_INFORMATION handle_type = (PPUBLIC_OBJECT_TYPE_INFORMATION)malloc(handle_type_size);

	NTSTATUS status = fpNtQueryObject(handle, ObjectTypeInformation, handle_type, handle_type_size, &handle_type_size);

	if (NT_ERROR(status)) {
		free(handle_type);
		return U"";
	}

	String ret_str = Unicode::FromWstring(handle_type->TypeName.Buffer);
	free(handle_type);
	return ret_str;
}

//

// プロセスのファイルパスを取得
String getProcessFilePath(HANDLE handle) {
	TCHAR process_filename[1024] = { 0 };
	GetProcessImageFileName(handle, process_filename, 1024);

	return Unicode::FromWstring(process_filename);
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
				Console << U"Couldn't get handle list";
				continue;
			}

			Print << U"TOTAL: " << pSysHandleInformation->HandleCount;
			for (ULONG i = 0; i < pSysHandleInformation->HandleCount; i++) {
				HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, false, pSysHandleInformation->Handles[i].HandleValue);
				if (hProcess == INVALID_HANDLE_VALUE) {
					Console << U"Open Error: " << GetLastError();
					continue;
				}

				// ハンドルのタイプを取得
				String handle_type = getHandleType((HANDLE)pSysHandleInformation->Handles[i].HandleValue);

				if (pSysHandleInformation->Handles[i].HandleValue != 23700) {
					continue;
				}
				Console << i << U" Object Type: |" << handle_type << U"|";

				Console << i << U":" << pSysHandleInformation->Handles[i].HandleValue << U" " << pSysHandleInformation->Handles[i].UniqueProcessId << U" type: " << pSysHandleInformation->Handles[i].Object;
				Console << GetLastError() << U" " << hProcess;

				// ファイル名を取得
				String process_filepath = getProcessFilePath(hProcess);
				Console << process_filepath;
			}
			Print << U"Done.";
		}
	}
}
