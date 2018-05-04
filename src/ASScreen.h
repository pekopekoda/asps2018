#pragma once

#include "ASSceneObject.h"

// Used with interface input query to warn program user may change currently picked field's parameters, or about different events :
constexpr uint8_t g_shaderColor	    = 97 ;//1 key for color on particles
constexpr uint8_t g_shaderTexture	= 98 ;//2 key for texture on particles
constexpr uint8_t g_shaderToon		= 99 ;//3 key for toon on particles
constexpr uint8_t g_shaderDiffuse	= 100;//4 key for diffuse and specular on particles
constexpr uint8_t g_shaderBump		= 101;//5 key for bump on particles
constexpr uint8_t g_shaderDof		= 102;//6 key for Depth on field on particles
constexpr uint8_t g_shaderGlow		= 103;//7 key for glow on particles
constexpr char*	  g_senvMapCfg		= "Environment map";

extern constexpr uint8_t g_showPanelCommand = 80;

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
	effectIntVariable m_bvHUD;

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
	void InitBuffers();
	void Render();
};

void ASScreen::PrePass()
{
	m_renderer->SetInputLayout(m_layout);

	ID3D10Buffer* pBuffers[1];
	pBuffers[0] = m_firstBuffer;
	// Set IA parameters
	UINT stride[1] = { sizeof(VERTEX_PROTOTYPE) };
	UINT offset[1] = { 0 };
	m_renderer->SetVertexBuffers(0, 1, pBuffers, stride, offset);
	m_renderer->SetPrimitiveTopology();
}

void ASScreen::FirstPass()
{
	D3D10_TECHNIQUE_DESC techDesc;
	m_technique->GetDesc(&techDesc);
	m_technique->GetPassByIndex(0)->Apply(0);
	m_renderer->Draw(6);
}


void ASScreen::SecondPass()
{
	D3D10_TECHNIQUE_DESC techDesc;
	m_technique->GetDesc(&techDesc);
	// Draw
	m_technique->GetPassByIndex(1)->Apply(0);
	m_renderer->Draw(6);
}

void ASScreen::ThirdPass()
{
	D3D10_TECHNIQUE_DESC techDesc;
	m_technique->GetDesc(&techDesc);
	// Draw
	m_technique->GetPassByIndex(2)->Apply(0);
	m_renderer->Draw(6);
}


void ASScreen::InitBuffers()
{
	const char *techniqueName = "RenderScreen";
	m_technique = m_renderer->GetTechniqueByName(techniqueName);
	D3D10_PASS_DESC passDesc;
	m_technique->GetPassByIndex(0)->GetDesc(&passDesc);
	const vector<D3D10_INPUT_ELEMENT_DESC> proto = GetLayoutPrototype();
	m_renderer->CreateInputLayout(GetLayoutPrototype(), passDesc, &m_layout);

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
	m_renderer->CreateBuffer(vbdesc, vbInitData, &m_firstBuffer);
	m_vBuffers = { m_firstBuffer };
}

void ASScreen::InitViews()
{
	//Render target handlers
	textures1D renderForOtherObjects { texture1D() };
	textures2D renderForSpecialEffects{ texture2D() };

	//renderForSpecialEffects will be used by the swap chain to display the final result of the screen
	m_renderer->SetSwapChain(&renderForSpecialEffects);

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
	m_bvHUD = effectIntVariable("g_display");
	m_bvHUD.val = 1;

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
			string txtPath = TEXTURE_PATH + value;
			m_rrEnvMap.SetFromFile(txtPath.c_str());
			m_rrEnvMap.Push();
			m_vConstResourceVariable.Add(&m_rrEnvMap);
		}
	}

	m_bvDOF .Push();
	m_bvGlow.Push();
	m_bvHUD .Push();

	m_vEffectResourceVariable2D.Add(&m_rrMainRenderResource);
}

//--------------------------------------------------------------------------------------
// Render screen(s)
//--------------------------------------------------------------------------------------
void ASScreen::Render()
{
	if (m_env->userInput.keyReleased)
	{
		int cua = m_env->userInput.currentKey;
		switch (cua)
		{
		case g_showPanelCommand:
			if (m_env->userInput.m_ivCurrentAction.val == g_showPanelCommand)
				m_env->userInput.m_ivCurrentAction.val = 0;
			else m_env->userInput.m_ivCurrentAction.val = g_showPanelCommand;
			break;
		case g_shaderDof:
			m_bvDOF.Push(int(!m_bvDOF.val));
			m_env->userInput.m_ivCurrentAction.val = 0;
			break;
		case g_shaderGlow:
			m_bvGlow.Push(int(!m_bvGlow.val));
			m_env->userInput.m_ivCurrentAction.val = 0;
			break;
		}
	}
	m_pRenderTargetViews2D.ClearDepthStencilView();
	m_pRenderTargetViews2D.SetRenderTarget(0);
	//Set shader resources of the other 3D objects (fields, particles)
	int rollback = m_extra2DResourcesNbr + 1;
	m_vEffectResourceVariable2D.Push(rollback);

	m_renderTechnique.PrePass();
	//If user wants a blur pass
	if (m_bvDOF.val || m_bvGlow.val)
	{
		m_renderTechnique.FirstPass();
		m_renderer->SetRenderTarget(m_pRenderTargetViews2D[1], m_pDepthStencilView2D);
		m_rrMainRenderResource.Push();
		m_renderTechnique.SecondPass();
	}
	else
	{
		m_renderer->SetRenderTarget(m_pRenderTargetViews2D[1], m_pDepthStencilView2D);
		m_renderTechnique.ThirdPass();
	}
}