#include <stdio.h>

#define MAX_ROWS 100
#define MAX_COLS 100

typedef struct {
    int rows;
    int cols;
    double data[MAX_ROWS][MAX_COLS];
} Matrix;

void sortMergeJoin(Matrix *matrix1, Matrix *matrix2, int joinColumn) {
    int i = 0, j = 0;

    while (i < matrix1->rows && j < matrix2->rows) {
        if (matrix1->data[i][joinColumn] == matrix2->data[j][joinColumn]) {
            // Join matched rows
            for (int k = 0; k < matrix1->cols; k++) {
                printf("%lf\t", matrix1->data[i][k]);
            }
            for (int k = 0; k < matrix2->cols; k++) {
                if (k != joinColumn) {
                    printf("%lf\t", matrix2->data[j][k]);
                }
            }
            printf("\n");
            i++;
            j++;
        } else if (matrix1->data[i][joinColumn] < matrix2->data[j][joinColumn]) {
            i++;
        } else {
            j++;
        }
    }
}

int main() {
    // Define two matrices (assumed to be sorted)
    // Matrix1
    Matrix matrix1;
    matrix1.rows = 3;
    matrix1.cols = 3;
    double values1[3][3] = {
        {1.0, 10.0, 100.0},
        {2.0, 20.0, 200.0},
        {3.0, 30.0, 300.0}
    };
    for (int i = 0; i < matrix1.rows; i++) {
        for (int j = 0; j < matrix1.cols; j++) {
            matrix1.data[i][j] = values1[i][j];
        }
    }

    // Matrix2
    Matrix matrix2;
    matrix2.rows = 3;
    matrix2.cols = 3;
    double values2[3][3] = {
        {1.0, 1000.0, 10000.0},
        {3.0, 3000.0, 30000.0},
        {4.0, 4000.0, 40000.0}
    };
    for (int i = 0; i < matrix2.rows; i++) {
        for (int j = 0; j < matrix2.cols; j++) {
            matrix2.data[i][j] = values2[i][j];
        }
    }

    printf("Sort-Merge Join Result:\n");
    sortMergeJoin(&matrix1, &matrix2, 0);

    return 0;
}

