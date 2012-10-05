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

#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <queue>
#include <set>
#include <fstream>
#include <pthread.h>
#include <ctime>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


#include "mibtree.h"
#include "snmpmsg.h"
#include "logger.h"

#define DH_GENERAL_LOG_MESSAGE_DEFAULT "trap received from %agent_hostname% SysUptime:%sysuptime% Enterprise: %enterprise% TrapOID: Varbinds: %trap_oid% %all_data_oids%"
#define DH_GENERAL_ALL_OIDS_LOG_MESSAGE_DEFAULT "%data_oid% : %value%\n"
#define DH_TRAP_OID_TOKEN "%trap_oid%"
#define DH_TRAP_OID_DESCR_TOKEN "%trap_oid_descr%"
#define DH_DATA_OID_TOKEN "%data_oid%"
#define DH_DATA_OID_DESCR_TOKEN "%data_oid_descr%"
#define DH_DATA_OIDS_ALL_TOKEN "%all_data_oids%"
#define DH_DATA_OID_VAL_TOKEN "%value%"
#define DH_AGENT_IP_TOKEN "%agent_ip%"
#define DH_AGENT_HOSTNAME_TOKEN "%agent_hostname%"
#define DH_SYSUPTIME_TOKEN "%sysuptime%"
#define DH_ENTERPRISE_OID_TOKEN "%enterprise%"


typedef const char* DH_STR_TOKENS;


static const char* fixed_tokens[] =
	{
		DH_TRAP_OID_TOKEN,
		DH_TRAP_OID_DESCR_TOKEN,
		DH_DATA_OID_TOKEN,
		DH_AGENT_IP_TOKEN,
		DH_AGENT_HOSTNAME_TOKEN,
		NULL
	};
static DH_STR_TOKENS conteiner_tokens[] = 
	{
		DH_DATA_OID_DESCR_TOKEN,
		DH_DATA_OIDS_ALL_TOKEN
	};



void replace_all(std::string& source, const std::string& past, const std::string& now);


 
struct custom_oid_base {
	int subid;
	std::string name;
	int log_type;
	int log_syslog_facility;
	int log_syslog_priority;
	std::string base;
	std::string log_dir;
	std::string log_filename_format;
	std::string log_prefix;
	std::string log_message_format;
	std::string log_all_data_oids_format;
	std::map<std::string, std::string> trap_oid;
	std::map<std::string, std::string> data_oid;
	
	std::map<int,custom_oid_base*> children;
};


class DataHandler {
	
	MibTree *mib_tree;
	custom_oid_base user_oids_root;
	std::map<std::string, std::string> custom_oid_bases;
	std::string general_log_message_format;
	std::string all_data_oids_log_format;
	Logger *logger;
	
public:
	DataHandler();
	DataHandler(MibTree * tree, Logger *s_logger);
	~DataHandler();
	void set_general_log_message_format(std::string &general_log_message);
	void set_all_data_oids_log_format(std::string &all_data_oids_message);
	int handle_message(u_char *socket_buffer, in_addr ia_agent_ip);
	int load_custom_oid_base(custom_oid_base * oid_base);
	custom_oid_base* find_custom_oid(std::vector<int> &oid);
	std::string find_data_oid_descr (std::vector<int> &oid,custom_oid_base * oid_base);
	
	

};

#endif // DATAHANDLER_H
