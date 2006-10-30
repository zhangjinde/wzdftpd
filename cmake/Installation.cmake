IF (NOT BACKENDS_INSTALL_PATH)
  SET(BACKENDS_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/share/wzdftpd/backends")
ENDIF (NOT BACKENDS_INSTALL_PATH)

IF (NOT MODULES_INSTALL_PATH)
  SET(MODULES_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/share/wzdftpd/modules")
ENDIF (NOT MODULES_INSTALL_PATH)

IF (NOT CONF_INSTALL_PATH)
  SET(CONF_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/etc/wzdftpd")
ENDIF (NOT CONF_INSTALL_PATH)

IF (NOT HEADERS_INSTALL_PATH)
  SET(HEADERS_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/include")
ENDIF (NOT HEADERS_INSTALL_PATH)

IF (NOT DOC_INSTALL_PATH)
  SET(DOC_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/share/doc/wzdftpd")
ENDIF (NOT DOC_INSTALL_PATH)

# set variables for replacement in wzd.cfg.sample
SET(PACKAGE "wzdftpd")
SET(localstatedir "${CMAKE_INSTALL_PREFIX}/var")
SET(datadir "${CMAKE_INSTALL_PREFIX}/share")
SET(sysconfdir "${CONF_INSTALL_PATH}")

IF(WIN32)
  SET(WZD_DEFAULT_CONF "wzd-win32.cfg")
ELSE(WIN32)
  SET(WZD_DEFAULT_CONF "${CONF_INSTALL_PATH}/wzd.cfg")
ENDIF(WIN32)

