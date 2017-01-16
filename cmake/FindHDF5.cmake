if (WIN32)

find_path(HDF5_ROOT_DIR
        NAMES third-party/include/hdf5.h
        PATHS /usr/local
        )   
find_path(HDF5_DIR
        NAMES hdf5.h
        HINTS third-party/include
        )
find_path(HDF5_INCLUDE_DIRS
        NAMES hdf5.h
        HINTS third-party/include
        )
find_library(HDF5
        NAMES hdf5
        HINTS third-party/lib
        )
find_library(HDF5_DEBUG
        NAMES hdf5_D
        HINTS third-party/lib
        )
        
find_library(HDF5_CXX
        NAMES hdf5_cpp 
        HINTS third-party/lib
        )
        
find_library(HDF5_CXX_DEBUG
        NAMES hdf5_cpp_D
        HINTS third-party/lib
        )

if(HDF5_CXX_DEBUG)

set (HDF5_CXX_LIBRARIES optimized ${HDF5_CXX}
                        debug ${HDF5_CXX_DEBUG})

else ()

set (HDF5_CXX_LIBRARIES ${HDF5_CXX})

endif ()

if( HDF5_DEBUG )

set( HDF5_LIBRARIES optimized ${HDF5}
                          debug ${HDF5_DEBUG}
)

else ()

set( HDF5_LIBRARIES ${HDF5}
)

endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HDF5 DEFAULT_MSG
        HDF5_LIBRARIES
        HDF5_CXX_LIBRARIES
        HDF5_INCLUDE_DIRS
        )

mark_as_advanced(
        HDF5_ROOT_DIR
        HDF5_LIBRARIES
        HDF5_CXX_LIBRARIES
        HDF5_INCLUDE_DIRS
)

endif ()