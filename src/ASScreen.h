#pragma once

#include <map>
#include "ASSceneObject.h"

extern uiCommandStruct g_uiCommands;
//// Used with interface input query to warn program user may change currently picked field's parameters, or about different events :
//constexpr uint8_t g_shaderColor	    = 97 ;//1 key for color on particles
//constexpr uint8_t g_shaderTexture	= 98 ;//2 key for texture on particles
//constexpr uint8_t g_shaderToon		= 99 ;//3 key for toon on particles
//constexpr uint8_t g_shaderDiffuse	= 100;//4 key for diffuse and specular on particles
//constexpr uint8_t g_shaderBump		= 101;//5 key for bump on particles
//constexpr uint8_t g_shaderDof		= 102;//6 key for Depth on field on particles
//constexpr uint8_t g_shaderGlow		= 103;//7 key for glow on particles
constexpr char*	  g_senvMapCfg		= "Environment map";

//constexpr uint8_t g_showPanelCommand = 80; // show explanations
constexpr initializer_list<float> g_displayCoords(int value)
{
	const std::pair<int, initializer_list<float> > coords[] =
	{
		{ 0,{ 0.0, 0.0, 0.0, 0.0 } }, //default value
		{ g_uiCommands.fields.changeType,{ 0.0f, 0.1f, 0.3f, 0.15f } }, // change_type		
		{ g_uiCommands.fields.changeSize,{ 0.0, 0.15, 0.2, 0.2 } }, // change_size
		{ g_uiCommands.fields.changeCenterForce,{ 0.0, 0.2, 0.42, 0.25 } }, // change_center_force	
		{ g_uiCommands.fields.changeExtremityForce,{ 0.55, 0.0, 1.0, 0.04 } }, // change_extremity_force
		{ g_uiCommands.fields.changeInterpolation,{ 0.55, 0.04, 1.0, 0.1 } }, // change_interpolation
		{ g_uiCommands.scene.switchGravity,{ 0.0, 0.0, 0.1, 0.03 } }, // switch_gravity_on
		{ g_uiCommands.scene.resetCamera,{ 0.0, 0.0, 0.0, 0.0 } }, // clear_cam
		{ g_uiCommands.particles.emissionType,{ 0.55, 0.14, 1.0, 0.18 } }, // emission_type	
		{ g_uiCommands.screen.showPanel,{ 0.0, 0.28, 1.0, 1.0 } },  // show_panel
	};

	for (auto p : coords)
		if (p.first == value)
			return p.second;
}

/*The screen is a square rendered in front of the camera. It is rendered through two passes.
The first pass displays the other objects rendered in the scene as a texture and drops the render into a render texture.
The second pass gets the previous render texture and render the scene one more time, allowing to perform special effect as depth of field or glow.*/
class ASScreen : public ASSceneObject
{
	struct VERTEX_PROTOTYPE
	{
		D3DXVECTOR3 pos;
		UINT coordsIndex;
	};
		
public:
	inline const vector<D3D10_INPUT_ELEMENT_DESC> GetLayoutPrototype()
	{
		return
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
			{ "INDEX"	, 0, DXGI_FORMAT_R32_UINT		, 0, 4, D3D10_INPUT_PER_VERTEX_DATA, 0 }
		};
	}
	void InitBuffers();
	void PrePass();
	void FirstPass();
	void SecondPass();
	void ThirdPass();
	void Init(const char *techniqueName);

	effectIntVariable m_bvDOF;
	effectIntVariable m_bvGlow;
	effectVectorVariable m_vvdisplayCoords;
	effectIntVariable m_ivShowPanel;

	effectResourceVariable m_rrHUD;
	effectResourceVariable m_rrEnvMap;

	const char* HUDTexturePath = "../Medias/Textures/Display.tif";
	int m_extra2DResourcesNbr = 0;
	//HUD																	
	bool m_textDisplayed;						
					
public:
	ASScreen();
	~ASScreen();
	void InitViews();
	void InitShaderResources(vector<tuple<string, string>> vsBuf);
	//Add extra resource to render from other objects
	void AddEffectResourceVariable(effectResourceVariable* rr);
	void InitBuffers();
	void Render();
};

void ASScreen::PrePass()
{
	ASRenderer::SetInputLayout(m_layout);

	ID3D10Buffer* pBuffers[1];
	pBuffers[0] = m_firstBuffer;
	// Set IA parameters
	UINT stride[1] = { sizeof(VERTEX_PROTOTYPE) };
	UINT offset[1] = { 0 };
	ASRenderer::SetVertexBuffers(0, 1, pBuffers, stride, offset);
	ASRenderer::SetPrimitiveTopology();
}

void ASScreen::FirstPass()
{
	D3D10_TECHNIQUE_DESC techDesc;
	m_technique->GetDesc(&techDesc);
	m_technique->GetPassByIndex(0)->Apply(0);
	ASRenderer::Draw(6);
}


void ASScreen::SecondPass()
{
	D3D10_TECHNIQUE_DESC techDesc;
	m_technique->GetDesc(&techDesc);
	// Draw
	m_technique->GetPassByIndex(1)->Apply(0);
	ASRenderer::Draw(6);
}

void ASScreen::ThirdPass()
{
	D3D10_TECHNIQUE_DESC techDesc;
	m_technique->GetDesc(&techDesc);
	// Draw
	m_technique->GetPassByIndex(2)->Apply(0);
	ASRenderer::Draw(6);
}


void ASScreen::AddEffectResourceVariable(effectResourceVariable * rr)
{
	m_vEffectResourceVariable2D.Add(rr);
}

void ASScreen::InitBuffers()
{
	const char *techniqueName = "RenderScreen";
	m_technique = ASRenderer::GetTechniqueByName(techniqueName);
	D3D10_PASS_DESC passDesc;
	m_technique->GetPassByIndex(0)->GetDesc(&passDesc);
	const vector<D3D10_INPUT_ELEMENT_DESC> proto = GetLayoutPrototype();
	ASRenderer::CreateInputLayout(GetLayoutPrototype(), passDesc, &m_layout);

	VERTEX_PROTOTYPE vp1;
	vector<VERTEX_PROTOTYPE> vps;
	vps.assign(6, vp1);
	vps[0].pos = D3DXVECTOR3(1.0f, 1.0f, 0.0f);
	vps[1].pos = D3DXVECTOR3(-1.0f, 1.0f, 0.0f);
	vps[2].pos = D3DXVECTOR3(-1.0f, -1.0f, 0.0f);
	vps[3].pos = D3DXVECTOR3(1.0f, 1.0f, 0.0f);
	vps[4].pos = D3DXVECTOR3(-1.0f, -1.0f, 0.0f);
	vps[5].pos = D3DXVECTOR3(1.0f, -1.0f, 0.0f);
	for (UINT i = 0; i < 6; i++)
		vps[i].coordsIndex = 0;

	D3D10_BUFFER_DESC vbdesc =
	{
		6 * sizeof(VERTEX_PROTOTYPE),
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
	m_vBuffers = { m_firstBuffer };
}

void ASScreen::InitViews()
{
	//Render target handlers
	textures1D renderForOtherObjects { texture1D() };
	textures2D renderForSpecialEffects{ texture2D() };

	//renderForSpecialEffects will be used by the swap chain to display the final result of the screen
	ASRenderer::SetSwapChain(&renderForSpecialEffects);

	ASSceneObject::InitViews(renderForOtherObjects);
	ASSceneObject::InitViews(renderForSpecialEffects);

	for (auto t : renderForOtherObjects)
		t.Release();
	for (auto t : renderForSpecialEffects)
		t.Release();
}

void ASScreen::InitShaderResources(vector<tuple<string, string>> vsBuf)
{
	m_rrMainRenderResource = effectResourceVariable("txScreen0");
	m_rrHUD = effectResourceVariable("txHUD");
	m_rrEnvMap = effectResourceVariable("txEnv");
	m_bvDOF = effectIntVariable("g_DOF");
	m_bvDOF.val = 0;
	m_bvGlow = effectIntVariable("g_glow");
	m_bvGlow.val = 0;
	m_vvdisplayCoords = effectVectorVariable("g_displayCoords");
	m_vvdisplayCoords.Push(g_displayCoords(0));
	m_ivShowPanel = effectIntVariable("g_showPanel");
	m_ivShowPanel.Push(0);

	m_rrHUD.SetFromFile(HUDTexturePath);
	m_rrHUD.Push();
	m_vConstResourceVariable.Add(&m_rrHUD);

	for (auto it = vsBuf.begin(); it != vsBuf.end(); it++)
	{
		string name = std::get<0>(*it);
		string value = std::get<1>(*it);
		if (name == g_senvMapCfg)
		{
			it++;
			string txtPath = g_texturePath + value;
			m_rrEnvMap.SetFromFile(txtPath.c_str());
			m_rrEnvMap.Push();
			m_vConstResourceVariable.Add(&m_rrEnvMap);
		}
	}

	m_bvDOF .Push();
	m_bvGlow.Push();

	m_vEffectResourceVariable2D.Add(&m_rrMainRenderResource);
}

//--------------------------------------------------------------------------------------
// Render screen(s)
//--------------------------------------------------------------------------------------
void ASScreen::Render()
{
	if (ASUserInterface::keyReleased)
	{
		switch (ASUserInterface::currentKey)
		{
		case g_uiCommands.screen.showPanel:
			if (m_ivShowPanel.val == 1)
				m_ivShowPanel.Push(0);
			else m_ivShowPanel.Push(1);
			break;
		case g_uiCommands.screen.shaderDof:
			m_bvDOF.Push(int(!m_bvDOF.val));
			break;
		case g_uiCommands.screen.shaderGlow:
			m_bvGlow.Push(int(!m_bvGlow.val));
			break;
		}
	}
	m_pRenderTargetViews2D.ClearDepthStencilView();
	m_pRenderTargetViews2D.SetRenderTarget(0);
	//Set shader resources of the other 3D objects (fields, particles)
	int rollback = m_extra2DResourcesNbr + 1;
	m_vEffectResourceVariable2D.Push(rollback);

	PrePass();
	//If user wants a blur pass
	if (m_bvDOF.val || m_bvGlow.val)
	{
		FirstPass();
		m_pRenderTargetViews2D.SetRenderTarget(1);
		m_rrMainRenderResource.Push();
		SecondPass();
	}
	else
	{
		m_pRenderTargetViews2D.SetRenderTarget(1);
		ThirdPass();
	}
}