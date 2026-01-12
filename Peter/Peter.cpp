// Peter.cpp : Defines the entry point for the application.
//

#include <windows.h>
#include <gdiplus.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <mmsystem.h>
#include <chrono>
#include <thread>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "winmm.lib")

using namespace std;
using namespace Gdiplus;

typedef struct {
    float x, y, speed, w, h, rad, dx, dy, jspeed;
    HBITMAP hbmp;
} sprite;

sprite man;
sprite enemy;
sprite plat;

struct {
    int score;
    bool active = false;
} game;

struct {
    HWND hwnd;
    HDC device_context, context;
    int w, h;
} window;

HBITMAP hBack;

void Ingame() {
    man.hbmp = (HBITMAP)LoadImageA(NULL, "chiken.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    enemy.hbmp = (HBITMAP)LoadImageA(NULL, "zombi.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    plat.hbmp = (HBITMAP)LoadImageA(NULL, "cloud.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBack = (HBITMAP)LoadImageA(NULL, "Home.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    if (!man.hbmp || !enemy.hbmp || !hBack) {
        MessageBoxA(NULL, "Failed to load game resources!", "Error", MB_ICONERROR);
    }

    man.w = 150;  
    man.h = 125;
    man.speed = 10;
    man.x = window.w / 10;
    man.y = window.h / 2;

    enemy.w = 150;
    enemy.h = 125;
    enemy.speed = 0;
    enemy.x = window.w - 200;
    enemy.y = window.h / 2 - enemy.h /2;

    plat.w = 500;
    plat.h = 200;
    plat.x = window.w / 2 - plat.w / 2;
    plat.y = window.h / 2 - plat.h / 2;

    game.score = 0;
    game.active = true;

    srand(static_cast<unsigned int>(time(NULL)));
}

void Sound(const char* name) {
}

void ShowScore() {
    SetTextColor(window.context, RGB(255, 255, 255));
    SetBkMode(window.context, TRANSPARENT);

    HFONT hFont = CreateFont(40, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, L"Arial");
    HFONT hOldFont = (HFONT)SelectObject(window.context, hFont);

    char txt[32];
    _itoa_s(game.score, txt, 10);

    TextOutA(window.context, 10, 10, "Score:", 6);
    TextOutA(window.context, 120, 10, txt, strlen(txt));

    SelectObject(window.context, hOldFont);
    DeleteObject(hFont);
}

void ProcessInput() {
    if (GetAsyncKeyState('W')) man.y -= man.speed;
    if (GetAsyncKeyState('S')) man.y += man.speed;
    if (GetAsyncKeyState('A')) man.x -= man.speed;
    if (GetAsyncKeyState('D')) man.x += man.speed;

    if (!game.active && (GetAsyncKeyState(VK_SPACE) & 0x8000)) {
        Ingame();
    }
}

void ShowBMP(HDC hdc, int x, int y, int x1, int y1, HBITMAP hbmp, bool alpha = false) {
    if (!hbmp) return;

    HDC hMemDC = CreateCompatibleDC(hdc);
    HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, hbmp);

    if (hOldBmp) {
        BITMAP bm;
        GetObject(hbmp, sizeof(BITMAP), &bm);

       
        StretchBlt(hdc, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
        
        SelectObject(hMemDC, hOldBmp);
    }
    DeleteDC(hMemDC);
}

void ShowManAndEnemy() {

    RECT rect = { 0, 0, window.w, window.h };
    HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(window.context, &rect, hBrush);
    DeleteObject(hBrush);

    ShowBMP(window.context, 0, 0, window.w, window.h, hBack);

    ShowBMP(window.context, plat.x ,plat.y , plat.w, plat.h, plat.hbmp, true);

    ShowBMP(window.context, man.x , man.y , man.w, man.h, man.hbmp, true);

    float dx = man.x - enemy.x;
    float dy = man.y - enemy.y;
    float distance = sqrt(dx * dx + dy * dy);

    if (distance > 0) {
        enemy.x += (dx / distance) * enemy.speed;
        enemy.y += (dy / distance) * enemy.speed;
    }

    ShowBMP(window.context, enemy.x , enemy.y , enemy.w, enemy.h, enemy.hbmp, true);
}

void Limit() {

    man.x = max(man.x, 0);
    man.x = min(man.x, window.w - man.w);

    man.y = max(man.y, 0);
    man.y = min(man.y, window.h - man.h);

    enemy.x = max(enemy.x, 0 );
    enemy.x = min(enemy.x, window.w - enemy.w );

    enemy.y = max(enemy.y, 0 );
    enemy.y = min(enemy.y, window.h - enemy.h);
}

void LimitPlat() {
    if (man.x >= plat.x - man.w  &&
        man.x <= plat.x + plat.w  &&
        man.y >= plat.y  - man.h  &&
        man.y <= plat.y + plat.h  ) 
    {
        float x1 = man.x + man.w - plat.x;
        float x2 = plat.x + plat.w - man.x;
        float y1 = man.y + man.h - plat.y;
        float y2 = plat.y + plat.h - man.y;
        
        float over1 = min(x1, x2);
        float over2 = min(y1, y2);
        float itog = min(over1, over2);

        if (itog == x1) {
            man.x = plat.x - man.w;
        }
        else if (itog == x2) {
            man.x = plat.x + plat.w;
        }
        else if (itog == y1) {
            man.y = plat.y - man.h;
        }
        else if (itog == y2) {
            man.y = plat.y + plat.h;
        }
    }
}

void Gravity() {

    float g = 0;
    float a = 1;
    if (man.y + man.h == window.h) {
        g += a;
        man.y = g;
        if (g <= 100) {
            g -= a;
            man.y = g;
        }
    }

}

void Fight() {

    float dx = man.x - enemy.x;
    float dy = man.y - enemy.y;
    float distance = sqrt(dx * dx + dy * dy);

    float collisionRadius = (man.w + enemy.w) / 6;

    if (distance < collisionRadius) {
        game.active = false;
        game.score = 0;
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            PostQuitMessage(0);
        }
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void InitializeWindow() {
    SetProcessDPIAware();

    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "GameWindow";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassA(&wc);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    window.hwnd = CreateWindowA("GameWindow", "Peter Game", WS_POPUP | WS_VISIBLE,
        0, 0, screenWidth, screenHeight, NULL, NULL, GetModuleHandle(NULL), NULL);

    RECT r;
    GetClientRect(window.hwnd, &r);
    window.w = r.right - r.left;
    window.h = r.bottom - r.top;

    window.device_context = GetDC(window.hwnd);
    window.context = CreateCompatibleDC(window.device_context);

    HBITMAP hBitmap = CreateCompatibleBitmap(window.device_context, window.w, window.h);
    SelectObject(window.context, hBitmap);
}

void Cleanup() {

    if (man.hbmp) DeleteObject(man.hbmp);
    if (enemy.hbmp) DeleteObject(enemy.hbmp);
    if (hBack) DeleteObject(hBack);

    if (window.context) {
        HBITMAP hBitmap = (HBITMAP)GetCurrentObject(window.context, OBJ_BITMAP);
        if (hBitmap) DeleteObject(hBitmap);
        DeleteDC(window.context);
    }

    if (window.device_context) {
        ReleaseDC(window.hwnd, window.device_context);
    }

    if (window.hwnd) {
        DestroyWindow(window.hwnd);
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hinstance,
    _In_opt_ HINSTANCE hprevinstance,
    _In_ LPWSTR lpcmdline,
    _In_ int ncmdshow) {

    ULONG_PTR gdiplusToken;
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    InitializeWindow();
    Ingame();


    ShowCursor(false);

    MSG msg = { 0 };
    while (true) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {

            ProcessInput();

            if (game.active) {

                Limit();
                LimitPlat();
                Gravity();
                Fight();

                game.score++;
            }

            ShowManAndEnemy();
            ShowScore();

            if (!game.active) {
                SetTextColor(window.context, RGB(255, 0, 0));
                SetBkMode(window.context, TRANSPARENT);

                HFONT hFont = CreateFont(60, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, L"Arial");
                HFONT hOldFont = (HFONT)SelectObject(window.context, hFont);

                TextOutA(window.context, window.w / 2 - 150, window.h / 2 - 30, "GAME OVER - PRESS SPACE", 23);

                SelectObject(window.context, hOldFont);
                DeleteObject(hFont);
            }

            BitBlt(window.device_context, 0, 0, window.w, window.h, window.context, 0, 0, SRCCOPY);

            Sleep(16);
        }
    }

    Cleanup();
    GdiplusShutdown(gdiplusToken);

    return 0;
}