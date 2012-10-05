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
*  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS S IS'' AND
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

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#include <boost/program_options.hpp>
#include <boost/program_options/detail/config_file.hpp>
#include <boost/program_options/parsers.hpp>

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <string>
#include <syslog.h>

#include "snmpmsg.h"
#include "mibtree.h"
#include "datahandler.h"
#include "sockethandler.h"
#include "definitions.h"

namespace po = boost::program_options;

void usage(const po::options_description& desc)
{
	std::cout << "Tiny SNMP trap logger." << std::endl<< desc;
}

void unquote_string(std::string &str)
{
	if (str.at(0) == '\"' && str.at(str.size()-1) == '\"' )
	{
		str = str.substr(1,str.size()-2);
	}
}

int main(int argc, char **argv)
{
#ifdef DEBUG
	openlog("tinytrapd", LOG_PID|LOG_CONS, LOG_DAEMON);
	syslog(LOG_DEBUG, "Alive ... ");
	closelog();
#endif

	std::string config_file_name;
	int port;
	
	/*Get command line options*/
	po::options_description desc("Allowed options");
	desc.add_options()
	("help,h", "Display this message")
	("config-file,c",po::value<std::string>(&config_file_name)->default_value("/etc/tinytrapd/tinytrapd.conf","/etc/tinytrapd/tinytrapd.conf (default)"))
	;
	
	po::variables_map vm;
	try
	{
		po::store(po::parse_command_line(argc, argv, desc), vm);
	}
	catch(std::exception& e)
	{
		std::cout << e.what() << std::endl;
		usage (desc);
		return 0;
	}
	
	po::notify(vm);
	if(vm.count("help") )
	{
		usage(desc);
		return 0;
	}
	
	/* load parameters from config file */
	std::ifstream config(config_file_name.c_str());
	if(!config)
	{
		std::cerr<<"Unable to open config file " << config_file_name << std::endl;
		return -1;
	}
	
	std::set<std::string> options;
	std::map<std::string, std::string> parameters;
	options.insert("*");
	try
	{
		for (po::detail::config_file_iterator i(config, options), e ; i != e; ++i)
		{
			parameters[i->string_key] = i->value[0];
			unquote_string(parameters[i->string_key]);
		}
	}
	catch(std::exception& e)
	{
		std::cerr<<"Unable to parse config file. Exception throwed: "<<e.what()<<std::endl;
		return -1;
	}
	
	/* daeminize */
#ifndef DEBUG
	daemon(1,1);
#endif
	/* ****************************
	 * Initialize logging subsytem BEGIN
	 * ****************************/
	Logger logger;
	int logger_init_result;
	std::map<std::string, int> log_priority_mappings;
	log_priority_mappings["alert"] = LOG_ALERT;
	log_priority_mappings["crit"] = LOG_CRIT;
	log_priority_mappings["debug"] = LOG_DEBUG;
	log_priority_mappings["emerg"] = LOG_EMERG;
	log_priority_mappings["err"] = LOG_ERR;
	log_priority_mappings["error"] = LOG_ERR;
	log_priority_mappings["info"] = LOG_INFO;
	log_priority_mappings["notice"] = LOG_NOTICE;
	log_priority_mappings["panic"] = LOG_EMERG;
	log_priority_mappings["warn"] = LOG_WARNING;
	log_priority_mappings["warning"] = LOG_WARNING;
	
	std::map<std::string, int> log_facility_mappings;
	log_facility_mappings["authpriv"] = LOG_AUTHPRIV;
	log_facility_mappings["cron"] = LOG_CRON;
	log_facility_mappings["daemon"] = LOG_DAEMON;
	log_facility_mappings["ftp"] = LOG_FTP;
	log_facility_mappings["kern"] = LOG_KERN;
	log_facility_mappings["lpr"] = LOG_LPR;
	log_facility_mappings["mail"] = LOG_MAIL;
	log_facility_mappings["news"] = LOG_NEWS;
	log_facility_mappings["security"] = LOG_AUTH;
	log_facility_mappings["syslog"] = LOG_SYSLOG;
	log_facility_mappings["user"] = LOG_USER;
	log_facility_mappings["uucp"] = LOG_UUCP;
	log_facility_mappings["local0"] = LOG_LOCAL0;
	log_facility_mappings["local1"] = LOG_LOCAL1;
	log_facility_mappings["local2"] = LOG_LOCAL2;
	log_facility_mappings["local3"] = LOG_LOCAL3;
	log_facility_mappings["local4"] = LOG_LOCAL4;
	log_facility_mappings["local5"] = LOG_LOCAL5;
	log_facility_mappings["local6"] = LOG_LOCAL6;
	log_facility_mappings["local7"] = LOG_LOCAL7;
	/*default facility and priority*/
	int facility = LOG_DAEMON, priority = LOG_INFO;
	if(parameters.count(CONFIG_LOG_USESYSLOG_STR) > 0)
	{
	
		if(log_facility_mappings.count(parameters[CONFIG_LOG_FACILITY_STR]))
		{
			facility = log_facility_mappings[parameters[CONFIG_LOG_FACILITY_STR]];
			parameters.erase(CONFIG_LOG_FACILITY_STR);
		}
		if(log_priority_mappings.count(parameters[CONFIG_LOG_PRIORITY_STR]))
		{
			priority = log_priority_mappings[parameters[CONFIG_LOG_PRIORITY_STR]];
			parameters.erase(CONFIG_LOG_PRIORITY_STR);
		}
		
		/*if both syslog and logfile is used*/
		if(parameters.count(CONFIG_LOG_USEMAINFILE_STR) > 0)
		{
			if(parameters.count(CONFIG_LOG_MAINFILE_STR) > 0)
			{
				logger_init_result = logger.init_logger(facility,priority,parameters[CONFIG_LOG_MAINFILE_STR]);
				parameters.erase(CONFIG_LOG_MAINFILE_STR);
			} 
			else
			{
				logger_init_result = logger.init_logger(facility,priority,LOGGER_DEFAULT_FILE);
			}
			
			parameters.erase(CONFIG_LOG_USEMAINFILE_STR);
		}
		/* if only syslog used */
		else
		{
			logger_init_result = logger.init_logger(facility,priority);
		}
		parameters.erase(CONFIG_LOG_USESYSLOG_STR);
		
	}
	/* if only log file used*/
	else if(parameters.count(CONFIG_LOG_USEMAINFILE_STR) > 0)
	{
		if(parameters.count(CONFIG_LOG_MAINFILE_STR) > 0)
		{
			logger_init_result = logger.init_logger(parameters[CONFIG_LOG_MAINFILE_STR]);
			parameters.erase(CONFIG_LOG_MAINFILE_STR);
		}
		else
		{
			logger_init_result = logger.init_logger(LOGGER_DEFAULT_FILE);
		}
		
		parameters.erase(CONFIG_LOG_USEMAINFILE_STR);
	}
	/* no default logging */
	else
	{
		logger_init_result = logger.init_logger();
	}
	
	if(logger_init_result <0)
	{
		std::cerr << "Unable to initialize logger subsystem\n";
		return -1;
	}
	
	if(logger.start_queue_thread()<0)
	{
		std::cerr << "Unable to start logger queue handler\n";
		return -1;
	}

	
	/* ****************************
	 * Initialize logging subsytem END
	 * ****************************/
	
	
	
	
	/* ********************************************
	 * Initialize MIB tree and datahandler BEGIN
	 * ********************************************/
	MibTree *mib_tree = new MibTree();
	std::string mibdir;
	if(parameters.count(CONFIG_MIB_MIBDIR)>0){
		mibdir = parameters[CONFIG_MIB_MIBDIR];
	}
	else{
		mibdir = DEFAULT_MIB_MIBDIR;
	}
	std::string mibfiles;
	if(parameters.count(CONFIG_MIB_MIBFILES)>0){
		mibfiles = parameters[CONFIG_MIB_MIBFILES];
	}
	else{
		mibfiles = DEFAULT_MIB_MIBFILES;
	}
	
	typedef boost::tokenizer< boost::char_separator<char> > Tokenizer;
	boost::char_separator<char> sep(" ");
	
	Tokenizer tok = Tokenizer(mibfiles,sep);
	MibModule m_module;
	for (Tokenizer::iterator it = tok.begin(); it!= tok.end(); ++it) {
		m_module.load_from_file(mibdir + "/" + *it);
		mib_tree->load_mib(&m_module);
	}
#ifdef DEBUG
	mib_tree->print_tree(NULL,0);
#endif
	
	
	DataHandler dhl(mib_tree, &logger);
	custom_oid_base u_oid;
	
	/* Get general log message formats */
	if(parameters.count(CONFIG_LOG_GENERAL_MESSAGE_STR) > 0){
		dhl.set_general_log_message_format(parameters[CONFIG_LOG_GENERAL_MESSAGE_STR]);
	}
	
	if(parameters.count(CONFIG_LOG_ALL_OIDS_MESSAGE_STR) > 0) {
		dhl.set_all_data_oids_log_format(parameters[CONFIG_LOG_ALL_OIDS_MESSAGE_STR]);
	}
	
	std::string tmp_str,tmp_str2;
	for(std::map<std::string,std::string>::iterator it = parameters.begin(); it != parameters.end(); it++)
	{
		std::string curr_param = (*it).first;
		
		/* if founded user defined OID handler */
		if(curr_param.find(CONFIG_CUSTOM_STR) != std::string::npos && curr_param.find('.') != std::string::npos)
		{
			u_oid = custom_oid_base();
			/* get name of user defined OID handler*/
			u_oid.name = curr_param.substr(sizeof(CONFIG_CUSTOM_STR)-1, curr_param.find('.') - sizeof(CONFIG_CUSTOM_STR) + 1);
			
			while((*it).first.find(CONFIG_CUSTOM_STR + u_oid.name) != std::string::npos)
			{
				curr_param = (*it).first;
				/* get defined-name parameters for user defined OID handler*/
				if(curr_param == CONFIG_CUSTOM_STR + u_oid.name + "." + CONFIG_CUSTOM_BASE_STR)
				{
					u_oid.base = parameters[curr_param];
					
				}
				if(curr_param == CONFIG_CUSTOM_STR + u_oid.name + "." + CONFIG_CUSTOM_LOGDIR_STR)
				{
					u_oid.log_dir = parameters[curr_param];
					
				}
				
				if(curr_param == CONFIG_CUSTOM_STR + u_oid.name + "." + CONFIG_CUSTOM_LOGFILENAMEFORMAT_STR)
				{
					u_oid.log_filename_format = parameters[curr_param];
					
				}
				if(curr_param == CONFIG_CUSTOM_STR + u_oid.name + "." + CONFIG_CUSTOM_LOGMESSAGEFORMAT_STR)
				{
					u_oid.log_message_format = parameters[curr_param];
					
				}
				if(curr_param == CONFIG_CUSTOM_STR + u_oid.name + "." + CONFIG_CUSTOM_LOGALLDATAOIDFORMAT_STR)
				{
					u_oid.log_all_data_oids_format = parameters[curr_param];
					
				}
				if(curr_param == CONFIG_CUSTOM_STR + u_oid.name + "." + CONFIG_CUSTOM_LOGPREFIX_STR)
				{
					u_oid.log_prefix = parameters[curr_param];
					
				}
				if(curr_param == CONFIG_CUSTOM_STR + u_oid.name + "." + CONFIG_CUSTOM_LOGTYPE_STR)
				{
				
					if(parameters[curr_param] == CONFIG_CUSTOM_LOGTYPE_MAINFILE )
						u_oid.log_type = LOGGER_MAIN_FILE;
					else if (parameters[curr_param] == CONFIG_CUSTOM_LOGTYPE_SEPARATEFILE)
						u_oid.log_type = LOGGER_SEPARATE_FILE;
					else if (parameters[curr_param] == CONFIG_CUSTOM_LOGTYPE_SYSLOG)
						u_oid.log_type = LOGGER_SYSLOG;
					else if (parameters[curr_param] == CONFIG_CUSTOM_LOGTYPE_GLOBAL)
						u_oid.log_type = LOGGER_ALL;
					else
						u_oid.log_type = LOGGER_NONE;
						
				}
				if(curr_param == CONFIG_CUSTOM_STR + u_oid.name + "." + CONFIG_CUSTOM_SYSLOGFACILITY_STR)
				{
					if(log_facility_mappings.count(parameters[curr_param]))
					{
						u_oid.log_syslog_facility = log_facility_mappings[parameters[curr_param]];
					}
					else
					{
						u_oid.log_syslog_facility = facility;
					}
				}
				if(curr_param == CONFIG_CUSTOM_STR + u_oid.name + "." + CONFIG_CUSTOM_SYSLOGPRIORITY_STR)
				{
					if(log_priority_mappings.count(parameters[CONFIG_LOG_PRIORITY_STR]))
					{
						u_oid.log_syslog_priority = log_priority_mappings[parameters[curr_param]];
					}
					else
					{
						u_oid.log_syslog_priority = priority;
					}
				}
				
				if(log_priority_mappings.count(parameters[CONFIG_LOG_PRIORITY_STR]))
				{
					priority = log_priority_mappings[parameters[CONFIG_LOG_PRIORITY_STR]];
					parameters.erase(CONFIG_LOG_PRIORITY_STR);
				}
				
				/* get variable-name parameters for user defined OID handler*/
				if(curr_param.find(CONFIG_CUSTOM_STR + u_oid.name + "." + CONFIG_CUSTOM_TRAPOID_STR) != std::string::npos)
				{
					int fixed_namepart_length = (CONFIG_CUSTOM_STR + u_oid.name + "." + CONFIG_CUSTOM_TRAPOID_STR).length();
					
					if (parameters[curr_param].find(DH_TRAP_OID_DESCR_TOKEN) == std::string::npos)
					{
						unquote_string(parameters[curr_param]);
						u_oid.trap_oid[curr_param.substr(fixed_namepart_length,curr_param.length() - fixed_namepart_length)] = parameters[curr_param];
					}
					else
					{
						std::cout << "infinite recursion here" << "\n";
					}
					
					
				}
				if(curr_param.find(CONFIG_CUSTOM_STR + u_oid.name + "." + CONFIG_CUSTOM_DATAOID_STR) != std::string::npos)
				{
					int fixed_namepart_length = (CONFIG_CUSTOM_STR + u_oid.name + "." + CONFIG_CUSTOM_DATAOID_STR).length();
					
					unquote_string(parameters[curr_param]);
					u_oid.data_oid[curr_param.substr(fixed_namepart_length,curr_param.length() - fixed_namepart_length)] = parameters[curr_param];
					
				}
				
				it++;
			}
			
			dhl.load_custom_oid_base(&u_oid);
			it--;
			
		}
	}
	/* ********************************************
	 * Initialize MIB tree and datahandler END
	 * ********************************************/
	
	/* ********************************************
	* Initialize socket handler 
	* ********************************************/
	/* Get listen port */
	if(parameters.count(CONFIG_MAIN_PORT_STR) > 0)
	{
		port = strtol (parameters[CONFIG_MAIN_PORT_STR].c_str(),NULL,10);
		if(port < 1 || port > 65535)
		{
			/*Wrong port or conversion error, use default port*/
			port == PORT;
		}
		parameters.erase(CONFIG_MAIN_PORT_STR);
	}
	else
	{
		port == PORT;
	}
	
	SocketHandler shl(port,&dhl);
	if(shl.get_listen_fd()<0){
		std::string die_message = "Unable to create socket, dying...";
		logger.log_message(die_message);
		return -1;
	}
	
	/* Change owner of process to unprivileged user for security reasons */
	passwd *pwd;
	if(parameters.count(CONFIG_MAIN_USER_STR)>0){
		
		pwd = getpwnam(parameters[CONFIG_MAIN_USER_STR].c_str());
	}
	else{
		pwd = getpwnam(DEFAULT_USER);
	}
	if(!pwd){
		std::string die_message = "Unable to obtain uid for change process owner, dying...";
		logger.log_message(die_message);
		return -1;
	}
	
	setgid(pwd->pw_gid);
	setuid(pwd->pw_uid);
	
	/*If we are still running as root */
	if(getuid() == 0 || getgid() == 0){
		std::string die_message = "Unable to change process owner, dying...";
		logger.log_message(die_message);
		return -1;
	}

	/*Start socket handler queue.*
	 *Determine the number of socket handler threads. 
	 * Get it from config file or take number of CPU's or take default defined value*/
	int workers_num;
	if(parameters.count(CONFIG_MAIN_SOCKET_THREADS_STR) > 0){
		workers_num = strtol(parameters[CONFIG_MAIN_SOCKET_THREADS_STR].c_str(),NULL,10);
	}
	else {
		workers_num = sysconf(_SC_NPROCESSORS_ONLN);
	}
	
	if(workers_num < DEFAULTS_SOCKET_THREADS || workers_num > DEFAULTS_MAX_SOCKET_THREADS) {
			workers_num = DEFAULTS_SOCKET_THREADS;
	}
	
	/* This function joins working threads, so it will be blocked until socket handler's threads terminated */
	shl.start_queue(workers_num);
	
	
	return 0;
}
