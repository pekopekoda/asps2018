#include "ASDisplayObject.h"

class ASFieldsDisplay : public ASDisplayObject
{
	class FieldsRenderTechnique : public ASDisplayObject::RenderTechnique
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
	};


	class FieldsInstanceRenderTechnique : public InstanceRenderTechnique
	{
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
				{ "POSITION"	, 1, DXGI_FORMAT_R32G32B32_FLOAT	, 0, 0	, D3D10_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL"		, 1, DXGI_FORMAT_R32G32B32_FLOAT	, 0, 12	, D3D10_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD"	, 1, DXGI_FORMAT_R32G32_FLOAT		, 0, 24	, D3D10_INPUT_PER_VERTEX_DATA, 0 },
				{ "INDEX"		, 0, DXGI_FORMAT_R32_FLOAT			, 1, 0	, D3D10_INPUT_PER_VERTEX_DATA, 1 },  // Field's serial number
				{ "ISSELECTED"	, 0, DXGI_FORMAT_R32_UINT			, 1, 4	, D3D10_INPUT_PER_VERTEX_DATA, 1 },  // if mouse pressed and cursor on the field, set to true
				{ "FIELDTYPE"	, 0, DXGI_FORMAT_R32_FLOAT			, 1, 8	, D3D10_INPUT_PER_VERTEX_DATA, 1 },  // type : 0 = bouncer; 1 = directional; 2 = portal; 3 = particles emitter
				{ "SIZE"		, 0, DXGI_FORMAT_R32_FLOAT			, 1, 12	, D3D10_INPUT_PER_VERTEX_DATA, 1 },  // size
				{ "OFFSET"		, 0, DXGI_FORMAT_R32G32B32_FLOAT	, 1, 16	, D3D10_INPUT_PER_VERTEX_DATA, 1 }, // Object pick offset
				{ "POSITION"	, 0, DXGI_FORMAT_R32G32B32_FLOAT	, 1, 28	, D3D10_INPUT_PER_VERTEX_DATA, 1 }, // position in 3D space
				{ "GOAL"		, 0, DXGI_FORMAT_R32G32B32_FLOAT	, 1, 40	, D3D10_INPUT_PER_VERTEX_DATA, 1 }, // Goal. Only for directional, portals and bouncers
				{ "FORCE"		, 0, DXGI_FORMAT_R32G32B32_FLOAT	, 1, 52	, D3D10_INPUT_PER_VERTEX_DATA, 1 }, // strength of the field
				{ "SV_Position"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 1, 64	, D3D10_INPUT_PER_VERTEX_DATA, 1 }, // position of the field in texture sent to particles
				{ "FIELDCOLOR"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 1, 80	, D3D10_INPUT_PER_VERTEX_DATA, 1 }, // color of the field
				{ "DUMMY"		, 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 1, 96	, D3D10_INPUT_PER_VERTEX_DATA, 1 }, // optional parameter to make cool custom samples
			};
		}
	};

	FieldsRenderTechnique  m_renderTechnique;
	effectResourceVariable m_rrFieldsPos;
	effectResourceVariable m_rrFieldsGoal;
	effectResourceVariable m_rrFieldsStrength;
	effectResourceVariable m_rrFieldsTypeSize;

	effectIntVariable m_bvEmitAtCenter;
	effectIntVariable m_ivFieldsNumber;
	effectFloatVariable m_fvWidth;
	bool m_areDisplayed = true;
	int m_fieldsNumber = 10;

public:
	void UpdateFieldsNumber(int incr);
	void InitShaderResources(vector<string> vsBuf);
	void InitViews();
	void InitBuffers();
	void InitShaders();
	void Render();
};
void ASFieldsDisplay::FieldsRenderTechnique::Init(const char *techniqueName)
{
	ASDisplayObject::RenderTechnique::Init(techniqueName);
	const UINT nbr = MAX_FIELDS * 2;
	VERTEX_PROTOTYPE vp1;
	vector<VERTEX_PROTOTYPE> vps;
	vps.assign(nbr, vp1);
	int j = 0;
	for (int i = 0; i < MAX_FIELDS; i += 2)
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
		MAX_FIELDS * 2 * sizeof(VERTEX_PROTOTYPE),
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
	m_device->CreateBuffer(vbdesc, vbInitData, &m_secondBuffer);
	m_vBuffers = { m_firstBuffer , m_secondBuffer };
}

void ASFieldsDisplay::FieldsRenderTechnique::FirstPass() 
{
	ID3D10Buffer* pBuffers[2];
	UINT stride[1] = { sizeof(VERTEX_PROTOTYPE) };
	UINT offset[1] = { 0 };

	pBuffers[0] = m_firstBuffer;
	// Point to the correct output buffer
	m_device->SetInputLayout(m_layout);
	m_device->SetVertexBuffers(0, 1, pBuffers, stride, offset);
	m_device->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	pBuffers[0] = m_secondBuffer;
	m_device->StreamOutputSetTargets(1, pBuffers, offset);

	// Draw
	D3D10_TECHNIQUE_DESC techDesc;
	techDesc.Passes = 1;
	m_technique->GetDesc(&techDesc);

	m_technique->GetPassByIndex(0)->Apply(0);
	m_device->Draw(MAX_FIELDS * 2);

	pBuffers[0] = NULL;
	m_device->StreamOutputSetTargets(1, pBuffers, offset);
}

void ASFieldsDisplay::FieldsRenderTechnique::SecondPass()
{
	ID3D10Buffer* pBuffers[2];
	UINT stride[1] = { sizeof(VERTEX_PROTOTYPE) };
	UINT offset[1] = { 0 };
	pBuffers[0] = m_firstBuffer;
	m_device->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	m_device->SetVertexBuffers(0, 1, pBuffers, stride, offset);
	m_technique->GetPassByIndex(1)->Apply(0);
	m_device->Draw();
}

void ASFieldsDisplay::UpdateFieldsNumber(int incr)
{
	if ((m_fieldsNumber + incr <= MAX_FIELDS) && (m_fieldsNumber + incr > 0.0))
	{
		m_fieldsNumber += incr;
		m_ivFieldsNumber.Push(m_fieldsNumber);
	}
}

void ASFieldsDisplay::InitShaderResources(vector<string> vsBuf)
{
	m_rrFieldsPos = effectResourceVariable("txFieldsPos");
	m_rrFieldsGoal = effectResourceVariable("txFieldsGoal");
	m_rrFieldsStrength = effectResourceVariable("txFieldsStrength");
	m_rrFieldsTypeSize = effectResourceVariable("txFieldsTypeSize");
	m_rrMainRenderResource = effectResourceVariable("txFields");
	m_ivFieldsNumber = effectIntVariable("g_fieldNbr");
	//m_pRenderTechnique = m_device->GetTechniqueByName("RenderFields");
	m_bvEmitAtCenter = effectIntVariable("g_emAtCenter");
	m_fvWidth = effectFloatVariable("g_width");
	//m_fvWidth.Push(WIDTH / 100.0f);

	for (vector<string>::iterator it = vsBuf.begin(); it != vsBuf.end(); it++)
	{
		if ((*it) != "F")
		{
			it += 2;
		}
		else
		{
			it++;
			if ((*it) == FNBR_CFG)
			{
				it++;
				m_ivFieldsNumber.Push(atoi((*it).c_str()));
			}
			else
				it++;
		}
	}

	m_bvEmitAtCenter.Push(0);
	m_ivFieldsNumber.Push(m_fieldsNumber);
	m_vEffectResourceVariable2D.push_back(&m_rrMainRenderResource);
	m_vEffectResourceVariable1D.push_back(&m_rrFieldsPos);
	m_vEffectResourceVariable1D.push_back(&m_rrFieldsGoal);
	m_vEffectResourceVariable1D.push_back(&m_rrFieldsStrength);
	m_vEffectResourceVariable1D.push_back(&m_rrFieldsTypeSize);
}

void ASFieldsDisplay::InitViews()
{

	renderTargets2D pRenderTargets2D(m_vEffectResourceVariable2D.size(), NULL);
	renderTargets1D pRenderTargets1D(m_vEffectResourceVariable1D.size(), NULL);

	m_device->CreateTextures2D(pRenderTargets2D, WIDTH, HEIGHT);
	m_device->CreateTextures1D(pRenderTargets1D, MAX_FIELDS);

	ASDisplayObject::InitViews(pRenderTargets2D, pRenderTargets1D);

	pRenderTargets2D[0]->Release();
	pRenderTargets1D[0]->Release();
	pRenderTargets1D[1]->Release();
	pRenderTargets1D[2]->Release();
	pRenderTargets1D[3]->Release();
}

void ASFieldsDisplay::InitBuffers()
{
	m_renderTechnique.Init("UpdateFields");
}

//--------------------------------------------------------------------------------------
// Render fields on screen
//--------------------------------------------------------------------------------------
void ASFieldsDisplay::Render()
{
	switch (m_env->userInput.currentKey)
	{
		case CHANGE_TYPE			   : m_env->userInput.m_ivCurrentAction.val = CHANGE_TYPE				; break;
		case CHANGE_SIZE			   : m_env->userInput.m_ivCurrentAction.val = CHANGE_SIZE				; break;
		case CHANGE_CENTER_FORCE	   : m_env->userInput.m_ivCurrentAction.val = CHANGE_CENTER_FORCE		; break;
		case CHANGE_EXTREMITY_FORCE	   : m_env->userInput.m_ivCurrentAction.val = CHANGE_EXTREMITY_FORCE	; break;
		case CHANGE_INTERPOLATION	   : m_env->userInput.m_ivCurrentAction.val = CHANGE_INTERPOLATION		; break;
		case ADD_FIELDS:
			if (m_env->userInput.keyReleased)
				UpdateFieldsNumber(1);
			break;
		case SUB_FIELDS:
			if (m_env->userInput.keyReleased)
				UpdateFieldsNumber(-1);
			break;
		case SWITCH_VISIBILITY		   :
			if(m_env->userInput.keyReleased)
				m_areDisplayed = !m_areDisplayed;
			break;
	}
	m_fvWidth.Push(m_device->GetDimensions().x / (float(MAX_FIELDS) * 2.0f));
	//Set correct render targets
	m_device->SetRenderTargets(m_pRenderTargetViews1D, m_pDepthStencilView1D);

	m_device->ClearDepthStencilView(m_pDepthStencilView1D);
	m_renderTechnique.FirstPass();

	m_device->ClearDepthStencilView(m_pDepthStencilView2D);
	m_rrFieldsPos.Push();
	m_rrFieldsGoal.Push();
	m_rrFieldsStrength.Push();
	m_rrFieldsTypeSize.Push();
	m_device->SetRenderTargets(m_pRenderTargetViews2D, m_pDepthStencilView2D);

	// Swap field buffers
	m_renderTechnique.SwapBuffers();
	//Draw fields only if user wants to see them
	if (m_areDisplayed)
	{
		m_renderTechnique.SecondPass();
	}
}