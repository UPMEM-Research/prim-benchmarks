#include <stdio.h>

#define MAX_ROWS 100
#define MAX_COLS 100

typedef struct {
    int rows;
    int cols;
    double data[MAX_ROWS][MAX_COLS];
} Matrix;

void initializeMatrix(Matrix *matrix, int rows, int cols, double values[][MAX_COLS]) {
    matrix->rows = rows;
    matrix->cols = cols;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix->data[i][j] = values[i][j];
        }
    }
}

Matrix selectFromMatrix(Matrix *matrix, int rowStart, int rowEnd, int colStart, int colEnd) {
    Matrix result;
    result.rows = rowEnd - rowStart + 1;
    result.cols = colEnd - colStart + 1;

    for (int i = 0; i < result.rows; i++) {
        for (int j = 0; j < result.cols; j++) {
            result.data[i][j] = matrix->data[i + rowStart][j + colStart];
        }
    }

    return result;
}

void printMatrix(Matrix *matrix) {
    for (int i = 0; i < matrix->rows; i++) {
        for (int j = 0; j < matrix->cols; j++) {
            printf("%lf\t", matrix->data[i][j]);
        }
        printf("\n");
    }
}

int main() {
    double values[3][3] = {
        {1.0, 2.0, 3.0},
        {4.0, 5.0, 6.0},
        {7.0, 8.0, 9.0}
    };

    Matrix matrix;
    initializeMatrix(&matrix, 3, 3, values);

    printf("Original Matrix:\n");
    printMatrix(&matrix);

    printf("Selected Matrix:\n");
    Matrix selectedMatrix = selectFromMatrix(&matrix, 0, 1, 1, 2);
    printMatrix(&selectedMatrix);

    return 0;
}