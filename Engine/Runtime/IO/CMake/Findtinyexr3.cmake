find_path(tinyexr3_INCLUDE_DIR
    NAMES exr.h
    PATH_SUFFIXES tinyexr/include
)
find_library(tinyexr3_LIBRARY
    NAMES tinyexr3
    PATH_SUFFIXES tinyexr/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(tinyexr3
    REQUIRED_VARS tinyexr3_INCLUDE_DIR tinyexr3_LIBRARY
)

if(tinyexr3_FOUND AND NOT TARGET tinyexr3::tinyexr3)
    add_library(tinyexr3::tinyexr3 UNKNOWN IMPORTED GLOBAL)
    set_target_properties(tinyexr3::tinyexr3 PROPERTIES
        IMPORTED_LOCATION "${tinyexr3_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${tinyexr3_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(tinyexr3_INCLUDE_DIR tinyexr3_LIBRARY)
