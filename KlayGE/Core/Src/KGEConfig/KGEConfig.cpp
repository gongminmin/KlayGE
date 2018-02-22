// KGEConfig.cpp
// KlayGE Configuration tool implement file
// Ver 3.11.0
// Copyright(C) Minmin Gong, 2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// First release (2010.8.15)
//
// CHANGE LIST
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KFL/Util.hpp>

#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <commctrl.h>
#include <sstream>

#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable" // Ignore unused variable (mpl_assertion_in_line_xxx) in boost
#endif
#include <boost/lexical_cast.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif

#include "resource.h"

using namespace KlayGE;

ContextCfg cfg;
bool save_cfg = false;

INT_PTR CALLBACK Graphics_Tab_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK Audio_Tab_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK Input_Tab_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK Show_Tab_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

enum nTabDialogs
{
	GRAPHICS_TAB,
	AUDIO_TAB,
	INPUT_TAB,
	SHOW_TAB,
	NTABS
};

HWND hTab = nullptr; // Handle to tab control.
HWND hTabDlg[NTABS] = {0}; // Array of handle to tab dialogs.
int iCurSelTab = 0;

HWND hOKButton;
HWND hCancelButton;

std::basic_string<TCHAR> tab_dlg_titles[] =
{
	TEXT("Graphics"),
	TEXT("Audio"),
	TEXT("Input"),
	TEXT("Show")
};

std::basic_string<TCHAR> tab_dlg_ids[] =
{
	TEXT("GraphicsTab"),
	TEXT("AudioTab"),
	TEXT("InputTab"),
	TEXT("ShowTab")
};

DLGPROC tab_dlg_procs[] =
{
	Graphics_Tab_DlgProc,
	Audio_Tab_DlgProc,
	Input_Tab_DlgProc,
	Show_Tab_DlgProc
};

uint32_t constexpr paper_white_candidates[] =
{
	80,
	100,
	200,
	400
};

uint32_t constexpr max_lum_candidates[] =
{
	80,
	100,
	400,
	1000
};

INT_PTR CALLBACK Graphics_Tab_DlgProc(HWND hDlg, UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HWND hFactoryCombo = GetDlgItem(hDlg, IDC_FACTORY_COMBO);
			HMODULE mod_d3d11 = LoadLibraryEx(TEXT("d3d11.dll"), nullptr, 0);
			if (mod_d3d11)
			{
				SendMessage(hFactoryCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("D3D11")));
				FreeLibrary(mod_d3d11);
			}
			HMODULE mod_d3d12 = LoadLibraryEx(TEXT("d3d12.dll"), nullptr, 0);
			if (mod_d3d12)
			{
				SendMessage(hFactoryCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("D3D12")));
				FreeLibrary(mod_d3d12);
			}
			HMODULE mod_gl = LoadLibraryEx(TEXT("opengl32.dll"), nullptr, 0);
			if (mod_gl)
			{
				SendMessage(hFactoryCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("OpenGL")));
				FreeLibrary(mod_gl);
			}
			HMODULE mod_gles2 = LoadLibraryEx(TEXT("libGLESv2.dll"), nullptr, 0);
			if (!mod_gles2)
			{
				mod_gles2 = LoadLibraryEx(TEXT("libGLES20.dll"), nullptr, 0);
				if (!mod_gles2)
				{
					mod_gles2 = LoadLibraryEx(TEXT("atioglxx.dll"), nullptr, 0);
				}
			}
			if (mod_gles2)
			{
				SendMessage(hFactoryCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("OpenGLES")));
				FreeLibrary(mod_gles2);
			}

			TCHAR buf[256];
			int n = static_cast<int>(SendMessage(hFactoryCombo, CB_GETCOUNT, 0, 0));
			int sel = 0;
			for (int i = 0; i < n; ++ i)
			{
				SendMessage(hFactoryCombo, CB_GETLBTEXT, i, reinterpret_cast<LPARAM>(buf));

				std::string str;
				Convert(str, buf);
				if (str == cfg.render_factory_name)
				{
					sel = i;
				}
			}
			SendMessage(hFactoryCombo, CB_SETCURSEL, sel, 0);
		}
		{
			HWND hResCombo = GetDlgItem(hDlg, IDC_RES_COMBO);
			SendMessage(hResCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("4096x2304 (16:9)")));
			SendMessage(hResCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("3200x2000 (16:10)")));
			SendMessage(hResCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("3000x2000 (3:2)")));
			SendMessage(hResCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("2560x1600 (16:10)")));
			SendMessage(hResCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("2160x1440 (3:2)")));
			SendMessage(hResCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("1920x1280 (3:2)")));
			SendMessage(hResCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("1920x1200 (16:10)")));
			SendMessage(hResCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("1920x1080 (16:9)")));
			SendMessage(hResCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("1680x1050 (16:10)")));
			SendMessage(hResCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("1600x900 (16:9)")));
			SendMessage(hResCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("1440x900 (16:10)")));
			SendMessage(hResCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("1366x768 (16:9)")));
			SendMessage(hResCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("1280x1024 (5:4)")));
			SendMessage(hResCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("1280x960 (4:3)")));
			SendMessage(hResCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("1280x800 (16:10)")));
			SendMessage(hResCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("1280x720 (16:9)")));
			SendMessage(hResCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("1024x768 (4:3)")));
			SendMessage(hResCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("800x600 (4:3)")));

			TCHAR buf[256];
			int n = static_cast<int>(SendMessage(hResCombo, CB_GETCOUNT, 0, 0));
			int sel = 0;
			for (int i = 0; i < n; ++ i)
			{
				SendMessage(hResCombo, CB_GETLBTEXT, i, reinterpret_cast<LPARAM>(buf));

				std::string const res = boost::lexical_cast<std::string>(cfg.graphics_cfg.width) + 'x'
					+ boost::lexical_cast<std::string>(cfg.graphics_cfg.height) + ' ';

				std::string str;
				Convert(str, buf);
				if (0 == str.find(res))
				{
					sel = i;
				}
			}
			SendMessage(hResCombo, CB_SETCURSEL, sel, 0);
		}
		{
			HWND hClrFmtCombo = GetDlgItem(hDlg, IDC_CLR_FMT_COMBO);
			SendMessage(hClrFmtCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("ARGB8")));
			SendMessage(hClrFmtCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("ABGR8")));
			SendMessage(hClrFmtCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("A2BGR10")));
			SendMessage(hClrFmtCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("ABGR16F")));

			int sel = 0;
			switch (cfg.graphics_cfg.color_fmt)
			{
			case EF_ARGB8:
				sel = 0;
				break;

			case EF_ABGR8:
				sel = 1;
				break;

			case EF_A2BGR10:
				sel = 2;
				break;

			default:
				sel = 0;
				break;
			}
			SendMessage(hClrFmtCombo, CB_SETCURSEL, sel, 0);
		}
		{
			HWND hDepthFmtCombo = GetDlgItem(hDlg, IDC_DEPTH_FMT_COMBO);
			SendMessage(hDepthFmtCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("None")));
			SendMessage(hDepthFmtCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("D16")));
			SendMessage(hDepthFmtCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("D24S8")));
			SendMessage(hDepthFmtCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("D32F")));

			int sel = 2;
			switch (cfg.graphics_cfg.depth_stencil_fmt)
			{
			case EF_D16:
				sel = 1;
				break;

			case EF_D24S8:
				sel = 2;
				break;

			case EF_D32F:
				sel = 3;
				break;

			default:
				sel = 0;
				break;
			}
			SendMessage(hDepthFmtCombo, CB_SETCURSEL, sel, 0);
		}
		{
			HWND hAACombo = GetDlgItem(hDlg, IDC_AA_COMBO);
			SendMessage(hAACombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("No")));
			SendMessage(hAACombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("2")));
			SendMessage(hAACombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("4")));
			SendMessage(hAACombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("8")));

			int sel = 0;
			switch (cfg.graphics_cfg.sample_count)
			{
			case 1:
				sel = 0;
				break;

			case 2:
				sel = 1;
				break;

			case 4:
				sel = 2;
				break;

			case 8:
				sel = 3;
				break;

			default:
				sel = 0;
				break;
			}
			SendMessage(hAACombo, CB_SETCURSEL, sel, 0);
		}
		{
			HWND hFSCombo = GetDlgItem(hDlg, IDC_FS_COMBO);
			SendMessage(hFSCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("Yes")));
			SendMessage(hFSCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("No")));
			SendMessage(hFSCombo, CB_SETCURSEL, cfg.graphics_cfg.full_screen ? 0 : 1, 0);
		}
		{
			HWND hSyncCombo = GetDlgItem(hDlg, IDC_SYNC_COMBO);
			SendMessage(hSyncCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("0")));
			SendMessage(hSyncCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("1")));
			SendMessage(hSyncCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("2")));
			SendMessage(hSyncCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("4")));

			int sel = 0;
			switch (cfg.graphics_cfg.sync_interval)
			{
			case 0:
				sel = 0;
				break;

			case 1:
				sel = 1;
				break;

			case 2:
				sel = 2;
				break;

			case 4:
				sel = 3;
				break;

			default:
				sel = 0;
				break;
			}
			SendMessage(hSyncCombo, CB_SETCURSEL, sel, 0);
		}
		{
			HWND hHDRCombo = GetDlgItem(hDlg, IDC_HDR_COMBO);
			SendMessage(hHDRCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("Yes")));
			SendMessage(hHDRCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("No")));
			SendMessage(hHDRCombo, CB_SETCURSEL, cfg.graphics_cfg.hdr ? 0 : 1, 0);
		}
		{
			HWND hPPAACombo = GetDlgItem(hDlg, IDC_PPAA_COMBO);
			SendMessage(hPPAACombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("Yes")));
			SendMessage(hPPAACombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("No")));
			SendMessage(hPPAACombo, CB_SETCURSEL, cfg.graphics_cfg.ppaa ? 0 : 1, 0);
		}
		{
			HWND hStereoCombo = GetDlgItem(hDlg, IDC_STEREO_COMBO);
			SendMessage(hStereoCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("None")));
			SendMessage(hStereoCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("Color anaglyph: Red Cyan")));
			SendMessage(hStereoCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("Color anaglyph: Yellow Blue")));
			SendMessage(hStereoCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("Color anaglyph: Green Red")));
			SendMessage(hStereoCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("LCD shutter")));
			SendMessage(hStereoCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("Horizontal interlacing")));
			SendMessage(hStereoCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("Vertical interlacing")));
			SendMessage(hStereoCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("Horizontal")));
			SendMessage(hStereoCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("Vertical")));
			SendMessage(hStereoCombo, CB_SETCURSEL, cfg.graphics_cfg.stereo_method, 0);
		}
		{
			HWND hStereoSepEdit = GetDlgItem(hDlg, IDC_STEREO_SEP_EDIT);

			std::ostringstream oss;
			oss.precision(2);
			oss << std::fixed << cfg.graphics_cfg.stereo_separation;
			std::basic_string<TCHAR> str;
			Convert(str, oss.str());

			SetWindowText(hStereoSepEdit, str.c_str());
		}
		{
			HWND hOutputCombo = GetDlgItem(hDlg, IDC_OUTPUT_COMBO);
			SendMessage(hOutputCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("sRGB")));
			SendMessage(hOutputCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("HDR10")));

			int sel = 0;
			switch (cfg.graphics_cfg.display_output_method)
			{
			case DOM_HDR10:
				sel = 1;
				break;

			case DOM_sRGB:
			default:
				sel = 0;
				break;
			}
			SendMessage(hOutputCombo, CB_SETCURSEL, sel, 0);
		}
		{
			int sel = -1;
			HWND hPaperWhiteCombo = GetDlgItem(hDlg, IDC_PAPER_WHITE_COMBO);
			for (uint32_t i = 0; i < std::size(paper_white_candidates); ++ i)
			{
				std::basic_string<TCHAR> str;
				Convert(str, boost::lexical_cast<std::string>(paper_white_candidates[i]));
				SendMessage(hPaperWhiteCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(str.c_str()));

				if ((sel == -1) && (cfg.graphics_cfg.paper_white <= paper_white_candidates[i]))
				{
					sel = static_cast<int>(i);
				}
			}
			SendMessage(hPaperWhiteCombo, CB_SETCURSEL, sel, 0);
		}
		{
			int sel = -1;
			HWND hMaxLumCombo = GetDlgItem(hDlg, IDC_MAX_LUM_COMBO);
			for (uint32_t i = 0; i < std::size(max_lum_candidates); ++ i)
			{
				std::basic_string<TCHAR> str;
				Convert(str, boost::lexical_cast<std::string>(max_lum_candidates[i]));
				SendMessage(hMaxLumCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(str.c_str()));

				if ((sel == -1) && (cfg.graphics_cfg.display_max_luminance <= max_lum_candidates[i]))
				{
					sel = static_cast<int>(i);
				}
			}
			SendMessage(hMaxLumCombo, CB_SETCURSEL, sel, 0);
		}
		return TRUE;

	default:
		return FALSE;
	}
}

INT_PTR CALLBACK Audio_Tab_DlgProc(HWND hDlg, UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HWND hFactoryCombo = GetDlgItem(hDlg, IDC_FACTORY_COMBO);
			HMODULE mod_al = LoadLibraryEx(TEXT("OpenAL32.dll"), nullptr, 0);
			if (mod_al)
			{
				SendMessage(hFactoryCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("OpenAL")));
				FreeLibrary(mod_al);
			}
			HMODULE mod_xaudio = LoadLibraryEx(TEXT("XAudio2_8.dll"), nullptr, 0);
			if (mod_xaudio)
			{
				SendMessage(hFactoryCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("XAudio")));
				FreeLibrary(mod_xaudio);
			}

			TCHAR buf[256];
			int n = static_cast<int>(SendMessage(hFactoryCombo, CB_GETCOUNT, 0, 0));
			int sel = 0;
			for (int i = 0; i < n; ++ i)
			{
				SendMessage(hFactoryCombo, CB_GETLBTEXT, i, reinterpret_cast<LPARAM>(buf));

				std::string str;
				Convert(str, buf);
				if (str == cfg.audio_factory_name)
				{
					sel = i;
				}
			}
			SendMessage(hFactoryCombo, CB_SETCURSEL, sel, 0);
		}
		return TRUE;

	default:
		return FALSE;
	}
}

INT_PTR CALLBACK Input_Tab_DlgProc(HWND hDlg, UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HWND hFactoryCombo = GetDlgItem(hDlg, IDC_FACTORY_COMBO);
			{
				SendMessage(hFactoryCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("MsgInput")));
			}

			TCHAR buf[256];
			int n = static_cast<int>(SendMessage(hFactoryCombo, CB_GETCOUNT, 0, 0));
			int sel = 0;
			for (int i = 0; i < n; ++ i)
			{
				SendMessage(hFactoryCombo, CB_GETLBTEXT, i, reinterpret_cast<LPARAM>(buf));

				std::string str;
				Convert(str, buf);
				if (str == cfg.input_factory_name)
				{
					sel = i;
				}
			}
			SendMessage(hFactoryCombo, CB_SETCURSEL, sel, 0);
		}
		return TRUE;

	default:
		return FALSE;
	}
}

INT_PTR CALLBACK Show_Tab_DlgProc(HWND hDlg, UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HWND hFactoryCombo = GetDlgItem(hDlg, IDC_FACTORY_COMBO);
			SendMessage(hFactoryCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("DShow")));

			TCHAR buf[256];
			int n = static_cast<int>(SendMessage(hFactoryCombo, CB_GETCOUNT, 0, 0));
			int sel = 0;
			for (int i = 0; i < n; ++ i)
			{
				SendMessage(hFactoryCombo, CB_GETLBTEXT, i, reinterpret_cast<LPARAM>(buf));

				std::string str;
				Convert(str, buf);
				if (str == cfg.show_factory_name)
				{
					sel = i;
				}
			}
			SendMessage(hFactoryCombo, CB_SETCURSEL, sel, 0);
		}
		return TRUE;

	default:
		return FALSE;
	}
}

INT_PTR CreateTabDialogs(HWND hWnd, HINSTANCE hInstance)
{
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);

	hTab = CreateWindowEx(
				WS_EX_CONTROLPARENT,
				WC_TABCONTROL,
				TEXT(""),
				WS_TABSTOP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_CHILD | TCS_FOCUSONBUTTONDOWN,
				rcClient.left + 5,
				rcClient.top + 5,
				rcClient.right - rcClient.left - 10,
				rcClient.bottom - rcClient.top - 45,
				hWnd,
				nullptr,
				hInstance,
				nullptr);

	ShowWindow(hTab, SW_SHOWNORMAL);

	HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);   
	::SendMessage(hTab, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), 1);

	TCITEM tci;
	tci.mask       = TCIF_TEXT;
	tci.iImage     = -1;

	RECT rc;
	GetClientRect(hTab, &rc);
	int ret = TabCtrl_AdjustRect(hTab, false, &rc);
	KFL_UNUSED(ret);
	rc.top += 20;

	for (int i = 0; i < NTABS; ++ i)
	{
		tci.pszText    = const_cast<TCHAR*>(tab_dlg_titles[i].c_str());
		tci.cchTextMax = static_cast<int>(tab_dlg_titles[i].size());
		ret = TabCtrl_InsertItem(hTab, i, &tci);
		KFL_UNUSED(ret);

		hTabDlg[i] = CreateDialogParam(hInstance, tab_dlg_ids[i].c_str(), hTab, tab_dlg_procs[i], 0);
		MoveWindow(hTabDlg[i], rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, FALSE);
	}

	ShowWindow(hTabDlg[GRAPHICS_TAB], SW_SHOWNORMAL);

	return FALSE;
}

INT_PTR CreateButtons(HWND hWnd, HINSTANCE hInstance)
{
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);

	hOKButton = CreateWindowEx(
				WS_EX_CONTROLPARENT,
				WC_BUTTON,
				TEXT("OK"),
				WS_TABSTOP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
				rcClient.right - 72 * 2 - 15,
				rcClient.bottom - 27,
				72,
				22,
				hWnd,
				nullptr,
				hInstance,
				nullptr);
	hCancelButton = CreateWindowEx(
				WS_EX_CONTROLPARENT,
				WC_BUTTON,
				TEXT("Cancel"),
				WS_TABSTOP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
				rcClient.right - 72 - 5,
				rcClient.bottom - 27,
				72,
				22,
				hWnd,
				nullptr,
				hInstance,
				nullptr);

	HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);   
	::SendMessage(hOKButton, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), 1);
	::SendMessage(hCancelButton, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), 1);

	return FALSE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		if (reinterpret_cast<HWND>(lParam) == hOKButton)
		{
			{
				{
					HWND hFactoryCombo = GetDlgItem(hTabDlg[GRAPHICS_TAB], IDC_FACTORY_COMBO);
					int n = static_cast<int>(SendMessage(hFactoryCombo, CB_GETCURSEL, 0, 0));
					TCHAR buf[256];
					SendMessage(hFactoryCombo, CB_GETLBTEXT, n, reinterpret_cast<LPARAM>(buf));
					Convert(cfg.render_factory_name, buf);
				}
				{
					HWND hResCombo = GetDlgItem(hTabDlg[GRAPHICS_TAB], IDC_RES_COMBO);
					int n = static_cast<int>(SendMessage(hResCombo, CB_GETCURSEL, 0, 0));
					TCHAR buf[256];
					SendMessage(hResCombo, CB_GETLBTEXT, n, reinterpret_cast<LPARAM>(buf));
					std::string str;
					Convert(str, buf);
					std::string::size_type p = str.find('x');
					std::istringstream(str.substr(0, p)) >> cfg.graphics_cfg.width;
					std::istringstream(str.substr(p + 1, str.size())) >> cfg.graphics_cfg.height;
				}
				{
					HWND hClrFmtCombo = GetDlgItem(hTabDlg[GRAPHICS_TAB], IDC_CLR_FMT_COMBO);
					int n = static_cast<int>(SendMessage(hClrFmtCombo, CB_GETCURSEL, 0, 0));
					switch (n)
					{
					case 0:
						cfg.graphics_cfg.color_fmt = EF_ARGB8;
						break;

					case 1:
						cfg.graphics_cfg.color_fmt = EF_ABGR8;
						break;

					case 2:
						cfg.graphics_cfg.color_fmt = EF_A2BGR10;
						break;

					default:
						cfg.graphics_cfg.color_fmt = EF_ARGB8;
						break;
					}
				}
				{
					HWND hDepthFmtCombo = GetDlgItem(hTabDlg[GRAPHICS_TAB], IDC_DEPTH_FMT_COMBO);
					int n = static_cast<int>(SendMessage(hDepthFmtCombo, CB_GETCURSEL, 0, 0));
					switch (n)
					{
					case 1:
						cfg.graphics_cfg.depth_stencil_fmt = EF_D16;
						break;

					case 2:
						cfg.graphics_cfg.depth_stencil_fmt = EF_D24S8;
						break;

					case 3:
						cfg.graphics_cfg.depth_stencil_fmt = EF_D32F;
						break;

					default:
						cfg.graphics_cfg.depth_stencil_fmt = EF_Unknown;
						break;
					}
				}
				{
					HWND hAACombo = GetDlgItem(hTabDlg[GRAPHICS_TAB], IDC_AA_COMBO);
					int n = static_cast<int>(SendMessage(hAACombo, CB_GETCURSEL, 0, 0));
					switch (n)
					{
					case 0:
						cfg.graphics_cfg.sample_count = 1;
						cfg.graphics_cfg.sample_quality = 0;
						break;

					case 1:
						cfg.graphics_cfg.sample_count = 2;
						cfg.graphics_cfg.sample_quality = 0;
						break;

					case 2:
						cfg.graphics_cfg.sample_count = 4;
						cfg.graphics_cfg.sample_quality = 0;
						break;

					case 3:
						cfg.graphics_cfg.sample_count = 8;
						cfg.graphics_cfg.sample_quality = 0;
						break;

					default:
						cfg.graphics_cfg.sample_count = 1;
						cfg.graphics_cfg.sample_quality = 0;
						break;
					}
				}
				{
					HWND hFSCombo = GetDlgItem(hTabDlg[GRAPHICS_TAB], IDC_FS_COMBO);
					int n = static_cast<int>(SendMessage(hFSCombo, CB_GETCURSEL, 0, 0));
					cfg.graphics_cfg.full_screen = (0 == n) ? 1 : 0;
				}
				{
					HWND hSyncCombo = GetDlgItem(hTabDlg[GRAPHICS_TAB], IDC_SYNC_COMBO);
					int n = static_cast<int>(SendMessage(hSyncCombo, CB_GETCURSEL, 0, 0));
					switch (n)
					{
					case 0:
						cfg.graphics_cfg.sync_interval = 0;
						break;

					case 1:
						cfg.graphics_cfg.sync_interval = 1;
						break;

					case 2:
						cfg.graphics_cfg.sync_interval = 2;
						break;

					case 3:
						cfg.graphics_cfg.sync_interval = 4;
						break;

					default:
						cfg.graphics_cfg.sync_interval = 0;
						break;
					}
				}
				{
					HWND hHDRCombo = GetDlgItem(hTabDlg[GRAPHICS_TAB], IDC_HDR_COMBO);
					int n = static_cast<int>(SendMessage(hHDRCombo, CB_GETCURSEL, 0, 0));
					cfg.graphics_cfg.hdr = (0 == n) ? 1 : 0;
				}
				{
					HWND hStereoCombo = GetDlgItem(hTabDlg[GRAPHICS_TAB], IDC_STEREO_COMBO);
					int n = static_cast<int>(SendMessage(hStereoCombo, CB_GETCURSEL, 0, 0));
					cfg.graphics_cfg.stereo_method = static_cast<StereoMethod>(n);
				}
				{
					HWND hStereoSepEdit = GetDlgItem(hTabDlg[GRAPHICS_TAB], IDC_STEREO_SEP_EDIT);
					TCHAR buf[256];
					GetWindowText(hStereoSepEdit, buf, static_cast<int>(std::size(buf)));
					std::basic_stringstream<TCHAR>(buf) >> cfg.graphics_cfg.stereo_separation;
				}
				{
					HWND hOutputCombo = GetDlgItem(hTabDlg[GRAPHICS_TAB], IDC_OUTPUT_COMBO);
					int n = static_cast<int>(SendMessage(hOutputCombo, CB_GETCURSEL, 0, 0));
					cfg.graphics_cfg.display_output_method = static_cast<DisplayOutputMethod>(n);
				}
				{
					HWND hPaperWhiteCombo = GetDlgItem(hTabDlg[GRAPHICS_TAB], IDC_PAPER_WHITE_COMBO);
					int n = static_cast<int>(SendMessage(hPaperWhiteCombo, CB_GETCURSEL, 0, 0));
					cfg.graphics_cfg.paper_white = paper_white_candidates[n];
				}
				{
					HWND hMaxLumCombo = GetDlgItem(hTabDlg[GRAPHICS_TAB], IDC_MAX_LUM_COMBO);
					int n = static_cast<int>(SendMessage(hMaxLumCombo, CB_GETCURSEL, 0, 0));
					cfg.graphics_cfg.display_max_luminance = max_lum_candidates[n];
				}
			}
			{
				HWND hFactoryCombo = GetDlgItem(hTabDlg[AUDIO_TAB], IDC_FACTORY_COMBO);
				int n = static_cast<int>(SendMessage(hFactoryCombo, CB_GETCURSEL, 0, 0));
				TCHAR buf[256];
				SendMessage(hFactoryCombo, CB_GETLBTEXT, n, reinterpret_cast<LPARAM>(buf));
				Convert(cfg.audio_factory_name, buf);
			}
			{
				HWND hFactoryCombo = GetDlgItem(hTabDlg[INPUT_TAB], IDC_FACTORY_COMBO);
				int n = static_cast<int>(SendMessage(hFactoryCombo, CB_GETCURSEL, 0, 0));
				TCHAR buf[256];
				SendMessage(hFactoryCombo, CB_GETLBTEXT, n, reinterpret_cast<LPARAM>(buf));
				Convert(cfg.input_factory_name, buf);
			}
			{
				HWND hFactoryCombo = GetDlgItem(hTabDlg[SHOW_TAB], IDC_FACTORY_COMBO);
				int n = static_cast<int>(SendMessage(hFactoryCombo, CB_GETCURSEL, 0, 0));
				TCHAR buf[256];
				SendMessage(hFactoryCombo, CB_GETLBTEXT, n, reinterpret_cast<LPARAM>(buf));
				Convert(cfg.show_factory_name, buf);
			}

			save_cfg = true;
			::PostQuitMessage(0);
		}
		else if (reinterpret_cast<HWND>(lParam) == hCancelButton)
		{
			save_cfg = false;
			::PostQuitMessage(0);
		}
		break;

	case WM_CLOSE:
		save_cfg = false;
		::PostQuitMessage(0);
		return 0;

	case WM_NOTIFY:
		switch (reinterpret_cast<LPNMHDR>(lParam)->code)
		{
		case TCN_SELCHANGE:
			{
				int iSelTab = TabCtrl_GetCurSel(hTab);
				if (iCurSelTab != iSelTab)
				{
					ShowWindow(hTabDlg[iCurSelTab], SW_HIDE);
					ShowWindow(hTabDlg[iSelTab], SW_SHOWNORMAL);
					iCurSelTab = iSelTab;
				}
				return TRUE;
			}
		}
		break;
	}

	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool UIConfiguration(HINSTANCE hInstance)
{
	::WNDCLASSEX wc;
	wc.cbSize			= sizeof(wc);
	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon			= ::LoadIcon(hInstance, TEXT("IDI_KLAYGEICON"));
	wc.hCursor			= ::LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground	= reinterpret_cast<HBRUSH>(COLOR_WINDOW);
	wc.lpszMenuName		= nullptr;
	wc.lpszClassName	= TEXT("KGEConfig");
	wc.hIconSm			= wc.hIcon;
	::RegisterClassEx(&wc);

	int cx = ::GetSystemMetrics(SM_CXSCREEN);
	int cy = ::GetSystemMetrics(SM_CYSCREEN);
	int width = 420;
	int height = 600;

	HWND hWnd = ::CreateWindow(wc.lpszClassName, TEXT("KlayGE Configuration Tool"),
		WS_CAPTION | WS_SYSMENU, (cx - width) / 2, (cy - height) / 2,
		width, height, 0, 0, hInstance, nullptr);

	::ShowWindow(hWnd, SW_SHOWNORMAL);
	::UpdateWindow(hWnd);

	CreateTabDialogs(hWnd, hInstance);
	CreateButtons(hWnd, hInstance);

	MSG msg = {};
	while (::GetMessage(&msg, nullptr, 0, 0))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	return save_cfg;
}

#if defined(KLAYGE_COMPILER_MSVC)
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPSTR /*lpszCmdLine*/, _In_ int /*nCmdShow*/)
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpszCmdLine*/, int /*nCmdShow*/)
#endif
{
	std::string cfg_path = ResLoader::Instance().Locate("KlayGE.cfg");
	Context::Instance().LoadCfg(cfg_path);
	cfg = Context::Instance().Config();

	if (UIConfiguration(hInstance))
	{
		Context::Instance().Config(cfg);
		Context::Instance().SaveCfg(cfg_path);
	}

	Context::Destroy();

	return 0;
}
