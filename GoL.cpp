#include <iostream>
#include "GameEngine.h"
using namespace std;

class GoL : public GameEngine
{
public:
    GoL()
    {
        appName = L"Conway's Game of Life";
    }

protected:
    bool OnUserCreate() override
    {
        output = new int[ScreenWidth() * ScreenHeight()];
        state = new int[ScreenWidth() * ScreenHeight()];

        playerX = 10.0f;
        playerY = 10.0f;

        memset(output, 0, ScreenWidth() * ScreenHeight() * sizeof(int));
        memset(state, 0, ScreenWidth() * ScreenHeight() * sizeof(int));

        auto set = [&](int x, int y, wstring s)
        {
            int p = 0;
            for (auto c : s)
            {
                state[x + y * ScreenWidth() + p] = c == L'#' ? 1 : 0;
                p++;
            }
        };

        // Custom initial input
        // set(80, 50, L" #   ####   #");
        // set(80, 51, L"#   ###    ##");
        // set(80, 52, L" #  #  ##  ##");
        // set(80, 53, L" #  #  ##  ##");
        // set(80, 54, L" #  #  ##  ##");
        // set(80, 55, L" #  #  ##  ##");
        // set(80, 56, L" #  #  ##  ##");
        // set(80, 57, L" #  #  ##  ##");

        auto cell = [&](int x, int y)
        {
            return output[x + y * ScreenWidth()];
        };

        for (int i = 0; i < ScreenWidth() * ScreenHeight(); i++)
        {
            output[i] = state[i];
        }

        for (int x = 1; x < ScreenWidth() - 1; x++)
        {
            for (int y = 1; y < ScreenHeight() - 1; y++)
            {
                int neighbours = cell(x - 1, y - 1) + cell(x - 0, y - 1) + cell(x + 1, y - 1) +
                                 cell(x - 1, y - 0) + 0 + cell(x + 1, y - 0) +
                                 cell(x - 1, y + 1) + cell(x - 0, y + 1) + cell(x + 1, y + 1);

                if (cell(x, y) == 1)
                    state[x + y * ScreenWidth()] = neighbours == 2 || neighbours == 3;
                else
                    state[x + y * ScreenWidth()] = neighbours == 3;

                if (cell(x, y) == 1)
                    Draw(x, y, 0x2588, FG_WHITE);
                else
                    Draw(x, y, 0x2588, FG_BLACK);
            }
        }

        Fill(10, 10, 20, 20, L'#', FG_BLUE);

        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        auto cell = [&](int x, int y)
        {
            return output[x + y * ScreenWidth()];
        };

        if (keys[VK_SPACE].held)
        {
            this_thread::sleep_for(20ms);

            for (int i = 0; i < ScreenWidth() * ScreenHeight(); i++)
            {
                output[i] = state[i];
            }
            for (int x = 1; x < ScreenWidth() - 1; x++)
            {
                for (int y = 1; y < ScreenHeight() - 1; y++)
                {
                    int neighbours = cell(x - 1, y - 1) + cell(x - 0, y - 1) + cell(x + 1, y - 1) +
                                     cell(x - 1, y - 0) + 0 + cell(x + 1, y - 0) +
                                     cell(x - 1, y + 1) + cell(x - 0, y + 1) + cell(x + 1, y + 1);

                    if (cell(x, y) == 1)
                        state[x + y * ScreenWidth()] = neighbours == 2 || neighbours == 3;
                    else
                        state[x + y * ScreenWidth()] = neighbours == 3;

                    if (cell(x, y) == 1)
                        Draw(x, y, PIXEL_SOLID, FG_WHITE);
                    else
                        Draw(x, y, PIXEL_SOLID, FG_BLACK);
                }
            }
        }

        if (keys[VK_UP].held)
        {
            playerY -= 10.5f * fElapsedTime;
        }
        if (keys[VK_DOWN].held)
        {
            playerY += 10.5f * fElapsedTime;
        }
        if (keys[VK_RIGHT].held)
        {
            playerX += 10.5f * fElapsedTime;
        }
        if (keys[VK_LEFT].held)
        {
            playerX -= 10.5f * fElapsedTime;
        }
        if (keys['Q'].held)
        {
            state[(int)playerX + (int)playerY * ScreenWidth()] = !state[(int)playerX + (int)playerY * ScreenWidth()];
        }
        if (keys['R'].pressed)
        {
            for (int x = 1; x < ScreenWidth() - 1; x++)
            {
                for (int y = 1; y < ScreenHeight() - 1; y++)
                {
                    int randX = rand() % ScreenWidth();
                    int randY = rand() % ScreenHeight();
                    int randOut = rand() % 2;
                    state[((int)randX + (int)randY * ScreenWidth())] = randOut ? !state[((int)randX + (int)randY * ScreenWidth())] : 0;
                }
            }
        }

        for (int x = 1; x < ScreenWidth() - 1; x++)
        {
            for (int y = 1; y < ScreenHeight() - 1; y++)
            {
                if (state[x + y * ScreenWidth()])
                    Draw(x, y, PIXEL_SOLID, FG_WHITE);
                else
                    Draw(x, y, PIXEL_SOLID, FG_BLACK);
            }
        }

        Draw(playerX, playerY, L'8', FG_BLUE);

        return true;
    }

private:
    int *output;
    int *state;

    float playerX;
    float playerY;
};

int main()
{

    GoL game;
    game.ConstructConsole(300, 80, 4, 4);
    game.Start();

    return 0;
}