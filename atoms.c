#include "atoms.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

int started = 0; // started boolean
int loaded = 0; // loaded boolean
const char colours[] = "RGPBYW"; // Characters for Game Board
int gameHeight = 0; // Game Height
int gameWidth = 0; // Game Width
int noPlayers = 0; // # of Players
struct grid_t **grid; //Game Board
struct game_t currentGame; // *moves struct
struct player_t *players; // Players data
uint32_t* raw; // raw move data
int turnNumber = 0;
int turnIndex = 0; // player index to read char from colours
int playersLeft = 0; // players still in game
int loadedMoves = 0;
void playFromEnd(void);

void helpCommand(void) {
	if (loaded == 1) {
		printf("Invalid Command\n\n");
		return;
	}
	printf("\nHELP displays this help message\n");
	printf("QUIT quits the current game\n\n");
	printf("DISPLAY draws the game board in terminal\n");
	printf("START <number of players> <width> <height> starts the game\n");
	printf("PLACE <x> <y> places an atom in a grid space\n");
	printf("UNDO undoes the last move made\n");
	printf("STAT displays game statistics\n\n");
	printf("SAVE <filename> saves the state of the game\n");
	printf("LOAD <filename> loads a save file\n");
	printf("PLAYFROM <turn> plays from n steps into the game\n\n");
}

void quitCommand(void) {
	for (int i = 0; i < gameHeight; i++) {
		free(grid[i]);
	}
	free(grid);
	free(players);
	free(raw);
	printf("Bye!\n");
	exit(0);
}

void displayCommand(void) {
	if (loaded == 1) {
		printf("Invalid Command\n\n");
		return;
	}
	if (started == 0){
		printf("Invalid Command\n\n");
		return;
	}
	printf("\n+");
	for (int i = 0; i < (gameWidth*3)-1; i++) {
		printf("-");
	}
	printf("+\n");
	for (int i = 0; i < gameHeight; i++) {
		for (int j = 0; j < gameWidth; j++) {
			printf("|");
			if (grid[i][j].atom_count == 0) {
				printf("  ");
			} else {
				printf("%c%d", grid[i][j].owner,grid[i][j].atom_count);
			}
		}
		printf("|\n");
	}
	printf("+");
	for (int i = 0; i < (gameWidth*3)-1; i++) {
		printf("-");
	}
	printf("+\n\n");
}

void startCommand(int k, int width, int height){
	if (loaded == 1) {
		printf("Invalid Command\n\n");
		return;
	}
	if (started == 1){
		printf("Invalid Command\n\n");
		return;
	}
	if (k < 0 || width < 0 || height < 0) {
		printf("Invalid command arguments\n\n");
		return;
	}
	if (width*height < k) {
		printf("Cannot Start Game\n\n");
		return;
	}
	if (k < MIN_PLAYERS || k > MAX_PLAYERS || width < MIN_WIDTH || MAX_WIDTH > 255 || height < MIN_HEIGHT || height > MAX_HEIGHT) {
		printf("Invalid command arguments\n\n");
		return;
	}
	started = 1;
	gameHeight = height;
	gameWidth = width;
	noPlayers = k;
	playersLeft = k;
	turnIndex = 0;
	turnNumber = 0;

	currentGame.moves = malloc(sizeof(struct move_t) * 1024);
	raw = (uint32_t*)malloc(sizeof(uint32_t) * 1024);

	grid = (grid_t**)malloc(sizeof(struct grid_t*)*height);

	for (int i = 0; i < height; i++) {
		grid[i] = (grid_t*) malloc(sizeof(struct grid_t)*width);
	}

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			grid[i][j].owner = '\0';
			grid[i][j].atom_count = 0;
		}
	}

	printf("Game Ready\n");
	printf("Red's Turn\n\n");

	players = (player_t*)calloc(k, sizeof(struct player_t));

	for (int i = 0; i < k; i++) {
		players[i].grids_owned = 0;
		players[i].lost = '0';
		if (i == 0) {
			players[i].colour = "Red";
		} else if (i == 1) {
			players[i].colour = "Green";
		} else if (i == 2) {
			players[i].colour = "Purple";
		} else if (i == 3) {
			players[i].colour = "Blue";
		} else if (i == 4) {
			players[i].colour = "Yellow";
		} else if (i == 5) {
			players[i].colour = "White";
		}
	}
}

void updateCount(int q) {
	int oldCount = players[q].grids_owned;
	int count = 0;
	for (int i = 0; i < gameHeight; i++) {
		for (int j = 0; j < gameWidth; j++) {
			if (grid[i][j].owner == colours[q]){
				count++;
			}
		}
	}
	players[q].grids_owned = count;
	if (oldCount > 0 && count == 0){
		players[q].lost = '1';
		playersLeft--;
	}
}

void updateStatus () {
	for (int i = 0; i < noPlayers; i++){
		updateCount(i);
	}
	if (playersLeft == 1) {
		for (int i = 0; i < noPlayers; i++) {
			if (players[i].lost == '0') {
				printf("%s Wins!\n", players[i].colour);
			}
		}
		started = 0;
		for (int i = 0; i < gameHeight; i++) {
			free(grid[i]);
		}
		free(grid);
		free(players);
		exit(0);
	}
}

void top(int y, int x) {
	grid[y-1][x].atom_count++; // Update Height
	grid[y-1][x].owner = colours[turnIndex]; // Update Ownership
}

void right(int y, int x) {
	grid[y][x+1].atom_count++; // right
	grid[y][x+1].owner = colours[turnIndex];
}

void bottom(int y, int x) {
	grid[y+1][x].atom_count++; // bottom
	grid[y+1][x].owner = colours[turnIndex];
}

void left(int y, int x) {
	grid[y][x-1].atom_count++;
	grid[y][x-1].owner = colours[turnIndex];
}

void reset(int y, int x) {
	grid[y][x].atom_count = 0; // make current number 0
	grid[y][x].owner = '\0'; // Get rid of ownership
}

// Recursive Expansion Algorithm for Overflows
void expansion(int y, int x) {
	// If its a corner && height is greated than one, overflow!!
	updateStatus();
	if (grid[y][x].atom_count > 1 && ((x == 0 && y == 0) || (x == gameWidth-1 && y == 0) || (x == 0 && y == gameHeight-1) || (x == gameWidth-1 && y == gameHeight-1))){
		reset(y,x);
		if (y == 0 && x == 0) { // top left corner
			right(y,x);
			expansion(y,x+1); // Recurse onto grids overflowed onto
			bottom(y,x);
			expansion(y+1,x);
		} else if (y == 0 && x == gameWidth-1) { // top right corner
			bottom(y,x);
			expansion(y+1,x);
			left(y,x);
			expansion(y,x-1);
		} else if (y == gameHeight-1 && x == 0) { // bottom left corner
			top(y,x);
			expansion(y-1,x);
			right(y,x);
			expansion(y,x+1);
		} else if (y == gameHeight-1 && x == gameWidth-1) { // bottom right corner
			top(y,x);
			expansion(y-1,x);
			left(y,x);
			expansion(y,x-1);
		}
	// If its a side and height is greater than two, overflow!!!!
	} else if (grid[y][x].atom_count > 2 && (y == 0 || x == 0 || y == gameHeight-1 || x == gameWidth-1)) {
		reset(y,x);
		if (y == 0) { // top edge
			right(y,x);
			expansion(y,x+1);
			bottom(y,x);
			expansion(y+1,x);
			left(y,x);
			expansion(y,x-1);
		} else if (x == 0) { // left edge
            top(y,x);
			expansion(y-1,x);
			right(y,x);
			expansion(y,x+1);
			bottom(y,x);
			expansion(y+1,x);
		} else if (y == gameHeight-1) { // bottom edge
			top(y,x);
			expansion(y-1,x);
			right(y,x);
			expansion(y,x+1);
			left(y,x);
			expansion(y,x-1);
		} else if (x == gameWidth-1) { // right edge
			top(y,x);
			expansion(y-1,x);
			bottom(y,x);
			expansion(y+1,x);
			left(y,x);
			expansion(y,x-1);
		}
	// Otherwise, if height is greater than three, overflow!!!
	} else if (grid[y][x].atom_count > 3) {
		reset(y,x);
		top(y,x);
		expansion(y-1,x);
		right(y,x);
		expansion(y,x+1);
		bottom(y,x);
		expansion(y+1,x);
		left(y,x);
		expansion(y,x-1);
	}
}

void placeCommand(int x, int y){
	if (loaded == 1) {
		printf("Invalid Command\n\n");
		return;
	}
	if (started == 0) {
		printf("Game Not In Progress\n\n");
		return;
	}
	if (x < 0 || x >= gameWidth || y < 0 || y >= gameHeight) {
		printf("Invalid coordinates\n\n");
		return;
	}
	if (grid[y][x].atom_count > 0) {
		if (grid[y][x].owner != colours[turnIndex]) {
			printf("Cannot Place Atom Here\n\n");
			return;
		} else {
			grid[y][x].atom_count++;
			expansion(y,x);
		}
	} else {
		grid[y][x].atom_count++;
		grid[y][x].owner = colours[turnIndex];
	}

	updateStatus();
	int lastIndex = turnIndex;
	if (turnIndex == noPlayers-1){
		turnIndex = 0;
	} else {
		turnIndex++;
	}

	while (players[turnIndex].lost == '1') {
		if (turnIndex == noPlayers-1){
			turnIndex = 0;
		} else {
			turnIndex++;
		}
	}

	if (turnNumber == 0) {
		currentGame.moves[turnNumber].x = x;
		currentGame.moves[turnNumber].y = y;
	} else {
		currentGame.moves[turnNumber].x = x;
		currentGame.moves[turnNumber].y = y;
		currentGame.moves[turnNumber].parent = &currentGame.moves[turnNumber-1];
		currentGame.moves[turnNumber].old_owner = &players[lastIndex];
	}
	uint8_t xraw = x;
	uint8_t yraw = y;
	uint32_t move = yraw;
	move = (move << 8) + xraw;
	raw[turnNumber] = move;
	turnNumber++;
	printf("%s's Turn\n\n", players[turnIndex].colour);
}

// Same as place, without printing stuff
void buildProgram(int x, int y){
	if (grid[y][x].atom_count > 0) {
		grid[y][x].atom_count++;
		expansion(y,x);
	} else {
		grid[y][x].atom_count++;
		grid[y][x].owner = colours[turnIndex];
	}
	for (int i = 0; i < noPlayers; i++){
		updateCount(i);
	}
	int lastIndex = turnIndex;
	if (turnIndex == noPlayers-1){
		turnIndex = 0;
	} else {
		turnIndex++;
	}
	while (players[turnIndex].lost == '1') {
		if (turnIndex == noPlayers-1){
			turnIndex = 0;
		} else {
			turnIndex++;
		}
	}
	if (turnNumber == 0) {
		currentGame.moves[turnNumber].x = x;
		currentGame.moves[turnNumber].y = y;
	} else {
		currentGame.moves[turnNumber].x = x;
		currentGame.moves[turnNumber].y = y;
		currentGame.moves[turnNumber].parent = &currentGame.moves[turnNumber-1];
		currentGame.moves[turnNumber].old_owner = &players[lastIndex];
	}
	turnNumber++;
}

void undoCommand(void){
	if (loaded == 1) {
		printf("Invalid Command\n\n");
		return;
	}
	if (turnNumber == 0) {
		printf("Cannot Undo\n\n");
		return;
	}

	for (int i = 0; i < gameHeight; i++) {
		for (int j = 0; j < gameWidth; j++) {
			grid[i][j].owner = '\0';
			grid[i][j].atom_count = 0;
		}
	}

	int newTurnNumber = turnNumber-1;
	turnIndex = 0;
	turnNumber = 0;
	for (int i = 0; i < noPlayers; i++){
		players[i].lost = '0';
		players[i].grids_owned = 0;
	}
	for (int i = 0; i < newTurnNumber; i++){
		int x = currentGame.moves[i].x;
		int y = currentGame.moves[i].y;
		buildProgram(x,y);
	}
	printf("%s's Turn\n\n", players[turnIndex].colour);

}

void statCommand(void){
	if (loaded == 1) {
		printf("Invalid Command\n\n");
		return;
	}
	if (started == 0) {
		printf("Game Not In Progress\n\n");
		return;
	}

	for (int i = 0; i < noPlayers; i++) {
		printf("Player %s:\n", players[i].colour);
		if (players[i].lost == '1') {
			printf("Lost\n\n");
		} else {
			printf("Grid Count: %d\n\n", players[i].grids_owned);
		}
	}
}

void saveCommand(char filename[]){

	if (loaded == 1) {
		printf("Invalid Command\n\n");
		return;
	}

	if (turnNumber == 0) {
		printf("Invalid Command\n\n");
		return;
	}

	if (strlen(filename) > 255) {
		printf("Invalid Command\n\n");
		return;
	}

	struct save_file_t saveFile = {gameWidth, gameHeight, noPlayers, raw};
	if(fopen(filename, "rb") != '\0') {
		printf("This file already exists\n\n");
		return;
	}
	FILE* file = fopen(filename,"w+b");
	fwrite(&saveFile.width, sizeof(uint8_t), 1, file);
	fwrite(&saveFile.height, sizeof(uint8_t), 1, file);
	fwrite(&saveFile.no_players, sizeof(uint8_t), 1, file);
	for (int i = 0; i < turnNumber; i++) {
		fwrite(&saveFile.raw_move_data[i], sizeof(uint32_t), 1, file);
	}
	fclose(file);
	printf("Game Saved\n\n");
}

void loadCommand(char filename[]){
	if(fopen(filename, "rb") == '\0') {
		printf("Cannot Load Save\n\n");
		return;
	}

	if (loaded == 1 || started == 1) {
		printf("Restart Application To Load Save\n\n");
		return;
	}

	loaded = 1;
	started = 1;

	printf("Game Loaded\n\n");
	currentGame.moves = malloc(sizeof(struct move_t) * 1024);
	raw = (uint32_t*)malloc(sizeof(uint32_t) * 1024);

	FILE* file = fopen(filename,"rb");
	uint8_t width = 0;
	uint8_t height = 0;
	uint8_t playerz = 0;
	fread(&width, 1, 1, file);
	fread(&height, 1, 1, file);
	fread(&playerz, 1, 1, file);
	gameWidth = width;
	gameHeight = height;
	noPlayers = playerz;

	while (!feof(file)) {
		fread(&raw[turnNumber], sizeof(uint32_t), 1, file);
		turnNumber++;
		loadedMoves++;
	}
	fclose(file);
	playersLeft = noPlayers;

	grid = (grid_t**)malloc(sizeof(struct grid_t*)*gameHeight);

	for (int i = 0; i < gameHeight; i++) {
		grid[i] = (grid_t*) malloc(sizeof(struct grid_t)*gameWidth);
	}

	for (int i = 0; i < gameHeight; i++) {
		for (int j = 0; j < gameWidth; j++) {
			grid[i][j].owner = '\0';
			grid[i][j].atom_count = 0;
		}
	}

	players = (player_t*)calloc(noPlayers, sizeof(struct player_t));

	for (int i = 0; i < noPlayers; i++) {
		players[i].grids_owned = 0;
		players[i].lost = '0';
		if (i == 0) {
			players[i].colour = "Red";
		} else if (i == 1) {
			players[i].colour = "Green";
		} else if (i == 2) {
			players[i].colour = "Purple";
		} else if (i == 3) {
			players[i].colour = "Blue";
		} else if (i == 4) {
			players[i].colour = "Yellow";
		} else if (i == 5) {
			players[i].colour = "White";
		}
	}
}

void playFromCommand(int turn){
	if (started == 1 && loaded == 0){
		printf("Invalid Command\n\n");
		return;
	}
	if (turn < 0) {
		printf("Invalid Turn Number\n\n");
		return;
	}
	if (turn > loadedMoves-1) {
		playFromEnd();
		return;
	}
	turnIndex = 0;
	turnNumber = 0;
	for (int i = 0; i < turn; i++){
		int y = (raw[i]>>8);
		int x = (raw[i] << 24) >> 24;
		buildProgram(x,y);
	}
	printf("Game Ready\n");
	printf("%s's Turn\n\n", players[turnIndex].colour);
	loaded = 0;
}

void playFromEnd(void){
	if (started == 1 && loaded == 0){
		printf("Invalid Command\n\n");
		return;
	}
	turnIndex = 0;
	turnNumber = 0;
	for (int i = 0; i < loadedMoves-1; i++){
		int y = (raw[i]>>8);
		int x = (raw[i] << 24) >> 24;
		buildProgram(x,y);
	}
	printf("Game Ready\n");
	printf("%s's Turn\n\n", players[turnIndex].colour);
	loaded = 0;
}

int main(void) {
	char input[MAX_LINE];
	char name[MAX_LINE];
	int num1 = 0;
	int num2 = 0;
	int num3 = 0;
	char char1 = '\0';
	char char2 = '\0';
	char char3 = '\0';
	while (1){
		fgets(input, MAX_LINE, stdin);
		input[strcspn(input, "\n")] = '\0';
		if (strcmp(input, "HELP") == 0) {
			helpCommand();
		} else if (strcmp(input, "QUIT") == 0) {
			quitCommand();
			break;
		} else if (strcmp(input, "DISPLAY") == 0) {
			displayCommand();
		} else if(sscanf(input, "START %d %d %d %s", &num1, &num2, &num3, name) > 3) {
			printf("Too Many Arguments\n\n");
		} else if(sscanf(input, "START %c %c %c", &char1, &char2, &char3) > 0 && (!isdigit(char1) || !isdigit(char2) || !isdigit(char3))) {
			printf("Invalid command arguments\n\n");
		} else if(sscanf(input, "START %d %d %d", &num1, &num2, &num3) == 3) {
			startCommand(num1, num2, num3);
		} else if(sscanf(input, "START %d %d %d", &num1, &num2, &num3) < 3 && sscanf(input, "START %d %d %d", &num1, &num2, &num3) > 0) {
			printf("Missing Argument\n\n");
		} else if (strcmp(input, "START") == 0) {
			printf("Missing Argument\n\n");
		} else if (sscanf(input, "PLACE %d %d %s", &num1, &num2, name) > 2) {
			printf("Invalid Command\n\n");
		} else if (sscanf(input, "PLACE %d %d", &num1, &num2) == 2) {
			placeCommand(num1, num2);
		} else if (strcmp(input, "UNDO") == 0) {
			undoCommand();
		} else if (strcmp(input, "STAT") == 0) {
			statCommand();
		} else if(sscanf(input, "SAVE %s %s", name, name) > 1){
			printf("Invalid Command\n\n");
		} else if(sscanf(input, "SAVE %s", name) == 1){
			saveCommand(name);
		} else if(sscanf(input, "LOAD %s %s", name, name) > 1){
			printf("Invalid Command\n\n");
		} else if(sscanf(input, "LOAD %s", name) == 1){
			loadCommand(name);
		} else if(sscanf(input, "PLAYFROM %d %s", &num1, name) > 1){
			printf("Invalid Command \n\n");
		} else if (strcmp(input, "PLAYFROM END") == 0) {
			playFromEnd();
		} else if(sscanf(input, "PLAYFROM %d", &num1) == 1){
			playFromCommand(num1);
		} else {
			printf("Invalid Command\n\n");
		}
	}
}




				 			
