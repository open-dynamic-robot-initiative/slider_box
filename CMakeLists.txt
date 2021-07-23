#
# Copyright (c) 2019, New York University and Max Planck Gesellschaft.
#
# License BSD-3 clause
#

#
# set up the project
#
cmake_minimum_required(VERSION 3.10.2)

project(slider_box)

# Using C++17
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

#
# Dependencies
#

# Depend on ament macros.
find_package(ament_cmake REQUIRED)

# Usual dependencies.
find_package(mpi_cmake_modules REQUIRED)
find_package(real_time_tools REQUIRED)
find_package(rt REQUIRED)
find_package(Threads REQUIRED)

# Export de dependencies.
ament_export_dependencies(real_time_tools)

# Prepare the final export.
set(all_targets)
set(all_target_exports)

#
# Define the include directory of all ${CATKIN_PKGS}.
#

if(Xenomai_FOUND)
  add_definitions(${Xenomai_DEFINITIONS})
endif()

#
# Manage the creation of the library.
#

set(${PROJECT_NAME}_src
    src/serial_reader.cpp
)

# Create the library.
add_library(${PROJECT_NAME} SHARED ${slider_box_src})

# Add the include dependencies.
target_include_directories(
  ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:include>)

if(Xenomai_FOUND)
  target_include_directories(${PROJECT_NAME} PUBLIC ${Xenomai_INCLUDE_DIR})
endif()

# Link the catkin dependencies.
ament_target_dependencies(${PROJECT_NAME} rt)
target_link_libraries(${PROJECT_NAME} real_time_tools::real_time_tools)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

# If on xenomai we need to link to the real time os librairies.
if(Xenomai_FOUND)
  target_link_libraries(${PROJECT_NAME} ${Xenomai_LIBRARY_XENOMAI}
                        ${Xenomai_LIBRARY_NATIVE} ${Xenomai_LIBRARY_RTDM})
endif()

# Export the target.
ament_export_interfaces(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)

# For the installation
list(APPEND all_targets ${PROJECT_NAME})
list(APPEND all_target_exports export_${PROJECT_NAME})


#
# Demos
#

add_executable(demo_basic demos/basic.cpp)

# dependencies
target_include_directories(
  demo_basic PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                        $<INSTALL_INTERFACE:include>)

target_link_libraries(demo_basic ${PROJECT_NAME})

# Save the name for installation
set(all_targets ${all_targets} demo_basic)

#
# Install the package.
#

# Install the script to initialize the can bus
install(DIRECTORY include/ DESTINATION include)

install(
  TARGETS ${all_targets}
  EXPORT ${all_target_exports}
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
  INCLUDES
  DESTINATION include)

#
# Building documentation.
#
add_documentation()

#
# Export as an ament_package.
#
ament_package()
