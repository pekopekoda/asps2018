#include <windows.h>
#include "time.h"
#include "ASScene.h"
#include "ASUserInterface.h"

//global objects
ASDisplayDevice* g_displayDevice = ASDisplayDevice::GetInstance();
typedef ASUserInterface g_ui;
ASEnvironment* g_env;
ASScene* g_scene;

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
	g_scene = ASScene::GetInstance();

	if (FAILED(g_displayDevice->InitViewport()))
	{
		g_displayDevice->Clear();
		g_scene->Clear();
		return 1;
	}
	g_displayDevice->CreateDepthStencilState();

	vector<std::string> buf;
	buf = g_ui::GetUserFileBuffer();
	g_env->InitWorld(buf);
	g_scene->Init(buf);

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
	g_scene->Clear();

    return ( int )msg.wParam;
}

void RenderFrame()
{
	g_env->PreFrame();
	g_scene->Render();
	g_env->PostFrame();

	g_displayDevice->PresentSwapChain();
}