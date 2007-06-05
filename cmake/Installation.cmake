IF (NOT WIN32)
  IF (NOT SBIN_INSTALL_PATH)
    SET(SBIN_INSTALL_PATH "sbin")
  ENDIF (NOT SBIN_INSTALL_PATH)
  
  IF (NOT BIN_INSTALL_PATH)
    SET(BIN_INSTALL_PATH "bin")
  ENDIF (NOT BIN_INSTALL_PATH)
  
  IF (NOT LIB_INSTALL_PATH)
    SET(LIB_INSTALL_PATH "lib")
  ENDIF (NOT LIB_INSTALL_PATH)
  
  IF (NOT BACKENDS_INSTALL_PATH)
    SET(BACKENDS_INSTALL_PATH "lib/wzdftpd/backends")
  ENDIF (NOT BACKENDS_INSTALL_PATH)
  
  IF (NOT MODULES_INSTALL_PATH)
    SET(MODULES_INSTALL_PATH "lib/wzdftpd/modules")
  ENDIF (NOT MODULES_INSTALL_PATH)
  
  IF (NOT CONF_INSTALL_PATH)
    SET(CONF_INSTALL_PATH "etc/wzdftpd")
  ENDIF (NOT CONF_INSTALL_PATH)
  
  IF (NOT HEADERS_INSTALL_PATH)
    SET(HEADERS_INSTALL_PATH "include")
  ENDIF (NOT HEADERS_INSTALL_PATH)
  
  IF (NOT DOC_INSTALL_PATH)
    SET(DOC_INSTALL_PATH "share/doc/wzdftpd")
  ENDIF (NOT DOC_INSTALL_PATH)
  
  IF (NOT MAN_INSTALL_PATH)
    SET(MAN_INSTALL_PATH "share/man")
  ENDIF (NOT MAN_INSTALL_PATH)
ELSE (NOT WIN32)
  IF (NOT SBIN_INSTALL_PATH)
    SET(SBIN_INSTALL_PATH "bin")
  ENDIF (NOT SBIN_INSTALL_PATH)
  
  IF (NOT BIN_INSTALL_PATH)
    SET(BIN_INSTALL_PATH "bin")
  ENDIF (NOT BIN_INSTALL_PATH)
  
  IF (NOT LIB_INSTALL_PATH)
    # we have forced to help windows: exe search a dll in its directory ..
    SET(LIB_INSTALL_PATH "bin")
  ENDIF (NOT LIB_INSTALL_PATH)
  
  IF (NOT BACKENDS_INSTALL_PATH)
    SET(BACKENDS_INSTALL_PATH "backends")
  ENDIF (NOT BACKENDS_INSTALL_PATH)
  
  IF (NOT MODULES_INSTALL_PATH)
    SET(MODULES_INSTALL_PATH "modules")
  ENDIF (NOT MODULES_INSTALL_PATH)
  
  IF (NOT CONF_INSTALL_PATH)
    SET(CONF_INSTALL_PATH "etc")
  ENDIF (NOT CONF_INSTALL_PATH)
  
  IF (NOT HEADERS_INSTALL_PATH)
    SET(HEADERS_INSTALL_PATH "include")
  ENDIF (NOT HEADERS_INSTALL_PATH)
  
  IF (NOT DOC_INSTALL_PATH)
    SET(DOC_INSTALL_PATH "doc")
  ENDIF (NOT DOC_INSTALL_PATH)
  
  IF (NOT MAN_INSTALL_PATH)
    SET(MAN_INSTALL_PATH "doc/man")
  ENDIF (NOT MAN_INSTALL_PATH)
ENDIF (NOT WIN32)

# set variables for replacement in wzd.cfg.sample and users.sample
SET(PACKAGE "wzdftpd")
SET(localstatedir "var")
SET(datadir "lib")
SET(sysconfdir "${CONF_INSTALL_PATH}")
SET(ftproot "home")

IF(WIN32)
  SET(WZD_DEFAULT_CONF "wzd-win32.cfg")
ELSE(WIN32)
  SET(WZD_DEFAULT_CONF "${CMAKE_INSTALL_PREFIX}/${CONF_INSTALL_PATH}/wzd.cfg")
ENDIF(WIN32)

