#include <openvdb/openvdb.h>
#include <openvdb/Grid.h>

int main()
{
    // Initialize the OpenVDB library
    openvdb::initialize();

    // Create a floating-point grid with background value 0.0
    openvdb::FloatGrid::Ptr grid = openvdb::FloatGrid::create();

    // Print a message to verify that OpenVDB has been initialized
    std::cout << "Hello, OpenVDB World!" << std::endl;

    // Access the grid's accessor
    openvdb::FloatGrid::Accessor accessor = grid->getAccessor();

    // Define some coordinates in the grid's index space
    openvdb::Coord xyz(1, 2, 3); // Example coordinate (x, y, z)

    // Set the voxel value at the coordinate to 1.0
    accessor.setValue(xyz, 1.0);

    // Retrieve the voxel value at the coordinate
    float value = accessor.getValue(xyz);

    // Print the voxel value to the console
    std::cout << "The value at coordinate (1, 2, 3) is: " << value << std::endl;

    // Cleanup OpenVDB
    openvdb::uninitialize();

    return 0;
}
