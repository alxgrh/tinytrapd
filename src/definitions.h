/*
*  Copyright (c) 2012 Alexey Grachev <alxgrh [at] yandex [dot] ru>
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*  1. Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*  2. Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*
*  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ''AS IS'' AND
*  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
*  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
*  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
*  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
*  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
*  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
*  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
*  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
*  SUCH DAMAGE.
*/

/*                   BOOST DISCLAIMER
*   Distributed under the Boost Software License, Version 1.0.
*    (See accompanying file LICENSE_1_0.txt or copy at
*          http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <syslog.h>

#define BUFLEN 1500
#define PORT 162


#define DEFAULT_USER "alexey"
#define DEFAULTS_SOCKET_THREADS 2
#define DEFAULTS_MAX_SOCKET_THREADS 32
#define DEFAULT_MIB_MIBDIR "/etc/tinytrapd/mibs"
#define DEFAULT_MIB_MIBFILES "rfc1151-smi.mib rfc1213-mib.mib if-mib.mib "
#define DEFAULT_SYSLOG_FACILITY LOG_DAEMON
#define DEFAULT_SYSLOG_PRIORITY LOG_INFO


/*config file parameters names*/
#define CONFIG_MAIN_PORT_STR "main.port"
#define CONFIG_MAIN_USER_STR "main.user"
#define CONFIG_MAIN_SOCKET_THREADS_STR "main.socket_threads"
#define CONFIG_MIB_MIBDIR "mib.mibdir"
#define CONFIG_MIB_MIBFILES "mib.mibfiles"


#define CONFIG_LOG_USESYSLOG_STR "log.use_syslog"
#define CONFIG_LOG_FACILITY_STR "log.facility"
#define CONFIG_LOG_PRIORITY_STR "log.priority"
#define CONFIG_LOG_USEMAINFILE_STR "log.use_main_file"
#define CONFIG_LOG_MAINFILE_STR "log.main_file"
#define CONFIG_LOG_GENERAL_MESSAGE_STR "log.general_message_format"
#define CONFIG_LOG_ALL_OIDS_MESSAGE_STR "log.all_data_oids_format"


#define CONFIG_CUSTOM_STR "custom_oid_"
#define CONFIG_CUSTOM_LOGDIR_STR "log_dir"
#define CONFIG_CUSTOM_LOGTYPE_STR "log_type"
#define CONFIG_CUSTOM_LOGTYPE_GLOBAL "global"
#define CONFIG_CUSTOM_LOGTYPE_SYSLOG "syslog"
#define CONFIG_CUSTOM_LOGTYPE_MAINFILE "main_file"
#define CONFIG_CUSTOM_LOGTYPE_SEPARATEFILE "separate_file"

#define CONFIG_CUSTOM_SYSLOGFACILITY_STR "syslog_facility"
#define CONFIG_CUSTOM_SYSLOGPRIORITY_STR "syslog_priority"

#define CONFIG_CUSTOM_LOGTYPE_STR "log_type"
#define CONFIG_CUSTOM_LOGFILENAMEFORMAT_STR "log_filename_format"
#define CONFIG_CUSTOM_LOGPREFIX_STR "log_prefix"
#define CONFIG_CUSTOM_LOGMESSAGEFORMAT_STR "log_message_format"
#define CONFIG_CUSTOM_LOGALLDATAOIDFORMAT_STR "log_all_data_oids_format"
#define CONFIG_CUSTOM_BASE_STR "base"
#define CONFIG_CUSTOM_TRAPOID_STR "trap_oid_"
#define CONFIG_CUSTOM_DATAOID_STR "data_oid_"

#endif
