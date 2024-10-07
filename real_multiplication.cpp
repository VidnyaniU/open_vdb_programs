#include <openvdb/openvdb.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

// Function to read a matrix from a .mtx file and store it in an OpenVDB grid
openvdb::FloatGrid::Ptr readMatrixFromFile(const string &filename, int &rows, int &cols)
{
    openvdb::FloatGrid::Ptr grid = openvdb::FloatGrid::create();
    openvdb::FloatGrid::Accessor accessor = grid->getAccessor();

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
            float value;
            ss >> row >> col >> value;

            // MatrixMarket is 1-based, convert to 0-based
            row--;
            col--;

            // Set the value in the grid
            accessor.setValue(openvdb::Coord(row, col, 0), value);
        }
    }

    file.close();
    return grid;
}

// Function to multiply two sparse matrices and return the result as an OpenVDB grid
openvdb::FloatGrid::Ptr multiplyMatrices(openvdb::FloatGrid::Ptr A, openvdb::FloatGrid::Ptr B, int rows, int cols)
{
    openvdb::FloatGrid::Ptr result = openvdb::FloatGrid::create(); // Create the result grid
    openvdb::FloatGrid::Accessor accessorA = A->getAccessor();
    openvdb::FloatGrid::Accessor accessorB = B->getAccessor();
    openvdb::FloatGrid::Accessor accessorC = result->getAccessor();

    // Multiply A and B
    for (int i = 0; i < rows; ++i)
    {
        for (int k = 0; k < cols; ++k)
        {
            // Get the value from A[i,k]
            openvdb::Coord coordA(i, k, 0);
            double valueA = accessorA.getValue(coordA);

            if (valueA != 0.0f) // Proceed if A[i,k] is non-zero
            {
                for (int j = 0; j < cols; ++j)
                {
                    // Get the value from B[k,j]
                    openvdb::Coord coordB(k, j, 0);
                    double valueB = accessorB.getValue(coordB);

                    if (valueB != 0.0f) // Proceed if B[k,j] is non-zero
                    {
                        // Calculate the contribution to C[i,j]
                        openvdb::Coord coordC(i, j, 0);
                        accessorC.setValue(coordC, accessorC.getValue(coordC) + (valueA * valueB));
                    }
                }
            }
        }
    }

    return result; // Return the result grid
}

// Function to calculate the trace of a matrix stored in an OpenVDB grid
double calculateTrace(openvdb::FloatGrid::Ptr grid, int rows)
{
    openvdb::FloatGrid::Accessor accessor = grid->getAccessor();
    double trace = 0.0f;

    for (int i = 0; i < rows; ++i)
    {
        // Access the diagonal element C[i,i]
        openvdb::Coord coord(i, i, 0);
        trace += accessor.getValue(coord); // Sum up the diagonal elements
    }

    return trace; // Return the trace
}

int main()
{
    openvdb::initialize();

    // Set matrix dimensions
    int rowsA = 0, colsA = 0;
    int rowsB = 0, colsB = 0;

    // Read the matrices from files
    openvdb::FloatGrid::Ptr A = readMatrixFromFile("/home/hp/GitHub/open_vdb_programs/submatrix_P_20x20.mtx", rowsA, colsA);
    openvdb::FloatGrid::Ptr B = readMatrixFromFile("/home/hp/GitHub/open_vdb_programs/submatrix_P_20x20.mtx", rowsB, colsB);

    // Ensure matrix dimensions are compatible for multiplication
    if (colsA != rowsB)
    {
        cerr << "Error: Matrix dimensions are not compatible for multiplication!" << endl;
        return 1;
    }

    // Multiply matrices A and B
    openvdb::FloatGrid::Ptr result = multiplyMatrices(A, B, rowsA, colsB);

    // Calculate the trace of the result matrix
    double trace = calculateTrace(result, rowsA);

    // Print the trace
    cout << "Trace of the result matrix: " << trace << endl;

    return 0;
}
