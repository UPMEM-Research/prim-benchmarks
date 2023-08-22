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

typedef struct {
    int groupCount;
    int groupIndices[MAX_ROWS]; // 저장된 행 인덱스들
} Group;

typedef struct {
    int groupByColumn; // 그룹화 기준 열
    int groupCount;
    Group groups[MAX_ROWS];
} GroupedData;

// 두 값이 같은지 확인하는 함수
bool isEqual(double a, double b) {
    // 부동소수점 비교에서는 정확한 값이 아닌 근사값으로 비교해야 합니다.
    double epsilon = 0.0001; // 적절한 값으로 조정
    return (abs(a - b) < epsilon);
}

// 비교 함수 (그룹화 기준 열을 기준으로 오름차순 정렬)
int compareRows(const void *a, const void *b) {
    Matrix *matrix = (Matrix *)a;
    int row1 = *((int *)b);
    int row2 = *((int *)a);
    double value1 = matrix->data[row1][matrix->groupByColumn];
    double value2 = matrix->data[row2][matrix->groupByColumn];

    if (isEqual(value1, value2)) {
        return 0;
    } else if (value1 < value2) {
        return -1;
    } else {
        return 1;
    }
}

// 그룹화 함수 (Sort-Merge 알고리즘)
GroupedData selectBySortMerge(Matrix *matrix, int columnIndex, double value) {
    GroupedData groupedData;
    groupedData.groupByColumn = columnIndex;
    groupedData.groupCount = 0;

    // 그룹화 기준 열을 기준으로 데이터 정렬
    int rowIndices[MAX_ROWS];
    for (int i = 0; i < matrix->rows; i++) {
        rowIndices[i] = i;
    }
    qsort(rowIndices, matrix->rows, sizeof(int), compareRows);

    // 정렬된 데이터로 그룹화된 결과 생성
    int currentGroup = -1;
    double currentValue = -1.0;

    for (int i = 0; i < matrix->rows; i++) {
        int rowIndex = rowIndices[i];
        double groupValue = matrix->data[rowIndex][columnIndex];

        if (i == 0 || !isEqual(groupValue, currentValue)) {
            // 새로운 그룹 생성
            currentGroup++;
            currentValue = groupValue;
            groupedData.groups[currentGroup].groupCount = 0;
        }

        if (isEqual(groupValue, value)) {
            // 조건에 해당하는 경우, 그룹에 추가
            groupedData.groups[currentGroup].groupIndices[groupedData.groups[currentGroup].groupCount] = rowIndex;
            groupedData.groups[currentGroup].groupCount++;
        }
    }

    groupedData.groupCount = currentGroup + 1;

    return groupedData;
}

// 그룹화된 데이터 출력 함수
void printGroupedData(Matrix *matrix, GroupedData *groupedData) {
    for (int i = 0; i < groupedData->groupCount; i++) {
        int groupIndex = groupedData->groups[i].groupIndices[0];
        double groupValue = matrix->data[groupIndex][groupedData->groupByColumn];
        printf("Group Value: %lf\n", groupValue);

        for (int j = 0; j < groupedData->groups[i].groupCount; j++) {
            int rowIndex = groupedData->groups[i].groupIndices[j];
            for (int k = 0; k < matrix->cols; k++) {
                printf("%lf\t", matrix->data[rowIndex][k]);
            }
            printf("\n");
        }
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
    GroupedData groupedData = selectBySortMerge(&matrix, columnIndex, value);

    printf("Selected Data (Sort-Merge):\n");
    printGroupedData(&matrix, &groupedData);

    return 0;
}
