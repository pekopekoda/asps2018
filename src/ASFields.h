#pragma once

#include "ASSceneObject.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
constexpr uint8_t g_maxFields = 50;				//number of fields in program
// The following are used with interface input query to warn the program that the user may change the parameters of the currenlty selected field
//or about different other events
//constexpr struct fieldCommandStruct
//{
//	const uint8_t changeType = 0x54;			// type may be changed
//	const uint8_t changeSize = 0x53;			// size may be changed
//	const uint8_t changeCenterForce = 0x43;	// center force may be changed
//	const uint8_t changeExtremityForce = 0x58;// extremity force may be changed
//	const uint8_t changeInterpolation = 0x49;	// interpolation may be changed
//	const uint8_t addFields = 107;			// Increase the current fields number
//	const uint8_t subFields = 109;			// Decrease the current fields number
//	const uint8_t switchVisibility = 0x48;	// visibillity on/off
//} g_fieldCommands;
constexpr char*	  fieldnbrCfg = "Fields initial number";

class ASFields : public ASSceneObject
{
	struct VERTEX_PROTOTYPE
	{
		float index;
		unsigned int isSelected;
		float		 type;
		float		 size;
		D3DXVECTOR3 offset;
		D3DXVECTOR3 position;
		D3DXVECTOR3 goal;
		D3DXVECTOR3 strength;
		D3DXVECTOR4 position2;
		D3DXVECTOR4 color;
		D3DXVECTOR4 dummy;
	};
public:
	inline const vector<D3D10_INPUT_ELEMENT_DESC> GetLayoutPrototype()
	{
		return
		{
			{ "INDEX", 0, DXGI_FORMAT_R32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },  // Field's serial number
			{ "ISSELECTED", 0, DXGI_FORMAT_R32_UINT, 0, 4, D3D10_INPUT_PER_VERTEX_DATA, 0 },  // if mouse pressed and cursor on the field, set to true
			{ "FIELDTYPE", 0, DXGI_FORMAT_R32_FLOAT, 0, 8, D3D10_INPUT_PER_VERTEX_DATA, 0 },  // type : 0 = bouncer; 1 = directional; 2 = portal; 3 = particles emitter
			{ "SIZE", 0, DXGI_FORMAT_R32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },  // size
			{ "OFFSET", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D10_INPUT_PER_VERTEX_DATA, 0 }, // Object pick offset
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D10_INPUT_PER_VERTEX_DATA, 0 }, // position in 3D space
			{ "GOAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 40, D3D10_INPUT_PER_VERTEX_DATA, 0 }, // Goal. Only for directional, portals and bouncers
			{ "FORCE", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 52, D3D10_INPUT_PER_VERTEX_DATA, 0 }, // strength of the field
			{ "SV_Position", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 64, D3D10_INPUT_PER_VERTEX_DATA, 0 }, // position of the field in texture sent to particles
			{ "FIELDCOLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 80, D3D10_INPUT_PER_VERTEX_DATA, 0 }, // color of the field
			{ "DUMMY", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 96, D3D10_INPUT_PER_VERTEX_DATA, 0 }, // optional parameter to make cool custom samples
		};
	}
	void Init(const char *techniqueName);
	void FirstPass();
	void SecondPass();
	effectResourceVariable m_rrFieldsPos;
	effectResourceVariable m_rrFieldsGoal;
	effectResourceVariable m_rrFieldsStrength;
	effectResourceVariable m_rrFieldsTypeSize;

	effectIntVariable m_bvEmitAtCenter;
	effectIntVariable m_ivFieldsNumber;
	effectIntVariable m_ivCurrentAction;
	effectIntVariable m_ivTypeUpdate;
	effectFloatVariable m_fvSizeUpdate;
	effectVectorVariable m_vvFieldPositionUpdate;
	effectFloatVariable m_fvCenterForceUpdate;
	effectFloatVariable m_fvExtremityForceUpdate;
	effectFloatVariable m_fvForceInterpolationUpdate;

	bool m_areDisplayed = true;
	int m_fieldsNumber = 10;
	renderTargetViews m_pRenderTargetViews2D1D;

public:
	void UpdateFieldsNumber(int incr);
	void InitShaderResources(vector<tuple<string, string>> vsBuf);
	void InitViews();
	void InitBuffers();
	void InitShaders();
	void Render();
};

void ASFields::FirstPass() 
{
	ID3D10Buffer* pBuffers[2];
	UINT stride[1] = { sizeof(VERTEX_PROTOTYPE) };
	UINT offset[1] = { 0 };

	pBuffers[0] = m_firstBuffer;
	// Point to the correct output buffer
	ASRenderer::SetInputLayout(m_layout);
	ASRenderer::SetVertexBuffers(0, 1, pBuffers, stride, offset);
	ASRenderer::SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	pBuffers[0] = m_secondBuffer;
	ASRenderer::StreamOutputSetTargets(1, pBuffers, offset);

	// Draw
	D3D10_TECHNIQUE_DESC techDesc;
	techDesc.Passes = 1;
	m_technique->GetDesc(&techDesc);

	m_technique->GetPassByIndex(0)->Apply(0);
	ASRenderer::Draw(g_maxFields * 2);

	pBuffers[0] = NULL;
	ASRenderer::StreamOutputSetTargets(1, pBuffers, offset);
}

void ASFields::SecondPass()
{
	ID3D10Buffer* pBuffers[2];
	UINT stride[1] = { sizeof(VERTEX_PROTOTYPE) };
	UINT offset[1] = { 0 };
	pBuffers[0] = m_firstBuffer;
	ASRenderer::SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	ASRenderer::SetVertexBuffers(0, 1, pBuffers, stride, offset);
	m_technique->GetPassByIndex(1)->Apply(0);
	ASRenderer::Draw();
}

void ASFields::UpdateFieldsNumber(int incr)
{
	if ((m_fieldsNumber + incr <= g_maxFields) && (m_fieldsNumber + incr > 0.0))
	{
		m_fieldsNumber += incr;
		m_ivFieldsNumber.Push(m_fieldsNumber);
	}
}

void ASFields::InitShaderResources(vector<tuple<string, string>> vsBuf)
{
	m_rrFieldsPos = effectResourceVariable("txFieldsPos");
	m_rrFieldsGoal = effectResourceVariable("txFieldsGoal");
	m_rrFieldsStrength = effectResourceVariable("txFieldsStrength");
	m_rrFieldsTypeSize = effectResourceVariable("txFieldsTypeSize");
	m_rrMainRenderResource = effectResourceVariable("txFields");
	m_ivFieldsNumber = effectIntVariable("g_fieldNbr");
	m_bvEmitAtCenter = effectIntVariable("g_emAtCenter");
	m_ivCurrentAction = effectIntVariable("g_fieldsInterface");
	m_ivTypeUpdate = effectIntVariable("g_fieldTypeUpdate");
	m_fvSizeUpdate = effectFloatVariable("g_fieldSizeUpdate");
	m_fvCenterForceUpdate = effectFloatVariable("g_fieldCenterForceUpdate");
	m_fvExtremityForceUpdate = effectFloatVariable("g_fieldExtremityForceUpdate");
	m_fvForceInterpolationUpdate = effectFloatVariable("g_fieldForceInterpolationUpdate");
	m_vvFieldPositionUpdate = effectVectorVariable("g_fieldPositionUpdate");

	for (auto it = vsBuf.begin(); it != vsBuf.end(); it++)
	{
		string name = std::get<0>(*it);
		string value = std::get<1>(*it);
	
		if (name == g_fieldCommands.fieldnbrCfg)
		{
			m_ivFieldsNumber.Push(atoi(value.c_str()));
		}
	}

	m_bvEmitAtCenter.Push(0);
	m_ivFieldsNumber.Push(m_fieldsNumber);
	m_vEffectResourceVariable2D.Add(&m_rrMainRenderResource);
	m_vEffectResourceVariable1D.Add(&m_rrFieldsPos);
	m_vEffectResourceVariable1D.Add(&m_rrFieldsGoal);
	m_vEffectResourceVariable1D.Add(&m_rrFieldsStrength);
	m_vEffectResourceVariable1D.Add(&m_rrFieldsTypeSize);
}

void ASFields::InitViews()
{

	textures2D pRenderTargets2D(m_vEffectResourceVariable2D.Size(), NULL);
	textures1D pRenderTargets1D(m_vEffectResourceVariable1D.Size(), NULL);

	for (auto t : pRenderTargets2D)
		t = texture2D(WIDTH, HEIGHT);
	for (auto t : pRenderTargets1D)
		t = texture2D(g_maxFields);

	ASSceneObject::InitViews(pRenderTargets1D);
	ASSceneObject::InitViews(pRenderTargets2D);

	pRenderTargets2D[0]->Release();
	pRenderTargets1D[0]->Release();
	pRenderTargets1D[1]->Release();
	pRenderTargets1D[2]->Release();
	pRenderTargets1D[3]->Release();
}

void ASFields::InitBuffers()
{
	const char* techniqueName = "UpdateFields";
	m_technique = ASRenderer::GetTechniqueByName(techniqueName);
	D3D10_PASS_DESC passDesc;
	m_technique->GetPassByIndex(0)->GetDesc(&passDesc);
	const vector<D3D10_INPUT_ELEMENT_DESC> proto = GetLayoutPrototype();
	ASRenderer::CreateInputLayout(GetLayoutPrototype(), passDesc, &m_layout);

	const UINT nbr = g_maxFields * 2;
	VERTEX_PROTOTYPE vp1;
	vector<VERTEX_PROTOTYPE> vps;
	vps.assign(nbr, vp1);
	int j = 0;
	for (int i = 0; i < g_maxFields; i += 2)
	{
		vps[i].index = float(j);
		vps[i + 1].index = float(j) + 0.5f;//two indices for a field to render field info in a 1D render texture as a line
		vps[i].isSelected = vps[i + 1].isSelected = 0;
		vps[i].type = vps[i + 1].type = 0;
		vps[i].size = vps[i + 1].size = 0;
		vps[i].offset = vps[i + 1].offset = D3DXVECTOR3(0, 0, 0);
		vps[i].position = vps[i + 1].position = D3DXVECTOR3(0.1, 0, 0);
		vps[i].goal = vps[i + 1].goal = D3DXVECTOR3(0, 0, 0);
		vps[i].strength = vps[i + 1].strength = D3DXVECTOR3(0, 0, 0);
		vps[i].position2 = vps[i + 1].position2 = D3DXVECTOR4(0, 0, 0, 1);
		vps[i].color = vps[i + 1].color = D3DXVECTOR4(RAND(0, 1), RAND(0, 1), RAND(0, 1), 0.8);
		vps[i].dummy = vps[i + 1].dummy = D3DXVECTOR4(0, 0, 0, 0);
		j++;
	}
	D3D10_BUFFER_DESC vbdesc =
	{
		g_maxFields * 2 * sizeof(VERTEX_PROTOTYPE),
		D3D10_USAGE_DEFAULT,
		D3D10_BIND_VERTEX_BUFFER | D3D10_BIND_STREAM_OUTPUT,
		0,
		0
	};

	D3D10_SUBRESOURCE_DATA vbInitData;
	ZeroMemory(&vbInitData, sizeof(D3D10_SUBRESOURCE_DATA));

	vbInitData.pSysMem = &vps[0];
	UINT _size = sizeof(vps.data());
	vbInitData.SysMemPitch = _size;
	vbInitData.SysMemSlicePitch = _size;
	//Buffer creation
	ASRenderer::CreateBuffer(vbdesc, vbInitData, &m_firstBuffer);
	ASRenderer::CreateBuffer(vbdesc, vbInitData, &m_secondBuffer);
	m_vBuffers = { m_firstBuffer , m_secondBuffer };
}

//--------------------------------------------------------------------------------------
// Render fields on screen
//--------------------------------------------------------------------------------------
void ASFields::Render()
{	
	m_pRenderTargetViews2D.ClearRenderTargets();
	if (!ASUserInterface::keyReleased)
	{
		switch (ASUserInterface::currentKey)
		{
			case g_fieldCommands.changeType: m_ivTypeUpdate.Push(int(ASUserInterface::mouseWheelDelta)); break;
			case g_fieldCommands.changeSize: m_fvSizeUpdate.Push(ASUserInterface::mouseWheelDelta); break;
			case g_fieldCommands.changeCenterForce: m_fvCenterForceUpdate.Push(ASUserInterface::mouseWheelDelta); break;
			case g_fieldCommands.changeExtremityForce: m_fvExtremityForceUpdate.Push(ASUserInterface::mouseWheelDelta); break;
			case g_fieldCommands.changeInterpolation: m_fvForceInterpolationUpdate.Push(ASUserInterface::mouseWheelDelta); break;
			case VK_LEFT: m_vvFieldPositionUpdate.Push(D3DXVECTOR3(-1.0f, 0.0f, 0.0f)); break;
			case VK_RIGHT: m_vvFieldPositionUpdate.Push(D3DXVECTOR3(1.0f, 0.0f, 0.0f)); break;
			case VK_UP: m_vvFieldPositionUpdate.Push(D3DXVECTOR3(0.0f, 1.0f, 0.0f)); break;
			case VK_DOWN: m_vvFieldPositionUpdate.Push(D3DXVECTOR3(0.0f, -1.0f, 0.0f)); break;
			case g_fieldCommands.addFields:
		}
	}
	else
	{
		switch (ASUserInterface::currentKey)
		{
		case g_fieldCommands.addFields: UpdateFieldsNumber(1); break;
		case g_fieldCommands.subFields: UpdateFieldsNumber(-1); break;
		case g_fieldCommands.switchVisibility: m_areDisplayed = !m_areDisplayed; break;
		}
	}

	//m_fvSizeUpdate.Push(ASRenderer::GetDimensions().x / (float(g_maxFields) * 2.0f));
	//Set correct render targets
	m_pRenderTargetViews2D1D.ClearRenderTargets();
	m_pRenderTargetViews2D1D.ClearDepthStencilView();
	FirstPass();
	m_pRenderTargetViews2D.ClearDepthStencilView();
	m_rrFieldsPos.Push();
	m_rrFieldsGoal.Push();
	m_rrFieldsStrength.Push();
	m_rrFieldsTypeSize.Push();
	m_pRenderTargetViews2D.SetRenderTargets();

	// Swap field buffers
	SwapBuffers();
	//Draw fields only if user wants to see them
	if (m_areDisplayed)
	{
		SecondPass();
	}
}
