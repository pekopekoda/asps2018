#pragma once

#include <string>
#include <memory>
#include <fstream>
#include "ASRenderer.h""

#include "ASParticles.h"
#include "ASFields.h"
#include "ASScreen.h"

constexpr char* g_gravityCfg = "Gravity";
constexpr char* g_atmosphereCfg = "Atmosphere thickness";
// Used with interface input query to warn program user may change currently picked field's parameters, or about different events :
constexpr int8_t g_switchGravity = 0x47; // turn gravity on/off
constexpr int8_t g_gravityOn = -1;
constexpr int8_t g_gravityOff = -2;
constexpr int8_t g_emissionType = 0x45; // particles emitted at center of emitter / randomly in emitter area
constexpr int8_t g_resetCameraCommand = 0x60; // camera is reseted to its initial position
constexpr int8_t g_rightClickEvent = 1;    //Correspond to a trigger which means "right mouse button has just been pushed".
constexpr int8_t g_exitModeCommand = 27;	//Cancel the current mode
constexpr uint8_t g_showPanelCommand = 80; // show explanations
constexpr uint8_t g_translateModeCommand = 77 //Translate mode

using namespace std;

class ASScene
{
	ASScene();
	~ASScene();

	unique_ptr<ASScreen> m_screen;
	vector<unique_ptr<ASSceneObject>> m_objects;

	effectMatrixVariable	m_mvWorldViewProj;
	effectMatrixVariable	m_mvWorld;
	effectMatrixVariable	m_mvProj;
	effectMatrixVariable	m_mvView;
	effectMatrixVariable	m_mvClicCoords;
	effectFloatVariable		m_fvTime;
	effectFloatVariable		m_fvDeltaTime;
	effectFloatVariable		m_fvGravity;
	effectFloatVariable		m_fvFriction;
	float					m_gravity;
	float					m_friction;
	effectIntVariable		m_bvGravity;
	effectIntVariable		m_ivCurrentAction;
	effectVectorVariable	m_vvTranslate;

	float lastTime;

public:

	ASCamera camera;
	ASUserInput userInput;

	float CurrentTime();
	void Init(vector<string> vsBuf);
	void Picking3D();
	void PreFrame();
	void PostFrame();
	void Clear();
};

float ASScene::CurrentTime()
{
	return m_fvTime.val;
}

void ASScene::Init(vector<string> vsBuf)
{
	// Obtain the variables
	m_mvWorldViewProj = effectMatrixVariable("matWorldViewProjection");
	m_mvWorld = effectMatrixVariable("matWorld");
	m_mvProj = effectMatrixVariable("matProj");
	m_fvTime = effectFloatVariable("g_time");
	m_fvDeltaTime = effectFloatVariable("g_deltaTime");
	m_fvGravity = effectFloatVariable("GRAVITY");
	m_fvFriction = effectFloatVariable("RESISTANCE");
	m_bvGravity = effectIntVariable("g_gravity");
	userInput.m_fvMouseWheelValue = effectFloatVariable("g_plusMinus");
	m_vvTranslate = effectVectorVariable("g_translate");
	m_ivCurrentAction = effectIntVariable("g_userInterface");
	m_mvClicCoords = effectMatrixVariable("matPickInfos");

	m_ivCurrentAction.Push(g_showPanelCommand);
	D3DXMATRIX mTemp;
	m_mvClicCoords.Push(mTemp);

	for (auto it = vsBuf.begin(); it != vsBuf.end(); it++)
	{
		if ((*it) != "E")
		{
			it += 2;
		}
		else
		{
			it++;
			if ((*it) == g_gravityCfg)
			{
				it++;
				m_fvGravity.Push(float(atof((*it).c_str())));
			}
			else if ((*it) == g_atmosphereCfg)
			{
				{
					it++;
					m_fvFriction.Push(float(atof((*it).c_str())));
				}
			}
			else
				it++;
		}
	}

	m_fvTime.Push(0.0f);
	m_fvDeltaTime.Push(0.0f);
	m_bvGravity.Push(1);
	userInput.m_fvMouseWheelValue.Push(0.0f);

	userInput.Init();
	// Initialize the world matrix
	D3DXMatrixIdentity(&m_mvWorld.m);
	camera.Reset();

	// Initialize the projection matrix
	D3DXMatrixPerspectiveFovLH(&m_mvProj.m, (float)D3DX_PI * 0.5f, WIDTH / (FLOAT)HEIGHT, 0.2f, 295.0f);

	//update matrices and variables
	m_mvProj.Push();
	m_mvView.Push(camera.m);
	m_mvWorld.Push();

	Picking3D();

	m_screen = make_unique<ASScreen>(new ASScreen());

	m_objects.push_back()
}

//--------------------------------------------------------------------------------------
// Picking
//--------------------------------------------------------------------------------------
void ASScene::Picking3D()
{
	D3DXVECTOR3 vPickRayDir;
	D3DXVECTOR3 vPickRayOrig;

	// Compute the vector of the pick ray in screen space
	D3DXVECTOR3 v;
	v.x = (((2.0f *  userInput.cursorPosition.x) / WIDTH) - 1) / m_mvProj.m._11;
	v.y = -(((2.0f * userInput.cursorPosition.y) / HEIGHT) - 1) / m_mvProj.m._22;
	v.z = 1.0f;

	// Get the inverse view matrix
	D3DXMATRIX mWorldView = m_mvWorld.m * camera.m;
	D3DXMATRIX m;
	D3DXMatrixInverse(&m, NULL, &mWorldView);

	// Transform the screen space pick ray into 3D space
	vPickRayDir.x = v.x * m._11 + v.y * m._21 + v.z * m._31;
	vPickRayDir.y = v.x * m._12 + v.y * m._22 + v.z * m._32;
	vPickRayDir.z = v.x * m._13 + v.y * m._23 + v.z * m._33;
	vPickRayOrig.x = m._41;
	vPickRayOrig.y = m._42;
	vPickRayOrig.z = m._43;

	D3DXVECTOR3 _lookAt = D3DXVECTOR3(m._31, m._32, m._33);
	/*D3D10_VIEWPORT _vp[1];
	UINT _nbr[1] = {1};
	m_device->m_d3dDevice->RSGetViewports( _nbr, _vp );*/

	D3DXMATRIX _mTemp;
	_mTemp._11 = vPickRayOrig.x;		_mTemp._12 = vPickRayDir.x;		_mTemp._13 = _lookAt.x;
	_mTemp._21 = vPickRayOrig.y;		_mTemp._22 = vPickRayDir.y;		_mTemp._23 = _lookAt.y;
	_mTemp._31 = vPickRayOrig.z;		_mTemp._32 = vPickRayDir.z;		_mTemp._33 = _lookAt.z;

	// Sends pick result to GPU
	m_mvClicCoords.Push(_mTemp);
}

void ASScene::PreFrame()
{
	userInput.UpdateInput();

	switch (userInput.currentKey)
	{
		case g_switchGravity:
			if (userInput.keyReleased)
			{
				m_bvGravity.Push(!m_bvGravity.val);
				m_ivCurrentAction.val = (m_bvGravity.val) ? g_gravityOn : g_gravityOff;
			}
			break;

		case g_emissionType:
			m_ivCurrentAction.val = g_emissionType;
			break;
		case g_resetCameraCommand:
			camera.Reset();
			Picking3D();
			break;
		case g_exitModeCommand:
			userInput.m_ivCurrentAction.val = 0;
			break;
		case g_translateModeCommand:
			userInput.m_ivCurrentAction.val = g_translateModeCommand;
			break;
		case g_rightClickEvent:
			int oldValue = m_ivCurrentAction.val;
			m_ivCurrentAction.Push(g_rightClickEvent);
			m_ivCurrentAction.val = oldValue;
			break;
	}
	switch (m_ivCurrentAction.val)
	{
		case g_translateModeCommand:
		{
			if (!userInput.keyReleased)
			{
				switch (userInput.currentKey)
				{
				case VK_UP: m_vvTranslate.Push(0.0f, 1.0f, 0.0f); break;
				case VK_DOWN: m_vvTranslate.Push(0.0f, -1.0f, 0.0f); break;
				case VK_LEFT: m_vvTranslate.Push(-1.0f, 0.0f, 0.0f); break;
				case VK_RIGHT: m_vvTranslate.Push(1.0f, 0.0f, 0.0f); break;
				}
			}
				m_vvTranslate.Push(0.0f, 0.0f, userInput.mouseWheelDelta*0.1);
		}
	}
}

void ASScene::PostFrame()
{
	static DWORD dwTimeStart = 0;
	DWORD dwTimeCur = GetTickCount();
	if (dwTimeStart == 0)
		dwTimeStart = dwTimeCur;
	float time = (dwTimeCur - dwTimeStart) / 1000.0f;

	m_fvDeltaTime.Push(time - m_fvTime.val);
	m_fvTime.Push(time);

	if(!userInput.mouseReleased)
		Picking3D();

	float mwd = m_ivCurrentAction.val ? 0.0 : userInput.mouseWheelDelta;
	if(!userInput.mouseReleased || mwd != 0.0)
		camera.Update(userInput.cursorOffset, userInput.mouseButton, mwd);
	m_mvProj.Push();
	m_ivCurrentAction.Push();
	userInput.Reset();
}


ASScene::ASScene()
{
	m_fvTime.val = 0.0f;
}