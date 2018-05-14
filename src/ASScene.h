#pragma once

#include <string>
#include <memory>
#include <fstream>
#include "ASRenderer.h""

#include "ASCamera.h"
#include "ASParticles.h"
#include "ASFields.h"
#include "ASScreen.h"

constexpr char* g_gravityCfg = "Gravity";
constexpr char* g_atmosphereCfg = "Atmosphere thickness";
// Used with interface input query to warn program user may change currently picked field's parameters, or about different events :
//constexpr int8_t g_switchGravity = 0x47; // turn gravity on/off
//constexpr int8_t g_gravityOn = -1;
//constexpr int8_t g_gravityOff = -2;
//constexpr int8_t g_emissionType = 0x45; // particles emitted at center of emitter / randomly in emitter area
//constexpr int8_t g_resetCameraCommand = 0x60; // camera is reseted to its initial position
//constexpr int8_t g_rightClickEvent = 1;    //Correspond to a trigger which means "right mouse button has just been pushed".
//constexpr int8_t g_exitModeCommand = 27;	//Cancel the current mode
//constexpr uint8_t g_translateModeCommand = 77; //Translate mode

using namespace std;

class ASScene
{
	unique_ptr<ASScreen> m_screen;
	unique_ptr<ASCamera> m_camera;

	vector<unique_ptr<ASSceneObject>> m_objects; 
	vector<unique_ptr<ASSceneInstance>> m_instances;


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

	float lastTime;

public:
	float CurrentTime();
	void Init(vector<tuple<string, string>> vsBuf);
	void Picking3D();
	void Update();
	void Render();
	void PostFrame();
	void Clear();
	ASScene();
	~ASScene();
};

float ASScene::CurrentTime()
{
	return m_fvTime.val;
}

void ASScene::Init(vector<tuple<string,string>> vsBuf)
{
	// Obtain the variables
	m_mvWorldViewProj = effectMatrixVariable("matWorldViewProjection");
	m_mvWorld = effectMatrixVariable("matWorld");
	m_mvProj = effectMatrixVariable("matProj");
	m_mvView = effectMatrixVariable("matView");
	m_fvTime = effectFloatVariable("g_time");
	m_fvDeltaTime = effectFloatVariable("g_deltaTime");
	m_fvGravity = effectFloatVariable("GRAVITY");
	m_fvFriction = effectFloatVariable("RESISTANCE");
	m_bvGravity = effectIntVariable("g_gravity");
	ASUserInterface::m_fvMouseWheelValue = effectFloatVariable("g_plusMinus");
	
	m_ivCurrentAction = effectIntVariable("g_userInterface");
	m_mvClicCoords = effectMatrixVariable("matPickInfos");

	D3DXMATRIX mTemp;
	m_mvClicCoords.Push(mTemp);

	string name, value;
	for (auto it = vsBuf.begin(); it != vsBuf.end(); it++)
	{
		name = std::get<0>(*it);
		value = std::get<1>(*it);
		if (name == g_gravityCfg)
			m_fvGravity.Push(float(atof(value.c_str())));
		else if (name == g_atmosphereCfg)
			m_fvFriction.Push(float(atof(value.c_str())));
	}

	m_fvTime.Push(0.0f);
	m_fvDeltaTime.Push(0.0f);
	m_bvGravity.Push(1);
	ASUserInterface::m_fvMouseWheelValue.Push(0.0f);

	ASUserInterface::InitInput();
	// Initialize the world matrix
	D3DXMatrixIdentity(&m_mvWorld.m);

	m_camera = make_unique<ASCamera>(this);
	m_camera->Reset();

	// Initialize the projection matrix
	D3DXMatrixPerspectiveFovLH(&m_mvProj.m, (float)D3DX_PI * 0.5f, WIDTH / (FLOAT)HEIGHT, 0.2f, 295.0f);

	//update matrices and variables
	m_mvProj.Push();
	m_mvView.Push(m_camera->m);
	m_mvWorld.Push();

	Picking3D();

	m_screen = make_unique<ASScreen>(this);
	m_objects.push_back(make_unique<ASFields>(this));
	m_objects.push_back(make_unique<ASParticles>(this));
	m_instances.push_back(make_unique<ASParticlesInstances>());
	m_instances.back()->SetInstancer(m_objects.back().get());

	m_screen->InitShaderResources(vsBuf);
	for (auto &o : m_objects)
	{
		o->InitShaderResources(vsBuf);
		auto p = o->GetMainRenderResource();
		m_screen->AddEffectResourceVariable(p);
	}

	for (auto &o : m_instances)
		o->InitShaderResources(vsBuf);

	m_screen->InitViews();
	for (auto &o : m_objects)
		o->InitViews();

	m_screen->InitBuffers();
	for (auto &o : m_instances)
		o->InitBuffers();
	for (auto &o : m_objects)
		o->InitBuffers();

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
	v.x = (((2.0f *  ASUserInterface::cursorPosition.x) / WIDTH) - 1) / m_mvProj.m._11;
	v.y = -(((2.0f * ASUserInterface::cursorPosition.y) / HEIGHT) - 1) / m_mvProj.m._22;
	v.z = 1.0f;

	// Get the inverse view matrix
	D3DXMATRIX mWorldView = m_mvWorld.m * m_camera->m;
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

	D3DXMATRIX _mTemp;
	_mTemp._11 = vPickRayOrig.x;		_mTemp._12 = vPickRayDir.x;		_mTemp._13 = _lookAt.x;
	_mTemp._21 = vPickRayOrig.y;		_mTemp._22 = vPickRayDir.y;		_mTemp._23 = _lookAt.y;
	_mTemp._31 = vPickRayOrig.z;		_mTemp._32 = vPickRayDir.z;		_mTemp._33 = _lookAt.z;

	// Sends pick result to GPU
	m_mvClicCoords.Push(_mTemp);
}

void ASScene::Update()
{
	ASUserInterface::UpdateInput();
	if (ASUserInterface::keyReleased)
	{
		switch (ASUserInterface::currentKey)
		{
		case g_uiCommands::scene::switchGravity:
			m_bvGravity.Push(!m_bvGravity.val);
			break;

		case g_uiCommands::scene::resetCamera:
			m_camera->Reset();
			Picking3D();
			break;
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

	if(!ASUserInterface::mouseReleased)
		Picking3D();

	double mwd = m_ivCurrentAction.val ? 0.0 : ASUserInterface::mouseWheelDelta;
	if(!ASUserInterface::mouseReleased || mwd != 0.0)
		m_camera->Update(ASUserInterface::cursorOffset, ASUserInterface::mouseButton, mwd);
	m_mvProj.Push();
	m_ivCurrentAction.Push();
	ASUserInterface::ResetInput();
}

void ASScene::Render()
{
	m_screen->Render();
	for (auto &o : m_objects)
		o->Render();
}

void ASScene::Clear()
{
	m_screen->Clear();
	for (auto &o : m_objects)
		o->Clear();
	for (auto &o : m_instances)
		o->Clear();
}

ASScene::ASScene()
{
	m_fvTime.val = 0.0f;
}

ASScene::~ASScene() {}