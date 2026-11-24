cmake_minimum_required(VERSION 3.14)
project(drone_target_system CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Тип збірки за замовчуванням (якщо не вказано) - Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Опція для увімкнення санітайзерів (типово вимкнена)
option(ENABLE_SANITIZERS "Enable Address and UndefinedBehavior Sanitizers" OFF)

# 1. Ціль-бібліотека ядра (Core Target)
add_library(core src/core.cpp)
target_include_directories(core PUBLIC include)

# 2. Ціль-застосунок (CLI App Target)
add_executable(app src/main.cpp)

# 3. Зв'язуємо застосунок із ядром
target_link_libraries(app PRIVATE core)

# 4. Налаштування санітайзерів
if(ENABLE_SANITIZERS)
    if(MSVC)
        target_compile_options(core PRIVATE /fsanitize=address)
        target_compile_options(app PRIVATE /fsanitize=address)
    else()
        target_compile_options(core PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_compile_options(app PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(core PRIVATE -fsanitize=address,undefined)
        target_link_options(app PRIVATE -fsanitize=address,undefined)
    endif()
endif()

# Вмикаємо можливість тестування
enable_testing()

# Підключаємо модуль FetchContent
include(FetchContent)

# Оголошуємо звідки скачати GoogleTest(GitHub repo)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

# Робимо GTest доступним для проекту
FetchContent_MakeAvailable(googletest)

# Створюємо ціль з тестами та лінкуємо до власного ядра(core)  та gtest_main
add_executable(unit_test tests/test_main.cpp)
target_link_libraries(unit_test PRIVATE core gtest_main)

# Реєструємо тести у CTest
include(GoogleTest)
gtest_discover_tests(unit_test)

#Автоматичний запуск тестів піcля збірки у режимі Debug
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_command(
        TARGET unit_test POST_BUILD
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    )
endif()
