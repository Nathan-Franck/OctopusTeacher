// TemplateWindows.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "main.h"
#include <sstream>
#include <string>
#include <ranges>

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

template<class T>
class getComponentsUnderParentResult  {
public:
	std::vector<T*> components;
	std::vector<size_t> indices;
};

using namespace wiECS;
using namespace wiScene;
template<class T>
getComponentsUnderParentResult<T> getComponentsUnderParent(
	ComponentManager<T>& manager,
	Entity parent
) {
	std::vector<T*> components;
	std::vector<size_t> componentIndices;
	for (int i = 0; i < GetScene().hierarchy.GetCount(); i++) {
		auto heir = GetScene().hierarchy[i];
		auto enti = GetScene().hierarchy.GetEntity(i);
		if (heir.parentID == parent) {
			auto component = manager.GetComponent(enti);
			if (component != nullptr) {
				components.push_back(component);
				componentIndices.push_back(manager.GetIndex(enti));
			}
		}
	}
	return { components, componentIndices };
}

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
    LoadStringW(hInstance, IDC_TEMPLATEWINDOWS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TEMPLATEWINDOWS));

	// just show some basic info:
	mainComponent.infoDisplay.active = true;
	mainComponent.infoDisplay.watermark = true;
	mainComponent.infoDisplay.resolution = true;
	mainComponent.infoDisplay.fpsinfo = true;

	using namespace wiScene;
	class Myrender : public RenderPath3D {
		wiSprite sprite;
		wiSpriteFont font;
		Entity teapot;
		Entity octopus;
		float time;
	private:

	public:
		Myrender() {
			sprite = wiSprite("../Content/logo_small.png");
			sprite.params.pos = XMFLOAT3(100, 100, 0);
			sprite.params.siz = XMFLOAT2(256, 256);
			sprite.anim.wobbleAnim.amount = XMFLOAT2(.2f, .2f);
			AddSprite(&sprite);

			font.SetText("Hello World!");
			font.params.posX = 100;
			font.params.posY = sprite.params.pos.y + sprite.params.siz.y;
			font.params.size = 42;
			AddFont(&font);

			teapot = LoadModel("../Content/models/teapot.wiscene", XMMatrixTranslation(0, 0, 10), true);
			octopus = LoadModel("../CustomContent/OctopusRiggedTopo.wiscene", XMMatrixTranslation(0, -5, 15), true);
			//Scene scene;
			//ImportModel_OBJ("../CustomContent/OctopusRiggedTopo.obj", scene);
			//GetScene().Merge(scene);
		}
		void Update(float dt) override {
			time += dt;
			TransformComponent* transform = GetScene().transforms.GetComponent(teapot);
			if (transform != nullptr) {
				transform->RotateRollPitchYaw(XMFLOAT3(0, 1.0f * dt, 0));
			}

			// Gather entity/components from scene to animate

			auto isOctopus = [](NameComponent* component) {
				return component->name.compare("OctopusRiggedTopo.glb") == 0;
			};
			auto isArmature = [](NameComponent* component) {
				return component->name.compare("Armature") == 0;
			};

			auto nameComponentsResult = getComponentsUnderParent(GetScene().names, octopus);
			auto names = nameComponentsResult.components;
			auto octopusModel = std::find_if(names.begin(), names.end(), isOctopus);
			auto indexFromResult = std::distance(names.begin(), octopusModel);
			auto names_octopus = getComponentsUnderParent(
				GetScene().names,
				GetScene().names.GetEntity(nameComponentsResult.indices[indexFromResult])
			).components;
			auto armature = names_octopus | std::views::filter(isArmature);
			std::vector vec{ 1, 2, 3, 4, 5, 6 };
			auto v = std::views::reverse(vec);
			//auto indexFromResult_armature = std::distance(names_octopus.begin(), armature);
			//auto armature = std::find_if(names_2.begin(), names_2.end(), [](NameComponent* component) {
			//	std::stringstream ss;
			//	ss << std::endl << component->name << std::endl;
			//	wiBackLog::post(ss.str().c_str());
			//	return component->name.compare("Armature") == 0;
			//});

			//std::stringstream ss;
			//ss << std::endl << names_2[0]->name << std::endl;
			//wiBackLog::post(ss.str().c_str());

			auto strobeLights = getComponentsUnderParent(GetScene().lights, teapot).components;
			for (int i = 0; i < strobeLights.size(); i++) {
				strobeLights[i]->color = { cos(time * 10 * 3.14f) * 0.5f + 0.5f, 0, 0 };
			}

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
ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TEMPLATEWINDOWS));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TEMPLATEWINDOWS);
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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId) {
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
		switch (wParam) {
		case VK_BACK:
			if (wiBackLog::isActive())
				wiBackLog::deletefromInput();
			wiTextInputField::DeleteFromInput();
			break;
		case VK_RETURN:
			break;
		default: {
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
    case WM_PAINT: {
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
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
