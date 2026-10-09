cmake_minimum_required(VERSION 3.20)
project(spartak_kit CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive- /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# --- Основная библиотека ---
add_library(spartak_kit_core STATIC
    src/data/BarStream.cpp
    src/data/DataSanitizer.cpp
    src/data/ClusterCsvReader.cpp
    src/cluster/RangeDetector.cpp
    src/cluster/VLevelDetector.cpp
    src/cluster/MirrorLevelDetector.cpp
    src/cluster/DeltaDetector.cpp
    src/cluster/DivergenceDetector.cpp
    src/cluster/FalseBreakoutDetector.cpp
    src/cluster/VolumeProfileFilter.cpp
    src/cluster/CascadeLevelDetector.cpp
    src/cluster/StopHuntDetector.cpp
    src/data/XfbarReader.cpp
)

target_include_directories(spartak_kit_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# --- Тесты ---
enable_testing()

add_executable(smoke_test tests/smoke_test.cpp)
target_link_libraries(smoke_test PRIVATE spartak_kit_core)
add_test(NAME smoke_test COMMAND smoke_test)

add_executable(test_cluster_csv tests/test_cluster_csv.cpp)
target_link_libraries(test_cluster_csv PRIVATE spartak_kit_core)
add_test(NAME test_cluster_csv COMMAND test_cluster_csv)

add_executable(test_barstream_file tests/test_barstream_file.cpp)
target_link_libraries(test_barstream_file PRIVATE spartak_kit_core)
add_test(NAME test_barstream_file COMMAND test_barstream_file)

add_executable(test_range_detector tests/test_range_detector.cpp)
target_link_libraries(test_range_detector PRIVATE spartak_kit_core)
add_test(NAME test_range_detector COMMAND test_range_detector)

add_executable(test_vlevel_detector tests/test_vlevel_detector.cpp)
target_link_libraries(test_vlevel_detector PRIVATE spartak_kit_core)
add_test(NAME test_vlevel_detector COMMAND test_vlevel_detector)

add_executable(test_mirror_detector tests/test_mirror_detector.cpp)
target_link_libraries(test_mirror_detector PRIVATE spartak_kit_core)
add_test(NAME test_mirror_detector COMMAND test_mirror_detector)

add_executable(test_delta_detector tests/test_delta_detector.cpp)
target_link_libraries(test_delta_detector PRIVATE spartak_kit_core)
add_test(NAME test_delta_detector COMMAND test_delta_detector)

add_executable(test_divergence_detector tests/test_divergence_detector.cpp)
target_link_libraries(test_divergence_detector PRIVATE spartak_kit_core)
add_test(NAME test_divergence_detector COMMAND test_divergence_detector)

add_executable(test_false_breakout_detector tests/test_false_breakout_detector.cpp)
target_link_libraries(test_false_breakout_detector PRIVATE spartak_kit_core)
add_test(NAME test_false_breakout_detector COMMAND test_false_breakout_detector)

add_executable(debug_mirror tests/debug_mirror.cpp)
target_link_libraries(debug_mirror PRIVATE spartak_kit_core)

add_executable(csv_stats tests/csv_stats.cpp)
target_link_libraries(csv_stats PRIVATE spartak_kit_core)

add_executable(test_volume_profile tests/test_volume_profile.cpp)
target_link_libraries(test_volume_profile PRIVATE spartak_kit_core)
add_test(NAME test_volume_profile COMMAND test_volume_profile)

add_executable(test_cascade_level tests/test_cascade_level.cpp)
target_link_libraries(test_cascade_level PRIVATE spartak_kit_core)
add_test(NAME test_cascade_level COMMAND test_cascade_level)

add_executable(test_stop_hunt tests/test_stop_hunt.cpp)
target_link_libraries(test_stop_hunt PRIVATE spartak_kit_core)
add_test(NAME test_stop_hunt COMMAND test_stop_hunt)

add_executable(crash_test tests/crash_test.cpp)
target_link_libraries(crash_test PRIVATE spartak_kit_core)