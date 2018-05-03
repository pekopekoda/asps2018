#pragma once

#include "ASRenderer.h"
#include "ASMesh.h"

#define MESH_PATH  "../Medias/Meshes/"
#define TEXTURE_PATH  "../Medias/Textures/"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////FIELDS MACROS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_FIELDS 50		//number of fields in program
// Used with interface input query to warn program user may change currently picked field's parameters, or about different events :
#define CHANGE_TYPE					0x54		// type may be changed
#define CHANGE_SIZE					0x53		// size may be changed
#define CHANGE_CENTER_FORCE			0x43		// center force may be changed
#define CHANGE_EXTREMITY_FORCE		0x58		// extremity force may be changed
#define CHANGE_INTERPOLATION		0x49		// interpolation may be changed
#define ADD_FIELDS					107			// Increase the current fields number
#define SUB_FIELDS					109			// Decrease the current fields number
#define SWITCH_VISIBILITY			0x48		// visibillity on/off
#define FNBR_CFG "Fields initial number"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////PARTICLES MACROS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define PMESH_CFG "Particle mesh"
#define PTEX_CFG  "Particle texture"
#define PRAMP_CFG "Particle ramp color"
#define PNBR_CFG  "Particle number"
#define CHANGE_RATE 82							//Particle emission per second

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////SCREEN MACROS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Used with interface input query to warn program user may change currently picked field's parameters, or about different events :
#define SHADER_COLOR	97 //1   key	for color on particles
#define SHADER_TEXTURE	98 //2   key	for texture on particles
#define SHADER_TOON		99 //3   key	for toon on particles
#define SHADER_DIFFUSE	100//4   key	for diffuse and specular on particles
#define SHADER_BUMP		101//5   key	for bump on particles
#define	SHADER_DOF		102//6   key	for Depth on field on particles
#define SHADER_GLOW		103//7   key	for glow on particles
#define SENVMAP_CFG		"Environment map"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////Abstract for objects to be rendered in the scene
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ASRenderer *const ASSceneObject::RenderTechnique::m_renderer = ASRenderer::GetInstance();
class ASSceneObject
{
protected:
	///////////////////////////////////////////////////////////////////////////////////////////////
	//This abstract class is an interface between the vertex buffers and related and the objects
	//to be rendered in the scene. Its job is to create a vertex buffer, make the connection between 
	//the vertices and the shader render technique and manage the result in a rendertarget
	class RenderTechnique
	{
	protected:
		static ASRenderer *const m_renderer;
		struct VERTEX_PROTOTYPE {}; //particle vertex properties
		//Vertex buffer to draw from
		ID3D10Buffer *m_firstBuffer;
		//Vertex buffer to stream to
		ID3D10Buffer *m_secondBuffer;

		//Get a handle on all the vertex buffers for abstract actions
		vector<ID3D10Buffer*>    m_vBuffers;
		//Handle on the layout for the vertex buffers
		ID3D10InputLayout		*m_layout;
		//HLSL Render technique
		ID3D10EffectTechnique	*m_technique;

		RenderTechnique();
		//Each vertex buffer needs a layout prototype for the shader to interpret its raw data (eg. position, index)
		virtual const vector<D3D10_INPUT_ELEMENT_DESC> GetLayoutPrototype()
		{
			return{};
		}

	public:
		//Returns draw from buffer
		ID3D10Buffer *GetFirstBuffer();
		//Get size of vertex prototype
		virtual UINT GetSizeOfVertexPrototype();
		//Swap drawfrom/streamto buffers
		void SwapBuffers();
		//Init the render technique and the input layout
		virtual void Init(const char *techniqueName);
		//render passes for each buffer to draw
		void Render();
		~RenderTechnique();
	};
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Same class as RenderTechnique except that it is specialized in instanced vertex buffers
	//
	class InstanceRenderTechnique : public RenderTechnique
	{
		//Instances need an index buffer in addition with the vertex buffer
		ID3D10Buffer *m_indexedBuffer;
		//Vertex buffer of an original mesh
		ASMesh		  m_instanceMesh;
		RenderTechnique *m_pInstancer;
		//Number of instances to send in the instance vertex buffer
		UINT m_instanceCount;
	public:
		//Init render technique, layout and load mesh to instance
		void Init(const char * techniqueName, const char * meshPath, UINT instanceCount, RenderTechnique * rt);
		virtual void FirstPass();
		InstanceRenderTechnique();
		~InstanceRenderTechnique();
	};
	//Handle to all the effect resource variables needed to render the object
	vector<effectResourceVariable*> m_vEffectResourceVariable2D;
	vector<effectResourceVariable*> m_vEffectResourceVariable1D;
	vector<effectResourceVariable*> m_vConstResourceVariable2D;
	vector<effectResourceVariable*> m_vConstResourceVariable1D;
	//As Some objects (screen) need resource variables from other objects extraresourcenbr
	//stores the offset in the resource variable vector
	int m_extra2DResourcesNbr;
	int m_extra1DResourcesNbr;

	ASEnvironment   *m_env;
	
	renderTargetViews m_pRenderTargetViews2D;
	renderTargetViews m_pRenderTargetViews1D;
	effectResourceVariable m_rrMainRenderResource;
	
	void InitViews(vector<texture2D>& pRenderTargets2D, vector<texture1D>& pRenderTargets1D);
	ASSceneObject(ASEnvironment *env);
	~ASSceneObject();

public:
	//Get the render resource used for main render pass for this object
	effectResourceVariable* GetMainRenderResource();
	//Add extra resource to render from other objects
	void AddEffectResourceVariable(effectResourceVariable* rr, int dimension);
	void ClearRenderTargetViews();
	int Clear();

	template<typename T>
	HRESULT InitBuffers(vector<ID3D10Buffer*> vBuffers, vector<D3D10_INPUT_ELEMENT_DESC> layout, vector<T> vps, D3D10_BUFFER_DESC vbdesc);

};


void ASSceneObject::InstanceRenderTechnique::Init(const char *techniqueName, const char * meshPath, UINT instanceCount, RenderTechnique *rt)
{
	m_instanceCount = instanceCount;
	m_pInstancer	= rt;
	ASSceneObject::RenderTechnique::Init(techniqueName);
	HRESULT hr = (ASMesh::LoadFromFile(&m_instanceMesh, meshPath)) ? S_OK : S_FALSE;
	test(hr, "Mesh load failed");
	UINT vertexSize = m_instanceMesh.GetVertexSize();
	UINT indexSize  = m_instanceMesh.GetIndexSize();
	D3D10_BUFFER_DESC vbdesc2 =
	{
	    m_instanceMesh.GetVertexCount () * vertexSize,
	    D3D10_USAGE_DEFAULT,
	    D3D10_BIND_VERTEX_BUFFER,
	    0,
	    0
	};
	D3D10_SUBRESOURCE_DATA vbInitData2;
	ZeroMemory( &vbInitData2, sizeof( D3D10_SUBRESOURCE_DATA ) );
	
	vbInitData2.pSysMem = m_instanceMesh.GetMeshVertexTab ();
	vbInitData2.SysMemPitch = vertexSize;
	
	m_renderer->CreateBuffer(vbdesc2, vbInitData2, &m_firstBuffer);
	
	vbInitData2.pSysMem = m_instanceMesh.GetMeshIndexTab ();
	vbInitData2.SysMemPitch = indexSize;
	vbdesc2.BindFlags = D3D10_BIND_INDEX_BUFFER;
	vbdesc2.ByteWidth = indexSize * m_instanceMesh.GetIndexCount ();
	m_renderer->CreateBuffer(vbdesc2, vbInitData2, &m_indexedBuffer);
	m_instanceMesh.Clear();
}

void ASSceneObject::InstanceRenderTechnique::FirstPass()
{
	m_renderer->SetInputLayout(m_layout) ;
	// Set IA parameters
	ID3D10Buffer* pBuffers[2] = { m_firstBuffer, m_pInstancer->GetFirstBuffer() };
	UINT stride[2] = { GetSizeOfVertexPrototype(), m_pInstancer->GetSizeOfVertexPrototype() };
	UINT offset[2] = { 0, 0 };
	m_renderer->SetVertexBuffers( 0, 2, pBuffers, stride, offset );
	m_renderer->SetIndexBuffer(m_indexedBuffer, 0);
	m_renderer->SetPrimitiveTopology();
	// Draw
	m_technique->GetPassByIndex(0)->Apply(0);
	m_renderer->DrawInstance(m_instanceMesh.GetIndexCount(), m_instanceCount);
}
ASSceneObject::InstanceRenderTechnique::InstanceRenderTechnique(){}
ASSceneObject::InstanceRenderTechnique::~InstanceRenderTechnique() {}











UINT ASSceneObject::RenderTechnique::GetSizeOfVertexPrototype()
{
	return sizeof(VERTEX_PROTOTYPE);
}


ID3D10Buffer *ASSceneObject::RenderTechnique::GetFirstBuffer()
{
	return m_firstBuffer;
}


void ASSceneObject::RenderTechnique::SwapBuffers()
{
	// Swap field buffers
	ID3D10Buffer* pTemp;
	pTemp = m_firstBuffer;
	m_firstBuffer = m_secondBuffer;
	m_secondBuffer = pTemp;
}
void ASSceneObject::RenderTechnique::Init(const char *techniqueName)
{
	m_technique = m_renderer->GetTechniqueByName(techniqueName);
	D3D10_PASS_DESC passDesc;
	m_technique->GetPassByIndex(0)->GetDesc(&passDesc);
	const vector<D3D10_INPUT_ELEMENT_DESC> proto = GetLayoutPrototype();
	m_renderer->CreateInputLayout(GetLayoutPrototype() , passDesc, &m_layout);
}
void ASSceneObject::RenderTechnique::Render() {}
ASSceneObject::RenderTechnique::RenderTechnique()
{}
ASSceneObject::RenderTechnique::~RenderTechnique()
{
	for (vector<ID3D10Buffer*>::iterator it = m_vBuffers.begin(); it != m_vBuffers.end(); it++)
		(*it)->Release();
	m_vBuffers.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ASSceneObject::InitViews(vector<texture2D>& pRenderTargets2D, vector<texture1D>& pRenderTargets1D)
{
	UINT RT2DSize = pRenderTargets2D.size();
	UINT RT1DSize = pRenderTargets1D.size();
	if (RT2DSize)
	{
		for (auto tv : pRenderTargets2D)
			m_pRenderTargetViews2D.AddRenderTargetView(tv);

		auto itERV2D = m_vEffectResourceVariable2D.begin();
		auto itRTV2D = m_pRenderTargetViews2D.begin();
		for(auto pRT2D: pRenderTargets2D)
		{
			(*itERV2D)->Set(pRT2D);
			(*itRTV2D)->Set(pRT2D);
			itERV2D++;
			itRTV2D++;
		}
		D3D10_TEXTURE2D_DESC pDesc2D;
		pRenderTargets2D[0]->GetDesc(&pDesc2D);
		m_renderer->CreateDepthStencilView2D(&m_pDepthStencilView2D, pDesc2D.Width, pDesc2D.Height);
	}
	if (RT1DSize)
	{
		m_pRenderTargetViews1D.assign(RT1DSize, NULL);
		for (UINT i = 0; i < RT1DSize; i++)
		{
			if (i<m_vEffectResourceVariable1D.size())
				m_vEffectResourceVariable1D[i]->Set(pRenderTargets1D[i]);
			m_renderer->CreateRenderTargetView1D(&pRenderTargets1D[i], &m_pRenderTargetViews1D[i]);
		}
		D3D10_TEXTURE1D_DESC pDesc1D;
		pRenderTargets1D[0]->GetDesc(&pDesc1D);
		m_renderer->CreateDepthStencilView1D(&m_pDepthStencilView1D, 50);
	}
}

effectResourceVariable *ASSceneObject::GetMainRenderResource()
{
	return &m_rrMainRenderResource;
}

void ASSceneObject::AddEffectResourceVariable(effectResourceVariable* rr, int dimension)
{
	if (dimension == 2)
	{
		m_vEffectResourceVariable2D.push_back(rr);
		m_extra2DResourcesNbr += 1;
	}
	else
	{
		m_vEffectResourceVariable1D.push_back(rr);
		m_extra1DResourcesNbr += 1;
	}
}

template <typename T>
HRESULT ASSceneObject::InitBuffers(vector<ID3D10Buffer*> vBuffers, vector<D3D10_INPUT_ELEMENT_DESC> layout, vector<T> vps, D3D10_BUFFER_DESC vbdesc)
{
	HRESULT hr;
	// Create the input layout
	D3D10_PASS_DESC PassDesc;
	m_pRenderTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);

	hr = m_renderer->CreateInputLayout(layout, PassDesc, &m_pLayout);

	D3D10_SUBRESOURCE_DATA vbInitData;
	ZeroMemory(&vbInitData, sizeof(D3D10_SUBRESOURCE_DATA));

	vbInitData.pSysMem = &vps[0];
	UINT _size = sizeof(vps.data());
	vbInitData.SysMemPitch = _size;
	vbInitData.SysMemSlicePitch = _size;
	//Buffer creation
	vector<ID3D10Buffer**> vbs = GetBuffers();
	for (vector<ID3D10Buffer*>::iterator it = vBuffers.begin(); it != vBuffers.end(); it++)
	{
		ID3D10Buffer *pBuffer = *it;
		hr = m_renderer->CreateBuffer(vbdesc, vbInitData, &pBuffer);
		test(hr);
	}

	return hr;
}

int ASSceneObject::Clear()
{
	if (m_pDepthStencilView2D ) m_pDepthStencilView2D->Release();
	if (m_pDepthStencilView1D)  m_pDepthStencilView1D->Release();

	for (renderTargetViews::iterator it = m_pRenderTargetViews2D.begin(); it != m_pRenderTargetViews2D.end(); ++it)
	{
		if ((*it) != NULL)
			(*it)->Release();
	}m_pRenderTargetViews2D.clear();

	for (renderTargetViews::iterator it = m_pRenderTargetViews1D.begin(); it != m_pRenderTargetViews1D.end(); ++it)
	{
		if ((*it) != NULL)
			(*it)->Release();
	}m_pRenderTargetViews1D.clear();

	for (vector<effectResourceVariable*>::iterator it = m_vEffectResourceVariable2D.begin(); it != m_vEffectResourceVariable2D.end(); ++it)
	{
		(*it)->Release();
	}m_vEffectResourceVariable2D.clear();

	for (vector<effectResourceVariable*>::iterator it = m_vEffectResourceVariable1D.begin(); it != m_vEffectResourceVariable1D.end(); ++it)
	{
		(*it)->Release();
	}m_vEffectResourceVariable1D.clear();

	for (vector<effectResourceVariable*>::iterator it = m_vConstResourceVariable2D.begin(); it != m_vConstResourceVariable2D.end(); ++it)
	{
		(*it)->Release();
	}m_vConstResourceVariable2D.clear();

	for (vector<effectResourceVariable*>::iterator it = m_vConstResourceVariable1D.begin(); it != m_vConstResourceVariable1D.end(); ++it)
	{
		(*it)->Release();
	}m_vConstResourceVariable1D.clear();


	return 0;						 
}

void ASSceneObject::ClearRenderTargetViews()
{
	m_renderer->ClearRenderTargetViews(m_pRenderTargetViews2D);
}

ASSceneObject::ASSceneObject(ASEnvironment *env)
{
	m_env	 = env;
	m_pDepthStencilView2D = NULL;
	m_pDepthStencilView1D = NULL;
}
ASSceneObject::~ASSceneObject(){}
