CMAKE_MINIMUM_REQUIRED (VERSION 2.8)


SET (ESCARGOT_INCDIRS
    ${ESCARGOT_INCDIRS}
    ${ESCARGOT_ROOT}/src/
    ${GCUTIL_ROOT}/
    ${GCUTIL_ROOT}/bdwgc/include/
    ${ESCARGOT_THIRD_PARTY_ROOT}/checked_arithmetic/
    ${ESCARGOT_THIRD_PARTY_ROOT}/double_conversion/
    ${ESCARGOT_THIRD_PARTY_ROOT}/rapidjson/include/
    ${ESCARGOT_THIRD_PARTY_ROOT}/yarr/
)

IF (${ESCARGOT_MODE} STREQUAL "debug")
    SET (ESCARGOT_CXXFLAGS ${ESCARGOT_CXXFLAGS} ${ESCARGOT_CXXFLAGS_DEBUG})
    SET (ESCARGOT_LDFLAGS ${ESCARGOT_LDFLAGS} ${ESCARGOT_LDFLAGS_DEBUG})
    SET (ESCARGOT_DEFINITIONS ${ESCARGOT_DEFINITIONS} ${ESCARGOT_DEFINITIONS_DEBUG})
ELSEIF (${ESCARGOT_MODE} STREQUAL "release")
    SET (ESCARGOT_CXXFLAGS ${ESCARGOT_CXXFLAGS} ${ESCARGOT_CXXFLAGS_RELEASE})
    SET (ESCARGOT_LDFLAGS ${ESCARGOT_LDFLAGS} ${ESCARGOT_LDFLAGS_RELEASE})
    SET (ESCARGOT_DEFINITIONS ${ESCARGOT_DEFINITIONS} ${ESCARGOT_DEFINITIONS_RELEASE})
ENDIF()

IF (${ESCARGOT_OUTPUT} STREQUAL "bin")
    SET (ESCARGOT_CXXFLAGS ${ESCARGOT_CXXFLAGS} ${ESCARGOT_CXXFLAGS_BIN})
    SET (ESCARGOT_LDFLAGS ${ESCARGOT_LDFLAGS} ${ESCARGOT_LDFLAGS_BIN})
    SET (ESCARGOT_DEFINITIONS ${ESCARGOT_DEFINITIONS} ${ESCARGOT_DEFINITIONS_BIN})
ELSEIF (${ESCARGOT_OUTPUT} STREQUAL "shared_lib")
    SET (ESCARGOT_CXXFLAGS ${ESCARGOT_CXXFLAGS} ${ESCARGOT_CXXFLAGS_SHAREDLIB})
    SET (ESCARGOT_LDFLAGS ${ESCARGOT_LDFLAGS} ${ESCARGOT_LDFLAGS_SHAREDLIB})
    SET (ESCARGOT_DEFINITIONS ${ESCARGOT_DEFINITIONS} ${ESCARGOT_DEFINITIONS_SHAREDLIB})
ELSEIF (${ESCARGOT_OUTPUT} STREQUAL "static_lib")
    SET (ESCARGOT_CXXFLAGS ${ESCARGOT_CXXFLAGS} ${ESCARGOT_CXXFLAGS_STATICLIB})
    SET (ESCARGOT_LDFLAGS ${ESCARGOT_LDFLAGS} ${ESCARGOT_LDFLAGS_STATICLIB})
    SET (ESCARGOT_DEFINITIONS ${ESCARGOT_DEFINITIONS} ${ESCARGOT_DEFINITIONS_STATICLIB})
ENDIF()

IF (${VENDORTEST})
    SET (ESCARGOT_DEFINITIONS ${ESCARGOT_DEFINITIONS} ${ESCARGOT_DEFINITIONS_VENDORTEST})
ENDIF()


# SOURCE FILES
FILE (GLOB_RECURSE ESCARGOT_SRC ${ESCARGOT_ROOT}/src/*.cpp)
FILE (GLOB YARR_SRC ${ESCARGOT_THIRD_PARTY_ROOT}/yarr/*.cpp)
FILE (GLOB DOUBLE_CONVERSION_SRC ${ESCARGOT_THIRD_PARTY_ROOT}/double_conversion/*.cc)

IF (NOT ${ESCARGOT_OUTPUT} STREQUAL "bin")
    LIST (REMOVE_ITEM ESCARGOT_SRC ${ESCARGOT_ROOT}/src/shell/Shell.cpp ${ESCARGOT_ROOT}/src/shell/GlobalObjectBuiltinTestFunctions.cpp)
ENDIF()

SET (ESCARGOT_SRC_LIST
    ${ESCARGOT_SRC}
    ${YARR_SRC}
    ${DOUBLE_CONVERSION_SRC}
)

# Compile custom GCUtil files with Escargot.
IF (ESCARGOT_PROFILE_BDWGC OR ESCARGOT_MEM_STATS)
    FILE (GLOB GCUTIL_SRC ${GCUTIL_ROOT}/*.cpp)
    LIST (APPEND ESCARGOT_SRC_LIST ${GCUTIL_SRC})
ENDIF()

# GC LIBRARY (static) only for binary output
IF (${ESCARGOT_OUTPUT} STREQUAL "bin")
    SET (GC_CFLAGS_COMMON "-g3 -fdata-sections -ffunction-sections -DHAVE_CONFIG_H -DESCARGOT -DIGNORE_DYNAMIC_LOADING -DGC_DONT_REGISTER_MAIN_STATIC_DATA -Wno-unused-variable -DGC_THREADS -D_REENTRANT")

    IF (${ESCARGOT_MODE} STREQUAL "debug")
        SET (GC_CFLAGS_MODE "-O0 -DGC_DEBUG")
    ELSE()
        SET (GC_CFLAGS_MODE "-O2")
    ENDIF()

    SET (GC_CFLAGS "${GC_CFLAGS_COMMON} ${GC_CFLAGS_ARCH} ${GC_CFLAGS_MODE} $ENV{CFLAGS}")
    SET (GC_LDFLAGS "${GC_LDFLAGS_ARCH} ${GC_CFLAGS}")

    SET (GC_CONFFLAGS_COMMON --enable-cplusplus   --enable-munmap --disable-parallel-mark --enable-large-config --enable-pthread --enable-threads=pthreads)
    IF (${ESCARGOT_MODE} STREQUAL "debug")
        SET (GC_CONFFLAGS_MODE --enable-debug --enable-gc-debug)
    ELSE()
        SET (GC_CONFFLAGS_MODE --disable-debug --disable-gc-debug)
    ENDIF()
    SET (GC_CONFFLAGS
        ${GC_CONFFLAGS_COMMON}
        ${GC_CONFFLAGS_MODE}
    )

    SET (GC_BUILDDIR ${GCUTIL_ROOT}/bdwgc/out/${ESCARGOT_HOST}/${ESCARGOT_ARCH}/${ESCARGOT_MODE}.static)
    SET (GC_TARGET ${GC_BUILDDIR}/.libs/libgc.a)

    ADD_CUSTOM_COMMAND (OUTPUT ${GC_TARGET}
            COMMENT "BUILD GC"
            WORKING_DIRECTORY ${GCUTIL_ROOT}/bdwgc
            COMMAND autoreconf -vif
            COMMAND automake --add-missing
            COMMAND ${CMAKE_COMMAND} -E make_directory ${GC_BUILDDIR}
            COMMAND cd ${GC_BUILDDIR} && CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} ../../../../configure ${GC_CONFIGURE_HOST} ${GC_CONFFLAGS} CFLAGS=${GC_CFLAGS} LDFLAGS=${GC_LDFLAGS}
            COMMAND cd ${GC_BUILDDIR} && make -j
    )

    ADD_CUSTOM_TARGET (gc
            DEPENDS ${GC_TARGET}
            COMMAND echo "GC TARGET"
    )
ENDIF()


# BUILD
IF (${ESCARGOT_OUTPUT} STREQUAL "bin")
    ADD_EXECUTABLE (${ESCARGOT_TARGET} ${ESCARGOT_SRC_LIST})
    ADD_DEPENDENCIES (${ESCARGOT_TARGET} gc)

    TARGET_LINK_LIBRARIES (${ESCARGOT_TARGET} ${ESCARGOT_LIBRARIES} ${GC_TARGET} ${ESCARGOT_LDFLAGS})
    TARGET_INCLUDE_DIRECTORIES (${ESCARGOT_TARGET} PUBLIC ${ESCARGOT_INCDIRS})
    TARGET_COMPILE_DEFINITIONS (${ESCARGOT_TARGET} PUBLIC ${ESCARGOT_DEFINITIONS})
    TARGET_COMPILE_OPTIONS (${ESCARGOT_TARGET} PUBLIC ${ESCARGOT_CXXFLAGS} ${CXXFLAGS_FROM_ENV} ${PROFILER_FLAGS})

    ADD_CUSTOM_COMMAND (TARGET ${ESCARGOT_TARGET} POST_BUILD
                        COMMAND cp ${ESCARGOT_OUTDIR}/${ESCARGOT_TARGET} .)

ELSEIF (${ESCARGOT_OUTPUT} STREQUAL "shared_lib")
    ADD_LIBRARY (${ESCARGOT_TARGET} SHARED ${ESCARGOT_SRC_LIST})

    TARGET_LINK_LIBRARIES (${ESCARGOT_TARGET} ${ESCARGOT_LIBRARIES} ${ESCARGOT_LDFLAGS})
    TARGET_INCLUDE_DIRECTORIES (${ESCARGOT_TARGET} PUBLIC ${ESCARGOT_INCDIRS})
    TARGET_COMPILE_DEFINITIONS (${ESCARGOT_TARGET} PUBLIC ${ESCARGOT_DEFINITIONS})
    TARGET_COMPILE_OPTIONS (${ESCARGOT_TARGET} PUBLIC ${ESCARGOT_CXXFLAGS} ${CXXFLAGS_FROM_ENV})

ELSEIF (${ESCARGOT_OUTPUT} STREQUAL "static_lib")
    ADD_LIBRARY (${ESCARGOT_TARGET} STATIC ${ESCARGOT_SRC_LIST})

    TARGET_LINK_LIBRARIES (${ESCARGOT_TARGET} ${ESCARGOT_LIBRARIES} ${ESCARGOT_LDFLAGS})
    TARGET_INCLUDE_DIRECTORIES (${ESCARGOT_TARGET} PUBLIC ${ESCARGOT_INCDIRS})
    TARGET_COMPILE_DEFINITIONS (${ESCARGOT_TARGET} PUBLIC ${ESCARGOT_DEFINITIONS})
    TARGET_COMPILE_OPTIONS (${ESCARGOT_TARGET} PUBLIC ${ESCARGOT_CXXFLAGS} ${CXXFLAGS_FROM_ENV})
ENDIF()
