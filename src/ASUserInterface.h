#pragma once

#include "ASDisplayDevice.h"
#include <string>
#include <tuple>
#include <fstream>


/*
User config file should contain the following keywords:

"Particle mesh"
"Emitter mesh"
"Force field mesh"
"Portal mesh"
"Bouncer mesh"
"Emitter texture"
"Force field texture"
"Portal texture"
"Bouncer texture"
"Particle texture"
"Particle ramp color"
"Gravity"
"Atmosphere thickness"

*/

#define CONFIG_FILE "config.cfg"
#define PARAM_FILE_MAX 30   //Number of lines which will be read in ASPSParameters.txt
#define MAXPARTICLES_FILELINE 3
#define EMISSIONRATE_FILELINE 4
#define GAVITY_FILELINE 5
#define FRICTION_FILELINE 6
#define EMITONFIELDCENTER_FILELINE 7

class ASUserInterface
{
public:
	static vector<tuple<string, string>> GetUserFileBuffer();
	static int currentKey;
	static bool keyReleased;
	static D3DXVECTOR2 cursorPosition;
	static D3DXVECTOR2 cursorOffset;
	static float mouseWheelDelta;
	static int mouseButton;
	static bool mouseReleased;

	static void MouseMoved(WPARAM wParam, LPARAM lParam);
	static void LButtonDown();
	static void LButtonUp();
	static void MouseWheel(WPARAM wParam);

	static bool GetInput(UINT message, WPARAM wParam, LPARAM lParam);
	static void ProcessKeyInput(WPARAM wParam, bool mode);

	effectIntVariable		m_bvIsMouseReleased;
	effectFloatVariable		m_fvMouseWheelValue;
	void InitInput();
	void UpdateInput();
	void ResetInput();

	ASUserInterface();
	~ASUserInterface();

};

vector<tuple<string, string>> ASUserInterface::GetUserFileBuffer()
{
	vector<tuple<string, string>> vsBuf;
	string delimiter = "==>";
	//Read user interface text file////////////////////////////////////////////////////////////////////////
	//This file is used by user to write extra parameters like gravity strength before program starts
	ifstream fi(CONFIG_FILE);
	string line = "";
	string type, name, value;

	size_t pos = 0;
	string token;
	string tmpBuf;
	while (getline(fi, line))
	{
		tmpBuf = line;
		pos = tmpBuf.find(delimiter);
		name = tmpBuf.substr(0, pos);
		tmpBuf.erase(0, pos + delimiter.length());
		pos = tmpBuf.find(delimiter);
		value = tmpBuf.substr(0, pos);

		vsBuf.push_back((name, value));
	}
	fi.close();
	return vsBuf;
}

////--------------------------------------------------------------------------------------
//// Mouse events
////--------------------------------------------------------------------------------------
void ASUserInterface::MouseMoved(WPARAM wParam, LPARAM lParam)
{
	float x = LOWORD(lParam);
	float y = HIWORD(lParam);
	float dx, dy;

	cursorOffset.x = cursorPosition.x - x;
	cursorOffset.y = cursorPosition.y - y;
	cursorPosition = D3DXVECTOR2(x, y);
	mouseButton = wParam;

	if (wParam == VK_RBUTTON)  //Mouse right button down
	{
		env->camera.Rotate(dy*0.006f, dx*0.006f);
	}
	else if (wParam == VK_MBUTTON)  //Mouse middle button down
	{
		env->camera.Move(dx*0.1f, -dy*0.1f, 0);
	}
	else if (env->userInput.m_bvIsMouseReleased.val)
		return;
	
	env->Picking3D();
}

void ASUserInterface::LButtonDown()
{
	if (mouseReleased)
	{
		mouseReleased = false;
		currentKey = CLICK;
	}
}

void ASUserInterface::LButtonUp()
{
	mouseReleased = true;
}

void ASUserInterface::MouseWheel(WPARAM wParam)
{
	mouseWheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
}

bool ASUserInterface::GetInput(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MOUSEMOVE:
		MouseMoved(wParam, lParam);
		break;
	case WM_LBUTTONDOWN:
		LButtonDown();
		break;
	case WM_LBUTTONUP:
		LButtonUp();
		break;
	case WM_MOUSEWHEEL:
		MouseWheel(wParam);
		break;
	case WM_KEYDOWN:
		ProcessKeyInput(wParam, false);
		break;
	case WM_KEYUP:
		ProcessKeyInput(wParam, true);
		break;
	default:
		return 0;
	}
	return 1;
}

////--------------------------------------------------------------------------------------
//// Key events
////--------------------------------------------------------------------------------------
void ASUserInterface::ProcessKeyInput(WPARAM wParam, bool mode)
{
	currentKey = wParam;
	keyReleased = mode;
}

void ASUserInterface::InitInput()
{
	mouseReleased = true;
	currentKey = 0;

	m_fvMouseWheelValue = effectFloatVariable("g_plusMinus");
	m_fvMouseWheelValue.Push(0.0f);
	m_bvIsMouseReleased = effectIntVariable("g_released");
	m_bvIsMouseReleased.Push(1);
}

void ASUserInterface::UpdateInput()
{
	if (mouseWheelDelta < 0.0f)
		m_fvMouseWheelValue.Push(-3);
	else
		m_fvMouseWheelValue.Push(3);
	m_bvIsMouseReleased.Push(float(mouseReleased));
}

void ASUserInterface::ResetInput()
{
	currentKey = 0;
	mouseWheelDelta = 0.0f;
	m_fvMouseWheelValue.Push(0.0f);

}
ASUserInterface::ASUserInterface()
{
	
}
ASUserInterface::~ASUserInterface()
{}