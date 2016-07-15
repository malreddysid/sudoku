#include "getImage.h"

using namespace std;
using namespace cv;

#define UNASSIGNED 0
#define N 9

bool FindUnassignedLocation(int **grid, int &row, int &col);
bool isSafe(int **grid, int row, int col, int num);

bool solve(int **grid)
{
    char valueExists[9];
    
    // check rows
    for (int i = 0; i < 9; i++) {
        memset(valueExists, 0, 9 * sizeof(char));
        for (int j = 0; j < 9; j++) {
            int value = grid[i][j];
            if (!value) continue;
            char* exist = &valueExists[value - 1];
            if (*exist) {
                return false;
            }
            *exist = 1;
        }
    }
    // check cols
    for (int j = 0; j < 9; j++) {
        memset(valueExists, 0, 9 * sizeof(char));
        for (int i = 0; i < 9; i++) {
            int value = grid[i][j];
            if (!value) continue;
            char* exist = &valueExists[value - 1];
            if (*exist) {
                return false;
            }
            *exist = 1;
        }
    }
    // check blocks
    for (int a = 0; a < 3; a++) {
        for (int b = 0; b < 3; b++) {
            memset(valueExists, 0, 9 * sizeof(char));
            for (int i = 3 * a; i < 3 * (a + 1); i++) {
                for (int j = 3 * b; j < 3 * (b + 1); j++) {
                    int value = grid[i][j];
                    if (!value) continue;
                    char* exist = &valueExists[value - 1];
                    if (*exist) {
                        return false;
                    }
                    *exist = 1;
                }
            }
        }
    }


    int row, col;
    if (!FindUnassignedLocation(grid, row, col))
        return true;
    for (int num = 1; num <= 9; num++)
    {
        if (isSafe(grid, row, col, num))
        {
            grid[row][col] = num;
            if (solve(grid))
                return true;
            grid[row][col] = UNASSIGNED;
        }
    }
    return false;
}

bool FindUnassignedLocation(int **grid, int &row, int &col)
{
    for (row = 0; row < N; row++)
        for (col = 0; col < N; col++)
            if (grid[row][col] == UNASSIGNED)
                return true;
    return false;
}

bool UsedInRow(int **grid, int row, int num)
{
    for (int col = 0; col < N; col++)
        if (grid[row][col] == num)
            return true;
    return false;
}

bool UsedInCol(int **grid, int col, int num)
{
    for (int row = 0; row < N; row++)
        if (grid[row][col] == num)
            return true;
    return false;
}

bool UsedInBox(int **grid, int boxStartRow, int boxStartCol, int num)
{
    for (int row = 0; row < 3; row++)
        for (int col = 0; col < 3; col++)
            if (grid[row + boxStartRow][col + boxStartCol] == num)
                return true;
    return false;
}

bool isSafe(int **grid, int row, int col, int num)
{
    return !UsedInRow(grid, row, num) && !UsedInCol(grid, col, num) &&
    !UsedInBox(grid, row - row % 3, col - col % 3, num);
}



int main()
{
    char image[] = "./sudoku.jpg";
    int **sudoku = (int **) malloc(sizeof(int *) * 9);
    for(int i = 0; i < 9; i++)
    {
        sudoku[i] = (int *) malloc(sizeof(int) * 9);
    }
    
    getImage(image, sudoku);
    // Print puzzle
    
    for(int i = 0; i < 9; i++)
    {
        for(int j = 0; j < 9; j++)
        {
            if(sudoku[i][j] == 0)
                printf("   |");
            else
                printf(" %d |", sudoku[i][j]);
        }
        printf("\n");
    }
    
    int possible = solve(sudoku);
    
    if(possible)
    {
        for(int i = 0; i < 9; i++)
        {
            for(int j = 0; j < 9; j++)
            {
                if(sudoku[i][j] == 0)
                    printf("   |");
                else
                    printf(" %d |", sudoku[i][j]);
            }
            printf("\n");
        }
    }
    else
    {
        printf("Not possible\n");
    }
    return 0;
}
