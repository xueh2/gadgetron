if (WIN32)
  ADD_DEFINITIONS(-D__BUILD_GADGETRON_GADGET_GPUPMRI__)
  # The two flags below is to fix Windows problems in relation to multiple defined constructors in our headers
  SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /FORCE:MULTIPLE") 
  SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /FORCE:MULTIPLE") 
endif (WIN32)

find_package(Ismrmrd REQUIRED)

include_directories(   
  ${CMAKE_SOURCE_DIR}/gadgets/mri_core
  ${CMAKE_SOURCE_DIR}/toolboxes/mri/pmri/gpu
  ${CMAKE_SOURCE_DIR}/toolboxes/nfft/gpu
  ${CMAKE_SOURCE_DIR}/toolboxes/core/cpu
  ${CMAKE_SOURCE_DIR}/toolboxes/core/gpu
  ${CMAKE_SOURCE_DIR}/toolboxes/fft/gpu
  ${CMAKE_SOURCE_DIR}/toolboxes/operators
  ${CMAKE_SOURCE_DIR}/toolboxes/operators/gpu
  ${CMAKE_SOURCE_DIR}/toolboxes/solvers
  ${CMAKE_SOURCE_DIR}/toolboxes/solvers/gpu
)

add_library(gadgetron_gpuparallelmri SHARED 
    gadgetron_gpupmri_export.h
    GenericReconJob.h
    gpuCgKtSenseGadget.h
    gpuCgSenseGadget.h
    gpuCgSpiritGadget.h
    gpuGenericSensePrepGadget.h
    gpuSbSenseGadget.h
    gpuCgSenseGadget.cpp 
    gpuCgKtSenseGadget.cpp 
    gpuSbSenseGadget.cpp 
    gpuGenericSensePrepGadget.cpp
    gpuCgSpiritGadget.cpp 
  )

set_target_properties(gadgetron_gpuparallelmri PROPERTIES VERSION ${GADGETRON_VERSION_STRING} SOVERSION ${GADGETRON_SOVERSION})

target_link_libraries(gadgetron_gpuparallelmri 
  gadgetron_gadgetbase
  gadgetron_toolbox_cpucore gadgetron_toolbox_gpucore gadgetron_toolbox_gpusolvers gadgetron_toolbox_gpuoperators
  ${Boost_LIBRARIES} ${ISMRMRD_LIBRARIES} ${FFTW3_LIBRARIES} ${CUDA_LIBRARIES} 
  optimized ${ACE_LIBRARIES} debug ${ACE_DEBUG_LIBRARY}
  )

target_link_libraries(gadgetron_gpuparallelmri gadgetron_toolbox_gpuparallelmri)

install (TARGETS gadgetron_gpuparallelmri DESTINATION lib COMPONENT main)

install (FILES  gadgetron_gpupmri_export.h
                GenericReconJob.h
                gpuCgKtSenseGadget.h
                gpuCgSenseGadget.h
                gpuCgSpiritGadget.h
                gpuGenericSensePrepGadget.h
                gpuSbSenseGadget.h
                DESTINATION include COMPONENT main)

add_subdirectory(config)
