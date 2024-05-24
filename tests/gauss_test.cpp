#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <chrono>

#include "../main/resistance_matrix.h"

// g++ -O1 tests/gauss_test.cpp main/resistance_matrix.cpp && ./a.out

int main() {

    calc_type** matrix = new calc_type*[4];
    for (int i=0;i<5;i++)
        matrix[i] = new calc_type[4];

    matrix[0][0] = 1;
    matrix[0][1] = 1;
    matrix[0][2] = 1;
    matrix[0][3] = 1;

    matrix[1][0] = 0;
    matrix[1][1] = 1;
    matrix[1][2] = 0;
    matrix[1][3] = 1;

    matrix[2][0] = 0;
    matrix[2][1] = 0;
    matrix[2][2] = 1;
    matrix[2][3] = 1;

    matrix[3][0] = 1;
    matrix[3][1] = 0;
    matrix[3][2] = 0;
    matrix[3][3] = 1;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i=0;i<1001;i++)
        gauss(matrix, 4, 4);
    auto elapsed = std::chrono::high_resolution_clock::now() - start;

    long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();


    for (int i=0;i<4;i++) {
        for (int j=0;j<4;j++)
            std::cout << (float)matrix[i][j] << "\t";

        std::cout << std::endl;
    }

    std::cout << microseconds << "us" << std::endl;

    for (int i=0;i<4;i++)
        delete [] matrix[i];
    delete [] matrix;

    return 0;
}
