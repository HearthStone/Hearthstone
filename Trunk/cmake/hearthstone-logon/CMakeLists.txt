PROJECT(hearthstone-logonserver)
SET( prefix ${ROOT_PATH}/src/hearthstone-logonserver)
SET( sources
    AccountCache.cpp
    AuthSocket.cpp
    AutoPatcher.cpp
    LogonCommServer.cpp
    LogonConsole.cpp
    LogonOpcodes.cpp
    LogonStdAfx.cpp
    Main.cpp
)

 SET( headers
    AccountCache.h
    AuthSocket.h
    AuthStructs.h
    AutoPatcher.h
    LogonCommServer.h
    LogonConsole.h
    LogonOpcodes.h
    LogonStdAfx.h
    Main.h
    PeriodicFunctionCall_Thread.h
)

foreach(src IN ITEMS ${sources} )
  SET( SRCS ${SRCS} ${prefix}/${src} )
endforeach(src)
foreach(src IN ITEMS ${headers} )
  SET( HEADERS ${HEADERS} ${prefix}/${src} )
endforeach(src)

include_directories( ${GLOBAL_INCLUDE_DIRS} )
link_directories( ${EXTRA_LIBS_PATH} ${DEPENDENCY_LIBS} )
IF( IS_64BIT )
	link_directories( ${DEPENDENCY_DLLS64} )
ENDIF()

include(PCHSupport)

ADD_EXECUTABLE( ${PROJECT_NAME} ${SRCS} ${HEADERS} )

IF( CMAKE_GENERATOR MATCHES Visual*)
	ADD_NATIVE_PRECOMPILED_HEADER(${PROJECT_NAME} LogonStdAfx.h ${prefix}/LogonStdAfx.cpp)
ENDIF()

#Link our hearthstone-shared lib.
target_link_libraries( ${PROJECT_NAME} hearthstone-shared ${ZLIB_LIBRARIES} ${PCRE_LIBRARIES} )

target_link_libraries( ${PROJECT_NAME} ${ZLIB_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${MYSQL_LIBRARY} ${OPENSSL_LIBRARIES} ${EXTRA_LIBS} )

#Set our compile information
set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_DEFINITIONS _LOGON)

#Provide make install rules.
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)

