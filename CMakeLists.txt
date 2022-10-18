tdaq_package(NO_HEADERS)

tdaq_use_java()

tdaq_add_jar(oksconfig_jar
  jsrc/oksconfig/*.java
  INCLUDE_JARS config/config.jar Jers/ers.jar TDAQExtJars/external.jar
  OUTPUT_NAME oksconfig
  GENERATE_NATIVE_HEADERS joksconfig_h
)

tdaq_add_jar(jtest_oksconfig
  test/Test.java
  INCLUDE_JARS config/config.jar Jers/ers.jar TDAQExtJars/external.jar oksconfig_jar
  OUTPUT_NAME jtest
  NOINSTALL
)

add_dependencies(jtest_oksconfig JAR_oksconfig_oksconfig)

tdaq_add_library(oksconfig
  src/OksConfiguration.cpp
  src/OksConfigObject.cpp
  LINK_LIBRARIES config oks_utils oks)

tdaq_add_library(roksconfig
  src/ROksConfiguration.cpp
  LINK_LIBRARIES roks oksconfig)
  
tdaq_add_library(joksconfig
  jsrc/oksconfig/*.cpp
  INCLUDE_DIRECTORIES ${JNI_INCLUDE_DIRS}
  LINK_LIBRARIES PRIVATE joksconfig_h oksconfig config oks)

tdaq_add_executable(test_oksdb NOINSTALL
  test/test_oksdb.cpp
  LINK_LIBRARIES config)

tdaq_add_executable(test_create_oksdb NOINSTALL
  test/test_create_oksdb.cpp
  LINK_LIBRARIES config)

add_test(NAME oksconfig_check  COMMAND ${TDAQ_RUNNER} ${CMAKE_CURRENT_SOURCE_DIR}/cmt/test.sh ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
