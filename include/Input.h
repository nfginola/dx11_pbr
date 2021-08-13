#pragma once
#include <windows.h>

// DirectXTK
#include <Keyboard.h>
#include <Mouse.h>

namespace Gino
{
	// Handles keyboard and mouse, not following SRP on purpose
	class Input
	{
	public:
		Input(HWND hwnd);
		~Input();

		void ProcessKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);
		void ProcessMouse(UINT uMsg, WPARAM wParam, LPARAM lParam);

		void Update();

	private:
		std::unique_ptr<DirectX::Keyboard> m_keyboard;
		std::unique_ptr<DirectX::Mouse> m_mouse;

		// Mouse helper
		DirectX::Mouse::State m_mouseState;
		DirectX::Mouse::ButtonStateTracker m_mouseTracker;

		// Keyboard helper
		

	};
}


