#include "main.h"


int main(int argc, char **argv) {

	srand(time(NULL));
	myBoard_t currBoard;
	myScreen_t currScreen;
	SDL_Event eventHandler;
	int moveScore = -3;	//-1 no move, -2 exit, -3 new game
	

	if (initSDL(&currScreen) == FAIL)
		return 1;

	do {

		if (moveScore == -3)
			if (newGame(&currBoard, &currScreen) == FAIL)
				return 1;
			else
				moveScore = -1;

		displayFrame(&currScreen, &currBoard);
		if (SDL_PollEvent(&eventHandler)) {
			moveScore = handleEvent(&currBoard, eventHandler, &currScreen);
		}

		if (moveScore >= 0) {
			if (currBoard.info.move == 1) {
				currBoard.info.time = SDL_GetTicks(); //saves start time
				currBoard.info.timerStatus = myOn;
			}
			spawnTile(&currBoard);
			moveScore += currBoard.info.score[(currBoard.info.move - 1) % MOVES_REMEMBERED];
			currBoard.info.score[currBoard.info.move % MOVES_REMEMBERED] =  moveScore;
			updateScoreString(&currBoard);
			animate(&currScreen, &currBoard);
			if (currBoard.info.movesRemembered < MOVES_REMEMBERED - 1)
				currBoard.info.movesRemembered += 1;
			moveScore = -1;
			gameStatusCheck(&currBoard);
		}

		

		if (moveScore == -3)
			deleteBoard(&currBoard);

	} while (moveScore != -2);



	deleteBoard(&currBoard);
	exitSDL(&currScreen);
	return 0;
}


void resetVectorsStates(myBoard_t *tempBoard) {
	for (int x = 0; x < tempBoard->info.width; ++x)
		for (int y = 0; y < tempBoard->info.height; ++y) {
			tempBoard->state[x][y] = myNull;
			tempBoard->vector[x][y].x = 0;
			tempBoard->vector[x][y].y = 0;
		}
	return;
}
int initBoardAlValues(myBoard_t *tempBoard) {
	int fail = 0;

	for (int i = 0; i < MOVES_REMEMBERED; ++i) { //loop allocates boards memory
		tempBoard->value[i] = (int**)malloc(sizeof(int*)*tempBoard->info.width);
		if (tempBoard->value[i] == NULL)
			fail = 1;
		else {
			for (int k = 0; k < tempBoard->info.width; ++k) {
				tempBoard->value[i][k] = (int*)malloc(sizeof(int)*tempBoard->info.height);
				if (tempBoard->value[i][k] == NULL) {
					fail = 1;
					for (--k; k >= 0; --k)
						free(tempBoard->value[i][k]);
					free(tempBoard->value[i]);
					break;
				}
			}
		}

		if (fail) { //cleans already allocated memory
			for (--i; i >= 0; --i) {
				for (int k = 0; k < tempBoard->info.width; ++k)
					free(tempBoard->value[i][k]);
				free(tempBoard->value[i]);
			}
			break;
		}
	}

	return fail;
}
int initBoardAlVectors(myBoard_t *tempBoard) {
	int fail = 0;

	tempBoard->vector = (myVector_t**)malloc(sizeof(myVector_t*)*tempBoard->info.width);
	if (tempBoard->vector == NULL)
		fail = 1;
	else {
		for (int k = 0; k < tempBoard->info.width; ++k) {
			tempBoard->vector[k] = (myVector_t*)malloc(sizeof(myVector_t)*tempBoard->info.height);
			if (tempBoard->vector[k] == NULL) {
				fail = 1;
				for (--k; k >= 0; --k)
					free(tempBoard->vector[k]);
				free(tempBoard->vector);
				break;
			}
		}
	}

	return fail;
}
int initBoardAlStates(myBoard_t *tempBoard) {
	int fail = 0;

	tempBoard->state = (myState_t**)malloc(sizeof(myState_t*)*tempBoard->info.width);
	if (tempBoard->state == NULL)
		fail = 1;
	else {
		for (int k = 0; k < tempBoard->info.width; ++k) {
			tempBoard->state[k] = (myState_t*)malloc(sizeof(myState_t)*tempBoard->info.height);
			if (tempBoard->state[k] == NULL) {
				fail = 1;
				for (--k; k >= 0; --k)
					free(tempBoard->state[k]);
				free(tempBoard->state);
				break;
			}
		}
	}

	return fail;
}
int initBoard(myBoard_t *tempBoard, const int width = 4, const int height = 4) {
	int fail = 0;

	tempBoard->info.height = height;
	tempBoard->info.width = width;
	tempBoard->info.move = 0;
	tempBoard->info.touchpadCtrl = 0;
	for (int i = 0; i < MOVES_REMEMBERED; ++i)
		tempBoard->info.score[i] = 0;
	tempBoard->info.movesRemembered = 0;
	tempBoard->info.gameStatus = myNull;
	tempBoard->info.timerStatus = myOff;
	updateScoreString(tempBoard);

	fail = initBoardAlValues(tempBoard);
	if (!fail)
		fail = initBoardAlVectors(tempBoard);
	if (!fail)
		fail = initBoardAlStates(tempBoard);
	if (!fail) {
		resetVectorsStates(tempBoard);
		for (int x = 0; x < width; ++x)
			for (int y = 0; y < height; ++y)
				tempBoard->value[0][x][y] = NULL;
	}
	return fail;
}
void deleteBoard(myBoard_t *tempboard) {
	for (int x = 0; x < tempboard->info.width; ++x) {
		free(tempboard->state[x]);
		free(tempboard->vector[x]);
		for (int i = 0; i < MOVES_REMEMBERED; ++i)
			free(tempboard->value[i][x]);
	}

	free(tempboard->state);
	free(tempboard->vector);
	for (int i = 0; i < MOVES_REMEMBERED; ++i)
		free(tempboard->value[i]);
	return;
}
int spawnTile(myBoard_t *tempboard) {
	int tileToSpawn = (rand() % 10 < CHANCE_FOR_FOUR ? 4 : 2);
	int fail = 1, x, y;
	int move = tempboard->info.move % MOVES_REMEMBERED;

	for (x = 0; x < tempboard->info.width && fail == 1; ++x)
		for (y = 0; y < tempboard->info.height; ++y)
			if (tempboard->value[move][x][y] == 0) {
				fail = 0;
				break;
			}
	if (fail == 1)
		return fail;

	do {
		x = rand() % tempboard->info.width;
		y = rand() % tempboard->info.height;
	} while (tempboard->value[move][x][y] != 0);

	tempboard->value[move][x][y] = tileToSpawn;
	tempboard->state[x][y] = justSpawned;

	return fail;
}
int moveTiles(myBoard_t *tempBoard, myDirections_t direction) {
	int preMove = tempBoard->info.move % MOVES_REMEMBERED;
	int currMove = (++(tempBoard->info.move)) % MOVES_REMEMBERED;

	for (int x = 0; x < tempBoard->info.width; ++x)
		for (int y = 0; y < tempBoard->info.height; ++y) {
			tempBoard->value[currMove][x][y] = tempBoard->value[preMove][x][y];
		}
	resetVectorsStates(tempBoard);

	int temp;
	switch (direction)
	{
	case myUp:
		temp = moveUp(tempBoard);
		break;
	case myDown:
		temp = moveDown(tempBoard);
		break;
	case myLeft:
		temp = moveLeft(tempBoard);
		break;
	case myRight:
		temp = moveRight(tempBoard);
		break;
	}

	if (temp == -1)
		--(tempBoard->info.move);
	return temp;
}
int moveLeft(myBoard_t *tempBoard) {
	int valueHolder, moveScore = 0, moveFlag = 0;
	int currMove = tempBoard->info.move % MOVES_REMEMBERED;
	for (int y = 0; y < tempBoard->info.height; ++y)
		for (int x = 0; x < tempBoard->info.width; ++x) {

			if ((valueHolder = tempBoard->value[currMove][x][y]) == 0)
				continue;
			tempBoard->value[currMove][x][y] = 0;

			for (int shiftValue = 1;; ++shiftValue) {
				if (x - shiftValue < 0) {
					tempBoard->value[currMove][x - (shiftValue - 1)][y] = valueHolder;
					break;
				}
				else if (tempBoard->value[currMove][x - shiftValue][y] == 0) {
					moveFlag = 1;
					--(tempBoard->vector[x][y].x);
				}
				else if ((tempBoard->value[currMove][x - shiftValue][y] == valueHolder) && (tempBoard->state[x - shiftValue][y] != justSpawned)) {
					moveFlag = 1;
					--(tempBoard->vector[x][y].x);
					moveScore += tempBoard->value[currMove][x - shiftValue][y] *= 2;
					tempBoard->state[x - shiftValue][y] = justSpawned;
					break;
				}
				else {
					tempBoard->value[currMove][x - (shiftValue - 1)][y] = valueHolder;
					break;
				}
			}
		}
	if (moveFlag)
		return moveScore;
	else
		return -1;
}
int moveRight(myBoard_t *tempBoard) {
	int valueHolder, moveScore = 0, moveFlag = 0;
	int currMove = tempBoard->info.move % MOVES_REMEMBERED;
	for (int y = 0; y < tempBoard->info.height; ++y)
		for (int x = tempBoard->info.width - 1; x >= 0; --x) {

			if ((valueHolder = tempBoard->value[currMove][x][y]) == 0)
				continue;
			tempBoard->value[currMove][x][y] = 0;

			for (int shiftValue = -1;; --shiftValue) {
				if (x - shiftValue >= tempBoard->info.width) {
					tempBoard->value[currMove][x - (shiftValue + 1)][y] = valueHolder;
					break;
				}
				else if (tempBoard->value[currMove][x - shiftValue][y] == 0) {
					moveFlag = 1;
					++(tempBoard->vector[x][y].x);
				}
				else if ((tempBoard->value[currMove][x - shiftValue][y] == valueHolder) && (tempBoard->state[x - shiftValue][y] != justSpawned)) {
					moveFlag = 1;
					++(tempBoard->vector[x][y].x);
					moveScore += tempBoard->value[currMove][x - shiftValue][y] *= 2;
					tempBoard->state[x - shiftValue][y] = justSpawned;
					break;
				}
				else {
					tempBoard->value[currMove][x - (shiftValue + 1)][y] = valueHolder;
					break;
				}
			}
		}
	if (moveFlag)
		return moveScore;
	else
		return -1;
}
int moveUp(myBoard_t *tempBoard) {
	int valueHolder, moveScore = 0, moveFlag = 0;
	int currMove = tempBoard->info.move % MOVES_REMEMBERED;
	for (int x = 0; x < tempBoard->info.width; ++x)
		for (int y = 0; y < tempBoard->info.height; ++y) {

			if ((valueHolder = tempBoard->value[currMove][x][y]) == 0)
				continue;
			tempBoard->value[currMove][x][y] = 0;

			for (int shiftValue = 1;; ++shiftValue) {
				if (y - shiftValue < 0) {
					tempBoard->value[currMove][x][y - (shiftValue - 1)] = valueHolder;
					break;
				}
				else if (tempBoard->value[currMove][x][y - shiftValue] == 0) {
					moveFlag = 1;
					--(tempBoard->vector[x][y].y);
				}
				else if ((tempBoard->value[currMove][x][y - shiftValue] == valueHolder) && (tempBoard->state[x][y - shiftValue] != justSpawned)) {
					moveFlag = 1;
					--(tempBoard->vector[x][y].y);
					moveScore += tempBoard->value[currMove][x][y - shiftValue] *= 2;
					tempBoard->state[x][y - shiftValue] = justSpawned;
					break;
				}
				else {
					tempBoard->value[currMove][x][y - (shiftValue - 1)] = valueHolder;
					break;
				}
			}
		}
	if (moveFlag)
		return moveScore;
	else
		return -1;
}
int moveDown(myBoard_t *tempBoard) {
	int valueHolder, moveScore = 0, moveFlag = 0;
	int currMove = tempBoard->info.move % MOVES_REMEMBERED;
	for (int x = 0; x < tempBoard->info.width; ++x)
		for (int y = tempBoard->info.height - 1; y >= 0; --y) {

			if ((valueHolder = tempBoard->value[currMove][x][y]) == 0)
				continue;
			tempBoard->value[currMove][x][y] = 0;

			for (int shiftValue = -1;; --shiftValue) {
				if (y - shiftValue >= tempBoard->info.height) {
					tempBoard->value[currMove][x][y - (shiftValue + 1)] = valueHolder;
					break;
				}
				else if (tempBoard->value[currMove][x][y - shiftValue] == 0) {
					moveFlag = 1;
					++(tempBoard->vector[x][y].y);
				}
				else if ((tempBoard->value[currMove][x][y - shiftValue] == valueHolder) && (tempBoard->state[x][y - shiftValue] != justSpawned)) {
					moveFlag = 1;
					++(tempBoard->vector[x][y].y);
					moveScore += tempBoard->value[currMove][x][y - shiftValue] *= 2;
					tempBoard->state[x][y - shiftValue] = justSpawned;
					break;
				}
				else {
					tempBoard->value[currMove][x][y - (shiftValue + 1)] = valueHolder;
					break;
				}
			}
		}
	if (moveFlag)
		return moveScore;
	else
		return -1;
}
int handleEvent(myBoard_t *tempBoard, SDL_Event eventHandler, myScreen_t *tempScreen) {
	int moveCheck = -1;
	if (eventHandler.type == SDL_KEYDOWN) {
		switch (eventHandler.key.keysym.sym)
		{
		case SDLK_UP:
			if (tempBoard->info.gameStatus != myWin)
				moveCheck = moveTiles(tempBoard, myUp);
			break;
		case SDLK_DOWN:
			if (tempBoard->info.gameStatus != myWin)
				moveCheck = moveTiles(tempBoard, myDown);
			break;
		case SDLK_LEFT:
			if (tempBoard->info.gameStatus != myWin)
				moveCheck = moveTiles(tempBoard, myLeft);
			break;
		case SDLK_RIGHT:
			if (tempBoard->info.gameStatus != myWin)
				moveCheck = moveTiles(tempBoard, myRight);
			break;
		case SDLK_ESCAPE:
			moveCheck = -2;
			break;
		case SDLK_n:
			moveCheck = -3;
			break;
		case SDLK_v:
			static myState_t fullScreen;
			if (fullScreen == myOn) {
				SDL_SetWindowBordered(tempScreen->window, SDL_TRUE);
				SDL_SetWindowSize(tempScreen->window, SCREEN_WIDTH, SCREEN_HEIGHT);
				SDL_SetWindowPosition(tempScreen->window, WINDOW_POSITION_X, WINDOW_POSITION_Y);
				fullScreen = myOff;
			}
			else {
				SDL_SetWindowBordered(tempScreen->window, SDL_FALSE);
				SDL_MaximizeWindow(tempScreen->window);
				fullScreen = myOn;
			}
			break;
		case SDLK_m:
			if (tempBoard->info.touchpadCtrl == 0) {
				tempBoard->info.touchpadCtrl = 1;
				SDL_WarpMouseInWindow(tempScreen->window, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
				SDL_ShowCursor(SDL_DISABLE);
			}
			else {
				tempBoard->info.touchpadCtrl = 0;
				SDL_ShowCursor(SDL_ENABLE);
				break;
			}
			break;
		case SDLK_b:
			if (tempBoard->info.movesRemembered > 0) {
				tempBoard->info.gameStatus = myNull;
				tempBoard->info.timerStatus = myOn;
				tempBoard->info.move -= 1;
				tempBoard->info.movesRemembered -= 1;
				updateScoreString(tempBoard);
				if (tempBoard->info.move == 0)
					tempBoard->info.timerStatus = myOff;
			}
		}
	}
	else if (eventHandler.type == SDL_MOUSEMOTION && tempBoard->info.touchpadCtrl == 1) {
		int x, y, valCheck = 0;
		static int lastMove;

		SDL_GetMouseState(&x, &y);
		x -= SCREEN_WIDTH / 2;
		y -= SCREEN_HEIGHT / 2;

		if (x != 0 || y != 0)
			if (eventHandler.motion.timestamp - lastMove > TOUCHPAD_MOVE_DELAY) {
				valCheck = 1;
				lastMove = eventHandler.motion.timestamp;
			}

		if (tempBoard->info.gameStatus != myWin && valCheck == 1) {
			if (x > 0 && x > abs(y)) {
				moveCheck = moveTiles(tempBoard, myRight);
			}
			else if (x < 0 && abs(x) > abs(y)) {
				moveCheck = moveTiles(tempBoard, myLeft);
			}
			else if (y > 0) {
				moveCheck = moveTiles(tempBoard, myDown);
			}
			else if (y < 0) {
				moveCheck = moveTiles(tempBoard, myUp);
			}
		}

		SDL_WarpMouseInWindow(tempScreen->window, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
	}
	else if (eventHandler.type == SDL_QUIT) {
		moveCheck = -2;
	}

	return moveCheck;
}
void updateScoreString(myBoard_t *tempBoard) {
	int score = tempBoard->info.score[tempBoard->info.move % MOVES_REMEMBERED];
	int power = 1, i;
	for (i = 0; i < SCORE_DIGITS - 1; ++i)
		power *= 10;
	for (i = 0; i < SCORE_DIGITS; ++i) {
		tempBoard->info.scoreString[i] = score / power + '0';
		score -= (score / power) * power;
		power /= 10;
	}
	tempBoard->info.scoreString[i] = '\0';
	return;
}
int newGame(myBoard_t *tempBoard, myScreen_t *tempScreen) {
	SDL_Event eventHandler;
	int flag = 0; // 1 - exit, 2 - enter
	int width = 4;
	int height = 4;

	do {
		while (SDL_PollEvent(&eventHandler)) {
			if (eventHandler.type == SDL_KEYDOWN) {
				switch (eventHandler.key.keysym.sym)
				{
				case SDLK_UP:
					height -= 1;
					if (height < 2)
						height = 2;
					break;
				case SDLK_DOWN:
					height += 1;
					if (height > MAX_BOARD_HEIGHT)
						height = MAX_BOARD_HEIGHT;
					break;
				case SDLK_LEFT:
					width -= 1;
					if (width < 2)
						width = 2;
					break;
				case SDLK_RIGHT:
					width += 1;
					if (width > MAX_BOARD_WIDTH)
						width = MAX_BOARD_WIDTH;
					break;
				case SDLK_ESCAPE:
					flag = 1;
					break;
				case SDLK_RETURN:
					flag = 2;
					break;
				}
			}
			else if (eventHandler.type == SDL_QUIT)
				flag = 1;
		}
		tempBoard->info.height = height;
		tempBoard->info.width = width;
		SDL_SetRenderDrawColor(tempScreen->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(tempScreen->renderer);
		displayGrid(tempScreen, tempBoard);
		displayText(tempScreen, "Enter to confirm size", BOARD_POSITION_X, height * CELL_HEIGHT + TEXT_MARGIN + BOARD_POSITION_Y, FONT_SIZE / 2);
		SDL_RenderPresent(tempScreen->renderer);
	} while (flag == 0);

	switch (flag) {
	case 1:
		exitSDL(tempScreen);
		SDL_Quit();
		return 1;
	case 2:
		if (initBoard(tempBoard, width, height) == FAIL) {
			exitSDL(tempScreen);
			SDL_Quit();
			return 1;
		}
		break;
	}

	for (int i = 0; i < STARTING_TILES_AMT; ++i)
		spawnTile(tempBoard);


	return 0;
}
void gameStatusCheck(myBoard_t *tempBoard) {
	int fullBoard = 1;
	int temp;
	int currMove = tempBoard->info.move % MOVES_REMEMBERED;
	for (int x = 0; x < tempBoard->info.width; ++x)
		for (int y = 0; y < tempBoard->info.height; ++y) {
			if (tempBoard->value[currMove][x][y] == 0) {
				fullBoard = 0;
			}
			if (tempBoard->value[currMove][x][y] == 2048) {
				tempBoard->info.gameStatus = myWin;
				tempBoard->info.timerStatus = myWin;
				tempBoard->info.endTime = SDL_GetTicks();
				return;
			}
		}
	if (fullBoard == 1) {
		if (moveTiles(tempBoard, myLeft) != -1) {
			--(tempBoard->info.move);
			if (tempBoard->info.movesRemembered > 0)
				--(tempBoard->info.movesRemembered);
			return;
		}
		if (moveTiles(tempBoard, myRight) != -1) {
			--(tempBoard->info.move);
			if (tempBoard->info.movesRemembered > 0)
				--(tempBoard->info.movesRemembered);
			return;
		}
		if (moveTiles(tempBoard, myUp) != -1) {
			--(tempBoard->info.move);
			if (tempBoard->info.movesRemembered > 0)
				--(tempBoard->info.movesRemembered);
			return;
		}
		if (moveTiles(tempBoard, myDown) != -1) {
			--(tempBoard->info.move);
			if (tempBoard->info.movesRemembered > 0)
				--(tempBoard->info.movesRemembered);
			return;
		}
		tempBoard->info.gameStatus = myLose;
		tempBoard->info.timerStatus = myLose;
		tempBoard->info.endTime = SDL_GetTicks();
	}
	return;
}

int initSDL(myScreen_t *tempScreen) {
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
		printf("Failed to load SDL");
		getchar();
		return 1;
	}
	if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, NULL, &(tempScreen->window), &(tempScreen->renderer)) == -1) {
		printf("Failed to open window");
		SDL_Quit();
		getchar();
		return 1;
	}
	SDL_SetWindowTitle(tempScreen->window, WINDOW_TITLE);

	SDL_Surface *tempSurface;

	int i, fail = 0;
	char filePath[] = FILE_PATH;
	char *fileName = filePath + (sizeof(filePath) / sizeof(char) - 7);
	for (i = 0; i < TEXTURES_AMT; ++i) {
		fileName[0] = (i / 10) + '0';
		fileName[1] = (i % 10) + '0';
		if ((tempSurface = SDL_LoadBMP(filePath)) == NULL) {
			fail = 1;
			break;
		}
		if ((tempScreen->texture[i] = SDL_CreateTextureFromSurface(tempScreen->renderer, tempSurface)) == NULL) {
			fail = 1;
			SDL_FreeSurface(tempSurface);
			break;
		}
		SDL_FreeSurface(tempSurface);
	}

	if ((tempSurface = SDL_LoadBMP(CHARSET_PATH)) == NULL && fail == 0) {
		fail = 1;
	}
	if ((tempScreen->charset = SDL_CreateTextureFromSurface(tempScreen->renderer, tempSurface)) == NULL && fail == 0) {
		fail = 1;
	}
	SDL_FreeSurface(tempSurface);

	if (fail) {
		SDL_DestroyRenderer(tempScreen->renderer);
		SDL_DestroyWindow(tempScreen->window);
		for (--i; i >= 0; --i)
			SDL_DestroyTexture(tempScreen->texture[i]);
		printf("Failed to load textures");
		SDL_Quit();
		getchar();
		return fail;
	}


	return 0;
}
void exitSDL(myScreen_t *tempScreen) {
	SDL_DestroyRenderer(tempScreen->renderer);
	SDL_DestroyWindow(tempScreen->window);
	for (int i = 0; i < TEXTURES_AMT; ++i)
		SDL_DestroyTexture(tempScreen->texture[i]);
	SDL_DestroyTexture(tempScreen->charset);
	SDL_Quit();
	return;
}
void displayFrame(myScreen_t *tempScreen, myBoard_t *tempBoard) {
	SDL_SetRenderDrawColor(tempScreen->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(tempScreen->renderer);

	displayGrid(tempScreen, tempBoard);
	displayTiles(tempScreen, tempBoard);
	displayLegend(tempScreen, tempBoard);

	SDL_RenderPresent(tempScreen->renderer);
}
void displayGrid(myScreen_t *tempScreen, myBoard_t *tempBoard) {
	SDL_Rect tempRect;

	tempRect.h = CELL_HEIGHT * tempBoard->info.height;
	tempRect.w = CELL_WIDTH * tempBoard->info.width;
	tempRect.x = BOARD_POSITION_X;
	tempRect.y = BOARD_POSITION_Y;
	SDL_SetRenderDrawColor(tempScreen->renderer, BOARD_COLOR, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(tempScreen->renderer, &tempRect);

	tempRect.h = CELL_HEIGHT * CELL_BACKGROUND_RATIO;
	tempRect.w = CELL_WIDTH * CELL_BACKGROUND_RATIO;
	SDL_SetRenderDrawColor(tempScreen->renderer, CELL_COLOR, SDL_ALPHA_OPAQUE);
	for (int x = 0; x < tempBoard->info.width; ++x)
		for (int y = 0; y < tempBoard->info.height; ++y) {
			tempRect.x = ((x + (1 - CELL_BACKGROUND_RATIO) / 2) * CELL_WIDTH) + BOARD_POSITION_X;
			tempRect.y = ((y + (1 - CELL_BACKGROUND_RATIO) / 2) * CELL_HEIGHT) + BOARD_POSITION_Y;
			SDL_RenderFillRect(tempScreen->renderer, &tempRect);
		}
	return;
}
void displayTiles(myScreen_t *tempScreen, myBoard_t *tempBoard) {
	int move = tempBoard->info.move % MOVES_REMEMBERED;
	SDL_Rect tempRect;
	tempRect.h = CELL_HEIGHT * CELL_TILE_RATIO;
	tempRect.w = CELL_WIDTH * CELL_TILE_RATIO;

	int i, power;
	for (int x = 0; x < tempBoard->info.width; ++x)
		for (int y = 0; y < tempBoard->info.height; ++y) {
			if (tempBoard->value[move][x][y] != 0) {
				tempRect.x = (x + (1 - CELL_TILE_RATIO) / 2) * CELL_WIDTH + BOARD_POSITION_X;
				tempRect.y = (y + (1 - CELL_TILE_RATIO) / 2) * CELL_HEIGHT + BOARD_POSITION_Y;
				i = 0;
				power = 1;
				while (tempBoard->value[move][x][y] % (power *= 2) == 0)
					++i;
				SDL_RenderCopy(tempScreen->renderer, tempScreen->texture[i - 1], NULL, &tempRect);
			}

		}
	return;
}
void animate(myScreen_t *tempScreen, myBoard_t *tempBoard) {
	double baseK = 1.0 / (ANIMATION_FRAMES * ANIMATION_RATIO);
	int preMove = (tempBoard->info.move - 1) % MOVES_REMEMBERED;
	int currMove = tempBoard->info.move % MOVES_REMEMBERED;
	int i, power;
	SDL_Rect tempRect;
	tempRect.h = CELL_HEIGHT * CELL_TILE_RATIO;
	tempRect.w = CELL_WIDTH * CELL_TILE_RATIO;

	for (double k = 0; k < 1; k += baseK) {
		SDL_SetRenderDrawColor(tempScreen->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(tempScreen->renderer);
		displayGrid(tempScreen, tempBoard);
		displayLegend(tempScreen, tempBoard);

		for (int x = 0; x < tempBoard->info.width; ++x)
			for (int y = 0; y < tempBoard->info.height; ++y) {
				if (tempBoard->value[preMove][x][y] != 0) {
					tempRect.x = ((x + (1 - CELL_TILE_RATIO) / 2) + tempBoard->vector[x][y].x * k) * CELL_WIDTH + BOARD_POSITION_X;
					tempRect.y = ((y + (1 - CELL_TILE_RATIO) / 2) + tempBoard->vector[x][y].y * k) * CELL_HEIGHT + BOARD_POSITION_Y;
					i = 0;
					power = 1;
					while (tempBoard->value[preMove][x][y] % (power *= 2) == 0)
						++i;
					SDL_RenderCopy(tempScreen->renderer, tempScreen->texture[i - 1], NULL, &tempRect);
				}
			}
		SDL_RenderPresent(tempScreen->renderer);
		SDL_Delay(ANIMATION_FRAME_DELAY);
	}
	baseK = 1.0 / (ANIMATION_FRAMES * (1 - ANIMATION_RATIO));
	for (double k = 0; k < 1; k += baseK) {
		SDL_SetRenderDrawColor(tempScreen->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(tempScreen->renderer);
		displayGrid(tempScreen, tempBoard);
		displayLegend(tempScreen, tempBoard);
		for (int x = 0; x < tempBoard->info.width; ++x)
			for (int y = 0; y < tempBoard->info.height; ++y) {
				if (tempBoard->value[currMove][x][y] != 0) {
					if (tempBoard->state[x][y] == justSpawned) {
						tempRect.w = CELL_WIDTH * CELL_TILE_RATIO * k;
						tempRect.h = CELL_HEIGHT * CELL_TILE_RATIO * k;
					}
					else {
						tempRect.w = CELL_WIDTH * CELL_TILE_RATIO;
						tempRect.h = CELL_HEIGHT * CELL_TILE_RATIO;
					}
					tempRect.x = (x + (1 - CELL_TILE_RATIO) / 2) * CELL_WIDTH + BOARD_POSITION_X;
					tempRect.y = (y + (1 - CELL_TILE_RATIO) / 2) * CELL_HEIGHT + BOARD_POSITION_Y;
					i = 0;
					power = 1;
					while (tempBoard->value[currMove][x][y] % (power *= 2) == 0)
						++i;
					SDL_RenderCopy(tempScreen->renderer, tempScreen->texture[i - 1], NULL, &tempRect);
				}

			}
		SDL_RenderPresent(tempScreen->renderer);
		SDL_Delay(ANIMATION_FRAME_DELAY);
	}
	return;
}
void displayText(myScreen_t *tempScreen, const char text[], int x, int y, int size) {
	int character;
	SDL_Rect src, dst;
	dst.w = size;
	dst.h = size;
	src.w = 8;
	src.h = 8;
	for (int i = 0; text[i] != '\0'; ++i) {
		character = text[i] & 255;
		src.x = (character % 16) * 8;
		src.y = (character / 16) * 8;
		dst.x = x;
		dst.y = y;
		SDL_RenderCopy(tempScreen->renderer, tempScreen->charset, &src, &dst);
		x += size;
	};
}
void displayLegend(myScreen_t *tempScreen, myBoard_t *tempBoard) {
	int x = BOARD_POSITION_X + CELL_WIDTH * tempBoard->info.width + TEXT_MARGIN * 2;
	int y = BOARD_POSITION_Y;
	displayText(tempScreen, "Score", x, y, FONT_SIZE);
	y += FONT_SIZE + TEXT_MARGIN;
	displayText(tempScreen, tempBoard->info.scoreString, x, y, FONT_SIZE);

	y += FONT_SIZE + TEXT_MARGIN * 4;
	displayText(tempScreen, "Time", x, y, FONT_SIZE);
	int currTime;
	switch (tempBoard->info.timerStatus) {
	case myOn:
		currTime = SDL_GetTicks() - tempBoard->info.time;
		break;
	case myOff:
		currTime = 0;
		break;
	case myWin:
		currTime = tempBoard->info.endTime;
	case myLose:
		currTime = tempBoard->info.endTime;
	}
	char timeString[TIME_STRING_ARRAY_SIZE];
	int i = 0;
	timeString[i] = '0' + (currTime / 600000) % 10;
	timeString[++i] = '0' + (currTime / 60000) % 10;
	timeString[++i] = ':';
	timeString[++i] = '0' + (currTime / 10000) % 6;
	timeString[++i] = '0' + (currTime / 1000) % 10;
	timeString[++i] = ':';
	timeString[++i] = '0' + (currTime / 10) % 10;
	timeString[++i] = '0' + currTime % 10;
	timeString[++i] = '\0';
	y += FONT_SIZE + TEXT_MARGIN;
	displayText(tempScreen, timeString, x, y, FONT_SIZE);

	y += FONT_SIZE + TEXT_MARGIN * 4;
	displayText(tempScreen, "Move", x, y, FONT_SIZE);
	char moveString[MOVE_DIGITS];
	moveString[MOVE_DIGITS - 1] = '\0';
	int multip = 1;
	for (i = MOVE_DIGITS - 2; i >= 0; --i) {
		moveString[i] = '0' + (tempBoard->info.move / multip) % 10;
		multip *= 10;
	}
	y += FONT_SIZE + TEXT_MARGIN;
	displayText(tempScreen, moveString, x, y, FONT_SIZE);
	if (tempBoard->info.gameStatus == myWin)
		displayText(tempScreen, "You won!", BOARD_POSITION_X, tempBoard->info.height * CELL_HEIGHT + TEXT_MARGIN + BOARD_POSITION_Y, FONT_SIZE / 2);
	else if (tempBoard->info.gameStatus == myLose)
		displayText(tempScreen, "You lost", BOARD_POSITION_X, tempBoard->info.height * CELL_HEIGHT + TEXT_MARGIN + BOARD_POSITION_Y, FONT_SIZE / 2);
	return;
}