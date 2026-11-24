cmake_minimum_required(VERSION 3.21)

project(TempControl LANGUAGES C)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/dependencies")
find_package(cppmodel REQUIRED)

enable_testing()

add_executable(SimulateTempControl tempControl.c SimulateTempControl.c)
target_link_libraries(SimulateTempControl PRIVATE cppmodel::cppmodel)
cppmodel_deploy_runtime(SimulateTempControl)

add_test(NAME TempControl COMMAND SimulateTempControl)
