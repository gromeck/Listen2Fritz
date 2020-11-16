#
#
#	$Id: configure.ac 1122 2007-10-03 11:29:03Z gromeck $
#
#	Copyright (c) 2004 by Christian Lorenz
#
#	Process this file with autoconf to produce a configure script.
#

AC_INIT
AC_CONFIG_SRCDIR([.])
AM_INIT_AUTOMAKE(listen2fritz, $( cat VERSION ))
AM_CONFIG_HEADER(config.h)


######################################
#
#	Change some defaults.
#
test "$prefix" = NONE && prefix=/usr
test "$exec_prefix" = NONE && exec_prefix=$prefix
test "$sysconfdir" = '${prefix}/etc' && sysconfdir=/etc
test "$localstatedir" = '${prefix}/var' && localstatedir=/var/run
test "$mandir" = '${prefix}/man' && mandir=/usr/share/man
#echo "prefix=$prefix"
#echo "exec_prefix=$exec_prefix"
#echo "sysconfdir=$sysconfdir"
#echo "localstatedir=$localstatedir"
#echo "mandir=$mandir"


# debug
AC_ARG_ENABLE(debug,
		[  --enable-debug          enable debugging support],
			       DEBUGGING_ENABLED="$enableval",DEBUGGING_ENABLED=no)
AM_CONDITIONAL(DEBUGGING_ENABLED, test "$DEBUGGING_ENABLED" = yes)

# static
AC_ARG_ENABLE(static,
		[  --enable-static         enable static build],
			       STATIC_ENABLED="$enableval",STATIC_ENABLED=no)
AM_CONDITIONAL(STATIC_ENABLED, test "$STATIC_ENABLED" = yes)

# mysql
AC_ARG_ENABLE(mysql,
		[  --disable-mysql         disable mysql support],
			       MYSQL_ENABLED="$enableval",MYSQL_ENABLED=yes)
AM_CONDITIONAL(MYSQL_ENABLED, test "$MYSQL_ENABLED" = yes)

# ldap
AC_ARG_ENABLE(ldap,
		[  --disable-ldap          disable ldap support],
			       LDAP_ENABLED="$enableval",LDAP_ENABLED=yes)
AM_CONDITIONAL(LDAP_ENABLED, test "$LDAP_ENABLED" = yes)

# curl
AC_ARG_ENABLE(curl,
		[  --disable-curl          disable curl support],
			       CURL_ENABLED="$enableval",CURL_ENABLED=yes)
AM_CONDITIONAL(CURL_ENABLED, test "$CURL_ENABLED" = yes)

# logging-file
AC_ARG_ENABLE(logging-file,
		[  --disable-logging-file  disable logging to file],
			       L2F_LOGGING_FILE="$enableval",L2F_LOGGING_FILE=yes)

# logging-mysql
AC_ARG_ENABLE(logging-mysql,
		[  --disable-logging-mysql disable logging to mysql],
			       L2F_LOGGING_MYSQL="$enableval",L2F_LOGGING_MYSQL=yes)

# logging-syslog
AC_ARG_ENABLE(logging-syslog,
		[  --disable-logging-syslog disable logging to syslog],
			       L2F_LOGGING_SYSLOG="$enableval",L2F_LOGGING_SYSLOG=yes)

# lookup-ldap
AC_ARG_ENABLE(lookup-ldap,
		[  --disable-lookup-ldap   disable lookup via ldap],
			       L2F_LOOKUP_LDAP="$enableval",L2F_LOOKUP_LDAP=yes)

# lookup-mysql
AC_ARG_ENABLE(lookup-mysql,
		[  --disable-lookup-mysql  disable lookup via mysql],
			       L2F_LOOKUP_MYSQL="$enableval",L2F_LOOKUP_MYSQL=yes)

# lookup-script
AC_ARG_ENABLE(lookup-script,
		[  --disable-lookup-script disable lookup via script],
			       L2F_LOOKUP_SCRIPT="$enableval",L2F_LOOKUP_SCRIPT=yes)

# notify-dreambox
AC_ARG_ENABLE(notify-dreambox,
		[  --disable-notify-dreambox  disable notify via dreambox],
			       L2F_NOTIFY_DREAMBOX="$enableval",L2F_NOTIFY_DREAMBOX=yes)

# notify-irc
AC_ARG_ENABLE(notify-irc,
		[  --disable-notify-irc    disable notify via irc],
			       L2F_NOTIFY_IRC="$enableval",L2F_NOTIFY_IRC=yes)

# notify-mail
AC_ARG_ENABLE(notify-mail,
		[  --disable-notify-mail   disable notify via mail],
			       L2F_NOTIFY_MAIL="$enableval",L2F_NOTIFY_MAIL=yes)

# notify-mail
AC_ARG_ENABLE(notify-script,
		[  --disable-notify-script disable notify via script],
			       L2F_NOTIFY_SCRIPT="$enableval",L2F_NOTIFY_SCRIPT=yes)
# notify-callback
AC_ARG_ENABLE(notify-callback,
		[  --disable-notify-callback  disable notify via callback],
			       L2F_NOTIFY_CALLBACK="$enableval",L2F_NOTIFY_CALLBACK=yes)


#######################################
#
#	Checks for programs.
#
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AM_PROG_LEX
AC_PROG_RANLIB


#######################################
#
#	Checks for libraries.
#
AC_CHECK_LIB(m,main)
AC_CHECK_LIB(readline,main,,exit 1)
if test "$LDAP_ENABLED" = yes; then
	AC_CHECK_LIB(ldap,ldap_bind,[],[LDAP_ENABLED=no])
fi
AH_TEMPLATE([HAVE_LIBMYSQL],[Enabled if MYSQL can be used])
if test "$MYSQL_ENABLED" = yes; then
#echo "Checking for mysql ... $( mysql_config --version )"
#if test "$( mysql_config --version )" != "" ; then
#	CFLAGS="$CFLAGS $( mysql_config --include )"
#	LDFLAGS="$LDFLAGS $( mysql_config --libs )"
#	AC_DEFINE([HAVE_LIBMYSQL],[1])
#else
#	AC_DEFINE([HAVE_LIBMYSQL],[0])
#fi
	AC_CHECK_LIB(mysqlclient,mysql_init,[],[MYSQL_ENABLED=no])
fi
if test "$CURL_ENABLED" = yes; then
	AC_CHECK_LIB(curl,curl_easy_init,[],[CURL_ENABLED=no])
fi

#######################################
#
#	disable features depending on libs
#
if test "$LDAP_ENABLED" != yes; then
	L2F_LOOKUP_LDAP=no
fi
if test "$MYSQL_ENABLED" != yes; then
	L2F_LOGGING_MYSQL=no
	L2F_LOOKUP_MYSQL=no
fi
if test "$CURL_ENABLED" != yes; then
	L2F_NOTIFY_DREAMBOX=no
	L2F_NOTIFY_CALLBACK=no
fi


#######################################
#
#	disable features depending on libs
#
AH_TEMPLATE([HAVE_L2F_LOGGING_FILE],[Enabled if logging to file should be supported])
AH_TEMPLATE([HAVE_L2F_LOGGING_MYSQL],[Enabled if logging to mysql should be supported])
AH_TEMPLATE([HAVE_L2F_LOGGING_SYSLOG],[Enabled if logging to syslog should be supported])
AH_TEMPLATE([HAVE_L2F_LOOKUP_LDAP],[Enabled if lookup via ldap should be supported])
AH_TEMPLATE([HAVE_L2F_LOOKUP_MYSQL],[Enabled if lookup via mysql should be supported])
AH_TEMPLATE([HAVE_L2F_LOOKUP_SCRIPT],[Enabled if lookup via script should be supported])
AH_TEMPLATE([HAVE_L2F_NOTIFY_DREAMBOX],[Enabled if notify via dreambox should be supported])
AH_TEMPLATE([HAVE_L2F_NOTIFY_IRC],[Enabled if notify via irc should be supported])
AH_TEMPLATE([HAVE_L2F_NOTIFY_MAIL],[Enabled if notify via mail should be supported])
AH_TEMPLATE([HAVE_L2F_NOTIFY_SCRIPT],[Enabled if notify via script should be supported])
AH_TEMPLATE([HAVE_L2F_NOTIFY_CALLBACK],[Enabled if notify via callback should be supported])

if test "$L2F_LOGGING_FILE" = yes; then
	AC_DEFINE([HAVE_L2F_LOGGING_FILE],[1])
fi
if test "$L2F_LOGGING_MYSQL" = yes; then
	AC_DEFINE([HAVE_L2F_LOGGING_MYSQL],[1])
fi
if test "$L2F_LOGGING_SYSLOG" = yes; then
	AC_DEFINE([HAVE_L2F_LOGGING_SYSLOG],[1])
fi
if test "$L2F_LOOKUP_LDAP" = yes; then
	AC_DEFINE([HAVE_L2F_LOOKUP_LDAP],[1])
fi
if test "$L2F_LOOKUP_MYSQL" = yes; then
	AC_DEFINE([HAVE_L2F_LOOKUP_MYSQL],[1])
fi
if test "$L2F_LOOKUP_SCRIPT" = yes; then
	AC_DEFINE([HAVE_L2F_LOOKUP_SCRIPT],[1])
fi
if test "$L2F_NOTIFY_DREAMBOX" = yes; then
	AC_DEFINE([HAVE_L2F_NOTIFY_DREAMBOX],[1])
fi
if test "$L2F_NOTIFY_IRC" = yes; then
	AC_DEFINE([HAVE_L2F_NOTIFY_IRC],[1])
fi
if test "$L2F_NOTIFY_MAIL" = yes; then
	AC_DEFINE([HAVE_L2F_NOTIFY_MAIL],[1])
fi
if test "$L2F_NOTIFY_SCRIPT" = yes; then
	AC_DEFINE([HAVE_L2F_NOTIFY_SCRIPT],[1])
fi
if test "$L2F_NOTIFY_CALLBACK" = yes; then
	AC_DEFINE([HAVE_L2F_NOTIFY_CALLBACK],[1])
fi


#######################################
#
#	set the conditionals
#
AM_CONDITIONAL(L2F_LOGGING_FILE, test "$L2F_LOGGING_FILE" = yes)
AM_CONDITIONAL(L2F_LOGGING_MYSQL, test "$L2F_LOGGING_MYSQL" = yes)
AM_CONDITIONAL(L2F_LOGGING_SYSLOG, test "$L2F_LOGGING_SYSLOG" = yes)
AM_CONDITIONAL(L2F_LOOKUP_LDAP, test "$L2F_LOOKUP_LDAP" = yes)
AM_CONDITIONAL(L2F_LOOKUP_MYSQL, test "$L2F_LOOKUP_MYSQL" = yes)
AM_CONDITIONAL(L2F_LOOKUP_SCRIPT, test "$L2F_LOOKUP_SCRIPT" = yes)
AM_CONDITIONAL(L2F_NOTIFY_DREAMBOX, test "$L2F_NOTIFY_DREAMBOX" = yes)
AM_CONDITIONAL(L2F_NOTIFY_IRC, test "$L2F_NOTIFY_IRC" = yes)
AM_CONDITIONAL(L2F_NOTIFY_MAIL, test "$L2F_NOTIFY_MAIL" = yes)
AM_CONDITIONAL(L2F_NOTIFY_SCRIPT, test "$L2F_NOTIFY_SCRIPT" = yes)
AM_CONDITIONAL(L2F_NOTIFY_CALLBACK, test "$L2F_NOTIFY_CALLBACK" = yes)


#######################################
#
#	Checks for header files.
#
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT


#######################################
#
#	Checks for typedefs, structures, and compiler characteristics.
#
AC_C_CONST
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_UID_T
AC_C_VOLATILE
AC_CHECK_TYPES([ptrdiff_t])


#######################################
#
#	Checks for library functions.
#
AC_FUNC_ALLOCA
AC_FUNC_CHOWN
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_GETPGRP
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_SETPGRP
AC_FUNC_SETVBUF_REVERSED
AC_DIAGNOSE([obsolete],[your code may safely assume C89 semantics that RETSIGTYPE is void.
Remove this warning and the `AC_CACHE_CHECK' when you adjust the code.])dnl
AC_CACHE_CHECK([return type of signal handlers],[ac_cv_type_signal],[AC_COMPILE_IFELSE(
[AC_LANG_PROGRAM([#include <sys/types.h>
#include <signal.h>
],
		 [return *(signal (0, 0)) (0) == 1;])],
		   [ac_cv_type_signal=int],
		   [ac_cv_type_signal=void])])
AC_DEFINE_UNQUOTED([RETSIGTYPE],[$ac_cv_type_signal],[Define as the return type of signal handlers
		    (`int' or `void').])

AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_FUNC_WAIT3
AC_CHECK_FUNCS([alarm atexit bzero dup2 getcwd gethostbyaddr gethostbyname gethostname getpagesize getpass gettimeofday inet_ntoa isascii localtime_r memchr memmove memset pow select socket sqrt strcasecmp strchr strcspn strdup strerror strncasecmp strpbrk strrchr strspn strstr strtol strtoul uname])


#######################################
#
#	config.h defines
#
AC_DEFINE_UNQUOTED(LOCALSTATEDIR,"$( eval echo $localstatedir )","local state directory")
AC_DEFINE_UNQUOTED(SYSCONFDIR,"$( eval echo $sysconfdir )","system configuration directory")
AC_DEFINE(__VERSION_NR__,VERSION,"Version Number")


#######################################
#
#	CFLAGS
#
if test "$DEBUGGING_ENABLED" = yes; then
	CFLAGS_DEFS="$CFLAGS_DEFS -DDEBUGGING=1"
fi
if test "$STATIC_ENABLED" = yes; then
	CFLAGS_DEFS="$CFLAGS_DEFS -static"
fi
CFLAGS="$CFLAGS -g -fno-strict-aliasing -fPIC -Wall"
CFLAGS="$CFLAGS -D_GNU_SOURCE"
CFLAGS="$CFLAGS $CFLAGS_DEFS"


#
#	find all Makefiles
#
#AC_CONFIG_FILES($( find . -name Makefile.am -printf "%h/Makefile " ))
AC_CONFIG_FILES([ \
			Makefile \
			src/Makefile \
			src/util/Makefile \
			src/fritzboxfon/Makefile \
			src/phonenumber/Makefile \
			src/event/Makefile \
			src/lookup/Makefile \
			src/notify/Makefile \
			src/log/Makefile \
			scripts/Makefile \
				  ])

AC_OUTPUT

#
#	summarize the configuration
#
echo "LDAP support        : $LDAP_ENABLED"
echo "MYSQL support       : $MYSQL_ENABLED"
echo "CURL support        : $CURL_ENABLED"
echo "logging to file     : $L2F_LOGGING_FILE"
echo "logging to mysql    : $L2F_LOGGING_MYSQL"
echo "logging to syslog   : $L2F_LOGGING_SYSLOG"
echo "lookup via ldap     : $L2F_LOOKUP_LDAP"
echo "lookup via mysql    : $L2F_LOOKUP_MYSQL"
echo "lookup via script   : $L2F_LOOKUP_SCRIPT"
echo "notify via dreambox : $L2F_NOTIFY_DREAMBOX"
echo "notify via irc      : $L2F_NOTIFY_IRC"
echo "notify via mail     : $L2F_NOTIFY_MAIL"
echo "notify via script   : $L2F_NOTIFY_SCRIPT"
echo "notify via callback : $L2F_NOTIFY_CALLBACK"
