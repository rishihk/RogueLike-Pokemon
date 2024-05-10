#ifndef CONNECT4_H
#define CONNECT4_H

#include <string>

// ANSI color codes
extern const std::string RED;     // Bright Red for player 1
extern const std::string YELLOW;  // Bright Yellow for player 2
extern const std::string RESET;   // Reset to default color
extern const std::string BLUE;    // Bright Blue for the board
extern const std::string GREEN;   // Bright Green
extern const std::string MAGENTA; // Bright Magenta

// The board
extern char arr[6][7];

// Check the places of board
extern int c1, c2, c3, c4, c5, c6, c7;

// Function prototypes
void board(int a, int b, char x);
void displayBoard();
int insert(int x);
bool win_p1_row(int a);
bool win_p1_dai1(int a, int b);
bool win_p1_dai2(int a, int b);
bool win_p1_col(int a);
bool win_p2_row(int a);
bool win_p2_col(int a);
bool win_p2_dai1(int a, int b);
bool win_p2_dai2(int a, int b);

#endif // CONNECT4_H
