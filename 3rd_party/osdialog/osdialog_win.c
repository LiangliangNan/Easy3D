#include "osdialog.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>


static char *wchar_to_utf8(const wchar_t *s) {
	if (!s) return NULL;
	int len = WideCharToMultiByte(CP_UTF8, 0, s, -1, NULL, 0, NULL, NULL);
	if (!len) return NULL;
	char *r = OSDIALOG_MALLOC(len);
	WideCharToMultiByte(CP_UTF8, 0, s, -1, r, len, NULL, NULL);
	return r;
}

static wchar_t *utf8_to_wchar(const char *s) {
	if (!s) return NULL;
	int len = MultiByteToWideChar(CP_UTF8, 0, s, -1, NULL, 0);
	if (!len) return NULL;
	wchar_t *r = OSDIALOG_MALLOC(len * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, s, -1, r, len);
	return r;
}


int osdialog_message(osdialog_message_level level, osdialog_message_buttons buttons, const char *message) {
	UINT type = MB_APPLMODAL;
	switch (level) {
		default:
		case OSDIALOG_INFO: type |= MB_ICONINFORMATION; break;
		case OSDIALOG_WARNING: type |= MB_ICONWARNING; break;
		case OSDIALOG_ERROR: type |= MB_ICONERROR; break;
	}

	switch (buttons) {
		default:
		case OSDIALOG_OK: type |= MB_OK; break;
		case OSDIALOG_OK_CANCEL: type |= MB_OKCANCEL; break;
		case OSDIALOG_YES_NO: type |= MB_YESNO; break;
	}

	HWND window = GetActiveWindow();
	wchar_t *messageW = utf8_to_wchar(message);
	int result = MessageBoxW(window, messageW, L"", type);
	OSDIALOG_FREE(messageW);

	switch (result) {
		case IDOK:
		case IDYES:
			return 1;
		default:
			return 0;
	}
}


char *osdialog_prompt(osdialog_message_level level, const char *message, const char *text) {
	// TODO
	(void) level;
	(void) message;
	(void) text;
	assert(0);
	return NULL;
}


static INT CALLBACK browseCallbackProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	if (Msg == BFFM_INITIALIZED) {
		SendMessageW(hWnd, BFFM_SETSELECTION, 1, lParam);
	}
	return 0;
}

char *osdialog_file(osdialog_file_action action, const char *path, const char *filename, osdialog_filters *filters) {
	char *result = NULL;

	if (action == OSDIALOG_OPEN_DIR) {
		// open directory dialog
		wchar_t szDir[MAX_PATH] = L"";

		BROWSEINFOW bInfo;
		ZeroMemory(&bInfo, sizeof(bInfo));
		bInfo.hwndOwner = GetActiveWindow();
		// bInfo.pszDisplayName = szDir;
		bInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
		bInfo.iImage = -1;

		// TODO Default paths do not seem to work
		wchar_t *pathW = NULL;
		if (path) {
			pathW = utf8_to_wchar(path);
			bInfo.lpfn = browseCallbackProc;
			bInfo.lParam = (LPARAM) pathW;
		}

		PIDLIST_ABSOLUTE lpItem = SHBrowseForFolderW(&bInfo);
		if (lpItem) {
		  SHGetPathFromIDListW(lpItem, szDir);
		  result = wchar_to_utf8(szDir);
		}
		if (pathW) {
			OSDIALOG_FREE(pathW);
		}
	}
	else {
		// open or save file dialog
		OPENFILENAMEW ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.hwndOwner = GetActiveWindow();
		ofn.lStructSize = sizeof(ofn);
		ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		// Filename
		wchar_t strFile[MAX_PATH] = L"";
		if (filename) {
			wchar_t *filenameW = utf8_to_wchar(filename);
			snwprintf(strFile, MAX_PATH, L"%s", filenameW);
			OSDIALOG_FREE(filenameW);
		}
		ofn.lpstrFile = strFile;
		ofn.nMaxFile = MAX_PATH;

		// Path
		wchar_t *strInitialDir = NULL;
		if (path) {
			strInitialDir = utf8_to_wchar(path);
		}
		ofn.lpstrInitialDir = strInitialDir;

		// Filters
		wchar_t *strFilter = NULL;
		if (filters) {
			char fBuf[4096];
			int fLen = 0;

			for (; filters; filters = filters->next) {
				fLen += snprintf(fBuf + fLen, sizeof(fBuf) - fLen, "%s", filters->name);
				fBuf[fLen++] = '\0';
				for (osdialog_filter_patterns *patterns = filters->patterns; patterns; patterns = patterns->next) {
					fLen += snprintf(fBuf + fLen, sizeof(fBuf) - fLen, "*.%s", patterns->pattern);
					if (patterns->next)
						fLen += snprintf(fBuf + fLen, sizeof(fBuf) - fLen, ";");
				}
				fBuf[fLen++] = '\0';
			}
			fBuf[fLen++] = '\0';

			// Don't use utf8_to_wchar() because this is not a NULL-terminated string.
			strFilter = OSDIALOG_MALLOC(fLen * sizeof(wchar_t));
			MultiByteToWideChar(CP_UTF8, 0, fBuf, fLen, strFilter, fLen);
			ofn.lpstrFilter = strFilter;
			ofn.nFilterIndex = 1;
		}

		BOOL success;
		if (action == OSDIALOG_OPEN) {
			success = GetOpenFileNameW(&ofn);
		}
		else {
			success = GetSaveFileNameW(&ofn);
		}

		// Clean up
		if (strInitialDir) {
			OSDIALOG_FREE(strInitialDir);
		}
		if (strFilter) {
			OSDIALOG_FREE(strFilter);
		}
		if (success) {
			result = wchar_to_utf8(strFile);
		}
	}

	return result;
}


int osdialog_color_picker(osdialog_color *color, int opacity) {
	(void) opacity;
	if (!color)
		return 0;

	CHOOSECOLOR cc;
	ZeroMemory(&cc, sizeof(cc));

	COLORREF c = RGB(color->r, color->g, color->b);
	static COLORREF acrCustClr[16];

	cc.lStructSize = sizeof(cc);
	cc.lpCustColors = (LPDWORD) acrCustClr;
	cc.rgbResult = c;
	cc.Flags = CC_FULLOPEN | CC_ANYCOLOR | CC_RGBINIT;

	if (ChooseColor(&cc)) {
		color->r = GetRValue(cc.rgbResult);
		color->g = GetGValue(cc.rgbResult);
		color->b = GetBValue(cc.rgbResult);
		color->a = 255;
		return 1;
	}

	return 0;
}
