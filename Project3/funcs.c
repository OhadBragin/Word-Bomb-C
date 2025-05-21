#define _CRT_SECURE_NO_WARNINGS

//colors
#define RESET   "\x1b[0m"
#define PASTEL_RED     "\x1b[38;5;211m"  // Light Red
#define PASTEL_GREEN   "\x1b[38;2;119;221;119m"  // Light Green
#define PASTEL_YELLOW  "\x1b[38;5;227m"  // Light Yellow
#define PASTEL_BLUE    "\x1b[38;5;117m"  // Light Blue
#define PASTEL_MAGENTA "\x1b[38;5;213m"  // Light Magenta
#define PASTEL_CYAN    "\x1b[38;5;123m"  // Light Cyan
#define PASTEL_WHITE   "\x1b[38;5;255m"  // Light White
#define RED "\033[1;31m" // Red

//defenitions
#define MAX_WORD_LENGTH 50
#define PROMPT_LENGTH 3
#define WAIT_TIME 1000
#define BASE_POINTS 300

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

//commands
#define CLEAR_CONSOLE "\033[H\033[J"

Game_t game;
HighScore_t highScores;
AvgTime_t avgTime;
LongestCombo_t longestCombo;

void loadLongestCombo(LongestCombo_t* longestCombo)
{
	FILE* pLongestCombo = fopen("longestcombo.bin", "rb");
	if (pLongestCombo == NULL) {
		// Initialize with zeros if file doesn't exist
		longestCombo->survival = 0;
		longestCombo->speed = 0;
		longestCombo->zen = 0;
		longestCombo->match = 0;
		return;
	}
	fread(longestCombo, sizeof(LongestCombo_t), 1, pLongestCombo);
	fclose(pLongestCombo);
}

void loadHighScores(HighScore_t* highScores)
{
	FILE* pHighScores = fopen("highscores.bin", "rb");
	if (pHighScores == NULL) {
		// Initialize with zeros if file doesn't exist
		highScores->survival = 0;
		highScores->speed = 0;
		highScores->zen = 0;
		highScores->match = 0;
		return;
	}
	fread(highScores, sizeof(HighScore_t), 1, pHighScores);
	fclose(pHighScores);
}

void loadAvgTimes(AvgTime_t* avgTime)
{
	FILE* pAvgTimes = fopen("avgtime.bin", "rb");
	if (pAvgTimes == NULL) {
		// Initialize with zeros if file doesn't exist
		avgTime->survivalTimeCounter = 0;
		avgTime->survivalCorrectCounter = 0;
		avgTime->speedTimeCounter = 0;
		avgTime->speedCorrectCounter = 0;
		avgTime->zenTimeCounter = 0;
		avgTime->zenCorrectCounter = 0;
		avgTime->matchTimeCounter = 0;
		avgTime->matchCorrectCounter = 0;
		return;
	}
	fread(avgTime, sizeof(AvgTime_t), 1, pAvgTimes);
	fclose(pAvgTimes);
}

void saveHighScores(HighScore_t* highScores)
{
	FILE* pHighScores = fopen("highscores.bin", "wb");
	if (pHighScores == NULL) {
		strcpy(game.error_message, "Error saving high scores!");
		strcpy(game.error_color, RED);
		return;
	}
	fwrite(highScores, sizeof(HighScore_t), 1, pHighScores);
	fclose(pHighScores);
}

void saveAvgTime(AvgTime_t* avgTime)
{
	FILE* pAvgTimes = fopen("avgtime.bin", "wb");
	if (pAvgTimes == NULL) {
		strcpy(game.error_message, "Error saving high scores!");
		strcpy(game.error_color, RED);
		return;
	}
	fwrite(avgTime, sizeof(AvgTime_t), 1, pAvgTimes);
	fclose(pAvgTimes);
}

void saveLongestCombo(LongestCombo_t* longestCombo)
{
	FILE* pLongestCombo = fopen("longestcombo.bin", "wb");
	if (pLongestCombo == NULL) {
		strcpy(game.error_message, "Error saving longest combo!");
		strcpy(game.error_color, RED);
		return;
	}
	fwrite(longestCombo, sizeof(LongestCombo_t), 1, pLongestCombo);
	fclose(pLongestCombo);
}

void initialize_game_state()
{
	loadLongestCombo(&longestCombo);
	loadHighScores(&highScores);
	loadAvgTimes(&avgTime);
	resetFile("used_words.txt");
	memset(game.error_message, 0, sizeof(game.error_message));
	game.lives = 3;
	game.score = 0;
	game.added_score = 0;
	game.input_index = 0;
	game.combo = 0;
	game.score_multiplier = 1.0f;
	game.randomLength = getRandomLength();
	game.timeCounter = 0;
	game.correctCounter = 0;
	game.time_since_answer = time(NULL);
}

void initialize_time() // resets time
{
	game.initial_time = time(NULL);
	game.time_since_answer = time(NULL);
	game.time_left = 0;
	game.time_to_answer = 0;
}

int randomPercent(int percent) //takes a percentage and returns true or false, based on the chance
{
	if (percent <= 0) return 0;
	if (percent >= 100) return 1;
	return (rand() % 100 + 1) <= percent;
}

int binarySearchWord(FILE* pWordBase, char* userWord) //searches a word in an ABC sorted file. return true if it exists, false if not.
{
	// Find file size
	fseek(pWordBase, 0, SEEK_END);
	long fileSize = ftell(pWordBase);
	rewind(pWordBase);

	char word[100];
	long left = 0;
	long right = fileSize;

	while (left <= right) {
		long mid = left + (right - left) / 2;

		// Seek to the middle position
		fseek(pWordBase, mid, SEEK_SET);

		// If not at the beginning of the file, we need to find the start of the next line
		if (mid > 0) {
			// Skip the current (potentially partial) line to get to the next complete line
			if (fgets(word, sizeof(word), pWordBase) == NULL) {
				return 0;  // Reached end of file
			}
		}

		// Now read the complete line
		if (fgets(word, sizeof(word), pWordBase) == NULL) {
			// If we can't read a line, adjust the right boundary
			right = mid - 1;
			continue;
		}

		// Remove newline
		word[strcspn(word, "\n")] = 0;

		// Compare with user word
		int compare = strcmp(word, userWord);

		if (compare == 0) {
			return 1;  // Word found!
		}

		// Track current position for accurate boundary updates
		long currentPos = ftell(pWordBase);

		if (compare < 0) {
			// Target word comes after current word
			left = currentPos;
		}
		else {
			// Target word comes before current word
			right = mid - 1;
		}

		// Handle case where boundaries are too close
		if (left > right || (right - left) <= 1) {
			// One more explicit check at the current boundaries
			fseek(pWordBase, left, SEEK_SET);
			if (fgets(word, sizeof(word), pWordBase) != NULL) {
				word[strcspn(word, "\n")] = 0;
				if (strcmp(word, userWord) == 0) {
					return 1;
				}
			}
			return 0;  // Word not found
		}
	}

	return 0;  // Word not found
}

char getVowel() // function to get a random vowel
{
	char vowels[] = "AEIOU";
	return vowels[rand() % 5];
}

void getLetters() // function to get 2 random letters (Revised)
{
	char letters[3]; // Local variable
	const char* impossiblePairs[] = {
		"BQ", "BX", "CF", "CG", "CJ", "CV", "CX", "DX", "FQ", "FX",
		"FZ", "GQ", "GX", "HX", "JB", "JC", "JF", "JG", "JM", "JQ",
		"JV", "JX", "JZ", "KQ", "KX", "KZ", "MX", "PQ", "PX", "QB",
		"QC", "QD", "QG", "QH", "QJ", "QK", "QL", "QM", "QN", "QP",
		"QQ", "QR", "QT", "QV", "QX", "QY", "QZ", "SX", "TX", "VB",
		"VC", "VF", "VJ", "VM", "VP", "VW", "VX", "WQ", "WX", "XJ",
		"XX", "XZ", "ZF", "ZR", "ZX", "JP", "XK"
	};
	int impossiblePairsLen = sizeof(impossiblePairs) / sizeof(impossiblePairs[0]);

	while (1) // Keep trying until a valid pair is generated
	{
		// Generate two letters
		for (int i = 0; i < 2; i++) {
			letters[i] = (randomPercent(20)) ? getVowel() : 'A' + rand() % 26;
		}
		letters[2] = '\0'; // Null-terminate

		// Handle Q case
		if (letters[0] == 'Q')
		{
			letters[1] = 'U';
			strcpy(game.current_prompt, letters); //copy the word to the prompt
			return; // exit the func
		}

		// Check if the generated pair is valid
		int is_invalid = 0; //bool var
		for (int i = 0; i < impossiblePairsLen; i++) {
			if ((strcmp(letters, impossiblePairs[i]) == 0) || strcmp(letters, game.last_prompt) == 0) {
				is_invalid = 1; //mark as invalid
				break;
			}
		}

		// If the pair is valid, we return. if its invalid, we do the loop again.
		if (!is_invalid) {
			strcpy(game.current_prompt, letters); // Copy the valid pair
			return; // Success - exit the function
		}


	}

}

int validateInput() // validates the user input, based on the game rules. return true if valid, false if not
{
	// פתיחת הקובץ
	FILE* pWordBase = fopen("wordBase.txt", "r");
	FILE* pUsedWords = fopen("used_words.txt", "r");
	if (pWordBase == NULL || pUsedWords == NULL)
	{
		strcpy(game.error_message, "Error opening word base!");
		strcpy(game.error_color, RED);
		return 0;
	}

	// הפיכת כל האותיות של הקלט לגדולות בשביל להשוות אותם לאותיות בקובץ 
	for (int i = 0; game.input_buffer[i]; i++) {
		game.input_buffer[i] = toupper(game.input_buffer[i]);
	}

	// בדיקה באמצעות חיפוש בינארי
	if (!binarySearchWord(pWordBase, game.input_buffer))
	{
		strcpy(game.error_message, "Word does not exist!");
		strcpy(game.error_color, PASTEL_RED);
		fclose(pWordBase);
		return 0;
	}
	if (binarySearchWord(pUsedWords, game.input_buffer))
	{
		strcpy(game.error_message, "Word already used!");
		strcpy(game.error_color, PASTEL_RED);
		fclose(pUsedWords);
		return 0;
	}
	fclose(pUsedWords);
	fclose(pWordBase); // סגירת הקובץ
	return 1;
}

int getRandomLength()
{
	return 3 + rand() % 4;
}

int getWordLength(char* word)
{
	int length = 0;
	while (word[length] != '\0')
	{
		length++;
	}
	return length;
}

int validateInputMatchMode() // validates the user input for Match Mode, based on the game rules. return true if valid, false if not
{
	int wordLength = getWordLength(game.input_buffer);

	// פתיחת הקובץ
	FILE* pWordBase = fopen("wordBase.txt", "r");
	FILE* pUsedWords = fopen("used_words.txt", "r");
	if (pWordBase == NULL || pUsedWords == NULL)
	{
		strcpy(game.error_message, "Error opening word base!");
		strcpy(game.error_color, RED);
		return 0;
	}

	// הפיכת כל האותיות של הקלט לגדולות בשביל להשוות אותם לאותיות בקובץ 
	for (int i = 0; game.input_buffer[i]; i++) {
		game.input_buffer[i] = toupper(game.input_buffer[i]);
	}

	// בדיקה באמצעות חיפוש בינארי
	if (!binarySearchWord(pWordBase, game.input_buffer))
	{
		strcpy(game.error_message, "Word does not exist!");
		strcpy(game.error_color, PASTEL_RED);
		fclose(pWordBase);
		return 0;
	}
	if (binarySearchWord(pUsedWords, game.input_buffer))
	{
		strcpy(game.error_message, "Word already used!");
		strcpy(game.error_color, PASTEL_RED);
		fclose(pUsedWords);
		return 0;
	}
	if (wordLength < game.randomLength)
	{
		strcpy(game.error_message, "Word is too short!");
		strcpy(game.error_color, PASTEL_RED);
		fclose(pUsedWords);
		fclose(pWordBase); // סגירת הקובץ
		return 0;
	}
	else if (wordLength > game.randomLength)
	{
		strcpy(game.error_message, "Word is too long!");
		strcpy(game.error_color, PASTEL_RED);
		fclose(pUsedWords);
		fclose(pWordBase); // סגירת הקובץ
		return 0;
	}
	fclose(pUsedWords);
	fclose(pWordBase); // סגירת הקובץ
	return 1;
}

int containLetters() // checks if the user input contains the prompt. return true if it does, false if not
{
	if (strstr(game.input_buffer, game.current_prompt))
		return 1;
	strcpy(game.error_message, "Word does not contain the prompt!");
	strcpy(game.error_color, PASTEL_RED);
	return 0;
}

void printLogo() // prints the game logo
{
	printf(PASTEL_YELLOW);
	printf(" ___       __   ________  ________  ________     \n");
	printf("|\\  \\     |\\  \\|\\   __  \\|\\   __  \\|\\   ___ \\    \n");
	printf("\\ \\  \\    \\ \\  \\ \\  \\|\\  \\ \\  \\|\\  \\ \\  \\\\ \\ \\ \n");
	printf(" \\ \\  \\  __\\ \\  \\ \\  \\\\\\  \\ \\   _  _\\ \\  \\ \\\\ \\  \n");
	printf("  \\ \\  \\|\\__\\_\\  \\ \\  \\\\\\  \\ \\  \\\\  \\\\ \\  \\_\\\\ \\ \n");
	printf("   \\ \\____________\\ \\_______\\ \\__\\\\ _\\\\ \\_______\\\n");
	printf("    \\|____________|\\|_______|\\|__|\\|__|\\|_______|\n");
	printf(PASTEL_GREEN                                      "\n");
	printf("	By: Ohad Bragin & Bar Mishaelof               \n");
	printf("                                                 \n");
	printf(RED);
	printf(" ________  ________  _____ ______   ________     \n");
	printf("|\\   __  \\|\\   __  \\|\\   _ \\  _   \\|\\   __  \\    \n");
	printf("\\ \\  \\\\ /\\ \\  \\|\\  \\ \\  \\\\\\__\\ \\  \\ \\  \\|\\ /_   \n");
	printf(" \\ \\   __  \\ \\  \\\\\\  \\ \\  \\\\|__| \\  \\ \\   __  \\  \n");
	printf("  \\ \\  \\|\\  \\ \\  \\\\\\  \\ \\  \\    \\ \\  \\ \\  \\|\\  \\ \n");
	printf("   \\ \\_______\\ \\_______\\ \\__\\    \\ \\__\\ \\_______\\\n");
	printf("    \\|_______|\\|_______|\\|__|     \\|__|\\|_______|\n");
	printf(RESET);
	fflush(stdout);

}

void printLogoWithTick() // prints the logo with a tick effect
{
	printLogo();
	fflush(stdout);
	Sleep(WAIT_TIME);

	int sleep = 400; // Start even slower for visibility
	for (int i = 0; i < 20; i++) // More ticks for better buildup
	{
		system("cls"); // Clear screen
		Sleep(sleep / 2); // Small pause while empty (makes blinking noticeable)

		printLogo();
		fflush(stdout);
		Sleep(sleep / 2); // Logo stays visible

		// Gradual acceleration (slower decrement)
		if (sleep > 100)
			sleep -= 20;
		else if (sleep > 50)
			sleep -= 10;
		else
			sleep -= 5;
	}

	// Explosion effect
	system("cls");
	Sleep(700); // Slight pause for suspense
	printf(RED);
	printf(" ____   ___   ___  __  __   _ \n");
	printf("| __ ) / _ \\ / _ \\|  \\/  | | |\n");
	printf("|  _ \\| | | | | | | |\\/| | | |\n");
	printf("| |_) | |_| | |_| | |  | | |_|\n");
	printf("|____/ \\___/ \\___/|_|  |_| (_)\n");
	printf(RESET);
	Sleep(1500);
	system("cls");
}

void writeCharToFile(char* word, char* filePath) // writes a word to a file
{
	// Open the file in the specified mode
	FILE* pF = fopen(filePath, "a");
	if (pF == NULL)
	{
		strcpy(game.error_message, "Error opening file!");
		strcpy(game.error_color, RED);
		return;
	}
	fprintf(pF, "%s\n", word); // Write the word to the file
	fclose(pF); // Close the file

}

void writeIntToFile(int value, char* filePath) // writes an int to a file
{
	// Open the file in the specified mode
	FILE* pF = fopen(filePath, "a"); //append mode
	if (pF == NULL)
	{
		strcpy(game.error_message, "Error opening file!");
		strcpy(game.error_color, RED);
		return;
	}
	fprintf(pF, "%d\n", value);
	fclose(pF);
}

void resetFile(char* filePath)
{
	FILE* pF = fopen(filePath, "w");
	if (pF == NULL) {
		strcpy(game.error_message, "Error opening file!");
		strcpy(game.error_color, RED);
		return;
	}
	fclose(pF);

	// Handle specific files
	if (strcmp(filePath, "highscores.bin") == 0)
	{
		HighScore_t tempScores = { 0, 0, 0, 0 }; // Zero all high scores
		saveHighScores(&tempScores); // Write zeroed scores to file
		highScores = tempScores; // Also update in-memory copy
	}
	if (strcmp(filePath, "avgtime.bin") == 0)
	{
		AvgTime_t tempAvgTime = { 0, 0, 0, 0, 0, 0, 0, 0}; // Zero all average times
		saveAvgTime(&tempAvgTime); // Write zeroed times to file
		avgTime = tempAvgTime; // Also update in-memory copy
	}
	if (strcmp(filePath, "longestcombo.bin") == 0)
	{
		LongestCombo_t tempCombo = { 0, 0, 0, 0 }; // Zero all combo stats
		saveLongestCombo(&tempCombo); // Write zeroed combos to file
		longestCombo = tempCombo; // Also update in-memory copy
	}
}

int handleInput() // handles the user input
{
	int valid;
	char ch = _getch();
	if (ch == 27) //esc - exit the game
	{
		memset(game.current_prompt, 0, sizeof(game.current_prompt));
		memset(game.input_buffer, 0, sizeof(game.input_buffer));
		memset(game.error_message, 0, sizeof(game.error_message));
		return -2; // Return -2 to indicate exit
	}
	if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
	{
		ch = toupper(ch);
		game.input_buffer[game.input_index] = ch;
		game.input_index++;
	}
	if (ch == 8) //backspace
	{
		if (game.input_index > 0)
		{
			game.input_index--;
		}
		game.input_buffer[game.input_index] = '\0';
	}

	if (ch == '!') // skip a word
	{
		if (game.can_skip)
		{
			strcpy(game.error_message, "Skipped word!");
			strcpy(game.error_color, PASTEL_GREEN);
			game.input_buffer[0] = '\0';
			game.input_index = 0;
			if (strcmp(game.gamemode, "zen") == 0) //no skip penalty in zen mode
				return -1;

			game.score -= BASE_POINTS;
			game.added_score = -1 * BASE_POINTS;
			return -1; // Return -1 to indicate a skip
		}
		strcpy(game.error_color, RED);
		strcpy(game.error_message, "You cant skip in this gamemode!");

	}

	if (ch == 13) //enter
	{
		game.input_buffer[game.input_index] = '\0';
		if (strcmp(game.gamemode, "match") == 0)
		{
			valid = validateInputMatchMode();
		}
		else valid = validateInput();

		if (valid)
		{
			if (containLetters())
			{
				writeCharToFile(game.input_buffer, "used_words.txt");
				strcpy(game.error_message, "Good job!");
				strcpy(game.error_color, PASTEL_GREEN);
				game.input_index = 0;
				return 1; // Return 1 if valid input
			}

		}

	}
	return 0; // Return 0 if no valid input
}

void hideCursor()  // hides the cursor
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursorInfo;

	GetConsoleCursorInfo(hConsole, &cursorInfo); // מקבל מידע על הסמן הנוכחי
	cursorInfo.bVisible = FALSE; // מסתיר את הסמן
	SetConsoleCursorInfo(hConsole, &cursorInfo); // מגדיר מחדש את המידע
}

void drawFrame() // draws the game frame
{
	printf(PASTEL_YELLOW);
	printf("+---------------------------------------+\n");
	printf("|        WORD   BOMB                    |\n");
	printf("+---------------------------------------+\n");
	printf(RESET);
	if (game.timeLimit)
	{
		printf("|  ");
		printf("Time left: ");
		if (game.time_left > 8) //משנה את הצבע של הטיימר כל שיש פחו זמן
			printf(PASTEL_GREEN);
		else if (game.time_left > 4)
			printf(PASTEL_YELLOW);
		else
			printf(RED);
		printf("%-2.2d seconds                ", game.time_left);
		printf(RESET);
		printf("|\n");
	}
	printf("|  Press");
	printf(PASTEL_BLUE);
	printf(" 'ESC' ");
	printf(RESET);
	printf("to exit                  ");
	printf(RESET);
	printf("|\n");
	printf("|  ");
	printf(PASTEL_YELLOW);
	printf("Current Prompt: %s                   ", game.current_prompt);
	printf(RESET);
	printf("|\n");
	if (strcmp(game.gamemode, "match") == 0)
	{
		printf("|");
		printf(PASTEL_GREEN);
		printf("  Word lenth: %d", game.randomLength);
		printf(RESET);
		printf("                        |\n");
	}
	printf("|---------------------------------------|\n");
	printf("|  Player: Enter a word...");
	if (game.can_skip)
	{
		printf(PASTEL_GREEN);
		printf("  '!' to skip");
		printf(RESET);
	}
	else
	{
		printf("             ");
	}
	printf(" |\n");
	printf("|  > _%-32.32s  |\n", game.input_buffer);
	printf("|  SCORE: ");
	if (game.score < 0)
	{
		printf(RED);
		printf("%-6d", game.score);
	}
	else
	{
		printf("%-6d", game.score);
		printf(PASTEL_GREEN);
	}
	if (game.added_score >= 0)
	{
		printf(PASTEL_GREEN);
		printf("+%-5d", game.added_score);
	}
	else if (game.added_score < 0)
	{
		printf(RED);
		printf("%-6d", game.added_score);
	}
	if (game.combo > 2)
	{
		printf(PASTEL_GREEN);
		printf("COMBO x%.1f        ", game.score_multiplier);
		printf(RESET);
		printf("|\n");
	}
	else
	{
		printf(RESET);
		printf("                  |\n");
	}
	printf(RESET);
	printf("+---------------------------------------+\n");

	printf("|  ");
	printf(game.error_color);
	printf("%-37.37s", game.error_message);
	printf(RESET);
	printf("|\n");
	if (game.isLives)
	{
		printf("|  Lives: %d ", game.lives);
		if (strcmp(game.error_message, "BOOM!") == 0)
		{
			printf(RED);
			printf("-1");
			printf(RESET);
		}
		else
			printf("  ");
		printf("                          |\n");
		
	}
	printf(PASTEL_YELLOW);
	printf("+---------------------------------------+");
}

void drawMenu() {
	printf(CLEAR_CONSOLE);
	printf(PASTEL_YELLOW);
	printf("+---------------------------------------+\n");
	printf("| ");
	printf("             WORD   BOMB");
	printf("              |\n");
	printf("+---------------------------------------+\n");
	printf(RESET);
	printf("| ");
	printf(PASTEL_MAGENTA);
	printf("             MAIN MENU");
	printf(RESET);
	printf("                |\n");
	printf("|---------------------------------------|\n");
	printf("| ");
	printf(PASTEL_CYAN);
	printf(" 1. Rules");
	printf(RESET);
	printf("                             |\n");
	printf("| ");
	printf(PASTEL_GREEN);
	printf(" 2. Play");
	printf(RESET);
	printf("                              |\n");
	printf("| ");
	printf(PASTEL_YELLOW);
	printf(" 3. Statistics ");
	printf(RESET);
	printf("                       |\n");
	printf("| ");
	printf(PASTEL_BLUE);
	printf(" 4. Reset Data");
	printf(RESET);
	printf("                        |\n");
	printf("| ");
	printf(PASTEL_RED);
	printf(" 5. Exit");
	printf(RESET);
	printf("                              |\n");
	printf("|                                       |\n");
	printf("| ");
	printf("                                      |\n");
	printf("|---------------------------------------|\n");
	printf("| ");
	printf(PASTEL_MAGENTA);
	printf(" Choose an option:");
	printf(RESET);
	printf("                    |\n");
	printf("| ");
	printf(" >_                                   |\n");
	printf(PASTEL_YELLOW);
	printf("+---------------------------------------+\n");
}

void drawGameModeMenu() // draws the game mode selection menu
{
	printf(CLEAR_CONSOLE);
	printf(PASTEL_YELLOW);
	printf("+---------------------------------------+\n");
	printf("| ");
	printf("             WORD   BOMB");
	printf("              |\n");
	printf("+---------------------------------------+\n");
	printf(RESET);
	printf("| ");
	printf(PASTEL_MAGENTA);
	printf("         SELECT GAME MODE");
	printf(RESET);
	printf("             |\n");
	printf("|---------------------------------------|\n");
	printf("| ");
	printf(PASTEL_CYAN);
	printf(" 1. Survival Mode");
	printf(RESET);
	printf("                     |\n");
	printf("| ");
	printf(PASTEL_GREEN);
	printf(" 2. Speed Mode");
	printf(RESET);
	printf("                        |\n");
	printf("| ");
	printf(PASTEL_YELLOW);
	printf(" 3. Zen Mode");
	printf(RESET);
	printf("                          |\n");
	printf("| ");
	printf(PASTEL_BLUE);
	printf(" 4. Match Mode");
	printf(RESET);
	printf("                        |\n");
	printf("| ");
	printf(PASTEL_RED);
	printf(" 5. Back to Main Menu");
	printf(RESET);
	printf("                 |\n");
	printf("|                                       |\n");
	printf("| ");
	printf(PASTEL_MAGENTA);
	printf(" Choose a game mode:");
	printf(RESET);
	printf("                  |\n");
	printf("| ");
	printf(" >_                                   |\n");
	printf(PASTEL_YELLOW);
	printf("+---------------------------------------+\n");
}

void printBasicRules() // prints the game rules
{
	printf(CLEAR_CONSOLE);
	printf(PASTEL_YELLOW);
	printf("+-----------------------------------------+\n");
	printf("|                WORD BOMB                |\n");
	printf("+-----------------------------------------+\n");
	printf(RESET);
	printf("|");
	printf(PASTEL_MAGENTA);
	printf("                GAME RULES ");
	// Set regular text color
	printf(RESET);
	printf("              | \n");
	printf("|-----------------------------------------|\n");
	printf("| How to play:                            |\n");
	printf("|                                         |\n");

	// Rule 1
	printf("| ");
	printf(PASTEL_CYAN);
	printf("1. ");
	printf(RESET);
	printf("A two-letter prompt will appear      |\n");
	printf("|    (e.g., \"TR\").                        |\n");

	// Rule 2
	printf("| ");
	printf(PASTEL_CYAN);
	printf("2. ");
	printf(RESET);
	printf("Type a word that CONTAINS the        |\n");
	printf("|    given prompt.                        |\n");
	printf("|    ");
	printf(PASTEL_GREEN);
	printf("Example:");
	printf(RESET);
	printf(" MA");
	printf(PASTEL_GREEN);
	printf("TR");
	printf(RESET);
	printf("IX                      |\n");
	// Rule 3
	printf("| ");
	printf(PASTEL_CYAN);
	printf("3. ");
	printf(RESET);
	printf("Press Enter to submit your word.     |\n");
	printf("|                                         |\n");
	// Rule 4
	printf("| ");
	printf(PASTEL_CYAN);
	printf("4. ");
	printf(RESET);
	printf("If valid, you'll earn points and     |\n");
	printf("|    get a new prompt.                    |\n");

	// Rule 5
	printf("| ");
	printf(PASTEL_CYAN);
	printf("5. ");
	printf(RESET);
	printf("If the time runs out: ");
	printf(RED);
	printf("BOOM!");
	printf(RESET);
	printf("	  |\n");
	printf("|    Game over!                           |\n");

	printf("|                                         |\n");

	// Valid words section
	printf("| ");
	printf(PASTEL_GREEN);
	printf("Valid words must:                       ");
	printf(RESET);
	printf("|\n");
	printf("| - Contain the current prompt            |\n");
	printf("| - Be real English words                 |\n");
	printf("| - Not used before                       |\n");
	printf("|                                         |\n");
	printf("| ");
	printf(PASTEL_RED);
	printf("Press any key to return to menu...      ");
	printf(RESET);
	printf("|\n");
	printf(PASTEL_YELLOW);
	printf("+-----------------------------------------+\n");


	printf(RESET);
	_getch();
}

void printSurvivalRules()
{
	printf(CLEAR_CONSOLE);
	printf(PASTEL_YELLOW);
	printf("+-----------------------------------------+\n");
	printf("|             WORD BOMB                   |\n");
	printf("+-----------------------------------------+\n");
	printf(RESET);
	printf("| ");
	printf(PASTEL_MAGENTA);
	printf("           SURVIVAL MODE RULES          ");
	printf(RESET);
	printf("|\n");
	printf("|-----------------------------------------|\n");
	printf("| - You have ");
	printf(PASTEL_RED);
	printf("3 lives");
	printf(RESET);
	printf(".                     |\n");
	printf("| - ");
	printf(PASTEL_CYAN);
	printf("12 seconds");
	printf(RESET);
	printf(" to answer each prompt.     |\n");
	printf("| - Answer correctly to reset the timer.  |\n");
	printf("| - ");
	printf(PASTEL_RED);
	printf("No skipping");
	printf(RESET);
	printf(" allowed.                  |\n");
	printf("| - Lose a life if time runs out.         |\n");
	printf("| - Game ends when you reach 0 lives.     |\n");
	printf("| - Earn more points the faster you type. |\n");
	printf("|                                         |\n");
	printf("| ");
	printf(PASTEL_RED);
	printf("Press any key to return to continue...  ");
	printf(RESET);
	printf("|\n");

	printf(PASTEL_YELLOW);
	printf("+-----------------------------------------+\n");
	printf(RESET);
	_getch();
}

void printSpeedRules()
{
	printf(CLEAR_CONSOLE);
	printf(PASTEL_YELLOW);
	printf("+--------------------------------------------+\n");
	printf("|                  WORD   BOMB               |\n");
	printf("+--------------------------------------------+\n");
	printf(RESET);
	printf("| ");
	printf(PASTEL_MAGENTA);
	printf("              SPEED MODE RULES ");
	printf(RESET);
	printf("            |\n");
	printf("|--------------------------------------------|\n");
	printf("| - You have ");
	printf(PASTEL_CYAN);
	printf("60 seconds");
	printf(RESET);
	printf(" total.               |\n");
	printf("| - Answer as many prompts as possible.      |\n");
	printf("| - Timer does ");
	printf(PASTEL_RED);
	printf("NOT reset");
	printf(RESET);
	printf(" on correct answers. |\n");
	printf("| - ");
	printf(PASTEL_GREEN);
	printf("Skip");
	printf(RESET);
	printf(" is allowed, but costs points.       |\n");
	printf("| - Game ends when time runs out.            |\n");
	printf("| - Points scale with answer speed.          |\n");
	printf("|                                            |\n");
	printf("| ");
	printf(PASTEL_RED);
	printf("Press any key to return to continue...     ");
	printf(RESET);
	printf("|\n");

	printf(PASTEL_YELLOW);
	printf("+--------------------------------------------+\n");
	printf(RESET);
	_getch();
}

void printZenRules()
{
	printf(CLEAR_CONSOLE);
	printf(PASTEL_YELLOW);
	printf("+----------------------------------------------+\n");
	printf("|             WORD BOMB RULES                  |\n");
	printf("+----------------------------------------------+\n");

	printf(PASTEL_MAGENTA);
	printf("|              ZEN MODE RULES                  |\n");
	printf(RESET);
	printf("|                                              |\n");
	printf("| - ");
	printf(PASTEL_GREEN);
	printf("No time limit");
	printf(RESET);
	printf(". Practice at your own pace!  |\n");
	printf("| - ");
	printf(PASTEL_GREEN);
	printf("No penalty");
	printf(RESET);
	printf(" for skipping prompts.           |\n");
	printf("| - Perfect for learning and relaxing.         |\n");
	printf("| - Points are still awarded for answers.      |\n");
	printf("| - Build combos without stress!               |\n");
	printf("|                                              |\n");
	printf("| ");
	printf(PASTEL_RED);
	printf("Press any key to return to continue...       ");
	printf(RESET);
	printf("|\n");

	printf(PASTEL_YELLOW);
	printf("+----------------------------------------------+\n");
	printf(RESET);
	_getch();
}

void printMatchRules()
{
	printf(CLEAR_CONSOLE);
	printf(PASTEL_YELLOW);
	printf("+-----------------------------------------+\n");
	printf("|             WORD BOMB                   |\n");
	printf("+-----------------------------------------+\n");
	printf(RESET);
	printf("| ");
	printf(PASTEL_MAGENTA);
	printf("           MATCH MODE RULES             ");
	printf(RESET);
	printf("|\n");
	printf("|-----------------------------------------|\n");
	printf("| - You have ");
	printf(PASTEL_RED);
	printf("5 lives");
	printf(RESET);
	printf(".                     |\n");
	printf("| - ");
	printf(PASTEL_CYAN);
	printf("20 seconds");
	printf(RESET);
	printf(" to answer each prompt.     |\n");
	printf("| - Word must be ");
	printf(PASTEL_GREEN);
	printf("exact length");
	printf(RESET);
	printf(" shown.      |\n");
	printf("| - Answer correctly to reset the timer.  |\n");
	printf("| - Lose a life if time runs out.         |\n");
	printf("| - Game ends when you reach 0 lives.     |\n");
	printf("| - Earn more points for faster answers.  |\n");
	printf("|                                         |\n");
	printf("| ");
	printf(PASTEL_RED);
	printf("Press any key to continue...            ");
	printf(RESET);
	printf("|\n");
	printf(PASTEL_YELLOW);
	printf("+-----------------------------------------+\n");
	printf(RESET);
	_getch();
}

void printSurvivalStats()
{
	printf(CLEAR_CONSOLE);
	printf(PASTEL_YELLOW);
	printf("+-----------------------------------------+\n");
	printf("|           SURVIVAL MODE STATS           |\n");
	printf("+-----------------------------------------+\n");

	printf(RESET);
	printf("|                                         |\n");
	printf("| ");
	printf(PASTEL_GREEN);
	printf("Highest Score: %-6d", highScores.survival);
	printf(RESET);
	printf("                   |\n");
	double averageTime = 0.0;
	if (avgTime.survivalCorrectCounter > 0)
	{
		averageTime = (double)avgTime.survivalTimeCounter / avgTime.survivalCorrectCounter;
		printf("| ");
		printf(PASTEL_CYAN);
		printf("Average Answer Time: ");
		printf(RESET);
		printf("%5.2f sec          |\n", averageTime);
	}
	else
	{
		printf("| ");
		printf(PASTEL_CYAN);
		printf("Average Answer Time: ");
		printf(RESET);
		printf("N/A                |\n");
	}
	printf("| ");
	printf(PASTEL_MAGENTA);
	printf("Longest combo: ");
	printf(RESET);
	printf("%-3d", longestCombo.survival);
	printf(RESET);
	printf("                      |\n");
	printf("|                                         |\n");
	printf("| ");
	printf(PASTEL_RED);
	printf("Press any key to return to menu...      ");
	printf(RESET);
	printf("|\n");
	printf(PASTEL_YELLOW);
	printf("+-----------------------------------------+\n");
	printf(RESET);
	_getch();
}

void printSpeedStats()
{
	printf(CLEAR_CONSOLE);
	printf(PASTEL_YELLOW);
	printf("+-----------------------------------------+\n");
	printf("|            SPEED MODE STATS             |\n");
	printf("+-----------------------------------------+\n");

	printf(RESET);
	printf("|                                         |\n");
	printf("| ");
	printf(PASTEL_GREEN);
	printf("Highest Score: ");
	printf(RESET);
	printf("%-25d|\n", highScores.speed);

	double averageTime = 0.0;
	if (avgTime.speedCorrectCounter > 0)
	{
		averageTime = (double)avgTime.speedTimeCounter / avgTime.speedCorrectCounter;
		printf("| ");
		printf(PASTEL_CYAN);
		printf("Average Answer Time: ");
		printf(RESET);
		printf("%5.2f sec          |\n", averageTime);
	}
	else
	{
		printf("| ");
		printf(PASTEL_CYAN);
		printf("Average Answer Time: ");
		printf(RESET);
		printf("N/A                |\n");
	}
	printf("| ");
	printf(PASTEL_MAGENTA);
	printf("Longest combo: ");
	printf(RESET);
	printf("%-3d", longestCombo.speed);
	printf(RESET);
	printf("                      |\n");
	printf("|                                         |\n");
	printf("| ");
	printf(PASTEL_RED);
	printf("Press any key to return to menu...      ");
	printf(RESET);
	printf("|\n");
	printf(PASTEL_YELLOW);
	printf("+-----------------------------------------+\n");
	printf(RESET);
	_getch();
}

void printZenStats()
{
	printf(CLEAR_CONSOLE);
	printf(PASTEL_YELLOW);
	printf("+-----------------------------------------+\n");
	printf("|             ZEN MODE STATS              |\n");
	printf("+-----------------------------------------+\n");

	printf(RESET);
	printf("|                                         |\n");
	printf("| ");
	printf(PASTEL_GREEN);
	printf("Highest Score: ");
	printf(RESET);
	printf("%-25d|\n", highScores.zen);
	double averageTime = 0.0;
	if (avgTime.zenCorrectCounter > 0)
	{
		averageTime = (double)avgTime.zenTimeCounter / avgTime.zenCorrectCounter;
		printf("| ");
		printf(PASTEL_CYAN);
		printf("Average Answer Time: ");
		printf(RESET);
		printf("%5.2f sec          |\n", averageTime);
	}
	else
	{
		printf("| ");
		printf(PASTEL_CYAN);
		printf("Average Answer Time: ");
		printf(RESET);
		printf("N/A                |\n");
	}
	printf("| ");
	printf(PASTEL_MAGENTA);
	printf("Longest combo: ");
	printf(RESET);
	printf("%-3d", longestCombo.zen);
	printf(RESET);
	printf("                      |\n");
	printf("|                                         |\n");
	printf("| ");
	printf(PASTEL_RED);
	printf("Press any key to return to menu...      ");
	printf(RESET);
	printf("|\n");
	printf(PASTEL_YELLOW);
	printf("+-----------------------------------------+\n");
	printf(RESET);
	_getch();
}

void printMatchStats()
{
	printf(CLEAR_CONSOLE);
	printf(PASTEL_YELLOW);
	printf("+-----------------------------------------+\n");
	printf("|            MATCH MODE STATS             |\n");
	printf("+-----------------------------------------+\n");

	printf(RESET);
	printf("|                                         |\n");
	printf("| ");
	printf(PASTEL_GREEN);
	printf("Highest Score: ");
	printf(RESET);
	printf("%-25d|\n", highScores.match);

	double averageTime = 0.0;
	if (avgTime.matchCorrectCounter > 0)
	{
		averageTime = (double)avgTime.matchTimeCounter / avgTime.matchCorrectCounter;
		printf("| ");
		printf(PASTEL_CYAN);
		printf("Average Answer Time: ");
		printf(RESET);
		printf("%5.2f sec          |\n", averageTime);
	}
	else
	{
		printf("| ");
		printf(PASTEL_CYAN);
		printf("Average Answer Time: ");
		printf(RESET);
		printf("N/A                |\n");
	}
	printf("| ");
	printf(PASTEL_MAGENTA);
	printf("Longest combo: ");
	printf(RESET);
	printf("%-3d", longestCombo.match);
	printf(RESET);
	printf("                      |\n");
	printf("|                                         |\n");
	printf("| ");
	printf(PASTEL_RED);
	printf("Press any key to return to menu...      ");
	printf(RESET);
	printf("|\n");
	printf(PASTEL_YELLOW);
	printf("+-----------------------------------------+\n");
	printf(RESET);
	_getch();
}

void displayHighScores() {
	printf(CLEAR_CONSOLE);
	printf(PASTEL_YELLOW);
	printf("+-----------------------------------------+\n");
	printf("|                WORD BOMB                |\n");
	printf("+-----------------------------------------+\n");
	printf(RESET);
	printf("| ");
	printf(PASTEL_MAGENTA);
	printf("             HIGH SCORES                ");
	printf(RESET);
	printf("|\n");
	printf("|-----------------------------------------|\n");
	printf("| ");
	printf(PASTEL_CYAN);
	printf("Survival Mode: ");
	printf(PASTEL_GREEN);
	printf("%-10d", highScores.survival);
	printf(RESET);
	printf("               |\n");
	printf("| ");
	printf(PASTEL_CYAN);
	printf("Speed Mode:    ");
	printf(PASTEL_GREEN);
	printf("%-10d", highScores.speed);
	printf(RESET);
	printf("               |\n");
	printf("| ");
	printf(PASTEL_CYAN);
	printf("Zen Mode:      ");
	printf(PASTEL_GREEN);
	printf("%-10d", highScores.zen);
	printf(RESET);
	printf("               |\n");
	printf("| ");
	printf(PASTEL_CYAN);
	printf("Match Mode:    ");
	printf(PASTEL_GREEN);
	printf("%-10d", highScores.match);
	printf(RESET);
	printf("               |\n");
	printf("|                                         |\n");
	printf("| ");
	printf(PASTEL_RED);
	printf("Press any key to return to menu...      ");
	printf(RESET);
	printf("|\n");
	printf(PASTEL_YELLOW);
	printf("+-----------------------------------------+\n");
	printf(RESET);
	_getch();
}

void gameOver()
{
	printf(CLEAR_CONSOLE);
	int newHighScore = 0;
	// Update high score for the current game mode
	if (strcmp(game.gamemode, "survival") == 0)
	{
		if (game.score > highScores.survival)
		{
			highScores.survival = game.score;
			newHighScore = 1;
		}
		if (game.combo > longestCombo.survival)
			longestCombo.survival = game.combo;
		avgTime.survivalCorrectCounter += game.correctCounter;
		avgTime.survivalTimeCounter += game.timeCounter;
	}
	else if (strcmp(game.gamemode, "speed") == 0)
	{
		if (game.score > highScores.speed)
		{
			highScores.speed = game.score;
			newHighScore = 1;
		}
		if (game.combo > longestCombo.speed)
			longestCombo.speed = game.combo;
		avgTime.speedCorrectCounter += game.correctCounter;
		avgTime.speedTimeCounter += game.timeCounter;
	}
	else if (strcmp(game.gamemode, "zen") == 0 ) 
	{
		if(game.score > highScores.zen)
		{ 
			highScores.zen = game.score;
			newHighScore = 1;
		}
		if (game.combo > longestCombo.zen)
			longestCombo.zen = game.combo;
		avgTime.zenCorrectCounter += game.correctCounter;
		avgTime.zenTimeCounter += game.timeCounter;
	
	}
	else if (strcmp(game.gamemode, "match") == 0)
	{
		if (game.score > highScores.speed)
		{
			highScores.match = game.score;
			newHighScore = 1;
		}
		if (game.combo > longestCombo.match)
			longestCombo.zen = game.combo;
		avgTime.matchCorrectCounter += game.correctCounter;
		avgTime.matchTimeCounter += game.timeCounter;
	}
	saveHighScores(&highScores); // Save updated high scores
	saveAvgTime(&avgTime); // Save average times
	saveLongestCombo(&longestCombo); // Save longest combo
	printf("Game Over! Your score: %d", game.score);
	if (newHighScore) {
		printf(PASTEL_GREEN);
		printf(" NEW HIGH SCORE!");
		printf(RESET);
	}
	Sleep(WAIT_TIME * 2);
	memset(game.current_prompt, 0, sizeof(game.current_prompt));
	memset(game.input_buffer, 0, sizeof(game.input_buffer));
	memset(game.error_message, 0, sizeof(game.error_message));
}

void handleUserChoice() // handles the user choice from the main menu
{
	switch (game.userChoice) {
	case '1': // Rules
		printBasicRules();
		game.userChoice = -1;
		break;
	case '2': // Play
		drawGameModeMenu();
		game.gamemodeChoice = _getch();
		initialize_game_state();
		switch (game.gamemodeChoice) {
		case '1': // Survival Mode
			strcpy(game.gamemode, "survival");
			printSurvivalRules();
			survivalGameLoop();
			break;
		case '2': // Speed Mode
			strcpy(game.gamemode, "speed");
			printSpeedRules();
			speedGameLoop();
			break;
		case '3': // Zen Mode
			strcpy(game.gamemode, "zen");
			printZenRules();
			zenGameLoop();
			break;
		case '4': // Match Mode
			strcpy(game.gamemode, "match");
			printMatchRules();
			matchGameLoop();
			break;
		}
		game.userChoice = -1;
		break;
	case '3': // Stats
		drawGameModeMenu();
		game.gamemodeChoice = _getch();
		switch (game.gamemodeChoice)
		{
		case '1': // Survival Mode
			printSurvivalStats();
			break;
		case '2': // Speed Mode
			printSpeedStats();
			break;
		case '3': // Zen Mode
			printZenStats();
			break;
		case '4': // Match Mode
			printMatchStats();
			break;
		}
		game.userChoice = -1;
		break;
	case '4': // Reset Data
		printf(CLEAR_CONSOLE);
		printf(PASTEL_YELLOW);
		printf("Resetting data...\n");
		Sleep(WAIT_TIME);
		printf(RESET);
		resetFile("highscores.bin"); // Reset high scores
		resetFile("avgtime.bin"); // Reset average times
		resetFile("longestcombo.bin");; //Reset longest combo
		// Reload the data after reset
		loadHighScores(&highScores);
		loadAvgTimes(&avgTime);
		loadLongestCombo(&longestCombo);
		printf(PASTEL_GREEN);
		printf("Data reset successfully!\n");
		printf(RESET);
		Sleep(WAIT_TIME);
		game.userChoice = -1;
		break;
	case '5': // Exit
		printf(CLEAR_CONSOLE);
		printf(PASTEL_RED);
		printf("Thank you for playing! \n");
		Sleep(1500);
		exit(0);
		break;
	}
}


void handleScore() // handles the game score
{
	int timeBonus = 0;
	int randomBonus;
	if (game.time_to_answer == 0)
	{
		game.time_to_answer = 1;
		timeBonus += BASE_POINTS/2;
	}
	timeBonus += BASE_POINTS / game.time_to_answer;  // More points for faster answers
	randomBonus = rand() % timeBonus;
	game.added_score = (BASE_POINTS + timeBonus + randomBonus) * game.score_multiplier;
	game.score += game.added_score;
}
void handleCombo()
{
	if (game.combo > 2)
	{
		game.score_multiplier += 0.2f;
		return;
	}
}
void generateNewState() // generates a new game state
{
	strcpy(game.last_prompt, game.current_prompt);
	memset(game.current_prompt, 0, sizeof(game.current_prompt));
	memset(game.input_buffer, 0, sizeof(game.input_buffer));
	memset(game.error_message, 0, sizeof(game.error_message));
	getLetters();
	if (strcmp(game.gamemode, "match") == 0)
	{
		game.randomLength = getRandomLength(); // Generate a new random length
	}
}

void survivalGameLoop() // main game loop
{
	int index;
	game.time_per_prompt = 12;
	getLetters(); // Get the first letters
	printf(CLEAR_CONSOLE);
	initialize_time(); // Initialize time
	game.can_skip = 0; // Disable skip in survival mode
	game.timeLimit = 1; // Enable time limit
	game.isLives = 1; //Enable lives
	strcpy(game.gamemode, "survival"); // Set game mode to survival
	while (game.lives > 0) {
		// Update game time
		game.current_time = time(NULL);
		game.time_left = game.time_per_prompt - (game.current_time - game.initial_time);

		// Handle timeout
		if (game.time_left <= 0) {

			if (game.lives <= 0) {
				break; // Exit the loop if no lives left
			}
			//reset combo
			if (game.combo > longestCombo.survival)
				longestCombo.survival = game.combo;
			game.combo = 0;
			game.score_multiplier = 1.0f; // reset combo
			strcpy(game.error_message, "BOOM!");
			strcpy(game.error_color, RED);
			game.lives--;

			// Clear and draw to show the timeout message
			printf(CLEAR_CONSOLE);
			drawFrame();
			Sleep(WAIT_TIME * 2);

			// Reset for next round
			generateNewState();
			initialize_time(); // Reset time
			index = 0;
			memset(game.input_buffer, 0, sizeof(game.input_buffer)); // Clear input
			continue;
		}

		// Draw current frame
		printf(CLEAR_CONSOLE);
		drawFrame();

		// Process input
		if (_kbhit()) {
			int valid = handleInput();
			if (valid == -2) //exit
				break;
			if (valid)
			{
				game.combo++;
				handleCombo();
				game.time_to_answer = game.current_time - game.time_since_answer;
				game.timeCounter += game.time_to_answer;
				game.correctCounter++;
				game.time_since_answer = time(NULL);
				handleScore();
				printf(CLEAR_CONSOLE);
				drawFrame();
				Sleep(WAIT_TIME);
				generateNewState();
				initialize_time(); // Reset time
			}
			
		}

		Sleep(16); // 60 fps
	}
	gameOver();
	Sleep(3000);
}

void speedGameLoop()
{
	int index = 0;
	game.time_per_prompt = 60;
	getLetters(); // Get the first letters
	printf(CLEAR_CONSOLE);
	initialize_time(); // Initialize time
	game.time_left = 60; // Set time for speed mode
	game.can_skip = 1; // Enable skip in speed mode
	game.timeLimit = 1; // Enable time limit
	game.isLives = 0; //Disable lives
	strcpy(game.gamemode, "speed"); // Set game mode to speed
	while (game.time_left > 0)
	{
		game.current_time = time(NULL);
		game.time_left = 60 - (game.current_time - game.initial_time);
		// Draw current frame
		printf(CLEAR_CONSOLE);
		drawFrame();
		if (_kbhit()) {
			int valid = handleInput();
			if (valid == -2) //exit
			{
				gameOver();
				return;
			}	
			if (valid == 1)
			{
				game.combo++;
				handleCombo();
				game.time_to_answer = game.current_time - game.time_since_answer;
				game.timeCounter += game.time_to_answer;
				game.correctCounter++;
				game.time_since_answer = time(NULL);
				printf(CLEAR_CONSOLE);
				handleScore();
				drawFrame();
				Sleep(WAIT_TIME);
				generateNewState();
			}
			else if (valid == -1) // Skip word
			{
				//reset combo
				if (game.combo > longestCombo.speed)
					longestCombo.speed = game.combo;
				game.combo = 0;
				game.score_multiplier = 1.0f;

				game.time_to_answer = game.current_time - game.time_since_answer;
				game.time_since_answer = time(NULL);
				printf(CLEAR_CONSOLE);
				drawFrame();
				Sleep(WAIT_TIME);
				generateNewState();
			}
		}
		Sleep(16); // 60 fps
	}
	strcpy(game.error_message, "BOOM!");
	strcpy(game.error_color, RED);
	// Clear and draw to show the timeout message
	printf(CLEAR_CONSOLE);
	drawFrame();
	Sleep(WAIT_TIME);
	gameOver();
}

void zenGameLoop() //zen gamemode gameloop
{
	int index = 0;
	getLetters(); // Get the first letters
	printf(CLEAR_CONSOLE);
	initialize_time(); // Initialize time
	game.can_skip = 1; // Enable skip in zen mode
	game.timeLimit = 0; // Disable time limit
	game.isLives = 0; //Disable lives
	strcpy(game.gamemode, "zen"); // Set game mode to zen
	while (1)
	{
		game.current_time = time(NULL);
		printf(CLEAR_CONSOLE);
		drawFrame();
		if (_kbhit()) {
			int valid = handleInput();
			if (valid == -2) //exit
				break;
			if (valid == 1) 
			{
				game.combo++;
				handleCombo();
				game.time_to_answer = game.current_time - game.time_since_answer;
				game.timeCounter += game.time_to_answer;
				game.correctCounter++;
				game.time_since_answer = time(NULL);
				handleScore();
				printf(CLEAR_CONSOLE);
				drawFrame();
				Sleep(WAIT_TIME);
				generateNewState();
			}
			else if (valid == -1) // Skip word
			{
				//reset combo
				if (game.combo > longestCombo.zen)
					longestCombo.zen = game.combo;
				game.combo = 0;
				game.score_multiplier = 1.0f;
				game.time_since_answer = time(NULL);
				printf(CLEAR_CONSOLE);
				drawFrame();
				Sleep(WAIT_TIME);
				generateNewState();
			}
		}
		Sleep(16); // 60 fps
	}
	gameOver();
}

void matchGameLoop() // main game loop
{
	int index = 0;
	game.lives = 5; // Set five lives for match mode
	game.time_per_prompt = 20;
	getLetters(); // Get the first letters
	game.randomLength = getRandomLength(); // Initialize random length
	printf(CLEAR_CONSOLE);
	initialize_time(); // Initialize time
	game.can_skip = 0; // Disable skip in match mode
	game.timeLimit = 1; // Enable time limit
	game.isLives = 1; //Enable lives
	strcpy(game.gamemode, "match"); // Set game mode to match
	while (game.lives > 0) {
		// Update game time
		game.current_time = time(NULL);
		game.time_left = game.time_per_prompt - (game.current_time - game.initial_time);

		// Handle timeout
		if (game.time_left <= 0) {
			if (game.lives <= 0) {
				break; // Exit the loop if no lives left
			}
			strcpy(game.error_message, "BOOM!");
			strcpy(game.error_color, RED);
			game.lives--;
			//reset combo
			if (game.combo > longestCombo.match)
				longestCombo.match = game.combo;
			game.combo = 0;
			game.score_multiplier = 1.0f;

			// Clear and draw to show the timeout message
			printf(CLEAR_CONSOLE);
			drawFrame();
			Sleep(WAIT_TIME * 2);

			// Reset for next round
			generateNewState();
			game.randomLength = getRandomLength(); // Set new random length after timeout
			initialize_time(); // Reset time
			index = 0;
			memset(game.input_buffer, 0, sizeof(game.input_buffer)); // Clear input
			continue;
		}

		// Draw current frame
		printf(CLEAR_CONSOLE);
		drawFrame();

		// Process input
		if (_kbhit()) {
			int valid = handleInput();
			if (valid == -2) //exit
				break;
			if (valid == 1) {
				if (strcmp(game.error_message, "Error opening word base!") == 0) {
					printf(CLEAR_CONSOLE);
					drawFrame();
					Sleep(WAIT_TIME * 2);
					gameOver();
					return; // Exit on file error
				}
				game.combo++;
				handleCombo();
				game.time_to_answer = game.current_time - game.time_since_answer;
				game.timeCounter += game.time_to_answer;
				game.correctCounter++;
				game.time_since_answer = time(NULL);
				handleScore();
				printf(CLEAR_CONSOLE);
				drawFrame();
				Sleep(WAIT_TIME);
				generateNewState();
				game.randomLength = getRandomLength(); // Set new random length after valid word
				initialize_time(); // Reset time
			}
			else if (valid == 0 && game.error_message[0] != '\0' && game.input_index == 0) {
				// Show error message for invalid input
				printf(CLEAR_CONSOLE);
				drawFrame();
				Sleep(WAIT_TIME / 2); // Brief pause to show error (500ms)
			}
		}

		Sleep(16); // 60 fps
	}
	gameOver();
	Sleep(3000);
}