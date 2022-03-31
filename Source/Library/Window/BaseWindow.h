/*+===================================================================
  File:      BASEWINDOW.H
  Summary:   BaseWindow header file contains declarations of the
             base class of all windows used in the library.
  Classes: BaseWindow<DerivedType>
  © 2022 Kyung Hee University
===================================================================+*/
#pragma once

#include "Common.h"

namespace library
{
    /*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
      Class:    BaseWindow
      Summary:  An abstract base class from specific window
      Methods:  WindowProc
                  The window procedure of the window
                Initialize
                    Purely virtual function that initializes window
                GetWindowClassName
                    Purely virtual function that returns the name of
                    the window class
                HandleMessage
                    Purely virtual function that that handles the
                    messages
                GetWindow
                    Getter for the handle to the window
                BaseWindow
                    Constructor.
                ~BaseWindow
                    Destructor.
    C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/

    template <class DerivedType>
    class BaseWindow
    {
    public:
        static LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

        BaseWindow();
        BaseWindow(const BaseWindow& rhs) = delete;
        BaseWindow(BaseWindow&& rhs) = delete;
        BaseWindow& operator=(const BaseWindow& rhs) = delete;
        BaseWindow& operator=(BaseWindow&& rhs) = delete;
        virtual ~BaseWindow() = default;

        virtual HRESULT Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow, _In_ PCWSTR pszWindowName) = 0;
        virtual PCWSTR GetWindowClassName() const = 0;
        virtual LRESULT HandleMessage(_In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) = 0;

        HWND GetWindow() const;

    protected:
        HRESULT initialize(
            _In_ HINSTANCE hInstance,
            _In_ INT nCmdShow,
            _In_ PCWSTR pszWindowName,
            _In_ DWORD dwStyle,
            _In_opt_ INT x = CW_USEDEFAULT,
            _In_opt_ INT y = CW_USEDEFAULT,
            _In_opt_ INT nWidth = CW_USEDEFAULT,
            _In_opt_ INT nHeight = CW_USEDEFAULT,
            _In_opt_ HWND hWndParent = nullptr,
            _In_opt_ HMENU hMenu = nullptr
        );

        HINSTANCE m_hInstance;
        HWND m_hWnd;
        LPCWSTR m_pszWindowName;
    };

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
        Method:   BaseWindow<DerivedType>::WindowProc
        Summary:  Defines the behavior of the window—its appearance, how
                it interacts with the user, and so forth
        Args:     HWND hWnd
                    Handle to the window
                UINT uMessage
                    Message code
                WPARAM wParam
                    Additional data the pertains to the message
                LPARAM lParam
                    Additional data the pertains to the message
        Modifies: [m_hWnd].
        Returns:  LRESULT
                    Integer value that your program returns to Windows
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    template<class DerivedType>
    inline LRESULT BaseWindow<DerivedType>::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        DerivedType* pThis = NULL;
        
        if (uMsg == WM_NCCREATE)
        {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            pThis = (DerivedType*)pCreate->lpCreateParams;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);

            pThis->m_hWnd = hWnd;
        }
        else
        {
            pThis = (DerivedType*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        }
        if (pThis)
        {
            return pThis->HandleMessage(uMsg, wParam, lParam);
        }
        else
        {
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
        Method:   BaseWindow<DerivedType>::BaseWindow
        Summary:  Constructor
        Modifies: [m_hInstance, m_hWnd, m_pszWindowName].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    template<class DerivedType>
    inline BaseWindow<DerivedType>::BaseWindow()
        : m_hInstance(nullptr), m_hWnd(nullptr), m_pszWindowName(L"Default")
    {
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
        Method:   BaseWindow<DerivedType>::GetWindow()
        Summary:  Returns the handle to the window
        Returns:  HWND
                    The handle to the window
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    template<class DerivedType>
    inline HWND BaseWindow<DerivedType>::GetWindow() const
    {
        return m_hWnd;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   BaseWindow<DerivedType>::initialize
      Summary:  Registers the window class and creates a window
      Args:     HINSTANCE hInstance
                  Handle to the instance
                INT nCmdShow
                  Is a flag that says whether the main application window
                  will be minimized, maximized, or shown normally
                PCWSTR pszWindowName
                  The window name
                DWORD dwStyle
                  The style of the window being created
                INT x
                  The initial horizontal position of the window
                INT y
                  The initial vertical position of the window
                INT nWidth
                  The width, in device units, of the window
                INT nHeight
                  The height, in device units, of the window
                HWND hWndParent
                  A handle to the parent or owner window of the window
                  being created
                HMENU hMenu
                  A handle to a menu, or specifies a child-window
                  identifier depending on the window style
      Modifies: [m_hInstance, m_pszWindowName, m_hWnd].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    template<class DerivedType>
    inline HRESULT BaseWindow<DerivedType>::initialize(HINSTANCE hInstance, INT nCmdShow,
        PCWSTR pszWindowName, DWORD dwStyle, INT x, INT y, INT nWidth, INT nHeight,
        HWND hWndParent, HMENU hMenu)
    {
        // Register class
        WNDCLASSEX wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WindowProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = GetWindowClassName();
        wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
        if (!RegisterClassEx(&wcex))
            return E_FAIL;

        // Create window
        /*struct StateInfo {
            HWND hWnd;
        };

        StateInfo* pState = new (std::nothrow) StateInfo;

        if (pState == NULL)
        {
            return 0;
        }*/

        /*RECT rc = { 0, 0, 800, 600 };
        AdjustWindowRect(&rc, dwStyle, FALSE);*/
        hWndParent = CreateWindow(GetWindowClassName(), pszWindowName,
            dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, this);
        //QUESTION : x,y,nWidth,nHeight 인수 받아주면 직사각형 나오는데, 인수 받아주는 이유는?
        // →HRESULT MainWindow::Initialize(HINSTANCE hInstance, INT nCmdShow, PCWSTR pszWindowName) 여기서 수정하는 사람도 있는데 별 상관은 없을듯?
        //QUESTION : CreateWindow()맨 마지막 변수 pState했다가 안돼서 this넣으니까 되네. 도대체 무슨 차이일까?
        // →윈도우 나 써달라는거임. pState는 다른 윈도우.
        //딱히 밖에서 수정하는 것도 아니고 ㄷㄷ.
        //TIP : WindowClassName이랑 WindowName이랑 다르다.
        if (!hWndParent)
            return E_FAIL;

        ShowWindow(hWndParent, nCmdShow);

        m_hWnd = hWndParent;
        m_hInstance = hInstance;
        m_pszWindowName = pszWindowName;

        return S_OK;
    }
}