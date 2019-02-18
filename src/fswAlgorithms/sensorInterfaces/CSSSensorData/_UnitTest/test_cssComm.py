#
#   Unit Test Script
#   Module Name:        cssComm
#   Creation Date:      October 4, 2018
#

from Basilisk.utilities import SimulationBaseClass
from Basilisk.utilities import unitTestSupport  # general support file with common unit test functions
from Basilisk.utilities import macros
from Basilisk.fswAlgorithms import cssComm
from Basilisk.simulation import simFswInterfaceMessages


def test_cssComm():
    [testResults, testMessage] = cssCommTestFunction()
    assert testResults < 1, testMessage

def cssCommTestFunction():
    """ Test the cssComm module """
    testFailCount = 0  # zero unit test result counter
    testMessages = []  # create empty array to store test log messages
    unitTaskName = "unitTask"  # arbitrary name (don't change)
    unitProcessName = "TestProcess"  # arbitrary name (don't change)

    # Create a sim module as an empty container
    unitTestSim = SimulationBaseClass.SimBaseClass()

    # This is needed if multiple unit test scripts are run
    # This create a fresh and consistent simulation environment for each test run
    unitTestSim.TotalSim.terminateSimulation()

    # Create test thread
    testProcessRate = macros.sec2nano(0.5)  # update process rate update time
    testProc = unitTestSim.CreateNewProcess(unitProcessName)
    testProc.addTask(unitTestSim.CreateNewTask(unitTaskName, testProcessRate)) # Add a new task to the process

    # Construct the cssComm module
    moduleConfig = cssComm.CSSConfigData() # Create a config struct
    # Populate the config
    moduleConfig.NumSensors = 8
    moduleConfig.MaxSensorValue = 500e-6
    moduleConfig.OutputDataName = "css_data_aggregate"
    ChebyList =  [-1.734963346951471e+06, 3.294117146099591e+06,
                     -2.816333294617512e+06, 2.163709942144332e+06,
                     -1.488025993860025e+06, 9.107359382775769e+05,
                     -4.919712500291216e+05, 2.318436583511218e+05,
                     -9.376105045529010e+04, 3.177536873430168e+04,
                     -8.704033370738143e+03, 1.816188108176300e+03,
                     -2.581556805090373e+02, 1.888418924282780e+01]
    moduleConfig.ChebyCount = len(ChebyList)
    moduleConfig.KellyCheby = ChebyList
    moduleConfig.SensorListName = "css_sensors_data_pass"

    moduleWrap = unitTestSim.setModelDataWrap(moduleConfig) # This calls the algContain to setup the selfInit, crossInit, and update
    moduleWrap.ModelTag = "cssComm"

    # Add the module to the task
    unitTestSim.AddModelToTask(unitTaskName, moduleWrap, moduleConfig)

    # The cssComm module reads in from the sensor list, so create that message here
    cssArrayMsg = simFswInterfaceMessages.CSSArraySensorIntMsg()

    # NOTE: This is nonsense. These are more or less random numbers
    cssArrayMsg.CosValue = [100e-6, 200e-6, 100e-6, 300e-6, 400e-6, 300e-6, 200e-6, 600e-6]
    # Write this message
    msgSize = cssArrayMsg.getStructSize()
    unitTestSim.TotalSim.CreateNewMessage(unitProcessName,
                                          moduleConfig.SensorListName,
                                          msgSize,
                                          2)
    unitTestSim.TotalSim.WriteMessageData(moduleConfig.SensorListName,
                                          msgSize,
                                          0,
                                          cssArrayMsg)

    # Log the output message
    unitTestSim.TotalSim.logThisMessage(moduleConfig.OutputDataName, testProcessRate)

    # Initialize the simulation
    unitTestSim.InitializeSimulation()

    unitTestSim.ConfigureStopTime(testProcessRate)
    unitTestSim.ExecuteSimulation()

    # Get the output from this simulation
    outputData = unitTestSim.pullMessageLogData(moduleConfig.OutputDataName+".CosValue", range(moduleConfig.NumSensors))

    # Create the true array
    trueCss = [
        [0.317205386467, 0.45791653042, 0.317205386467, 0.615444781018, 0.802325127473, 0.615444781018, 0.45791653042, 0.0],
        [0.317205386467, 0.45791653042, 0.317205386467, 0.615444781018, 0.802325127473, 0.615444781018, 0.45791653042, 0.0]
    ]

    accuracy = 1e-6

    testFailCount, testMessages = unitTestSupport.compareArrayND(trueCss, outputData, accuracy, "ccsComm",
                                                                 moduleConfig.NumSensors, testFailCount, testMessages)

    if testFailCount == 0:
        print "PASSED: " + moduleWrap.ModelTag

    return [testFailCount, ''.join(testMessages)]


if __name__ == '__main__':
    test_cssComm()