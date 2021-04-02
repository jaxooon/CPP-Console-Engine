// ConFirstPerson.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <chrono>
#include <vector>
#include <algorithm>
using namespace std;

int nScreenWidth = 120;
int nScreenHeight = 40;

//player location
float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerAngle = 0.0f;
float fPlayerSpeed = 5.0f;

//map
int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.1416 / 4;
float fDepth = 16.0f;

int main()
{
	//create screen buffer
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	wstring map;
	map += L"################";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#########..#####";
	map += L"#....#.........#";
	map += L"#....#.........#";
	map += L"#....#.........#";
	map += L"#..............#";
	map += L"#....#.........#";
	map += L"#....#.........#";
	map += L"#....#####...###";
	map += L"#....#.........#";
	map += L"#....#.........#";
	map += L"######.........#";
	map += L"#..............#";
	map += L"################";

	//Frame delta: time between two frames
	auto timeOne = chrono::system_clock::now();
	auto timeTwo = chrono::system_clock::now();


	//game loop
	while (1) {

		timeTwo = chrono::system_clock::now();
		chrono::duration<float> deltaFrame = timeOne - timeTwo;
		timeOne = timeTwo;
		float fDeltaFrame = deltaFrame.count(); 

		//controlls
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000) {
			fPlayerAngle += (1.0f)*fDeltaFrame;
		}
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000) {
			fPlayerAngle -= (1.0f)*fDeltaFrame;
		}
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
			fPlayerX -= sinf(fPlayerAngle) * fPlayerSpeed * fDeltaFrame;
			fPlayerY -= cosf(fPlayerAngle) * fPlayerSpeed * fDeltaFrame;
			if (map[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#') {
				fPlayerX += sinf(fPlayerAngle) * fPlayerSpeed * fDeltaFrame;
				fPlayerY += cosf(fPlayerAngle) * fPlayerSpeed * fDeltaFrame;
			}
		}
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
			fPlayerX += sinf(fPlayerAngle) * fPlayerSpeed * fDeltaFrame;
			fPlayerY += cosf(fPlayerAngle) * fPlayerSpeed * fDeltaFrame;
			if (map[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#') {
				fPlayerX -= sinf(fPlayerAngle) * fPlayerSpeed * fDeltaFrame;
				fPlayerY -= cosf(fPlayerAngle) * fPlayerSpeed * fDeltaFrame;
			}
		}

		for (int x = 0; x < nScreenWidth; x++) {

			float fRayAngle = (fPlayerAngle - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;
			float fDistanceToWall = 0.0f;
			float fAngleToPlayer = 0.0f;
			bool bHitWall = false;
			bool bBoundary = false;

			float fEyeX = sinf(fRayAngle); //unit vector for ray in player space
			float fEyeY = cosf(fRayAngle);

			while (!bHitWall && fDistanceToWall < fDepth) {

				fDistanceToWall += 0.1f;

				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

				//Make sure ray is not out of bounds
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {
					bHitWall = true;
					fDistanceToWall = fDepth;
				}
				else {

					//Ray is inbounds, check the cell
					if (map[nTestX * nMapWidth + nTestY] == '#') {
						bHitWall = true;

						//Get corners of cell
						vector<pair<float, float>> p; //distance to perfect corner, dot product

						//angle between ray and corners
						for (int tx = 0; tx < 2; tx++) { //corners
							for (int ty = 0; ty < 2; ty++) {
								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX + tx - fPlayerX;
								float d = sqrt(vx * vx + vy * vy);
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
								p.push_back(make_pair(d, dot));
							}
						}
						// Sort Pairs from closest to farthest
						sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.first < right.first; });

						// First two/three are closest (we will never see all four)
						float fBound = 0.01;
						if (acos(p.at(0).second) < fBound) bBoundary = true;
						if (acos(p.at(1).second) < fBound) bBoundary = true;
						if (acos(p.at(2).second) < fBound) bBoundary = true;

						fAngleToPlayer = 180.0 - 90.0 - fRayAngle; 

					}
				}
			}

			//calc distance to ceieling and floot (fake 3d)
			int nCeiling = (float)(nScreenHeight / 2.0f) - nScreenHeight / ((float)fDistanceToWall);
			int nFloor = nScreenHeight - nCeiling;

			short nWallShade = ' ';
			if (fDistanceToWall <= fDepth / 4.0f) nWallShade = 0x2588;
			else if (fDistanceToWall < fDepth / 3.0f) nWallShade = 0x2593;
			else if (fDistanceToWall < fDepth / 2.0f) nWallShade = 0x2592;
			else if (fDistanceToWall < fDepth) nWallShade = 0x2591;
			else										nWallShade = ' ';

			short nFloorShade = ' ';

			if (bBoundary) {
				nWallShade = ' ';
			}

			for (int y = 0; y < nScreenHeight; y++) {
				if (y <= nCeiling) {
					screen[y * nScreenWidth + x] = ' ';
				}
				else if (y > nCeiling && y <= nFloor) {
					screen[y * nScreenWidth + x] = nWallShade;
				}
				else {
					//percent of floor shade as a function of distance from wall
					float fFloor = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
					if (fFloor < 0.25) nFloorShade = '#';
					else if (fFloor < 0.35) nFloorShade = 'x';
					else if (fFloor < 0.80) nFloorShade = '-';
					else nFloorShade = '.';
					screen[y * nScreenWidth + x] = nFloorShade;
				}  
			}

		}
		//display stats
		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerAngle, 1.0f / fDeltaFrame);

		//display map
		for (int nx = 0; nx < nMapWidth; nx++)
			for (int ny = 0; ny < nMapWidth; ny++)
			{
				screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
			}

		screen[(int)fPlayerX * nScreenWidth + (int)fPlayerY] = 'p';

		screen[nScreenWidth * nScreenHeight - 1] = '\0'; //last char of screen array to escape char
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);

	}

	return 0;


}
