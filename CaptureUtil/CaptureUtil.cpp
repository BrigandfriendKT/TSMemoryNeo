#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <tchar.h>
#include <mutex>
#include <atomic>
#include <string>
#include <vector>
#include <cstdio>          
#include <cstdarg> 
#include <cmath>
#include "plugin2.h"
#include "filter2.h"
#include "Image.h"
#include <uxtheme.h>
#include <commdlg.h>
#include <commctrl.h>

#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib, "UxTheme.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "comctl32.lib")

#define numberof(a) (sizeof(a)/sizeof(a[0]))
#define TIMER_ID_REDRAW_RETRY 1
#define TIMER_ID_MEDIA_INFO_POLL 2


#define TIMER_ID_EASTEREGG_CLEAR 60

static int g_easterEggClickCount = 0;
static DWORD g_easterEggLastClickTick = 0;
static std::wstring g_easterEggOverlayText;
HWND g_hwndEasterEggOverlay = NULL;

// ============================================================
// イースターエッグ (隠しメッセージ) ※既存機能とは完全に独立
// ============================================================
namespace EasterEgg {

	// UTF-8バイト列を1バイトごとXOR(0x5A)で軽く隠しただけの、ごく簡易な難読化
	static const unsigned char kSecretData[] = {
	0xBC, 0xD4, 0xF2, 0xB9, 0xDB, 0xCD, 0xB9, 0xD8, 0xEA, 0xB9, 0xD9, 0xF1, 0xB9, 0xD9, 0xE6, 0xB9,
    0xD9, 0xCD, 0xB9, 0xDB, 0xFD, 0xB9, 0xDB, 0xD8, 0xB9, 0xD8, 0xD1, 0xB9, 0xDB, 0xE4, 0xB9, 0xDB,
	0xF7, 0xB9, 0xDB, 0xD7, 0xB9, 0xD8, 0xEB, 0xB9, 0xD9, 0xDB, 0xB9, 0xD9, 0xF9, 0xB9, 0xDB, 0xD6,
	0x50, 0xB5, 0xE6, 0xC8, 0xB5, 0xE6, 0xCA, 0xB5, 0xE6, 0xC8, 0xB5, 0xE6, 0xCC, 0xBF, 0xE3, 0xEE,
	0xB5, 0xE6, 0xC2, 0xBC, 0xC6, 0xD2, 0xB5, 0xE6, 0xCB, 0xB5, 0xE6, 0xCE, 0xBC, 0xCD, 0xFF, 0xB5,
	0xE6, 0xD2, 0xBC, 0xC6, 0xF6, 0xBC, 0xCD, 0xFF, 0xB5, 0xE6, 0xD3, 0xB9, 0xDB, 0xFD, 0xB5, 0xE6,
	0xCB, 0xB5, 0xE6, 0xCB, 0xBF, 0xCB, 0xF2, 0xBF, 0xE3, 0xEE, 0xB9, 0xD8, 0xC8, 0xB2, 0xE5, 0xD4,
	0xB9, 0xDB, 0xD2, 0xB9, 0xDB, 0xE4, 0xB9, 0xDB, 0xCD, 0xB9, 0xDB, 0xC5, 0xB9, 0xDA, 0xD8, 0x50,
	0xB9, 0xDB, 0xC9, 0xB9, 0xD8, 0xD6, 0xB9, 0xDB, 0xD1, 0xB9, 0xD8, 0xD3, 0xBF, 0xDF, 0xD2, 0xB9,
	0xD8, 0xD8, 0xB9, 0xDB, 0xC9, 0xB9, 0xDB, 0xF4, 0xB9, 0xD8, 0xEA, 0xB9, 0xD9, 0xF1, 0xB9, 0xD9,
	0xE6, 0xB9, 0xD9, 0xCD, 0xB9, 0xDB, 0xD6, 0xB3, 0xCF, 0xED, 0xB9, 0xDB, 0xD5, 0xBD, 0xEC, 0xC0,
	0xB9, 0xDB, 0xD7, 0xB9, 0xDB, 0xE4, 0xB9, 0xDB, 0xC3, 0xB9, 0xD8, 0xD2, 0xB9, 0xDB, 0xDC, 0xB9,
	0xDB, 0xF1, 0xB9, 0xDA, 0xD8, 0xB9, 0xDA, 0xD8, 0xB9, 0xDA, 0xD8, 0xB5, 0xE6, 0xDB, 0x50, 0x50,
	0xB9, 0xDB, 0xE4, 0xB9, 0xDB, 0xF7, 0xB9, 0xDB, 0xD7, 0xB9, 0xD8, 0xEB, 0xB9, 0xD9, 0xDB, 0xB9,
	0xD9, 0xF9, 0xB9, 0xDB, 0xF4, 0xBF, 0xDD, 0xF0, 0xBF, 0xC8, 0xE8, 0xBC, 0xFF, 0xC9, 0xB9, 0xDA,
	0xDB, 0xBE, 0xE2, 0xD3, 0xBE, 0xE0, 0xCF, 0xB9, 0xDB, 0xDE, 0xB9, 0xD8, 0xD7, 0xB9, 0xDB, 0xF5,
	0xB9, 0xDA, 0xDB, 0xBC, 0xCD, 0xF3, 0xBD, 0xDA, 0xF6, 0xBD, 0xEF, 0xCA, 0xBF, 0xF4, 0xC5, 0xB9,
	0xD8, 0xC8, 0xBC, 0xD4, 0xF2, 0xB9, 0xDB, 0xCD, 0xB9, 0xDB, 0xFC, 0xB9, 0xD8, 0xD1, 0xBE, 0xE7,
	0xC6, 0xB2, 0xDA, 0xDF, 0xB9, 0xDB, 0xF4, 0x18, 0x28, 0x33, 0x3D, 0x3B, 0x34, 0x3E, 0x3C, 0x28,
	0x33, 0x3F, 0x34, 0x3E, 0x11, 0x0E, 0xB5, 0xE6, 0xD2, 0x0E, 0x1B, 0x11, 0x1B, 0x12, 0x13, 0x08,
	0x15, 0xB5, 0xE6, 0xD3, 0xB9, 0xDB, 0xFD, 0xB9, 0xDB, 0xCD, 0xB9, 0xDB, 0xC5, 0xB9, 0xDA, 0xD8
	};

	static std::wstring DecodeSecretMessage()
	{
		std::vector<unsigned char> raw(sizeof(kSecretData));
		for (size_t i = 0; i < sizeof(kSecretData); i++) {
			raw[i] = kSecretData[i] ^ 0x5A;
		}
		int wlen = MultiByteToWideChar(CP_UTF8, 0, (LPCCH)raw.data(), (int)raw.size(), NULL, 0);
		if (wlen <= 0) return L"";
		std::wstring result(wlen, 0);
		MultiByteToWideChar(CP_UTF8, 0, (LPCCH)raw.data(), (int)raw.size(), &result[0], wlen);
		return result;
	}

#define EASTEREGG_WINDOW_CLASS L"TSMemoryNeoConsole"
#define TIMER_ID_TYPEWRITER 1

	enum ConsoleState {
		STATE_TYPING_INTRO,
		STATE_PROGRESS,
		STATE_COMPLETE_PAUSE,
		STATE_TYPING_MESSAGE,
		STATE_DONE
	};

	static ConsoleState g_state = STATE_TYPING_INTRO;
	static std::vector<std::wstring> g_introLines;
	static std::wstring g_secretMessage;
	static std::wstring g_currentLine;
	static size_t g_lineIndex = 0;
	static size_t g_charIndex = 0;
	static int g_progressPercent = 0;
	static DWORD g_pauseStartTick = 0;
	static HFONT g_consoleFont = NULL;
	static HFONT g_consoleFontBig = NULL;
	static HWND g_consoleHwnd = NULL;

	static void ResetConsoleState()
	{
		g_state = STATE_TYPING_INTRO;
		g_introLines.clear();
		g_introLines.push_back(L"Initializing...");
		g_introLines.push_back(L"Decoding secret message...");
		g_lineIndex = 0;
		g_charIndex = 0;
		g_currentLine.clear();
		g_progressPercent = 0;
		g_secretMessage = DecodeSecretMessage();
	}

	static LRESULT CALLBACK ConsoleWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		switch (msg) {
		case WM_CREATE:
			ResetConsoleState();
			SetTimer(hwnd, TIMER_ID_TYPEWRITER, 30, NULL);
			return 0;

		case WM_TIMER:
			if (wparam == TIMER_ID_TYPEWRITER) {
				switch (g_state) {
				case STATE_TYPING_INTRO:
					if (g_lineIndex < g_introLines.size()) {
						const std::wstring& line = g_introLines[g_lineIndex];
						if (g_charIndex < line.size()) {
							g_currentLine.push_back(line[g_charIndex]);
							g_charIndex++;
						}
						else {
							g_lineIndex++;
							g_charIndex = 0;
							g_currentLine.clear();
						}
					}
					else {
						g_state = STATE_PROGRESS;
						g_progressPercent = 0;
					}
					break;

				case STATE_PROGRESS:
					g_progressPercent += 4;
					if (g_progressPercent >= 100) {
						g_progressPercent = 100;
						g_state = STATE_COMPLETE_PAUSE;
						g_pauseStartTick = GetTickCount();
					}
					break;

				case STATE_COMPLETE_PAUSE:
					// 一定時間待ったら自動で進むが、キー入力/クリックでもすぐ進める(WM_LBUTTONDOWN/WM_KEYDOWN側で処理)
					if (GetTickCount() - g_pauseStartTick > 4000) {
						g_state = STATE_TYPING_MESSAGE;
						g_charIndex = 0;
						g_currentLine.clear();
					}
					break;

				case STATE_TYPING_MESSAGE:
					if (g_charIndex < g_secretMessage.size()) {
						g_currentLine.push_back(g_secretMessage[g_charIndex]);
						g_charIndex++;
					}
					else {
						g_state = STATE_DONE;
						KillTimer(hwnd, TIMER_ID_TYPEWRITER);
					}
					break;

				default:
					break;
				}
				InvalidateRect(hwnd, NULL, FALSE);
			}
			return 0;

		case WM_LBUTTONDOWN:
		case WM_KEYDOWN:
			if (g_state == STATE_DONE) {
				DestroyWindow(hwnd);
			}
			else if (g_state == STATE_COMPLETE_PAUSE) {
				g_state = STATE_TYPING_MESSAGE;
				g_charIndex = 0;
				g_currentLine.clear();
			}
			return 0;

		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			RECT rc; GetClientRect(hwnd, &rc);
			FillRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, RGB(0, 255, 0));

			if (g_state == STATE_TYPING_INTRO) {
				HGDIOBJ old = SelectObject(hdc, g_consoleFont);
				int y = 30, lineHeight = 22;
				for (size_t i = 0; i < g_lineIndex && i < g_introLines.size(); i++) {
					TextOutW(hdc, 20, y, g_introLines[i].c_str(), (int)g_introLines[i].size());
					y += lineHeight;
				}
				TextOutW(hdc, 20, y, g_currentLine.c_str(), (int)g_currentLine.size());
				SelectObject(hdc, old);
			}
			else if (g_state == STATE_PROGRESS || g_state == STATE_COMPLETE_PAUSE) {
				HGDIOBJ old = SelectObject(hdc, g_consoleFont);
				// バー本体 (20マス)
				const int barCells = 20;
				int filled = g_progressPercent * barCells / 100;
				std::wstring bar = L"[";
				for (int i = 0; i < barCells; i++) bar += (i < filled) ? L'#' : L' ';
				bar += L"]";

				wchar_t buf[64];
				wsprintfW(buf, L"%s %3d%%", bar.c_str(), g_progressPercent);

				RECT rcBar = rc;
				DrawText(hdc, buf, -1, &rcBar, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

				if (g_state == STATE_COMPLETE_PAUSE) {
					SelectObject(hdc, g_consoleFontBig);
					RECT rcComplete = rc;
					rcComplete.top = rc.bottom / 2 + 40;
					DrawText(hdc, L"COMPLETE!!!", -1, &rcComplete, DT_CENTER | DT_TOP | DT_SINGLELINE);

					// 「Press any key」を点滅表示 (約500msごとにON/OFF)
					if ((GetTickCount() / 500) % 2 == 0) {
						SelectObject(hdc, g_consoleFont);
						RECT rcPress = rc;
						rcPress.top = rcComplete.top + 50;
						DrawText(hdc, L"Press any key", -1, &rcPress, DT_CENTER | DT_TOP | DT_SINGLELINE);
					}
				}
				SelectObject(hdc, old);
			}
			else { // STATE_TYPING_MESSAGE or STATE_DONE
				HGDIOBJ old = SelectObject(hdc, g_consoleFont);
				TextOutW(hdc, 20, 30, L"MESSAGE:", 8);

				// メッセージを緑の線だけで囲む(中は黒のまま)
				RECT rcMsg = rc;
				rcMsg.left += 30; rcMsg.right -= 30;
				rcMsg.top = 70; rcMsg.bottom = rc.bottom - 60;

				HPEN greenPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
				HGDIOBJ oldPen = SelectObject(hdc, greenPen);
				HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));   // 塗りつぶしなし
				Rectangle(hdc, rcMsg.left - 10, rcMsg.top - 10, rcMsg.right + 10, rcMsg.bottom + 10);
				SelectObject(hdc, oldPen);
				SelectObject(hdc, oldBrush);
				DeleteObject(greenPen);

				DrawText(hdc, g_currentLine.c_str(), -1, &rcMsg, DT_CENTER | DT_VCENTER | DT_WORDBREAK);

				if (g_state == STATE_DONE) {
					RECT rcFooter = rc;
					rcFooter.top = rc.bottom - 50;
					TextOutW(hdc, 20, rcFooter.top, L"END OF SECRET MESSAGE", 21);
					TextOutW(hdc, 20, rcFooter.top + 22, L"Press any key to close...", 26);
				}
				SelectObject(hdc, old);
			}

			EndPaint(hwnd, &ps);
			return 0;
		}

		case WM_DESTROY:
			KillTimer(hwnd, TIMER_ID_TYPEWRITER);
			g_consoleHwnd = NULL;
			return 0;
		}
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	static void ShowSecretConsole(HINSTANCE hInstDll, HWND hOwner)
	{
		if (g_consoleHwnd && IsWindow(g_consoleHwnd)) {
			SetForegroundWindow(g_consoleHwnd);
			return;
		}

		static bool classRegistered = false;
		if (!classRegistered) {
			WNDCLASSEXW wcex = { 0 };
			wcex.cbSize = sizeof(wcex);
			wcex.lpszClassName = EASTEREGG_WINDOW_CLASS;
			wcex.lpfnWndProc = ConsoleWndProc;
			wcex.hInstance = hInstDll;
			wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
			wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
			RegisterClassExW(&wcex);
			classRegistered = true;
		}

		if (!g_consoleFont) {
			g_consoleFont = CreateFont(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
				DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
				FIXED_PITCH, TEXT("Consolas"));
			if (!g_consoleFont) {
				g_consoleFont = CreateFont(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
					DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
					FIXED_PITCH, TEXT("Lucida Console"));
			}
		}
		if (!g_consoleFontBig) {
			g_consoleFontBig = CreateFont(-28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
				DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
				FIXED_PITCH, TEXT("Consolas"));
		}

		g_consoleHwnd = CreateWindowExW(WS_EX_APPWINDOW, EASTEREGG_WINDOW_CLASS, L"TSMemoryNeo",
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
			CW_USEDEFAULT, CW_USEDEFAULT, 560, 360,
			hOwner, NULL, hInstDll, NULL);

		if (g_consoleHwnd) {
			ShowWindow(g_consoleHwnd, SW_SHOW);
			SetForegroundWindow(g_consoleHwnd);
		}
	}

} // namespace EasterEgg


// ============================================================
// 共通
// ============================================================
static HINSTANCE hInst = NULL;
static CImageCodec ImageCodec;
static TCHAR szIniFileName[MAX_PATH];

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
	if (dwReason == DLL_PROCESS_ATTACH) hInst = hInstance;
	return TRUE;
}

// ============================================================
// 共有状態
// ============================================================
static int  g_jpeg_level = 90;
static int  g_png_level = 6;
static EDIT_HANDLE* g_edit_handle = nullptr;
static HWND g_panel_hwnd = NULL;
static OBJECT_HANDLE g_last_media_object = nullptr;
static std::wstring g_pending_capture_path;   
static double g_last_media_total_time = -1;   
static int    g_media_stable_count = 0;       
static int    g_media_poll_count = 0;

static bool  g_copy_filename = false;
static int   g_seq_count = 0;
static int   g_seq_digits = 3;
static bool  g_deinterlace_enabled = false;
static double g_squeeze_ratio = 1.0;   // ini: SqueezeRatio (対象はwidth==1440の動画のみ)

// ============================================================
// 出力解像度プリセット (長辺の目安ピクセル数。0=オリジナル)
// ============================================================
struct ResolutionPreset { const wchar_t* label; int width; };
static const ResolutionPreset g_resolutions[] = {
	{ L"オリジナル",         0 },
	{ L"4K (3840x2160)",  3840 },
	{ L"FHD (1920x1080)", 1920 },
	{ L"HD (1280x720)",   1280 },
	{ L"SD (640x480)",     640 },
	{ L"リサイズなし",      -1 },
};

// ============================================================
// 出力解像度プリセットの前方宣言に必要な、レンダリング結果保存情報一式
// ============================================================
struct CaptureRenderContext {
	std::wstring filename;
	int format = 0;
	int jpeg_level = 90;
	int png_level = 6;
	bool deinterlace = false;
	double squeeze_ratio = 1.0;   // 1.0=補正なし
	int resolution_index = 0;
	int media_width = 0;
	int frame = 0;
};

// ============================================================
// rendering_scene_videoのコールバックで受け取った画像を、補正・保存する
// buffer: PIXEL_RGBA形式, pitchは1行のバイト数(パディングを含む場合がある)
// ============================================================
static void SaveRenderedFrame(CaptureRenderContext* ctx, const void* buffer, int origW, int origH, int pitch)
{
	if (!buffer || origW <= 0 || origH <= 0) {
		return;
	}

	// pitch(1行のバイト数)を考慮して、隙間なく詰まった配列にコピーする
	std::vector<PIXEL_RGBA> srcRgba(size_t(origW) * origH);
	const BYTE* srcBytes = (const BYTE*)buffer;
	for (int y = 0; y < origH; y++) {
		memcpy(&srcRgba[size_t(y) * origW], srcBytes + size_t(y) * pitch, size_t(origW) * sizeof(PIXEL_RGBA));
	}

	int format = ctx->format;
	int jpeg_level = ctx->jpeg_level;
	int png_level = ctx->png_level;
	int resolution_index = ctx->resolution_index;
	std::wstring filename = ctx->filename;
	double squeeze_ratio = ctx->squeeze_ratio;

	// ▼ インターレース解除(隣接ラインブレンド方式)
	bool deinterlace = ctx->deinterlace;
	if (deinterlace && origH >= 2) {
		std::vector<PIXEL_RGBA> deint(srcRgba.size());
		for (int y = 0; y < origH; y++) {
			int y0 = (y > 0) ? y - 1 : y;
			int y1 = (y < origH - 1) ? y + 1 : y;
			const PIXEL_RGBA* row0 = &srcRgba[size_t(y0) * origW];
			const PIXEL_RGBA* row1 = &srcRgba[size_t(y1) * origW];
			const PIXEL_RGBA* rowC = &srcRgba[size_t(y) * origW];
			PIXEL_RGBA* dst = &deint[size_t(y) * origW];
			for (int x = 0; x < origW; x++) {
				// 奇数ラインは上下平均、偶数ラインはそのまま(簡易ブレンド)
				if (y % 2 == 1) {
					dst[x].r = (unsigned char)((row0[x].r + row1[x].r) / 2);
					dst[x].g = (unsigned char)((row0[x].g + row1[x].g) / 2);
					dst[x].b = (unsigned char)((row0[x].b + row1[x].b) / 2);
					dst[x].a = rowC[x].a;
				}
				else {
					dst[x] = rowC[x];
				}
			}
		}
		srcRgba = std::move(deint);
	}

	// ▼ 「リサイズなし」が選択されているかどうかを先に判定しておく
	bool noResizeAtAll = (resolution_index >= 0 && resolution_index < (int)numberof(g_resolutions)
		&& g_resolutions[resolution_index].width < 0);

	int targetLongEdge = 0;
	if (resolution_index >= 0 && resolution_index < (int)numberof(g_resolutions))
		targetLongEdge = g_resolutions[resolution_index].width;

	int w = origW, h = origH;
	if (targetLongEdge > 0 && targetLongEdge < origW) {
		w = targetLongEdge;
		h = (int)((long long)origH * targetLongEdge / origW);
		if (h < 1) h = 1;
	}

	std::vector<PIXEL_RGBA> rgba;
	if (w == origW && h == origH) {
		rgba = std::move(srcRgba);
	}
	else {
		rgba.resize(size_t(w) * h);
		for (int y = 0; y < h; y++) {
			int sy0 = (int)((long long)y * origH / h);
			int sy1 = (int)((long long)(y + 1) * origH / h);
			if (sy1 <= sy0) sy1 = sy0 + 1;
			if (sy1 > origH) sy1 = origH;
			for (int x = 0; x < w; x++) {
				int sx0 = (int)((long long)x * origW / w);
				int sx1 = (int)((long long)(x + 1) * origW / w);
				if (sx1 <= sx0) sx1 = sx0 + 1;
				if (sx1 > origW) sx1 = origW;
				long rr = 0, gg = 0, bb = 0, aa = 0, cnt = 0;
				for (int sy = sy0; sy < sy1; sy++) {
					const PIXEL_RGBA* row = &srcRgba[size_t(sy) * origW];
					for (int sx = sx0; sx < sx1; sx++) {
						rr += row[sx].r; gg += row[sx].g; bb += row[sx].b; aa += row[sx].a;
						cnt++;
					}
				}
				if (cnt < 1) cnt = 1;
				PIXEL_RGBA& dst = rgba[size_t(y) * w + x];
				dst.r = (unsigned char)(rr / cnt);
				dst.g = (unsigned char)(gg / cnt);
				dst.b = (unsigned char)(bb / cnt);
				dst.a = (unsigned char)(aa / cnt);
			}
		}
	}

	std::vector<BYTE> bgr(size_t(w) * h * 3);
	for (int y = 0; y < h; y++) {
		const PIXEL_RGBA* src = &rgba[size_t(h - 1 - y) * w];
		BYTE* dst = &bgr[size_t(y) * w * 3];
		for (int x = 0; x < w; x++) {
			dst[x * 3 + 0] = src[x].b;
			dst[x * 3 + 1] = src[x].g;
			dst[x * 3 + 2] = src[x].r;
		}
	}

	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = w;
	bmi.bmiHeader.biHeight = h;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;

	TCHAR szOption[16] = TEXT("");
	LPCTSTR pszFormatName = ImageCodec.EnumSaveFormat(format);
	if (pszFormatName != NULL) {
		if (lstrcmpi(pszFormatName, TEXT("jpeg")) == 0)
			wsprintf(szOption, TEXT("%d"), jpeg_level);
		else if (lstrcmpi(pszFormatName, TEXT("png")) == 0)
			wsprintf(szOption, TEXT("%d"), png_level);
	}

	ImageCodec.SaveImage(filename.c_str(), format, szOption, &bmi, bgr.data(), NULL);
}

// ============================================================
// ② パネル側 (見た目): 旧CaptureUtilと同じ配置
// ============================================================
#define IDC_FILENAME   1000
#define IDC_FORMAT     1001
#define IDC_JPEGLEVEL  1002
#define IDC_PNGLEVEL   1003
#define IDC_CAPTURE    1007
#define IDC_RESOLUTION 1008
#define IDC_DEINTERLACE 1009
#define IDC_BROWSE     1010
#define IDC_EASTEREGG_OVERLAY 1201
#define TSMemoryWindowClass L"TSMemoryCapturePanel"   // ← 内部識別用。変更しない
#define TSMemoryDisplayName L"TSMemoryNeo Panel"      // ← 見た目のタイトル。ここだけ変更

static HFONT hfontControls = NULL;
static HFONT hfontSaveCaption = NULL;   // ← 保存ボタンの文字用(1.2倍サイズ)
static HFONT hfontFileName = NULL;      // ← ファイル名ボックス用(一回り小さいサイズ、\が¥にならないフォント)
static HWND hwndFileName = NULL;
static HWND hwndFormat = NULL;
static HWND hwndJpegLevel = NULL;
static HWND hwndPngLevel = NULL;
static HWND hwndSeqCount = NULL;
static HWND hwndSeqDigits = NULL;
static HWND hwndCapture = NULL;
static HWND hwndDeinterlace = NULL;
static HWND hwndResolution = NULL;
static HWND hwndBrowse = NULL;
static const int ROW_HEIGHT = 24;
static const int FONT_SIZE = 14;
static HBRUSH g_darkBrush = NULL;
static HBRUSH g_purpleBrush = NULL;
static HBRUSH g_editBrush = NULL;
static COLORREF COLOR_BG = RGB(45, 45, 48);
static COLORREF COLOR_TEXT = RGB(230, 230, 230);
static COLORREF COLOR_EDIT_BG = RGB(28, 28, 30);      // ← パネル本体より暗く
static COLORREF COLOR_EDIT_BORDER = RGB(95, 95, 100); // ← 少し明るめの境界線
static COLORREF COLOR_PURPLE = RGB(0x70, 0x6B, 0xCF); // #706BCF
static COLORREF COLOR_PURPLE_HOVER = RGB(0x8A, 0x85, 0xDE); // ← 追加: 保存ボタンのホバー色(少し明るい紫)
static RECT g_fileNameBoxRect = { 0 };   // ファイル名ボックスの外枠(親クライアント座標)
static COLORREF COLOR_EDIT_HOVER = RGB(45, 45, 50);   // ← 追加: 参照BOX(…)のホバー色

// ▼ パラメータボタン(解像度・拡張子・圧縮レベル)用の色
static COLORREF COLOR_PARAM_BG     = RGB(58, 58, 62);   // 通常時の背景
static COLORREF COLOR_PARAM_BORDER = RGB(85, 85, 90);   // 境界線
static COLORREF COLOR_PARAM_HOVER  = RGB(65, 132, 208); // ホバー時の背景(明るい青)

// ▼ パラメータボタンの現在選択状態
static int g_ui_resolution_index = 0;
static int g_ui_format_index = 0;
static int g_ui_jpeg_level = 90;
static int g_ui_png_level = 6;

// ▼ 各パラメータボタンのホバー状態(マウスが乗っているか)
static bool g_hoverResolution = false;
static bool g_hoverFormat = false;
static bool g_hoverJpeg = false;
static bool g_hoverPng = false;
static bool g_hoverCapture = false;   // ← 追加: 保存ボタンのホバー状態
static bool g_hoverBrowse = false;    // ← 追加: 参照BOX(…)のホバー状態
static int DpiScaled(int value, double scale);   // ← 前方宣言(WM_PAINTで使用するため)

// ▼ パラメータボタンをサブクラス化し、マウスの出入りを検知してホバー状態を更新する
static LRESULT CALLBACK ParamButtonSubclassProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam,
	UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	bool* pHover = (bool*)dwRefData;
	switch (msg) {
	case WM_MOUSEMOVE: {
		if (!*pHover) {
			*pHover = true;
			InvalidateRect(hwnd, NULL, TRUE);
			TRACKMOUSEEVENT tme = { sizeof(tme) };
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = hwnd;
			TrackMouseEvent(&tme);
		}
		break;
	}
	case WM_MOUSELEAVE: {
		*pHover = false;
		InvalidateRect(hwnd, NULL, TRUE);
		break;
	}
	case WM_NCDESTROY:
		RemoveWindowSubclass(hwnd, ParamButtonSubclassProc, uIdSubclass);
		break;
	}
	return DefSubclassProc(hwnd, msg, wparam, lparam);
}

// ▼ 指定した項目一覧をボタンの直下にポップアップメニューとして表示し、選ばれたインデックスを返す
//   (キャンセル時は currentIndex をそのまま返す)
static int ShowPopupList(HWND hwndOwner, HWND hwndAnchor, const std::vector<std::wstring>& items, int currentIndex)
{
	HMENU hMenu = CreatePopupMenu();
	for (size_t i = 0; i < items.size(); i++) {
		UINT flags = MF_STRING | (((int)i == currentIndex) ? MF_CHECKED : MF_UNCHECKED);
		AppendMenu(hMenu, flags, (UINT_PTR)(2000 + i), items[i].c_str());
	}
	RECT rc;
	GetWindowRect(hwndAnchor, &rc);
	int cmd = TrackPopupMenuEx(hMenu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN,
		rc.left, rc.bottom, hwndOwner, NULL);
	DestroyMenu(hMenu);
	if (cmd == 0) return currentIndex;  // キャンセルされた
	return cmd - 2000;
}




LRESULT CALLBACK wnd_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message) {
	case WM_DRAWITEM: {
		LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT)lparam;
		if (pdis->CtlID == IDC_CAPTURE) {
			// ▼ 角丸の紫ボタン(押下中は少し暗く)
			bool pressed = (pdis->itemState & ODS_SELECTED) != 0;
			COLORREF fill = pressed ? RGB(90, 86, 166) : (g_hoverCapture ? COLOR_PURPLE_HOVER : COLOR_PURPLE);
			HBRUSH hBrush = CreateSolidBrush(fill);
			HPEN hPen = CreatePen(PS_SOLID, 1, fill);
			HGDIOBJ oldBrush = SelectObject(pdis->hDC, hBrush);
			HGDIOBJ oldPen = SelectObject(pdis->hDC, hPen);
			RoundRect(pdis->hDC, pdis->rcItem.left, pdis->rcItem.top,
				pdis->rcItem.right, pdis->rcItem.bottom, 14, 14);
			SelectObject(pdis->hDC, oldBrush);
			SelectObject(pdis->hDC, oldPen);
			DeleteObject(hBrush);
			DeleteObject(hPen);

			SelectObject(pdis->hDC, hfontSaveCaption);

			// ▼ アイコン(上)とテキスト「保存」(下)を縦に並べて中央揃えする
			LPCTSTR szCaption = TEXT("保存");
			SIZE textSize;
			GetTextExtentPoint32(pdis->hDC, szCaption, lstrlen(szCaption), &textSize);

			const int iconH = 20, vgap = 4;
			int totalH = iconH + vgap + textSize.cy;
			int cx = (pdis->rcItem.left + pdis->rcItem.right) / 2;
			int startY = pdis->rcItem.top + ((pdis->rcItem.bottom - pdis->rcItem.top) - totalH) / 2;
			int iconCy = startY + iconH / 2;

			HPEN iconPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
			HGDIOBJ oldIconPen = SelectObject(pdis->hDC, iconPen);
			// 矢印の軸
			MoveToEx(pdis->hDC, cx, iconCy - 8, NULL); LineTo(pdis->hDC, cx, iconCy + 3);
			// 矢印の先端(ハの字)
			MoveToEx(pdis->hDC, cx - 6, iconCy - 3, NULL); LineTo(pdis->hDC, cx, iconCy + 3);
			MoveToEx(pdis->hDC, cx + 6, iconCy - 3, NULL); LineTo(pdis->hDC, cx, iconCy + 3);
			// 受け皿(器): 底辺+両端を立ち上げた「コ」の字型にして、単なる下線に見えないようにする
			int trayY = iconCy + 8;
			MoveToEx(pdis->hDC, cx - 9, trayY - 5, NULL); LineTo(pdis->hDC, cx - 9, trayY);
			LineTo(pdis->hDC, cx + 9, trayY);
			LineTo(pdis->hDC, cx + 9, trayY - 5);
			SelectObject(pdis->hDC, oldIconPen);
			DeleteObject(iconPen);

			SetTextColor(pdis->hDC, RGB(255, 255, 255));
			SetBkMode(pdis->hDC, TRANSPARENT);
			RECT textRect = pdis->rcItem;
			textRect.top = startY + iconH + vgap;
			DrawText(pdis->hDC, szCaption, -1, &textRect, DT_CENTER | DT_TOP | DT_SINGLELINE);
			return TRUE;
		}
		if (pdis->CtlID == IDC_BROWSE) {
			// ▼ 参照ボタン(…): ファイル名ボックスと同じ配色・角丸で描画
			HBRUSH hBrush = CreateSolidBrush(g_hoverBrowse ? COLOR_EDIT_HOVER : COLOR_EDIT_BG);
			FillRect(pdis->hDC, &pdis->rcItem, hBrush);
			DeleteObject(hBrush);

			HPEN hPen = CreatePen(PS_SOLID, 1, COLOR_EDIT_BORDER);
			HGDIOBJ oldPen = SelectObject(pdis->hDC, hPen);
			HGDIOBJ oldBrush = SelectObject(pdis->hDC, GetStockObject(NULL_BRUSH));
			RoundRect(pdis->hDC, pdis->rcItem.left, pdis->rcItem.top,
				pdis->rcItem.right, pdis->rcItem.bottom, 4, 4);
			SelectObject(pdis->hDC, oldPen);
			SelectObject(pdis->hDC, oldBrush);
			DeleteObject(hPen);

			SetTextColor(pdis->hDC, COLOR_TEXT);
			SetBkMode(pdis->hDC, TRANSPARENT);
			SelectObject(pdis->hDC, hfontControls);
			DrawText(pdis->hDC, TEXT("…"), -1, &pdis->rcItem,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			return TRUE;
		}
		if (pdis->CtlID == IDC_FORMAT || pdis->CtlID == IDC_JPEGLEVEL ||
			pdis->CtlID == IDC_PNGLEVEL || pdis->CtlID == IDC_RESOLUTION) {
			// ▼ パラメータボタン(解像度・拡張子・圧縮レベル): 背景+境界線+ホバー
			bool hover = false;
			switch (pdis->CtlID) {
			case IDC_RESOLUTION: hover = g_hoverResolution; break;
			case IDC_FORMAT:     hover = g_hoverFormat;     break;
			case IDC_JPEGLEVEL:  hover = g_hoverJpeg;       break;
			case IDC_PNGLEVEL:   hover = g_hoverPng;        break;
			}

			COLORREF bg = hover ? COLOR_PARAM_HOVER : COLOR_PARAM_BG;
			HBRUSH hBrush = CreateSolidBrush(bg);
			FillRect(pdis->hDC, &pdis->rcItem, hBrush);
			DeleteObject(hBrush);

			HPEN hPen = CreatePen(PS_SOLID, 1, COLOR_PARAM_BORDER);
			HGDIOBJ oldPen = SelectObject(pdis->hDC, hPen);
			HGDIOBJ oldBrush = SelectObject(pdis->hDC, GetStockObject(NULL_BRUSH));
			Rectangle(pdis->hDC, pdis->rcItem.left, pdis->rcItem.top, pdis->rcItem.right, pdis->rcItem.bottom);
			SelectObject(pdis->hDC, oldPen);
			SelectObject(pdis->hDC, oldBrush);
			DeleteObject(hPen);

			TCHAR szText[64];
			GetWindowText(pdis->hwndItem, szText, numberof(szText));
			SetTextColor(pdis->hDC, RGB(255, 255, 255));
			SetBkMode(pdis->hDC, TRANSPARENT);
			SelectObject(pdis->hDC, hfontControls);
			DrawText(pdis->hDC, szText, -1, &pdis->rcItem,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}
		return FALSE;
	}
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORBTN: {
		HDC hdcCtl = (HDC)wparam;
		HWND hwndCtl = (HWND)lparam;
		if (hwndCtl == g_hwndEasterEggOverlay) {
			SetTextColor(hdcCtl, RGB(255, 210, 80));
		}
		else {
			SetTextColor(hdcCtl, COLOR_TEXT);
		}
		SetBkColor(hdcCtl, COLOR_BG);
		SetBkMode(hdcCtl, TRANSPARENT);
		return (LRESULT)g_darkBrush;
	}
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORLISTBOX: {
		HDC hdcCtl = (HDC)wparam;
		SetTextColor(hdcCtl, COLOR_TEXT);
		SetBkColor(hdcCtl, COLOR_EDIT_BG);
		if (g_editBrush == NULL) g_editBrush = CreateSolidBrush(COLOR_EDIT_BG);
		return (LRESULT)g_editBrush;
	}
	case WM_COPYDATA: {
		COPYDATASTRUCT* pcds = (COPYDATASTRUCT*)lparam;
		if (pcds != nullptr && pcds->dwData == 1 && pcds->lpData != nullptr) {
			g_pending_capture_path.assign((LPCWSTR)pcds->lpData, pcds->cbData / sizeof(wchar_t));
			while (!g_pending_capture_path.empty() && g_pending_capture_path.back() == L'\0')
				g_pending_capture_path.pop_back();

			g_last_media_total_time = -1;
			g_media_stable_count = 0;
			g_media_poll_count = 0;
			SetTimer(hwnd, TIMER_ID_MEDIA_INFO_POLL, 300, NULL);
		}
		return TRUE;
	}
	case WM_TIMER: {
		if (wparam == TIMER_ID_MEDIA_INFO_POLL) {
			KillTimer(hwnd, TIMER_ID_MEDIA_INFO_POLL);
			g_media_poll_count++;

			double total_time = 0;
			if (g_edit_handle) {
				g_edit_handle->call_edit_section_param(&total_time,
					[](void* param, EDIT_SECTION* edit) {
						double* pTotal = (double*)param;
						MEDIA_INFO info = {};
						if (edit->get_media_info(g_pending_capture_path.c_str(), &info, sizeof(info))) {
							*pTotal = info.total_time;
						}
					});
			}

			bool stable = (total_time > 0 && total_time == g_last_media_total_time);
			g_media_stable_count = stable ? (g_media_stable_count + 1) : 0;
			g_last_media_total_time = total_time;

			// 2回連続で同じ長さが取れたら「確定」とみなす。最大約6秒で諦めて進める
			if (g_edit_handle) {
				g_edit_handle->call_edit_section_param(&g_pending_capture_path,
					[](void* param, EDIT_SECTION* edit) {
						std::wstring* p = (std::wstring*)param;
						MEDIA_INFO info = {};
						int totalFrames = 0;
						bool gotInfo = edit->get_media_info(p->c_str(), &info, sizeof(info));
						if (gotInfo && info.total_time > 0) {
							totalFrames = (int)(info.total_time * edit->info->rate / edit->info->scale + 0.5);
						}
						if (g_last_media_object) {
							edit->delete_object(g_last_media_object);
							g_last_media_object = nullptr;
						}
						OBJECT_HANDLE media = edit->create_object_from_media_file(p->c_str(), 0, 0, totalFrames);
						if (media) {
							g_last_media_object = media;
							OBJECT_LAYER_FRAME lf = edit->get_object_layer_frame(media);

							// ▼ スクイーズ放送(横1440px)の場合だけ、拡大率・縦横比を補正する
							//   これによりプレビュー画面にも即座に反映され、保存時にも改めて引き伸ばす必要がなくなる
							if (gotInfo && info.width == 1440 && g_squeeze_ratio != 1.0) {
								double scalePercent = g_squeeze_ratio * 100.0;                 // 例: 133.333 (横方向の拡大率)
								double aspectPercent = 100.0 * (100.0 / scalePercent - 1.0);   // 例: -25.000 (縦を100%のまま保つ補正値)

								char szScale[32], szAspect[32];
								sprintf_s(szScale, "%.3f", scalePercent);
								sprintf_s(szAspect, "%.3f", aspectPercent);

								edit->set_object_item_value(media, L"動画ファイル", L"拡大率", szScale);
								edit->set_object_item_value(media, L"動画ファイル", L"縦横比", szAspect);
								edit->set_object_item_value(media, L"映像再生", L"拡大率", szScale);
								edit->set_object_item_value(media, L"映像再生", L"縦横比", szAspect);
							}

							edit->set_focus_object(media);
							edit->set_cursor_layer_frame(lf.layer, lf.start);
						}
					});
			}
			else {
				// まだ安定していないので、もう少し待ってから再チェック
				SetTimer(hwnd, TIMER_ID_MEDIA_INFO_POLL, 300, NULL);
			}
		}
		else if (wparam == TIMER_ID_REDRAW_RETRY) {
			KillTimer(hwnd, TIMER_ID_REDRAW_RETRY);
			if (g_edit_handle) {
				g_edit_handle->call_edit_section_param(nullptr,
					[](void*, EDIT_SECTION* edit) {
						edit->set_cursor_layer_frame(edit->info->layer, edit->info->frame);
					});
			}
		}

		else if (wparam == TIMER_ID_EASTEREGG_CLEAR) {
			KillTimer(hwnd, TIMER_ID_EASTEREGG_CLEAR);
			g_easterEggOverlayText.clear();
			ShowWindow(g_hwndEasterEggOverlay, SW_HIDE);
		}

		return 0;
	}
	case WM_LBUTTONDBLCLK: {
		// 何もない背景をダブルクリックした時だけ発動 (ボタン等は自分でクリックを処理するため、ここには来ない)
		DWORD now = GetTickCount();
		if (now - g_easterEggLastClickTick > 3000) {
			g_easterEggClickCount = 0;   // 間隔が空きすぎたらリセット
		}
		g_easterEggLastClickTick = now;
		g_easterEggClickCount++;

		if (g_easterEggClickCount == 1) {
			g_easterEggOverlayText = L"残り2回";
		}
		else if (g_easterEggClickCount == 2) {
			g_easterEggOverlayText = L"残り1回";
		}
		else {
			g_easterEggOverlayText = L"congratulation!!";
			g_easterEggClickCount = 0;
			EasterEgg::ShowSecretConsole(hInst, hwnd);
		}

		SetWindowText(g_hwndEasterEggOverlay, g_easterEggOverlayText.c_str());
		ShowWindow(g_hwndEasterEggOverlay, SW_SHOW);

		SetTimer(hwnd, TIMER_ID_EASTEREGG_CLEAR, 900, NULL);
		return 0;
	}
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		RECT rc; GetClientRect(hwnd, &rc);

		// ▼ ファイル名ボックスの境界線(少し明るめの黒)
		if (!IsRectEmpty(&g_fileNameBoxRect)) {
			HPEN borderPen = CreatePen(PS_SOLID, 1, COLOR_EDIT_BORDER);
			HGDIOBJ oldPen2 = SelectObject(hdc, borderPen);
			HGDIOBJ oldBrush2 = SelectObject(hdc, GetStockObject(NULL_BRUSH));
			Rectangle(hdc, g_fileNameBoxRect.left, g_fileNameBoxRect.top,
				g_fileNameBoxRect.right, g_fileNameBoxRect.bottom);
			SelectObject(hdc, oldPen2);
			SelectObject(hdc, oldBrush2);
			DeleteObject(borderPen);
		}

		// ▼ パラメータボタンは WM_DRAWITEM 側で自前の境界線を描画しているので、
		//   ここでは何もしなくてよい


		EndPaint(hwnd, &ps);
		return 0;
	}
	case WM_COMMAND:
		switch (LOWORD(wparam)) {
		case IDC_CAPTURE: {
			TCHAR szBaseName[MAX_PATH];
			GetWindowText(hwndFileName, szBaseName, MAX_PATH);
			if (szBaseName[0] == '\0') {
				MessageBox(hwnd, TEXT("ファイル名を入力してください。"), NULL, MB_OK | MB_ICONEXCLAMATION);
				return 0;
			}

			int format = g_ui_format_index;
			LPCTSTR pszFormatName = ImageCodec.EnumSaveFormat(format);
			if (pszFormatName == NULL) return 0;

			int seqDigits = g_seq_digits;   // ← UIからではなく、内部固定値(3桁)を使用
			if (seqDigits < 1) seqDigits = 3;

			// 拡張子の分離
			TCHAR szExt[16];
			LPTSTR pszExistingExt = PathFindExtension(szBaseName);
			if (*pszExistingExt != '\0') {
				lstrcpy(szExt, pszExistingExt);
				*pszExistingExt = '\0';
			}
			else {
				wsprintf(szExt, TEXT(".%s"), ImageCodec.GetExtension(format));
			}

			// 常に一番小さい番号から探し、空いている最初の番号で保存する
			TCHAR szFileName[MAX_PATH], szFormatStr[16];
			wsprintf(szFormatStr, TEXT("%%s_%%0%dd%%s"), seqDigits);
			int seqNum = 0;
			while (true) {
				wsprintf(szFileName, szFormatStr, szBaseName, seqNum, szExt);
				if (!PathFileExists(szFileName)) break;
				seqNum++;
			}

			// ini保存 (次回起動時も設定を覚えている)
			bool deinterlace_now = (IsDlgButtonChecked(hwnd, IDC_DEINTERLACE) == BST_CHECKED);
			{
				WritePrivateProfileString(TEXT("Settings"), TEXT("FileName"), szBaseName, szIniFileName);
				if (pszFormatName != NULL)
					WritePrivateProfileString(TEXT("Settings"), TEXT("Format"), pszFormatName, szIniFileName);
				TCHAR szValue[16];
				wsprintf(szValue, TEXT("%d"), g_ui_jpeg_level);
				WritePrivateProfileString(TEXT("Settings"), TEXT("JpegLevel"), szValue, szIniFileName);
				wsprintf(szValue, TEXT("%d"), g_ui_png_level);
				WritePrivateProfileString(TEXT("Settings"), TEXT("PngLevel"), szValue, szIniFileName);
				wsprintf(szValue, TEXT("%d"), deinterlace_now ? 1 : 0);
				WritePrivateProfileString(TEXT("Settings"), TEXT("Deinterlace"), szValue, szIniFileName);
					szIniFileName, szBaseName, pszFormatName ? pszFormatName : L"(null)";
			}

			// ▼ ここからが新方式: レイヤー配置に依存せず、今カーソルがある位置を直接レンダリングする
			CaptureRenderContext* ctx = new CaptureRenderContext();
			ctx->filename = szFileName;
			ctx->format = format;
			ctx->jpeg_level = g_ui_jpeg_level;
			ctx->png_level = g_ui_png_level;
			ctx->deinterlace = deinterlace_now;
			ctx->resolution_index = g_ui_resolution_index;
			ctx->squeeze_ratio = 1.0;
			ctx->frame = 0;

			if (g_edit_handle) {
				// ① 現在のカーソル位置・対象動画の元解像度(スクイーズ判定用)を調べる
				g_edit_handle->call_edit_section_param(ctx,
					[](void* param, EDIT_SECTION* edit) {
						CaptureRenderContext* ctx = (CaptureRenderContext*)param;
						ctx->frame = edit->info->frame;

						int maxLayer = edit->info->layer_max;
						for (int layer = 0; layer <= maxLayer; layer++) {
							OBJECT_HANDLE obj = edit->find_object(layer, ctx->frame);
							if (!obj) continue;

							OBJECT_LAYER_FRAME lf = edit->get_object_layer_frame(obj);
							if (ctx->frame < lf.start || ctx->frame >= lf.end) continue;  // ← カーソルの範囲外なら無視

							int c = edit->get_effect_list(obj, nullptr, 0);
							if (c <= 0) continue;
							std::vector<EFFECT_HANDLE> ef(c);
							edit->get_effect_list(obj, ef.data(), c);

							bool isMedia = false;
							for (int i = 0; i < c; i++) {
								LPCWSTR n = edit->get_effect_name(ef[i]);
								if (n && lstrcmpiW(n, L"映像再生") == 0) { isMedia = true; break; }
							}
							if (!isMedia) continue;

							if (isMedia) {
								LPCSTR aliasRaw = edit->get_object_alias(obj);
									layer, aliasRaw ? aliasRaw : "(null)";
							}

							LPCSTR path = edit->get_object_item_value(obj, L"動画ファイル", L"ファイル");
							if (path) {
								int wlen = MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
								if (wlen > 0) {
									std::wstring wpath(wlen, 0);
									MultiByteToWideChar(CP_UTF8, 0, path, -1, &wpath[0], wlen);
									while (!wpath.empty() && wpath.back() == L'\0') wpath.pop_back();

									MEDIA_INFO info = {};
									bool gotInfo = edit->get_media_info(wpath.c_str(), &info, sizeof(info));
										gotInfo ? 1 : 0, info.width, info.height, g_squeeze_ratio;
									if (edit->get_media_info(wpath.c_str(), &info, sizeof(info))) {
										if (info.width == 1440 && g_squeeze_ratio != 1.0) {
											ctx->squeeze_ratio = g_squeeze_ratio;
											ctx->media_width = info.width;
										}
									}
								}
							}
							break;
						}
					});


				// ② その位置のフレームを実際にレンダリング依頼する(非同期。届いたら保存)
				bool requested = g_edit_handle->rendering_scene_video(ctx->frame, ctx,
					[](void* param, int frame, const void* buffer, int width, int height, int pitch) {
						CaptureRenderContext* ctx = (CaptureRenderContext*)param;
						SaveRenderedFrame(ctx, buffer, width, height, pitch);
						delete ctx;
					});
				if (!requested) {
					delete ctx;
				}
			}
			else {
				delete ctx;
			}

			if (g_copy_filename) {
				if (OpenClipboard(hwnd)) {
					EmptyClipboard();
					HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (lstrlen(szFileName) + 1) * sizeof(TCHAR));
					if (hGlobal != NULL) {
						LPTSTR p = (LPTSTR)GlobalLock(hGlobal);
						if (p) {
							lstrcpy(p, szFileName); GlobalUnlock(hGlobal);
							SetClipboardData(
#ifdef UNICODE
								CF_UNICODETEXT,
#else
								CF_TEXT,
#endif
								hGlobal);
						}
					}
					CloseClipboard();
				}
			}
			return 0;
		}
		case IDC_RESOLUTION:
			if (HIWORD(wparam) == BN_CLICKED) {
				std::vector<std::wstring> items;
				for (auto& r : g_resolutions) items.push_back(r.label);
				int sel = ShowPopupList(hwnd, hwndResolution, items, g_ui_resolution_index);
				if (sel != g_ui_resolution_index) {
					g_ui_resolution_index = sel;
					SetWindowText(hwndResolution, g_resolutions[sel].label);
				}
			}
			return 0;
		case IDC_FORMAT:
			if (HIWORD(wparam) == BN_CLICKED) {
				std::vector<std::wstring> items;
				for (int i = 0;; i++) {
					LPCTSTR name = ImageCodec.EnumSaveFormat(i);
					if (name == NULL) break;
					items.push_back(name);
				}
				int sel = ShowPopupList(hwnd, hwndFormat, items, g_ui_format_index);
				if (sel != g_ui_format_index && sel >= 0 && sel < (int)items.size()) {
					g_ui_format_index = sel;
					SetWindowText(hwndFormat, items[sel].c_str());

					LPCTSTR pszFormatName = ImageCodec.EnumSaveFormat(sel);
					bool isJpeg = pszFormatName != NULL && lstrcmpi(pszFormatName, TEXT("jpeg")) == 0;
					bool isPng = pszFormatName != NULL && lstrcmpi(pszFormatName, TEXT("png")) == 0;
					ShowWindow(hwndJpegLevel, isJpeg ? SW_SHOW : SW_HIDE);
					ShowWindow(hwndPngLevel, isPng ? SW_SHOW : SW_HIDE);
				}
			}
			return 0;
		case IDC_JPEGLEVEL:
			if (HIWORD(wparam) == BN_CLICKED) {
				std::vector<std::wstring> items;
				for (int i = 1; i <= 10; i++) {
					TCHAR szText[4]; wsprintf(szText, TEXT("%d"), i * 10);
					items.push_back(szText);
				}
				int curIndex = (g_ui_jpeg_level / 10) - 1;
				int sel = ShowPopupList(hwnd, hwndJpegLevel, items, curIndex);
				if (sel != curIndex && sel >= 0) {
					g_ui_jpeg_level = (sel + 1) * 10;
					SetWindowText(hwndJpegLevel, items[sel].c_str());
				}
			}
			return 0;
		case IDC_PNGLEVEL:
			if (HIWORD(wparam) == BN_CLICKED) {
				std::vector<std::wstring> items;
				for (int i = 0; i <= 9; i++) {
					TCHAR szText[2]; wsprintf(szText, TEXT("%d"), i);
					items.push_back(szText);
				}
				int sel = ShowPopupList(hwnd, hwndPngLevel, items, g_ui_png_level);
				if (sel != g_ui_png_level && sel >= 0) {
					g_ui_png_level = sel;
					SetWindowText(hwndPngLevel, items[sel].c_str());
				}
			}
			return 0;
		case IDC_BROWSE: {
			TCHAR szPath[MAX_PATH];
			GetWindowText(hwndFileName, szPath, MAX_PATH);

			OPENFILENAME ofn = { 0 };
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hwnd;
			ofn.lpstrFile = szPath;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFilter = TEXT("すべてのファイル\0*.*\0");
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_NOVALIDATE | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
			ofn.lpstrTitle = TEXT("保存先を選択");

			if (GetSaveFileName(&ofn)) {
				SetWindowText(hwndFileName, szPath);
			}
			return 0;
		}
		}
		break;
	}
	return DefWindowProc(hwnd, message, wparam, lparam);
}


static int DpiScaled(int value, double scale) { return (int)(value * scale + 0.5); }


// ============================================================
// 汎用プラグイン エントリポイント
// ============================================================
EXTERN_C __declspec(dllexport) void RegisterPlugin(HOST_APP_TABLE* host)
{
	if (g_darkBrush == NULL) g_darkBrush = CreateSolidBrush(COLOR_BG);
	if (g_purpleBrush == NULL) g_purpleBrush = CreateSolidBrush(COLOR_PURPLE);
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);  
	ImageCodec.Init();

	// --- ini読み込み(旧func_init相当) ---
	TCHAR szSaveFileName[MAX_PATH] = TEXT("");
	TCHAR szSaveFormat[8] = TEXT("bmp");
	GetModuleFileName(hInst, szIniFileName, MAX_PATH);
	PathRenameExtension(szIniFileName, TEXT(".ini"));
	GetPrivateProfileString(TEXT("Settings"), TEXT("FileName"), NULL, szSaveFileName, numberof(szSaveFileName), szIniFileName);
	if (szSaveFileName[0] == '\0') {
		SHGetSpecialFolderPath(NULL, szSaveFileName, CSIDL_MYPICTURES, FALSE);
		PathAppend(szSaveFileName, TEXT("Capture"));
	}
	GetPrivateProfileString(TEXT("Settings"), TEXT("Format"), TEXT("bmp"), szSaveFormat, numberof(szSaveFormat), szIniFileName);
	g_jpeg_level = GetPrivateProfileInt(TEXT("Settings"), TEXT("JpegLevel"), g_jpeg_level, szIniFileName);
	g_png_level = GetPrivateProfileInt(TEXT("Settings"), TEXT("PngLevel"), g_png_level, szIniFileName);
	g_copy_filename = GetPrivateProfileInt(TEXT("Settings"), TEXT("CopyFileName"), g_copy_filename, szIniFileName) != 0;
	g_seq_count = GetPrivateProfileInt(TEXT("Settings"), TEXT("SequenceCount"), 0, szIniFileName);
	g_seq_digits = GetPrivateProfileInt(TEXT("Settings"), TEXT("SequenceDigits"), 3, szIniFileName);
	g_deinterlace_enabled = GetPrivateProfileInt(TEXT("Settings"), TEXT("Deinterlace"), 0, szIniFileName) != 0;

	// ▼ 追加: iniのパスと、実際に読み込んだ値を記録
		szSaveFileName, szSaveFormat, g_jpeg_level, g_png_level, g_seq_count, g_seq_digits;

	g_squeeze_ratio = 1920.0 / 1440.0;

	// --- ウィンドウ生成 ---
	WNDCLASSEXW wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpszClassName = TSMemoryWindowClass;
	wcex.lpfnWndProc = wnd_proc;
	wcex.hInstance = hInst;
	wcex.hbrBackground = g_darkBrush;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.style = CS_DBLCLKS;
	RegisterClassExW(&wcex);

	UINT dpi = GetDpiForSystem();  // 例: 96(100%), 144(150%), 192(200%)
	double dpiScale = dpi / 96.0;

	const int MARGIN = DpiScaled(1, dpiScale);        // ← 左詰め対策の余白
	const int CONTENT_WIDTH = DpiScaled(360, dpiScale);
	const int WINDOW_WIDTH = 360 + 6 * 2;             // 余白の分だけウィンドウ自体も広げる
	const int WINDOW_HEIGHT = 80;
	const int CONTENT_RIGHT = MARGIN + CONTENT_WIDTH; // コンテンツ領域の右端(スケール済み)

	HWND hwnd = CreateWindowEx(0, TSMemoryWindowClass, TSMemoryDisplayName, WS_POPUP,
		CW_USEDEFAULT, CW_USEDEFAULT,
		DpiScaled(WINDOW_WIDTH, dpiScale), DpiScaled(WINDOW_HEIGHT, dpiScale),
		nullptr, nullptr, hInst, nullptr);

	hfontControls = CreateFont(-DpiScaled(FONT_SIZE, dpiScale), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FF_DONTCARE, TEXT("ＭＳ Ｐゴシック"));

	// ▼ 保存ボタンの文字を0.9倍にして、上下にも少し余白ができるようにする
	hfontSaveCaption = CreateFont(-DpiScaled((int)(FONT_SIZE * 0.9 + 0.5), dpiScale), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FF_DONTCARE, TEXT("ＭＳ Ｐゴシック"));

	// ▼ ファイル名ボックスは一回り小さいフォントにして上下に余白を作る。
	//   また「ＭＳ Ｐゴシック」は円記号(¥)グリフでバックスラッシュを描画してしまうため、
	//   実際に「\」として表示される「Meiryo UI」を使う(日本語フォルダ名も表示可能)。
	hfontFileName = CreateFont(-DpiScaled(FONT_SIZE - 2, dpiScale), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FF_DONTCARE, TEXT("Meiryo UI"));

	// ▼ 圧縮レベルボタン(JPEG品質/PNG圧縮)の位置を先に確定しておく
	//   (保存ボタンの位置が、これとの隙間で決まるため)
	const int LEVEL_BTN_X = MARGIN + DpiScaled(190, dpiScale);
	const int LEVEL_BTN_WIDTH = DpiScaled(68, dpiScale);   // 60→68 (右端を8px拡張)

	// ▼ 保存ボタンの横位置だけ先に確定 (ファイル名ボックス幅の計算に必要なため)
	const int ROW_HEIGHT_SCALED = DpiScaled(ROW_HEIGHT, dpiScale);
	const int SAVE_BTN_WIDTH = DpiScaled(50, dpiScale);
	const int SAVE_BTN_GAP = DpiScaled(7, dpiScale);   // 15→7 (保存ボタンの位置は変えず、隙間だけ半分に)
	const int SAVE_BTN_X = LEVEL_BTN_X + LEVEL_BTN_WIDTH + SAVE_BTN_GAP;
	// ボタン本体の右端(参照BOXの右端もここに揃える)
	const int CONTENT_RIGHT_ADJ = SAVE_BTN_X + SAVE_BTN_WIDTH;

	// 1段目: ファイル名 (枠 + 参照ボタン)
	const int BROWSE_BTN_WIDTH = DpiScaled(24, dpiScale);
	const int FILENAME_ROW_H = DpiScaled(20, dpiScale);

	// 外枠(境界線)の座標を保存しておき、WM_PAINTで描画する
	g_fileNameBoxRect.left = MARGIN;
	g_fileNameBoxRect.top = 0;
	g_fileNameBoxRect.right = CONTENT_RIGHT_ADJ - BROWSE_BTN_WIDTH;
	g_fileNameBoxRect.bottom = FILENAME_ROW_H;

	// Editは枠より1px内側に配置し、境界線が見えるようにする
	hwndFileName = CreateWindowEx(0, TEXT("EDIT"), TEXT(""),
		WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_NOHIDESEL,
		g_fileNameBoxRect.left + 1, g_fileNameBoxRect.top + 1,
		(g_fileNameBoxRect.right - g_fileNameBoxRect.left) - 2, FILENAME_ROW_H - 2,
		hwnd, (HMENU)IDC_FILENAME, hInst, NULL);
	SetWindowFont(hwndFileName, hfontFileName, TRUE);
	SetWindowTheme(hwndFileName, L"DarkMode_Explorer", NULL);
	Edit_LimitText(hwndFileName, MAX_PATH - 1);
	SetWindowText(hwndFileName, szSaveFileName);

	// 参照ボタン(フォルダ選択) ※ファイル名ボックスと同じ配色になるようオーナードロー化
	hwndBrowse = CreateWindowEx(0, TEXT("BUTTON"), TEXT("…"),
		WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		g_fileNameBoxRect.right, 0, BROWSE_BTN_WIDTH, FILENAME_ROW_H,
		hwnd, (HMENU)IDC_BROWSE, hInst, NULL);
	SetWindowFont(hwndBrowse, hfontControls, TRUE);
	SetWindowSubclass(hwndBrowse, ParamButtonSubclassProc, 6, (DWORD_PTR)&g_hoverBrowse);   // ← 追加

	// ▼ ④ レイアウト並び替え(3段構成)
	const int Y0 = DpiScaled(20, dpiScale);   // 1段目(ファイル名)の直下 = 2段目の開始位置
	const int Y1 = Y0 + DpiScaled(ROW_HEIGHT, dpiScale);   // 3段目(チェックボックス)の開始位置

	// 2段目: 解像度・拡張子・圧縮レベル(保存ボタンとぶつからない範囲に収める)
	// ▼ プルダウンではなく、クリックでポップアップメニューが出る「パラメータボタン」方式
	g_ui_resolution_index = 0;
	g_ui_format_index = ImageCodec.FormatNameToIndex(szSaveFormat);
	if (g_ui_format_index < 0) g_ui_format_index = 0;
	g_ui_jpeg_level = g_jpeg_level;
	g_ui_png_level = g_png_level;

	hwndResolution = CreateWindowEx(0, TEXT("BUTTON"), g_resolutions[g_ui_resolution_index].label,
		WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | BS_NOTIFY,
		MARGIN, Y0, DpiScaled(120, dpiScale), DpiScaled(ROW_HEIGHT, dpiScale),
		hwnd, (HMENU)IDC_RESOLUTION, hInst, NULL);
	SetWindowFont(hwndResolution, hfontControls, TRUE);
	SetWindowSubclass(hwndResolution, ParamButtonSubclassProc, 1, (DWORD_PTR)&g_hoverResolution);

	hwndFormat = CreateWindowEx(0, TEXT("BUTTON"), szSaveFormat,
		WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | BS_NOTIFY,
		MARGIN + DpiScaled(120, dpiScale), Y0, DpiScaled(70, dpiScale), DpiScaled(ROW_HEIGHT, dpiScale),
		hwnd, (HMENU)IDC_FORMAT, hInst, NULL);
	SetWindowFont(hwndFormat, hfontControls, TRUE);
	SetWindowSubclass(hwndFormat, ParamButtonSubclassProc, 2, (DWORD_PTR)&g_hoverFormat);

	TCHAR szJpegText[8]; wsprintf(szJpegText, TEXT("%d"), g_ui_jpeg_level);
	hwndJpegLevel = CreateWindowEx(0, TEXT("BUTTON"), szJpegText,
		WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | BS_NOTIFY,
		LEVEL_BTN_X, Y0, LEVEL_BTN_WIDTH, DpiScaled(ROW_HEIGHT, dpiScale),
		hwnd, (HMENU)IDC_JPEGLEVEL, hInst, NULL);
	SetWindowFont(hwndJpegLevel, hfontControls, TRUE);
	SetWindowSubclass(hwndJpegLevel, ParamButtonSubclassProc, 3, (DWORD_PTR)&g_hoverJpeg);

	TCHAR szPngText[4]; wsprintf(szPngText, TEXT("%d"), g_ui_png_level);
	hwndPngLevel = CreateWindowEx(0, TEXT("BUTTON"), szPngText,
		WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | BS_NOTIFY,
		LEVEL_BTN_X, Y0, LEVEL_BTN_WIDTH, DpiScaled(ROW_HEIGHT, dpiScale),
		hwnd, (HMENU)IDC_PNGLEVEL, hInst, NULL);
	SetWindowFont(hwndPngLevel, hfontControls, TRUE);
	SetWindowSubclass(hwndPngLevel, ParamButtonSubclassProc, 4, (DWORD_PTR)&g_hoverPng);

	// フォーマットに応じて、JPEG品質ボタン/PNG圧縮ボタンのどちらかだけを表示
	bool isJpegInit = lstrcmpi(szSaveFormat, TEXT("jpeg")) == 0;
	bool isPngInit = lstrcmpi(szSaveFormat, TEXT("png")) == 0;
	ShowWindow(hwndJpegLevel, isJpegInit ? SW_SHOW : SW_HIDE);
	ShowWindow(hwndPngLevel, isPngInit ? SW_SHOW : SW_HIDE);

	// 3段目: インターレース解除 (レイヤー配置が不要になったため、レイヤー追加ボタンは廃止)
	const int ROW3_RIGHT = LEVEL_BTN_X + LEVEL_BTN_WIDTH;   // 圧縮レベルの右端と揃える
	const int DEINTERLACE_WIDTH = ROW3_RIGHT - MARGIN;
	const int ROW_VGAP = DpiScaled(5, dpiScale);   // 2段目との間に5pxの余白

	hwndDeinterlace = CreateWindowEx(0, TEXT("BUTTON"), TEXT("インターレース解除"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		MARGIN, Y1 + ROW_VGAP, DEINTERLACE_WIDTH, DpiScaled(ROW_HEIGHT, dpiScale),
		hwnd, (HMENU)IDC_DEINTERLACE, hInst, NULL);
	SetWindowFont(hwndDeinterlace, hfontControls, TRUE);
	// ▼ ビジュアルスタイルを無効化し、WM_CTLCOLORSTATICの色指定(白文字)が効くようにする
	SetWindowTheme(hwndDeinterlace, L" ", L" ");
	Button_SetCheck(hwndDeinterlace, g_deinterlace_enabled ? BST_CHECKED : BST_UNCHECKED);

	// ▼ イースターエッグ用オーバーレイ表示 (チェックボックスの右側〜保存ボタン手前の空きスペースに配置)
	const int EGG_OVERLAY_X = MARGIN + DpiScaled(110, dpiScale);   // チェックボックス文字の右側あたりから
	const int EGG_OVERLAY_RIGHT = ROW3_RIGHT;                       // チェックボックス領域の右端(保存ボタン手前)
	const int EGG_OVERLAY_WIDTH = EGG_OVERLAY_RIGHT - EGG_OVERLAY_X;

	g_hwndEasterEggOverlay = CreateWindowEx(0, TEXT("STATIC"), TEXT(""),
		WS_CHILD | SS_CENTER | SS_CENTERIMAGE,   // 初期状態は非表示(WS_VISIBLEを付けない)
		EGG_OVERLAY_X, Y1 + ROW_VGAP, EGG_OVERLAY_WIDTH, DpiScaled(ROW_HEIGHT, dpiScale),
		hwnd, (HMENU)IDC_EASTEREGG_OVERLAY, hInst, NULL);
	SetWindowFont(g_hwndEasterEggOverlay, hfontControls, TRUE);


	// ▼ 保存ボタンの高さ・Y座標は、3段目の下端が確定した後(ここ)で計算する
	const int addLayerBottom = Y1 + ROW_VGAP + DpiScaled(ROW_HEIGHT, dpiScale);
	const int SAVE_BTN_Y = Y0 + DpiScaled(5, dpiScale);
	const int SAVE_BTN_HEIGHT = addLayerBottom - SAVE_BTN_Y;

	// 保存ボタン: 2段目・3段目にまたがるスロットいっぱいに配置
	hwndCapture = CreateWindowEx(0, TEXT("BUTTON"), TEXT("保存"),
		WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		SAVE_BTN_X, SAVE_BTN_Y, SAVE_BTN_WIDTH, SAVE_BTN_HEIGHT,
		hwnd, (HMENU)IDC_CAPTURE, hInst, NULL);
	// ※ スロット(Y0から高さROW_HEIGHT*2)の中央に、84%サイズのボタンを配置しています。
	SetWindowSubclass(hwndCapture, ParamButtonSubclassProc, 5, (DWORD_PTR)&g_hoverCapture);   // ← 追加


	g_edit_handle = host->create_edit_handle();
	host->register_window_client(TSMemoryDisplayName, hwnd);
	// ※ フィルタプラグインの登録(register_filter_plugin)は、
	//    rendering_scene_video方式への移行に伴い不要になったため廃止しました。
	g_panel_hwnd = hwnd;
}

EXTERN_C __declspec(dllexport) bool InitializePlugin(DWORD) { return true; }

EXTERN_C __declspec(dllexport) void UninitializePlugin()
{
	// ini書き込み処理は全て削除 (保存ボタン押下時に既に書き込み済みのため)
	// ここでコントロールから値を取得しようとすると、
	// 既に破棄されたウィンドウから取得することになり、壊れた値で上書きしてしまう

	if (hfontControls != NULL) {
		DeleteObject(hfontControls);
		hfontControls = NULL;
	}
	if (hfontSaveCaption != NULL) {
		DeleteObject(hfontSaveCaption);
		hfontSaveCaption = NULL;
	}
	if (hfontFileName != NULL) {
		DeleteObject(hfontFileName);
		hfontFileName = NULL;
	}
	if (g_editBrush != NULL) {
		DeleteObject(g_editBrush);
		g_editBrush = NULL;
	}
}

COMMON_PLUGIN_TABLE g_common_table = { L"TSMemory Capture", L"TSMemory Capture Utility ver.1.0" };
EXTERN_C __declspec(dllexport) COMMON_PLUGIN_TABLE* GetCommonPluginTable(void) { return &g_common_table; }
