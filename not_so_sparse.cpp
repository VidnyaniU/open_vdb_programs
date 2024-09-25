#include <openvdb/openvdb.h>
#include <iostream>
#include <random>
#include <unordered_set>
#include <sys/resource.h>
#include <sys/time.h>

using namespace std;

void fun(int rows, int cols)
{
    // Create an OpenVDB FloatGrid
    openvdb::FloatGrid::Ptr grid = openvdb::FloatGrid::create();
    openvdb::FloatGrid::Accessor accessor = grid->getAccessor();

    // Define diagonal width for the sparse region (5% of matrix size)
    int diagonalWidth = static_cast<int>(0.05 * std::max(rows, cols));

    // Random number generators
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis_diag(0.0, 1.0);      // For diagonal elements
    std::uniform_real_distribution<> dis_non_diag(-0.02, 0.02); // For non-diagonal elements
    std::uniform_int_distribution<> col_selector(0, cols - 1);  // For random column selection

    ulong noe = 0;

    // Populate the grid
    for (int i = 0; i < rows; ++i)
    {
        // Set for storing unique random column indices
        unordered_set<int> non_zero_columns;

        // Ensure the diagonal element is non-zero
        non_zero_columns.insert(i);

        // Select 9 random non-diagonal columns for the current row
        while (non_zero_columns.size() < 10)
        {
            int col = col_selector(gen);
            if (col != i)  // Ensure it's not the diagonal element
            {
                non_zero_columns.insert(col);
            }
        }

        // Iterate through the chosen columns and assign values
        for (int j : non_zero_columns)
        {
            ++noe;
            float value;
            if (i == j)
            {
                // Diagonal element: value between 0 and 1
                value = static_cast<float>(dis_diag(gen));
            }
            else
            {
                // Non-diagonal element: value between -0.02 and 0.02
                value = static_cast<float>(dis_non_diag(gen));
            }

            // Create a Coord object
            openvdb::Coord coord(i, j, 0);
            // Set the value in the grid
            accessor.setValue(coord, value);
        }
    }

    // Measure memory usage
    double percentage = (noe / (rows * cols)) * 100;
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0)
    {
        std::cout << "\t noe :: " << noe << "\t non-zero (%) :: " << percentage
                  << "\t Memory usage: " << usage.ru_maxrss / 1024 << " Mega Bytes" << std::endl;
    }
    else
    {
        std::cerr << "Error retrieving resource usage" << std::endl;
    }
}

int main()
{
    // Initialize OpenVDB library
    openvdb::initialize();

    // Set matrix size
    int rows = 10000;
    int cols = 10000;

    for (int i = 0; i < 100; i++)
    {
        cout << "\n Mat.row :: " << rows << "\t Mat.cols :: " << cols;
        cout.flush();
        fun(rows, cols);

        rows += 10000;
        cols += 10000;
    }

    return 0;
}
