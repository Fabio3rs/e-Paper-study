cmake_minimum_required(VERSION 3.5)

project(epd_project LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_DEFAULT 20)
set(CXX17 ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_C_EXTENSIONS ON)

# Find FreeType2 using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(FREETYPE REQUIRED freetype2)


# Directories
set(DIR_Config ${CMAKE_CURRENT_SOURCE_DIR}/lib/Config)
set(DIR_EPD ${CMAKE_CURRENT_SOURCE_DIR}/lib/e-Paper)
set(DIR_FONTS ${CMAKE_CURRENT_SOURCE_DIR}/lib/Fonts)
set(DIR_GUI ${CMAKE_CURRENT_SOURCE_DIR}/lib/GUI)
set(DIR_Examples ${CMAKE_CURRENT_SOURCE_DIR}/examples)
set(DIR_BIN ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# EPD selection
set(EPD epd7in5V2)


# Source files
file(GLOB SOURCES_C
    "${DIR_GUI}/*.c"
    "${DIR_GUI}/*.cpp"
    "${DIR_Examples}/main.c"
    "${DIR_Examples}/ImageData.c"
    "${DIR_Examples}/EPD_7in5_V2_test.cpp"
    "${DIR_FONTS}/*.c"
    "${DIR_EPD}/*.c"
)

# Include directories
include_directories(${DIR_Config} ${DIR_GUI} ${DIR_EPD} ${FREETYPE_INCLUDE_DIRS})

add_compile_definitions(epd7in5V2=1 USE_BCM2835_LIB=1 RPI=1)

# Target
add_executable(epd ${SOURCES_C})

# RPI options
set(USELIB_RPI USE_BCM2835_LIB)
set(LIB_RPI -Wl,--gc-sections)
if (USELIB_RPI STREQUAL USE_BCM2835_LIB)
    #set(LIB_RPI ${LIB_RPI} -lbcm2835 -lm)
elseif (USELIB_RPI STREQUAL USE_WIRINGPI_LIB)
    #set(LIB_RPI ${LIB_RPI} -lwiringPi -lm)
elseif (USELIB_RPI STREQUAL USE_DEV_LIB)
    #set(LIB_RPI ${LIB_RPI} -lm)
endif()
set(DEBUG_RPI -D ${USELIB_RPI} -D RPI)

# Jetson options
set(USELIB_JETSONI USE_DEV_LIB)
set(LIB_JETSONI -lm)
set(DEBUG_JETSONI -D ${USELIB_JETSONI} -D JETSON)

# Link libraries
#target_link_libraries(epd PRIVATE ${LIB_RPI} ${DEBUG_RPI})
#target_link_libraries(epd PUBLIC ${LIB_JETSONI} ${DEBUG_JETSONI})

# RPI_DEV library
add_library(RPI_DEV OBJECT ${DIR_Config}/dev_hardware_SPI.c ${DIR_Config}/RPI_sysfs_gpio.c ${DIR_Config}/DEV_Config.c)
set_target_properties(RPI_DEV PROPERTIES LINKER_LANGUAGE C)
target_link_libraries(RPI_DEV PUBLIC -lbcm2835 -lm)
target_link_libraries(epd PUBLIC $<TARGET_OBJECTS:RPI_DEV> -lbcm2835 -lm freetype curses)

# Clean rule
add_custom_target(clean2
        COMMAND rm -rf ${DIR_BIN}/*
        COMMAND rm -f epd
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
