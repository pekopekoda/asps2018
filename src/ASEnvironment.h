#pragma once

#include <string>
#include <fstream>
#include "ASDisplayDevice.h"

constexpr char* g_gravityCfg = "Gravity";
constexpr char* g_atmosphereCfg = "Atmosphere thickness";
// Used with interface input query to warn program user may change currently picked field's parameters, or about different events :
constexpr int8_t SWITCH_GRAVITY_ONOFF	= 0x47; // turn gravity on/off
constexpr int8_t GRAVITY_ON				= -1;
constexpr int8_t GRAVITY_OFF			= -2;
constexpr int8_t EMISSION_TYPE			= 0x45; // particles emitted at center of emitter / randomly in emitter area
constexpr int8_t CLEAR_CAM				= 0x60; // camera is reseted to its initial position
constexpr int8_t CLICK					= 1;    //Correspond to a trigger which means "right mouse button has just been pushed".
constexpr int8_t EXIT					= 27;	//Cancel the current mode
constexpr uint8_t g_showPanel = 80; // show explanations
#define MOVE_OBJECT					77 //Translate mode

using namespace std;

class ASEnvironment
{
	ASDisplayDevice* m_device;

	ASEnvironment();
	~ASEnvironment();
	static ASEnvironment* m_singleton;
	

	effectMatrixVariable	m_mvWorldViewProj;
	effectMatrixVariable	m_mvWorld;
	effectMatrixVariable	m_mvProj;
	effectMatrixVariable	m_mvView;
	effectFloatVariable		m_fvTime;
	effectFloatVariable		m_fvDeltaTime;
	effectFloatVariable		m_fvGravity;
	effectFloatVariable		m_fvFriction;
	float					m_gravity;
	float					m_friction;
	effectIntVariable		m_bvGravity;
	effectVectorVariable	m_vvTranslate;
	
	D3DXMATRIX		m_matProjection;
	D3DXVECTOR2		m_mousePosition;

	float lastTime;

	class ASCamera
	{
		D3DXVECTOR3		m_position;//private coordinates
		D3DXVECTOR2		m_rotation;

		D3DXVECTOR3		m_vUp, m_vLook, m_vRight; // camera axis
		D3DXMATRIX		m_matView;

	protected:
		ASCamera();
		~ASCamera();
		
	public:
		const float &x; //Public access to private coordinates, read only
		const float &y; //Public access to private coordinates, read only
		const float &z; //Public access to private coordinates, read only
		const D3DXVECTOR3 &xyz;
		D3DXMATRIX &m; //Public access to private view matrix, read only
		void Reset();

		D3DXVECTOR2 GetRotation();
		void SetPosition(D3DXVECTOR3 m);
		void Move(D3DXVECTOR3 m);
		void SetRotation(D3DXVECTOR2 r);
		void Rotate(D3DXVECTOR2 r);
		void SetPosition(float x, float y, float z);
		void Move(float x, float y, float z);
		void SetRotation(float x, float y);
		void Rotate(float x, float y);

		void MoveOntoPlane(float dx, float dy);
		void Update();

		friend ASEnvironment;
	};

	class ASUserInput
	{
	protected:
		ASUserInput();
		void Init();
		~ASUserInput();
	public:
		int currentKey;
		bool keyReleased;
		effectIntVariable		m_ivCurrentAction;
		effectIntVariable		m_bvIsMouseReleased;
		effectFloatVariable		m_fvMouseWheelValue;
		effectMatrixVariable	m_mvClicCoords;
		D3DXVECTOR2				cursorPosition;


		friend ASEnvironment;
	};

public:

	ASCamera camera;
	ASUserInput userInput;
	static ASEnvironment* GetInstance();
	float CurrentTime();
	void InitWorld(vector<string> vsBuf);
	void Picking3D();
	void PreFrame();
	void PostFrame();
	void Clear();
};

void ASEnvironment::ASUserInput::Init()
{
	currentKey = 0;
	m_ivCurrentAction = effectIntVariable("g_userInterface");
	m_bvIsMouseReleased = effectIntVariable("g_released");
	m_mvClicCoords = effectMatrixVariable("matPickInfos");

	m_ivCurrentAction.Push(g_showPanel);
	m_bvIsMouseReleased.Push(1);
	D3DXMATRIX mTemp;
	m_mvClicCoords.Push(mTemp);
}

ASEnvironment::ASUserInput::ASUserInput():cursorPosition(D3DXVECTOR2(0,0)), keyReleased(true){}
ASEnvironment::ASUserInput::~ASUserInput() {}

ASEnvironment* ASEnvironment::m_singleton = NULL;
ASEnvironment* ASEnvironment::GetInstance()
{
	if (m_singleton == NULL)
		m_singleton = new ASEnvironment();

	return m_singleton;
}

float ASEnvironment::CurrentTime()
{
	return m_fvTime.val;
}

void ASEnvironment::InitWorld(vector<string> vsBuf)
{

	// Obtain the variables
	m_mvWorldViewProj = effectMatrixVariable("matWorldViewProjection");
	m_mvWorld		  = effectMatrixVariable("matWorld");
	m_mvProj		  = effectMatrixVariable("matProj");
	m_fvTime 		  = effectFloatVariable("g_time");
	m_fvDeltaTime 	  = effectFloatVariable("g_deltaTime");
	m_fvGravity		  = effectFloatVariable("GRAVITY");
	m_fvFriction	  = effectFloatVariable("RESISTANCE");
	m_bvGravity 	  = effectIntVariable("g_gravity");
	userInput.m_fvMouseWheelValue = effectFloatVariable("g_plusMinus");
	m_vvTranslate = effectVectorVariable("g_translate");

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
	D3DXMatrixPerspectiveFovLH(&m_matProjection, (float)D3DX_PI * 0.5f, WIDTH / (FLOAT)HEIGHT, 0.2f, 295.0f);

	//update matrices and variables
	//m_mvWorldViewProj.Push(m_mvWorld.m * camera.m * m_matProjection);
	m_mvProj.Push(m_matProjection);
	m_mvView.Push(camera.m);

	m_mvWorld.Push();

	Picking3D();

}

//--------------------------------------------------------------------------------------
// Picking
//--------------------------------------------------------------------------------------
void ASEnvironment::Picking3D()
{
    D3DXVECTOR3 vPickRayDir;
    D3DXVECTOR3 vPickRayOrig;
	
    // Compute the vector of the pick ray in screen space
    D3DXVECTOR3 v;
	v.x = ( ( ( 2.0f *  userInput.cursorPosition.x ) / WIDTH ) - 1 ) /  m_matProjection._11;
    v.y = -( ( ( 2.0f * userInput.cursorPosition.y ) / HEIGHT ) - 1 ) / m_matProjection._22;
    v.z = 1.0f;

    // Get the inverse view matrix
    D3DXMATRIX mWorldView = m_mvWorld.m * camera.m;
    D3DXMATRIX m;
    D3DXMatrixInverse( &m, NULL, &mWorldView );

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
	userInput.m_mvClicCoords.Push(_mTemp);
}
void ASEnvironment::PreFrame()
{
	switch (userInput.currentKey)
	{
	case SWITCH_GRAVITY_ONOFF	:
		if (userInput.keyReleased)
		{
			m_bvGravity.Push(!m_bvGravity.val);
			userInput.m_ivCurrentAction.val = (m_bvGravity.val)? GRAVITY_ON:GRAVITY_OFF;
		}

	case EMISSION_TYPE			: userInput.m_ivCurrentAction.val = EMISSION_TYPE;			break;
	case CLEAR_CAM				:
		camera.Reset();
		Picking3D();
		break;
	case EXIT					: userInput.m_ivCurrentAction.val = 0;						break;
	case MOVE_OBJECT			: userInput.m_ivCurrentAction.val = MOVE_OBJECT;			break;
	case CLICK					:
	{
		int oldValue = userInput.m_ivCurrentAction.val;
		userInput.m_ivCurrentAction.Push(CLICK);
		userInput.m_ivCurrentAction.val = oldValue;
		break;
	}
	}
	switch (userInput.m_ivCurrentAction.val)
	{
	case MOVE_OBJECT:
	{
		if (!userInput.keyReleased || userInput.m_fvMouseWheelValue.val)
		{
			switch (userInput.currentKey)
			{
			case VK_UP: m_vvTranslate.Push(0.0f, 1.0f, 0.0f); break;
			case VK_DOWN: m_vvTranslate.Push(0.0f, -1.0f, 0.0f); break;
			case VK_LEFT: m_vvTranslate.Push(-1.0f, 0.0f, 0.0f); break;
			case VK_RIGHT: m_vvTranslate.Push(1.0f, 0.0f, 0.0f); break;
			}
			if (userInput.m_fvMouseWheelValue.val > 0.0f)
				m_vvTranslate.Push(0.0f, 0.0f, 1.0f);
			else if (userInput.m_fvMouseWheelValue.val < 0.0f)
				m_vvTranslate.Push(0.0f, 0.0f, -1.0f);
		}
		else
			m_vvTranslate.Push(0.0f, 0.0f, 0.0f);

	} break;
	}
}

void ASEnvironment::PostFrame()
{
	float time = m_device->GetCurrentFrameTime();

	m_fvDeltaTime.Push(time - m_fvTime.val);
	m_fvTime.Push(time);

	//update matrices and variables
	//m_mvWorldViewProj.Push(m_mvWorld.m * camera.m * m_matProjection);

	camera.Update();
	m_mvProj.Push(m_matProjection);
	
	userInput.currentKey = 0;
	userInput.m_fvMouseWheelValue.Push(0.0f);
	userInput.m_ivCurrentAction.Push();
}


ASEnvironment::ASEnvironment()
{
	m_device = ASDisplayDevice::GetInstance();
	m_fvTime.val = 0.0f;
}