#include "Game/Game.h"

namespace library
{
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Game::Game
	  Summary:  Constructor
	  Args:     PCWSTR pszGameName
				  Name of the game
	  Modifies: [m_pszGameName, m_mainWindow, m_renderer].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	Game::Game(_In_ PCWSTR pszGameName)
		: m_renderer(std::make_unique<library::Renderer>())
		, m_mainWindow(std::make_unique<library::MainWindow>())
		, m_pszGameName(pszGameName)
	{
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
		Method:   Game::Initialize
		Summary:  Initializes the components of the game
		Args:     HINSTANCE hInstance
					Handle to the instance
				INT nCmdShow
					Is a flag that says whether the main application window
					will be minimized, maximized, or shown normally
		Modifies: [m_mainWindow, m_renderer].
		Returns:  HRESULT
				Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Game::Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow)
	{
		HRESULT hr = m_mainWindow.get()->Initialize(hInstance, nCmdShow, m_pszGameName);
		if (FAILED(hr))
			return E_FAIL;

		hr = m_renderer.get()->Initialize(m_mainWindow.get()->GetWindow());
		if (FAILED(hr))
			return E_FAIL;

		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Game::Run
	  Summary:  Runs the game loop
	  Returns:  INT
				  Status code to return to the operating system
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	INT Game::Run()
	{
		// Update our time
		LARGE_INTEGER startingTime = {};
		LARGE_INTEGER endingTime = {};
		LARGE_INTEGER elapsedMicroseconds = {};
		LARGE_INTEGER frequency = {};
		float deltaTime(0.0f);

		QueryPerformanceFrequency(&frequency);
		QueryPerformanceCounter(&startingTime);

		MSG msg = { 0 };
		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				QueryPerformanceCounter(&endingTime);
				elapsedMicroseconds.QuadPart = endingTime.QuadPart - startingTime.QuadPart;	//second 아님. tick임.
				elapsedMicroseconds.QuadPart *= 1000000;
				elapsedMicroseconds.QuadPart /= frequency.QuadPart;	//이제야 second.
				deltaTime = (float)elapsedMicroseconds.QuadPart / 1000000.0f;	//micro 원상복귀. 위에서 곱해줘서 다시 나눠준 거임.
				
				WCHAR szDebugMessage[64];  // 배열의 크기는 메시지의 길이에 따라 조정하시면 됩니다
				swprintf_s(szDebugMessage, L"deltaTime: %f\n", deltaTime);
				OutputDebugString(szDebugMessage);

				//deltaTime = elapsedMicroseconds.QuadPart;
				//us = 10^-6 s
				m_renderer.get()->Update(deltaTime);
				m_renderer.get()->Render();

				QueryPerformanceFrequency(&frequency);
				QueryPerformanceCounter(&startingTime);
			}
		}

		return static_cast<INT>(msg.wParam);
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Game::GetGameName
	  Summary:  Returns the name of the game
	  Returns:  PCWSTR
				  Name of the game
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	PCWSTR Game::GetGameName() const
	{
		return m_pszGameName;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Game::GetWindow

	  Summary:  Returns the main window

	  Returns:  std::unique_ptr<MainWindow>&
				  The main window
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	std::unique_ptr<MainWindow>& Game::GetWindow()
	{
		return m_mainWindow;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Game::GetRenderer

	  Summary:  Returns the renderer

	  Returns:  std::unique_ptr<Renderer>&
				  The renderer
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	std::unique_ptr<Renderer>& Game::GetRenderer()
	{
		return m_renderer;
	}
}