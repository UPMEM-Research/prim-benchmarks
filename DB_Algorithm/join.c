#include <stdio.h>

#define MAX_ROWS 100
#define MAX_COLS 100

typedef struct {
    int rows;
    int cols;
    double data[MAX_ROWS][MAX_COLS];
} Matrix;

Matrix innerJoinMatrices(Matrix *matrix1, Matrix *matrix2, int joinColumn) {
    Matrix result;
    result.rows = 0;
    result.cols = matrix1->cols + matrix2->cols - 1;

    for (int i = 0; i < matrix1->rows; i++) {
        for (int j = 0; j < matrix2->rows; j++) {
            if (matrix1->data[i][joinColumn] == matrix2->data[j][joinColumn]) {
                for (int k = 0; k < matrix1->cols; k++) {
                    result.data[result.rows][k] = matrix1->data[i][k];
                }
                for (int k = 0; k < matrix2->cols; k++) {
                    if (k != joinColumn) {
                        result.data[result.rows][matrix1->cols + k - 1] = matrix2->data[j][k];
                    }
                }
                result.rows++;
            }
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
    double values1[3][3] = {
        {1.0, 10.0, 100.0},
        {2.0, 20.0, 200.0},
        {3.0, 30.0, 300.0}
    };

    double values2[3][2] = {
        {1.0, 1000.0},
        {2.0, 2000.0},
        {3.0, 3000.0}
    };

    Matrix matrix1;
    Matrix matrix2;

    matrix1.rows = 3;
    matrix1.cols = 3;
    matrix2.rows = 3;
    matrix2.cols = 2;

    for (int i = 0; i < matrix1.rows; i++) {
        for (int j = 0; j < matrix1.cols; j++) {
            matrix1.data[i][j] = values1[i][j];
        }
    }

    for (int i = 0; i < matrix2.rows; i++) {
        for (int j = 0; j < matrix2.cols; j++) {
            matrix2.data[i][j] = values2[i][j];
        }
    }

    printf("Matrix 1:\n");
    printMatrix(&matrix1);

    printf("Matrix 2:\n");
    printMatrix(&matrix2);

    printf("Joined Matrix:\n");
    Matrix joinedMatrix = innerJoinMatrices(&matrix1, &matrix2, 0);
    printMatrix(&joinedMatrix);

    return 0;
}