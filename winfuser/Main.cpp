#include <Siv3D.hpp> // OpenSiv3D v0.6.3
#include <Siv3D/Windows/Windows.hpp>
#include <fileapi.h>
#include <string>
#include <regex>

#include <winternl.h>
#include <ntstatus.h>

#include <RestartManager.h>
#include <winerror.h>

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


	while (System::Update())
	{
		// ファイルがドラッグアンドドロップされたら
		if (DragDrop::HasNewFilePaths()) {
			// ファイルの総数
			Array<DroppedFilePath> files = DragDrop::GetDroppedFilePaths();
			int files_n = files.size();

			// Win32でファイル読み込み
			// 読み取りアクセス、
			//HANDLE hFile = CreateFileA(file_path.c_str(), GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			//Print << hFile;

			/*if (hFile == INVALID_HANDLE_VALUE) {
				printf("CreateFile failed\n");
				return 1;
			}*/

			//BY_HANDLE_FILE_INFORMATION file_information;
			//GetFileInformationByHandle(hFile, &file_information);

			//Print << file_information.ftCreationTime.dwLowDateTime;

			//NtQuerySystemInformation

			// RestartManagerセッションの開始
			DWORD dw_session, dw_error;
			WCHAR sz_session_key[CCH_RM_SESSION_KEY + 1]{};

			dw_error = RmStartSession(&dw_session, 0, sz_session_key);
			if (dw_error != ERROR_SUCCESS) {
				throw std::runtime_error("fail to start restart manager.");
			}

			// ファイルをセッションに登録
			//wchar_t* buf;
			//file_path.toWstr().assign(buf);
			LPCWSTR files_list[256] = {};
			for (int i = 0; i < files_n; i++) {
				std::string str = files[i].path.toUTF8();
				str = std::regex_replace(str, std::regex("/"), "\\");
				std::wstring wstr = Unicode::FromUTF8(str).toWstr();

				files_list[i] = L"D:\\func2.csv";//(LPCWSTR)wstr.c_str();
				Print << Unicode::FromWstring(std::wstring(files_list[i]));
			}
			//PCWSTR file_path = L"D:\\ts.txt";
			dw_error = RmRegisterResources(dw_session, files_n, files_list, 0, NULL, 0, NULL);
			if (dw_error != ERROR_SUCCESS) {
				Console << U"Err";
				throw std::runtime_error("fail to register target files.");
			}

			// そのファイルを使用しているプロセスの一覧を取得
			UINT n_proc_info_needed = 0;
			UINT n_proc_info = 256;
			RM_PROCESS_INFO* rgpi = new RM_PROCESS_INFO[n_proc_info];
			DWORD dw_reason;

			dw_error = RmGetList(dw_session, &n_proc_info_needed, &n_proc_info, rgpi, &dw_reason);

			if (dw_error == ERROR_MORE_DATA) {
				Print << U"this file is opened by " << (int)n_proc_info_needed;
				delete[] rgpi;
				n_proc_info = n_proc_info_needed;
				rgpi = new RM_PROCESS_INFO[n_proc_info];
				dw_error = RmGetList(dw_session, &n_proc_info_needed, &n_proc_info, rgpi, &dw_reason);
			}
			else if (dw_error == ERROR_SUCCESS) {
				Print << U"this file is opened by " << (int)n_proc_info_needed << U"processes.";
			}
			else {
				Print << U"Error";
				break;
			}
			if (dw_error != ERROR_SUCCESS) {
				Print << dw_error;
				//throw std::runtime_error("fail to get process list.");
			}
			
			RmEndSession(dw_session);

			Print << U"Total: " << n_proc_info_needed;
			for (int i = 0; i < n_proc_info_needed; i++) {
				Print << U"プロセスID: " << rgpi[i].Process.dwProcessId;
				Print << U"アプリ名: " << Unicode::FromWstring((std::wstring)rgpi[i].strAppName);
				Print << U"アプリのタイプ：: " << rgpi[i].ApplicationType;
			}
		}
	}
}

//
// = アドバイス =
// Debug ビルドではプログラムの最適化がオフになります。
// 実行速度が遅いと感じた場合は Release ビルドを試しましょう。
// アプリをリリースするときにも、Release ビルドにするのを忘れないように！
//
// 思ったように動作しない場合は「デバッグの開始」でプログラムを実行すると、
// 出力ウィンドウに詳細なログが表示されるので、エラーの原因を見つけやすくなります。
//
// = お役立ちリンク | Quick Links =
//
// Siv3D リファレンス
// https://zenn.dev/reputeless/books/siv3d-documentation
//
// Siv3D Reference
// https://zenn.dev/reputeless/books/siv3d-documentation-en
//
// Siv3D コミュニティへの参加
// Slack や Twitter, BBS で気軽に質問や情報交換ができます。
// https://zenn.dev/reputeless/books/siv3d-documentation/viewer/community
//
// Siv3D User Community
// https://zenn.dev/reputeless/books/siv3d-documentation-en/viewer/community
//
// 新機能の提案やバグの報告 | Feedback
// https://github.com/Siv3D/OpenSiv3D/issues
//
