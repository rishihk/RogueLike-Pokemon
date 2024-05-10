#include<iostream>
#include "connect4_engine.hpp" // Include the header file here
using namespace std;

// ANSI color codes
const string RED = "\033[91m";     // Bright Red for player 1
const string YELLOW = "\033[93m";  // Bright Yellow for player 2
const string RESET = "\033[0m";    // Reset to default color
const string BLUE = "\033[94m";    // Bright Blue for the board
const string GREEN = "\033[92m";   // Bright Green
const string MAGENTA = "\033[95m"; // Bright Magenta


// The board
char arr[6][7] = {{'.','.','.','.','.','.','.'}, 
                  {'.','.','.','.','.','.','.'},
                  {'.','.','.','.','.','.','.'},
                  {'.','.','.','.','.','.','.'},
                  {'.','.','.','.','.','.','.'},
                  {'.','.','.','.','.','.','.'}};

// Check the places of board
int c1 = 6, c2 = 6, c3 = 6, c4 = 6, c5 = 6, c6 = 6, c7 = 6;

// [Rest of your functions remain unchanged]

// Function to display the board
void board(int a, int b, char x) {
    arr[a][b-1] = x;

    // Display column headers
   cout << MAGENTA << "  1   2   3   4   5   6   7 " << RESET << endl;
   cout << BLUE << "-----------------------------" << RESET << endl;

    // Display the board
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 7; j++) {
            cout << BLUE << "|" << RESET; // Board boundary
            if (arr[i][j] == 'X')
                cout << RED << " X " << RESET;
            else if (arr[i][j] == 'O')
                cout << YELLOW << " O " << RESET;
            else
                cout << GREEN << " . " << RESET;
        }
        cout << BLUE << "|" << RESET << endl; // Right boundary of the board
    }
    cout << BLUE << "-----------------------------" << RESET << endl; // Bottom boundary of the board
}

// Function to display the board
void displayBoard() {
    // Display column headers
    cout << MAGENTA  << "  1   2   3   4   5   6   7 " << RESET << endl;
    cout << BLUE << "-----------------------------" << RESET << endl;

    // Display the board
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 7; j++) {
            cout << BLUE << "|" << RESET; // Board boundary
            // Print 'X' in red, 'O' in yellow, or '.' in default color
            if (arr[i][j] == 'X')
                cout << RED << " X " << RESET;
            else if (arr[i][j] == 'O')
                cout << YELLOW << " O " << RESET;
            else
                cout << GREEN << " . " << RESET;
        }
        cout << BLUE << "|" << RESET << endl; // Right boundary of the board
    }
    cout << BLUE << "-----------------------------" << RESET << endl;
}


//function of place of x or o
int insert(int x)
{
	//cheak values
	if(x==1){
		if (c1>0) {
			c1--;
			return c1;
		}
		else
		return -1;
	}
	else if(x==2){
		if (c2>0){
			c2--;
			return c2;
		}
		else
		return -1;
	}
	else if(x==3){
		if (c3>0){
			c3--;
			return c3;
		}
		else
		return -1;
	} 
	else if(x==4){
		if (c4>0)
		{
			c4--;
			return c4;
		}
		else
		return -1;
	}
	else if(x==5){
		if (c5>0){
			c5--;
			return c5;
		}
		else
		return -1;	
	}
	else if(x==6){	
		if (c6>0){
			c6--;
			return c6;
		}
		else
		return -1;
	}
	else if(x==7){
		if (c7>0){
			c7--;
			return c7;
		}
		else
		return -1;	
	}
	else 
	return -1;
	
}

bool win_p1_row (int a)//1st case to win p1
{
	int count=0;
		for(int j=0; j<7;j++){
			if (arr[a][j]=='X')
			{
				count++;
				if (count==4)
				return true ;
			}
			else
			count =0;
		}
		return false;
		
}

bool win_p1_dai1 (int a , int b)//2nd case to win p1
{
	int count=0;
	b--;
	if (a>=b)
	{
		a-=b;
		b=0;
		while(a<6)
		{
			if (arr[a][b]=='X')
			{
				count++;
				if (count==4)
				return true ;
			}
			else
			count =0;
			a++;
			b++;
		}
	}
	else
	{
		b-=a;
		a=0;
		while(b<7)
		{
			if (arr[a][b]=='X')
			{
				count++;
				if (count==4)
				return true ;
			}
			else
			count =0;
			a++;
			b++;
		}
	}
		return false;
		
}

bool win_p1_dai2 (int a , int b)//3rd case to win p1
{
	int count=0;
	b--;
	int c =a+b;
	if (c<7 && c>2)
	{
		b=c;
		a=0;
		while(a<6)
		{
			if (arr[a][b]=='X')
			{
				count++;
				if (count==4)
				return true ;
			}
			else
			count =0;
			a++;
			b--;
		}
	}
	else if (c>6 && c<9)
	{
		b=6;
		a=c-6;
		while(a<6)
		{
			if (arr[a][b]=='X')
			{
				count++;
				if (count==4)
				return true ;
			}
			else
			count =0;
			a++;
			b--;
		}
	}
		return false;
		
}

bool win_p1_col (int a)//4th case to win p1
{
	int count=0;
	a--;
		for(int j=0; j<6;j++)
		{
			if (arr[j][a]=='X')
			{
				count++;
				if (count==4)
				return true ;
			}
			else
			count =0;
		}
		return false;
		
}

bool win_p2_row (int a)//1st case to win p2
{
	int count=0;
	
		for(int j=0; j<7;j++)
		{
			if (arr[a][j]=='O')
			{
				count++;
				if (count==4)
				return true ;
			}
			else
			count =0;
		}
		return false;
		
}

bool win_p2_col (int a)//2nd case to win p2
{
	int count=0;
	a--;
		for(int j=0; j<6;j++)
		{
			if (arr[j][a]=='O')
			{
				count++;
				if (count==4)
				return true ;
			}
			else
			count =0;
		}
		return false;
		
}

bool win_p2_dai1 (int a , int b)//3rd case to win p2
{
	int count=0;
	b--;
	if (a>=b)
	{
		a-=b;
		b=0;
		while(a<6)
		{
			if (arr[a][b]=='O')
			{
				count++;
				if (count==4)
				return true ;
			}
			else
			count =0;
			a++;
			b++;
		}
	}
	else
	{
		b-=a;
		a=0;
		while(b<7)
		{
			if (arr[a][b]=='O')
			{
				count++;
				if (count==4)
				return true ;
			}
			else
			count =0;
			a++;
			b++;
		}
	}
		return false;
		
}

bool win_p2_dai2 (int a , int b)//4th case to win p2
{
	int count=0;
	b--;
	int c =a+b;
	if (c<7 && c>2)
	{
		b=c;
		a=0;
		while(a<6)
		{
			if (arr[a][b]=='O')
			{
				count++;
				if (count==4)
				return true ;
			}
			else
			count =0;
			a++;
			b--;
		}
	}
	else if (c>6 && c<9)
	{
		b=6;
		a=c-6;
		while(a<6)
		{
			if (arr[a][b]=='O')
			{
				count++;
				if (count==4)
				return true ;
			}
			else
			count =0;
			a++;
			b--;
		}
	}
		return false;
		
}

int main() {
    cout << "\n  Welcome To Connect 4 !!\n\n"; // Welcome message with padding

    displayBoard(); // Initial display of the board

    int c = 0; // Counter for the number of moves made
    while (c < 21) { // Loop to take the moves, maximum of 42 moves (6x7 board)
        int p1 = 0, p2 = 0; // Player move indicators
        int col = 0; // Column chosen by the player
        int k = 0; // Row calculated based on column

        // Player 1's turn
        while (p1 < 1) {
            cout << "Player 1 - Enter column (1 - 7): ";
            col = 0;
            cin >> col;
            cout << "\n";
            k = insert(col); // Calculate the row

            // If the column is full or invalid input
            while (k == -1) {
                cout << "Invalid place, Play Again: ";
                cin >> col;
                cout << "\n";
                k = insert(col);
            }

            board(k, col, 'X'); // Update the board with player 1's move
            p1++; // Indicate player 1 has made a move

            // Check for winning conditions for Player 1
            if (win_p1_row(k) || win_p1_col(col) || win_p1_dai1(k, col) || win_p1_dai2(k, col)) {
                cout << "\nPlayer 1 is the Winner! Congratulations!!\n\n";
				cout << "\n";
                return 0; // End the game as we have a winner
            }
        }

        // Player 2's turn
        while (p2 < 1) {
            cout << "Player 2 - Enter column: (1 - 7): ";
            col = 0;
            cin >> col;
            cout << "\n";
            k = insert(col); // Calculate the row

            // If the column is full or invalid input
            while (k == -1) {
                cout << "Invalid place, Play Again: ";
                cin >> col;
                cout << "\n";
                k = insert(col);
            }

            board(k, col, 'O'); // Update the board with player 2's move
            p2++; // Indicate player 2 has made a move

            // Check for winning conditions for Player 2
            if (win_p2_row(k) || win_p2_col(col) || win_p2_dai1(k, col) || win_p2_dai2(k, col)) {
                cout << "\nPlayer 2 is the Winner! Congratulations!!\n\n";
                cout << "\n";
                return 0; // End the game as we have a winner
            }
        }

        c++; // Increment move counter

        // If all places are taken and no winner, declare a draw
        if (c == 21) {
            cout << "\nSorry! No Winner, it's a draw.\n";
            cout << "\n";
            break;
        }
    }

    return 0; // End of main function
}


