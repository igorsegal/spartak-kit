cmake_minimum_required(VERSION 3.20)
project(SPARTAK_KIT LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options(/W4 /permissive- /utf-8 /wd4996 /wd4834)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O2)
endif()

add_library(spartak_kit_core STATIC
    src/core/version.cpp
    src/data/BarStream.cpp
    src/data/DataSanitizer.cpp
    src/data/ClusterCsvReader.cpp
    src/cluster/RangeDetector.cpp
    src/cluster/VLevelDetector.cpp
)
target_include_directories(spartak_kit_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(spartak_kit_core PUBLIC cxx_std_20)

add_executable(spartak_kit src/main.cpp)
target_link_libraries(spartak_kit PRIVATE spartak_kit_core)

# --- tests ---
enable_testing()

add_executable(smoke_test tests/smoke_test.cpp)
target_link_libraries(smoke_test PRIVATE spartak_kit_core)
add_test(NAME smoke_test COMMAND smoke_test)

add_executable(test_cluster_csv tests/test_cluster_csv.cpp)
target_link_libraries(test_cluster_csv PRIVATE spartak_kit_core)
add_test(NAME test_cluster_csv COMMAND test_cluster_csv "${CMAKE_CURRENT_SOURCE_DIR}/tests/data/sample.csv")

add_executable(csv_stats tests/csv_stats.cpp)
target_link_libraries(csv_stats PRIVATE spartak_kit_core)
add_executable(test_barstream_file tests/test_barstream_file.cpp)
target_link_libraries(test_barstream_file PRIVATE spartak_kit_core)
add_test(NAME test_barstream_file COMMAND test_barstream_file "${CMAKE_CURRENT_SOURCE_DIR}/tests/data/sample.csv")

add_executable(test_range_detector tests/test_range_detector.cpp)
target_link_libraries(test_range_detector PRIVATE spartak_kit_core)
add_test(NAME test_range_detector COMMAND test_range_detector)

add_executable(test_vlevel_detector tests/test_vlevel_detector.cpp)
target_link_libraries(test_vlevel_detector PRIVATE spartak_kit_core)
add_test(NAME test_vlevel_detector COMMAND test_vlevel_detector)
