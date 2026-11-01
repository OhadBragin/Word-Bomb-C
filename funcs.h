#pragma once
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <time.h>

#define MAX_WORD_LENGTH 50
#define PROMPT_LENGTH 3

typedef struct {
	int score;
	time_t initial_time;
	int time_left;
	int time_to_answer;
	time_t time_since_answer;
	time_t current_time;
	int time_per_prompt;
	char current_prompt[PROMPT_LENGTH];
	char input_buffer[MAX_WORD_LENGTH];
	char error_message[MAX_WORD_LENGTH];
	char last_prompt[MAX_WORD_LENGTH];
	char error_color[MAX_WORD_LENGTH];
	int added_score;
	int lives;
	int userChoice;
	int gamemodeChoice;
	int input_index;
	char gamemode[10];
	int can_skip;
	int timeLimit;
	int isLives;
	int randomLength;
	int combo;
	float score_multiplier;
	double avgTime;
	int timeCounter;
	int correctCounter;

} Game_t;

typedef struct {
	int survival;
	int speed;
	int zen;
	int match;
} HighScore_t;

typedef struct {
	int survivalTimeCounter;
	int survivalCorrectCounter;
	int speedTimeCounter;
	int speedCorrectCounter;
	int zenTimeCounter;
	int zenCorrectCounter;
	int matchTimeCounter;
	int matchCorrectCounter;
} AvgTime_t;

typedef struct {
	int survival;
	int speed;
	int zen;
	int match;
} LongestCombo_t;




void initialize_game_state();
void initialize_time();
int randomPercent(int percent);
int binarySearchWord(FILE* pWordBase, char* userWord);
char getVowel();
void getLetters();
int validateInput();
int containLetters();
void printLogo();
void printLogoWithTick();
void writeCharToFile(char* word, char* filePath);
void writeIntToFile(int value, char* filePath);
void resetFile(char* filePath);
int handleInput();
void hideCursor();
void drawFrame();
void drawMenu();
void printBasicRules();
void handleUserChoice();
void handleScore();
void generateNewState();
void survivalGameLoop();
void speedGameLoop();
void gameOver();
void printSurvivalRules();
void printSpeedRules();
void zenGameLoop();
void printZenRules();
int getRandomLength();
int getWordLength(char* word);
int validateInputMatchMode();
void matchGameLoop();
void printMatchModeRules();
void loadHighScores(HighScore_t* highScores);
void saveHighScores(HighScore_t* highScores);
void displayHighScores();
void loadAvgTimes(AvgTime_t* avgTime);
void saveAvgTime(AvgTime_t* avgTime);
void printSurvivalStats();
#endif
