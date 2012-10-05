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

#include "snmpmsg.h"

SnmpMsg::SnmpMsg(int arg_version, std::string &arg_community, int arg_pdu_type,
                 int arg_request_id, int arg_error, int arg_error_index, std::vector<int> &arg_trapOID, 
				 std::vector<int> &arg_enterpriseOID, uint32_t arg_timestamp, std::vector<varbind*> &arg_varbindlist)
{
	version = arg_version;
	community = arg_community;
	pdu_type = arg_pdu_type;
	request_id =  arg_request_id;
	error = arg_error;
	error_index =  arg_error_index;
	varbindlist = arg_varbindlist;
	trapOID = arg_trapOID;
	enterpriseOID = arg_enterpriseOID;
	timestamp = arg_timestamp;
	
}
/*Thist constructor must be called only from make_pdu() function and only if constrution of valid PDU fails*/
SnmpMsg::SnmpMsg()
{
	/* version==-1 indicates, that it is not valid PDU */
	version = -1;
}

SnmpMsg::~SnmpMsg()
{
	/*Free memory that was dynamically allocated for varbindlist elements.*/
	for(std::vector<varbind*>::iterator it = varbindlist.begin(); it != varbindlist.end(); ++it){
		if(*it != NULL)
			delete *it;
	}
}

std::pair<int,SnmpMsg*> SnmpMsg::make_pdu(u_char *buf)

{
	uint curr_pos = 0;
	int pdu_length;
	int tmp_length = 0;
	int tmp_type;
	int version;
	std::string community;
	int pdu_type;
	int request_id = 0;
	int error = 0;
	int error_index = 0;
	uint32_t agent_addr,timestamp;
	int generic_trap,specific_trap;
	
	std::vector<int> tmp_oid, enterprise, trapOID;
	std::vector<varbind*> tmp_varbindlist_t;
	varbind *tmp_varbind;
	boost::variant<int, uint64_t, uint32_t, std::string, std::vector<int> > tmp_varbindval;
	
	
	/* Lets go.
	 * First byte of PDU have to be a SEQUENCE type with a Constructror flag (0x30) */
	if(buf[curr_pos++] != (ASN_SEQUENCE | ASN_CONSTRUCTOR) )
	{
#ifdef DEBUG
		std::cerr << "ASN_SEQUENCE type expected at first octet. Skip packet...\n";
#endif
		return std::make_pair(-1,new SnmpMsg());
	}
	
	
	/* The next field represents length of entire PDU(without first two bytes)*/
	pdu_length = decode_length_field(buf,&curr_pos) + 2;
	
	/* In next field we expect to see a version TLV*/
	/* Look for ASN_INTEGER type (0x02)*/
	if(buf[curr_pos++] != ASN_INTEGER)
	{
#ifdef DEBUG
		std::cerr << "ASN_INTEGER type expected for version number. Skip packet...\n";
#endif
		return std::make_pair(-1,new SnmpMsg());
	}
	
	/* Look for version length, we expect 0x01*/
	tmp_length = decode_length_field(buf,&curr_pos);
	if(tmp_length !=1 )
	{
#ifdef DEBUG
		std::cerr << "Too big length for version field. Skip packet...\n";
#endif
		return std::make_pair(-1,new SnmpMsg());
	}
	/*look for version value and move curr_pos forward*/
	decode_numeric_field<int>(&version, buf,&curr_pos,tmp_length,ASN_INTEGER);
	if(version != SNMP_VERSION_2c && version != SNMP_VERSION_1)
	{
#ifdef DEBUG
		std::cerr << "Unsupported version of SNMP: " << version << std::endl;
#endif
		return std::make_pair(-1,new SnmpMsg());
	}
	
	/* Next look for community string*/
	/* Now we expect a type ASN_OCTET_STR (0x04)*/
	if(buf[curr_pos++] !=  ASN_OCTET_STR)
	{
#ifdef DEBUG
		std::cerr << "ASN_OCTET_STR type expected for community string. Skip packet...\n";
#endif
		return std::make_pair(-1,new SnmpMsg());
	}
	
	/*Look for community length */
	tmp_length = decode_length_field(buf,&curr_pos);
	
	/* Now get community string */
	char * bbbuf = new char[tmp_length];
	memcpy(bbbuf, buf+curr_pos, tmp_length);
	community = std::string(bbbuf,tmp_length);
	delete bbbuf;
	curr_pos+=tmp_length;
	
	
	/* now we took closer to type-specific PDU sequence*/
	/* Let get message type */
	pdu_type = buf[curr_pos++];
	/* get type-specific sequence length */
	tmp_length = decode_length_field(buf,&curr_pos);
		
	switch(pdu_type)
	{
	case SNMP_MSG_TRAP2:
		
		/* get request_id */
		if(buf[curr_pos++] != (ASN_INTEGER))
		{
#ifdef DEBUG
			std::cerr << "ASN_INTEGER type expected for request id. Skip packet...\n";
#endif
			return std::make_pair(-1,new SnmpMsg());			
		}
		/* Look for request_id length*/
		tmp_length = decode_length_field(buf,&curr_pos);
		if(tmp_length <1)
		{
#ifdef DEBUG
			std::cerr << "Bad length for request id. Skip packet...\n";
#endif
			return std::make_pair(-1,new SnmpMsg());					}
		else
		{
			/*while (tmp_length--)
			{
				request_id = (request_id << 8) + buf[curr_pos++];
			}*/
			decode_numeric_field<int>(&request_id, buf,&curr_pos,tmp_length,ASN_INTEGER);
		}
		
		/* get error field */
		if(buf[curr_pos++] != ASN_INTEGER)
		{
#ifdef DEBUG
			std::cerr << "ASN_INTEGER type expected for error. Skip packet...\n";
#endif
			return std::make_pair(-1,new SnmpMsg());
		}
		/* Look for error length, we expect 0x01*/
		tmp_length = decode_length_field(buf,&curr_pos);
		if(tmp_length !=1 )
		{
			std::cout << "error\n";
		}
		else
		{
			//error = buf[curr_pos++];
			decode_numeric_field<int>(&error, buf,&curr_pos,tmp_length,ASN_INTEGER);
		}
		
		/* get error index field */
		if(buf[curr_pos++] != ASN_INTEGER)
		{
#ifdef DEBUG
			std::cerr << "ASN_INTEGER type expected for error index. Skip packet...\n";
#endif
			return std::make_pair(-1,new SnmpMsg());			
		}
		/* Look for error index length, we expect 0x01*/
		tmp_length = decode_length_field(buf,&curr_pos);
		if(tmp_length !=1 )
		{
			std::cerr << "error\n";
		}
		else
		{
			decode_numeric_field<int>(&error_index, buf,&curr_pos,tmp_length,ASN_INTEGER);
		}
		
		break;
	case SNMP_MSG_TRAP:
		/*get enterprice OID*/
		if(buf[curr_pos++] != (ASN_OBJECT_ID))
		{
#ifdef DEBUG
			std::cerr << "ASN_OBJECT_ID type expected for enterprice OID. Skip packet...\n";
#endif
			return std::make_pair(-1,new SnmpMsg());			
		}
		tmp_length = decode_length_field(buf,&curr_pos);
		enterprise = decode_oid_field(buf, &curr_pos, tmp_length);
		
		/* get agent address */
		if(buf[curr_pos++] != (ASN_IPADDRESS))
		{
#ifdef DEBUG
			std::cerr << "ASN_IPADDRESS type expected for agent address. Skip packet...\n";
#endif
			return std::make_pair(-1,new SnmpMsg());			
		}
		tmp_length = decode_length_field(buf,&curr_pos);
		if(tmp_length!=4){
			std::cerr << "Incorrect agent address length...\n";
			return std::make_pair(-1,new SnmpMsg());
		}
		decode_numeric_field<uint32_t>(&agent_addr, buf,&curr_pos,tmp_length,ASN_IPADDRESS);
		
		/* get generic trap*/
		if(buf[curr_pos++] != (ASN_INTEGER))
		{
#ifdef DEBUG
			std::cerr << "ASN_INTEGER type expected for generic trap. Skip packet...\n";
#endif
			return std::make_pair(-1,new SnmpMsg());			
		}
		tmp_length = decode_length_field(buf,&curr_pos);
		decode_numeric_field<int>(&generic_trap, buf,&curr_pos,tmp_length,ASN_INTEGER);
		
		/* get specific trap*/
		if(buf[curr_pos++] != (ASN_INTEGER))
		{
#ifdef DEBUG
			std::cerr << "ASN_INTEGER type expected for specific trap. Skip packet...\n";
#endif
			return std::make_pair(-1,new SnmpMsg());
		}
		tmp_length = decode_length_field(buf,&curr_pos);
		decode_numeric_field<int>(&specific_trap, buf,&curr_pos,tmp_length,ASN_INTEGER);
		
		/* now constructing trapOID. 
		 * if trap is enterprice-specific, trapOID = enterprise + 0 + specific_trap */
		if(generic_trap == SNMP_TRAP_ENTERPRISESPECIFIC){
			trapOID = enterprise;
			trapOID.push_back(0);
			trapOID.push_back(specific_trap);
		}
		/* if trap is not enterprise specific, trapOID = 1.3.6.1.6.3.1.1.5.(generic_trap+1) */
		else {
			trapOID = std::vector<int>(snmpGenericTrapBase, snmpGenericTrapBase + sizeof(snmpGenericTrapBase) / sizeof(int));
			trapOID.push_back(generic_trap+1);
		}
		
		/* get timestamp */
		if(buf[curr_pos++] != (ASN_TIMETICKS))
		{
#ifdef DEBUG
			std::cerr << "ASN_TIMETICKS type expected for timestamp. Skip packet...\n";
#endif
			return std::make_pair(-1,new SnmpMsg());

		}
		tmp_length = decode_length_field(buf,&curr_pos);
		decode_numeric_field<uint32_t>(reinterpret_cast<uint32_t*>(&timestamp), buf,&curr_pos,tmp_length,ASN_TIMETICKS);
		break;
		
	default:
		break;
#ifdef DEBUG
		std::cerr << "Unsupported PDU type. Skip packet...\n";
#endif
		return std::make_pair(-1,new SnmpMsg());
	}
	
	/* next @varbindlist follows. we expect a sequence type (0x30). */
	if(buf[curr_pos++] != (ASN_SEQUENCE | ASN_CONSTRUCTOR))
	{
#ifdef DEBUG
		std::cerr << "ASN_SEQUENCE type expected for Varbind list. Skip packet...\n";
#endif
		return std::make_pair(-1,new SnmpMsg());
	}
	/*get @varbindlist sequence length*/
	tmp_length = decode_length_field(buf,&curr_pos);
	
	if(pdu_length - curr_pos > tmp_length)
	{
#ifdef DEBUG
		std::cerr << "Varbindlist length is out of band of rest packet length. Skip packet...\n";
#endif
		return std::make_pair(-1,new SnmpMsg());
	}
	
	/*get varbinds*/
	int varbind_number=0;
	while (curr_pos < pdu_length)
	{
		/* get type of varbind sequence itself. 0x30 expected*/
		if(buf[curr_pos++] != (ASN_SEQUENCE | ASN_CONSTRUCTOR))
		{
#ifdef DEBUG
			std::cout << "Error: ASN_SEQUENCE expected fot varbind sequence\n";
#endif
			return std::make_pair(-1,new SnmpMsg());
		}
		/*get varbind sequence length*/
		tmp_length = decode_length_field(buf,&curr_pos);
		
		/* get type of first member, OID (0x06) expected*/
		if(buf[curr_pos++] != ASN_OBJECT_ID)
		{
#ifdef DEBUG
			std::cout << "Error: ASN_OBJECT_ID expected for first member of varbind\n";
#endif
			return std::make_pair(-1,new SnmpMsg());
		}
		
		/* get OID length*/
		tmp_length = decode_length_field(buf,&curr_pos);
		/* get OID */
		tmp_oid = decode_oid_field(buf, &curr_pos, tmp_length);
		
		/* get type of value*/
		tmp_type = buf[curr_pos++];
		/* get length of value */
		tmp_length = decode_length_field(buf,&curr_pos);
		/* according to type, get value*/
		int res = decode_varbind_val(&tmp_varbindval,buf,&curr_pos,tmp_type,tmp_length,tmp_oid);
		
		tmp_varbind = new varbind;
		tmp_varbind->oid = tmp_oid;
		tmp_varbind->type = tmp_type;
		tmp_varbind->value = tmp_varbindval;
		
		
		
		if(pdu_type == SNMP_MSG_TRAP2){
			if(tmp_oid == std::vector<int>(snmpTrapOid, snmpTrapOid + sizeof(snmpTrapOid) / sizeof(int)) && varbind_number == 1){
				try {
					trapOID = boost::get<std::vector<int> >(tmp_varbind->value);
				}
				catch (std::exception &e) {
					std::cerr << "Try to get trap OID exception: " << e.what() << std::endl;
				}
				varbind_number ++;
				delete tmp_varbind;
				continue;
			}
			if(tmp_oid == std::vector<int>(snmpSysUpTime, snmpSysUpTime + sizeof(snmpSysUpTime) / sizeof(int)) && varbind_number == 0){
				try{
				timestamp = boost::get<uint32_t>(tmp_varbind->value);
				
				}
				catch (std::exception &e) {
					std::cerr << "Try to get SysUptime exception: " << e.what() << std::endl;
				} 
				varbind_number ++;
				delete tmp_varbind;
				continue;
			}
			if(tmp_oid == std::vector<int>(snmpTrapEnterpriseOID, snmpTrapEnterpriseOID + sizeof(snmpTrapEnterpriseOID) / sizeof(int))){
				try {
				enterprise = boost::get<std::vector<int> >(tmp_varbind->value);
				}
				catch (std::exception &e) {
					std::cout << "Try to get enterprice OID exception: " << e.what() << std::endl;
				}
				varbind_number ++;
				delete tmp_varbind;
				continue;
			}
			
		}
		
		tmp_varbindlist_t.push_back(tmp_varbind);
		varbind_number ++;
	}


/* Some debugging output.*/
#ifdef DEBUG
	std::cout << "Received SNMP PDU:\nPDU length: " << pdu_length
	          << "\n SNMP version: " << version
	          << "\n Community: " << community
	          << "\n Message type: " << pdu_type
			  << "\n Trap OID: " << oid2string(trapOID);
	if(pdu_type == SNMP_MSG_TRAP2){		  
		std::cout << "\n Request ID: " << request_id
	          << "\n Error: " << error
	          << "\n Error index: " << error_index;
	} else{
		
		std::cout << "\n Enterprise: " <<  oid2string(enterprise)
					<< "\n Agent address: " << (agent_addr >>24) << "." << ((agent_addr>>16)&255) << "." 
											<< ((agent_addr>>8)&255) << "." << (agent_addr&255) 
					<< "\n Generic trap: " << generic_trap
					<< "\n Specific trap: " << specific_trap
					<< "\n SysUptime: " << timestamp;
	}
	std::cout << "\n Varbinds: \n" ;
	
	for(std::vector<varbind*>::iterator it = tmp_varbindlist_t.begin(); it < tmp_varbindlist_t.end(); it++)
	{
		std::cout << "  OID: " << oid2string((*it)->oid);
		std::cout << "\n  Type: " << (*it)->type ;
		std::cout << "\n  Value: " << varbind_value2string((*it)->value,(*it)->type) << std::endl;
	}
#endif
	
	return std::make_pair(0, new SnmpMsg(version, community, pdu_type, 
					request_id, error, error_index, trapOID, enterprise, timestamp,	tmp_varbindlist_t ));
	
}

int SnmpMsg::decode_length_field(u_char* buf, uint* curr_pos)
{
	int length=0;
	
	/*first of all we have to check if the ASN_LONG_LEN flag exists in first byte*/
	int tmp_int = buf[(*curr_pos)++];
	
	/* if we have length encoded in >1 bytes*/
	if(tmp_int & ASN_LONG_LEN )
	{
		/*lets find the number of bytes we need*/
		tmp_int = tmp_int & ~ASN_LONG_LEN;
		/* we do not support indefinite length PDUs*/
		if(tmp_int == 0)
			return -1;
		/*also we don't accept length outside the bounds of type size*/
		if (tmp_int > sizeof(int))
		{
			return -1;
		}
		
		/* then read that bytes and construct entire length*/
		for(int i = 0; i<tmp_int; i++)
		{
			length = (length << 8) + buf[(*curr_pos)++];
		}
	}
	else
	{
		/*if there is no ASN_LONG_LEN flag, then length is encoded in 1 byte  */
		length = tmp_int;
	}
	return length;
	
}

/* this function decodes ugly BER-encoded OID into human-friendly vector<int> format */
std::vector<int> SnmpMsg::decode_oid_field(u_char* buf, uint* curr_pos, int oid_length)
{
	std::vector<int> oid;
	
	int tmp_int = buf[(*curr_pos)++];
	/* The first 2 digits of OID encoded in single byte calculated by formula: byte=first*40+second.
	 * First digit value can be 0, 1 or 2
	 * In more then 99.9999% cases the first 2 digits are "1.3" that looks like "0x2b" */
	if(tmp_int == 0x2b)
	{
		oid.push_back(1);
		oid.push_back(3);
	}
	else
	{
		int restdiv40 = tmp_int % 40;
		if (tmp_int-restdiv40 < 81)
		{
			oid.push_back((tmp_int-restdiv40)/40);
			oid.push_back(restdiv40);
		}
		else
		{
			oid.push_back(2);
			oid.push_back(tmp_int - 80);
		}
	}
	
	/* decrement oid length*/
	oid_length--;
	
	/* Next digits encoded in one or several bytes.
	 * 0-127 values encoded in single byte.
	 * >127 values encoded in multiple bytes.
	 * The highest bit (mask ASN_BIT8 0x80) indentificates that current byte is not last byte for current digit */
	while(oid_length)
	{
		tmp_int = 0;
		do
		{
			tmp_int = (tmp_int << 7) + ( buf[(*curr_pos)] & ~ASN_BIT8 );
			oid_length--;
		}
		while (buf[(*curr_pos)++] & ASN_BIT8);
		if(tmp_int > static_cast<unsigned long>(MAX_SUBID))
		{
		
		}
		oid.push_back(tmp_int);
	}
	
	/* Check for resulted oid size*/
	if(oid.size() > MAX_OID_LEN || oid.size() < MIN_OID_LEN)
	{
#ifdef DEBUG
		std::cerr << "OID size is out of band - " << oid.size() << std::endl;
#endif
		return std::vector<int>();
	}
	else
	{
#ifdef DEBUG
		//std::cout << oid2string(oid) << std::endl;
#endif
		return oid;
		
	}
	
}
int  SnmpMsg::decode_varbind_val(boost::variant<int, uint64_t, uint32_t, std::string, std::vector<int> > *varbind_val,
                                 u_char* buf, uint* curr_pos, int vartype, int length, std::vector<int> &oid)
{
	if(vartype == ASN_INTEGER)
	{
		if(length > sizeof(int))
		{
			return -1;
		}
		/* integer is negative*/
		if(buf[*curr_pos] & 0x80)
			*varbind_val = int(-1);
		else
			*varbind_val = int(0);
		while (length--)
		{
			try{
				*varbind_val = (boost::get<int>(*varbind_val) << 8) +static_cast<u_char> (buf[(*curr_pos)++]);
			}
			catch (std::exception &e) {
#ifdef DEBUG
				std::cerr << "Try to get (int) exception: " << e.what() << std::endl;
#endif
				}
		}
	}
	else if (vartype == ASN_COUNTER || vartype == ASN_GAUGE || vartype == ASN_TIMETICKS || vartype == ASN_UINTEGER)
	{
		if(length > sizeof(uint32_t))
			return -1;
			
		*varbind_val = uint32_t(0);
		
		/* integer is negative*/
		if(buf[*curr_pos] & 0x80){
			try{
				*varbind_val = ~(boost::get<uint32_t>(*varbind_val));
			}
			catch (std::exception &e) {
#ifdef DEBUG
					std::cerr << "Try to get (uint32_t) exception: " << e.what() << std::endl;
#endif
				}
		}	
			
		while (length--)
		{
			try{
			*varbind_val = (boost::get<uint32_t>(*varbind_val) << 8) +static_cast<u_char> (buf[(*curr_pos)++]);
			
			}
			catch (std::exception &e) {
#ifdef DEBUG
					std::cerr << "Try to get (uint32_t) exception: " << e.what() << std::endl;
#endif
				}
		}
		
	}
	else if(vartype == ASN_OCTET_STR || vartype == ASN_BIT_STR)
	{
		char dst[length+1];
		strncpy(dst,reinterpret_cast<char*>(buf+*curr_pos),length);
		dst[length]=0;
		*varbind_val = std::string(dst);
		*curr_pos += length;
	}
	else if(vartype == ASN_OBJECT_ID)
	{
		*varbind_val = decode_oid_field(buf, curr_pos, length);
	}
	else if (vartype == ASN_NULL)
	{
		*varbind_val = int(0);
		return 0;
	}
	else if (vartype == ASN_COUNTER64)
	{
		if(length > 8)
		{
			return -1;
		}
		while (length--)
		{
			try{
			*varbind_val = (boost::get<uint64_t>(*varbind_val) << 8) + static_cast<u_char> (buf[(*curr_pos)++]);
			
			}
			catch (std::exception &e) {
#ifdef DEBUG
					std::cerr << "Try to get (uint64_t) exception: " << e.what() << std::endl;
#endif
				}
		}
	}
	else
	{
#ifdef DEBUG
		std::cout << "Unknown type - " << vartype << "\n";
#endif
		return -1;
	}
	
	return 0;
	
}
std::string SnmpMsg::oid2string(std::vector<int> &oid)
{
	std::string result="";
	std::stringstream ss;
	for(std::vector<int>::iterator i = oid.begin(); i < oid.end(); i++)
		ss << "." << *i ;
	result = ss.str();
	//result.resize(result.size() -1 );
	return result;
}
std::string SnmpMsg::varbind_value2string(boost::variant<int, uint64_t, uint32_t, std::string, std::vector<int> > &val, int vartype)
{
	std::string result="";
	std::stringstream ss;
	
	if(vartype == ASN_OCTET_STR || vartype == ASN_BIT_STR)
	{
		try{
		result = boost::get<std::string>(val);
		
		}
		catch (std::exception &e) {
#ifdef DEBUG
					std::cerr << "Try to get (string) exception " << e.what() << std::endl;
#endif
		}
	}
	else if (vartype == ASN_COUNTER || vartype == ASN_GAUGE || vartype == ASN_TIMETICKS || vartype == ASN_UINTEGER )
	{
		try{
		ss <<  boost::get<uint32_t>(val);
		}
		catch (std::exception &e) {
#ifdef DEBUG
					std::cerr << "Try to get (uint32_t) exception: " << e.what() << std::endl;
#endif
		}
		result = ss.str();
	}
	else if (vartype == ASN_COUNTER64)
	{
		try{
		ss <<  boost::get<uint64_t>(val);
		}
		catch (std::exception &e) {
#ifdef DEBUG
					std::cerr << "Try to get (uint64_t) exception: " << e.what() << std::endl;
#endif
		}
		result = ss.str();
	}
	else if (vartype == ASN_INTEGER )
	{
		try{
		ss <<  boost::get<int>(val);
		}
		catch (std::exception &e) {
#ifdef DEBUG
					std::cerr << "Try to get (int) exception: " << e.what() << std::endl;
#endif
		}
		result = ss.str();
	}
	else if (vartype == ASN_NULL )
	{
		result = "";
	}
	else if (vartype == ASN_OBJECT_ID )
	{
		try {
			result = oid2string(boost::get<std::vector<int> >(val));
		
		}
		catch (std::exception &e) {
#ifdef DEBUG
					std::cout << "Try to get (string) exception: " << e.what() << std::endl;
#endif
		}
	}
	else
	{
		result = "";
	}
	return result;
}

template<class T>
int SnmpMsg::decode_numeric_field(T *result , u_char* buf, uint* curr_pos, int length, int type)
{
	/*some ugly implementations of agents encodes 4-byte type variables in more than 4-byte characters
	if there all leading zeroes we just skip them, otherwise return with error*/
	if(length > sizeof(*result))
	{
		while(length != sizeof(*result)){
			if(buf[(*curr_pos)++] != 0){
				return -1;
			}
			length--;
		}
	}
	
	*result=0;
	
	/* integer is negative*/
	if(buf[*curr_pos] & 0x80)
	{
		if(type == ASN_INTEGER)
			*result = int(-1);
		else
			*result = ~(*result);
	}
	
	while (length--)
	{
		*result = ((*result) << 8) + (buf[(*curr_pos)++]);
	}
	return 0;
	 
}
std::vector<int> SnmpMsg::string2oid(std::string &oidstring)
{
	if(oidstring.at(0) != '.')
		return std::vector<int>();
	else
		oidstring = oidstring.substr(1,oidstring.length()-1);
	
	std::vector<int> result;
	std::stringstream ss(oidstring);
    std::string oid_elem;
    while(std::getline(ss, oid_elem, '.')) {
    	
		int ii = strtol (oid_elem.c_str(),NULL,10);
		if(ii == 0 && *(oid_elem.c_str()) !='0')
			{
#ifdef DEBUG
				std::cerr << "Error converting from string to int " << oid_elem  << ".\n";
#endif
				return std::vector<int>();
			}
		result.push_back(ii);
	}
	return result;
}
std::vector<int> SnmpMsg::get_trap_oid()
{
	return trapOID;
}
std::vector<int> SnmpMsg::get_enterprise_oid()
{
	return enterpriseOID;
}
std::vector<varbind*> SnmpMsg::get_var_bind_list()
{
	return varbindlist;
}
uint32_t SnmpMsg::get_sys_uptime()
{
	return timestamp;
}

