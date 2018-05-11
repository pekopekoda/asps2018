#pragma once

#include <vector>
#include <memory>
#include "ASRenderer.h"
#include "ASUserInterface.h"
#include "ASMesh.h"

const char* g_meshPath = "../Medias/Meshes/";
const char* g_texturePath = "../Medias/Textures/";

class ASScene;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////Abstract for objects to be rendered in the scene
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ASSceneObject
{
protected:
	struct VERTEX_PROTOTYPE {};
	ASScene *m_scene;
	const UINT m_maxCount = 0;
	///////////////////////////////////////////////////////////////////////////////////////////////
	//This abstract class is an interface between the vertex buffers and related and the objects
	//to be rendered in the scene. Its job is to create a vertex buffer, make the connection between 
	//the vertices and the shader render technique and manage the result in a rendertarget
	//Vertex buffer to draw from
	ID3D10Buffer *m_firstBuffer;
	//Vertex buffer to stream to
	ID3D10Buffer *m_secondBuffer;

	//Get a handle on all the vertex buffers for abstract actions
	vector<ID3D10Buffer*>    m_vBuffers;
	//Handle on the layout for the vertex buffers
	ID3D10InputLayout		*m_layout;
	//HLSL Render technique
	const char *m_techniqueName;
	ID3D10EffectTechnique	*m_technique;
	effectResourceVariable m_rrMainRenderResource;
	//Handle to all the effect resource variables needed to render the object
	effectResourceVariables m_vEffectResourceVariable2D;
	effectResourceVariables m_vEffectResourceVariable1D;
	effectResourceVariables m_vConstResourceVariable;

	renderTargetViews m_pRenderTargetViews2D;
	renderTargetViews m_pRenderTargetViews1D;

	virtual const vector<D3D10_INPUT_ELEMENT_DESC> GetLayoutPrototype()
	{
		return{};
	}

	ASSceneObject();
	ASSceneObject(ASScene* scene);

public:
	virtual UINT GetSizeOfVertexPrototype();
	virtual UINT GetMaxCount();
	virtual const char * GetTechniqueName();
	virtual void InitShaderResources(vector<tuple<string,string>> vsBuf);
	//Returns draw from buffer
	ID3D10Buffer *GetFirstBuffer();
	
	//Swap drawfrom/streamto buffers
	void SwapBuffers();
	//Init the render technique and the input layout
	virtual void Init(const char *techniqueName);
	//render passes for each buffer to draw
	void Render();

	virtual void FirstPass();	
	//Get the render resource used for main render pass for this object
	effectResourceVariable* ASSceneObject::GetMainRenderResource();

	virtual void InitViews();
	virtual void InitViews(textures1D & pRenderTargets);
	virtual void InitViews(textures2D & pRenderTargets);
	virtual void ClearRenderTargetViews();
	virtual void Clear();

	template<typename T>
	HRESULT InitBuffers(vector<ID3D10Buffer*> vBuffers, vector<D3D10_INPUT_ELEMENT_DESC> layout, vector<T> vps, D3D10_BUFFER_DESC vbdesc);
	virtual void InitBuffers();
	virtual ~ASSceneObject();
};


UINT ASSceneObject::GetSizeOfVertexPrototype()
{
	return sizeof(VERTEX_PROTOTYPE);
}

UINT ASSceneObject::GetMaxCount()
{
	return m_maxCount;
}

const char *ASSceneObject::GetTechniqueName()
{
	return m_techniqueName;
}

ASSceneObject::ASSceneObject(){}

ASSceneObject::ASSceneObject(ASScene * scene)
{
	m_scene = scene;
}


void ASSceneObject::InitShaderResources(vector<tuple<string, string>> vsBuf)
{
}

ID3D10Buffer *ASSceneObject::GetFirstBuffer()
{
	return m_firstBuffer;
}


void ASSceneObject::SwapBuffers()
{
	// Swap field buffers
	ID3D10Buffer* pTemp;
	pTemp = m_firstBuffer;
	m_firstBuffer = m_secondBuffer;
	m_secondBuffer = pTemp;
}
void ASSceneObject::Init(const char *techniqueName)
{
	m_technique = ASRenderer::GetTechniqueByName(techniqueName);
	D3D10_PASS_DESC passDesc;
	m_technique->GetPassByIndex(0)->GetDesc(&passDesc);
	const vector<D3D10_INPUT_ELEMENT_DESC> proto = GetLayoutPrototype();
	ASRenderer::CreateInputLayout(GetLayoutPrototype() , passDesc, &m_layout);
}

void ASSceneObject::Render()
{
}

void ASSceneObject::FirstPass()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////

effectResourceVariable* ASSceneObject::GetMainRenderResource()
{
	return &m_rrMainRenderResource;
}

void ASSceneObject::InitViews()
{
}

inline void ASSceneObject::InitViews(textures1D& pRenderTargets)
{
	for (auto &tv : pRenderTargets)
		m_pRenderTargetViews1D.AddRenderTargetView(*tv);

	m_pRenderTargetViews1D.SetRenderTargets();
	m_vEffectResourceVariable1D.Set(pRenderTargets);
}
inline void ASSceneObject::InitViews(textures2D& pRenderTargets)
{
	for (auto &tv : pRenderTargets)
		m_pRenderTargetViews2D.AddRenderTargetView(*tv);

	m_pRenderTargetViews2D.SetRenderTargets();
	m_vEffectResourceVariable2D.Set(pRenderTargets);
}
template <typename T>
HRESULT ASSceneObject::InitBuffers(vector<ID3D10Buffer*> vBuffers, vector<D3D10_INPUT_ELEMENT_DESC> layout, vector<T> vps, D3D10_BUFFER_DESC vbdesc)
{
	HRESULT hr;
	// Create the input layout
	D3D10_PASS_DESC PassDesc;
	test(m_pRenderTechnique->GetPassByIndex(0)->GetDesc(&PassDesc));

	hr = ASRenderer::CreateInputLayout(layout, PassDesc, &m_pLayout);

	D3D10_SUBRESOURCE_DATA vbInitData;
	ZeroMemory(&vbInitData, sizeof(D3D10_SUBRESOURCE_DATA));

	vbInitData.pSysMem = &vps[0];
	UINT _size = sizeof(vps.data());
	vbInitData.SysMemPitch = _size;
	vbInitData.SysMemSlicePitch = _size;
	//Buffer creation
	for (auto pBuffer: vBuffers)
	{
		hr = ASRenderer::CreateBuffer(vbdesc, vbInitData, &pBuffer);
		test(hr);
	}

	return hr;
}

void ASSceneObject::Clear()
{
	m_pRenderTargetViews1D.Release();
	m_pRenderTargetViews2D.Release();
	m_rrMainRenderResource.Release();
	m_vEffectResourceVariable1D.Release();
	m_vEffectResourceVariable2D.Release();
	m_vConstResourceVariable.Release();		
	for (auto b : m_vBuffers)
		b->Release();
	m_vBuffers.clear();
}

void ASSceneObject::ClearRenderTargetViews()
{
	m_pRenderTargetViews2D.ClearRenderTargets();
}

void ASSceneObject::InitBuffers()
{
}
ASSceneObject::~ASSceneObject(){}
