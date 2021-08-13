#pragma once
#include <windows.h>

// DirectXTK
#include <Keyboard.h>
#include <Mouse.h>

namespace Gino
{
	using Keys = DirectX::Keyboard::Keys;

	// Handles keyboard and mouse, not following SRP on purpose to make it simple
	class Input
	{
	public:
		enum class MouseMode
		{
			ABS,	// Absolute mode
			REL		// Relative mode
		};

	public:
		Input(HWND hwnd);
		~Input();

		// For systems to call to prepare input for use
		void ProcessKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);
		void ProcessMouse(UINT uMsg, WPARAM wParam, LPARAM lParam);
		void Update();	// Called every frame

		// Mouse
		void ChangeMouseMode(MouseMode mode);

		bool LMBIsPressed() const;
		bool LMBIsReleased() const;
		bool LMBIsDown() const;

		bool RMBIsPressed() const;
		bool RMBIsReleased() const;
		bool RMBIsDown() const;

		bool MMBIsPressed() const;
		bool MMBIsReleased() const;
		bool MMBIsDown() const;

		int GetScrollWheelValue() const;
		const std::pair<int, int>& GetScreenPosition() const;
		const std::pair<int, int>& GetMouseDelta() const;

		// Keyboard
		bool KeyIsPressed(Keys key) const;
		bool KeyIsReleased(Keys key) const;
		bool KeyIsDown(Keys key) const;

	private:
		void InitMouse(DirectX::Mouse::Mode mode);
		void SetMouseMode(DirectX::Mouse::Mode mode);
		void RestorePreviousMouseMode();

	private:
		std::unique_ptr<DirectX::Keyboard> m_keyboard;
		std::unique_ptr<DirectX::Mouse> m_mouse;

		// Mouse helper
		DirectX::Mouse::State m_mouseState;
		DirectX::Mouse::ButtonStateTracker m_mouseTracker;
		DirectX::Mouse::Mode m_prevMouseMode;
		DirectX::Mouse::Mode m_currMouseMode;
		std::pair<int, int> m_prevScreenPosition;
		std::pair<int, int> m_currScreenPosition;
		std::pair<int, int> m_mouseDelta;

		// Keyboard helper
		DirectX::Keyboard::State m_keyboardState;
		DirectX::Keyboard::KeyboardStateTracker m_keyboardTracker;

	};
}


