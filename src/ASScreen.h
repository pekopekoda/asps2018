#pragma once

#include "ASSceneObject.h"

/*The screen is a square rendered in front of the camera. It is rendered through two passes.
The first pass displays the other objects rendered in the scene as a texture and drops the render into a render texture.
The second pass gets the previous render texture and render the scene one more time, allowing to perform special effect as depth of field or glow.*/
class ASScreen : public ASSceneObject
{
	class ScreenRenderTechnique : public ASSceneObject::RenderTechnique
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

	};

	ScreenRenderTechnique m_renderTechnique;
	effectIntVariable m_bvDOF;
	effectIntVariable m_bvGlow;
	effectIntVariable m_bvHUD;

	effectResourceVariable m_rrHUD;
	effectResourceVariable m_rrEnvMap;
	const char* HUDTexturePath = "../Medias/Textures/Display.tif";
	//HUD																	
	bool m_textDisplayed;						
					
public:
	void InitViews();
	void InitShaderResources(vector<string> vsBuf);
	void InitBuffers();
	void Render();
};

void ASScreen::ScreenRenderTechnique::Init(const char *techniqueName)
{
	ASSceneObject::RenderTechnique::Init(techniqueName);

	VERTEX_PROTOTYPE vp1;
	vector<VERTEX_PROTOTYPE> vps;
	vps.assign(6, vp1);
	vps[0].pos = D3DXVECTOR3(1.0f , 1.0f , 0.0f);
	vps[1].pos = D3DXVECTOR3(-1.0f, 1.0f , 0.0f);
	vps[2].pos = D3DXVECTOR3(-1.0f, -1.0f, 0.0f);
	vps[3].pos = D3DXVECTOR3(1.0f , 1.0f , 0.0f);
	vps[4].pos = D3DXVECTOR3(-1.0f, -1.0f, 0.0f);
	vps[5].pos = D3DXVECTOR3(1.0f , -1.0f, 0.0f);
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
	m_device->CreateBuffer(vbdesc, vbInitData, &m_firstBuffer);
	m_vBuffers = { m_firstBuffer };
}

void ASScreen::ScreenRenderTechnique::PrePass()
{
	m_device->SetInputLayout(m_layout);

	ID3D10Buffer* pBuffers[1];
	pBuffers[0] = m_firstBuffer;
	// Set IA parameters
	UINT stride[1] = { sizeof(VERTEX_PROTOTYPE) };
	UINT offset[1] = { 0 };
	m_device->SetVertexBuffers(0, 1, pBuffers, stride, offset);
	m_device->SetPrimitiveTopology();
}

void ASScreen::ScreenRenderTechnique::FirstPass()
{
	D3D10_TECHNIQUE_DESC techDesc;
	m_technique->GetDesc(&techDesc);
	m_technique->GetPassByIndex(0)->Apply(0);
	m_device->Draw(6);
}


void ASScreen::ScreenRenderTechnique::SecondPass()
{
	D3D10_TECHNIQUE_DESC techDesc;
	m_technique->GetDesc(&techDesc);
	// Draw
	m_technique->GetPassByIndex(1)->Apply(0);
	m_device->Draw(6);
}

void ASScreen::ScreenRenderTechnique::ThirdPass()
{
	D3D10_TECHNIQUE_DESC techDesc;
	m_technique->GetDesc(&techDesc);
	// Draw
	m_technique->GetPassByIndex(2)->Apply(0);
	m_device->Draw(6);
}


void ASScreen::InitBuffers()
{
	m_renderTechnique.Init("RenderScreen");
}

void ASScreen::InitViews()
{
	//Render target handlers
	ID3D10Texture2D *renderForOtherObjects = NULL;
	ID3D10Texture2D *renderForSpecialEffects = NULL;

	m_device->CreateTexture2D(&renderForOtherObjects);
	m_device->CreateTexture2D(&renderForSpecialEffects);

	//renderForSpecialEffects will be used by the swap chain to display the final result of the screen
	m_device->SetSwapChain(&renderForSpecialEffects);

	//Init the rendertarget views and the depth stencil views
	renderTargets2D pRenderTargets2D = { renderForOtherObjects, renderForSpecialEffects };
	renderTargets1D pRenderTargets1D;
	int views2DNbr = pRenderTargets2D.size();

	ASSceneObject::InitViews(pRenderTargets2D, pRenderTargets1D);

	for (int i = 0; i < views2DNbr; i++)
		pRenderTargets2D[i]->Release();
}

void ASScreen::InitShaderResources(vector<string> vsBuf)
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
	m_vConstResourceVariable2D.push_back(&m_rrHUD);

	for (vector<string>::iterator it = vsBuf.begin(); it != vsBuf.end(); it++)
	{
		if ((*it) != "S")
		{
			it += 2;
		}
		else
		{
			it++;
			if ((*it) == SENVMAP_CFG)
			{
				it++;
				string txtPath = TEXTURE_PATH + (*it);
				m_rrEnvMap.SetFromFile(txtPath.c_str());
				m_rrEnvMap.Push();
				m_vConstResourceVariable2D.push_back(&m_rrEnvMap);
			}
			else
				it++;
		}
	}

	m_bvDOF .Push();
	m_bvGlow.Push();
	m_bvHUD .Push();

	m_vEffectResourceVariable2D.push_back(&m_rrMainRenderResource);
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
		case SHOW_PANEL:
			if (m_env->userInput.m_ivCurrentAction.val == SHOW_PANEL)
				m_env->userInput.m_ivCurrentAction.val = 0;
			else m_env->userInput.m_ivCurrentAction.val = SHOW_PANEL;
			break;
		case SHADER_DOF:
			m_bvDOF.Push(int(!m_bvDOF.val));
			m_env->userInput.m_ivCurrentAction.val = 0;
			break;
		case SHADER_GLOW:
			m_bvGlow.Push(int(!m_bvGlow.val));
			m_env->userInput.m_ivCurrentAction.val = 0;
			break;
		}
	}
	m_device->ClearDepthStencilView(m_pDepthStencilView2D);
	m_device->SetRenderTarget(m_pRenderTargetViews2D[0], m_pDepthStencilView2D);
	//Set shader resources of the other 3D objects (fields, particles)
	int rollback = m_extra2DResourcesNbr + 1;
	for (vector<effectResourceVariable*>::iterator it = m_vEffectResourceVariable2D.end()-rollback; it != m_vEffectResourceVariable2D.end(); it++)
		(*it)->Push();

	m_renderTechnique.PrePass();
	//If user wants a blur pass
	if (m_bvDOF.val || m_bvGlow.val)
	{
		m_renderTechnique.FirstPass();
		m_device->SetRenderTarget(m_pRenderTargetViews2D[1], m_pDepthStencilView2D);
		m_rrMainRenderResource.Push();
		m_renderTechnique.SecondPass();
	}
	else
	{
		m_device->SetRenderTarget(m_pRenderTargetViews2D[1], m_pDepthStencilView2D);
		m_renderTechnique.ThirdPass();
	}
}