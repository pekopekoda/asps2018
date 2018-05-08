#pragma once
#include <tuple>
#include "ASSceneObject.h"
#include "ASSceneInstance.h"

constexpr char* g_pmeshCfg = "Particle mesh";
constexpr char* g_ptexCfg = "Particle texture";
constexpr char* g_prampCfg = "Particle ramp color";
constexpr char* g_pnbrCfg = "Particle number";
//constexpr uint8_t g_changeRate = 82; //Particle emission per second

extern const char* g_meshPath;

class ASParticles : public ASSceneObject
{
	const UINT m_maxCount = 2000;
	const char* m_techniqueName = "RenderParticles";
	struct VERTEX_PROTOTYPE//particle vertex properties
	{
		D3DXVECTOR3 pos;
		D3DXVECTOR3 vel;
		UINT		type;
		float		lifespan;
		float		birth;
		float		mass;
	};

	inline const vector<D3D10_INPUT_ELEMENT_DESC> GetLayoutPrototype()
	{
		return
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
			{ "TYPE", 0, DXGI_FORMAT_R32_UINT, 0, 24, D3D10_INPUT_PER_VERTEX_DATA, 0 },
			{ "TIMER", 0, DXGI_FORMAT_R32_FLOAT, 0, 28, D3D10_INPUT_PER_VERTEX_DATA, 0 },
			{ "BIRTH", 0, DXGI_FORMAT_R32_FLOAT, 0, 32, D3D10_INPUT_PER_VERTEX_DATA, 0 },
			{ "MASS", 0, DXGI_FORMAT_R32_FLOAT, 0, 36, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		};
	}

	effectResourceVariable m_rrParticlesVelocity;
	effectResourceVariable m_rrBump;
	effectResourceVariable m_rrDiffuse;
	effectResourceVariable m_rrRampColor;


	bool m_isFirstFrame = true;
	float m_fLastEmittedTime;
	float m_rateVariation = 0.1f * float(m_maxCount);
	string m_texturePath;
	string m_normalMapPath;
	string m_rampTexturePath;


	effectIntVariable m_rrMaxParticles;
	//What shader is used			
	effectIntVariable m_bvColor;
	effectIntVariable m_bvTexture;
	effectIntVariable m_bvToon;
	effectIntVariable m_bvDiffuseAndSpec;
	effectIntVariable m_bvBump;
	effectIntVariable m_ivCurrentAction;
	effectFloatVariable m_fvRandX;
	effectFloatVariable m_fvRandY;
	effectFloatVariable m_fvRandZ;
	effectFloatVariable m_fvRate;
	effectFloatVariable m_fvTimeToNextEmit;
	ID3D10Buffer *m_bufferStart;

public:
	UINT GetSizeOfVertexPrototype();
	void FirstPass(bool isFirstFrame = false);

	void InitShaderResources(vector<tuple<string, string>> vsBuf);
	void InitViews();
	void InitBuffers();
	void Render();
	void Clear();
	ASParticles(ASScene *scene);
	~ASParticles();
};


class ASParticlesInstances: public ASSceneInstance
{
	const char* m_techniqueName = "UpdateParticles";
	const char* m_meshPath = g_meshPath;
	ASParticles *m_instancer = nullptr;

	struct VERTEX_PROTOTYPE//particle vertex properties
	{
		D3DXVECTOR3 position;
		D3DXVECTOR3 normal;
		D3DXVECTOR2 UV;
	};
public:
	const vector<D3D10_INPUT_ELEMENT_DESC> GetLayoutPrototype()
	{
		return
		{
			{ "POSITION", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D10_INPUT_PER_VERTEX_DATA,   0 },
			{ "NORMAL"  , 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA,   0 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT   , 0, 24, D3D10_INPUT_PER_VERTEX_DATA,   0 },
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0,  D3D10_INPUT_PER_INSTANCE_DATA, 1 },
			{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 12, D3D10_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TYPE"    , 0, DXGI_FORMAT_R32_UINT,		  1, 24, D3D10_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TIMER"   , 0, DXGI_FORMAT_R32_FLOAT,		  1, 28, D3D10_INPUT_PER_INSTANCE_DATA, 1 },
			{ "BIRTH"   , 0, DXGI_FORMAT_R32_FLOAT,		  1, 32, D3D10_INPUT_PER_INSTANCE_DATA, 1 },
			{ "MASS"    , 0, DXGI_FORMAT_R32_FLOAT,		  1, 36, D3D10_INPUT_PER_INSTANCE_DATA, 1 },
		};
	}
	void InitShaderResources(vector<tuple<string, string>> vsBuf);
	
};

UINT ASParticles::GetSizeOfVertexPrototype()
{
	return sizeof(VERTEX_PROTOTYPE);
}

void ASParticles::FirstPass(bool isFirstFrame)
{
	ASRenderer::SetInputLayout(m_layout);
	ID3D10Buffer* pBuffers[1];
	if (isFirstFrame)
		pBuffers[0] = m_bufferStart;
	else
		pBuffers[0] = m_firstBuffer;
	UINT stride[1] = { sizeof(VERTEX_PROTOTYPE) };
	UINT offset[1] = { 0 };
	ASRenderer::SetVertexBuffers(0, 1, pBuffers, stride, offset);
	ASRenderer::SetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

	// Point to the correct output buffer
	pBuffers[0] = m_secondBuffer;
	ASRenderer::StreamOutputSetTargets(1, pBuffers, offset);
	// Draw
	D3D10_TECHNIQUE_DESC techDesc;
	m_technique->GetDesc(&techDesc);
	m_technique->GetPassByIndex(0)->Apply(0);
	if (isFirstFrame)
		ASRenderer::Draw(1);
	else
		ASRenderer::Draw();

	// Get back to normal
	pBuffers[0] = NULL;
	ASRenderer::StreamOutputSetTargets(1, pBuffers, offset);
}


void ASParticles::InitShaderResources(vector<tuple<string, string>> vsBuf)
{
	//m_pUpdateRenderTechnique= ASRenderer::GetTechniqueByName("UpdateParticles");
	//m_pRenderTechnique		= ASRenderer::GetTechniqueByName("RenderParticles");
	m_rrMaxParticles = effectIntVariable("g_maxParticles");
	m_ivCurrentAction = effectIntVariable("g_particlesInterface");
	m_rrMainRenderResource = effectResourceVariable("txParticles");
	m_rrBump = effectResourceVariable("txBump");
	m_rrDiffuse = effectResourceVariable("txDiffuse");
	m_rrRampColor = effectResourceVariable("txRamp");

	m_fvRandX = effectFloatVariable("g_randX");
	m_fvRandY = effectFloatVariable("g_randY");
	m_fvRandZ = effectFloatVariable("g_randZ");
	m_fvRate = effectFloatVariable("g_rate");
	m_fvTimeToNextEmit = effectFloatVariable("g_timeToNextEmit");

	m_bvColor = effectIntVariable("g_color");
	m_bvTexture = effectIntVariable("g_tex");
	m_bvToon = effectIntVariable("g_toon");
	m_bvDiffuseAndSpec = effectIntVariable("g_diffSpec");
	m_bvBump = effectIntVariable("g_bump");

	for (auto it = vsBuf.begin(); it != vsBuf.end(); it++)
	{
		string name = std::get<0>(*it);
		string value = std::get<1>(*it);

		if (name == g_ptexCfg)
		{
			string txtPath = g_texturePath + value;
			string bumpPath = g_texturePath + string("NM_") + value;
			m_rrBump.SetFromFile(txtPath.c_str());
			m_rrBump.Push();
			m_rrDiffuse.SetFromFile(txtPath.c_str());
			m_rrDiffuse.Push();

			m_vConstResourceVariable.Add(&m_rrBump);
			m_vConstResourceVariable.Add(&m_rrDiffuse);
		}
		else if(name == g_prampCfg)
		{
			string rampPath = g_texturePath + value;
			m_rrRampColor.SetFromFile(rampPath.c_str());
			m_rrRampColor.Push();

			m_vConstResourceVariable.Add(&m_rrRampColor);
		}
		else if (name == g_pnbrCfg)
		{
			m_rrMaxParticles.Push(atoi(value.c_str()));
			m_fvRate.Push(0.1f * float(m_maxCount));
		}
	}
	
	m_vEffectResourceVariable2D.Add(&m_rrMainRenderResource);

	m_fLastEmittedTime = 0.0f;
	m_fvTimeToNextEmit.Push(0.0f);

	m_bvColor			.Push(1);
	m_bvTexture			.Push(0);
	m_bvToon			.Push(0);
	m_bvDiffuseAndSpec	.Push(0);
	m_bvBump			.Push(0);

}

void ASParticles::InitViews()
{

	textures2D pRenderTargets2D;
	int s = m_vEffectResourceVariable2D.Size();
	for (auto i = 0; i < s;i++)
		pRenderTargets2D.push_back(&texture2D());

	int views2DNbr = pRenderTargets2D.size();

	ASSceneObject::InitViews(pRenderTargets2D);

	for (int i = 0; i < views2DNbr; i++)
		pRenderTargets2D[i]->Release();
}

void ASParticles::InitBuffers()
{
	m_firstBuffer = NULL;
	m_secondBuffer = NULL;

	m_technique = ASRenderer::GetTechniqueByName(m_techniqueName);
	D3D10_PASS_DESC passDesc;
	m_technique->GetPassByIndex(0)->GetDesc(&passDesc);
	const vector<D3D10_INPUT_ELEMENT_DESC> proto = GetLayoutPrototype();
	ASRenderer::CreateInputLayout(GetLayoutPrototype(), passDesc, &m_layout);

	VERTEX_PROTOTYPE vp1;
	vector<VERTEX_PROTOTYPE> vps;
	vps.assign(1, vp1);

	vps[0].pos = D3DXVECTOR3(0.1f, 0.0f, 0.0f); //position
	vps[0].vel = D3DXVECTOR3(1.0f, 1.0f, 1.0f); //direction
	vps[0].type = int(0);//type
	vps[0].lifespan = float(-1);//lifespan
	vps[0].birth = float(1);//birth
	vps[0].mass = float(1);//mass
	UINT vertSize = sizeof(VERTEX_PROTOTYPE);

	D3D10_BUFFER_DESC vbdesc =
	{
		vertSize,
		D3D10_USAGE_DEFAULT,
		D3D10_BIND_VERTEX_BUFFER,
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
	ASRenderer::CreateBuffer(vbdesc, vbInitData, &m_bufferStart);

	vbdesc.ByteWidth = (m_maxCount+1) * vertSize;
	vbdesc.BindFlags |= D3D10_BIND_STREAM_OUTPUT;
	ASRenderer::CreateBuffer(vbdesc, &m_firstBuffer);
	ASRenderer::CreateBuffer(vbdesc, &m_secondBuffer);
	m_vBuffers = { m_bufferStart, m_firstBuffer , m_secondBuffer };
}   

//--------------------------------------------------------------------------------------
// Render particles
//--------------------------------------------------------------------------------------
void ASParticles::Render()
{
	m_pRenderTargetViews2D.ClearRenderTargets();
	switch (ASUserInterface::currentKey)
	{
		case g_uiCommands::particles::changeRate: m_fvRate.Push(1.0);	break;
			break;
	}

	if (ASUserInterface::keyReleased)
	{
		switch (ASUserInterface::currentKey)
		{
		case 97: m_bvColor.Push(!m_bvColor.val); ASUserInterface::currentKey = 0;	break;
		case 98: m_bvTexture.Push(!m_bvTexture.val); ASUserInterface::currentKey = 0;	break;
		case 99: m_bvToon.Push(!m_bvToon.val); ASUserInterface::currentKey = 0;	break;
		case 100: m_bvDiffuseAndSpec.Push(!m_bvDiffuseAndSpec.val); ASUserInterface::currentKey = 0;	break;
		case 101: m_bvBump.Push(!m_bvBump.val); ASUserInterface::currentKey = 0;	break;
		}
	}

	if (ASUserInterface::currentKey == g_uiCommands::particles::changeRate && ASUserInterface::mouseWheelDelta != 0.0)
	{
		float rate = m_fvRate.val;
		rate += (ASUserInterface::mouseWheelDelta < 0.0) ? -m_rateVariation : m_rateVariation;
		rate = (rate >= m_rateVariation)  ? rate : m_rateVariation;
		rate = (rate <= m_maxCount) ? rate : m_maxCount;
		m_fvRate.Push(rate);
	}

	// Set Effects Parameters
	double _rand = D3DX_PI * 2.0f;
	m_fvRandZ.Push(RAND(-_rand, _rand));
	m_fvRandY.Push(RAND(-_rand, _rand));
	m_fvRandX.Push(RAND(-_rand, _rand));

	m_pRenderTargetViews2D.ClearDepthStencilView();
	m_pRenderTargetViews2D.SetRenderTargets();
	FirstPass(m_isFirstFrame);
	SwapBuffers();

	m_isFirstFrame = false;
}

void ASParticles::Clear(){}
ASParticles::ASParticles(ASScene *scene)
{
	m_scene = scene;
}
ASParticles::~ASParticles(){}



void ASParticlesInstances::InitShaderResources(vector<tuple<string, string>> vsBuf)
{
	m_rrMainRenderResource = effectResourceVariable("txParticlesParam");

	for (auto it = vsBuf.begin(); it != vsBuf.end(); it++)
	{
		string name = std::get<0>(*it);
		string value = std::get<1>(*it);

		if (name == g_pmeshCfg)
		{
			m_meshPath = g_meshPath + *value.c_str();
		}

		m_vEffectResourceVariable2D.Add(&m_rrMainRenderResource);

	}
}
