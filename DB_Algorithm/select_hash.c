#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_ROWS 100
#define MAX_COLS 100
#define HASH_TABLE_SIZE 100

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

// Hash 테이블 구조체
typedef struct {
    int count;
    int indices[MAX_ROWS]; // 저장된 행 인덱스들
} HashTableBucket;

// 두 값이 같은지 확인하는 함수
bool isEqual(double a, double b) {
    // 부동소수점 비교에서는 정확한 값이 아닌 근사값으로 비교해야 합니다.
    double epsilon = 0.0001; // 적절한 값으로 조정
    return (abs(a - b) < epsilon);
}

// 해시 함수 (그룹화 기준 열을 기준으로 해시 값을 계산)
int hashFunction(double value) {
    // 간단한 해시 함수로 임의의 값을 더한 뒤, modulo 연산을 수행합니다.
    return (int)(value) % HASH_TABLE_SIZE;
}

// 그룹화 함수 (Hash 알고리즘)
GroupedData selectByHash(Matrix *matrix, int columnIndex, double value) {
    GroupedData groupedData;
    groupedData.groupByColumn = columnIndex;
    groupedData.groupCount = 0;

    // Hash 테이블 초기화
    HashTableBucket hashTable[HASH_TABLE_SIZE];
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        hashTable[i].count = 0;
    }

    // 데이터를 Hash 테이블에 추가
    for (int i = 0; i < matrix->rows; i++) {
        double groupValue = matrix->data[i][columnIndex];
        int hashIndex = hashFunction(groupValue);

        hashTable[hashIndex].indices[hashTable[hashIndex].count] = i;
        hashTable[hashIndex].count++;
    }

    // 조건에 해당하는 그룹을 추출하여 GroupedData 구조체로 옮기기
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        for (int j = 0; j < hashTable[i].count; j++) {
            int rowIndex = hashTable[i].indices[j];
            double groupValue = matrix->data[rowIndex][columnIndex];

            if (isEqual(groupValue, value)) {
                groupedData.groups[groupedData.groupCount].groupIndices[0] = rowIndex;
                groupedData.groups[groupedData.groupCount].groupCount = 1;
                groupedData.groupCount++;
            }
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

    int columnIndex = 0; // 검색할 열 (첫 번째 열)
    double value = 1.0; // 검색할 값
    GroupedData groupedData = selectByHash(&matrix, columnIndex, value);

    printf("Selected Data (Hash):\n");
    printGroupedData(&matrix, &groupedData);

    return 0;
}
