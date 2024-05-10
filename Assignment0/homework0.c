#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

/* Do not modify write_pgm() or read_pgm() */
int write_pgm(char *file, void *image, uint32_t x, uint32_t y)
{
    FILE *o;

    if (!(o = fopen(file, "w")))
    {
        perror(file);

        return -1;
    }

    fprintf(o, "P5\n%u %u\n255\n", x, y);

    /* Assume input data is correctly formatted. *
     * There's no way to handle it, otherwise.   */

    if (fwrite(image, 1, x * y, o) != (x * y))
    {
        perror("fwrite");
        fclose(o);

        return -1;
    }

    fclose(o);

    return 0;
}

/* A better implementation of this function would read the image dimensions *
 * from the input and allocate the storage, setting x and y so that the     *
 * user can determine the size of the file at runtime.  In order to         *
 * minimize complication, I've written this version to require the user to  *
 * know the size of the image in advance.                                   */
int read_pgm(char *file, void *image, uint32_t x, uint32_t y)
{
    FILE *f;
    char s[80];
    unsigned i, j;

    if (!(f = fopen(file, "r")))
    {
        perror(file);

        return -1;
    }

    if (!fgets(s, 80, f) || strncmp(s, "P5", 2))
    {
        fprintf(stderr, "Expected P6\n");

        return -1;
    }

    /* Eat comments */
    do
    {
        fgets(s, 80, f);
    } while (s[0] == '#');

    if (sscanf(s, "%u %u", &i, &j) != 2 || i != x || j != y)
    {
        fprintf(stderr, "Expected x and y dimensions %u %u\n", x, y);
        fclose(f);

        return -1;
    }

    /* Eat comments */
    do
    {
        fgets(s, 80, f);
    } while (s[0] == '#');

    if (strncmp(s, "255", 3))
    {
        fprintf(stderr, "Expected 255\n");
        fclose(f);

        return -1;
    }

    if (fread(image, 1, x * y, f) != x * y)
    {
        perror("fread");
        fclose(f);

        return -1;
    }

    fclose(f);

    return 0;
}

int main(int argc, char *argv[])
{
    int8_t image[1024][1024];
    int8_t out[1024][1024];

    /* Example usage of PGM functions */
    /* This assumes that motorcycle.pgm is a pgm image of size 1024x1024 */
    read_pgm("bigger_digger.pgm", image, 1024, 1024);

    int Ox[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}}; // sobel filters for horizontal and vertical
    int Oy[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

    for (int r = 1; r < 1023; r++) // going thru the image
    {
        for (int c = 1; c < 1023; c++)
        {

            int8_t accumulate_x = 0; // initialized accumulator
            int8_t accumulate_y = 0;

            for (int j = 0; j < 3; j++) // going through the kernel
            {
                for (int i = 0; i < 3; i++)
                {

                    int neighbour_row = r - 1 + j; // getting neighbour co-ords
                    int neighbour_col = c - 1 + i;

                    accumulate_x += Ox[j][i] * image[neighbour_row][neighbour_col]; // accumulating
                    accumulate_y += Oy[j][i] * image[neighbour_row][neighbour_col];
                }
            }

            int magnitude = sqrt(pow(accumulate_x, 2) + pow(accumulate_y, 2)); // getting magnitude

            out[r][c] = (int8_t)magnitude; // setting it to output
        }
    }

    /* After processing the image and storing your output in "out", write *
     * to motorcycle.edge.pgm.                                            */
    write_pgm("bigger_digger.edge.pgm", out, 1024, 1024);

    return 0;
}