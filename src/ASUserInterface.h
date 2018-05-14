#pragma once

#include "ASRenderer.h"
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

#define CONFIG_FILE "./config.cfg"
#define PARAM_FILE_MAX 30   //Number of lines which will be read in ASPSParameters.txt
#define MAXPARTICLES_FILELINE 3
#define EMISSIONRATE_FILELINE 4
#define GAVITY_FILELINE 5
#define FRICTION_FILELINE 6
#define EMITONFIELDCENTER_FILELINE 7

using g_uiCommands = const struct
{
	 const struct fields
	{
		 static const uint8_t changeType = 0x54;			// type may be changed
		 static const uint8_t changeSize = 0x53;			// size may be changed
		 static const uint8_t changeCenterForce = 0x43;	// center force may be changed
		 static const uint8_t changeExtremityForce = 0x58;// extremity force may be changed
		 static const uint8_t changeInterpolation = 0x49;	// interpolation may be changed
		 static const uint8_t addFields = 107;			// Increase the current fields snumber
		 static const uint8_t subFields = 109;			// Decrease the current fields number
		 static const uint8_t switchVisibility = 0x48;	// visibillity on/off
	};
	 const struct scene
	{
		static const int8_t switchGravity = 0x47; // turn gravity on/off
		static const int8_t resetCamera = 0x60; // camera is reset to its initial position
	};
	 const struct particles
	{
		static const uint8_t changeRate = 82; //Particle emission per second
	};
	 const struct screen
	{
		// Used with interface input query to warn program user may change currently picked field's parameters, or about different events :
		static const uint8_t shaderColor = 97;//1 key for color on particles
		static const uint8_t shaderTexture = 98;//2 key for texture on particles
		static const uint8_t shaderToon = 99;//3 key for toon on particles
		static const uint8_t shaderDiffuse = 100;//4 key for diffuse and specular on particles
		static const uint8_t shaderBump = 101;//5 key for bump on particles
		static const uint8_t shaderDof = 102;//6 key for Depth on field on particles
		static const uint8_t shaderGlow = 103;//7 key for glow on particles
		static const uint8_t showPanel = 80; // show explanations
	};

};


class ASUserInterface
{
	ASUserInterface();
	~ASUserInterface();
public:
	static int currentKey;
	static bool keyReleased;
	static D3DXVECTOR2 cursorPosition;
	static D3DXVECTOR2 cursorOffset;
	static float mouseWheelDelta;
	static int mouseButton;
	static bool mouseReleased;

	static vector<tuple<string, string>> GetUserFileBuffer();
	static void MouseMoved(WPARAM wParam, LPARAM lParam);
	static void LButtonDown();
	static void LButtonUp();
	static void MouseWheel(WPARAM wParam);

	static bool GetInput(UINT message, WPARAM wParam, LPARAM lParam);
	static void ProcessKeyInput(WPARAM wParam, bool mode);

	static effectIntVariable		m_bvIsMouseReleased;
	static effectIntVariable		m_bvClickEvent;
	static effectFloatVariable		m_fvMouseWheelValue;
	static void InitInput();
	static void UpdateInput();
	static void ResetInput();

};

int ASUserInterface::currentKey = 0;
bool ASUserInterface::keyReleased = true;
D3DXVECTOR2 ASUserInterface::cursorPosition = D3DXVECTOR2();
D3DXVECTOR2 ASUserInterface::cursorOffset = D3DXVECTOR2();
float ASUserInterface::mouseWheelDelta = 0.0f;
int ASUserInterface::mouseButton = 0;
bool ASUserInterface::mouseReleased = false;

effectIntVariable	ASUserInterface::m_bvIsMouseReleased = effectIntVariable();
effectIntVariable	ASUserInterface::m_bvClickEvent = effectIntVariable();
effectFloatVariable	ASUserInterface::m_fvMouseWheelValue = effectFloatVariable();

vector<tuple<string, string>> ASUserInterface::GetUserFileBuffer()
{
	vector<tuple<string, string>> vsBuf;
	string delimiter = "==>";
	//Read user interface text file////////////////////////////////////////////////////////////////////////
	//This file is used by user to write extra parameters like gravity strength before program starts
	ifstream fi("../config.cfg");
	test(HRESULT(bool(!fi)), "Cannot open config file");
	string line = "";
	string type, name, value;

	size_t pos = 0;
	string token;
	string tmpBuf;
	for (std::string line; std::getline(fi, line); )
	{
		tmpBuf = line;
		pos = tmpBuf.find(delimiter);
		name = tmpBuf.substr(0, pos);
		tmpBuf.erase(0, pos + delimiter.length());
		pos = tmpBuf.find(delimiter);
		value = tmpBuf.substr(0, pos);
		vsBuf.push_back({ name, value });
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

	cursorOffset.x = cursorPosition.x - x;
	cursorOffset.y = cursorPosition.y - y;
	cursorPosition = D3DXVECTOR2(x, y);
	mouseButton = wParam;
}

void ASUserInterface::LButtonDown()
{
	if (mouseReleased)
	{
		mouseReleased = false;
		m_bvClickEvent.val = 1;
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
	m_bvIsMouseReleased = effectIntVariable("g_released");
	m_bvIsMouseReleased.Push(1);
	m_bvClickEvent = effectIntVariable("g_clickEvent");
	m_bvClickEvent.Push(0);
}

void ASUserInterface::UpdateInput()
{
	if (mouseWheelDelta < 0.0f)
		m_fvMouseWheelValue.Push(-3);
	else
		m_fvMouseWheelValue.Push(3);
	m_bvIsMouseReleased.Push(mouseReleased);
	m_bvClickEvent.Push();
}

void ASUserInterface::ResetInput()
{
	currentKey = 0;
	mouseWheelDelta = 0.0f;
	m_fvMouseWheelValue.Push(0.0f);
	m_bvClickEvent.Push(0);

}
ASUserInterface::ASUserInterface()
{
	
}
ASUserInterface::~ASUserInterface()
{}