if(WIN32)
    set(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/nfd_common.c" 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/nfd_win.cpp"
    )
elseif(UNIX)
set(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/nfd_common.c" 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/nfd_gtk.c"
    )
elseif(APPLE)
set(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/nfd_common.c" 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/nfd_cocoa.m"
    )
else()
    message(FATAL_ERROR "Unsupported platform")
endif()


add_library("nativefiledialog" STATIC ${FILES})
target_include_directories("nativefiledialog" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/include")