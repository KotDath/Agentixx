cmake_minimum_required(VERSION 3.15)
project(Agentixx VERSION 0.1.0)

# Установить стандарт C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Опции сборки
option(BUILD_EXAMPLES "Build example applications" ON)
option(BUILD_TESTS "Build tests" OFF)

# Найти зависимости
find_package(PkgConfig REQUIRED)
find_package(nlohmann_json QUIET)

# Если nlohmann_json не найден через find_package, попробуем через pkg-config
if(NOT nlohmann_json_FOUND)
    pkg_check_modules(NLOHMANN_JSON REQUIRED nlohmann_json)
endif()

# Найти libcurl
find_package(CURL REQUIRED)

# Создать основную библиотеку
add_library(agentixx
    src/core/response.cpp
    src/core/http_client.cpp
    src/llm/openai_adapter.cpp
)

# Установить заголовки
target_include_directories(agentixx PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Линковать зависимости
if(nlohmann_json_FOUND)
    target_link_libraries(agentixx PUBLIC nlohmann_json::nlohmann_json)
else()
    target_include_directories(agentixx PUBLIC ${NLOHMANN_JSON_INCLUDE_DIRS})
endif()

target_link_libraries(agentixx PUBLIC CURL::libcurl)

# Компилятор специфичные флаги
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(agentixx PRIVATE
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter
    )
    
    # Debug флаги
    target_compile_options(agentixx PRIVATE
        $<$<CONFIG:Debug>:-g -O0 -DDEBUG>
    )
    
    # Release флаги
    target_compile_options(agentixx PRIVATE
        $<$<CONFIG:Release>:-O3 -DNDEBUG>
    )
endif()

# Создать alias для удобства
add_library(Agentixx::Agentixx ALIAS agentixx)

# Сборка примеров
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Сборка тестов
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Информация для установки
include(GNUInstallDirs)

# Установка библиотеки
install(TARGETS agentixx
    EXPORT AgentixxTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Установка заголовков
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
)

# Создание конфига для других проектов
install(EXPORT AgentixxTargets
    FILE AgentixxTargets.cmake
    NAMESPACE Agentixx::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Agentixx
)

# Создание AgentixxConfig.cmake
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/AgentixxConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/AgentixxConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Agentixx
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/AgentixxConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/AgentixxConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/AgentixxConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Agentixx
)