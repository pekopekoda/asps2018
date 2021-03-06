#pragma once
#include "ASSceneObject.h"

class ASSceneObject;
class ASSceneInstance
{
protected:
	struct VERTEX_PROTOTYPE
	{};
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Same class as RenderTechnique except that it is specialized in instanced vertex buffers
	//
	ASSceneObject *m_instancer = nullptr;
	//Instances need an index buffer in addition with the vertex buffer
	ID3D10Buffer *m_firstBuffer;
	ID3D10Buffer *m_indexedBuffer;
	//Vertex buffer of an original mesh
	ASMesh		  m_instanceMesh;
	string m_meshPath = "__undefined__";

	ID3D10EffectTechnique	*m_technique;
	//Handle on the layout for the vertex buffers
	ID3D10InputLayout		*m_layout;
	//Number of instances to send in the instance vertex buffer
	UINT m_instanceCount;
	effectResourceVariable m_rrMainRenderResource;
	effectResourceVariables m_vEffectResourceVariable2D;
	//Each vertex buffer needs a layout prototype for the shader to interpret its raw data (shared_ptr<ASSceneObject> instancer, const char* meshPatheg. position, index)

	ID3D10Buffer **m_instancerBuffer;
	UINT m_sizeOfInstancerVertexPrototype;

	virtual const vector<D3D10_INPUT_ELEMENT_DESC> GetLayoutPrototype()
	{
		return{};
	}
	virtual UINT GetSizeOfVertexPrototype();
	virtual const char *GetTechniqueName();
	virtual string GetMeshPath();
	ASSceneInstance();

public:
	virtual void SetInstancer(ASSceneObject * instancer);
	virtual effectResourceVariable *GetMainRenderResource();
	virtual void FirstPass();
	virtual void InitShaderResources(vector<tuple<string, string>> vsBuf);
	virtual void InitBuffers();
	virtual void Clear();
	virtual ~ASSceneInstance();	

};

UINT ASSceneInstance::GetSizeOfVertexPrototype()
{
	return sizeof(VERTEX_PROTOTYPE);
}

const char * ASSceneInstance::GetTechniqueName()
{
	return "";
}

string ASSceneInstance::GetMeshPath()
{
	return m_meshPath;
}

void ASSceneInstance::SetInstancer(ASSceneObject *instancer)
{
	m_instancer = instancer;
}

ASSceneInstance::ASSceneInstance(){}

void ASSceneInstance::InitShaderResources(vector<tuple<string, string>> vsBuf)
{
}

void ASSceneInstance::InitBuffers()
{
	UINT sizeOfInstancerVertexPrototype = m_instancer->GetSizeOfVertexPrototype();
	m_instanceCount = m_instancer->GetMaxCount();
	m_sizeOfInstancerVertexPrototype = sizeOfInstancerVertexPrototype;

	m_technique = ASRenderer::GetTechniqueByName(GetTechniqueName());
	D3D10_PASS_DESC passDesc;
	test(m_technique->GetPassByIndex(0)->GetDesc(&passDesc));
	ASRenderer::CreateInputLayout(GetLayoutPrototype(), passDesc, &m_layout);
	string mp = GetMeshPath();
	HRESULT hr = (ASMesh::LoadFromFile(&m_instanceMesh, mp.c_str())) ? S_OK : S_FALSE;
	test(hr, "Mesh load failed");
	UINT vertexSize = m_instanceMesh.GetVertexSize();
	UINT indexSize = m_instanceMesh.GetIndexSize();
	D3D10_BUFFER_DESC vbdesc2 =
	{
		m_instanceMesh.GetVertexCount() * vertexSize,
		D3D10_USAGE_DEFAULT,
		D3D10_BIND_VERTEX_BUFFER,
		0,
		0
	};
	D3D10_SUBRESOURCE_DATA vbInitData2;
	ZeroMemory(&vbInitData2, sizeof(D3D10_SUBRESOURCE_DATA));

	vbInitData2.pSysMem = m_instanceMesh.GetMeshVertexTab();
	vbInitData2.SysMemPitch = vertexSize;

	ASRenderer::CreateBuffer(vbdesc2, vbInitData2, &m_firstBuffer);

	vbInitData2.pSysMem = m_instanceMesh.GetMeshIndexTab();
	vbInitData2.SysMemPitch = indexSize;
	vbdesc2.BindFlags = D3D10_BIND_INDEX_BUFFER;
	vbdesc2.ByteWidth = indexSize * m_instanceMesh.GetIndexCount();
	ASRenderer::CreateBuffer(vbdesc2, vbInitData2, &m_indexedBuffer);
	m_instanceMesh.Clear();
}

inline void ASSceneInstance::Clear()
{
	m_rrMainRenderResource.Release();
}

inline effectResourceVariable * ASSceneInstance::GetMainRenderResource()
{
	return &m_rrMainRenderResource;
}

void ASSceneInstance::FirstPass()
{
	ASRenderer::SetInputLayout(m_layout);
	// Set IA parameters
	ID3D10Buffer* pBuffers[2] = { m_firstBuffer, *m_instancerBuffer };
	UINT stride[2] = { GetSizeOfVertexPrototype(), m_sizeOfInstancerVertexPrototype };
	UINT offset[2] = { 0, 0 };
	ASRenderer::SetVertexBuffers(0, 2, pBuffers, stride, offset);
	ASRenderer::SetIndexBuffer(m_indexedBuffer, 0);
	ASRenderer::SetPrimitiveTopology();
	// Draw
	m_technique->GetPassByIndex(0)->Apply(0);
	ASRenderer::DrawInstance(m_instanceMesh.GetIndexCount(), m_instanceCount);
}

ASSceneInstance::~ASSceneInstance() {}
