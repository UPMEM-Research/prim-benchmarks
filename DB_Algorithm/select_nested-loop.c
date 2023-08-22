#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_ROWS 100
#define MAX_COLS 100

typedef struct {
    int rows;
    int cols;
    double data[MAX_ROWS][MAX_COLS];
} Matrix;

// SELECT 함수 (Nested-Loop 알고리즘)
Matrix selectByNestedLoop(Matrix *matrix, int columnIndex, double value) {
    Matrix result;
    result.rows = 0;
    result.cols = matrix->cols;

    for (int i = 0; i < matrix->rows; i++) {
        double currentValue = matrix->data[i][columnIndex];

        // 해당 행이 조건을 만족하는지 확인
        if (currentValue == value) {
            // 결과 행에 추가
            for (int j = 0; j < matrix->cols; j++) {
                result.data[result.rows][j] = matrix->data[i][j];
            }
            result.rows++;
        }
    }

    return result;
}

// 행렬 출력 함수
void printMatrix(Matrix *matrix) {
    for (int i = 0; i < matrix->rows; i++) {
        for (int j = 0; j < matrix->cols; j++) {
            printf("%lf\t", matrix->data[i][j]);
        }
        printf("\n");
    }
}

int main() {
    // Define a matrix
    Matrix matrix;
    matrix.rows = 6;
    matrix.cols = 3;
    double values[6][3] = {
        {1.0, 10.0, 100.0},
        {2.0, 20.0, 200.0},
        {1.0, 15.0, 150.0},
        {3.0, 30.0, 300.0},
        {2.0, 25.0, 250.0},
        {3.0, 35.0, 350.0}
    };
    for (int i = 0; i < matrix.rows; i++) {
        for (int j = 0; j < matrix.cols; j++) {
            matrix.data[i][j] = values[i][j];
        }
    }

    int columnIndex = 0; // 검색할 열 (첫 번째 열)
    double value = 1.0; // 검색할 값
    Matrix result = selectByNestedLoop(&matrix, columnIndex, value);

    printf("Selected Data (Nested-Loop):\n");
    printMatrix(&result);

    return 0;
}
