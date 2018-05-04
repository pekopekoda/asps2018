#pragma once

#include "ASRenderer.h"
#include "ASMesh.h"

#define MESH_PATH  "../Medias/Meshes/"
#define TEXTURE_PATH  "../Medias/Textures/"


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
	effectResourceVariables m_vEffectResourceVariable2D;
	effectResourceVariables m_vEffectResourceVariable1D;
	effectResourceVariables m_vConstResourceVariable2D;
	effectResourceVariables m_vConstResourceVariable1D;
	//As Some objects (screen) need resource variables from other objects extraresourcenbr
	//stores the offset in the resource variable vector
	int m_extra2DResourcesNbr;
	int m_extra1DResourcesNbr;

	ASEnvironment   *m_env;
	
	renderTargetViews m_pRenderTargetViews2D;
	renderTargetViews m_pRenderTargetViews1D;
	effectResourceVariable m_rrMainRenderResource;
	
	void InitViews(textures1D& pRenderTargets1D);
	void InitViews(textures2D& pRenderTargets2D);
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
	for (auto b: m_vBuffers)
		b->Release();
	m_vBuffers.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////


void  ASSceneObject::InitViews(textures1D& pRenderTargets1D)
{
	for (auto tv : pRenderTargets1D)
		m_pRenderTargetViews1D.AddRenderTargetView(tv);
	
	m_pRenderTargetViews1D.SetRenderTargets();
	m_vEffectResourceVariable1D.Set(pRenderTargets1D);
}

void  ASSceneObject::InitViews(textures2D& pRenderTargets2D)
{
	for (auto tv : pRenderTargets2D)
		m_pRenderTargetViews2D.AddRenderTargetView(tv);

	m_pRenderTargetViews2D.SetRenderTargets();
	m_vEffectResourceVariable2D.Set(pRenderTargets2D);
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
	m_pRenderTargetViews1D.Release();
	m_pRenderTargetViews2D.Release();

	m_vEffectResourceVariable1D.Release();
	m_vEffectResourceVariable2D.Release();
	m_vConstResourceVariable1D.Release();

	return 0;						 
}

void ASSceneObject::ClearRenderTargetViews()
{
	m_pRenderTargetViews2D.ClearRenderTargets();
}

ASSceneObject::ASSceneObject(ASEnvironment *env)
{
	m_env	 = env;
}
ASSceneObject::~ASSceneObject(){}
