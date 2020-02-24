#pragma once

extern "C" {
#include"./sdl-2.0.7/include/SDL.h"
#include"./sdl-2.0.7/include/SDL_main.h"
}

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

#include "macros.h"

enum myDirections_t {
	myUp,
	myDown,
	myLeft,
	myRight
};
enum myState_t {
	myNull,
	justSpawned,
	myOn,
	myOff,
	myWin,
	myLose
};
typedef struct myScreen_t {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture[TEXTURES_AMT];
	SDL_Texture *charset;
};
typedef struct myBoardInfo_t {
	int width;
	int height;
	int move;
	int startTime;
	int endTime;
	int score[MOVES_REMEMBERED];
	char scoreString[SCORE_DIGITS + 1];
	int time;
	int movesRemembered;
	int touchpadCtrl;
	myState_t timerStatus;
	myState_t gameStatus;
};
typedef struct myVector_t {
	int x;
	int y;
};
typedef struct myBoard_t {
	int **value[MOVES_REMEMBERED];
	myState_t **state;
	myVector_t **vector;
	myBoardInfo_t info;
};


void resetVectorsStates(myBoard_t *tempBoard); //sets 0 as value of every vector and myNull as value of state
int initBoardAlValues(myBoard_t *tempBoard);
int initBoardAlVectors(myBoard_t *tempBoard);
int initBoardAlStates(myBoard_t *tempBoard);
int initBoard(myBoard_t *tempBoard, const int width, const int height);
void deleteBoard(myBoard_t *tempboard);
int spawnTile(myBoard_t *tempboard);
int moveTiles(myBoard_t *tempBoard, myDirections_t direction);
int moveLeft(myBoard_t *tempBoard);
int moveRight(myBoard_t *tempBoard);
int moveUp(myBoard_t *tempBoard);
int moveDown(myBoard_t *tempBoard);
int handleEvent(myBoard_t *tempBoard, SDL_Event eventHandler, myScreen_t *tempScreen); //returns -1 if no tiles were moved else returns move score
void updateScoreString(myBoard_t *tempBoard);
int newGame(myBoard_t *tempBoard, myScreen_t *tempScreen);
void gameStatusCheck(myBoard_t *tempBoard);

int initSDL(myScreen_t *tempScreen); //0 - success, 1 - fail
void exitSDL(myScreen_t *tempScreen);
void displayFrame(myScreen_t *tempScreen, myBoard_t *tempBoard);
void displayGrid(myScreen_t *tempScreen, myBoard_t *tempBoard);
void displayTiles(myScreen_t *tempScreen, myBoard_t *tempBoard);
void animate(myScreen_t *tempScreen, myBoard_t *tempBoard);
void displayText(myScreen_t *tempScreen, const char text[], int x, int y, int size);
void displayLegend(myScreen_t *tempScreen, myBoard_t *tempBoard);
