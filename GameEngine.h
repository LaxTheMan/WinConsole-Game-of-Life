#pragma once
#pragma comment(lib, "winmm.lib")

#define UNICODE
#define _UNICODE

#include <iostream>
#include <math.h>
#include <stdio.h>
#include <chrono>
#include <vector>
#include <atomic>
#include <cstring>
#include <thread>
#include <algorithm>
#include <Windows.h>
using namespace std;

enum COLOUR
{
    FG_BLACK = 0x0000,
    FG_DARK_BLUE = 0x0001,
    FG_DARK_GREEN = 0x0002,
    FG_DARK_CYAN = 0x0003,
    FG_DARK_RED = 0x0004,
    FG_DARK_MAGENTA = 0x0005,
    FG_DARK_YELLOW = 0x0006,
    FG_GREY = 0x0007, // Thanks MS :-/
    FG_DARK_GREY = 0x0008,
    FG_BLUE = 0x0009,
    FG_GREEN = 0x000A,
    FG_CYAN = 0x000B,
    FG_RED = 0x000C,
    FG_MAGENTA = 0x000D,
    FG_YELLOW = 0x000E,
    FG_WHITE = 0x000F,
    BG_BLACK = 0x0000,
    BG_DARK_BLUE = 0x0010,
    BG_DARK_GREEN = 0x0020,
    BG_DARK_CYAN = 0x0030,
    BG_DARK_RED = 0x0040,
    BG_DARK_MAGENTA = 0x0050,
    BG_DARK_YELLOW = 0x0060,
    BG_GREY = 0x0070,
    BG_DARK_GREY = 0x0080,
    BG_BLUE = 0x0090,
    BG_GREEN = 0x00A0,
    BG_CYAN = 0x00B0,
    BG_RED = 0x00C0,
    BG_MAGENTA = 0x00D0,
    BG_YELLOW = 0x00E0,
    BG_WHITE = 0x00F0,
};

enum PIXEL
{
    PIXEL_SOLID = 0x2588,
    PIXEL_THREEQUARTERS = 0x2593,
    PIXEL_HALF = 0x2592,
    PIXEL_QUARTER = 0x2591,
};

class GameEngine
{
public:
    // Constructor for GameEngine
    GameEngine()
    {
        screenWidth = 0;
        screenHeight = 0;

        console = GetStdHandle(STD_OUTPUT_HANDLE);

        memset(keyNewState, 0, 256 * sizeof(short));
        memset(keyOldState, 0, 256 * sizeof(short));
        memset(keys, 0, 256 * sizeof(keyState));

        appName = L"Default";
    }

    int ConstructConsole(int width, int height, int fontw = 12, int fonth = 12)
    {
        screenWidth = width;
        screenHeight = height;

        // Get original font
        originalFont.cbSize = sizeof(CONSOLE_FONT_INFOEX);
        if(!GetCurrentConsoleFontEx(console, false, &originalFont)) {
            return Error(L"GetCurrentConsoleFontEx");
        }

        // Get original buffer
        if(!GetConsoleScreenBufferInfo(console, &originalBuffer)) {
            return Error(L"GetConsoleScreenBufferInfo");
        }
        
        //Get original console window size
        consoleWindow = GetConsoleWindow();
        if(!GetWindowRect(consoleWindow, &originalWinSize)) {
            return Error(L"GetWindowRect");
        }

        // Setting to minimum possible dimension first
        customWinSize = {0, 0, 1, 1};
        SetConsoleWindowInfo(console, TRUE, &customWinSize);

        // Set console buffer size
        COORD buffer = {(short)screenWidth, (short)screenHeight};
        if (!SetConsoleScreenBufferSize(console, buffer))
        {
            cout << "Attempt to set:" << ' ' << buffer.X << ' ' << buffer.Y << endl;
            return Error(L"Reduce physical console size");
        }

        // Assign screen buffer to console
        if (!SetConsoleActiveScreenBuffer(console))
        {
            return Error(L"SetActiveScreenBuffer");
        }

        // Set custom font
        customFont.cbSize = sizeof(CONSOLE_FONT_INFOEX);
        customFont.nFont = 0;
        customFont.dwFontSize.X = fontw;
        customFont.dwFontSize.Y = fonth;
        customFont.FontFamily = FF_DONTCARE;
        customFont.FontWeight = FW_NORMAL;
        wcscpy_s(customFont.FaceName, L"Consolas");
        if(!SetCurrentConsoleFontEx(console, false, &customFont)) {
            return Error(L"Set Custom Font error");
        }

        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (!GetConsoleScreenBufferInfo(console, &csbi)) {
			return Error(L"GetConsoleScreenBufferInfo");
        }
		if (screenHeight > csbi.dwMaximumWindowSize.Y) {
			return Error(L"Screen Height / Font Height Too Big");
        }
		if (screenWidth > csbi.dwMaximumWindowSize.X) {
			return Error(L"Screen Width / Font Width Too Big");
        }


        // Set physical console window size
        customWinSize = {0, 0, (short)(screenWidth - 1), (short)(screenHeight - 1)};
        if (!SetConsoleWindowInfo(console, TRUE, &customWinSize))
        {
            return Error(L"SetConsoleWindowInfo");
        }

        bufferScreen = new CHAR_INFO[screenWidth * screenHeight];
        memset(bufferScreen, 0, sizeof(CHAR_INFO) * screenWidth * screenHeight);

        // SetConsoleCtrlHandler((PHANDLER_ROUTINE)CloseHandler, TRUE);

        Fill(0, 0, screenWidth, screenHeight, PIXEL_SOLID, FG_BLACK);

        return 1;
    }

    virtual void Draw(int x, int y, short c = 0x2588, short col = 0x000F)
    {
        if (x >= 0 && x < screenWidth && y >= 0 && y < screenHeight)
        {
            bufferScreen[y * screenWidth + x].Char.UnicodeChar = c;
            bufferScreen[y * screenWidth + x].Attributes = col;
        }
    }

    void Clip(int &x, int &y)
    {
        if (x < 0)
            x = 0;
        if (x >= screenWidth)
            x = screenWidth;
        if (y < 0)
            y = 0;
        if (y >= screenHeight)
            y = screenHeight;
    }

    void Fill(int x1, int y1, int x2, int y2, short c = 0x2588, short col = 0x000F)
    {
        Clip(x1, y1);
        Clip(x2, y2);
        for (int x = x1; x < x2; x++)
            for (int y = y1; y < y2; y++)
                Draw(x, y, c, col);
    }

    void Start()
    {
        atomActive = true;
        std::thread t = std::thread(&GameEngine::GameThread, this);

        t.join();
    }

    int ScreenWidth()
    {
        return screenWidth;
    }

    int ScreenHeight()
    {
        return screenHeight;
    }

    // ~GameEngine()
    // {
    //     cout << "Destructor called!" << endl;
    //     SetConsoleActiveScreenBuffer(originalConsole);
    //     SetCurrentConsoleFontEx(console, false, &originalConsoleFont);
    //     delete[] bufferScreen;
    // }

private:
    void GameThread()
    {
        if (!OnUserCreate())
            return;

        auto tp1 = std::chrono::system_clock::now();
        auto tp2 = std::chrono::system_clock::now();

        while (atomActive)
        {
            while (atomActive)
            {

                // Variable timestep FPS
                tp2 = std::chrono::system_clock::now();
                std::chrono::duration<float> elapsedTime = tp2 - tp1;
                tp1 = tp2;
                float fElapsedTime = elapsedTime.count();

                GameUpdateLogic();

                if (!(OnUserUpdate(fElapsedTime)))
                {
                    atomActive = false;
                }

                wchar_t s[128];
                swprintf_s(s, 128, L"Console Game Engine - %s - FPS: %3.2f", appName.c_str(), 1.0f / fElapsedTime);
                SetConsoleTitle(s);
                WriteConsoleOutput(console, bufferScreen, {(short)screenWidth, (short)screenHeight}, {0, 0}, &customWinSize);
            }
        }
    }

private:
    void GameUpdateLogic()
    {
        for (int i = 0; i < 256; i++)
        {
            keyNewState[i] = GetAsyncKeyState(i);

            keys[i].pressed = false;
            keys[i].released = false;

            if (keyNewState[i] != keyOldState[i])
            {
                if (keyNewState[i] & 0x8000)
                {
                    keys[i].pressed = !keys[i].held;
                    keys[i].held = true;
                }
                else
                {
                    keys[i].released = true;
                    keys[i].held = false;
                }
            }
            keyOldState[i] = keyNewState[i];
        }

        if (keys[VK_ESCAPE].pressed)
        {
            cout << "Terminated" << endl;
            Fill(0, 0, screenWidth, screenHeight, L' ', FG_BLACK);

            originalFont.cbSize = sizeof(originalFont);
            SetCurrentConsoleFontEx(console, false, &originalFont);

            COORD buffer = {(short)originalBuffer.dwSize.X, (short)originalBuffer.dwSize.Y};
            SetConsoleScreenBufferSize(console, buffer);

            customWinSize = {0, 0, (short)(originalWinSize.right - originalWinSize.left - 1), (short)(originalWinSize.bottom - originalWinSize.top - 1)};
            SetConsoleWindowInfo(console, TRUE, &customWinSize);
        }
    }

public:
    virtual bool OnUserCreate() = 0;
    virtual bool OnUserUpdate(float fElapsedTime) = 0;
    // virtual bool OnUserDestroy() = 0;

protected:
    struct keyState
    {
        bool pressed;
        bool released;
        bool held;
    } keys[256];

protected:
    int Error(const wchar_t *msg)
    {
        wchar_t buf[256];
        wcscpy(buf, msg);
        wcout << L"Error: " << msg << endl;
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 256, NULL);
        return 0;
    }

protected:
    int screenWidth;
    int screenHeight;
    HANDLE console;
    HWND consoleWindow;
    RECT originalWinSize;
    SMALL_RECT customWinSize;
    CONSOLE_SCREEN_BUFFER_INFO originalBuffer;
    CONSOLE_SCREEN_BUFFER_INFO customBuffer;
    CONSOLE_FONT_INFOEX originalFont;
    CONSOLE_FONT_INFOEX customFont;
    wstring appName;
    CHAR_INFO *bufferScreen;
    short keyOldState[256] = {0};
    short keyNewState[256] = {0};

    static std::atomic<bool> atomActive;
};

std::atomic<bool> GameEngine::atomActive(false);