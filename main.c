#define _CRT_SECURE_NO_WARNINGS



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <assert.h>
#include <locale.h>
#include <windows.h>
#include <conio.h>


#include "funcs.h" // funcs file

extern Game_t game;
extern HighScore_t highScores;
extern LongestCombo_t longestCombo;
extern AvgTime_t avgTime;


int main() {
	srand(time(NULL)); // set random seed
	loadLongestCombo(&longestCombo);
	loadHighScores(&highScores);
	loadAvgTimes(&avgTime);
	while (1)
	{
		hideCursor();
		game.userChoice = -1;
		//printLogoWithTick();
		drawMenu();
		game.userChoice = _getch();
		handleUserChoice();	
	}
	return 0;
}