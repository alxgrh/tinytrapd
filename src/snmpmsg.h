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

#ifndef SNMPMSG_H
#define SNMPMSG_H
#include <iostream>
#include <string>
#include <string.h>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <stdlib.h>
#include <boost/variant/variant.hpp>
#include <boost/variant/get.hpp>
#include <stdint.h>
#include <typeinfo>


/* ********************************************
 * ASN and SNMP related constants definitions 
 * ********************************************/
#define MIN_OID_LEN	    2
#define MAX_OID_LEN	    128 
#define MAX_SUBID   0xFFFFFFFF

/*ASN types*/
#define ASN_BOOLEAN	    ((u_char)0x01)
#define ASN_INTEGER	    ((u_char)0x02)
#define ASN_BIT_STR	    ((u_char)0x03)
#define ASN_OCTET_STR	    ((u_char)0x04)
#define ASN_NULL	    ((u_char)0x05)
#define ASN_OBJECT_ID	    ((u_char)0x06)
#define ASN_SEQUENCE	    ((u_char)0x10)
#define ASN_SET		    ((u_char)0x11)

#define ASN_UNIVERSAL	    ((u_char)0x00)
#define ASN_APPLICATION     ((u_char)0x40)
#define ASN_CONTEXT	    ((u_char)0x80)
#define ASN_PRIVATE	    ((u_char)0xC0)

#define ASN_PRIMITIVE	    ((u_char)0x00)
#define ASN_CONSTRUCTOR	    ((u_char)0x20)
#define ASN_UNI_PRIV (ASN_UNIVERSAL | ASN_PRIMITIVE)
#define ASN_SEQ_CON (ASN_SEQUENCE | ASN_CONSTRUCTOR)

#define ASN_LONG_LEN	    (0x80)
#define ASN_EXTENSION_ID    (0x1F)
#define ASN_BIT8	    (0x80)

#define ASN_IPADDRESS   (ASN_APPLICATION | 0)
#define ASN_COUNTER     (ASN_APPLICATION | 1)
#define ASN_GAUGE       (ASN_APPLICATION | 2)
#define ASN_UNSIGNED    (ASN_APPLICATION | 2)   
#define ASN_TIMETICKS   (ASN_APPLICATION | 3)
#define ASN_OPAQUE      (ASN_APPLICATION | 4)   
#define ASN_NSAP        (ASN_APPLICATION | 5)
#define ASN_COUNTER64   (ASN_APPLICATION | 6)
#define ASN_UINTEGER    (ASN_APPLICATION | 7)

/*SNMP versions */
#define SNMP_VERSION_1	   0
#define SNMP_VERSION_2c    1
#define SNMP_VERSION_3     3 // not supported yet

/*SNMP PDU types*/
#define SNMP_MSG_TRAP       (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x4)
#define SNMP_MSG_INFORM     (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x6) 
#define SNMP_MSG_TRAP2      (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x7) 

/* Generic traps */
#define SNMP_TRAP_COLDSTART		0
#define SNMP_TRAP_WARMSTART		1
#define SNMP_TRAP_LINKDOWN		2
#define SNMP_TRAP_LINKUP		3
#define SNMP_TRAP_AUTHFAIL		4
#define SNMP_TRAP_EGPNEIGHBORLOSS	5
#define SNMP_TRAP_ENTERPRISESPECIFIC	6


const int snmpTrapOid[] = {1,3,6,1,6,3,1,1,4,1,0};
const int snmpTrapEnterpriseOID[] = {1,3,6,1,6,3,1,1,4,3,0};
const int snmpSysUpTime[] = {1,3,6,1,2,1,1,3,0};
const int snmpGenericTrapBase[] = {1,3,6,1,6,3,1,1,5};

struct varbind {
	std::vector<int> oid;
	int type;
	boost::variant<int, uint64_t, uint32_t, std::string, std::vector<int> > value;
};

enum snmp_generic_trap {
	ColdStart = 0,
	WarmStart = 1,
	LinkDown = 2,
	LinkUp = 3,
	AuthFail = 4,
	EgpNeigbhorLoss = 5,
	EnterpriceSpecific = 6,
};

class SnmpMsg {

private:
	int version;
	std::string community;
	int pdu_type;
	int request_id;
	int error;
	int error_index;
	//std::vector< std::pair< std::vector<int>,std::string> > varbindlist;
	std::vector<int> trapOID;

	std::vector<varbind*> varbindlist;
	
	u_char *raw_data;
	
	/* v1 trap specific*/
	std::vector<int> enterpriseOID;
	uint32_t agent_addr;
	int generic_trap;
	int specific_trap;
	uint32_t timestamp;
	 
	
	SnmpMsg(int version, std::string &community, int pdu_type, 
			int request_id, int error, int error_index, std::vector<int> &trapOID, std::vector<int> &enterpriseOID, 
			uint32_t timestamp, std::vector<varbind*> &varbindlist );
	SnmpMsg();
	
public:
	
	/*make self generated PDU*/
	static SnmpMsg make_pdu(int version, std::string community, int pdu_type, 
			int request_id, int error, int error_index, std::vector<std::pair<std::vector<int>,std::string> > varbindlist );
	/* make PDU from message received on socket */
	static std::pair<int,SnmpMsg*> make_pdu (u_char * buf);
	
	static int decode_length_field(u_char* buf, uint *curr_pos);
	
	static std::vector<int> decode_oid_field(u_char* buf, uint *curr_pos, int oid_length);
	
	template<class T>
	static int decode_numeric_field (T *result, u_char *buf, uint *curr_pos, int length, int type);
	
	static int decode_varbind_val(boost::variant<int, uint64_t, uint32_t, std::string, std::vector<int> > *val, u_char* buf, uint* curr_pos, int vartype, int length, std::vector<int> &oid);
	static std::string oid2string (std::vector<int> &oid);
	static std::string varbind_value2string (boost::variant<int, uint64_t, uint32_t, std::string, std::vector<int> > &val, int vartype);
	static std::vector<int> string2oid (std::string &oidstring);
	
	std::vector<int> get_trap_oid();
	std::vector<varbind*> get_var_bind_list();
	uint32_t get_sys_uptime();
	std::vector<int> get_enterprise_oid();
	
	~SnmpMsg();

};

#endif // SNMPMSG_H
