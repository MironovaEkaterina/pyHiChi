#include "TestingUtility.h"
#include "Fdtd.h"
#include "Simulation.h"
#include "DataManager.h"
using namespace pfc;


template <typename Real>
class SimulationTest : public ::testing::Test {
};

typedef ::testing::Types<float, double> types;
TYPED_TEST_CASE(SimulationTest, types);

TYPED_TEST(SimulationTest, Can_create_Simulation_with_Grid_and_FDTD)
{
    Int3 gridSize(11, 5, 6);
    FP3 minCoords = FP3(-1.0, 0.0, 0.0);
    FP3 maxCoords = FP3(1.0, 1.0, 1.0);
    FP3 steps((maxCoords.x - minCoords.x) / gridSize.x,
        (maxCoords.y - minCoords.y) / gridSize.y,
        (maxCoords.z - minCoords.z) / gridSize.z);
    FP timeStep = 1e-15;
    const unsigned int numSteps = 100;

    YeeGrid grid(gridSize, timeStep, minCoords, steps, gridSize);
    FDTD fdtd(&grid);
    Simulation simulation = Simulation(grid, numSteps);
    simulation.addModule(fdtd);
    ASSERT_NO_THROW(simulation.run());
}

TYPED_TEST(SimulationTest, Can_create_DataManager)
{
    ASSERT_NO_THROW(DataManager manager("test"));
}