#pragma once
#include <Windows.h>

#define ID_EDITCHILD 100

class COverlappedWindow {
public:
	COverlappedWindow();
	~COverlappedWindow();

	static bool RegisterClass(HINSTANCE instance);

	bool Create(HINSTANCE instance);

	void Show(int cmdShow);

	HACCEL haccel;
	HWND handle;
	HWND settingsHandle;
	HBRUSH backBrush;
protected:
	void OnCreate(HWND handle);
	void OnSize(LPARAM lParam);
	void OnSetFocus();
	void OnDestroy();
	void OnCommand(WPARAM wParam, LPARAM lParam);
	bool OnClose();
	void OnSettings();
	LRESULT OnCtlColorEdit(WPARAM wParam);

	//dialog
	// Dialog window functions
	void OnSettingsInit(HWND settingsHandle);
	void OnSettingsOk(HWND settingsHandle);
	void OnSettingsCancel(HWND settingsHandle);
	void OnSettingsColor(HWND settingsHandle, UINT buttonID);
	void OnSettingsScroll(HWND settingsHandle);
	void OnSettingsCheck(HWND settingsHandle);

private:
	bool changed;
	HWND hwndEdit;
	HINSTANCE hInst;
	static LRESULT __stdcall windowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
	static BOOL __stdcall settingsProc(HWND settingsHandle, UINT message, WPARAM wParam, LPARAM lParam);

	bool saveFile();

	HANDLE iconHandle;

	struct EditorProperties {
		COLORREF background = RGB(255, 255, 255);
		COLORREF fontColor = RGB(0, 0, 0);
		UINT fontSize = 20;
		BYTE visible = 255;
	} applyedProperties, previewProperties;

	bool isChecked;

};
