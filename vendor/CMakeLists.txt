cmake_minimum_required(VERSION 3.30)
project(bodies_vendor VERSION 0.0.1 LANGUAGES C)

set(HEADER_LIST tlsf.h)
add_library(vendor STATIC tlsf.c ${HEADER_LIST})

target_include_directories(vendor SYSTEM BEFORE PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/vendor>
)

if (MSVC AND NOT BUILD_SHARED_LIBS)
    # Set the output name for debugger files, otherwise they are not generated:
    set_target_properties(vendor
        PROPERTIES
        COMPILE_PDB_NAME_DEBUG vendor
        COMPILE_PDB_NAME_RELEASE vendor
        COMPILE_PDB_NAME_MINSIZEREL vendor
        COMPILE_PDB_NAME_RELWITHDEBINFO vendor)
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(LIB_NAME "BodiesVendor")
set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(PACKAGE_CONFIG "${GENERATED_DIR}/${LIB_NAME}Config.cmake")
set(VERSION_CONFIG "${GENERATED_DIR}/${LIB_NAME}ConfigVersion.cmake")
set(CONFIG_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/cmake")
set(TARGETS_EXPORT_NAME "${LIB_NAME}Targets")

configure_package_config_file("cmake/${LIB_NAME}Config.cmake.in" "${PACKAGE_CONFIG}" INSTALL_DESTINATION "${CONFIG_INSTALL_DIR}")
write_basic_package_version_file("${VERSION_CONFIG}" COMPATIBILITY ExactVersion)

install(TARGETS vendor
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/vendor"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES "${HEADER_LIST}" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/vendor")
install(FILES "${PACKAGE_CONFIG}" "${VERSION_CONFIG}" DESTINATION "${CONFIG_INSTALL_DIR}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/vendor.pdb" DESTINATION "${CMAKE_INSTALL_LIBDIR}" CONFIGURATIONS Debug)
install(EXPORT "${TARGETS_EXPORT_NAME}" NAMESPACE "bodies::" DESTINATION "${CONFIG_INSTALL_DIR}")
