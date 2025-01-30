cmake_minimum_required (VERSION 3.8)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Task_3")

set(exe Task_3)
set(lib Code_reader)
set(testing Task_3_test)

add_executable (${exe} "Main.cpp")
add_library(${lib} "Code_reader.cpp")
target_link_libraries(${exe} ${lib})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${exe} PROPERTY CXX_STANDARD 20)
  set_property(TARGET ${lib} PROPERTY CXX_STANDARD 20)
endif()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(${testing} "Test.cpp")
target_link_libraries(${testing} ${lib} gtest_main)
add_test(NAME Code_reader_testing COMMAND ${testing})

