//verification PSP == 2P

#include <openvdb/openvdb.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <tuple>
#include <cmath>  // for fabs()

using namespace std;

// Function to read a matrix from a .mtx file and return it as a vector of triplets (row, col, value)
vector<tuple<int, int, double>> readMatrixFromFile(const string& filename, int& rows, int& cols)
{
    vector<tuple<int, int, double>> matrixData;

    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Unable to open file " << filename << endl;
        exit(1);
    }

    string line;
    bool isHeader = true;

    while (getline(file, line)) {
        if (line[0] == '%' || line.empty()) {
            // Skip comment lines or empty lines
            continue;
        }

        if (isHeader) {
            // Read the matrix size from the header (rows, cols, non-zeros)
            isHeader = false;
            stringstream ss(line);
            int nonZeroElements;
            ss >> rows >> cols >> nonZeroElements;  // Read the dimensions and the number of non-zero elements
        } else {
            // Read the matrix data (row, col, value)
            stringstream ss(line);
            int row, col;
            double value;
            ss >> row >> col >> value;

            // MatrixMarket is 1-based, convert to 0-based
            row--;
            col--;

            // Store the triplet (row, col, value)
            matrixData.emplace_back(row, col, value);
        }
    }

    file.close();
    return matrixData;
}

// Function to multiply two sparse matrices and return the result
vector<tuple<int, int, double>> multiplyMatrices(const vector<tuple<int, int, double>>& A, int rowsA, int colsA,
                                                const vector<tuple<int, int, double>>& B, int rowsB, int colsB)
{
    vector<tuple<int, int, double>> result;

    // Create a sparse matrix for the result, assuming it's initialized to zero
    // Here, you can use a more efficient method with hash maps if the data is large

    for (const auto& [rowA, colA, valueA] : A) {
        for (const auto& [rowB, colB, valueB] : B) {
            if (colA == rowB) {  // Check for matching indices for matrix multiplication
                result.emplace_back(rowA, colB, valueA * valueB);
            }
        }
    }

    return result;
}

// Function to multiply a matrix by a scalar and return the result
vector<tuple<int, int, double>> scalarMultiplyMatrix(const vector<tuple<int, int, double>>& A, double scalar)
{
    vector<tuple<int, int, double>> result;

    for (const auto& [row, col, value] : A) {
        result.emplace_back(row, col, value * scalar);
    }

    return result;
}

// Function to compare two matrices to check if they are equal (within a tolerance for doubleing-point comparisons)
bool compareMatrices(const vector<tuple<int, int, double>>& A, const vector<tuple<int, int, double>>& B, double tolerance = 1e-6)
{
    if (A.size() != B.size()) {
        return false;  // Matrices must have the same number of non-zero elements
    }

    for (size_t i = 0; i < A.size(); ++i) {
        int rowA, colA, rowB, colB;
        double valueA, valueB;
        tie(rowA, colA, valueA) = A[i];
        tie(rowB, colB, valueB) = B[i];

        if (rowA != rowB || colA != colB || fabs(valueA - valueB) > tolerance) {
            return false;  // Matrices differ either in index or value
        }
    }

    return true;
}

int main()
{
    // Initialize OpenVDB library
    openvdb::initialize();

    // Matrix dimensions
    int rowsP = 0, colsP = 0;
    int rowsS = 0, colsS = 0;

    // Read matrices P and S from files
    vector<tuple<int, int, double>> matrixP = readMatrixFromFile("/home/hp/GitHub/open_vdb_programs/submatrix_P_20x20.mtx", rowsP, colsP);
    vector<tuple<int, int, double>> matrixS = readMatrixFromFile("/home/hp/GitHub/open_vdb_programs/submatrix_S_20x20.mtx", rowsS, colsS);

    // Step 1: Multiply P * S
    vector<tuple<int, int, double>> PS = multiplyMatrices(matrixP, rowsP, colsP, matrixS, rowsS, colsS);

    // Step 2: Multiply (P * S) * P
    vector<tuple<int, int, double>> PSP = multiplyMatrices(PS, rowsP, colsS, matrixP, rowsP, colsP);

    // Step 3: Compute 2 * P
    vector<tuple<int, int, double>> doubleP = scalarMultiplyMatrix(matrixP, 2.0f);

    // Step 4: Compare PSP with 2 * P
    bool areEqual = compareMatrices(PSP, doubleP);

    if (areEqual) {
        cout << "PSP is equal to 2P!" << endl;
    } else {
        cout << "PSP is NOT equal to 2P." << endl;
    }

    return 0;
}
