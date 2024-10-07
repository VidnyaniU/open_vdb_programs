#include <openvdb/openvdb.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

// Function to read a matrix from a .mtx file and return only a 20x20 submatrix
vector<tuple<int, int, double>> readSubMatrixFromFile(const string& filename, int& rows, int& cols)
{
    vector<tuple<int, int, double>> subMatrixData;

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

            // Only keep entries within the 20x20 submatrix (i.e., row < 20, col < 20)
            if (row < 20 && col < 20) {
                subMatrixData.emplace_back(row, col, value);
            }
        }
    }

    file.close();
    return subMatrixData;
}

// Function to save the 20x20 submatrix into a new .mtx file
void saveSubMatrixToFile(const vector<tuple<int, int, double>>& subMatrixData, const string& outputFilename)
{
    ofstream outfile(outputFilename);
    if (!outfile.is_open()) {
        cerr << "Error: Unable to open file " << outputFilename << endl;
        exit(1);
    }

    // Write the Matrix Market header
    outfile << "%%MatrixMarket matrix coordinate real general" << endl;
    outfile << "% Generated 20x20 submatrix" << endl;

    // Write the dimensions and number of non-zero entries
    outfile << "20 20 " << subMatrixData.size() << endl;  // 20x20 matrix, number of non-zero entries

    // Write the non-zero matrix entries
    for (const auto& entry : subMatrixData) {
        int row, col;
        double value;
        tie(row, col, value) = entry;
        
        // Matrix Market format uses 1-based indices, so convert back to 1-based
        outfile << row + 1 << " " << col + 1 << " " << value << endl;
    }

    outfile.close();
    cout << "Submatrix saved to " << outputFilename << endl;
}

int main()
{
    // Initialize OpenVDB library
    openvdb::initialize();

    // Set matrix dimensions (will be read from the file header)
    int rowsA = 0, colsA = 0;

    // Read the 20x20 submatrix from file
    vector<tuple<int, int, double>> subMatrixP = readSubMatrixFromFile("/home/hp/Desktop/project/subodh_data/P.mtx", rowsA, colsA);

    // Save the submatrix to a new .mtx file
    saveSubMatrixToFile(subMatrixP, "submatrix_P_20x20.mtx");

     // Read the 20x20 submatrix from file
    vector<tuple<int, int, double>> subMatrixS = readSubMatrixFromFile("/home/hp/Desktop/project/subodh_data/S.mtx", rowsA, colsA);

    // Save the submatrix to a new .mtx file
    saveSubMatrixToFile(subMatrixS, "submatrix_S_20x20.mtx");

    return 0;
}
