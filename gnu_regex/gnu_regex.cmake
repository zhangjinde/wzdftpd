IF (NOT HAVE_REGCOMP)
  ADD_DEFINITIONS(-DREGEX_MALLOC -DSTDC_HEADERS)
  SET(gnu_regex_SOURCES
	${WZDFTPD_SOURCE_DIR}/gnu_regex/regex.c
	${WZDFTPD_SOURCE_DIR}/gnu_regex/regex.h
	)
ENDIF (NOT HAVE_REGCOMP)
