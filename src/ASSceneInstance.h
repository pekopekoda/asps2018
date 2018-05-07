#pragma once
#include "ASSceneObject.h"

class ASSceneInstance
{
protected:
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Same class as RenderTechnique except that it is specialized in instanced vertex buffers
	//
	const unique_ptr<ASSceneObject> m_instancer;
	//Instances need an index buffer in addition with the vertex buffer
	ID3D10Buffer *m_firstBuffer;
	ID3D10Buffer *m_indexedBuffer;
	//Vertex buffer of an original mesh
	ASMesh		  m_instanceMesh;
	const char* m_meshPath;

	ID3D10EffectTechnique	*m_technique;
	//Handle on the layout for the vertex buffers
	ID3D10InputLayout		*m_layout;
	//Number of instances to send in the instance vertex buffer
	UINT m_instanceCount;
	effectResourceVariable m_rrMainRenderResource;
	//Each vertex buffer needs a layout prototype for the shader to interpret its raw data (shared_ptr<ASSceneObject> instancer, const char* meshPatheg. position, index)

	ID3D10Buffer **m_instancerBuffer;
	UINT m_sizeOfInstancerVertexPrototype;

	virtual const vector<D3D10_INPUT_ELEMENT_DESC> GetLayoutPrototype()
	{
		return{};
	}
	virtual UINT GetSizeOfVertexPrototype();

	ASSceneInstance(ASSceneObject *instancer, const char* meshPath);
	~ASSceneInstance();

public:
	effectResourceVariable * GetMainRenderResource();
	void FirstPass();
	void Init(const char *techniqueName, const char * meshPath, UINT instanceCount, ID3D10Buffer *instancerBuffer, UINT sizeOfInstancerVertexPrototype);
	void InitShaderResources(vector<tuple<string, string>> vsBuf);
	void InitBuffers();
	void Clear();

};


void ASSceneInstance::Init(const char *techniqueName, const char * meshPath, UINT instanceCount, ID3D10Buffer *instancerBuffer, UINT sizeOfInstancerVertexPrototype)
{
	m_instanceCount = instanceCount;
	m_instancerBuffer = &instancerBuffer;
	m_sizeOfInstancerVertexPrototype = sizeOfInstancerVertexPrototype;

	m_technique = ASRenderer::GetTechniqueByName(techniqueName);
	D3D10_PASS_DESC passDesc;
	m_technique->GetPassByIndex(0)->GetDesc(&passDesc);
	const vector<D3D10_INPUT_ELEMENT_DESC> proto = GetLayoutPrototype();
	ASRenderer::CreateInputLayout(GetLayoutPrototype(), passDesc, &m_layout);

	HRESULT hr = (ASMesh::LoadFromFile(&m_instanceMesh, meshPath)) ? S_OK : S_FALSE;
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

ASSceneInstance::ASSceneInstance(ASSceneObject *instancer, const char* meshPath) : m_instancer(make_unique<ASSceneObject>(instancer)), g_meshPath(meshPath)
{}
ASSceneInstance::~ASSceneInstance() {}
