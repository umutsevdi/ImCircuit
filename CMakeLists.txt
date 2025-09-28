cmake_minimum_required(VERSION 3.15)
set(PROJECT_LONG  "ImCircuit")
project(ImCircuit C CXX)
if (WIN32)
    message(${CMAKE_SOURCE_DIR})
    # If Win32 use vcpkg to get dependencies.
    set(CMAKE_TOOLCHAIN_FILE 
       "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE})
endif()

# OPTIONS
option(IMC_ENABLE_DOXYGEN "Generate Doxygen documentation" YES)
option(IMC_BUILD_TESTS "Build and run tests" YES)
option(IMC_GUI "Build with user interface" YES)
option(IMC_PACKAGE "Package the executable" YES)
option(NFD_PORTAL "Use xdg-dektop-portal on Linux builds" YES)

set(CMAKE_C_STANDARD 11)
if (MSVC)
    set(CMAKE_CXX_STANDARD 20)
else()
    set(CMAKE_CXX_STANDARD 17)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message(STATUS "Started CMake for ${PROJECT_NAME} v${PROJECT_VERSION}...\n")
message("Build Tests: ${IMC_BUILD_TESTS}")
message("Doxygen: ${IMC_ENABLE_DOXYGEN}")
message("GUI: ${IMC_GUI}")
message("Package: ${IMC_PACKAGE}")
message("Build Type: ${CMAKE_BUILD_TYPE}")

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -Og -Wpedantic)
    else()
        add_compile_options(-O3)
    endif()
endif()

include(FetchContent)
FetchContent_Declare(keychain
    GIT_REPOSITORY https://github.com/hrantzsch/keychain.git
    GIT_TAG v1.3.1
)
FetchContent_MakeAvailable(keychain)

set(JSONCPP_WITH_TESTS OFF)
set(JSONCPP_WITH_POST_BUILD_UNITTEST OFF)
set(BUILD_STATIC_LIBS ON)
set(BUILD_SHARED_LIBS OFF)
FetchContent_Declare(jsoncpp
    GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
    GIT_TAG 1.9.6
)
FetchContent_MakeAvailable(jsoncpp)

include_directories(${keychain_SOURCE_DIR}/include)
include_directories(${jsoncpp_SOURCE_DIR}/include)

include_directories(src/common)
include_directories(src/core)
include_directories(src/cli)
add_subdirectory(src/common)
add_subdirectory(src/core)
add_subdirectory(src/cli)

include_directories(src/api)
add_subdirectory(src/api)

if(IMC_GUI)
    include_directories(src/ui)
    add_subdirectory(src/ui)
    if (WIN32)
		file(GLOB SOURCES src/main.cpp pkg/resource.rc)
    else()
		file(GLOB SOURCES src/main.cpp)
    endif()
    add_executable(ImCircuit ${SOURCES})

	# This ensures the application is treated as a Windows GUI application
	if((CMAKE_BUILD_TYPE STREQUAL "Release") AND WIN32)
		set_target_properties(${PROJECT_NAME} PROPERTIES
			WIN32_EXECUTABLE TRUE
		)
	endif()
    target_link_libraries(ImCircuit PRIVATE
        api
        common
        core
        jsoncpp_static
        #        keychain
        cli
        ui)
    target_compile_definitions(ImCircuit
        PRIVATE
        IMC_GUI=1
    )
else()
    # Build with cli only
    file(GLOB SOURCES src/main.cpp)
    add_executable(ImCircuit ${SOURCES})
    target_link_libraries(ImCircuit PRIVATE
        api
        common
        core
        jsoncpp_static
        #        keychain
        cli
    )
endif()
if(IMC_BUILD_TESTS)
    FetchContent_Declare(doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG v2.4.12
    )
    FetchContent_MakeAvailable(doctest)
    file(GLOB TESTS src/main.cpp test/*.cpp)
    add_executable(ImCircuit.tst ${TESTS})
    target_link_libraries(ImCircuit.tst PRIVATE
        common
        core
        doctest::doctest
        jsoncpp_static)

    target_include_directories(ImCircuit.tst PRIVATE
        ${doctest_SOURCE_DIR}/doctest
    )

    target_compile_definitions(ImCircuit.tst
        PRIVATE
        IMC_TEST=1
    )

    add_custom_target(run_tests
        COMMAND ImCircuit.tst
        DEPENDS ImCircuit.tst
        COMMENT "Running tests after build"
    )
    if(CMAKE_BUILD_TYPE STREQUAL "Release" AND IMC_GUI)
        add_dependencies(${PROJECT_NAME} run_tests)
    endif()
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Release" AND IMC_ENABLE_DOXYGEN)
    find_package(Doxygen QUIET)
    if(DOXYGEN_FOUND)
        FetchContent_Declare(doxygen-awesome-css
            GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css.git
            GIT_TAG v2.3.4
        )
        FetchContent_MakeAvailable(doxygen-awesome-css)
        
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
        set(DOXYGEN_OUT ${CMAKE_BINARY_DIR}/docs)
        message("${DOX} ${DOXYFILE_IN} ${DOXYFILE_OUT} Doxfile at: ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile")
        file(MAKE_DIRECTORY ${DOXYGEN_OUT})
        set(DOXYGEN_GENERATE_HTML YES)
        set(DOXYGEN_OUTPUT_DIRECTORY ${DOXYGEN_OUT})
        add_custom_target(doc
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_IN}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM)
        if(IMC_GUI)
            add_dependencies(${PROJECT_NAME} doc)
        else()
            add_dependencies(${PROJECT_NAME}.tst doc)
        endif()
    endif()
endif()

add_custom_target(bundle)
set(PKGDIR "${CMAKE_BINARY_DIR}/package")
add_custom_command(TARGET bundle POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/pkg/${PROJECT_NAME}"
            "${PKGDIR}/win32/${PROJECT_LONG}")
add_custom_command(TARGET bundle POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_BINARY_DIR}/locale
                "${PKGDIR}/win32/${PROJECT_LONG}/locale")
add_custom_command(TARGET bundle POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/pkg/imcircuit.ico
                "${PKGDIR}/win32/${PROJECT_LONG}/imcircuit.ico")
add_custom_command(TARGET bundle POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/CHANGELOG.txt
        "${PKGDIR}/win32/${PROJECT_LONG}/")

add_custom_command(TARGET bundle POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_BINARY_DIR}/locale
                "${PKGDIR}/usr/share/locale")
add_custom_command(TARGET bundle POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/pkg/${PROJECT_NAME}
        "${PKGDIR}/usr/share/${PROJECT_NAME}")
add_custom_command(TARGET bundle POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/pkg/imcircuit.desktop
                "${PKGDIR}/usr/share/applications/imcircuit.desktop")
add_custom_command(TARGET bundle POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/CHANGELOG.txt "${PKGDIR}/usr/share/${PROJECT_NAME}")
add_dependencies(${PROJECT_NAME} bundle)

if(CMAKE_BUILD_TYPE STREQUAL "Release" AND IMC_PACKAGE)
	if(NOT WIN32)
		set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
		set(CPACK_PACKAGE_VERSION "0.1.1")
		set(CPACK_PACKAGE_DESCRIPTION "A free and open-source cross-platform logic circuit simulator.")
		set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${CPACK_PACKAGE_DESCRIPTION})
		set(CPACK_PACKAGE_VENDOR "Umut Sevdi")
		set(CPACK_PACKAGE_MAINTAINER "Umut Sevdi <mail@umutsevdi.com>")
		set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
		set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

		set(CPACK_GENERATOR "DEB;RPM")  
		set(CPACK_RPM_PACKAGE_LICENSE "GPL")  
		set(CPACK_RPM_PACKAGE_GROUP "Development")  
		set(CPACK_RPM_PACKAGE_REQUIRES "libcurl, libsecret, glfw, mesa-libGL")
		set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Umut Sevdi <mail@umutsevdi.com>")
		set(CPACK_DEBIAN_PACKAGE_DEPENDS "libcurl4, libsecret-1-0, libglfw3")

		install(FILES "build/release/ImCircuit"
				DESTINATION "/usr/bin/"
				PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
										GROUP_EXECUTE GROUP_READ
										WORLD_EXECUTE WORLD_READ
		)

		install(DIRECTORY ${PKGDIR}/usr/share/
				DESTINATION /usr/share/
				USE_SOURCE_PERMISSIONS
				PATTERN "*.*"
		)

		install(FILES "LICENSE"
			DESTINATION "/usr/share/ImCircuit"
		)
		include(CPack)
	endif()
endif()
