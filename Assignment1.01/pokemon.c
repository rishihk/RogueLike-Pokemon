#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char terrain[21][80];
int randNS;
int randEW;

// Initializing my terrain with short grass (clearings)
void initializeTerrain()
{
    for (int i = 0; i < 21; i++)
    {
        for (int j = 0; j < 80; j++)
        {
            terrain[i][j] = '.';
        }
    }
}

void createPaths()
{
    // East-West path
    int x = rand() % 80; // Random starting point on North border
    int y = 0;           // Start from the North border

    while (y < 21)
    {
        terrain[y][x] = '#';
        int move = rand() % 2; // 0 = move sideways, 1 = move down

        if (move == 0 && x > 1 && x < 79)
        {
            x += (rand() % 2) ? 1 : -1; // Move east or west
        }
        else
        {
            y += 1; // Move south
        }
    }

    // North-South path
    x = 0;           // Start from the West border
    y = rand() % 21; // Random starting point on West border

    while (x < 80)
    {
        terrain[y][x] = '#';
        int move = rand() % 2; // 0 = move up/down, 1 = move right

        if (move == 0 && y > 1 && y < 20)
        {
            y += (rand() % 2) ? 1 : -1; // Move north or south
        }
        else
        {
            x += 1; // Move east
        }
    }
}

// Creating the borders of the terrain
void createBorders()
{
    for (int i = 0; i < 80; i++)
    {
        if (terrain[0][i] != '#') // North border with exit
        {
            terrain[0][i] = '%';
        }
    }

    for (int i = 0; i < 80; i++)
    {
        if (terrain[20][i] != '#') // South border with exit
        {
            terrain[20][i] = '%';
        }
    }

    for (int i = 0; i < 21; i++)
    {
        if (terrain[i][0] != '#') // West border with exit
        {
            terrain[i][0] = '%';
        }
    }

    for (int i = 0; i < 21; i++)
    {
        if (terrain[i][79] != '#') // East border with exit
        {
            terrain[i][79] = '%';
        }
    }
}

void pokeCenter()
{
}

void pokeMart()
{
}

void tallGrass()
{
}

void water()
{
}

void shortGrass()
{
}

void fillRandTerrain()
{
}

// Print the terrain
void printTerrain()
{
    for (int i = 0; i < 21; i++)
    {
        for (int j = 0; j < 80; j++)
        {
            putchar(terrain[i][j]);
        }
        putchar('\n');
    }
}

int main(int argc, char *argv[])
{
    srand(time(NULL)); // random seed

    // making sure the paths dont fall too much near the borders
    randNS = rand() % 61 + 10;
    randEW = rand() % 11 + 5;

    initializeTerrain(); // initialize the terrain
    createPaths();       // create the paths
    createBorders();     // create the borders
    printTerrain();      // print the terrain

    return 0;
}
