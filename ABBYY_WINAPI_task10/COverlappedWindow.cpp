#include "COverlappedWindow.h"
#include <Windows.h>
#include <iostream>
#include "resource.h"
#include <Commctrl.h>
#include "wordcountDLL.h"

COverlappedWindow::COverlappedWindow()
{

}

COverlappedWindow::~COverlappedWindow()
{

}

LRESULT __stdcall COverlappedWindow::windowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_ERASEBKGND) {
		return true;
	}

	if (message == WM_NCCREATE) {
		COverlappedWindow* window = reinterpret_cast<COverlappedWindow*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
		::SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG>(window));
	} else {
		COverlappedWindow* window = reinterpret_cast<COverlappedWindow*>(::GetWindowLongPtr(handle, GWLP_USERDATA));
		switch (message) {
		case WM_CREATE:
			window->OnCreate(handle);
			break;
		case WM_SIZE:
			window->OnSize(lParam);
			break;
		case WM_CTLCOLOREDIT:
			::DeleteObject(window->backBrush);
			window->backBrush = (HBRUSH)window->OnCtlColorEdit(wParam);
			return (LRESULT)window->backBrush;
		case WM_SETFOCUS:
			window->OnSetFocus();
			break;
		case WM_COMMAND:
			window->OnCommand(wParam, lParam);
			break;
		case WM_CLOSE:
			if (!window->OnClose()) {
				return 0;
			}

			break;
		case WM_DESTROY:
			window->OnDestroy();
			break;
		default:
			break;
		}
	}

	return DefWindowProc(handle, message, wParam, lParam);

}

bool COverlappedWindow::RegisterClass(HINSTANCE instance)
{
	WNDCLASSEX windowClass;

	::ZeroMemory(&windowClass, sizeof(windowClass));
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = COverlappedWindow::windowProc;
	windowClass.hIcon = (HICON) ::LoadImage(instance, L"pain.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_SHARED);
	windowClass.hInstance = instance;
	windowClass.lpszClassName = L"COverlappedWindow";
	windowClass.hCursor = LoadCursor(NULL, IDC_CROSS);
	windowClass.lpszMenuName = L"MainMenu";

	return ::RegisterClassEx(&windowClass) != 0;
}

bool COverlappedWindow::Create(HINSTANCE instance)
{
	hInst = instance;

	wchar_t windowName[32];
	if (::LoadString(instance, IDS_WINDOW_NAME, windowName, 64)) {
		//pain
	}

	COverlappedWindow::handle = ::CreateWindowEx(0,
		L"COverlappedWindow",
		windowName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		instance,
		this);

	return COverlappedWindow::handle != 0;
}

void setFont(HWND hwndEdit, int fontSize);

void COverlappedWindow::OnCreate(HWND handle) {
	changed = false;

	//creating edit
	RECT rect;
	::GetClientRect(handle, &rect);

	hwndEdit = CreateWindowEx(
		0, L"EDIT",   // predefined class 
		NULL,         // no window title 
		WS_CHILD | WS_VISIBLE | WS_VSCROLL |
		ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
		0, 0, 0, 0,   // set size in WM_SIZE message 
		handle,         // parent window 
		(HMENU)ID_EDITCHILD,   // edit control ID 
		(HINSTANCE)GetWindowLong(handle, GWL_HINSTANCE),
		NULL);        // pointer not needed 


	HRSRC res = ::FindResource(hInst, MAKEINTRESOURCE(IDR_DEFAULTEDITTEXT), L"MYRES");
	HGLOBAL loadedRs = ::LoadResource(hInst, res);
	LPVOID pointerToRes = ::LockResource(loadedRs);
	DWORD size = ::SizeofResource(hInst, res);
	wchar_t* text = (wchar_t*)pointerToRes;

	if (!::SetWindowText(hwndEdit, text)) {
		//pain
	}

	setFont(hwndEdit, applyedProperties.fontSize);

	haccel = ::LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_ACCELERATOR1));
}

void COverlappedWindow::OnSize(LPARAM lParam) {
	// Make the edit control the size of the window's client area. 
	MoveWindow(hwndEdit,
		0, 0,                  // starting x- and y-coordinates 
		LOWORD(lParam),        // width of client area 
		HIWORD(lParam),        // height of client area 
		TRUE);                 // repaint window 
}

void COverlappedWindow::OnSetFocus() {
	SetFocus(hwndEdit);
}

bool getFileName(HWND handle, LPWSTR fileName) {
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = handle;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = L"txt";

	return GetSaveFileName(&ofn);
}

bool writeToFile(LPWSTR fileName, int textLen, LPTSTR lString) {
	HANDLE hFile = ::CreateFile(
		fileName,
		GENERIC_WRITE,
		0,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (INVALID_HANDLE_VALUE == hFile) {
		return false;
	}

	DWORD writtenBytesNumber;
	if (!::WriteFile(hFile, lString, textLen * sizeof(char), &writtenBytesNumber, 0)) {
		return false;
	}

	::CloseHandle(hFile);

	return true;
}

bool COverlappedWindow::saveFile() {
	int textLen = ::GetWindowTextLength(hwndEdit);
	LPTSTR lString = new wchar_t[textLen + 1];
	::GetWindowText(hwndEdit, lString, textLen + 1);
	if (!lString) {
		return false;
	}

	wchar_t fileName[MAX_PATH] = L"";
	if (!getFileName(handle, fileName))
	{
		delete lString;
		return false;
	}

	if (!writeToFile(fileName, textLen, lString)) {
		delete lString;
		return false;
	}

	delete lString;
	return true;
}

bool COverlappedWindow::OnClose() {
	if (!changed) {
		return true;
	}

	int textLen = ::GetWindowTextLength(hwndEdit);

	if (textLen == 0) {
		return true;
	}

	int msgboxID = MessageBox(
		NULL,
		(LPCWSTR)L"Сохранить изменения?\n",
		(LPCWSTR)L"Сохранение\n",
		MB_ICONWARNING | MB_YESNOCANCEL | MB_DEFBUTTON3
	);


	bool exit;
	switch (msgboxID)
	{
	case IDYES:
	{
		exit = saveFile();
		break;
	}
	case IDNO:
		exit = true;
		break;
	case IDCANCEL:
		exit = false;
		break;
	}

	return exit;
}

void COverlappedWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
	if (HIWORD(wParam) == EN_CHANGE) {
		changed = true;
	}

	switch (LOWORD(wParam)) {
	case ID_ACCELERATOR_FAST_QUIT:
		::PostMessage(handle, WM_QUIT, (WPARAM)0, (LPARAM)0);
		return;
	case IDM_SAVE:
		saveFile();
		return;
	case IDM_EXIT:
		::PostMessage(handle, WM_CLOSE, (WPARAM)0, (LPARAM)0);
		break;
	case ID_VIEW_SETTINGS:
		if (settingsHandle != 0) break;
		OnSettings();
		break;
	case ID_COUNT:
		DWORD len = SendMessage(hwndEdit, WM_GETTEXTLENGTH, 0, 0);
		wchar_t* buffer = new wchar_t[len + 1];
		::SendMessage(hwndEdit, WM_GETTEXT, (WPARAM)len + 1, (LPARAM)buffer);

		wchar_t b[32];

		_itow_s(WordsCount(buffer), b, 10);

		MessageBox(0,
			b,
			L"Word Count",
			MB_OK);

		free(buffer);
		break;
	}
}

void COverlappedWindow::OnSettings() {
	settingsHandle = ::CreateDialog(::GetModuleHandle(0), MAKEINTRESOURCE(IDD_DIALOG1),
		handle, COverlappedWindow::settingsProc);

	::ShowWindow(settingsHandle, SW_SHOW);
}

void COverlappedWindow::OnDestroy()
{
	::DeleteObject(backBrush);
	::DestroyAcceleratorTable(haccel);
	::PostQuitMessage(0);
}

void COverlappedWindow::Show(int cmdShow)
{
	::ShowWindow(handle, cmdShow);
}


LRESULT COverlappedWindow::OnCtlColorEdit(WPARAM wParam) {
	if (isChecked) {
		::SetTextColor((HDC)wParam, previewProperties.fontColor);
		::SetBkColor((HDC)wParam, previewProperties.background);

		return (LRESULT)CreateSolidBrush(previewProperties.background);
	} else {
		::SetTextColor((HDC)wParam, applyedProperties.fontColor);
		::SetBkColor((HDC)wParam, applyedProperties.background);

		return (LRESULT)CreateSolidBrush(applyedProperties.background);

	}
}

//////////////dialog

BOOL __stdcall COverlappedWindow::settingsProc(HWND settingsHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	COverlappedWindow* window = reinterpret_cast<COverlappedWindow*>(::GetWindowLongPtr(::GetParent(settingsHandle),
		GWLP_USERDATA));

	switch (message) {
	case WM_INITDIALOG:
	{
		window->OnSettingsInit(settingsHandle);
		return 1;
	}
	case WM_HSCROLL:
	{
		window->OnSettingsScroll(settingsHandle);
		return 1;
	}

	case WM_COMMAND:
	{
		switch (LOWORD(wParam)) {
		case IDCANCEL:
		{
			window->OnSettingsCancel(settingsHandle);
			return 1;
		}
		case IDOK:
		{
			window->OnSettingsOk(settingsHandle);
			return 1;
		}
		case IDC_BUTTON_FONT_COLOR:
		{
			window->OnSettingsColor(settingsHandle, IDC_BUTTON_FONT_COLOR);
			return 1;
		}
		case IDC_BUTTON_BACK_COLOR:
		{
			window->OnSettingsColor(settingsHandle, IDC_BUTTON_BACK_COLOR);
			return 1;
		}
		case IDC_CHECK1:
		{
			window->OnSettingsCheck(settingsHandle);
			return 1;
		}

		}
	}
	}
	return 0;
}

void setFont(HWND hwndEdit, int fontSize) {
	HFONT font = (HFONT)::SendMessage(hwndEdit, WM_GETFONT, 0, 0);
	if (!font) {
		font = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
	}

	LOGFONT lg;
	::GetObject(font, sizeof(LOGFONT), &lg);
	lg.lfHeight = fontSize;

	::DeleteObject(font);
	font = ::CreateFontIndirect(&lg);
	::SendMessage(hwndEdit, WM_SETFONT, (WPARAM)font, 1);
}

void setVisible(HWND handle, int visible) {
	DWORD dwStyle = GetWindowLong(handle, GWL_EXSTYLE);
	dwStyle = dwStyle | WS_EX_LAYERED;
	SetWindowLong(handle, GWL_EXSTYLE, dwStyle);
	::SetLayeredWindowAttributes(handle, 0, visible, LWA_ALPHA);
}


void COverlappedWindow::OnSettingsInit(HWND settingsHandle) {
	HWND transpSlider = ::GetDlgItem(settingsHandle, IDC_SLIDER2);
	HWND fontSlider = ::GetDlgItem(settingsHandle, IDC_SLIDER1);
	HWND checkBox = ::GetDlgItem(settingsHandle, IDC_CHECK1);

	::SendMessage(transpSlider, TBM_SETRANGE, 1, MAKELONG(0, 255));
	::SendMessage(fontSlider, TBM_SETRANGE, 1, MAKELONG(8, 72));

	::SendMessage(checkBox, BM_SETCHECK, isChecked ? BST_CHECKED : BST_UNCHECKED, 0);

	::SendMessage(fontSlider, TBM_SETPOS, 1, applyedProperties.fontSize);
	::SendMessage(transpSlider, TBM_SETPOS, 1, applyedProperties.visible);
}

void COverlappedWindow::OnSettingsOk(HWND settingsHandle) {
	applyedProperties = previewProperties;
	setVisible(handle, applyedProperties.visible);
	::setFont(hwndEdit, applyedProperties.fontSize);
	::DestroyWindow(settingsHandle);
	this->settingsHandle = 0;
}

void COverlappedWindow::OnSettingsCancel(HWND settingsHandle) {
	previewProperties = applyedProperties;
	setFont(hwndEdit, applyedProperties.fontSize);
	setVisible(handle, applyedProperties.visible);
	::InvalidateRect(hwndEdit, NULL, 1);
	::DestroyWindow(settingsHandle);
	this->settingsHandle = 0;
}

void COverlappedWindow::OnSettingsColor(HWND settingsHandle, UINT buttonID) {
	CHOOSECOLOR cc;
	// Array of custom colors
	static COLORREF acrCustClr[16];

	// Prepare structure
	ZeroMemory(&cc, sizeof(cc));
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = settingsHandle;
	cc.lpCustColors = (LPDWORD)acrCustClr;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;

	if (::ChooseColor(&cc) == TRUE)
	{
		switch (buttonID) {
		case IDC_BUTTON_FONT_COLOR:
		{
			previewProperties.fontColor = cc.rgbResult;
			break;
		}
		case IDC_BUTTON_BACK_COLOR:
		{
			previewProperties.background = cc.rgbResult;
			break;
		}
		}
	}

	if (isChecked) {
		::InvalidateRect(hwndEdit, NULL, 1);
	}
}

void COverlappedWindow::OnSettingsScroll(HWND settingsHandle) {
	previewProperties.fontSize = ::SendMessage(::GetDlgItem(settingsHandle, IDC_SLIDER1), TBM_GETPOS, 0, 0);
	previewProperties.visible = ::SendMessage(::GetDlgItem(settingsHandle, IDC_SLIDER2), TBM_GETPOS, 0, 0);

	if (isChecked) {
		setFont(hwndEdit, previewProperties.fontSize);
		setVisible(handle, previewProperties.visible);
		::InvalidateRect(hwndEdit, NULL, 1);
	}
}

void COverlappedWindow::OnSettingsCheck(HWND settingsHandle) {
	HWND checkBox = ::GetDlgItem(settingsHandle, IDC_CHECK1);
	isChecked = ::SendMessage(checkBox, BM_GETCHECK, 0, 0);

	if (isChecked) {
		setFont(hwndEdit, previewProperties.fontSize);
		setVisible(handle, previewProperties.visible);
	} else {
		setFont(hwndEdit, applyedProperties.fontSize);
		setVisible(handle, applyedProperties.visible);
	}

	::InvalidateRect(hwndEdit, NULL, 1);
}
