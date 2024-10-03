#include <openvdb/openvdb.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

// Function to read a matrix from a .mtx file and return it as a vector of triplets (row, col, value)
vector<tuple<int, int, double>> readMatrixFromFile(const string &filename, int &rows, int &cols)
{
    vector<tuple<int, int, double>> matrixData;

    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Error: Unable to open file " << filename << endl;
        exit(1);
    }

    string line;
    bool isHeader = true;

    while (getline(file, line))
    {
        if (line[0] == '%' || line.empty())
        {
            // Skip comment lines or empty lines
            continue;
        }

        if (isHeader)
        {
            // Read the matrix size from the header (rows, cols, non-zeros)
            isHeader = false;
            stringstream ss(line);
            int nonZeroElements;
            ss >> rows >> cols >> nonZeroElements; // Read the dimensions and the number of non-zero elements
        }
        else
        {
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

// Function to print the first 30 rows of the matrix data
void printFirst30Rows(const vector<tuple<int, int, double>> &matrixData)
{
    int rowCount = 0;

    cout << "First 30 rows of the matrix:" << endl;
    for (const auto &entry : matrixData)
    {
        int row, col;
        double value;
        tie(row, col, value) = entry;

        if (rowCount >= 30)
        {
            break; // Stop after printing 30 rows
        }

        cout << "Row: " << row + 1 << ", Col: " << col + 1 << ", Value: " << value << endl;
        rowCount++;
    }
}

int main()
{
    // Initialize OpenVDB library
    openvdb::initialize();

    // Set matrix dimensions
    int rowsA = 0, colsA = 0;
    int rowsB = 0, colsB = 0;

    // Read the matrices from files
    vector<tuple<int, int, double>> matrixA = readMatrixFromFile("/home/hp/Desktop/project/subodh_data/P.mtx", rowsA, colsA);
    vector<tuple<int, int, double>> matrixB = readMatrixFromFile("/home/hp/Desktop/project/subodh_data/S.mtx", rowsB, colsB);

    // Print the first 30 rows of matrix A
    cout << "Matrix A:" << endl;
    printFirst30Rows(matrixA);

    // Print the first 30 rows of matrix B
    cout << "Matrix B:" << endl;
    printFirst30Rows(matrixB);

    return 0;
}
