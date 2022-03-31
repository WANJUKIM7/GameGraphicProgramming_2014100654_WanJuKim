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

	Game::Game(PCWSTR pszGameName)
	{
		m_renderer = std::make_unique<library::Renderer>();
		m_mainWindow = std::make_unique<library::MainWindow>();
		m_pszGameName = pszGameName;
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
				m_renderer.get()->Render();
			}
		}

		return 0;
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

}