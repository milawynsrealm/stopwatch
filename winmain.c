/*
    A simple stopwatch application
    Copyright (C) 2014  Lee Schroeder

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "stopwatch.h"

static HWND mainDlgHwnd;
static HINSTANCE mainAppInstance;
static int hDay, hHour, hMin, hSec, hMSec;
static BOOL isRunning = FALSE;

int ResourceMessageBox(HWND hWnd, int nText, int nCaption, UINT uType)
{
    WCHAR nFinalText[MAX_BUFFER_SIZE], nFinalCaption[MAX_BUFFER_SIZE];

    LoadStringW(GetModuleHandle(NULL), nText, nFinalText, MAX_BUFFER_SIZE);
    LoadStringW(GetModuleHandle(NULL), nCaption, nFinalCaption, MAX_BUFFER_SIZE);

    return MessageBoxW(hWnd, nFinalText, nFinalCaption, uType);
}

VOID StartStopwatch(HWND hWnd)
{
    WCHAR startButton[MAX_BUFFER_SIZE];

    /* Toggles the current state of the stopwatch itself */
    if (isRunning)
        isRunning = FALSE;
    else
        isRunning = TRUE;

    /* Sets the string of the start/stop button based on if the timer is running or not */
    LoadStringW(mainAppInstance, (isRunning ? IDS_CONTROL_PAUSE : IDS_CONTROL_START), startButton, MAX_BUFFER_SIZE);
    SetWindowTextW(GetDlgItem(hWnd, IDC_START_TIMER), startButton);

    /* Enables the buttons since they will now be usable */
    EnableWindow(GetDlgItem(hWnd, IDC_RESET_TIMER), TRUE);
    EnableWindow(GetDlgItem(hWnd, IDC_COPY_TIMER), TRUE);
}

VOID ResetStopwatch(HWND hWnd)
{
    WCHAR startButton[MAX_BUFFER_SIZE];

    isRunning = FALSE;

    /* Resets the value to zero */
    hDay = hHour = hMin = hSec = hMSec = 0;

    /* Visually resets the values of the timer to zero so
    the user will know when it has been successfully reset */
    SetDlgItemTextW(hWnd, IDC_TIME_MS,   L"0");
    SetDlgItemTextW(hWnd, IDC_TIME_SEC,  L"0");
    SetDlgItemTextW(hWnd, IDC_TIME_MIN,  L"0");
    SetDlgItemTextW(hWnd, IDC_TIME_HOUR, L"0");
    SetDlgItemTextW(hWnd, IDC_TIME_DAY,  L"0");

    /* Resets the string back to "Start" */
    LoadStringW(mainAppInstance, IDS_CONTROL_START, startButton, MAX_BUFFER_SIZE);
    SetWindowTextW(GetDlgItem(hWnd, IDC_START_TIMER), startButton);

    /* Disable these buttons since there is nothing to reset or copy */
    EnableWindow(GetDlgItem(hWnd, IDC_RESET_TIMER), FALSE);
    EnableWindow(GetDlgItem(hWnd, IDC_COPY_TIMER), FALSE);
}

VOID CopyStopwatch(HWND hWnd)
{
    const SIZE_T entrySize = 60;
    WCHAR output[100], tmpHeader[40];
    WCHAR szDay[entrySize];
    WCHAR szHour[entrySize];
    WCHAR szMin[entrySize];
    WCHAR szSec[entrySize];
    WCHAR szMilSec[entrySize];
    HGLOBAL hMem;

    if( !OpenClipboard(hWnd))
        return;

    /* First, empty the clipboard before we begin to use it */
    EmptyClipboard();

    /* Get the formatted text needed to place the content into */
    LoadStringW(mainAppInstance, IDS_COPY, tmpHeader, sizeof(tmpHeader) / sizeof(WCHAR));

    /* Grabs all the information and get it ready for the clipboard */
    GetDlgItemTextW(hWnd, IDC_TIME_DAY, szDay, entrySize);
    GetDlgItemTextW(hWnd, IDC_TIME_HOUR, szHour, entrySize);
    GetDlgItemTextW(hWnd, IDC_TIME_MIN, szMin, entrySize);
    GetDlgItemTextW(hWnd, IDC_TIME_SEC, szSec, entrySize);
    GetDlgItemTextW(hWnd, IDC_TIME_MS, szMilSec, entrySize);

    /* Consolidate the information into on big piece */
    wsprintfW(output, tmpHeader, szDay, szHour, szMin, szSec, szMilSec);

    /* Sort out the memory needed to write to the clipboard */
    hMem = GlobalAlloc(GMEM_MOVEABLE, entrySize);
    memcpy(GlobalLock(hMem), output, entrySize);
    GlobalUnlock(hMem);

    /* Write the final content to the clipboard */
    SetClipboardData(CF_UNICODETEXT, hMem);

    /* Close the clipboard once we're done with it */
    CloseClipboard();
}

VOID RunStopWatch(HWND hWnd)
{
    if (isRunning)
    {
        WCHAR tmpValue[MAX_BUFFER_SIZE];

        /* Increases the time by 1 Millisecond */
        hMSec+=14;

        /* Determines what to do if any time slot reaches a certain time */
        if (hMSec >= 1000)
        {
            hSec++;
            hMSec = 0;
        }
        if (hSec >= 60)
        {
            hMin++;
            hSec = 0;
        }
        if (hMin >= 60)
        {
            hHour++;
            hMin = 0;
        }
        if (hHour >= 24)
        {
            hDay++;
            hHour = 0;
        }

        /* Update each of the text boxes */
        _itow(hMSec, tmpValue, 10);
        SetDlgItemTextW(hWnd, IDC_TIME_MS, tmpValue);
        _itow(hSec, tmpValue, 10);
        SetDlgItemTextW(hWnd, IDC_TIME_SEC, tmpValue);
        _itow(hMin, tmpValue, 10);
        SetDlgItemTextW(hWnd, IDC_TIME_MIN, tmpValue);
        _itow(hHour, tmpValue, 10);
        SetDlgItemTextW(hWnd, IDC_TIME_HOUR, tmpValue);
        _itow(hDay, tmpValue, 10);
        SetDlgItemTextW(hWnd, IDC_TIME_DAY, tmpValue);
    }
}

BOOL CALLBACK StopwatchDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message)
	{
    case WM_INITDIALOG:
    {
        HICON hIcon;

        /* Gets the timer set up so we can actually use it in the program */
        if (SetTimer(hwnd, IDT_MAIN_TIMER, 1, (TIMERPROC)NULL) == 0)
        {
            EndDialog(hwnd, 0);
            ResourceMessageBox(hwnd, IDS_ERROR_TIMER, IDS_EROR_CAPTION, MB_OK | MB_ICONERROR);
        }

        /* Loads the program icon for the main window itself */
        hIcon = LoadImage(mainAppInstance,
                          MAKEINTRESOURCE(IDI_ICON),
                          IMAGE_ICON,
                          GetSystemMetrics(SM_CXSMICON),
                          GetSystemMetrics(SM_CYSMICON),
                          0);
        if(hIcon)
            SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

        /* Since we're just starting out, we need to reset it so it's ready to run */
        ResetStopwatch(hwnd);

        break;
    }
    case WM_CLOSE:
        {
            KillTimer(hwnd, IDT_MAIN_TIMER);
            EndDialog(hwnd, 0);
            break;
        }
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_START_TIMER:
            StartStopwatch(hwnd);
            return FALSE;
        case IDC_RESET_TIMER:
            ResetStopwatch(hwnd);
            break;
        case IDC_COPY_TIMER:
            CopyStopwatch(hwnd);
            break;
        }
        case WM_TIMER:
            switch (wParam)
            {
            case IDT_MAIN_TIMER:
                RunStopWatch(hwnd);
                break;
            }
            break;
    default:
        return FALSE;
	}

    return TRUE;
}

int
WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    mainAppInstance = hInstance;

    if(DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_MAIN), mainDlgHwnd, StopwatchDlgProc) == -1)
    {
        ResourceMessageBox(mainDlgHwnd, IDS_ERROR_DIALOG, IDS_EROR_CAPTION, MB_OK | MB_ICONERROR);
        return -1;
    }

    return 0;
}
