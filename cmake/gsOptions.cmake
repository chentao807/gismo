# #################################################################
#
# Project options
#
#
# #################################################################


## #################################################################
## Build types
## #################################################################

SET( CMAKE_CXX_FLAGS_MAINTAINER "-Wall -Wabi -DUSE_GISMO_STACK_WALKER"
     CACHE STRING
    "Flags used by the C++ compiler during maintainer builds."
    FORCE )
SET( CMAKE_C_FLAGS_MAINTAINER "-Wall -pedantic" CACHE STRING
    "Flags used by the C compiler during maintainer builds."
    FORCE )
SET( CMAKE_EXE_LINKER_FLAGS_MAINTAINER
    "-Wl,--warn-unresolved-symbols,--warn-once" CACHE STRING
    "Flags used for linking binaries during maintainer builds."
    FORCE )
SET( CMAKE_SHARED_LINKER_FLAGS_MAINTAINER
    "-Wl,--warn-unresolved-symbols,--warn-once" CACHE STRING
    "Flags used by the shared libraries linker during maintainer builds."
    FORCE )
MARK_AS_ADVANCED(
    CMAKE_CXX_FLAGS_MAINTAINER
    CMAKE_C_FLAGS_MAINTAINER
    CMAKE_EXE_LINKER_FLAGS_MAINTAINER
    CMAKE_SHARED_LINKER_FLAGS_MAINTAINER )
# Update the documentation string of CMAKE_BUILD_TYPE for GUIs
SET( CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel Maintainer."
    FORCE )


# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
   set(CMAKE_BUILD_TYPE Debug CACHE STRING 
   "Type of build (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
   # Set the possible values of build type for cmake-gui
   set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
     "MinSizeRel" "RelWithDebInfo")
endif()

## #################################################################
## Options list
## #################################################################

# Set a default coefficient type if none was specified
if(NOT GISMO_COEFF_TYPE)
  set (GISMO_COEFF_TYPE "double" CACHE STRING
   "Coefficient type(float, double, long double, mpfr::mpreal)" FORCE)
   set_property(CACHE GISMO_COEFF_TYPE PROPERTY STRINGS
   "float" "double" "long double" "mpfr::mpreal"
   )
endif()

option(GISMO_EXTRA_DEBUG         "Extra debug features"   false  )
option(GISMO_BUILD_STATIC_LIB    "Build static library"   false  )
option(GISMO_BUILD_SHARED_LIB    "Build shared library"   true   )
if(CMAKE_COMPILER_IS_GNUCXX)
option(GISMO_BUILD_COVERAGE      "Build with coverage"    false  )
endif(CMAKE_COMPILER_IS_GNUCXX)
option(GISMO_BUILD_EXAMPLES      "Build examples"         true   )
option(GISMO_BUILD_CPPLOT        "Build cpplot"           false  )
option(GISMO_BUILD_AXL           "Build Axel Plugin"      false  )
option(GISMO_BUILD_PVIEW         "Build Paraview Plugin"  false  )
option(GISMO_BUILD_MEX           "Build Mex files"        false  )
option(GISMO_BUILD_QT_APP        "Build Qt application"   false  )
option(GISMO_WITH_OPENMP         "With OpenMP"            false  )
option(GISMO_WITH_VTK            "With VTK"               false  )
option(GISMO_WITH_PSOLID         "With Parasolid"         false  )
option(GISMO_WITH_MPFR           "With MPFR"              false  )
option(GISMO_WITH_ONURBS         "With OpenNurbs"         false  )
option(GISMO_WITH_IPOPT          "With IpOpt"             false  )
option(GISMO_WARNINGS            "Compiler Warnings"      true   )
option(GISMO_LOGGING_WARN        "Display warning messages (gsWarn)"     true)
option(GISMO_LOGGING_INFO        "Display information messages (gsInfo)" true)
option(GISMO_BUILD_CPP11         "Compile using C++11 flags" false)

message ("Configuration:")
message ("  CMAKE_BUILD_TYPE        ${CMAKE_BUILD_TYPE}")
message ("  GISMO_COEFF_TYPE        ${GISMO_COEFF_TYPE}")
message ("  GISMO_EXTRA_DEBUG       ${GISMO_EXTRA_DEBUG}")
message ("  GISMO_BUILD_STATIC_LIB  ${GISMO_BUILD_STATIC_LIB}")
message ("  GISMO_BUILD_SHARED_LIB  ${GISMO_BUILD_SHARED_LIB}")
if(CMAKE_COMPILER_IS_GNUCXX)
message ("  GISMO_BUILD_COVERAGE    ${GISMO_BUILD_COVERAGE}")
endif(CMAKE_COMPILER_IS_GNUCXX)
message ("  GISMO_BUILD_EXAMPLES    ${GISMO_BUILD_EXAMPLES}")
message ("  GISMO_BUILD_CPPLOT      ${GISMO_BUILD_CPPLOT}")
message ("  GISMO_BUILD_AXL         ${GISMO_BUILD_AXL}")
message ("  GISMO_BUILD_PVIEW       ${GISMO_BUILD_PVIEW}")
message ("  GISMO_BUILD_MEX         ${GISMO_BUILD_MEX}")
message ("  GISMO_BUILD_QT_APP      ${GISMO_BUILD_QT_APP}")
message ("  GISMO_WITH_OPENMP       ${GISMO_WITH_OPENMP}")
message ("  GISMO_WITH_VTK          ${GISMO_WITH_VTK}")
message ("  GISMO_WITH_PSOLID       ${GISMO_WITH_PSOLID}")
message ("  GISMO_WITH_MPFR         ${GISMO_WITH_MPFR}")
message ("  GISMO_WITH_ONURBS       ${GISMO_WITH_ONURBS}")
message ("  GISMO_WITH_IPOPT        ${GISMO_WITH_IPOPT}")
#message ("  GISMO_WARNINGS          ${GISMO_WARNINGS}")
message ("  GISMO_LOGGING_WARN      ${GISMO_LOGGING_WARN}")
message ("  GISMO_LOGGING_INFO      ${GISMO_LOGGING_INFO}")

#https://www.threadingbuildingblocks.org/documentation
#message ("  GISMO_WITH_ITBB          ${GISMO_WITH_ITBB}")