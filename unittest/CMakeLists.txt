include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG main
)

FetchContent_MakeAvailable(googletest)
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
	#googletest를 빌드하기위한 sub-build 구성
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
    include_directories(${gtest_SOURCE_DIR}/include ${gmock_SOURCE_DIR}/include)
endif()

set(TARGET_EXEC_TEST udpclient_test)
add_executable(${TARGET_EXEC_TEST})

target_include_directories(${TARGET_EXEC_TEST} PRIVATE
    ${PROJECT_WORKSPACE_DIR}/include)

target_sources(${TARGET_EXEC_TEST} PRIVATE
    ${PROJECT_WORKSPACE_DIR}/src/Logger.cpp
    ${PROJECT_WORKSPACE_DIR}/src/UDPSocketClient.cpp
    ${PROJECT_UNITTEST_DIR}/src/main_test.cpp
    ${PROJECT_UNITTEST_DIR}/src/UDPSocketClient_test.cpp
    ${PROJECT_UNITTEST_DIR}/src/Logger_test.cpp)

target_link_libraries(${TARGET_EXEC_TEST} gtest gtest_main gmock gmock_main)