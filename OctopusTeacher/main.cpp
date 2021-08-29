// OctopusTeacher.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "main.h"
#include "utils.h"
#include <sstream>
#include <string>
#include <ranges>
#include "../Editor/Translator.h"
#include "Octopus.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
MainComponent mainComponent;								// Wicked Engine Main Runtime Component

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

using namespace wiECS;
using namespace wiScene;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    BOOL dpi_success = SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    assert(dpi_success);

	wiStartupArguments::Parse(lpCmdLine); // if you wish to use command line arguments, here is a good place to parse them...

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_OCTOPUS_TEACHER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_OCTOPUS_TEACHER));

	// just show some basic info:
	mainComponent.infoDisplay.active = true;
	mainComponent.infoDisplay.watermark = true;
	mainComponent.infoDisplay.resolution = true;
	mainComponent.infoDisplay.fpsinfo = true;

	using namespace wiScene;
	class Myrender : public RenderPath3D
	{
		Translator translator;
		Octopus octopus;
		Entity testTarget;
		float time;
	private:

	public:
		Myrender()
		{
			const auto octopusScene = LoadModel("../CustomContent/OctopusRiggedTopo.wiscene", XMMatrixTranslation(0, 0, 15), true);

			testTarget = GetScene().Entity_CreateObject("Tentacle Target");
			const auto transform = mutableComponentFromEntity<TransformComponent>(testTarget);
			transform->translation_local = { 0, 0, 15 };
			transform->UpdateTransform();

			octopus = Octopus( testTarget, octopusScene );

			translator.Create();
			translator.enabled = true;
			translator.selected.push_back({ .entity = testTarget });
		}
		void Compose(wiGraphics::CommandList cmd) const override
		{
			RenderPath3D::Compose(cmd);
			translator.Draw(*camera, cmd);
		}
		void Update(float dt) override
		{
			time += dt;

			translator.Update(*this);

			const auto trans = mutableComponentFromEntity<TransformComponent>(octopus.octopusScene);
			trans->RotateRollPitchYaw(XMFLOAT3{ .001, 0, 0 });
			octopus.Update(time);

			RenderPath3D::Update(dt);
		}
	};

	Myrender render;
	mainComponent.ActivatePath(&render);

	MSG msg = { 0 };
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			mainComponent.Run(); // run the update - render loop (mandatory)

		}
	}

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_OCTOPUS_TEACHER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_OCTOPUS_TEACHER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd) {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);


   mainComponent.SetWindow(hWnd); // assign window handle (mandatory)


   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
	{
    case WM_COMMAND:
		{
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
			switch (wmId)
			{
			case IDM_ABOUT:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
				break;
			case IDM_EXIT:
				DestroyWindow(hWnd);
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
        }
		break;
    case WM_SIZE:
    case WM_DPICHANGED:
		if (mainComponent.is_window_active)
			mainComponent.SetWindow(hWnd);
        break;
	case WM_CHAR:
		switch (wParam)
		{
		case VK_BACK:
			if (wiBackLog::isActive())
				wiBackLog::deletefromInput();
			wiTextInputField::DeleteFromInput();
			break;
		case VK_RETURN:
			break;
			default:
			{
				const char c = (const char)(TCHAR)wParam;
				if (wiBackLog::isActive()) {
					wiBackLog::input(c);
				}
				wiTextInputField::AddInput(c);
			}
			break;
		}
		break;
	case WM_KILLFOCUS:
		mainComponent.is_window_active = false;
		break;
	case WM_SETFOCUS:
		mainComponent.is_window_active = true;
		break;
    case WM_PAINT:
		{	
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code that uses hdc here...
			EndPaint(hWnd, &ps);
		}
		break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
    return (INT_PTR)FALSE;
}
