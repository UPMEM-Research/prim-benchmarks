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

// 그룹화 함수 (Nested-Loop 알고리즘)
GroupedData groupByNestedLoop(Matrix *matrix, int groupByColumn) {
    GroupedData groupedData;
    groupedData.groupByColumn = groupByColumn;
    groupedData.groupCount = 0;

    for (int i = 0; i < matrix->rows; i++) {
        bool foundGroup = false;
        double groupValue = matrix->data[i][groupByColumn];

        for (int j = 0; j < groupedData.groupCount; j++) {
            int groupIndex = groupedData.groups[j].groupIndices[0];
            double firstGroupValue = matrix->data[groupIndex][groupByColumn];

            if (isEqual(groupValue, firstGroupValue)) {
                // 기존 그룹에 추가
                groupedData.groups[j].groupIndices[groupedData.groups[j].groupCount] = i;
                groupedData.groups[j].groupCount++;
                foundGroup = true;
                break;
            }
        }

        if (!foundGroup) {
            // 새로운 그룹 생성
            groupedData.groups[groupedData.groupCount].groupIndices[0] = i;
            groupedData.groups[groupedData.groupCount].groupCount = 1;
            groupedData.groupCount++;
        }
    }

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

    int groupByColumn = 0; // 기준 열 (첫 번째 열)
    GroupedData groupedData = groupByNestedLoop(&matrix, groupByColumn);

    printf("Grouped Data (Nested-Loop):\n");
    printGroupedData(&matrix, &groupedData);

    return 0;
}
