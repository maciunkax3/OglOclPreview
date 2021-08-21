# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.10.2)

# Declares and names the project.

project("oclbenchark")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

set(SOURCE_FILES native-lib.cpp)
# Build stub
add_subdirectory(libopencl-stub)
add_subdirectory(OCL_Common)
MESSAGE(status ${SOURCE_FILES})
add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
            ${SOURCE_FILES})


add_library( # Sets the name of the library.
        setEnvLib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${CMAKE_CURRENT_SOURCE_DIR}/setEnvLib.cpp)

find_library( # Sets the name of the path variable.
        GLESv2-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        GLESv2 )

add_library( # Sets the name of the library.
        NativeOGL

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${CMAKE_CURRENT_SOURCE_DIR}/nativeOGLLib.cpp)

target_link_libraries(NativeOGL
        ${GLESv2-lib}
        ${log-lib}
        log)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

include_directories(includes)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

#add_library(opencl SHARED IMPORTED)
#add_library(utils SHARED IMPORTED)
#add_library(cpp SHARED IMPORTED)
#add_library(ndkSupport SHARED IMPORTED)
#set_target_properties(opencl
#        PROPERTIES IMPORTED_LOCATION
#        ${CMAKE_CURRENT_SOURCE_DIR}/libs/libOpenCL_system.so)
#set_target_properties(utils
#        PROPERTIES IMPORTED_LOCATION
#        ${CMAKE_CURRENT_SOURCE_DIR}/libs/libcutils.so)
#set_target_properties(cpp
#        PROPERTIES IMPORTED_LOCATION
#        ${CMAKE_CURRENT_SOURCE_DIR}/libs/libc++.so)
#set_target_properties(ndkSupport
#        PROPERTIES IMPORTED_LOCATION
#        ${CMAKE_CURRENT_SOURCE_DIR}/libs/libvndksupport.so)
#
target_link_libraries(native-lib
                      ${log-lib}
                      log
                      atomic
                      dl
                      OpenCL
                      GLESv2)
