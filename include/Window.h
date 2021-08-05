#pragma once
#include <Windows.h>

namespace Gino
{
	class Window
	{
	public:
		Window(HINSTANCE hInstance, std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> winProc, int clientWidth = 1600, int clientHeight = 900, const std::string& title = "NagiEngine");
		~Window();
		
		void PumpMessages() const;
		bool IsAlive() const;
		void SetFullscreen(bool fullscreenState);
		bool IsFullscreen() const;
		
	private:
		void RegisterWindow();
		void MakeWindow();
		void FitToClientDim();


		LRESULT HandleProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

		static std::string GetNextWindowID();
		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:
		HINSTANCE m_hInstance;
		HWND m_hwnd;
		std::string m_title;
		std::string m_winID;
		int m_clientWidth;
		int m_clientHeight;
		bool m_isAlive;
		bool m_isFullscreen;
		DWORD m_winStyle;
		DWORD m_winExStyle;
		
		struct ScreenTransitionDetails
		{
			int prevX;
			int prevY;
			int prevWidth;
			int prevHeight;
		} m_screenTransitionDetails;
		
		std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> m_winProc;

	};

}
