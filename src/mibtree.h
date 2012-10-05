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

#ifndef MIBTREE_H
#define MIBTREE_H
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <fstream>
#include <sstream>


#include <boost/tokenizer.hpp>
 
struct MibNode {
	int subid;
	int type;
	std::string name;
	std::string mibname;
	std::set<std::string> mibnames;
	MibNode *parent;
	std::map<int,MibNode*> children;
};

class MibModule {
public:	
	std::string name;
	//MibModule *imports;
	std::map<std::string, std::string> imports;
	std::map<std::string, MibModule*> imported;
	std::vector<std::string> objects_order;
	std::map<std::string, std::pair<std::string,int> > objects;
	int load_from_file(std::string filename);
	MibModule();
	~MibModule();
};

class MibTree {
	MibNode root;
	std::map<std::string, MibModule*> loaded_mibs;
public:
	int add_node(int subid, int type, std::string name, MibNode *parent);
	MibNode* find_node(std::string &name, std::string &modulename, MibNode *from);
	std::string get_oid_description(std::vector<int> &oid);
	int load_mib(MibModule *mod);
	void print_tree(MibNode *from, int depth);
	MibTree();
	~MibTree();

};


#endif // MIBTREE_H
