cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Task_3")

set(exe Task_3)
set(lib Code_reader)
set(testing Task_3_test)
set(output ${PROJECT_SOURCE_DIR}/output)
set(test_output ${PROJECT_SOURCE_DIR}/Testing)

add_executable (${exe} "Main.cpp")
add_library(${lib} "Code_reader.cpp")
target_link_libraries(${exe} ${lib})

set_target_properties(${exe} PROPERTIES 
	RUNTIME_OUTPUT_DIRECTORY ${output}
	ARCHIVE_OUTPUT_DIRECTORY ${output})

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

set_target_properties(${testing} PROPERTIES 
	RUNTIME_OUTPUT_DIRECTORY ${test_output}
	ARCHIVE_OUTPUT_DIRECTORY ${test_output})

