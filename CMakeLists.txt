cmake_minimum_required(VERSION 3.19)
project(Spider VERSION 1.0 LANGUAGES CXX)

set(OUTPUT_NAME Spider)
set(MYLIB_PATH ${PROJECT_SOURCE_DIR}/SecondFunk)
set(LIBPQ_ROOT $ENV{LIBPQXX_ROOT})      # Укажите путь к libpq++
set(HEADERS Types.h
			${MYLIB_PATH}/SecondaryFunction.h
			HtmlClient/HtmlClient.h
			ConfigFile/ConfigFile.h
			Clientdb/Clientdb.h
			wordSearch/wordSearch.h
			ThreadPool/Thread_pool.h
			ThreadPool/Safe_queue.hpp)
set(SOURCES main.cpp
			${MYLIB_PATH}/SecondaryFunction.cpp
			HtmlClient/HtmlClient.cpp
			ConfigFile/ConfigFile.cpp
			Clientdb/Clientdb.cpp
			wordSearch/wordSearch.cpp
			ThreadPool/Thread_pool.cpp)
# C/C++: доп. каталоги включаемых файлов (#include)
include_directories(${MYLIB_PATH}
					${PROJECT_SOURCE_DIR}/HtmlClient
					${PROJECT_SOURCE_DIR}/ConfigFile
					${PROJECT_SOURCE_DIR}/Clientdb
					${PROJECT_SOURCE_DIR}/wordSearch
					${PROJECT_SOURCE_DIR}/ThreadPool)

# Boost:
# Компоновщик: доп. каталоги библиотек
link_directories($ENV{BOOST_ROOT}/lib)
find_package(Boost 1.80.0 REQUIRED)
if(NOT Boost_FOUND)
	message(FATAL_ERROR "Could not find Boost")
endif()

# OpenSSL
find_package(OpenSSL REQUIRED)

#[[ if (Visual Studio)
	настраиваю в качестве запускаемого проекта
	настройка на стандарт с++17 ]]
if(MSVC)
add_compile_definitions(_WIN32_WINNT=0x0601) # минимальная версия ОС win-7
#add_compile_definitions(_WIN32_WINNT=0x0A01) # минимальная версия ОС win-10
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${OUTPUT_NAME})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
else()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif()

add_executable(${OUTPUT_NAME} ${HEADERS} ${SOURCES})

# PQXX
#set(PostgreSQL_ADDITIONAL_VERSIONS "15")
set(SKIP_BUILD_TEST ON)
add_subdirectory(${LIBPQ_ROOT} build-pqxx)

target_include_directories(${OUTPUT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(${OUTPUT_NAME} PRIVATE
										${Boost_LIBRARIES}
										pqxx
										OpenSSL::SSL)
