/*
	kpu-warp-winapi-framework		Copyright ⓒ https://github.com/KPU-WARP

	#0. 해당 프로젝트는 한국산업기술대 게임공학과 학술 소모임 WARP의 오픈소스 WinAPI Framework입니다.
	#1. 관련 세부 정보는 깃헙 저장소 리드미에서 확인하실 수 있습니다.
		- https://github.com/KPU-WARP/winapi-framework

	#2. 기타 문의는, KoreaGameMaker@gmail.com으로 부탁드립니다. 감사합니다 :)
*/

#include "stdafx.h"
#include "Client.h"
#include "ClientDefine.h"
#include "GameFramework.h"

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[GLOBAL_DEFINE::MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[GLOBAL_DEFINE::MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

static std::unique_ptr<WGameFramework> gGameFramework;

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	// 편한 디버깅 환경을 제공하기 위해, 개발 모드일 때, 콘솔창을 켜줍니다. 
	//	-> 과제 2 변경사항으로 항상 콘솔창을 켜줍니다.
#ifdef UNICODE
	UNICODE_UTIL::SetLocaleToKorean();
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console") 
#else
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console") 
#endif
	gGameFramework = std::make_unique<WGameFramework>(
		[/* void */]() noexcept(false) -> const std::string /*== IpAddress*/
			{
				std::cout << "\n"
					<< "	접속을 희망하는 IP 주소를 선택하세요. \n"
					<< "		1. Local Host (127.0.0.1) \n"
					<< "		2. 직접 입력 \n"
					<< "								==> ";

				int inputtedIPType{};
				std::cin >> inputtedIPType;

				if (inputtedIPType == 1) { system("cls"); return "127.0.0.1"; }
				if (inputtedIPType == 2)
				{
					std::cout << "\n"
						<< "	IP 주소를 입력해주세요. : ";

					std::string inputtedIP{};
					std::cin >> inputtedIP;

					system("cls");

					return inputtedIP;
				}
				
				std::cout << " 정의되지 않은 커맨드 입니다. 클라이언트를 종료합니다. ";
				throw ERROR;
			}()
		);
    
	UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, GLOBAL_DEFINE::MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CLIENT, szWindowClass, GLOBAL_DEFINE::MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 응용 프로그램 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow)) return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENT));
    MSG msg;

	while (true)
	{
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;
			if (!::TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
	}
	
    return (int) msg.wParam;
}


//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = NULL;
    wcex.cbWndExtra     = NULL;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WARP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;//MAKEINTRESOURCEW(IDC_CLIENT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_WARP));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(
	   szWindowClass,
	   szTitle,
	   WS_OVERLAPPEDWINDOW,
	   CW_USEDEFAULT, 
	   0, 
	   GLOBAL_DEFINE::FRAME_WIDTH,
	   GLOBAL_DEFINE::FRAME_HEIGHT,
	   nullptr,
	   nullptr,
	   hInstance,
	   nullptr
   );

   if (!hWnd) return FALSE;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 응용 프로그램 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
		{
			gGameFramework->Create(hWnd);
			SetTimer(hWnd, GLOBAL_DEFINE::MAIN_TIMER, GLOBAL_DEFINE::MAIN_TIMER_FRAME, NULL);
		}
		break;

		case WM_PAINT:
		{
#pragma region [FOR DOUBLE BUFFER]
			PAINTSTRUCT ps;
			HDC mainHDC = BeginPaint(hWnd, &ps);
			HBITMAP GLay = CreateCompatibleBitmap(mainHDC, GLOBAL_DEFINE::FRAME_WIDTH, GLOBAL_DEFINE::FRAME_HEIGHT);
			HDC GLayDC = CreateCompatibleDC(mainHDC);
			SelectObject(GLayDC, GLay);
#pragma endregion

			gGameFramework->OnDraw(GLayDC);

#pragma region [FOR DOUBLE BUFFER]
			BitBlt(mainHDC, 0, 0, GLOBAL_DEFINE::FRAME_WIDTH, GLOBAL_DEFINE::FRAME_HEIGHT, GLayDC, 0, 0, SRCCOPY);
			DeleteDC(GLayDC);
			DeleteObject(GLay);
			EndPaint(hWnd, &ps);
#pragma endregion
		}
		break;

		case WM_TIMER:
		{
			gGameFramework->OnUpdate();
			InvalidateRgn(hWnd, NULL, false);
		}
		break;

		case WM_LBUTTONDOWN:
		{
			gGameFramework->Mouse(message, wParam, lParam);
		}
		break;

		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			gGameFramework->KeyBoard(message, wParam, lParam);
		}
		break;

		case WM_DESTROY:
		{	
			PostQuitMessage(0);
		}
		break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}