#include <iostream>
#include <Windows.h>
#include <thread>
#include <vector>
#include <time.h>
using namespace std;

wstring tetromino[7];
int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char* pField = nullptr;

int screenWidth = 80;
int screenHeight = 30;

int rotate(int px, int py, int r)
{
	switch (r % 4)
	{
	case 0: return px + py * 4; 
	case 1: return 3 + px * 4 - py;
	case 2: return 15 - py * 4 - px;
	case 3: return 12 + py - px * 4;
	}

	return 0;
}

bool doesPieceFit(int nTetramino, int nRotation, int nPosX, int nPosY)
{

	for (int px = 0; px < 4; px++)
		for (int py = 0; py < 4; py++)
		{
			int pi = rotate(px, py, nRotation);

			int fi = (nPosX + px) + (nPosY + py) * nFieldWidth;

			if (nPosX + px >= 0 && nPosX + px < nFieldWidth)
			{
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight)
				{
					if (tetromino[nTetramino][pi] == L'X' && pField[fi] != 0)
						return false;
				}
			}
		}

	return true;
}

int main()
{
	//Create assets
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");

	tetromino[1].append(L"..X.");
	tetromino[1].append(L".XX.");
	tetromino[1].append(L".X..");
	tetromino[1].append(L"....");

	tetromino[2].append(L".X..");
	tetromino[2].append(L".XX.");
	tetromino[2].append(L"..X.");
	tetromino[2].append(L"....");

	tetromino[3].append(L"..X.");
	tetromino[3].append(L".XX.");
	tetromino[3].append(L"..X.");
	tetromino[3].append(L"....");

	tetromino[4].append(L".X..");
	tetromino[4].append(L".X..");
	tetromino[4].append(L".XX.");
	tetromino[4].append(L"....");

	tetromino[5].append(L"..X.");
	tetromino[5].append(L"..X.");
	tetromino[5].append(L".XX.");
	tetromino[5].append(L"....");

	tetromino[6].append(L"....");
	tetromino[6].append(L".XX.");
	tetromino[6].append(L".XX.");
	tetromino[6].append(L"....");
	
	pField = new unsigned char[nFieldWidth * nFieldHeight];
	for (int x = 0; x < nFieldWidth; x++)
		for (int y = 0; y < nFieldHeight; y++)
			pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;

	wchar_t *screen = new wchar_t[screenWidth * screenHeight];
	for (int i = 0; i < screenWidth * screenHeight; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	// Game Logic Stuff
	srand(time(NULL));
	bool bGameOver = false;

	int nCurrentPiece = rand() % 7;
	int nCurrentRotation = rand() % 4;
	int nCurrentX = nFieldWidth / 2 - 2;
	int nCurrentY = 0;

	int nNextPiece = rand() % 7;
	int nNextPieceRotation = rand() % 4;
	int nNextPieceX = nFieldWidth + 4;
	int nNextPieceY = 4;
	
	int nPieces = 0;
	int nScore = 0;
	int nSpeed = 20;
	int nSpeedCounter = 0;
	bool bForceDown = false;

	vector<int> vLines;

	bool bKey[4];
	bool bRotateHold = false;

	
	while (!bGameOver)
	{

		// GAME TIMING ==================================
		this_thread::sleep_for(50ms);
		nSpeedCounter += 1;
		bForceDown = (nSpeedCounter == nSpeed);
			
		// INPUT ========================================
		for (int k = 0; k < 4; k++)
		{
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28\x26"[k]))) != 0;
		}

		// GAME LOGIC =================================
		// move left
		nCurrentX -= (bKey[1] && doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
		// move right
		nCurrentX += (bKey[0] && doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
		// move down
		nCurrentY += (bKey[2] && doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;
		// rotate
		if (bKey[3])
		{
			nCurrentRotation += (!bRotateHold && doesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
			bRotateHold = true;
		}
		else 
		{
			bRotateHold = false;
		}

		if (bForceDown)
		{
			if (doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
				nCurrentY++;
			else
			{
				// Lock current piece into place
				for (int px = 0; px < 4; px++)
					for (int py = 0; py < 4; py++)
						if (tetromino[nCurrentPiece][rotate(px, py, nCurrentRotation)] == L'X')
							pField[(nCurrentX + px) + (nCurrentY + py) * nFieldWidth] = nCurrentPiece + 1;

				nPieces++;
				if (nPieces % 10 == 0)
					if (nSpeed >= 10) 
						nSpeed--;
				
				// Check have we got any lines
				for (int py = 0; py < 4; py++)
					if (nCurrentY + py < nFieldHeight - 1)
					{
						bool bLine = true;
						for (int x = 1; x < nFieldWidth - 1; x++)
							bLine &= (pField[(nCurrentY + py) * nFieldWidth + x] != 0);

						if (bLine)
						{
							for (int x = 1; x < nFieldWidth - 1; x++)
								pField[(nCurrentY + py) * nFieldWidth + x] = 8;

							vLines.push_back(nCurrentY + py);
						}
					}

				// Scoring
				nScore += 25;
				if (!vLines.empty())
					nScore += (1 << vLines.size()) * 100;

				// Choose next piece
				nCurrentPiece = nNextPiece;
				nCurrentRotation = nNextPieceRotation;
				nCurrentX = nFieldWidth / 2 - 2;
				nCurrentY = 0;

				nNextPiece = rand() % 7;
				nNextPieceRotation = rand() % 4;

				// if piece does not fit
				bGameOver = !doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
			}

			nSpeedCounter = 0;
		}

		// RENDER OUTPUT
		
		// Draw Field
		for (int x = 0; x < nFieldWidth; x++)
		{
			for (int y = 0; y < nFieldHeight; y++)
			{
				screen[(y + 2) * screenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];
			}
		}

		// Draw Current Piece
		for (int px = 0; px < 4; px++)
			for (int py = 0; py < 4; py++)
				if (tetromino[nCurrentPiece][rotate(px, py, nCurrentRotation)] == L'X')
					screen[(nCurrentY + py + 2) * screenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65;

		

		// Draw Score
		swprintf_s(&screen[2 * screenWidth + nFieldWidth + 6], 16, L"Score: %8d", nScore);
		// Draw Next Piece
		swprintf_s(&screen[(nNextPieceY + 2) * screenWidth + (nNextPieceX + 2)], 16, L"Next Tetromino:");
		for (int px = 0; px < 4; px++)
			for (int py = 0; py < 4; py++)
				if (tetromino[nNextPiece][rotate(px, py, nNextPieceRotation)] == L'X')
					screen[(nNextPieceY + 1 + py + 2) * screenWidth + (nNextPieceX + px + 2)] = nNextPiece + 65;
				else
					screen[(nNextPieceY + 1 + py + 2) * screenWidth + (nNextPieceX + px + 2)] = L' ';

		if (!vLines.empty())
		{
			WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);
			this_thread::sleep_for(400ms);

			for (auto &v : vLines)
				for (int x = 1; x < nFieldWidth - 1; x++)
				{
					for (int y = v; y > 0; y--)
						pField[y * nFieldWidth + x] = pField[(y - 1) * nFieldWidth + x];
					pField[x] = 0;
				}
			vLines.clear();
		}

		// Display Frame
		WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);
	}

	CloseHandle(hConsole);
	cout << "Game Over!! Score: " << nScore << endl;
	system("pause");

	return 0;
}