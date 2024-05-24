#include "resistance_matrix.h"
#include <cmath>
#include <iostream>


void printMatrix(calc_type** matrix, unsigned cols, unsigned rows) {
    std::cout << "Matrix cols: " << cols << " rows: " << rows << std::endl;
    for (unsigned i=0;i<rows;i++) {
        for (unsigned j=0;j<cols;j++) 
            std::cout << (float)matrix[i][j] << "\t";
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

calc_type** newMatrix(unsigned cols, unsigned rows) {
    calc_type** matrix = new calc_type*[rows+1];
    for (unsigned i=0;i<rows;i++)
        matrix[i] = new calc_type[cols]();
    matrix[rows] = NULL;
    return matrix;
}

void deleteMatrix(calc_type** matrix) {
    for (unsigned i=0;matrix[i];i++)
        delete [] matrix[i];
    delete [] matrix;
}


// rows >= cols
bool gauss(calc_type** matrix, unsigned cols, unsigned rows, calc_type** output, unsigned* rowOrder, bool* used) {
    calc_type result[rows][cols];
    unsigned rowOrderBuf[cols];
    bool usedBuf[rows];
    if (rowOrder == NULL) rowOrder = rowOrderBuf;
    if (used == NULL) used = usedBuf;

    for (unsigned i=0;i<rows;i++) {
        used[i] = false;
        for (unsigned j=0;j<cols;j++)
            result[i][j] = 0;
    }
    
    for (unsigned i=0;i<cols;i++) {

        unsigned bestRow = 0;
        calc_type bestGrade = -1;
        for (unsigned j=0;j<rows;j++) {
            if (used[j]) continue;
            calc_type absValue = std::abs(matrix[j][i]);
            if (absValue > bestGrade) {
                bestGrade = absValue;
                bestRow = j;
            }           
        }
        
        // if (bestGrade <= 0) std::cout << i << " " << (float)bestGrade << std::endl;
        if (bestGrade <= 0) return false;

        used[bestRow] = true;
        rowOrder[i] = bestRow;
        calc_type inversion = (calc_type)1.f / matrix[bestRow][i];
        result[bestRow][i] = 1;
        for (unsigned k=i;k<cols;k++) 
            matrix[bestRow][k] *= inversion;
        for (unsigned k=0;k<=i;k++)
            result[bestRow][k] *= inversion;

        for (unsigned j=0;j<rows;j++) {
            if (j!=bestRow) {
                calc_type mult = matrix[j][i];
                for (unsigned k=i;k<cols;k++) 
                    matrix[j][k] -= mult * matrix[bestRow][k];
                for (unsigned k=0;k<=i;k++)
                    result[j][k] -= mult * result[bestRow][k];
            }
        }
    }
    if (output == NULL)
        output = matrix;
    for (unsigned i=0;i<cols;i++) {
        unsigned cr = rowOrder[i];
        for (unsigned j=0;j<cols;j++)
            output[i][j] = result[cr][j];
    }
    return true;
}


void matMul(calc_type** a, calc_type** b, unsigned a_rows, unsigned common_size, unsigned b_cols, calc_type** output) {
    for (unsigned i=0;i<a_rows;i++)
        for (unsigned j=0;j<b_cols;j++) {
            calc_type sum = 0;
            for (unsigned k=0;k<common_size;k++)
                sum += a[i][k] * b[k][j];
            output[i][j] = sum;
        }
}


// len(resistors) = 3 * cols * rows; order: additional, potentiometer up, potentiometer down
calc_type calculateVoltage(calc_type* resistors, unsigned cols, unsigned rows, unsigned combination) {

    unsigned m = cols * rows;
    unsigned inputCount = cols + rows;
    unsigned mat_rows = 2 * m * (m - 1) + 2 * m + 2;
    unsigned m3 = 3 * m;
    int8_t inputs[inputCount];
    for (unsigned p=0;p<inputCount;p++)
        inputs[p] = (combination & (1 << p)) != 0;

    calc_type** matrix = newMatrix(m3, mat_rows);
    calc_type* vec = new calc_type[mat_rows];

    unsigned rowCounter = 0;

    if (true)
    for (unsigned input_col=0;input_col<cols;input_col++) {
        for (unsigned output_col=input_col;output_col<cols;output_col++) {

            for (unsigned input_row=0;input_row<rows;input_row++) {
                for (unsigned output_row=input_row+1;output_row<rows;output_row++) {
                    
                    unsigned input_resistor_id = 3 * (input_row * cols + input_col);
                    unsigned output_resistor_id = 3 * (output_row * cols + output_col);

                    matrix[rowCounter][input_resistor_id+1] += resistors[input_resistor_id+1];
                    matrix[rowCounter][input_resistor_id] += resistors[input_resistor_id];
                    matrix[rowCounter][output_resistor_id] -= resistors[output_resistor_id];
                    matrix[rowCounter][output_resistor_id+1] -= resistors[output_resistor_id+1];
                    vec[rowCounter++] = inputs[input_col] - inputs[output_col];

                    matrix[rowCounter][input_resistor_id+1] += resistors[input_resistor_id+1];
                    matrix[rowCounter][input_resistor_id] += resistors[input_resistor_id];
                    matrix[rowCounter][output_resistor_id] -= resistors[output_resistor_id];
                    matrix[rowCounter][output_resistor_id+2] -= resistors[output_resistor_id+2];
                    vec[rowCounter++] = inputs[input_col] - inputs[cols + output_row];

                    matrix[rowCounter][input_resistor_id+2] += resistors[input_resistor_id+2];
                    matrix[rowCounter][input_resistor_id] += resistors[input_resistor_id];
                    matrix[rowCounter][output_resistor_id] -= resistors[output_resistor_id];
                    matrix[rowCounter][output_resistor_id+1] -= resistors[output_resistor_id+1];
                    vec[rowCounter++] = inputs[cols + input_row] - inputs[output_col];

                    matrix[rowCounter][input_resistor_id+2] += resistors[input_resistor_id+2];
                    matrix[rowCounter][input_resistor_id] += resistors[input_resistor_id];
                    matrix[rowCounter][output_resistor_id] -= resistors[output_resistor_id];
                    matrix[rowCounter][output_resistor_id+2] -= resistors[output_resistor_id+2];
                    vec[rowCounter++] = inputs[cols + input_row] - inputs[cols + output_row];

                }
            }

        }
    }

    if (false)
    for (unsigned i=0;i<m;i++) {
        unsigned resistorIdA = 3 * i;
        unsigned col_i = i % cols;
        unsigned row_i = i / cols;
        for (unsigned j=i+1;j<m;j++) {
            unsigned resistorIdB = 3 * j;
            unsigned col_j = j % cols;
            unsigned row_j = j / cols;
            // lets assume that current always flows to measurement line

            matrix[rowCounter][resistorIdA+1] += resistors[resistorIdA+1];
            matrix[rowCounter][resistorIdA] += resistors[resistorIdA];
            matrix[rowCounter][resistorIdB] -= resistors[resistorIdB];
            matrix[rowCounter][resistorIdB+1] -= resistors[resistorIdB+1];
            vec[rowCounter++] = inputs[col_i] - inputs[col_j];

            matrix[rowCounter][resistorIdA+1] += resistors[resistorIdA+1];
            matrix[rowCounter][resistorIdA] += resistors[resistorIdA];
            matrix[rowCounter][resistorIdB] -= resistors[resistorIdB];
            matrix[rowCounter][resistorIdB+2] -= resistors[resistorIdB+2];
            vec[rowCounter++] = inputs[col_i] - inputs[row_j];

            matrix[rowCounter][resistorIdA+2] += resistors[resistorIdA+2];
            matrix[rowCounter][resistorIdA] += resistors[resistorIdA];
            matrix[rowCounter][resistorIdB] -= resistors[resistorIdB];
            matrix[rowCounter][resistorIdB+1] -= resistors[resistorIdB+1];
            vec[rowCounter++] = inputs[row_i] - inputs[col_j];

            matrix[rowCounter][resistorIdA+2] += resistors[resistorIdA+2];
            matrix[rowCounter][resistorIdA] += resistors[resistorIdA];
            matrix[rowCounter][resistorIdB] -= resistors[resistorIdB];
            matrix[rowCounter][resistorIdB+2] -= resistors[resistorIdB+2];
            vec[rowCounter++] = inputs[row_i] - inputs[row_j];

        }

        matrix[rowCounter][resistorIdA+1] += resistors[resistorIdA+1];
        matrix[rowCounter][resistorIdA+2] -= resistors[resistorIdA+2];
        vec[rowCounter++] = inputs[col_i] - inputs[row_i];

    }
    //std::cout << " " << rowCounter << std::endl;

    for (unsigned i=0;i<m3;) {
        matrix[rowCounter][i++] = -1;
        matrix[rowCounter][i++] = 1;
        matrix[rowCounter][i++] = 1;
        vec[rowCounter++] = 0;
    }
    for (unsigned i=0;i<m3;i+=3)
        matrix[rowCounter][i] = 1;
    vec[rowCounter++] = 0;
    // for (unsigned i=0;i<m3;i+=3) {
    //     matrix[rowCounter][i+1] = 1;
    //     matrix[rowCounter][i+2] = 1;
    // }
    // vec[rowCounter++] = 0;

    calc_type result = -1;
    bool used[mat_rows];

     printMatrix(matrix, m3, mat_rows);
    if (gauss(matrix, m3, mat_rows, NULL, NULL, used)) {
        calc_type** new_vector = newMatrix(1, m3);
        calc_type** out_vector = newMatrix(1, 2);
        unsigned j=0;
        for (unsigned i=0;i<mat_rows;i++)
            if (used[i]) {
                new_vector[j][0] = vec[j];
                std::cout << vec[j] << ", ";
                j++;
            }
            std::cout << std::endl;
        matMul(matrix, new_vector, 2, m3, 1, out_vector);

        result = calc_type(inputs[0]) - out_vector[0][0] * resistors[0] - out_vector[1][0] * resistors[1];
        

        deleteMatrix(out_vector);
        deleteMatrix(new_vector);
    }
     printMatrix(matrix, m3, mat_rows);

    deleteMatrix(matrix);

    delete [] vec;
    return result;
}


SignalVector calculateVoltage2(calc_type* resistors, unsigned cols, unsigned rows) {

    const unsigned count = cols * rows;

    calc_type col_siemens[count];
    calc_type row_siemens[count];

    unsigned j=0;
    for (unsigned i=0;i<count;i++) {
        const calc_type Ra = resistors[j++];
        const calc_type Rb = resistors[j++];
        const calc_type Rc = resistors[j++];

        calc_type counter = Ra * Rb + Ra * Rc + Rb * Rc;
        col_siemens[i] = Rc / counter;
        row_siemens[i] = Rb / counter;
    }

    SignalVector result;
    result.reserve(1 << count);

    unsigned end = (1<<count)-1;
    for (unsigned it=1;it<end;it++) {
        calc_type high_simens = 0;
        calc_type low_simens = 0;
        for (unsigned i=0;i<count;i++) {
            unsigned col = i % cols;
            unsigned row = i / cols;
            if (it & (1 << col)) high_simens += col_siemens[i]; else low_simens += col_siemens[i];
            if (it & (1 << row << cols)) high_simens += row_siemens[i]; else low_simens += row_siemens[i];
        }
        if (low_simens == 0) { result.push_back(1); continue; }
        if (high_simens == 0) { result.push_back(0); continue; }        
        calc_type high_resistor = 1/high_simens;
        calc_type low_resistor = 1/low_simens;
        result.push_back(1 / (low_simens/high_simens + 1));
    }

    return std::move(result);
}

calc_type calculateDifference(SignalVector dest, calc_type* resistors, unsigned cols, unsigned rows) {
    auto outSignal = calculateVoltage2(resistors, cols, rows);
    unsigned ds = dest.size();
    calc_type result = 0;
    for (unsigned i=0;i<ds;i++) {
        calc_type difference = dest[i] - outSignal[i];
        result += difference * difference;
    }
    return result;
}


calc_type constrain(const calc_type& v, const calc_type& min, const calc_type& max) {
    return (v <= min) ? min : (v >= max) ? max : v;
}

SignalVector calculatePotentiometers(SignalVector dest, unsigned cols, unsigned rows) {
    unsigned m = cols * rows;
    unsigned m3 = 3*m;
    calc_type resistors[3*m];
    calc_type test_vector[3*m];
    calc_type direct_vector[3*m];
    calc_type velocity = 0.25f;
    calc_type value = m3;
    calc_type new_value = value + 1;

    for (unsigned i=0;i<m3;i+=3) 
        resistors[i] = 1;
    for (unsigned i=1;i<m3;i+=3) 
        resistors[i] = 5;
    for (unsigned i=2;i<m3;i+=3) 
        resistors[i] = 5;


    unsigned divider = 0x44fe660d;
    unsigned rand = 1 << 30;
    auto nextRandom = [&]() { 
        rand = (rand << 1)+1; 
        if (rand >= divider) 
            rand -= divider;
        return calc_type((rand % 2048) - 1024)/1024;
    };

    auto randomDirect = [&]() {
        for (unsigned i=0;i<m3;i+=3) {
            direct_vector[i] = 0;
            direct_vector[i+1] = nextRandom();
            direct_vector[i+2] = 1-direct_vector[i+1];
        }
    };

    auto updateTest = [&]() {
        for (unsigned i=0;i<m3;i+=3) {
            test_vector[i] = resistors[i];
            test_vector[i+1] = constrain(resistors[i+1] + direct_vector[i+1] * velocity, 0, 10);
            test_vector[i+2] = 10 - test_vector[i+1];
        }
    };

    const auto saveTest = [&]() {
        for (unsigned i=0;i<m3;i++) 
            resistors[i] = test_vector[i];
    };


    for (unsigned it=0;it<1000&&velocity>1e-3f;it++) {
        if (new_value >= value) {
            randomDirect();
            velocity *= 0.99;
        } else {
            value = new_value;
            saveTest();
            //velocity *= 1.618;
        }
        std::cout << it << " " << value << " " << velocity << std::endl;
        updateTest();
        new_value = calculateDifference(dest, test_vector, cols, rows);
    }

    SignalVector result;
    result.reserve(m);
    // for (unsigned i=0;i<m3;i+=3)
    //     result.push_back(resistors[i+2]/(resistors[i+2] + resistors[i+1]));
    for (unsigned i=0;i<m3;i++)
        result.push_back(resistors[i]);
    return std::move(result);
}


SignalVector gainSignal(
    const PinVector& colPins, const PinVector& rowPins,
    const AnalogAutoReadFunction readFunction,
    const DigitalWriteFunction writeFunction,
    const AutoDelayFunction delayFunction
) {
    unsigned colPinCount = colPins.size();
    unsigned rowPinCount = rowPins.size();
    unsigned m = 1 << (colPinCount + rowPinCount);
    SignalVector signal;
    signal.reserve(m);

    for (unsigned it=1;it<m;it++) {
        unsigned pi = 0;
        bool colPinStates[colPinCount];
        bool rowPinStates[rowPinCount];
        for (unsigned p=0;p<colPinCount;p++) {
            colPinStates[p] = (it & (1 << pi++)) != 0;
            writeFunction(colPins[p], colPinStates[p]);
        }
        for (unsigned p=0;p<rowPinCount;p++) {
            rowPinStates[p] = (it & (1 << pi++)) != 0;
            writeFunction(rowPins[p], rowPinStates[p]);
        }
        delayFunction();
        signal.push_back(readFunction());
    }
    return std::move(signal);
}


void calculate(
    const PinVector& colPins, const PinVector& rowPins,
    const AnalogAutoReadFunction readFunction,
    const DigitalWriteFunction writeFunction,
    const AutoDelayFunction delayFunction,
    const ResultCallback resultCallback
) {

    unsigned colPinCount = colPins.size();
    unsigned rowPinCount = rowPins.size();

    //count of matrices
    unsigned count = rowPinCount * colPinCount;
    unsigned n = 3;
    unsigned m = (1 << (rowPinCount + colPinCount)) - 1;
    calc_type* buf = new calc_type[count*n*m];
    calc_type* bufPtr = buf;

    //array of matrices, row first
    calc_type* matrices[count][m];
    for (unsigned i=0;i<count;i++) {
        for (unsigned j=0;j<m;j++) {
            matrices[i][j] = bufPtr;
            bufPtr += n;
        }
    }

    {
        unsigned k = 0;
        for (unsigned i=0;i<colPinCount;i++)
            for (unsigned j=0;j<rowPinCount;j++)
                matrices[k++][0][0] = 1;
    }

    for (unsigned it=1;it<m;it++) {
        unsigned pi = 0;
        bool colPinStates[colPinCount];
        bool rowPinStates[rowPinCount];
        for (int p=0;p<colPinCount;p++) {
            colPinStates[p] = (it & (1 << pi++)) != 0;
            writeFunction(colPins[p], colPinStates[p]);
        }
        for (int p=0;p<rowPinCount;p++) {
            rowPinStates[p] = (it & (1 << pi++)) != 0;
            writeFunction(rowPins[p], rowPinStates[p]);
        }
        delayFunction();

        unsigned k = 0;
        for (unsigned i=0;i<colPinCount;i++) {
            for (unsigned j=0;j<rowPinCount;j++) {
                calc_type value = readFunction();
                calc_type upValue = colPinStates[i] ? calc_type(1) - value : -value;
                calc_type downValue = rowPinStates[i] ? value - calc_type(1) : value;
                matrices[k][it][0] = downValue - upValue;
                matrices[k][it][1] = upValue;
                matrices[k][it][2] = -downValue;
                k++;
            }
        }
    }

    for (int p=0;p<colPinCount;p++)
        writeFunction(colPins[p], false);
    for (int p=0;p<rowPinCount;p++)
        writeFunction(rowPins[p], false);

    for (unsigned i=0;i<count;i++) {
        gauss(matrices[i], n, m);

        // for (int j=0;j<m;j++) {
        //     for (int k=0;k<n;k++) 
        //         std::cout << matrices[i][j][k] << "\t";
        //     std::cout << std::endl;
        // }
        // std::cout << std::endl;

        resultCallback(i, matrices[i][1][0] / (matrices[i][1][0] + matrices[i][2][0]));
    }

    delete [] buf;
}
