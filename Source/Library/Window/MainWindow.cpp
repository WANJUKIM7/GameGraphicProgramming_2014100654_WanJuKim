#include "Window/MainWindow.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::Initialize
      Summary:  Initializes main window
      Args:     HINSTANCE hInstance
                  Handle to the instance
                INT nCmdShow
                    Is a flag that says whether the main application window
                    will be minimized, maximized, or shown normally
                PCWSTR pszWindowName
                    The window name
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT MainWindow::Initialize(HINSTANCE hInstance, INT nCmdShow, PCWSTR pszWindowName)
    {
        RECT rc = { 0, 0, 800, 600 };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

        return initialize(hInstance, nCmdShow, pszWindowName, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::GetWindowClassName
      Summary:  Returns the name of the window class
      Returns:  PCWSTR
                  Name of the window class
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    
    PCWSTR MainWindow::GetWindowClassName() const
    {   
        //�̷� �� ��ü ������ �� �ʿ��� �Ŵ�. �����찡 ���� ���� ���� �ְ�...
        return m_pszWindowName;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::HandleMessage
      Summary:  Handles the messages
      Args:     UINT uMessage
                  Message code
                WPARAM wParam
                    Additional data the pertains to the message
                LPARAM lParam
                    Additional data the pertains to the message
      Returns:  LRESULT
                  Integer value that your program returns to Windows
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(m_hWnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
            EndPaint(m_hWnd, &ps);
        }
        return 0;

        default:
            return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
        }
        return TRUE;
    }
}