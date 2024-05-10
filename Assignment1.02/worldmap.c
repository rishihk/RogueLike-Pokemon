#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAP_HEIGHT 21
#define MAP_WIDTH 80
#define CLEARING '.'
#define POKEMON_CENTER 'C'
#define POKEMART 'M'
#define ROAD '#'
#define TALL_GRASS ':'
#define WATER '~'

int worldXCoord = 200;
int worldYCoord = 200;
struct Map *lastPrintedMap = NULL;

int playerX = 0;
int playerY = 0;

struct Map
{
    char map[21][80];
    int nGate;
    int eGate;
    int sGate;
    int wGate;
};

int centerX;
int centerY;

struct Map *world[401][401];

void createNSpaths(struct Map *m, int centerX, int centerY)
{
    if (m->nGate == 0)
    {
        m->nGate = rand() % 70 + 5;
    }
    if (m->sGate == 0)
    {
        m->sGate = rand() % 70 + 5;
    }

    // Initialize the starting point of the path and one row below it
    int row = 0; // Start at the north border
    int col = m->nGate;

    m->map[row][col] = '#';
    m->map[row + 1][col] = '#';

    row = 1; // Start from row 1 for the random behavior

    // Loop until the path reaches the south border
    while (row < 20)
    {
        int r = (rand() % 3);

        // Move one row down and stay in the same column
        if (r == 0)
        {
            row++;
        }
        // Move one column to the right and stay in the same row
        else if (r == 1 && col < 78)
        {
            col++;
        }
        // Move one column to the left and stay in the same row
        else if (r == 2 && col > 1)
        {
            col--;
        }

        // Set the new position as part of the path
        m->map[row][col] = '#';

        // Stop if we've reached the south border
        if (row == 20)
        {
            m->sGate = col; // Update the sGate to match the ending column
            break;
        }
    }
}

void createEWpaths(struct Map *m, int centerX, int centerY)
{
    if (m->wGate == 0)
    {
        m->wGate = rand() % 12 + 5;
    }
    if (m->eGate == 0)
    {
        m->eGate = rand() % 12 + 5;
    }

    // Initialize the starting point of the path
    int row = centerY; // Start at centerY
    int col = 0;       // Start at the west border

    m->map[row][col] = '#';
    m->map[row][col + 1] = '#';

    col = 1; // Start from column 1 for the random behavior

    // Loop until the path reaches the east border
    while (col < 79)
    {
        int r = (rand() % 3);

        // Move one row up and stay in the same column
        if (r == 0 && row > 1)
        {
            row--;
        }
        // Move one column to the right and stay in the same row
        else if (r == 1)
        {
            col++;
        }
        // Move one row down and stay in the same column
        else if (r == 2 && row < 19)
        {
            row++;
        }

        // Set the new position as part of the path
        m->map[row][col] = '#';

        // Stop if we've reached the east border
        if (col == 79)
        {
            break;
        }
    }
}

void sanitizeMap(struct Map *m)
{
    // Define the set of allowed characters in the map
    char allowed[] = {'%', '.', '#', 'C', 'M', ':', '~', '^'};

    // Iterate over the map array
    for (int i = 0; i < 21; i++)
    {
        for (int j = 0; j < 80; j++)
        {
            char current = m->map[i][j];
            int found = 0;

            // Check if the character is in the list of allowed characters
            for (int k = 0; k < sizeof(allowed) / sizeof(char); k++)
            {
                if (current == allowed[k])
                {
                    found = 1;
                    break;
                }
            }

            // If the character is not allowed, replace it with '.'
            if (!found)
            {
                m->map[i][j] = '.';
            }
        }
    }
}

void setPath(struct Map *m)
{
    int centerX = rand() % 20 + 30;
    int centerY = rand() % 5 + 7;

    createNSpaths(m, centerX, centerY);
    createEWpaths(m, centerX, centerY);
}

void addBorder(struct Map *m)
{
    // top
    for (int i = 0; i < 80; i++)
    {
        if (m->map[0][i] != '#')
        {
            m->map[0][i] = '%';
        }
    }
    // bottom
    for (int i = 0; i < 80; i++)
    {
        if (m->map[20][i] != '#')
        {
            m->map[20][i] = '%';
        }
    }

    // left
    for (int i = 0; i < 21; i++)
    {
        if (m->map[i][0] != '#')
        {
            m->map[i][0] = '%';
        }
    }

    // right
    for (int i = 0; i < 21; i++)
    {
        if (m->map[i][79] != '#')
        {
            m->map[i][79] = '%';
        }
    }

    if (worldYCoord == 0)
    {
        for (int i = 0; i < 80; i++)
        {
            m->map[0][i] = '%';
        }
    }
    if (worldYCoord == 400)
    {
        for (int i = 0; i < 80; i++)
        {
            m->map[20][i] = '%';
        }
    }
    if (worldXCoord == 0)
    {
        for (int i = 0; i < 21; i++)
        {
            m->map[i][0] = '%';
        }
    }
    if (worldXCoord == 400)
    {
        for (int i = 0; i < 21; i++)
        {
            m->map[i][79] = '%';
        }
    }
}

void PCandPM(struct Map *m)
{
    int distance = (abs(playerX) + abs(playerY)) / 2;
    int p = distance == 0 ? 100 : ((distance * -45) / 200 + 50); // 100% probability at 0,0

    int x = 0;
    int y = 0;

    // For placing C
    do
    {
        x = rand() % 68 + 5;
        y = rand() % 13 + 3;
    } while (
        m->map[y][x] != '#' || m->map[y + 1][x] != '#' ||
        m->map[y][x + 1] != '#' || m->map[y + 1][x + 1] != '#');

    if (p > rand() % 100)
    {
        m->map[y][x] = 'C';
        m->map[y][x + 1] = 'C';
        m->map[y + 1][x] = 'C';
        m->map[y + 1][x + 1] = 'C';
    }

    // Reset x and y
    x = 0;
    y = 0;

    // For placing M
    do
    {
        x = rand() % 68 + 5;
        y = rand() % 13 + 3;
    } while (
        m->map[y][x] != '#' || m->map[y + 1][x] != '#' ||
        m->map[y][x + 1] != '#' || m->map[y + 1][x + 1] != '#');

    if (p > rand() % 100)
    {
        m->map[y][x] = 'M';
        m->map[y][x + 1] = 'M';
        m->map[y + 1][x] = 'M';
        m->map[y + 1][x + 1] = 'M';
    }
}

void tallGrass(struct Map *m)
{
    int grassWidth = rand() % 10 + 6;
    int grassHeight = rand() % 3 + 5;
    int x = 0;
    int y = 0;
    while (x <= 0 || y <= 0)
    {
        x = rand() % 35 - grassWidth;
        y = rand() % 10 - grassHeight;
    }

    for (int i = y; i < y + grassHeight - 1; i++)
    {
        for (int j = x; j < x + grassWidth - 1; j++)
        {
            if (m->map[i][j] == 0)
            {
                m->map[i][j] = ':';
            }
        }
    }

    grassWidth = rand() % 10 + 6;
    grassHeight = rand() % 3 + 5;
    x = 0;
    y = 0;
    while (x <= 0 || y <= 0)
    {
        x = rand() % 40 + 35 - grassWidth;
        y = rand() % 10 + 10 - grassHeight;
    }

    for (int i = y; i < y + grassHeight - 1; i++)
    {
        for (int j = x; j < x + grassWidth - 1; j++)
        {
            if (m->map[i][j] == 0)
            {
                m->map[i][j] = ':';
            }
        }
    }
}

void shortGrass(struct Map *m)
{
    int grassWidth = rand() % 12 + 3;
    int grassHeight = rand() % 6 + 3;
    int x = 0;
    int y = 0;
    while (x <= 0 || y <= 0 || m->map[y][x] != 0)
    {
        x = rand() % 35 - grassWidth;
        y = rand() % 10 - grassHeight;
    }

    for (int i = y; i < y + grassHeight - 1; i++)
    {
        for (int j = x; j < x + grassWidth - 1; j++)
        {
            if (m->map[i][j] == 0)
            {
                m->map[i][j] = '.';
            }
        }
    }

    grassWidth = rand() % 12 + 3;
    grassHeight = rand() % 6 + 3;
    x = 0;
    y = 0;
    while (x <= 0 || y <= 0 || m->map[y][x] != 0)
    {
        x = rand() % 40 + 35 - grassWidth;
        y = rand() % 10 + 10 - grassHeight;
    }

    for (int i = y; i < y + grassHeight - 1; i++)
    {
        for (int j = x; j < x + grassWidth - 1; j++)
        {
            if (m->map[i][j] == 0)
            {
                m->map[i][j] = '.';
            }
        }
    }
}

void water(struct Map *m)
{
    int waterWidth = 9;
    int waterHeight = 6;
    int x = 0;
    int y = 0;
    do
    {
        x = rand() % 80 - waterWidth - 1;
        y = rand() % 20 - waterHeight - 1;
    } while (y < 0 || x < 0 || m->map[y][x] != 0);

    for (int i = y; i < y + waterHeight - 1; i++)
    {
        for (int j = x; j < x + waterWidth - 1; j++)
        {
            if (m->map[i][j] == 0)
            {
                m->map[i][j] = '~';
            }
        }
    }
}

void fillMap(struct Map *m)
{
    for (int i = 0; i < 21; i++)
    {
        for (int j = 0; j < 80; j++)
        {
            if (m->map[i][j] == 0)
            {
                int k = rand() % 3;
                if (k == 0)
                {
                    m->map[i][j] = '.';
                }
                if (k == 1)
                {
                    m->map[i][j] = '.';
                }
                if (k == 2)
                {
                    m->map[i][j] = '.';
                }
            }
        }
    }
}

void printMap(struct Map *m)
{
    for (int i = 0; i < 21; i++)
    {
        for (int j = 0; j < 80; j++)
        {
            char current = m->map[i][j];
            switch (current)
            {
            case '.': // Short grass
                printf("\033[0;33m.\033[0m");
                break;
            case '#': // Path
                printf("\033[1;33m#\033[0m");
                break;
            case '%': // Borders
                printf("\033[1;31m%%\033[0m");
                break;
            case 'C':
                printf("\033[1;94mC\033[0m");
                break;

            case 'M':
                printf("\033[1;35mM\033[0m");
                break;
            case ':': // Tall grass
                printf("\033[1;92m:\033[0m");
                break;
            case '~': // Water
                printf("\033[1;36m~\033[0m");
                break;
            case '^':
                printf("\033[1;37m^\033[0m");
                break;
            default: // Default color
                putchar(current);
                break;
            }
        }
        putchar('\n');
    }
}

void placeTrees(struct Map *m)
{
    int numberOfTrees = 20;
    for (int i = 0; i < numberOfTrees; i++)
    {
        int row, col;
        do
        {
            row = rand() % 19 + 1; // Between 1 and 19 to avoid borders
            col = rand() % 78 + 1; // Between 1 and 78 to avoid borders
        } while (m->map[row][col] != '.');
        m->map[row][col] = '^';
    }
}

void placeBoulders(struct Map *m)
{
    int numberOfBoulders = 10;
    for (int i = 0; i < numberOfBoulders; i++)
    {
        int row, col;
        do
        {
            row = rand() % 19 + 1; // Between 1 and 19 to avoid borders
            col = rand() % 78 + 1; // Between 1 and 78 to avoid borders
        } while (m->map[row][col] != '.');
        m->map[row][col] = '%';
    }
}

void createMap(struct Map *m)
{
    setPath(m);
    PCandPM(m);
    addBorder(m);
    tallGrass(m);
    shortGrass(m);
    water(m);
    fillMap(m);
    placeBoulders(m);
    placeTrees(m);
    sanitizeMap(m);
}

void north()
{
    if (worldYCoord > 0)
    {
        playerY = playerY + 1;
        worldYCoord = worldYCoord - 1;
    }
    if (world[worldYCoord][worldXCoord] == 0)
    {
        struct Map *m = malloc(sizeof(*m));
        world[worldYCoord][worldXCoord] = m;

        if (world[worldYCoord + 1][worldXCoord] != 0 && worldYCoord != 400)
        {
            m->sGate = world[worldYCoord + 1][worldXCoord]->nGate;
        }
        if (world[worldYCoord - 1][worldXCoord] != 0 && worldYCoord != 0)
        {
            m->nGate = world[worldYCoord - 1][worldXCoord]->sGate;
        }

        if (world[worldYCoord][worldXCoord + 1] != 0 && worldXCoord != 400)
        {
            m->eGate = world[worldYCoord][worldXCoord + 1]->wGate;
        }

        if (world[worldYCoord][worldXCoord - 1] != 0 && worldXCoord != 0)
        {
            m->wGate = world[worldYCoord][worldXCoord - 1]->eGate;
        }

        createMap(m);
        printMap(m);
        lastPrintedMap = m;
        printf("You are at (%d, %d) ", playerX, playerY);
    }
    else
    {
        printMap(world[worldYCoord][worldXCoord]);
        lastPrintedMap = world[worldYCoord][worldXCoord];
        printf("You are at (%d, %d) ", playerX, playerY);
    }
}

void south()
{
    if (worldYCoord < 400)
    {
        playerY = playerY - 1;
        worldYCoord = worldYCoord + 1;
    }
    if (world[worldYCoord][worldXCoord] == 0)
    {
        struct Map *m = malloc(sizeof(*m));
        world[worldYCoord][worldXCoord] = m;

        if (world[worldYCoord + 1][worldXCoord] != 0 && worldYCoord != 400)
        {
            m->sGate = world[worldYCoord + 1][worldXCoord]->nGate;
        }
        if (world[worldYCoord - 1][worldXCoord] != 0 && worldYCoord != 0)
        {
            m->nGate = world[worldYCoord - 1][worldXCoord]->sGate;
        }

        if (world[worldYCoord][worldXCoord + 1] != 0 && worldXCoord != 400)
        {
            m->eGate = world[worldYCoord][worldXCoord + 1]->wGate;
        }

        if (world[worldYCoord][worldXCoord - 1] != 0 && worldXCoord != 0)
        {
            m->wGate = world[worldYCoord][worldXCoord - 1]->eGate;
        }
        createMap(m);
        printMap(m);
        lastPrintedMap = m;
        printf("You are at (%d, %d) ", playerX, playerY);
    }
    else
    {
        printMap(world[worldYCoord][worldXCoord]);
        lastPrintedMap = world[worldYCoord][worldXCoord];
        printf("You are at (%d, %d) ", playerX, playerY);
    }
}

void west()
{
    if (worldXCoord > 0)
    {
        playerX = playerX - 1;
        worldXCoord = worldXCoord - 1;
    }
    if (world[worldYCoord][worldXCoord] == 0)
    {
        struct Map *m = malloc(sizeof(*m));
        world[worldYCoord][worldXCoord] = m;

        if (world[worldYCoord + 1][worldXCoord] != 0 && worldYCoord != 400)
        {
            m->sGate = world[worldYCoord + 1][worldXCoord]->nGate;
        }
        if (world[worldYCoord - 1][worldXCoord] != 0 && worldYCoord != 0)
        {
            m->nGate = world[worldYCoord - 1][worldXCoord]->sGate;
        }

        if (world[worldYCoord][worldXCoord + 1] != 0 && worldXCoord != 400)
        {
            m->eGate = world[worldYCoord][worldXCoord + 1]->wGate;
        }

        if (world[worldYCoord][worldXCoord - 1] != 0 && worldXCoord != 0)
        {
            m->wGate = world[worldYCoord][worldXCoord - 1]->eGate;
        }

        createMap(m);
        printMap(m);
        lastPrintedMap = m;
        printf("You are at (%d, %d) ", playerX, playerY);
    }
    else
    {
        printMap(world[worldYCoord][worldXCoord]);
        lastPrintedMap = world[worldYCoord][worldXCoord];
        printf("You are at (%d, %d) ", playerX, playerY);
    }
}

void east()
{
    if (worldXCoord < 400)
    {
        playerX = playerX + 1;
        worldXCoord = worldXCoord + 1;
    }
    if (world[worldYCoord][worldXCoord] == 0)
    {
        struct Map *m = malloc(sizeof(*m));
        world[worldYCoord][worldXCoord] = m;

        if (world[worldYCoord + 1][worldXCoord] != 0 && worldYCoord != 400)
        {
            m->sGate = world[worldYCoord + 1][worldXCoord]->nGate;
        }
        if (world[worldYCoord - 1][worldXCoord] != 0 && worldYCoord != 0)
        {
            m->nGate = world[worldYCoord - 1][worldXCoord]->sGate;
        }

        if (world[worldYCoord][worldXCoord + 1] != 0 && worldXCoord != 400)
        {
            m->eGate = world[worldYCoord][worldXCoord + 1]->wGate;
        }

        if (world[worldYCoord][worldXCoord - 1] != 0 && worldXCoord != 0)
        {
            m->wGate = world[worldYCoord][worldXCoord - 1]->eGate;
        }

        createMap(m);
        printMap(m);
        lastPrintedMap = m;
        printf("You are at (%d, %d) ", playerX, playerY);
    }
    else
    {
        printMap(world[worldYCoord][worldXCoord]);
        lastPrintedMap = world[worldYCoord][worldXCoord];
        printf("You are at (%d, %d) ", playerX, playerY);
    }
}

void fly(int x, int y)
{
    // Coordinates for world
    worldXCoord = 200 + x;
    worldYCoord = 200 - y;

    // Check if the coordinates are within the boundaries
    if (x < -200 || x > 200 || y < -200 || y > 200)
    {
        printf("Error: Unreachable coordinate. Taking you back to the start!\n");
        playerX = 0;
        playerY = 0;
        worldXCoord = 200;
        worldYCoord = 200;
        printMap(world[200][200]);
        lastPrintedMap = world[200][200];
        return;
    }

    // Update player's coordinates
    playerX = x;
    playerY = y;

    if (world[worldYCoord][worldXCoord] == 0)
    {
        struct Map *m = malloc(sizeof(*m));
        world[worldYCoord][worldXCoord] = m;

        // Handle gates
        if (worldYCoord < 200 && worldYCoord != 0)
        {
            if (world[worldYCoord + 1][worldXCoord] != 0)
            {
                m->sGate = world[worldYCoord + 1][worldXCoord]->nGate;
            }
        }
        if (worldYCoord > 0)
        {
            if (world[worldYCoord - 1][worldXCoord] != 0)
            {
                m->nGate = world[worldYCoord - 1][worldXCoord]->sGate;
            }
        }
        if (worldXCoord < 400 && worldXCoord != 0)
        {
            if (world[worldYCoord][worldXCoord + 1] != 0)
            {
                m->eGate = world[worldYCoord][worldXCoord + 1]->wGate;
            }
        }
        if (worldXCoord > 0)
        {
            if (world[worldYCoord][worldXCoord - 1] != 0)
            {
                m->wGate = world[worldYCoord][worldXCoord - 1]->eGate;
            }
        }

        // If at border, close that particular border
        if (worldYCoord == 0)
        {
            m->nGate = 0;
        }
        if (worldYCoord == 400)
        {
            m->sGate = 0;
        }
        if (worldXCoord == 0)
        {
            m->wGate = 0;
        }
        if (worldXCoord == 400)
        {
            m->eGate = 0;
        }

        // Create and print the map
        createMap(m);
        printMap(m);
        lastPrintedMap = m;
        printf("You are at (%d, %d) ", playerX, playerY);
    }
    else
    {
        printMap(world[worldYCoord][worldXCoord]);
        lastPrintedMap = world[worldYCoord][worldXCoord];
        printf("You are at (%d, %d) ", playerX, playerY);
    }
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    struct Map *m = malloc(sizeof(*m));
    // struct Map *lastPrintedMap = NULL; // Keep track of the last printed map

    world[worldYCoord][worldXCoord] = m;
    setPath(m);
    PCandPM(m);
    addBorder(m);
    tallGrass(m);
    shortGrass(m);
    water(m);
    fillMap(m);
    placeBoulders(m);
    placeTrees(m);
    printMap(m);
    lastPrintedMap = m; // Update the last printed map

    sanitizeMap(m);
    printf("Welcome to Pokemon! \nYou are at (%d, %d) ", playerX, playerY);

    char input;
    int z;
    printf("\nEnter a command to travel the world (n ,s, e, w, f x y): ");

    while (input != 'q')
    {
        z = scanf(" %c", &input);
        if (z == 0)
        {
            printf("Invalid input. Try again.\n");
            continue;
        }

        // Handle commands
        if (input == 'n')
        {
            north();
        }
        else if (input == 'e')
        {
            east();
        }
        else if (input == 's')
        {
            south();
        }
        else if (input == 'w')
        {
            west();
        }
        else if (input == 'f')
        {
            int x, y;
            z = scanf(" %d %d", &x, &y);
            if (z != 2)
            {
                printf("Invalid coordinates. Try again.\n");
                continue;
            }
            fly(x, y);
        }
        else if (input == 'q')
        {
            break;
        }
        else
        {
            printf("Unknown command, taking you back to where you were. Try again.\n");
            if (lastPrintedMap != NULL) // If we have a last printed map, show it
            {
                printMap(lastPrintedMap);
            }
        }

        printf("\nEnter a command to travel the world (n ,s, e, w, f x y, q): ");
    }

    return 0;
}
