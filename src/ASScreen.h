#pragma once

#include <map>
#include "ASSceneObject.h"

//// Used with interface input query to warn program user may change currently picked field's parameters, or about different events :
//constexpr uint8_t g_shaderColor	    = 97 ;//1 key for color on particles
//constexpr uint8_t g_shaderTexture	= 98 ;//2 key for texture on particles
//constexpr uint8_t g_shaderToon		= 99 ;//3 key for toon on particles
//constexpr uint8_t g_shaderDiffuse	= 100;//4 key for diffuse and specular on particles
//constexpr uint8_t g_shaderBump		= 101;//5 key for bump on particles
//constexpr uint8_t g_shaderDof		= 102;//6 key for Depth on field on particles
//constexpr uint8_t g_shaderGlow		= 103;//7 key for glow on particles
constexpr char*	g_senvMapCfg		= "Environment map";
constexpr char* HUDTexturePath = "../../Medias/Textures/Display.tif";

class ASScene;

//constexpr uint8_t g_showPanelCommand = 80; // show explanations

D3DXVECTOR4 g_displayCoords(int value)
{
	switch (value)
	{
	case g_uiCommands::fields::changeType: return D3DXVECTOR4( 0.0f, 0.1f, 0.3f, 0.15f ); // change_type		
	case g_uiCommands::fields::changeSize: return D3DXVECTOR4(0.0f, 0.15f, 0.2f, 0.2f); // change_size
	case g_uiCommands::fields::changeCenterForce: return D3DXVECTOR4(0.0f, 0.2f, 0.42f, 0.25f);// change_center_force	
	case g_uiCommands::fields::changeExtremityForce: return D3DXVECTOR4(0.55f, 0.0f, 1.0f, 0.04f);// change_extremity_force
	case g_uiCommands::fields::changeInterpolation: return D3DXVECTOR4(0.55f, 0.04f, 1.0f, 0.1f);// change_interpolation
	case g_uiCommands::scene::switchGravity: return D3DXVECTOR4(0.0f, 0.0f, 0.1f, 0.03f);// switch_gravity_on
	case g_uiCommands::scene::resetCamera: return D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);// clear_cam
	case g_uiCommands::screen::showPanel: return D3DXVECTOR4(0.0f, 0.28f, 1.0f, 1.0f);// show_panel
	default: return D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);
	};
}

/*The screen is a square rendered in front of the camera. It is rendered through two passes.
The first pass displays the other objects rendered in the scene as a texture and drops the render into a render texture.
The second pass gets the previous render texture and render the scene one more time, allowing to perform special effect as depth of field or glow.*/
class ASScreen : public ASSceneObject
{
	const char *m_techniqueName = "RenderScreen";
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

	effectIntVariable m_bvDOF;
	effectIntVariable m_bvGlow;
	effectVectorVariable m_vvdisplayCoords;
	effectIntVariable m_ivShowPanel;

	effectResourceVariable m_rrHUD;
	effectResourceVariable m_rrEnvMap;

	int m_extra2DResourcesNbr = 0;
	//HUD																	
	bool m_textDisplayed;						
					
public:
	ASScreen();
	ASScreen(ASScene *scene);
	//ASScreen(ASScene* const parent);
	void InitViews();
	void InitShaderResources(vector<tuple<string, string>> vsBuf);
	//Add extra resource to render from other objects
	void AddEffectResourceVariable(effectResourceVariable* rr);
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
	m_technique = ASRenderer::GetTechniqueByName(m_techniqueName);
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

ASScreen::ASScreen(){}

ASScreen::ASScreen(ASScene * scene)
{
	m_scene = scene;
}

void ASScreen::InitViews()
{
	//Render target handlers
	textures2D renderTargets{ &texture2D(), &texture2D() };

	//renderForSpecialEffects will be used by the swap chain to display the final result of the screen
	ASRenderer::SetSwapChain(renderTargets);

	ASSceneObject::InitViews(renderTargets);

	for (auto &t : renderTargets)
		t->Release();
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
		case g_uiCommands::screen::showPanel:
			if (m_ivShowPanel.val == 1)
				m_ivShowPanel.Push(0);
			else m_ivShowPanel.Push(1);
			break;
		case g_uiCommands::screen::shaderDof:
			m_bvDOF.Push(int(!m_bvDOF.val));
			break;
		case g_uiCommands::screen::shaderGlow:
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