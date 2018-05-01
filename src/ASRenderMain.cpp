#include <windows.h>
#include "time.h"
#include "ASDisplayDevice.h"
#include "ASScreen.h"
#include "ASFields.h"
#include "ASParticles.h"
#include "ASUserInterface.h"
#include "ASEnvironment.h"

//global objects
ASDisplayDevice* g_displayDevice = ASDisplayDevice::GetInstance();
typedef ASUserInterface g_ui;
ASEnvironment*		g_env		;
ASScreenDisplay*    g_screen	;
ASFieldsDisplay*    g_fields	;
ASParticlesDisplay* g_particles ;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow);
void RenderFrame();

////--------------------------------------------------------------------------------------
//// Called every time the application receives a message
////--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT _rc;

	if(g_ui::GetInput(message, wParam, lParam))
		return 0;
	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &_rc);
		g_displayDevice->Resize(_rc.right, _rc.bottom);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

	//Function pointer of WndProc to pass within displayDevice init
	LRESULT(CALLBACK *proc)(HWND, UINT, WPARAM, LPARAM);
	proc = WndProc;

	if(FAILED(g_displayDevice->OpenWindow(hInstance, nCmdShow, proc)))
		return 1;

	srand((unsigned)time(NULL));

	g_displayDevice->InitRasterizer();
	g_displayDevice->InitBlendstate();
	g_displayDevice->CreateEffect();


	g_env = ASEnvironment::GetInstance();
	g_screen = new ASScreenDisplay();
	g_fields = new ASFieldsDisplay();
	g_particles = new ASParticlesDisplay();

	if (FAILED(g_displayDevice->InitViewport()))
	{
		g_displayDevice->Clear();
		g_screen->Clear();
		g_fields->Clear();
		g_particles->Clear();
		return 1;
	}

	vector<std::string> buf;
	buf = g_ui::GetUserFileBuffer();
	g_env->InitWorld(buf);
	g_screen->InitShaderResources(buf);
	g_fields->InitShaderResources(buf);
	g_particles->InitShaderResources(buf);

	g_displayDevice->CreateDepthStencilState();

	g_screen->InitViews();
	g_fields->InitViews();
	g_particles->InitViews();
	g_screen->AddEffectResourceVariable(g_fields->GetMainRenderResource(), 2);
	g_screen->AddEffectResourceVariable(g_particles->GetMainRenderResource(), 2);
	g_screen->AddEffectResourceVariable(g_particles->GetInstanceRenderResource(), 2);

	g_screen->InitBuffers();
	g_fields->InitBuffers();
	g_particles->InitBuffers();

    // Main message loop
    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
       else
        {
            RenderFrame();
        }
    }
	
	g_displayDevice->Clear();
	g_screen->Clear();
	g_fields->Clear();
	g_particles->Clear();

    return ( int )msg.wParam;
}

void RenderFrame()
{
	g_env->PreFrame();
	g_fields->PreRender();
	g_displayDevice->SetShaderResources();
	g_fields->Render();
	g_particles->PreRender();
	g_particles->Render();
	g_screen->Render();
	g_env->PostFrame();

	g_displayDevice->PresentSwapChain();
}