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
		DWORD ProcessIDs[1000];
		DWORD ProcessNum;
		HMODULE Modules[1000];
		DWORD ModuleNum;
		char FileName[1000];
		DWORD RetSize;
		DWORD i, j;
		HANDLE hProcess;
		BOOL bResult;

		/* プロセスの一覧と数を取得 */
		EnumProcesses(ProcessIDs, sizeof(ProcessIDs), &RetSize);
		ProcessNum = RetSize / sizeof(DWORD);

		for (i = 0; i < ProcessNum; i++) {
			Console << U"プロセスID: " << ProcessIDs[i];

			hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, ProcessIDs[i]);

			if (hProcess == NULL) {
				Console << U"オープン失敗.";
			}
			else {
				/* プロセス中のモジュールの一覧を取得 */
				bResult = EnumProcessModules(hProcess, Modules, sizeof(Modules), &RetSize);
				if (bResult) {
					ModuleNum = RetSize / sizeof(HMODULE);

					/* 各モジュールのファイル名を表示 */
					for (j = 0; j < ModuleNum; j++) {
						GetModuleFileNameEx(hProcess, Modules[j], (LPWSTR)FileName, 1000);
						for (int k = 0; k < 1000; k++) {
							if (FileName[k] == '\0') {
								continue;
							}
							Console << FileName[k];
						}
						Console << U"----";
					}
				}
				else {
					Console << U"モジュール一覧の取得に失敗.";
				}

				CloseHandle(hProcess);
			}
		}
	}
}
