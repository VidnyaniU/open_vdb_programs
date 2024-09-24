#include <openvdb/openvdb.h>
#include <iostream>
#include <random>
#include <unordered_set>
#include <sys/resource.h>
#include <sys/time.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

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
            float valueA = accessorA.getValue(coordA);

            if (valueA != 0.0f) // Proceed if A[i,k] is non-zero
            {
                for (int j = 0; j < cols; ++j)
                {
                    // Get the value from B[k,j]
                    openvdb::Coord coordB(k, j, 0);
                    float valueB = accessorB.getValue(coordB);

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
float calculateTrace(openvdb::FloatGrid::Ptr grid, int rows)
{
    openvdb::FloatGrid::Accessor accessor = grid->getAccessor();
    float trace = 0.0f;

    for (int i = 0; i < rows; ++i)
    {
        // Access the diagonal element C[i,i]
        openvdb::Coord coord(i, i, 0);
        trace += accessor.getValue(coord); // Sum up the diagonal elements
    }

    return trace; // Return the trace
}

void fun(int rows, int cols)
{
    // Create two OpenVDB FloatGrids for the matrices
    openvdb::FloatGrid::Ptr A = openvdb::FloatGrid::create();
    openvdb::FloatGrid::Ptr B = openvdb::FloatGrid::create();
    openvdb::FloatGrid::Accessor accessorA = A->getAccessor();
    openvdb::FloatGrid::Accessor accessorB = B->getAccessor();

    // Random number generators
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis_diag(0.0, 1.0);
    std::uniform_real_distribution<> dis_non_diag(-0.02, 0.02);
    std::uniform_int_distribution<> col_selector(0, cols - 1);

    auto start1 = high_resolution_clock::now();

    // Populate both grids A and B with sparse data
    for (int i = 0; i < rows; ++i)
    {
        unordered_set<int> non_zero_columns;

        // Ensure diagonal element is non-zero
        non_zero_columns.insert(i);

        // Select 9 random non-diagonal columns
        while (non_zero_columns.size() < 10)
        {
            int col = col_selector(gen);
            if (col != i)
            {
                non_zero_columns.insert(col);
            }
        }

        // Assign values to A and B
        for (int j : non_zero_columns)
        {
            float valueA, valueB;
            if (i == j)
            {
                valueA = static_cast<float>(dis_diag(gen));
                valueB = static_cast<float>(dis_diag(gen));
            }
            else
            {
                valueA = static_cast<float>(dis_non_diag(gen));
                valueB = static_cast<float>(dis_non_diag(gen));
            }

            // Set values in A and B
            accessorA.setValue(openvdb::Coord(i, j, 0), valueA);
            accessorB.setValue(openvdb::Coord(i, j, 0), valueB);
        }
    }
    cout << endl
         << "For " << rows << " by " << rows << endl;

    for (int i = 0; i < 10; i++)
    {

        cout << endl
             << "Iteration " << i << "for dimension " << rows << endl;
        auto stop1 = high_resolution_clock::now();

        auto duration1 = duration_cast<seconds>(stop1 - start1);

        cout << i << "Time taken for matrix creation :: " << duration1.count() << "s" << endl;

        auto start2 = high_resolution_clock::now();
        // Multiply matrices A and B
        openvdb::FloatGrid::Ptr result = multiplyMatrices(A, B, rows, cols);

        auto stop2 = high_resolution_clock::now();

        auto duration2 = duration_cast<seconds>(stop2 - start2);

        cout << "Time taken for matrix multiplication :: " << duration2.count() << "s" << endl;

        // Calculate the trace of the result matrix
        float trace = calculateTrace(result, rows);

        // Print the trace
        std::cout << "Trace of the result matrix :: " << trace << std::endl;
    }
}

int main()
{
    // Initialize OpenVDB library
    openvdb::initialize();

    // Set matrix size
    int rows = 1000;
    int cols = 1000;

    for (int i = 0; i < 10; i++)
    {
        // cout << "\n Mat.row :: " << rows << "\t Mat.cols :: " << cols << endl;
        cout.flush();
        fun(rows, cols);

        rows += 1000;
        cols += 1000;
    }

    return 0;
}
