# jdu na to
#cmake_minimum_required (VERSION 3.5.1)
cmake_minimum_required (VERSION 3.0.2)
project (binary_xml)

#-----------------------------------------------------------------------------------------------
set(TARGET_DEVICE "default" CACHE STRING "device we are building for")
set_property(CACHE TARGET_DEVICE PROPERTY STRINGS default pc)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC")
find_package (OpenSSL)  #sudo apt-get install libssl-dev
find_package (LibXml2)
#include_directories(${OPENSSL_INCLUDE_DIRS})
#--------------------------------------------------------------------------------------------------



set_property(GLOBAL PROPERTY CXX_STANDARD 11)


file(MAKE_DIRECTORY build)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "build/${TARGET_DEVICE}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

include_directories(${LIBXML2_INCLUDE_DIR})
include_directories(${CMAKE_BINARY_DIR})
include_directories(/usr/include)

file(GLOB CPP_FILES src/*.cpp)
#message("Found: ${CPP_FILES}")
#message("openssl: ${OPENSSL_INCLUDE_DIR}")
#message("libxml2: ${LIBXML2_INCLUDE_DIR}")
#message("binary_xml: ${CPP_FILES}")

add_library(binary_xml SHARED ${CPP_FILES} src/jsmn/jsmn.c)
    target_include_directories(binary_xml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(binary_xml ${LIBXML2_LIBRARIES} ${LIBZ_LIBRARIES})

add_library(binary_xml_static ${CPP_FILES} src/jsmn/jsmn.c)
    target_include_directories(binary_xml_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(binary_xml ${LIBXML2_LIBRARIES} ${LIBZ_LIBRARIES})



#--------------------------------------------------------------------------------------------------
#enable_testing()

file(GLOB XML_SOURCES src/tests/data/*.xml)
file(GLOB XB_SOURCES src/tests/data/*.xb)
file(GLOB JSON_SOURCES src/tests/data/*.json)

add_executable(2xb src/tests/2xb.cpp)
    target_include_directories(2xb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(2xb binary_xml)
#   add_test(2xb-1 COMMAND 2xb t0.xml )
#   add_test(2xb-2 COMMAND 2xb t1.xml )
#   add_test(2xb-3 COMMAND 2xb t1.json )

add_executable(xb2xml src/tests/xb2xml.cpp)
    target_include_directories(xb2xml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(xb2xml binary_xml)
    add_test(xb2xml xb2xml ${XB_SOURCES} )
    
add_executable(json-test src/tests/json-test.cpp)
    target_include_directories(json-test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_include_directories(json-test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/jsmn)
    target_link_libraries(json-test binary_xml)

#--------------------------------------------------------------------------------------------------
add_custom_target(tests COMMAND ctest .)

add_custom_target(tags  COMMAND ctags -R -f .tags . /usr/include)
add_custom_target(run   
    COMMAND 2xb ${XML_SOURCES}
    COMMAND test -f ${XB_RESULT} && hexdump -C ${XB_RESULT})
add_custom_target(dbg   COMMAND gdb xml2xbin)


install(TARGETS 2xb xb2xml binary_xml binary_xml_static
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)