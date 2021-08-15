#include "pch.h"
#include "Input.h"

namespace Gino
{
	Input::Input(HWND hwnd) :
		m_hwnd(hwnd),
		m_prevScreenPosition({ 0, 0 }),
		m_currScreenPosition({ 0, 0 }),
		m_mouseDelta({ 0, 0 }),
		m_cursorCentered(false)
	{
		m_keyboard = std::make_unique<DirectX::Keyboard>();
		m_mouse = std::make_unique<DirectX::Mouse>();
		m_mouse->SetWindow(hwnd);

		InitMouse(DirectX::Mouse::Mode::MODE_ABSOLUTE);
	}

	Input::~Input()
	{
	}

	void Input::ProcessKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		DirectX::Keyboard::ProcessMessage(uMsg, wParam, lParam);
	}

	void Input::ProcessMouse(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		DirectX::Mouse::ProcessMessage(uMsg, wParam, lParam);

		switch (uMsg)
		{

		case WM_MOUSEMOVE:
			// abs position
			break;
		case WM_INPUT:
		{
			UINT dwSize = 0;

			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
			LPBYTE lpb = new BYTE[dwSize];
			if (lpb == NULL)
			{
				assert(false);
			}

			if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
			{
				assert(false);
				OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));
			}

			RAWINPUT* raw = (RAWINPUT*)lpb;

			if (raw->header.dwType == RIM_TYPEMOUSE)
			{
				// We grab all the raw delta WMs that occured between last frames PumpMessage and this frames PumpMessage!
				// We used to grab the last delta, which is incorrect! (We would be ignoring a bunch of movement)
				m_mouseDeltasThisFrame.push({ raw->data.mouse.lLastX, raw->data.mouse.lLastY });
			}

			delete[] lpb;
			break;
		}
		default:
			break;
		}
	}
	
	void Input::Update()
	{
		// We cant update this on ProcessMouse. Otherwise some functionality dont work properly (e.g PRESSED)
		m_mouseState = m_mouse->GetState();
		m_mouseTracker.Update(m_mouseState);

		m_keyboardState = m_keyboard->GetState();
		m_keyboardTracker.Update(m_keyboardState);

		// Track position only in absolute mode
		if (m_currMouseMode == DirectX::Mouse::Mode::MODE_ABSOLUTE)
		{
			m_currScreenPosition = { m_mouseState.x, m_mouseState.y };
			m_prevScreenPosition = m_currScreenPosition;
		}

		// Calculate total mouse delta for this frame
		while (!m_mouseDeltasThisFrame.empty())
		{
			const auto& d = m_mouseDeltasThisFrame.front();
			m_mouseDeltasThisFrame.pop();

			m_mouseDelta.first += d.first;
			m_mouseDelta.second += d.second;
		}
	}

	void Input::Reset()
	{
		// Reset delta
		/*
			There is no WM for "no input", so we need to do this at the end of the frame once.
			
			NOTE:

			The only time we should be using "GetDelta" is if it was computed through manual position absolute delta,
			which does have full move information (unlike downsampling deltas from raw input WMs)
		*/
		m_mouseDelta = { 0, 0 };
	}

	void Input::SetMouseMode(MouseMode mode)
	{
		switch (mode)
		{
		case MouseMode::Relative:
			SetMouseMode(DirectX::Mouse::Mode::MODE_RELATIVE);

			CenterCursor();
			break;
		case MouseMode::Absolute:
			SetMouseMode(DirectX::Mouse::Mode::MODE_ABSOLUTE);

			// Restore cursor location
			SetCursorPos(m_currScreenPosition.first, m_currScreenPosition.second);
			UncenterCursor();
			break;
		default:
			assert(false);
		}
	}

	void Input::HideCursor() const
	{
		m_mouse->SetVisible(false);
	}

	void Input::ShowCursor() const
	{
		m_mouse->SetVisible(true);
	}


	void Input::CenterCursor()
	{
		// We cant explicitly interfere with the cursor position since we are using mouse absolute mode to calculate delta manually
		//RECT rect{};
		//GetWindowRect(m_hwnd, &rect);
		//SetCursorPos((rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2);

		m_cursorCentered = true;
	}

	void Input::UncenterCursor()
	{
		m_cursorCentered = false;
	}

	

	void Input::InitMouse(DirectX::Mouse::Mode mode)
	{
		m_currMouseMode = mode;
		SetMouseMode(mode);
	}
	
	bool Input::LMBIsPressed() const
	{
		return m_mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::ButtonState::PRESSED;
	}
	bool Input::LMBIsReleased() const
	{
		return m_mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::ButtonState::RELEASED;
	}
	bool Input::LMBIsDown() const
	{
		return m_mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::ButtonState::HELD;
	}

	bool Input::RMBIsPressed() const
	{
		return m_mouseTracker.rightButton == DirectX::Mouse::ButtonStateTracker::ButtonState::PRESSED;
	}
	bool Input::RMBIsReleased() const
	{
		return m_mouseTracker.rightButton == DirectX::Mouse::ButtonStateTracker::ButtonState::RELEASED;
	}
	bool Input::RMBIsDown() const
	{
		return m_mouseTracker.rightButton == DirectX::Mouse::ButtonStateTracker::ButtonState::HELD;
	}

	bool Input::MMBIsPressed() const
	{
		return m_mouseTracker.middleButton == DirectX::Mouse::ButtonStateTracker::ButtonState::PRESSED;
	}
	bool Input::MMBIsReleased() const
	{
		return m_mouseTracker.middleButton == DirectX::Mouse::ButtonStateTracker::ButtonState::RELEASED;
	}
	bool Input::MMBIsDown() const
	{
		return m_mouseTracker.middleButton == DirectX::Mouse::ButtonStateTracker::ButtonState::HELD;
	}
	
	int Input::GetScrollWheelValue() const
	{
		return m_mouseState.scrollWheelValue;
	}
	std::pair<int, int> Input::GetScreenPosition() const
	{
		if (m_cursorCentered)
		{
			RECT rect{};
			GetWindowRect(m_hwnd, &rect);
			return { (rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2 };
		}
		else
		{
			return m_currScreenPosition;
		}
	}
	const std::pair<int, int>& Input::GetMouseDelta() const
	{
		return m_mouseDelta;
	}

	bool Input::KeyIsPressed(Keys key) const
	{
		return m_keyboardTracker.IsKeyPressed(key);
	}
	bool Input::KeyIsReleased(Keys key) const
	{
		return m_keyboardTracker.IsKeyReleased(key);
	}
	bool Input::KeyIsDown(Keys key) const
	{
		return m_keyboardState.IsKeyDown(key);
	}

	void Input::SetMouseMode(DirectX::Mouse::Mode mode)
	{
		m_prevMouseMode = m_currMouseMode;
		m_currMouseMode = mode;
		m_mouse->SetMode(mode);
	}
	void Input::RestorePreviousMouseMode()
	{
		m_currMouseMode = m_prevMouseMode;
		m_mouse->SetMode(m_prevMouseMode);
	}
}
