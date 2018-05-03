#pragma once

#include <d3d10.h>
#include <d3dx10.h>
#include <vector>
#include <windows.h>
#include<comdef.h>
using namespace std;

#define WIDTH 1920
#define HEIGHT 1080
//Random
#define RAND(a,b) ((rand()/(float)RAND_MAX)*((b)-(a))+(a))
//Test routine for each device operation on the device
void test(HRESULT hr, char *errTitle = "Error");
void test(HRESULT hr, char *errTitle)
{
	if (hr != S_OK)
	{
		LPCSTR errMsg = _com_error(hr).ErrorMessage();
		int response = MessageBoxA(NULL, errMsg, errTitle, MB_ABORTRETRYIGNORE);
		switch (response)
		{
		case IDABORT: exit(EXIT_FAILURE);
		case IDRETRY: system("pause");
		case IDIGNORE: return;
		default: exit(EXIT_FAILURE);
		};
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
//This class is the interface between DirectX and the objects in the scene.Build
////////////////////////////////////////////////////////////////////////////////////////////
class ASRenderer
{
	friend class shaderEffect;
	friend class shaderResource;

	ASRenderer();
	~ASRenderer();
	//As we want only a unique instance, we use a singleton
	static ASRenderer* m_singleton;

	static ID3D10Device*   m_d3dDevice;


	//////////////////////////////////////////////////////////////////////////////////
	//m_window contains the dimensions of the window and a handle to the window object
	struct
	{
		//Window//////////////////////////////////////////////////////////////////////
		HINSTANCE						    hInst;									//
		HWND							    hWnd;									//
																					//Viewport////////////////////////////////////////////////////////////////////
		LONG								width;									//
		LONG								height;									//
	} m_window;																		//
																					//////////////////////////////////////////////////////////////////////////////////
	D3D10_DRIVER_TYPE				    m_d3dDriverType;							//
																					//The swap chain is the object to manage the buffer which will be presented to the screen
																					//as the final render result
	IDXGISwapChain*					    m_d3dSwapChain;								//
																					//The effect is the object allowing to create resource from C++ to the GPU/HLSL shaders
	static ID3D10Effect*				m_d3dEffect;
	//////////////////////////////////////////////////////////////////////////////////

public:

	IDXGISwapChain * GetD3DSwapChain();
	//Returns a single instance of ASRenderer as we want a unique instance during runtime
	static ASRenderer* GetInstance();

	//Open window
	HRESULT OpenWindow(HINSTANCE hInstance, int nCmdShow, LRESULT(CALLBACK *)(HWND, UINT, WPARAM, LPARAM));
	//Init viewport
	HRESULT InitViewport();
	D3DXVECTOR2 GetDimensions();
	//Called during window resize event
	void	Resize(int width, int height);

	//Return time elapsed since the program started (float whom the integer part equals the seconds)
	float GetCurrentFrameTime();
	//Return the cursor position relative to the window dimensions
	D3DXVECTOR2 GetMousePosition();

	//Link the swap chain to a render texture
	HRESULT SetSwapChain(ID3D10Texture2D **pRenderTarget);
	//Presents the final render result to the screen
	void PresentSwapChain();
	//Create the interface with the GPU
	HRESULT CreateEffect();

	//Create the depth stencil state
	HRESULT CreateDepthStencilState();
	//Create the rasterizer state
	HRESULT InitRasterizer();
	//Create the blend state
	HRESULT InitBlendstate();
	//Reserve the layout passed in argument for future vertex buffer assignation
	HRESULT CreateInputLayout(const std::vector<D3D10_INPUT_ELEMENT_DESC> pInputElementDescs, D3D10_PASS_DESC pShaderBytecodeWithInputSignature, ID3D10InputLayout**ppInputLayout);
	//Reserve vertex buffer size in memory
	HRESULT CreateBuffer(D3D10_BUFFER_DESC pDesc, D3D10_SUBRESOURCE_DATA pInitialData, ID3D10Buffer **ppBuffer);
	//Reserve vertex buffer size in memory
	HRESULT CreateBuffer(D3D10_BUFFER_DESC pDesc, ID3D10Buffer **ppBuffer);

	//Create new effect technique with specific name
	ID3D10EffectTechnique*				 GetTechniqueByName(LPCSTR techniqueName);
	//Create new effect variable with specific name
	ID3D10EffectVariable*				 GetVariableByName(LPCSTR textureName);

	//Set the layout as the current input layout before vertex draw
	void SetInputLayout(ID3D10InputLayout *pIL);
	//Used with the geometry shader to determine the tessellation of the extra vertices draw
	void SetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY prim = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//Set this veertex buffer as the current input vertex buffer before draw
	void SetVertexBuffers(UINT StartSlot, UINT NumBuffers, ID3D10Buffer **ppVertexBuffers, UINT *pStrides, UINT *pOffsets);
	//For vertex instancing (used with particles for example), set an index buffer
	void SetIndexBuffer(ID3D10Buffer * ppVertexBuffer, UINT offset, DXGI_FORMAT format = DXGI_FORMAT_R32_UINT);
	//Set the target buffer for the vertex draw
	void StreamOutputSetTargets(UINT nbr, ID3D10Buffer** pBuffers, UINT* offset);
	//Set the shader resources
	void SetShaderResources();

	//swap draw and stream buffers
	void SwapBuffers();
	//Draw vertices to render
	void Draw(UINT vtxCnt = 0);
	//Draw instance vertices to render
	void DrawInstance(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation = 0, INT BaseVertexLocation = 0, UINT StartInstanceLocation = 0);
	int Clear();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


HRESULT ASRenderer::SetSwapChain(ID3D10Texture2D **pRenderTarget)
{
	HRESULT hr = S_OK;
	//Set swap chain buffer///////////////////////////////////////////////////////////////////////////////
	hr = m_d3dSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*)pRenderTarget);
	test(hr);
	return hr;
}

void ASRenderer::PresentSwapChain()
{
	m_d3dSwapChain->Present(0, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

ASRenderer* ASRenderer::m_singleton = NULL;
ASRenderer* ASRenderer::GetInstance()
{
	if (m_singleton == NULL)
	{
		m_singleton = new ASRenderer();
		return m_singleton;
	}

	return m_singleton;
}

IDXGISwapChain* ASRenderer::GetD3DSwapChain()
{
	return m_d3dSwapChain;
}
//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT ASRenderer::OpenWindow(HINSTANCE hInstance, int nCmdShow, LRESULT(CALLBACK *WndProc)(HWND, UINT, WPARAM, LPARAM))
{
	HRESULT hr = S_OK;
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, LPCSTR("iconName"));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = LPCSTR("TutorialWindowClass");
	wcex.hIconSm = LoadIcon(wcex.hInstance, LPCSTR("iconName"));
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	// Create window
	m_window.hInst = hInstance;
	RECT rc = { 0, 0, m_window.width, m_window.height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	m_window.hWnd = CreateWindow(LPCSTR("TutorialWindowClass"), ("ASParticeSystem"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
		NULL);
	if (!(&m_window.hWnd))
		return E_FAIL;

	ShowWindow(m_window.hWnd, nCmdShow);

	//Direct3D device///////////////////////////////////////////////////////////////////////////////////
	/*RECT rc;
	GetClientRect(m_window.hWnd, &rc);
	m_window.width = rc.right - rc.left;
	m_window.height = rc.bottom - rc.top;*/

	UINT createDeviceFlags = 0;
	//#ifdef _DEBUG
	//	createDeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
	//#endif

	D3D10_DRIVER_TYPE driverTypes[] =
	{
		D3D10_DRIVER_TYPE_HARDWARE,
		D3D10_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = sizeof(driverTypes) / sizeof(driverTypes[0]);
	/****************************************************************************************************/

	//Swap chain//////////////////////////////////////////////////////////////////////////////////////////
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = m_window.width;
	sd.BufferDesc.Height = m_window.height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = m_window.hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		m_d3dDriverType = driverTypes[driverTypeIndex];
		hr = D3D10CreateDeviceAndSwapChain(NULL, m_d3dDriverType, NULL, createDeviceFlags,
			D3D10_SDK_VERSION, &sd, &m_d3dSwapChain, &m_d3dDevice);
		if (SUCCEEDED(hr))
			break;
	}
	return hr;
}

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT ASRenderer::InitViewport()
{
	HRESULT hr = S_OK;

	// Setup the viewport/////////////////////////////////////////////////////////////////////////////////
	D3D10_VIEWPORT vp;
	vp.Width = m_window.width;
	vp.Height = m_window.height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_d3dDevice->RSSetViewports(1, &vp);
	/****************************************************************************************************/

	return hr;
}

D3DXVECTOR2 ASRenderer::GetDimensions()
{
	return D3DXVECTOR2(m_window.width, m_window.height);
}
void ASRenderer::Resize(int width, int height)
{
	m_window.width = width;
	m_window.height = height;
}

float ASRenderer::GetCurrentFrameTime()
{
	// Update our time
	float t = 0.0f;
	if (m_d3dDriverType == D3D10_DRIVER_TYPE_REFERENCE)
	{
		t += (float)D3DX_PI * 0.0125f;
	}
	else
	{
		static DWORD dwTimeStart = 0;
		DWORD dwTimeCur = GetTickCount();
		if (dwTimeStart == 0)
			dwTimeStart = dwTimeCur;
		t = (dwTimeCur - dwTimeStart) / 1000.0f;
	}
	return t;
}

D3DXVECTOR2 ASRenderer::GetMousePosition()
{
	POINT ptCursor;
	D3DXVECTOR2 mp;
	ptCursor.x = long(mp.x);
	ptCursor.y = long(mp.y);
	ScreenToClient(m_window.hWnd, &ptCursor);
	return mp;
}

HRESULT ASRenderer::CreateDepthStencilState()
{
	HRESULT hr = S_OK;
	//Depth buffer creation
	D3D10_DEPTH_STENCIL_DESC dsDesc;
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D10_COMPARISON_GREATER;
	dsDesc.StencilEnable = false;
	//Depth stencil state
	ID3D10DepthStencilState* pDSState;
	hr = m_d3dDevice->CreateDepthStencilState(&dsDesc, &pDSState);
	test(hr);
	m_d3dDevice->OMSetDepthStencilState(pDSState, 0);
	test(hr);
	return hr;

}

HRESULT ASRenderer::InitRasterizer()
{
	//Rasterizer//////////////////////////////////////////////////////////////////////////////////////////
	D3D10_RASTERIZER_DESC rasterizerState;
	rasterizerState.CullMode = D3D10_CULL_BACK;
	rasterizerState.FillMode = D3D10_FILL_SOLID;
	rasterizerState.FrontCounterClockwise = false;
	rasterizerState.DepthBias = false;
	rasterizerState.DepthBiasClamp = 0;
	rasterizerState.SlopeScaledDepthBias = 0;
	rasterizerState.DepthClipEnable = true;
	rasterizerState.ScissorEnable = false;
	rasterizerState.MultisampleEnable = false;
	rasterizerState.AntialiasedLineEnable = true;
	ID3D10RasterizerState* pRS;
	HRESULT hr = m_d3dDevice->CreateRasterizerState(&rasterizerState, &pRS);
	test(hr);
	m_d3dDevice->RSSetState(pRS);
	/****************************************************************************************************/
	return S_OK;
}
HRESULT ASRenderer::InitBlendstate()
{
	HRESULT hr = S_OK;
	//Creating blend state///////////////////////////////////////////////////////////////////////////////
	ID3D10BlendState* g_pBlendState = NULL;
	D3D10_BLEND_DESC BlendState;
	ZeroMemory(&BlendState, sizeof(D3D10_BLEND_DESC));
	BlendState.BlendEnable[0] = TRUE;
	BlendState.BlendEnable[1] = TRUE;
	BlendState.SrcBlend = D3D10_BLEND_SRC_ALPHA;
	BlendState.DestBlend = D3D10_BLEND_INV_SRC_ALPHA;
	BlendState.BlendOp = D3D10_BLEND_OP_ADD;
	BlendState.SrcBlendAlpha = D3D10_BLEND_ZERO;
	BlendState.DestBlendAlpha = D3D10_BLEND_ZERO;
	BlendState.BlendOpAlpha = D3D10_BLEND_OP_ADD;
	BlendState.RenderTargetWriteMask[0] = D3D10_COLOR_WRITE_ENABLE_ALL;
	BlendState.RenderTargetWriteMask[1] = D3D10_COLOR_WRITE_ENABLE_ALL;
	hr = m_d3dDevice->CreateBlendState(&BlendState, &g_pBlendState);
	test(hr);
	float _r[4] = { 0, 0, 0, 0 };
	m_d3dDevice->OMSetBlendState(g_pBlendState, _r, 0xffffffff);
	/****************************************************************************************************/
	return hr;
}

HRESULT ASRenderer::CreateEffect()
{
	HRESULT hr = S_OK;
	// Create the effect/////////////////////////////////////////////////////////////////////////////////////
	DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
	//#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3D10_SHADER_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	//dwShaderFlags |= D3D10_SHADER_DEBUG;
	//#endif

	ID3D10Blob *compErrors;
	hr = D3DX10CreateEffectFromFile(("ASEntryPoint.fx"), NULL, NULL, "fx_4_0", dwShaderFlags, 0, m_d3dDevice, NULL,
		NULL, &m_d3dEffect, &compErrors, NULL);
	char* pCompileErrors = static_cast<char*>(compErrors->GetBufferPointer());
	test(hr, pCompileErrors);
	return hr;
}

HRESULT ASRenderer::CreateInputLayout(const std::vector<D3D10_INPUT_ELEMENT_DESC> pInputElementDescs, D3D10_PASS_DESC pShaderBytecodeWithInputSignature, ID3D10InputLayout **ppInputLayout)
{
	int size = sizeof(D3D10_INPUT_ELEMENT_DESC) * pInputElementDescs.size();
	HRESULT hr = m_d3dDevice->CreateInputLayout(&pInputElementDescs[0], size / sizeof(pInputElementDescs[0]),
		pShaderBytecodeWithInputSignature.pIAInputSignature,
		pShaderBytecodeWithInputSignature.IAInputSignatureSize, ppInputLayout);
	test(hr);
	return hr;
}

HRESULT ASRenderer::CreateBuffer(D3D10_BUFFER_DESC pDesc, D3D10_SUBRESOURCE_DATA pInitialData, ID3D10Buffer **ppBuffer)
{
	HRESULT hr = m_d3dDevice->CreateBuffer(&pDesc, &pInitialData, ppBuffer);
	test(hr);
	return hr;
}

HRESULT ASRenderer::CreateBuffer(D3D10_BUFFER_DESC pDesc, ID3D10Buffer ** ppBuffer)
{
	HRESULT hr = m_d3dDevice->CreateBuffer(&pDesc, NULL, ppBuffer);
	test(hr);
	return hr;
}

ID3D10EffectTechnique* ASRenderer::GetTechniqueByName(LPCSTR techniqueName)
{
	return m_d3dEffect->GetTechniqueByName(techniqueName);
}

void ASRenderer::SetInputLayout(ID3D10InputLayout *pIL)
{
	m_d3dDevice->IASetInputLayout(pIL);
}

void ASRenderer::SwapBuffers()
{
	m_d3dSwapChain->Present(0, 0);
}

void ASRenderer::Draw(UINT vtxCnt)
{
	if (vtxCnt)
		m_d3dDevice->Draw(vtxCnt, 0);
	else
		m_d3dDevice->DrawAuto();
}

void ASRenderer::DrawInstance(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
{
	m_d3dDevice->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

void ASRenderer::SetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY prim)
{
	m_d3dDevice->IASetPrimitiveTopology(prim);
}

void ASRenderer::SetVertexBuffers(UINT start, UINT num, ID3D10Buffer **pB, UINT *pS, UINT *pO)
{
	m_d3dDevice->IASetVertexBuffers(start, num, pB, pS, pO);
}

void ASRenderer::SetIndexBuffer(ID3D10Buffer * ppVertexBuffer, UINT offset, DXGI_FORMAT format)
{
	m_d3dDevice->IASetIndexBuffer(ppVertexBuffer, format, offset);
}

void ASRenderer::StreamOutputSetTargets(UINT nbr, ID3D10Buffer** pBuffers, UINT* offset)
{
	m_d3dDevice->SOSetTargets(nbr, pBuffers, offset);
}

void ASRenderer::SetShaderResources()
{
	ID3D10ShaderResourceView *const pSRV[6] = { NULL, NULL, NULL, NULL, NULL, NULL };
	m_d3dDevice->GSSetShaderResources(0, 4, pSRV);
	m_d3dDevice->PSSetShaderResources(0, 5, pSRV);
}

int ASRenderer::Clear()
{

	if (m_d3dDevice)	m_d3dDevice->ClearState();
	if (m_d3dDevice)	m_d3dDevice->Release();
	if (m_d3dSwapChain)	m_d3dSwapChain->Release();
	if (m_d3dEffect)	m_d3dEffect->Release();

	if (m_singleton != NULL)
	{
		delete(m_singleton);
		m_singleton = NULL;
	}
	return 0;
}

ASRenderer::ASRenderer()
{
	m_window.width = WIDTH;
	m_window.height = HEIGHT;
	m_window.hInst = NULL;
	m_window.hWnd = NULL;
	m_d3dDriverType = D3D10_DRIVER_TYPE_NULL;
	m_d3dDevice = NULL;
	m_d3dSwapChain = NULL;
	m_d3dEffect = NULL;

	
}
ASRenderer::~ASRenderer() {}

////////////////////////////////////////////////////////////////////////////////////////////
//Generic constructors and typedef
////////////////////////////////////////////////////////////////////////////////////////////
//shader resources containers
using renderTargets1D = vector<ID3D10Texture1D*>;
using renderTargets2D = vector<ID3D10Texture2D*>;
using renderTargetViews = vector<ID3D10RenderTargetView*>;


const ID3D10Effect *shaderEffect::m_d3dEffect = ASRenderer::m_d3dEffect;
class shaderEffect
{
protected:
	static const ID3D10Effect *m_d3dEffect;
};

const ID3D10Device *shaderResource::m_d3dDevice = ASRenderer::m_d3dDevice;
class shaderResource
{
protected:
	static const ID3D10Device *m_d3dDevice;
};

////////////////////////////////////////////////////////////////////////////////////////////
//These classes manage the render resources variables. Theses variables are used to send informations
//From the C++ part to the shader part through variable names.
template<class T>
class effectScalarVariable:public shaderEffect
{
protected:
	//The variable used to pass int and float values
	ID3D10EffectScalarVariable * var;
	virtual void Init(char* name)
	{
		//Get the corresponding variable in the shader by its name
		var = m_d3dEffect->GetVariableByName(name)->AsScalar();
	}
	effectScalarVariable() {}

public:
	//val is the value to pass to the shader variable. it is updated during cpu runtime
	T val;
};
class effectIntVariable : public effectScalarVariable<int>
{
public:
	effectIntVariable() {}
	void Push()
	{
		//Sends the value updated to the shader variable
		var->SetInt(val);
	}
	void Push(int newVal)
	{
		val = newVal;
		Push();
	}
	effectIntVariable(char* name) { Init(name); }
};
class effectFloatVariable : public effectScalarVariable<float>
{
public:
	effectFloatVariable() {}
	void Push()
	{
		//Sends the value updated to the shader variable
		var->SetFloat(val);
	}
	void Push(float newVal)
	{
		val = newVal;
		Push();
	}
	effectFloatVariable(char* name) { Init(name); }
};

class effectVectorVariable
{
	ID3D10EffectVectorVariable *var;
	virtual void Init(char* name)
	{
		//Get the corresponding variable in the shader by its name
		var = ASRenderer::GetInstance()->GetVariableByName(name)->AsVector();
	}
public:
	float x, y, z;
	effectVectorVariable() {}
	void Push(D3DXVECTOR3 v)
	{
		//Sends the value updated to the shader variable
		x = v.x;
		y = v.y;
		z = v.z;
		var->SetFloatVector(v);
	}
	void Push(float _x, float _y, float _z)
	{
		Push(D3DXVECTOR3(_x, _y, _z));
	}
	effectVectorVariable(char* name) { Init(name); }
};

class effectMatrixVariable
{
	ID3D10EffectMatrixVariable* var;
public:
	D3DXMATRIX m;
	effectMatrixVariable() {}
	effectMatrixVariable(char* name)
	{
		//Get the corresponding variable in the shader by its name
		var = ASRenderer::GetInstance()->GetVariableByName(name)->AsMatrix();
	}

	void Push()
	{
		//Sends the value updated to the shader variable
		var->SetMatrix((float*)&m);
	}

	void Push(D3DXMATRIX newM)
	{
		m = newM;
		Push();
	}
};

////////////////////////////////////////////////////////////////////////////////////////////
//These classes manage the render resources views (a render resource is a texture on which a previous pass has been rendered. It is 
//then usually used by another pass to retrieve informations of the previous render pass).
class effectResourceVariable
{
	//The output texture on which the rendered pass will be stored
	ID3D10EffectShaderResourceVariable* ESRV;
public:
	//The input texture the next pass will read to get informations of the previous pass
	ID3D10ShaderResourceView * SRV = NULL;
	effectResourceVariable() {}
	effectResourceVariable(const char *name)
	{
		//The connection between the two textures is made by the input texture name.
		ESRV = ASRenderer::GetInstance()->GetVariableByName(name)->AsShaderResource();
	}
	void Set(ID3D10Texture2D *t)
	{
		//Initialization with a texture
		ASRenderer::GetInstance()->CreateShaderResourceView2D(&t, &SRV);
	}
	void Set(ID3D10Texture1D *t)
	{
		//Initialization with a texture
		ASRenderer::GetInstance()->CreateShaderResourceView1D(&t, &SRV);
	}
	void SetFromFile(const char *path)
	{
		//Initialization with a file. It won't be necessary to push any output texture then.
		ASRenderer::GetInstance()->CreateShaderResourceViewFromFile(&SRV, path);
	}
	void Push()
	{
		//Push the output texture to the shader input texture
		ESRV->SetResource(SRV);
	}
	void Release()
	{
		if (SRV != NULL)
		{
			SRV->Release();
			SRV = NULL;
		}
	}
};