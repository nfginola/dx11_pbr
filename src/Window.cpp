#include "pch.h"
#include "Window.h"
#include "Utilities.h"

namespace Gino
{
	Window::Window(HINSTANCE hInstance, std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> winProc, int clientWidth, int clientHeight, const std::string& title) :
		m_hInstance(hInstance),
		m_hwnd(nullptr),
		m_title(title),
		m_clientWidth(clientWidth),
		m_clientHeight(clientHeight),
		m_isAlive(true),
		m_isFullscreen(false),
		m_winStyle(WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX),
		m_winExStyle(0),
		m_screenTransitionDetails(ScreenTransitionDetails{ 0, 0, 0, 0 }),
		m_winProc(winProc)
	{
		m_winID = GetNextWindowID();

		RegisterWindow();
		MakeWindow();
		FitToClientDim();

		assert(m_hwnd != NULL);
		ShowWindow(m_hwnd, SW_SHOWDEFAULT);
	}

	Window::~Window()
	{
		DestroyWindow(m_hwnd);
		UnregisterClass(Gino::Utils::StrToWstr(m_winID).c_str(), m_hInstance);
	}

	void Window::PumpMessages() const
	{
		MSG msg = { };
		while (PeekMessageW(&msg, m_hwnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	bool Window::IsAlive() const
	{
		return m_isAlive;
	}

	void Window::RegisterWindow()
	{
		std::wstring classID = Gino::Utils::StrToWstr(m_winID);

		WNDCLASS wc = { 0 };
		wc.lpfnWndProc = WindowProc;												
		wc.hInstance = m_hInstance;					
		wc.lpszClassName = classID.c_str();

		// https://stackoverflow.com/questions/4503506/cursor-style-doesnt-stay-updated
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);	// Standard cursor to load (Fixes bug where the sizing icon is stuck)

		RegisterClass(&wc);
	}

	void Window::MakeWindow()
	{
		std::wstring classID = Gino::Utils::StrToWstr(m_winID);
		std::wstring title = Gino::Utils::StrToWstr(m_title);

		m_hwnd = CreateWindowEx(
			0,								// Default behaviour (optionals)
			classID.c_str(),				// Class name (identifier)
			title.c_str(),					// Window title 
			m_winStyle,						// Window style 
			65,								// Window x-pos
			10,								// Window y-pos 

			// This will be resized to our desired client dimensions later
			CW_USEDEFAULT,					// Default width
			CW_USEDEFAULT,					// Default height

			NULL,							// Parent window (none, we are top level window)
			NULL,							// Menu
			m_hInstance,
			(LPVOID)this					// Passing this pointer so that we can use member function procedure handling
		);
		assert(m_hwnd != NULL);

		// Associate this window with the hwnd
		SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)this);
	}

	void Window::FitToClientDim()
	{
		// Store previous window position
		RECT oldRect{};
		GetWindowRect(m_hwnd, &oldRect);
		LONG prevX = oldRect.left;
		LONG prevY = oldRect.top;

		// We pass a rect with desired client dim
		// Rect is modified with dimensions for the Window that accomodates the client dimensions
		RECT desired_rect{};
		desired_rect.right = m_clientWidth;
		desired_rect.bottom = m_clientHeight;
		assert(AdjustWindowRectEx(&desired_rect, m_winStyle, false, m_winExStyle));

		LONG window_width = desired_rect.right - desired_rect.left;
		LONG window_height = desired_rect.bottom - desired_rect.top;
		assert(SetWindowPos(m_hwnd, HWND_TOP, prevX, prevY, window_width, window_height, SWP_NOREPOSITION));
	}

	void Window::SetFullscreen(bool fullscreenState)
	{
		m_isFullscreen = fullscreenState;
		int& prevX = m_screenTransitionDetails.prevX;
		int& prevY = m_screenTransitionDetails.prevY;
		int& prevWidth = m_screenTransitionDetails.prevWidth;
		int& prevHeight = m_screenTransitionDetails.prevHeight;

		if (!fullscreenState)	// Go to windowed mode
		{
			// Set back to windowed
			SetWindowLongPtr(m_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
			SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, 0);

			ShowWindow(m_hwnd, SW_SHOWNORMAL);

			SetWindowPos(m_hwnd, HWND_TOP, prevX, prevY, prevWidth, prevHeight, SWP_NOZORDER | SWP_FRAMECHANGED);
			//AdjustWindowDimensions();	// No need to re-adjust w.r.t border, since prev width/height (previous windowed) already accounts for the border
		}
		else  // Go to fullscreen mode
		{
			// Save windowed window dimensions
			RECT oldRect{};
			GetWindowRect(m_hwnd, &oldRect);
			prevX = oldRect.left;
			prevY = oldRect.top;
			prevWidth = oldRect.right - oldRect.left;
			prevHeight = oldRect.bottom - oldRect.top;

			// Set fake fullscreen
			SetWindowLongPtr(m_hwnd, GWL_STYLE, WS_POPUP);
			SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, WS_EX_TOPMOST);

			SetWindowPos(m_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

			RECT currRect{};
			GetWindowRect(m_hwnd, &currRect);
			m_clientWidth = currRect.right - currRect.left;
			m_clientHeight = currRect.bottom - currRect.top;

			ShowWindow(m_hwnd, SW_SHOWMAXIMIZED);
		}
	}

	bool Window::IsFullscreen() const
	{
		return m_isFullscreen;
	}

	HWND Window::GetHWND() const
	{
		return m_hwnd;
	}

	std::string Window::GetNextWindowID()
	{
		static int id = 0;
		std::string winID = "GinoEngineWindowID" + std::to_string(id);
		++id;
		return winID;
	}

	LRESULT Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		Window* activeWin = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		if (activeWin)
			return activeWin->HandleProc(uMsg, wParam, lParam);

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	LRESULT Window::HandleProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_CLOSE:
		{
			//DestroyWindow(m_hwnd);	// Window destructor cleans up hwnd
			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(m_hwnd, &ps);

			FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
			EndPaint(m_hwnd, &ps);
			break;
		}
		}

		return m_winProc(m_hwnd, uMsg, wParam, lParam);
	}




}




//case WM_SIZE:
//{
//	// We want to hook this to ImGui viewport later
//	//if (m_resizeCallback)
//	//	m_resizeCallback(LOWORD(lParam), HIWORD(lParam));
//	break;
//}

//case WM_SYSKEYDOWN:
//	// Custom Alt + Enter
//	if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
//	{
//		SetFullscreen(!m_isFullscreen);
//		// Resizing will be handled through WM_SIZE through a subsequent WM
//	}
//	break;

//case WM_KEYDOWN:
//{
//	switch (wParam)
//	{
//	case VK_ESCAPE:
//	{
//		DestroyWindow(m_hwnd);
//		break;
//	}


//	}

//	// InputKeyDownResponseCallback()


//	break;
//}



//case WM_ACTIVATEAPP:
//case WM_INPUT:
//case WM_MOUSEMOVE:
//case WM_LBUTTONDOWN:
//case WM_LBUTTONUP:
//case WM_RBUTTONDOWN:
//case WM_RBUTTONUP:
//case WM_MBUTTONDOWN:
//case WM_MBUTTONUP:
//case WM_MOUSEWHEEL:
//case WM_XBUTTONDOWN:
//case WM_XBUTTONUP:
//case WM_MOUSEHOVER:
//case WM_KEYDOWN:
//case WM_KEYUP:
//case WM_SYSKEYUP:
//case WM_SYSKEYDOWN:
