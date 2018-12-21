/* include/log4cplus/config/defines.hxx.  Generated from defines.hxx.in by configure.  */
#ifndef LOG4CPLUS_CONFIG_DEFINES_HXX
#define LOG4CPLUS_CONFIG_DEFINES_HXX

/* */
#define LOG4CPLUS_HAVE_SYSLOG_H 1

/* */
#define LOG4CPLUS_HAVE_ARPA_INET_H 1

/* */
#define LOG4CPLUS_HAVE_NETINET_IN_H 1

/* */
#define LOG4CPLUS_HAVE_NETINET_TCP_H 1

/* */
#define LOG4CPLUS_HAVE_SYS_TIMEB_H 1

/* */
#define LOG4CPLUS_HAVE_SYS_TIME_H 1

/* */
#define LOG4CPLUS_HAVE_SYS_TYPES_H 1

/* */
#define LOG4CPLUS_HAVE_SYS_STAT_H 1

/* */
#define LOG4CPLUS_HAVE_SYS_SYSCALL_H 1

/* */
#define LOG4CPLUS_HAVE_SYS_FILE_H 1

/* */
#define LOG4CPLUS_HAVE_TIME_H 1

/* */
#define LOG4CPLUS_HAVE_SYS_SOCKET_H 1

/* */
#define LOG4CPLUS_HAVE_NETDB_H 1

/* */
#define LOG4CPLUS_HAVE_UNISTD_H 1

/* */
#define LOG4CPLUS_HAVE_FCNTL_H 1

/* */
#define LOG4CPLUS_HAVE_STDARG_H 1

/* */
#define LOG4CPLUS_HAVE_STDIO_H 1

/* */
#define LOG4CPLUS_HAVE_STDLIB_H 1

/* */
#define LOG4CPLUS_HAVE_ERRNO_H 1

/* */
#define LOG4CPLUS_HAVE_WCHAR_H 1

/* */
/* #undef LOG4CPLUS_HAVE_ICONV_H */

/* */
#define LOG4CPLUS_HAVE_LIMITS_H 1

/* */
#define LOG4CPLUS_HAVE_FTIME 1

/* */
#define LOG4CPLUS_HAVE_GETADDRINFO 1

/* */
#define LOG4CPLUS_HAVE_GETHOSTBYNAME_R 1

/* */
#define LOG4CPLUS_HAVE_GETPID 1

/* */
#define LOG4CPLUS_HAVE_GETTIMEOFDAY 1

/* Define to 1 if you have the `clock_gettime' function. */
#define LOG4CPLUS_HAVE_CLOCK_GETTIME 1

/* Define to 1 if you have the `nanosleep' function. */
#define LOG4CPLUS_HAVE_NANOSLEEP 1

/* Define to 1 if you have the `clock_nanosleep' function. */
#define LOG4CPLUS_HAVE_CLOCK_NANOSLEEP 1

/* */
#define LOG4CPLUS_HAVE_GMTIME_R 1

/* */
#define LOG4CPLUS_HAVE_HTONL 1

/* */
#define LOG4CPLUS_HAVE_HTONS 1

/* */
#define LOG4CPLUS_HAVE_LOCALTIME_R 1

/* */
#define LOG4CPLUS_HAVE_LSTAT 1

/* */
#define LOG4CPLUS_HAVE_FCNTL 1

/* */
#define LOG4CPLUS_HAVE_LOCKF 1

/* */
#define LOG4CPLUS_HAVE_FLOCK 1

/* */
#define LOG4CPLUS_HAVE_NTOHL 1

/* */
#define LOG4CPLUS_HAVE_NTOHS 1

/* Define to 1 if you have the `shutdown' function. */
#define LOG4CPLUS_HAVE_SHUTDOWN 1

/* */
#define LOG4CPLUS_HAVE_PIPE 1

/* */
#define LOG4CPLUS_HAVE_PIPE2 1

/* */
#define LOG4CPLUS_HAVE_POLL 1

/* */
#define LOG4CPLUS_HAVE_POLL_H 1

/* */
#define LOG4CPLUS_HAVE_STAT 1

/* Define if this is a single-threaded library. */
/* #undef LOG4CPLUS_SINGLE_THREADED */

/* */
/* #undef LOG4CPLUS_USE_PTHREADS */

/* Define for compilers/standard libraries that support more than just the "C"
   locale. */
/* #undef LOG4CPLUS_WORKING_LOCALE */

/* Define for C99 compilers/standard libraries that support more than just the
   "C" locale. */
/* #undef LOG4CPLUS_WORKING_C_LOCALE */

/* Define to int if undefined. */
/* #undef socklen_t */

/* Defined for --enable-debugging builds. */
/* #undef LOG4CPLUS_DEBUGGING */

/* Defined if the compiler understands __declspec(dllexport) or
   __attribute__((visibility("default"))) construct. */
#define LOG4CPLUS_DECLSPEC_EXPORT __attribute__ ((visibility("default")))

/* Defined if the compiler understands __declspec(dllimport) or
   __attribute__((visibility("default"))) construct. */
#define LOG4CPLUS_DECLSPEC_IMPORT __attribute__ ((visibility("default")))

/* Defined if the compiler understands
   __attribute__((visibility("hidden"))) construct. */
#define LOG4CPLUS_DECLSPEC_PRIVATE __attribute__ ((visibility("hidden")))

/* */
#define LOG4CPLUS_HAVE_TLS_SUPPORT 1

/* */
#define LOG4CPLUS_THREAD_LOCAL_VAR __thread

/* Defined if the host OS provides ENAMETOOLONG errno value. */
#define LOG4CPLUS_HAVE_ENAMETOOLONG 1

/* Defined if the compiler provides __sync_add_and_fetch(). */
#define LOG4CPLUS_HAVE___SYNC_ADD_AND_FETCH 1

/* Defined if the compiler provides __sync_sub_and_fetch(). */
#define LOG4CPLUS_HAVE___SYNC_SUB_AND_FETCH 1

/* Defined if the compiler provides C++11 <atomic> header and increment,
   decrement operations. */
/* #undef LOG4CPLUS_HAVE_CXX11_ATOMICS */

/* */
#define LOG4CPLUS_HAVE_C99_VARIADIC_MACROS 1

/* */
#define LOG4CPLUS_HAVE_GNU_VARIADIC_MACROS 1

/* */
#define LOG4CPLUS_HAVE_VSNPRINTF 1

/* Define to 1 if you have the `vsnwprintf' function. */
/* #undef LOG4CPLUS_HAVE_VSNWPRINTF */

/* Define to 1 if you have the `_vsnwprintf' function. */
/* #undef LOG4CPLUS_HAVE__VSNWPRINTF */

/* */
/* #undef LOG4CPLUS_HAVE__VSNPRINTF */

/* Define to 1 if you have the `vfprintf_s' function. */
/* #undef LOG4CPLUS_HAVE_VFPRINTF_S */

/* Define to 1 if you have the `vfwprintf_s' function. */
/* #undef LOG4CPLUS_HAVE_VFWPRINTF_S */

/* Define to 1 if you have the `vsprintf_s' function. */
/* #undef LOG4CPLUS_HAVE_VSPRINTF_S */

/* Define to 1 if you have the `vswprintf_s' function. */
/* #undef LOG4CPLUS_HAVE_VSWPRINTF_S */

/* Define to 1 if you have the `_vsnprintf_s' function. */
/* #undef LOG4CPLUS_HAVE__VSNPRINTF_S */

/* Define to 1 if you have the `_vsnwprintf_s' function. */
/* #undef LOG4CPLUS_HAVE__VSNWPRINTF_S */

/* Defined if the compiler supports __FUNCTION__ macro. */
#define LOG4CPLUS_HAVE_FUNCTION_MACRO 1

/* Defined if the compiler supports __PRETTY_FUNCTION__ macro. */
#define LOG4CPLUS_HAVE_PRETTY_FUNCTION_MACRO 1

/* Defined if the compiler supports __func__ symbol. */
#define LOG4CPLUS_HAVE_FUNC_SYMBOL 1

/* Define to 1 if you have the `mbstowcs' function. */
#define LOG4CPLUS_HAVE_MBSTOWCS 1

/* Define to 1 if you have the `wcstombs' function. */
#define LOG4CPLUS_HAVE_WCSTOMBS 1

/* Define to 1 if you have Linux style syscall(SYS_gettid). */
#define LOG4CPLUS_HAVE_GETTID 1

/* Define when iconv() is available. */
/* #undef LOG4CPLUS_WITH_ICONV */

/* Define to 1 if you have the `iconv' function. */
/* #undef LOG4CPLUS_HAVE_ICONV */

/* Define to 1 if you have the `iconv_close' function. */
/* #undef LOG4CPLUS_HAVE_ICONV_CLOSE */

/* Define to 1 if you have the `iconv_open' function. */
/* #undef LOG4CPLUS_HAVE_ICONV_OPEN */

/* Define to 1 if the system has the `constructor' function attribute */
#define LOG4CPLUS_HAVE_FUNC_ATTRIBUTE_CONSTRUCTOR 1

/* Define to 1 if the system has the `init_priority' variable attribute */
#define LOG4CPLUS_HAVE_VAR_ATTRIBUTE_INIT_PRIORITY 1

#endif // LOG4CPLUS_CONFIG_DEFINES_HXX
