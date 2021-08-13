#include "pch.h"
#include "Input.h"

namespace Gino
{
	Input::Input(HWND hwnd)
	{
		m_keyboard = std::make_unique<DirectX::Keyboard>();
		m_mouse = std::make_unique<DirectX::Mouse>();
		m_mouse->SetWindow(hwnd);


		m_mouseState = m_mouse->GetState();
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
	}
	void Input::Update()
	{
		m_mouseTracker.Update(m_mouseState);
	}
}
