#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(tinytrapd, 1.1, aag at inet)
AC_CONFIG_SRCDIR([src/logger.h])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_HEADER([config.h])


AC_LANG_CPLUSPLUS
# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

dnl extra argument: --with-mysql
WITH_SOME_PACKAGE=
AC_ARG_WITH(some_package,
[  --with-some_package[=PATH]           Compile in some_package support. (default=no)],
[ if test -x "$withval"; then
    WHERE_MYSQL=$withval
  else
    WHERE_MYSQL="/usr"
  fi
]
)

# Checks for libraries.
AC_CHECK_LIB([pthread], [pthread_create], [],  [AC_MSG_ERROR([pthread library was not found])] )
AC_CHECK_LIB([boost_program_options], [main], [echo boost_program_options library found], [AC_MSG_ERROR([boost_program_options library was not found])])

if test "x$WHERE_SOME_PACKAGE" != "x"; then
  LIBS="-L$WHERE_SOME_PACKAGE/lib/some_package"
  AC_CHECK_LIB(some_package, init,
    [
      SOME_PACKAGE_CFLAGS="-L$WHERE_SOME_PACKAGE/lib/mysql -I$WHERE_SOME_PACKAGE/include/mysql"
      SOME_PACKAGE_LIB="-lsome_package"
      AC_DEFINE([HAVE_SOME_PACKAGE], [], [enable some package])
    ]
  )
fi

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdint.h stdlib.h string.h sys/socket.h sys/time.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_CHECK_MEMBERS([struct stat.st_blksize])
AC_STRUCT_ST_BLOCKS
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_CLOSEDIR_VOID
AC_REPLACE_FNMATCH
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([gethostbyaddr inet_ntoa memset mkdir rmdir socket strerror strtol strtoul strtoull])

AC_CONFIG_FILES([Makefile
                 src/Makefile
		data/Makefile])
AC_OUTPUT
