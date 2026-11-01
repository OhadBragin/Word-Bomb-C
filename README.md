# Word Bomb

A fast word game for the Windows console, written entirely in C. You get a two-letter prompt and race the timer to type a real English word that contains it, before the bomb goes off.

By Ohad Bragin & Bar Mishaelof.

```
+---------------------------------------+
|        WORD   BOMB                    |
+---------------------------------------+
|  Time left: 09 seconds                |
|  Press 'ESC' to exit                  |
|  Current Prompt: TR                   |
|---------------------------------------|
|  Player: Enter a word...              |
|  > _MATRIX                            |
```

## How to play

1. A two-letter prompt appears, such as `TR`.
2. Type a word that contains it, such as `MATRIX`, and press Enter.
3. If the word is valid, you score points and get a new prompt.
4. If the time runs out: **BOOM!**

A word counts if it contains the prompt, is in the game's dictionary, and hasn't been used yet in the current game.

## Game modes

| Mode | Time limit | Lives | Skipping | Twist |
|------|-----------|-------|----------|-------|
| **Survival** | 12 s per prompt | 3 | No | A timeout costs a life |
| **Speed** | 60 s in total | None | Yes, costs 300 points | The timer never resets |
| **Zen** | None | None | Yes, free | Practice at your own pace |
| **Match** | 20 s per prompt | 5 | No | The word must be an exact length, from 3 to 6 letters |

## Controls

| Key | Action |
|-----|--------|
| Letters | Type your word |
| `Enter` | Submit |
| `Backspace` | Delete a letter |
| `!` | Skip the prompt (Speed and Zen) |
| `Esc` | End the game |

## Scoring

Each correct word earns a base of 300 points, plus a speed bonus, plus a small random bonus. The faster you answer, the bigger the bonus.

From your third correct answer in a row, a combo starts: the score multiplier grows by 0.2 with each correct answer. Timing out or skipping resets it.

The game saves your high score, average answer time and longest combo for each mode. You can see them under **Statistics** and clear them with **Reset Data**.

## Running the game

The game is Windows only and is built from source with gcc ([MinGW-w64](https://www.mingw-w64.org/)):

```bash
git clone https://github.com/OhadBragin/Word-Bomb-C.git
cd Word-Bomb-C
gcc main.c funcs.c -o WordBomb.exe
./WordBomb.exe
```

Run it from the repo folder, because the game looks for `wordBase.txt` and its save files in the current folder. Use a terminal that supports ANSI colors, such as Windows Terminal.

## How it works

- **Word lookup:** `wordBase.txt` is about 2.7 MB of sorted words. The game checks each answer with a binary search directly on the file, so it never loads the list into memory.
- **Prompts:** letters are random and lean slightly towards vowels. Letter pairs that are hard to find in English, such as `QZ` or `XJ`, are rejected, a `Q` is always followed by `U`, and the same prompt never appears twice in a row.
- **Display:** the screen is redrawn about 60 times a second with ANSI escape codes, while `_kbhit` and `_getch` read keys without blocking the timer.
- **Saves:** statistics are stored as raw structs in small binary files.

## Project layout

| File | Purpose |
|------|---------|
| `main.c` | Entry point and main menu loop |
| `funcs.c` | Game loops, input validation, scoring, drawing and saves |
| `funcs.h` | Game state structs and function declarations |
| `wordBase.txt` | The dictionary |
