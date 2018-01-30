# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library(
            live555
            STATIC
            IMPORTED)
set_target_properties(
            live555
            PROPERTIES IMPORTED_LOCATION
            ../../../../src/main/jniLibs/armeabi/liblive555.a)

add_library(
            polarssl
            STATIC
            IMPORTED)
set_target_properties(
            polarssl
            PROPERTIES IMPORTED_LOCATION
            ../../../../src/main/jniLibs/armeabi/libpolarssl.a)

add_library(
            rtmp
            STATIC
            IMPORTED)
set_target_properties(
            rtmp
            PROPERTIES IMPORTED_LOCATION
            ../../../../src/main/jniLibs/armeabi/librtmp.a)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
#判断编译器类型,如果是gcc编译器,则在编译选项中加入c++11支持
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
    message(STATUS "optional:-std=c++11")
endif(CMAKE_COMPILER_IS_GNUCXX)

#配置编译的头文件
include_directories(src/main/cpp
                    src/main/cpp/librtmp
                    src/main/cpp/live555/BasicUsageEnvironment/include
                    src/main/cpp/live555/groupsock/include
                    src/main/cpp/live555/liveMedia
                    src/main/cpp/live555/liveMedia/include
                    src/main/cpp/live555/UsageEnvironment/include)

add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/LiveAudioMediaSubsession.cpp
             src/main/cpp/LiveAudioSource.cpp
             src/main/cpp/LiveVideoMediaSubsession.cpp
             src/main/cpp/LiveVideoSource.cpp
             src/main/cpp/MediaBuffer.cpp
             src/main/cpp/MediaServer.cpp
             src/main/cpp/RtmpClient.cpp
             src/main/cpp/TriggerStream.cpp
             src/main/cpp/native-lib.cpp)



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

find_library( # Sets the name of the path variable.
              dl-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              dl )

find_library( # Sets the name of the path variable.
              z-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              z )

find_library( # Sets the name of the path variable.
              android-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              android )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       native-lib
                       rtmp
                       polarssl
                       live555
                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib}
                       ${dl-lib}
                       ${z-lib}
                       ${android-lib})