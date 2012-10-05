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

#include "datahandler.h"

DataHandler::DataHandler()
{
}

DataHandler::~DataHandler()
{
}

DataHandler::DataHandler(MibTree* tree, Logger* s_logger)
{
	general_log_message_format = DH_GENERAL_LOG_MESSAGE_DEFAULT;
	all_data_oids_log_format = DH_GENERAL_ALL_OIDS_LOG_MESSAGE_DEFAULT;
	mib_tree = tree;
	logger = s_logger;
}


int DataHandler::load_custom_oid_base(custom_oid_base* oid_base)
{
	custom_oid_base * tmp_oid = &user_oids_root;
	std::vector<int> oid = SnmpMsg::string2oid(oid_base->base);
	
	for (std::vector<int>::iterator it = oid.begin(); it < oid.end(); ++it)
	{
		if(tmp_oid->children[*it] == NULL)
		{
			tmp_oid->children[*it] = new custom_oid_base;
		}
		tmp_oid->children[*it]->subid = *it;
		tmp_oid = tmp_oid->children[*it];
	}
	
	tmp_oid->base = oid_base->base;
	tmp_oid->name = oid_base->name;
	tmp_oid->log_type = oid_base->log_type ;
	tmp_oid->log_syslog_facility = oid_base->log_syslog_facility;
	tmp_oid->log_syslog_priority = oid_base->log_syslog_priority;
	tmp_oid->log_dir = oid_base->log_dir;
	tmp_oid->log_filename_format = oid_base->log_filename_format;
	tmp_oid->log_message_format = oid_base->log_message_format;
	tmp_oid->log_all_data_oids_format = oid_base->log_all_data_oids_format;
	tmp_oid->trap_oid = oid_base->trap_oid;
	tmp_oid->data_oid = oid_base->data_oid;
	
	
}
custom_oid_base* DataHandler::find_custom_oid(std::vector<int> &oid)
{
	custom_oid_base * tmp_oid = &user_oids_root;
	for(std::vector<int>::iterator it = oid.begin(); it != oid.end(); ++it)
	{
		if(tmp_oid->children[*it] == NULL)
		{
			return NULL;
		}
		if (tmp_oid->children[*it]->base !="")
		{
#ifdef DEBUG
			std::cout << "base founded " << tmp_oid->children[*it]->base << "\n" ;
#endif
			return tmp_oid->children[*it];
		}
		tmp_oid = tmp_oid->children[*it];
	}
}

int DataHandler::handle_message(u_char* socket_buffer,in_addr ia_agent_ip)
{
	/* TODO complete refactoring of this piece of code:
	 * remove CopyPasting,
	 * split code to several little functions,
	 * make iteration loop over array of token=>refernce-to-function-call pairs, instead of multiple replace_all() calls */
	
	std::pair<int,SnmpMsg*> rpair = SnmpMsg::make_pdu((u_char*)socket_buffer);
	if(rpair.first < 0)
		return -1;
	SnmpMsg *snmp_pdu = rpair.second;
	//std::vector<varbind*> varbindlist = snmp_pdu.get_var_bind_list();
	std::vector<varbind*> varbindlist = snmp_pdu->get_var_bind_list();
	custom_oid_base *oid_handler;
	
	//std::vector<int> trap_oid = snmp_pdu.get_trap_oid();
	std::vector<int> trap_oid = snmp_pdu->get_trap_oid();
	
	std::string trapoid_str = SnmpMsg::oid2string(trap_oid);
	std::string tmp_str;
	/*first check if custom OID handler exists for received trap*/
	oid_handler = find_custom_oid(trap_oid);
	std::string log_message;
	if(oid_handler)
	{
		log_message = oid_handler->log_message_format;
		/*iterate over varbind list and replace tokens to values in each varbind*/
		if(log_message.find(DH_DATA_OIDS_ALL_TOKEN) != std::string::npos)
		{
			std::string log_all_data_oids_str = "";
			
			for(std::vector<varbind*>::iterator it = varbindlist.begin(); it!= varbindlist.end(); ++it)
			{
				tmp_str = oid_handler->log_all_data_oids_format;
#ifdef DEBUG
				//std::cout << SnmpMsg::oid2string((*it)->oid) << " " << oid_handler->data_oid[SnmpMsg::oid2string((*it)->oid)] << "\n" ;
#endif
				if(tmp_str.find(DH_DATA_OID_DESCR_TOKEN) != std::string::npos )
				{
					std::string data_oid_descr = find_data_oid_descr((*it)->oid, oid_handler);
					if(oid_handler->data_oid.count(data_oid_descr) > 0)
					{
						replace_all (tmp_str, DH_DATA_OID_DESCR_TOKEN,  oid_handler->data_oid[data_oid_descr]);
					}
					else
					{
						replace_all (tmp_str, DH_DATA_OID_DESCR_TOKEN, std::string(DH_DATA_OID_TOKEN) + " : " + DH_DATA_OID_VAL_TOKEN );
					}
				}
				replace_all (tmp_str, DH_DATA_OID_TOKEN,  mib_tree->get_oid_description((*it)->oid));
				replace_all (tmp_str, DH_DATA_OID_VAL_TOKEN,  SnmpMsg::varbind_value2string((*it)->value,(*it)->type));
				log_all_data_oids_str+=tmp_str;
			}
			replace_all (log_message, DH_DATA_OIDS_ALL_TOKEN, log_all_data_oids_str);
		}
		
		/* check if user-defined description exists for receved trapOID */
		if(oid_handler->trap_oid.count(trapoid_str) > 0)
		{
			replace_all (log_message, DH_TRAP_OID_DESCR_TOKEN,  oid_handler->trap_oid[trapoid_str]);
		}
	}
	/* user defined OID handler does exists, use default format for log message */
	else
	{
		log_message = general_log_message_format;
		if(log_message.find(DH_DATA_OIDS_ALL_TOKEN) != std::string::npos)
		{
			std::string log_all_data_oids_str = "";
			
			for(std::vector<varbind*>::iterator it = varbindlist.begin(); it!= varbindlist.end(); ++it)
			{
				tmp_str = all_data_oids_log_format;
				replace_all (tmp_str, DH_DATA_OID_TOKEN,  mib_tree->get_oid_description((*it)->oid));
				replace_all (tmp_str, DH_DATA_OID_VAL_TOKEN,  SnmpMsg::varbind_value2string((*it)->value,(*it)->type));
				log_all_data_oids_str+=tmp_str;
			}
			replace_all (log_message, DH_DATA_OIDS_ALL_TOKEN, log_all_data_oids_str);
		}
	}
	
	/*TODO: make iteration loop over array of token=>refernce-to-function-call pairs, instead of multiple replace_all() calls*/
	if(log_message.find(DH_TRAP_OID_TOKEN) != std::string::npos)
	{
		replace_all (log_message, DH_TRAP_OID_TOKEN, mib_tree->get_oid_description(trap_oid));
	}
	
	if (log_message.find(DH_AGENT_IP_TOKEN) != std::string::npos)
	{
		replace_all (log_message, DH_AGENT_IP_TOKEN,  inet_ntoa(ia_agent_ip));
	}
	
	if(log_message.find(DH_AGENT_HOSTNAME_TOKEN) != std::string::npos)
	{
		hostent *he = gethostbyaddr(static_cast<const void*>(&ia_agent_ip.s_addr),sizeof(ia_agent_ip.s_addr),AF_INET);
		if(he)
		{
			replace_all (log_message, DH_AGENT_HOSTNAME_TOKEN,  he->h_name);
		}
		else
		{
			replace_all (log_message, DH_AGENT_HOSTNAME_TOKEN,  inet_ntoa(ia_agent_ip));
		}		
	}
	if(log_message.find(DH_SYSUPTIME_TOKEN) != std::string::npos)
	{
		std::stringstream ss;
		uint32_t uptime = snmp_pdu->get_sys_uptime();
		int days=0;
		if (uptime > 8640000)
		{
			days = uptime/8640000;
			ss << days << " days, ";
		}
		int hours = (uptime/360000)-(days * 24);
		int minutes = (uptime/6000) - (days * 1440) - (hours * 60);
		int seconds = (uptime/100) - (days * 86400) - (hours * 3600) - (minutes*60);
		int low_seconds = uptime%100;
		
		ss << hours << ":" ;
		if(minutes<10)
			ss<< 0;
		ss << minutes << ":";
		if(seconds<10)
			ss << 0;
		ss << seconds << "." << low_seconds;
		
		replace_all (log_message, DH_SYSUPTIME_TOKEN, ss.str() );
	}
	
	if(log_message.find(DH_ENTERPRISE_OID_TOKEN) != std::string::npos)
	{
		std::vector<int> enterprise_oid = snmp_pdu->get_enterprise_oid();
		//replace_all (log_message, DH_ENTERPRISE_OID_TOKEN, SnmpMsg::oid2string(enterprise_oid) );
		if(enterprise_oid.size()>1)
		{
			replace_all (log_message, DH_ENTERPRISE_OID_TOKEN, mib_tree->get_oid_description(enterprise_oid) );
		}
		else
		{
			replace_all (log_message, DH_ENTERPRISE_OID_TOKEN, "none" );
		}
		
	}
	
	/*replace "\\n" to '\n'*/
	replace_all (log_message, "\\n", "\n" );
	
	
	
	if(oid_handler)
	{
		switch(oid_handler->log_type)
		{
		case LOGGER_MAIN_FILE:
			logger->log_message(log_message);
			break;
		case LOGGER_SYSLOG:
			logger->log_message(oid_handler->log_syslog_facility,oid_handler->log_syslog_priority, log_message);
			break;
		case LOGGER_SEPARATE_FILE:
		{
			std::string sep_file = oid_handler->log_dir + oid_handler->log_filename_format;
			if (sep_file.find(DH_AGENT_IP_TOKEN) != std::string::npos)
			{
				replace_all (sep_file, DH_AGENT_IP_TOKEN,  inet_ntoa(ia_agent_ip));
			}
			
			if(sep_file.find(DH_AGENT_HOSTNAME_TOKEN) != std::string::npos)
			{
				hostent *he = gethostbyaddr(static_cast<const void*>(&ia_agent_ip.s_addr),sizeof(ia_agent_ip.s_addr),AF_INET);
				if(he)
				{
					replace_all (sep_file, DH_AGENT_HOSTNAME_TOKEN,  he->h_name);
				}
				else
				{
					replace_all (sep_file, DH_AGENT_HOSTNAME_TOKEN,  inet_ntoa(ia_agent_ip));
				}
				
			}
			logger->log_message(log_message,sep_file);
			
		}
		break;
		case LOGGER_ALL:
			logger->log_message_global(log_message);
			break;
		default:
			break;
		}
	}
	else
	{
		logger->log_message_global(log_message);
	}
	
	/*cleanup memory */
	delete snmp_pdu;
	
}

void replace_all(std::string &source, const std::string& past, const std::string& now)
{
	int pos = source.find(past);
	while(pos !=std::string::npos)
	{
		source.replace(pos,past.size(),now);
		pos = source.find(past);
	}
}
void DataHandler::set_general_log_message_format(std::string& general_log_message)
{
	general_log_message_format = general_log_message;
}
void DataHandler::set_all_data_oids_log_format(std::string& all_data_oids_message)
{
	all_data_oids_log_format = all_data_oids_message;
}
std::string DataHandler::find_data_oid_descr(std::vector<int>& oid, custom_oid_base * oid_base)
{
	std::stringstream ss;
	for(std::vector<int>::iterator it = oid.begin(); it != oid.end(); ++it)
	{
		ss << "." << *it ;
		if (oid_base->data_oid.count(ss.str()) > 0)
		{
			return ss.str();
		}
	}
	
	return "";
}
