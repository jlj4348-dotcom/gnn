cmake_minimum_required(VERSION 3.20)

project(gnn
    VERSION 0.1.0
    DESCRIPTION "Graph based neural network design/training framework"
    LANGUAGES CXX)

# ---------------------------------------------------------------------------
# 기본 설정
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 단일 구성 generator 에서 build type 을 지정하지 않으면 Debug 로 둔다.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

option(GNN_BUILD_TESTS "Build tests (doctest)" ON)

# 공개 헤더 루트. 코드에서는 #include "gnn/<module>/<File>.hpp" 로 쓴다.
set(GNN_IMPORT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/import)
set(GNN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/gnn)

# 경고 옵션. 코어는 표준 C++17 만 사용하므로 컴파일러별 옵션은 여기에서만 다룬다.
function(gnn_set_warnings target scope)
    if(MSVC)
        # /utf-8: 소스와 실행 문자 집합을 UTF-8 로 (주석/문자열에 한국어를 쓰므로 필요하다)
        target_compile_options(${target} ${scope} /W4 /permissive- /EHsc /utf-8 /external:W0)
    else()
        target_compile_options(${target} ${scope} -Wall -Wextra -Wpedantic)
    endif()
endfunction()

# ---------------------------------------------------------------------------
# module 정의
#   gnn_add_module(<name> [DEPS <module>...])
#   - import/gnn/<name>/ 은 공개 헤더, src/gnn/<name>/ 은 구현이다.
#   - 구현(.cpp)이 있으면 STATIC library, 없으면 헤더만 있는 INTERFACE library 로 만든다.
#   - target 이름은 gnn_<name>, alias 는 gnn::<name> 이다.
# ---------------------------------------------------------------------------
function(gnn_add_module name)
    cmake_parse_arguments(ARG "" "" "DEPS" ${ARGN})

    file(GLOB_RECURSE sources CONFIGURE_DEPENDS
        ${GNN_SOURCE_DIR}/${name}/*.cpp)

    set(target gnn_${name})

    if(sources)
        add_library(${target} STATIC ${sources})
        target_include_directories(${target} PUBLIC ${GNN_IMPORT_DIR})
        gnn_set_warnings(${target} PRIVATE)
        set(scope PUBLIC)
    else()
        add_library(${target} INTERFACE)
        target_include_directories(${target} INTERFACE ${GNN_IMPORT_DIR})
        set(scope INTERFACE)
    endif()

    foreach(dep IN LISTS ARG_DEPS)
        target_link_libraries(${target} ${scope} gnn_${dep})
    endforeach()

    add_library(gnn::${name} ALIAS ${target})
endfunction()

# 의존 방향: common <- tensor <- graph 들 <- trainer <- cli
# (storage 는 graph 를 읽고 쓰므로 graph 들에 의존하고, trainer 의 checkpoint 가 storage 를 사용한다)
gnn_add_module(common)
gnn_add_module(tensor         DEPS common)
gnn_add_module(designGraph    DEPS common)
gnn_add_module(trainingGraph  DEPS common tensor designGraph)
gnn_add_module(inferenceGraph DEPS common tensor trainingGraph)
gnn_add_module(storage        DEPS common tensor designGraph trainingGraph inferenceGraph)
gnn_add_module(trainer        DEPS common tensor trainingGraph storage)

# ---------------------------------------------------------------------------
# CLI 실행 파일 (src/gnn/cli/ 에 .cpp 가 생기면 만들어진다)
# ---------------------------------------------------------------------------
file(GLOB_RECURSE GNN_CLI_SOURCES CONFIGURE_DEPENDS ${GNN_SOURCE_DIR}/cli/*.cpp)
if(GNN_CLI_SOURCES)
    add_executable(gnn_cli ${GNN_CLI_SOURCES})
    target_include_directories(gnn_cli PRIVATE ${GNN_IMPORT_DIR})
    gnn_set_warnings(gnn_cli PRIVATE)
    target_link_libraries(gnn_cli PRIVATE gnn_trainer gnn_storage)
    set_target_properties(gnn_cli PROPERTIES OUTPUT_NAME gnn)
else()
    message(STATUS "gnn: src/gnn/cli 에 소스가 없어 CLI 실행 파일을 만들지 않는다")
endif()

# ---------------------------------------------------------------------------
# 테스트 (doctest)
#   doctest 는 third_party/doctest/doctest/doctest.h 또는 시스템에 설치된 위치에서 찾는다.
#   테스트 중 한 파일에 DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN 이 있어야 main 이 만들어진다.
# ---------------------------------------------------------------------------
if(GNN_BUILD_TESTS)
    file(GLOB_RECURSE GNN_TEST_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)

    find_path(GNN_DOCTEST_INCLUDE_DIR
        NAMES doctest/doctest.h
        HINTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/doctest)

    if(NOT GNN_TEST_SOURCES)
        message(STATUS "gnn: tests/ 에 소스가 없어 테스트를 만들지 않는다")
    elseif(NOT GNN_DOCTEST_INCLUDE_DIR)
        message(WARNING "gnn: doctest 를 찾지 못해 테스트를 만들지 않는다 "
                        "(third_party/doctest/doctest/doctest.h 에 두거나 GNN_DOCTEST_INCLUDE_DIR 지정)")
    else()
        enable_testing()
        add_executable(gnn_tests ${GNN_TEST_SOURCES})
        target_include_directories(gnn_tests PRIVATE ${GNN_IMPORT_DIR})
        # 외부 헤더로 취급해 doctest 내부 경고가 우리 경고에 섞이지 않게 한다.
        target_include_directories(gnn_tests SYSTEM PRIVATE ${GNN_DOCTEST_INCLUDE_DIR})
        gnn_set_warnings(gnn_tests PRIVATE)
        target_link_libraries(gnn_tests PRIVATE
            gnn_common gnn_tensor gnn_designGraph gnn_trainingGraph
            gnn_inferenceGraph gnn_storage gnn_trainer)
        add_test(NAME gnn_tests COMMAND gnn_tests)
    endif()
endif()
