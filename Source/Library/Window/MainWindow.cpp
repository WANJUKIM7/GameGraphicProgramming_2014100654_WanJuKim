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

    HRESULT MainWindow::Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow, _In_ PCWSTR pszWindowName)
    {
        RECT rc = { 0, 0, 800, 600 };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

        HRESULT hr =  initialize(hInstance, nCmdShow, pszWindowName, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr);

        if (FAILED(hr))
            return E_FAIL;

        // Clip Mouse.
        GetClientRect(m_hWnd, &rc);

        POINT p1, p2;
        p1.x = rc.left;
        p1.y = rc.top;
        p2.x = rc.right;
        p2.y = rc.bottom;

        ClientToScreen(m_hWnd, &p1);
        ClientToScreen(m_hWnd, &p2);

        rc.left = p1.x;
        rc.top = p1.y - 25; //Question : GetWindowRect()라는 함수는 윈도우 창 크기를 가져오면 창 부분을 깔끔하게 가져올 수 있다.
        rc.right = p2.x;
        rc.bottom = p2.y;

        ClipCursor(&rc);

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::GetWindowClassName
      Summary:  Returns the name of the window class
      Returns:  PCWSTR
                  Name of the window class
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    
    PCWSTR MainWindow::GetWindowClassName() const
    {   
        //이런 게 객체 지향할 때 필요한 거다. 윈도우가 여러 개일 수도 있고...
        return L"Window ClassName";
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

    LRESULT MainWindow::HandleMessage(_In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
    {
        PAINTSTRUCT ps;
        HDC hdc;

        switch (uMsg)
        {
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }
        case WM_PAINT:
        {
            hdc = BeginPaint(m_hWnd, &ps);
            //FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
            EndPaint(m_hWnd, &ps);
            break;
        }
        case WM_INPUT:
        {
            UINT dataSize = 0u;
            GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER));
            if (dataSize > 0u)
            {
                std::unique_ptr<BYTE[]> rawdata = std::make_unique<BYTE[]>(dataSize);
                if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, rawdata.get(), &dataSize, sizeof(RAWINPUTHEADER)))
                {
                    RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(rawdata.get());
                    if (raw->header.dwType == RIM_TYPEMOUSE)
                    {
                        m_mouseRelativeMovement.X = raw->data.mouse.lLastX;
                        m_mouseRelativeMovement.Y = raw->data.mouse.lLastY;
                    }
                }
            }

            return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
        }
        case WM_KEYDOWN:
        {
            if (wParam == 'W')
            {
                m_directions.bFront = true;
            }
            else if (wParam == 'S')
            {
                m_directions.bBack = true;
            }

            if (wParam == 'D')
            {
                m_directions.bRight = true;
                m_directions.bLeft = false;
            }
            else if (wParam == 'A')
            {
                m_directions.bLeft = true;
                m_directions.bRight = false;;
            }
            
            if (wParam == VK_SPACE)
            {
                m_directions.bUp = true;
                m_directions.bDown = false;
            }
            else if (wParam == VK_SHIFT)
            {
                m_directions.bDown = true;
                m_directions.bUp = false;;
            }
            break;
        }
        case WM_KEYUP:
        {
            m_directions.bFront = false;
            m_directions.bBack = false;
            m_directions.bRight = false;
            m_directions.bLeft = false;
            m_directions.bUp = false;
            m_directions.bDown = false;
            break;
        }
        return 0;

        default:
            return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
        }
        return TRUE;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::GetDirections

      Summary:  Returns the keyboard direction input

      Returns:  const DirectionsInput&
                  Keyboard direction input
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    const DirectionsInput& MainWindow::GetDirections() const
    {
        return m_directions;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::GetMouseRelativeMovement

      Summary:  Returns the mouse relative movement

      Returns:  const MouseRelativeMovement&
                  Mouse relative movement
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    const MouseRelativeMovement& MainWindow::GetMouseRelativeMovement() const
    {
        return m_mouseRelativeMovement;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::ResetMouseMovement

      Summary:  Reset the mouse relative movement to zero
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void MainWindow::ResetMouseMovement()
    {
        ZeroMemory(&m_mouseRelativeMovement, sizeof(MouseRelativeMovement));
    }
}