// 2DSideScroller.cpp : This file contains the 'main' function. Program execution begins and ends there.
// This is my first foray into C++ & Game Programming. This was a result of following OLC's 'Simple Tile Based Platform Game'
// I wanted to teach myself the basics of C++ as well as gain an understanding of Rudimentary Game Programming concepts.

#include <iostream>
#include <string>
using namespace std;

// Adding olcConsoleGameEngine. This library handles simple tasks such as drawing to the Console and User Input.
#include "olcConsoleGameEngine.h"

class OneLoneCoder_Platformer : public olcConsoleGameEngine
{
public:
    OneLoneCoder_Platformer()
    {
        m_sAppName = L"2DPlatformer"; //L before string signifies string is wide string literal. this is an array of n const wchar_T.
    }

private:
    wstring sLevel; // Wide String
    int nLevelWidth;
    int nLevelHeight;

    // These variable represent the position & velocity of a player.
    float fPlayerPosX = 0.0f;
    float fPlayerPosY = 0.0f;

    float fPlayerVelX = 0.0f;
    float fPlayerVelY = 0.0f;

    // Ground Check
    bool bPlayerOnGround = false;

    // Sprites
    olcSprite* spriteTiles = nullptr;
    olcSprite* spriteMan = nullptr;
    int nDirModX = 0;
    int nDirModY = 0;

    // We need a camera in order to tell what part of the 'world' our character is in.
    float fCameraPosX = 0.0f;
    float fCameraPosY = 0.0f;

protected:
    virtual bool OnUserCreate() // Used to allocate system resources.
    {
        // 64 Tiles wide by 16 Tiles high
        nLevelWidth = 64;
        nLevelHeight = 16;

        // Now we create the layout of our 'Level'. We can append level information to our sLevel wide string.
        sLevel += L"................................................................";
        sLevel += L"................................................................";
        sLevel += L"................................................................";
        sLevel += L"................................................................";
        sLevel += L"......................................##########................";
        sLevel += L"...................................####........#................";
        sLevel += L"................................####...........#................";
        sLevel += L"..............................#####.............................";
        sLevel += L"#########################..###########################.#########";
        sLevel += L".........................#................############.#........";
        sLevel += L"..........................#...............############.#........";
        sLevel += L"..........................#############...####.........#........";
        sLevel += L".......................................#..####..########........";
        sLevel += L"........................................#..............#........";
        sLevel += L".........................................###############........";
        sLevel += L"................................................................";

        spriteTiles = new olcSprite(L"sprites/leveljario.spr");
        spriteMan = new olcSprite(L"sprites/minijario.spr");

        return true;
    }

    virtual bool OnUserUpdate(float fElapsedTime) //Game Logic.
    {
        // Utility Lambda Functions
        auto GetTile = [&](int x, int y)
        {
            if (x >= 0 && x < nLevelWidth && y >= 0 && y < nLevelHeight) // Ensuring we're within the level boundary.
                return sLevel[y * nLevelWidth + x]; // Return char at specific point of Level String.
            else
                return L' '; // If we see this, an error has occurred and we're accessing a tile that either doesn't exist or is broken.
        };

        auto SetTile = [&](int x, int y, wchar_t c)
        {
            if (x >= 0 && x < nLevelWidth && y >= 0 && y < nLevelHeight)
                sLevel[y * nLevelWidth + x]; // Set the tile to the char passed in. No else statement because we're only changing chars within the game bounds.
        };

        //fPlayerVelY = 0.0f; 
        //fPlayerVelX = 0.0f;

        // Handle Input
        if (IsFocused()) // Checks if the command terminal is in focus (i.e. actively being used by the user)
        {
            // DEBUG
            /*if (GetKey(VK_UP).bHeld)
            {
                fPlayerVelY = -6.0f;
            }

            if (GetKey(VK_DOWN).bHeld)
            {
                fPlayerVelY = 6.0f;
            }
            */
            if (GetKey(VK_LEFT).bHeld)
            {
                fPlayerVelX += (bPlayerOnGround ? -6.0f : -4.0f) * fElapsedTime;
                nDirModY = 1;
            }

            if (GetKey(VK_RIGHT).bHeld)
            {
                fPlayerVelX += (bPlayerOnGround ? 6.0f : 4.0f) * fElapsedTime;
                nDirModY = 0;
            }

            if (GetKey(VK_SPACE).bPressed)
            {
                if (fPlayerVelY == 0) // Not already jumping/falling
                {
                    fPlayerVelY = -12.0f;
                    nDirModX = 1;
                }
            }
        }

        //Gravity + Atmospheric Drag
        fPlayerVelY += 20.0f * fElapsedTime;

        if (bPlayerOnGround)
        {
            fPlayerVelX += -3.0f * fPlayerVelX * fElapsedTime;
            if (fabs(fPlayerVelX) < 0.01f)
                fPlayerVelX = 0.0f;
        }

        // Update player position between current frame and last frame on screen.
        float fNewPlayerPosX = fPlayerPosX + fPlayerVelX * fElapsedTime; 
        float fNewPlayerPosY = fPlayerPosY + fPlayerVelY * fElapsedTime;

        //Velocity 'clamp' <-- Prevent become out of control.
        if (fPlayerVelX > 10.0f)
            fPlayerVelX = 10.0f;

        if (fPlayerVelX < -10.0f)
            fPlayerVelX = -10.0f;

        if (fPlayerVelY > 100.0f)
            fPlayerVelY = 100.0f;

        if (fPlayerVelY < -100.0f)
            fPlayerVelY = -100.0f;

        // Collision
        if (fPlayerVelX <= 0) // Moving left
        { // Using newPlayerPosX & OLD fPlayerPosY in order to negate Y velocity, only concerned with X axis.
            // Using 0.9f instead of 1.0f to account for false positives. 1.0f could show player is in two seperate tiles at once, e.g. (3 , 5) and (3 , 6) due to rounding. 0.9f accounts for this so that when we truncate its to the floor, not ceiling.
            if (GetTile(fNewPlayerPosX + 0.0f, fPlayerPosY + 0.0f) != L'.' || GetTile(fNewPlayerPosX + 0.0f, fPlayerPosY + 0.9f) != L'.')
            {
                // Casting to ints as tiles lie on Integer boundaries. E.g. having a collision at pos x = 4.8 (overlapping it into the block at pos x = 4) and truncating that to 4 via integer casting, we add 1 to ensure we remain at pos x = 5 instead of phasing into the solid block at pos x = 4.
                fNewPlayerPosX = (int)fNewPlayerPosX + 1; // Set requested position cast to an integer + 1; i.e. get rid of fractional positional data and + 1 to remainder.
                fPlayerVelX = 0;
            }
        }
        else // Moving Right
        {
            if (GetTile(fNewPlayerPosX + 1.0f, fPlayerPosY + 0.0f) != L'.' || GetTile(fNewPlayerPosX + 1.0f, fPlayerPosY + 0.9f) != L'.')
            {
                fNewPlayerPosX = (int)fNewPlayerPosX; // Set requested position cast to an integer; i.e. get rid of fractional positional data to stay within bounds.
                fPlayerVelX = 0;
            }
        }

        bPlayerOnGround = false;
        if (fPlayerVelY <= 0) // Moving Up
        {
            if (GetTile(fNewPlayerPosX + 0.0f, fNewPlayerPosY) != L'.' || GetTile(fNewPlayerPosX + 0.9f, fNewPlayerPosY) != L'.')
            {
                fNewPlayerPosY = (int)fNewPlayerPosY + 1;
                fPlayerVelY = 0;
            }
        }
        else // Moving Down
        {
            if (GetTile(fNewPlayerPosX + 0.0f, fNewPlayerPosY + 1.0f) != L'.' || GetTile(fNewPlayerPosX + 0.9f, fNewPlayerPosY + 1.0f) != L'.')
            {
                fNewPlayerPosY = (int)fNewPlayerPosY;
                fPlayerVelY = 0;
                bPlayerOnGround = true;
                nDirModX = 0;
            }
        }

        fPlayerPosX = fNewPlayerPosX;
        fPlayerPosY = fNewPlayerPosY;

        fCameraPosX = fPlayerPosX;
        fCameraPosY = fPlayerPosY;

        // Draw the Level
        int nTileWidth = 16; // Each tile is 8x8px
        int nTileHeight = 16;
        int nVisibleTilesX = ScreenWidth() / nTileWidth; // Allows us to work out how many tiles to display.
        int nVisibleTilesY = ScreenHeight() / nTileHeight;

        // Calculate Top-Leftmost visible tile.
        // Takes camera position, and subtracts half the screen's dimension in both X&Y axis.
        float fOffsetX = fCameraPosX - (float)nVisibleTilesX / 2.0f;
        float fOffsetY = fCameraPosY - (float)nVisibleTilesY / 2.0f;

        // Ensures camera stays within Level boundaries.
        if (fOffsetX < 0) fOffsetX = 0;
        if (fOffsetY < 0) fOffsetY = 0;

        // Ensuring that the Camera doesn't go beyond the level width/height (minus the visible tiles on the screen for specific axis)
        if (fOffsetX > nLevelWidth - nVisibleTilesX) fOffsetX = nLevelWidth - nVisibleTilesX;
        if (fOffsetY > nLevelHeight - nVisibleTilesY) fOffsetY = nLevelHeight - nVisibleTilesY;

        // Get offsets for smoother movement (comprimise for using integer tiles)
        float fTileOffsetX = (fOffsetX - (int)fOffsetX) * nTileWidth;
        float fTileOffsetY = (fOffsetY - (int)fOffsetY) * nTileHeight;
        
        // Draw our tiles onto the screen
        for (int x = -1; x < nVisibleTilesX + 1; x++)
        {
            for (int y = -1; y < nVisibleTilesY + 1; y++)
            {
                wchar_t sTileID = GetTile(x + fOffsetX, y + fOffsetY);
                switch (sTileID)
                {
                case L'.':
                    // Draw a rectangle at the location provided (x,y), scaled by the tile width, also drawing a square from top left to bottom right in that rectangle (x+1, y+1 values)  
                    Fill(x * nTileWidth - fTileOffsetX, y * nTileHeight - fTileOffsetY, (x + 1) * nTileWidth - fTileOffsetX, (y + 1) * nTileHeight - fTileOffsetY, PIXEL_SOLID, FG_CYAN);
                    break;
                case L'#':
                    //Fill(x * nTileWidth - fTileOffsetX, y * nTileHeight - fTileOffsetY, (x + 1) * nTileWidth - fTileOffsetX, (y + 1) * nTileHeight - fTileOffsetY, PIXEL_SOLID, FG_RED);
                    DrawPartialSprite(x * nTileWidth - fTileOffsetX, y * nTileHeight - fTileOffsetY, spriteTiles, 2 * nTileWidth, 0 * nTileHeight, nTileWidth, nTileHeight);
                    break;
                default:
                    Fill(x * nTileWidth - fTileOffsetX, y * nTileHeight - fTileOffsetY, (x + 1) * nTileWidth - fTileOffsetX, (y + 1) * nTileHeight - fTileOffsetY, PIXEL_SOLID, FG_BLACK);
                    break;
                }
            }
        }

        // Draw player onto the screen. Subtracting offset is for when player has reached the boundaries of the level.
        //Fill((fPlayerPosX - fOffsetX) * nTileWidth, (fPlayerPosY - fOffsetY) * nTileWidth, (fPlayerPosX - fOffsetX + 1.0f) * nTileWidth, (fPlayerPosY - fOffsetY + 1.0f) * nTileHeight, PIXEL_SOLID, FG_GREEN);
        DrawPartialSprite((fPlayerPosX - fOffsetX) * nTileWidth, (fPlayerPosY - fOffsetY) * nTileWidth, spriteMan, nDirModX * nTileWidth, nDirModY * nTileHeight, nTileWidth, nTileHeight);


        return true;
    }
};

int main()
{
    OneLoneCoder_Platformer game;
    if (game.ConstructConsole(256, 240, 4, 4)) //256 char width, 240 char height, each char is 4x4 pixels
        game.Start();
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
