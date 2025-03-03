# Copyright CERN and copyright holders of ALICE O2. This software is distributed
# under the terms of the GNU General Public License v3 (GPL Version 3), copied
# verbatim in the file "COPYING".
#
# See http://alice-o2.web.cern.ch/license for full licensing information.
#
# In applying this license CERN does not waive the privileges and immunities
# granted to it by virtue of its status as an Intergovernmental Organization or
# submit itself to any jurisdiction.

set(MODULE GPUTrackingHIP)

set(CMAKE_CXX_COMPILER ${hip_HIPCC_EXECUTABLE})
set(CMAKE_CXX_EXTENSIONS OFF)

#setting flags as a global option for all HIP targets.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${O2_HIP_CMAKE_CXX_FLAGS} -fno-gpu-rdc")
if(DEFINED HIP_AMDGPUTARGET)
  set(TMP_TARGET "(GPU Target ${HIP_AMDGPUTARGET})")
endif()

message(STATUS "Building GPUTracking with HIP support ${TMP_TARGET}")

set(SRCS GPUReconstructionHIP.hip.cxx)
set(HDRS GPUReconstructionHIP.h GPUReconstructionHIPInternals.h)

if(ALIGPU_BUILD_TYPE STREQUAL "O2")
  o2_add_library(
    ${MODULE}
    SOURCES ${SRCS}
    PUBLIC_LINK_LIBRARIES O2::GPUTracking O2::ITStrackingHIP hip::host hip::device hip::hipcub roc::rocthrust
    PUBLIC_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/Detectors/TRD/base/src
                               ${CMAKE_SOURCE_DIR}/Detectors/Base/src
                               ${CMAKE_SOURCE_DIR}/DataFormats/Reconstruction/src
    TARGETVARNAME targetName)

  target_compile_definitions(
    ${targetName} PUBLIC GPUCA_GPULIBRARY=HIP
    $<TARGET_PROPERTY:O2::GPUTracking,COMPILE_DEFINITIONS>)

  install(FILES ${HDRS} DESTINATION include/GPU)

#  o2_add_test(GPUsortHIP NAME test_GPUsortHIP
#                  SOURCES test/testGPUsortHIP.hip.cxx
#                  PUBLIC_LINK_LIBRARIES O2::GPUCommon hip::host hip::device hip::hipcub roc::rocthrust
#                  COMPONENT_NAME GPU
#                  LABELS gpu)
endif()

if(ALIGPU_BUILD_TYPE STREQUAL "ALIROOT")
  if(NOT CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 14)
    message(FATAL_ERROR "HIP support needs at least C++14")
  endif()
  add_definitions(-DGPUCA_GPULIBRARY=HIP)

  # Generate the dictionary
  get_directory_property(incdirs INCLUDE_DIRECTORIES)
  generate_dictionary("Ali${MODULE}" "" "GPUReconstructionHIP.h" "${incdirs} .")

  # Generate the ROOT map Dependecies
  generate_rootmap("Ali${MODULE}" "" "")

  # Add a library to the project using the specified source files
  add_library_tested(Ali${MODULE} SHARED ${SRCS} G__Ali${MODULE}.cxx)
  # ROOT Cannot load this library, so we have to remove it from the list
  list(REMOVE_ITEM ALILIBSTESTED Ali${MODULE})
  set(ALILIBSTESTED ${ALILIBSTESTED} CACHE INTERNAL "ALILIBSTESTED")

  # HIP run-time and driver
  target_link_libraries(Ali${MODULE} PUBLIC AliGPUTracking hip::host hip::device hip::hipcub roc::rocthrust)

  # Additional compilation flags
  set_target_properties(Ali${MODULE} PROPERTIES COMPILE_FLAGS "")

  # Installation
  install(TARGETS Ali${MODULE} ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)

  install(FILES ${HDRS} DESTINATION include)
  set(targetName "Ali${MODULE}")
endif()

if(ALIGPU_BUILD_TYPE STREQUAL "Standalone")
  add_definitions(-DGPUCA_GPULIBRARY=HIP)
  add_library(${MODULE} SHARED ${SRCS})
  target_link_libraries(${MODULE} GPUTracking hip::host hip::device hip::hipcub roc::rocthrust)
  set(targetName "${MODULE}")
  install(TARGETS GPUTrackingHIP)
endif()
