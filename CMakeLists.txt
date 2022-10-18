cmake_minimum_required(VERSION 3.12)
project(config VERSION 0.0.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(Boost COMPONENTS program_options REQUIRED)
find_package(ers REQUIRED)
find_package(logging REQUIRED)
find_package(config REQUIRED)
find_package(oks REQUIRED)
find_package(oks_utils REQUIRED)

daq_add_library(oksconfig OksConfiguration.cpp OksConfigObject.cpp LINK_LIBRARIES config::config oks_utils::oks_utils oks::oks)

# JCF, Oct-18-2022: not yet able to compile roks library from oks_utils since it requires CORAL
# tdaq_add_library(roksconfig src/ROksConfiguration.cpp LINK_LIBRARIES roks oksconfig)

daq_add_application(test_oksdb test_oksdb.cpp TEST LINK_LIBRARIES config::config)
daq_add_application(test_create_oksdb test_create_oksdb.cpp LINK_LIBRARIES config::config)

daq_install()
