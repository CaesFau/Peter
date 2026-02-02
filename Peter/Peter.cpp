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
    float x, y, speed, w, h, rad, dx, dy, jf, g;
    bool ongr, mj, Mj, hp; bool take; bool actual; bool cum;
    HBITMAP hbmp;
} sprite;

const int l = 3;
int r ;

sprite man;
sprite enemy;
sprite plat;
sprite hill;
sprite hp[l];

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

POINT p;
void MouseInit() {

    GetCursorPos(&p);
    ScreenToClient(window.hwnd, &p);
   
}
void Ingame() {
    man.hbmp = (HBITMAP)LoadImageA(NULL, "chiken.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    enemy.hbmp = (HBITMAP)LoadImageA(NULL, "zombi.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    plat.hbmp = (HBITMAP)LoadImageA(NULL, "cloud.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hill.hbmp = (HBITMAP)LoadImageA(NULL, "snus.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBack = (HBITMAP)LoadImageA(NULL, "Home.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    if (!man.hbmp || !enemy.hbmp || !hBack) {
        MessageBoxA(NULL, "Failed to load game resources!", "Error", MB_ICONERROR);
    }

    man.w = 150;  
    man.h = 125;
    man.speed = 10;
    man.jf = 55;
    man.g = 20;
    man.mj = 0;
    man.Mj = 5;
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

    hill.w = 200;
    hill.h = 200;
    hill.x = window.w/2 - hill.w /2;
    hill.y = plat.y - hill.h;
    hill.take = false;
    hill.cum = true;

    for (int i = 0; i < l; i++) {
        r += l;
        hp[i].w = 450 / l;
        hp[i].h = 150;
        hp[i].x += (hp[i].w+20)* i; 
        hp[i].y = 100;
        hp[i].actual = true;
        hp[i].hbmp = (HBITMAP)LoadImageA(NULL, "hp.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    }

    

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
    if (GetAsyncKeyState('A')) man.x -= man.speed;
    if (GetAsyncKeyState('D')) man.x += man.speed;
    if (GetAsyncKeyState(VK_SPACE)&& man.ongr) {
      
        man.dy -= 40;
        man.ongr = false;
      
    }

    if(GetAsyncKeyState(VK_LBUTTON) && p.x >= enemy.x && p.x <= enemy.x + enemy.w &&
        p.y >= enemy.y && p.y <= enemy.y + enemy.h) {
        enemy.x = p.x - enemy.w / 2;
        enemy.y = p.y - enemy.h / 2;
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

    ShowBMP(window.context, hill.x, hill.y, hill.w, hill.h, hill.hbmp, true);

    for (int i = 0; i < r; i++) {
        if (hp[i].actual) {
            ShowBMP(window.context, hp[i].x, hp[i].y, hp[i].w, hp[i].h, hp[i].hbmp, true);
        }
    }

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
            if (man.y >= plat.y - man.h && man.x >= plat.x - man.w && man.x <= plat.x + plat.w) {
                man.y = plat.y - man.h;
                man.dy = 0;
                man.ongr = true;
                if (GetAsyncKeyState(VK_SPACE)) {
                    man.dy -= 20;
                    man.y += man.dy;
                    man.ongr = false;
                }
            }

        }
        else if (itog == y2) {
            man.y = plat.y + plat.h;
        }
        man.ongr = false;
    }
}

void TakeHill() {
    if (man.x >= hill.x - man.w &&
        man.y >= hill.y &&
        man.y <= hill.y + hill.h &&
        man.x <= hill.x + hill.w &&
        !hill.take) {
        hill.w = 0;
        hill.h = 0;
        hill.take = true;
        
    }
}

void Gravity() {

    if (!man.ongr) {
        man.dy += 0.9;
        man.y += man.dy;
        
    }

    if (man.y >= window.h - man.h){
        man.y = window.h - man.h;
        man.dy = 0;
        man.ongr = true;
    }
   
}

void Fight() {

    
}

void MamaPapa() {

    if (man.x >= enemy.x - man.w &&
        man.x <= enemy.x + enemy.w &&
        man.y >= enemy.y - man.h &&
        man.y <= enemy.y + enemy.h) {
        man.x = 0;
        man.y = window.h;
        r--;
    }
}
        

void DwaPapy() {

    if (r == 0) {
        game.active = false;
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


    ShowCursor(true);

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
                TakeHill();
                Gravity();
                Fight();
                MouseInit();
                MamaPapa();
                DwaPapy();

            }

            ShowManAndEnemy();
            ShowScore();

            if (!game.active) {
                SetTextColor(window.context, RGB(255, 0, 0));
                SetBkMode(window.context, TRANSPARENT);

                HFONT hFont = CreateFont(60, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, L"Arial");
                HFONT hOldFont = (HFONT)SelectObject(window.context, hFont);

                TextOutA(window.context, window.w / 2 - 155, window.h / 2, "GAME OVER", 23);

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