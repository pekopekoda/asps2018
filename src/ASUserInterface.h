#pragma once

#include "ASDisplayDevice.h"
#include <string>
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
	ASUserInterface();
	~ASUserInterface();

	//effectMatrixVariable			d3dPickInfosVariable;				
	//Used for key and mouse events											
	int									currentNumberKey;					
	bool								mouseReleased;//Used for 3D picking	
	D3DXVECTOR2							mousePosition;						


public:
	static vector<string> GetUserFileBuffer();

	static void MouseMoved(WPARAM wParam, LPARAM lParam);
	static void LButtonDown();
	static void LButtonUp();
	static void MouseWheel(WPARAM wParam);

	static bool GetInput(UINT message, WPARAM wParam, LPARAM lParam);
	static void ProcessKeyInput(WPARAM wParam, bool mode);


};

vector<string> ASUserInterface::GetUserFileBuffer()
{
	vector<string> vsBuf;
	const char *separator = "==>";
	UINT ccLength = 3;
	//Read user interface text file////////////////////////////////////////////////////////////////////////
	//This file is used by user to write extra parameters like gravity strength before program starts
	ifstream fi(CONFIG_FILE);
	string line = "";
	string type, name, value;
	size_t nameIdx, valueIdx;

	string tmpBuf;
	while (getline(fi, line))
	{
		tmpBuf = line;
		nameIdx  = tmpBuf.find(separator) + ccLength;
		tmpBuf.erase(0, nameIdx);
		valueIdx = tmpBuf.find(separator) + ccLength;
		tmpBuf.erase(0, valueIdx);

		type = line.substr(0, nameIdx-ccLength);
		name = line.substr(nameIdx, valueIdx-ccLength);
		value = tmpBuf;
		vsBuf.push_back(type);
		vsBuf.push_back(name);
		vsBuf.push_back(value);
	}
	fi.close();
	return vsBuf;
}

////--------------------------------------------------------------------------------------
//// Mouse events
////--------------------------------------------------------------------------------------
void ASUserInterface::MouseMoved(WPARAM wParam, LPARAM lParam)
{
	ASEnvironment *env = ASEnvironment::GetInstance();
	float x = LOWORD(lParam);
	float y = HIWORD(lParam);
	float dx, dy;

	dx = env->userInput.cursorPosition.x - x;
	dy = env->userInput.cursorPosition.y - y;
	env->userInput.cursorPosition = D3DXVECTOR2(x, y);
	if (wParam == VK_RBUTTON)  //Mouse right button down
	{
		env->camera.Rotate(dy*0.006f, dx*0.006f);
	}
	else if (wParam == 16)  //Mouse middle button down
	{
		env->camera.Move(dx*0.1f, -dy*0.1f, 0);
	}
	else if (env->userInput.m_bvIsMouseReleased.val)
		return;
	
	env->Picking3D();
}

void ASUserInterface::LButtonDown()
{
	ASEnvironment *env = ASEnvironment::GetInstance();
	if (env->userInput.m_bvIsMouseReleased.val == 1)
	{
		env->userInput.currentKey = CLICK;
		env->userInput.m_bvIsMouseReleased.Push(0);
	}
	env->Picking3D();
}

void ASUserInterface::LButtonUp()
{
	ASEnvironment *env = ASEnvironment::GetInstance();
	env->userInput.m_bvIsMouseReleased.Push(1);
}

void ASUserInterface::MouseWheel(WPARAM wParam)
{
	ASEnvironment *env = ASEnvironment::GetInstance();
	if (GET_WHEEL_DELTA_WPARAM(wParam) < 0)
	{
		if (!env->userInput.m_ivCurrentAction.val)
			env->camera.Move(0, 0, -13);
		env->userInput.m_fvMouseWheelValue.Push(-3);
	}
	else
	{
		if (!env->userInput.m_ivCurrentAction.val)
			env->camera.Move(0, 0, 13);
		env->userInput.m_fvMouseWheelValue.Push(3);
	}
	env->Picking3D();
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
	ASEnvironment *env = ASEnvironment::GetInstance();
	env->userInput.currentKey = wParam;
	env->userInput.keyReleased = mode;

}

ASUserInterface::ASUserInterface()
{
	
}
ASUserInterface::~ASUserInterface()
{}