INCLUDE_DIRECTORIES(${WZDFTPD_SOURCE_DIR}
	${WZDFTPD_SOURCE_DIR}/libwzd-core
	${WZDFTPD_BINARY_DIR}
	${TCL_INCLUDE_PATH}
	)

ADD_DEFINITIONS(-DHAVE_CONFIG_H)

ADD_LIBRARY (libwzd_tcl SHARED
	libwzd_tcl.c
	tcl.def
	)

if (CYGWIN OR NOT WIN32)
  SET_TARGET_PROPERTIES(libwzd_tcl PROPERTIES PREFIX "")
endif (CYGWIN OR NOT WIN32)

SET(tcl_LIBS libwzd_core)

IF(TCL_LIBRARY)
  SET(tcl_LIBS ${tcl_LIBS} ${TCL_LIBRARY})
ENDIF(TCL_LIBRARY)

TARGET_LINK_LIBRARIES (libwzd_tcl ${tcl_LIBS})

INSTALL(TARGETS libwzd_tcl DESTINATION ${MODULES_INSTALL_PATH})
