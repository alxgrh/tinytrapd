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

#include "mibtree.h"

MibTree::MibTree()
{
	/*set root node*/
	root.subid = 0;
	root.name = "global_root";
	root.parent = NULL;
	
	/* Hardcode ccitt(0), iso(1) and joint-iso-ccitt(2) nodes*/
	MibNode *m_node = new MibNode;
	m_node->subid = 0;
	m_node->name = "ccitt";
	m_node->parent = &root;
	m_node->mibnames.insert("GLOBAL_ROOT");
	root.children[0] = m_node;
	
	m_node = new MibNode;
	m_node->subid = 1;
	m_node->name = "iso";
	m_node->parent = &root;
	m_node->mibnames.insert("GLOBAL_ROOT");
	root.children[1] = m_node;
	
	m_node = new MibNode;
	m_node->subid = 2;
	m_node->name = "joint-iso-ccitt";
	m_node->parent = &root;
	m_node->mibnames.insert("GLOBAL_ROOT");
	root.children[2] = m_node;
	
	/* Hardcoded SNMPv2-SMI MIB*/
	MibModule m_module;
	m_module.name = "SNMPv2-SMI";	
	m_module.objects["org"] = std::pair<std::string,int>("iso",3);
	m_module.objects["dod"] = std::pair<std::string,int>("org",6);
	m_module.objects["internet"] = std::pair<std::string,int>("dod",1);
	m_module.objects["directory"] = std::pair<std::string,int>("internet",1);
	m_module.objects["mgmt"] = std::pair<std::string,int>("internet",2);
	m_module.objects["mib-2"] = std::pair<std::string,int>("mgmt",1);
	m_module.objects["transmission"] = std::pair<std::string,int>("mib-2",10);
	m_module.objects["experimental"] = std::pair<std::string,int>("internet",3);
	m_module.objects["private"] = std::pair<std::string,int>("internet",4);
	m_module.objects["enterprises"] = std::pair<std::string,int>("private",1);
	m_module.objects["security"] = std::pair<std::string,int>("internet",5);
	m_module.objects["snmpV2"] = std::pair<std::string,int>("internet",6);
	m_module.objects["snmpDomains"] = std::pair<std::string,int>("snmpV2",1);
	m_module.objects["snmpProxys"] = std::pair<std::string,int>("snmpV2",2);
	m_module.objects["snmpModules"] = std::pair<std::string,int>("snmpV2",3);
	
	m_module.objects_order.push_back("org");
	m_module.objects_order.push_back("dod");
	m_module.objects_order.push_back("internet");
	m_module.objects_order.push_back("directory");
	m_module.objects_order.push_back("mgmt");
	m_module.objects_order.push_back("mib-2");
	m_module.objects_order.push_back("transmission");
	m_module.objects_order.push_back("experimental");
	m_module.objects_order.push_back("private");
	m_module.objects_order.push_back("enterprises");
	m_module.objects_order.push_back("security");
	m_module.objects_order.push_back("snmpV2");
	m_module.objects_order.push_back("snmpDomains");
	m_module.objects_order.push_back("snmpProxys");
	m_module.objects_order.push_back("snmpModules");
	
	
	load_mib(&m_module);
	
}

MibTree::~MibTree()
{
}
MibModule::MibModule()
{
}
MibModule::~MibModule()
{
}
int MibTree::load_mib(MibModule* mod)
{
	//std::map<std::string, std::pair<std::string,int> >::iterator it = mod->objects.begin();
	std::vector<std::string>::iterator it = mod->objects_order.begin();
	std::string obj;
	std::string parent_obj;
	
	std::map<std::string,MibNode*> already_loaded;
	
	MibNode *m_node, *parent_node;
	int id;
	while(it!= mod->objects_order.end())
	{
		obj = (*it);
		parent_obj = mod->objects[obj].first;
		id = mod->objects[obj].second;
		it++;
		
		m_node = new MibNode;
		m_node->name = obj;
		m_node->subid = id;
		m_node->mibname = mod->name;
		m_node->mibnames.insert(mod->name);
		m_node->parent = NULL;
		
		if(mod->name == "RFC1155-SMI" && obj == "private"){
			std::cout << "";
		}
		/*Is it top of tree?*/
		if(parent_obj == "iso" || parent_obj == "ccitt" || parent_obj == "joint-iso-ccitt")
		{
			std::string gr = "GLOBAL_ROOT";
			parent_node = find_node(parent_obj,gr,&root);
			
			//parent_node = root.children[1];
			/*Check if Node already exist in tree*/
			//if(root.children[1]->children.count(id) > 0 ){
			if(parent_node->children.count(id) > 0 )
			{
				delete m_node;
				already_loaded[obj] = parent_node->children[id];
				/*Check if MIB loaded*/
				if(parent_node->children[id]->mibnames.count(mod->name) > 0)
				{
					/* Nothing to do, skip*/
					continue;
				}
				else
				{
					/* refresh mibnames set*/
					parent_node->children[id]->mibnames.insert(mod->name);
					continue;
				}
			}
			else
			{
				m_node->parent = parent_node;
				parent_node->children[id] = m_node;
				already_loaded[obj] = m_node;
				m_node->mibnames.insert(mod->name);
				continue;
			}
		}
		/*if(parent_obj == "ccitt" )
		{
			parent_node = root.children[0];
			m_node->parent = root.children[0];
			continue;
		}
		if(parent_obj == "joint-iso-ccitt")
		{
			parent_node = root.children[2];
			m_node->parent = root.children[2];
			continue;
		}*/
		
		/* if parent object is located in imported mib*/
		if(mod->imports.count(parent_obj) > 0)
		{
			/*search it in loaded tree in referenced module*/
			m_node->parent = find_node(parent_obj,mod->imports[parent_obj],&root);
			if (!m_node->parent)
			{
				std::cerr << "Parent object " << parent_obj << ", module name "
				          << mod->imports[parent_obj] << " not found in already loaded tree." << std::endl;
				delete m_node;
			}
			else
			{
				if(m_node->parent->children.count(id) > 0)
				{
					m_node->parent->children[id]->mibnames.insert(mod->name);
					already_loaded[obj] = m_node->parent->children[id];
					delete m_node;
				}
				else
				{
					m_node->parent->children[id] = m_node;
					already_loaded[obj] = m_node;
					m_node->mibnames.insert(mod->name);
				}
			}
			continue;
		}
		
		/* if parent object in this mib*/
		if(already_loaded.count(parent_obj) > 0)
		{
			m_node->parent = already_loaded[parent_obj];
			if(m_node->parent->children.count(id) > 0)
			{
				m_node->parent->children[id]->mibnames.insert(mod->name);
				already_loaded[obj] = m_node->parent->children[id];
				delete m_node;
			}
			else
			{
				m_node->parent->children[id] = m_node;
				already_loaded[obj] = m_node;
				m_node->mibnames.insert(mod->name);
			}
		}
		else
		{
			std::cerr << "Parent object " << parent_obj << " not found in already loaded mib." << std::endl;
			delete m_node;
		}
		
	}
}
MibNode* MibTree::find_node(std::string &name, std::string &modulename, MibNode* from)
{
	MibNode* looked_for = NULL;
	for(std::map<int,MibNode*>::iterator it = from->children.begin(); it != from->children.end(); ++it)
	{
		/* Found node with given name and MIB name*/
		if(it->second->mibnames.count(modulename) > 0 && it->second->name == name )
		{
			return it->second;
		}
		else
		{
			looked_for = find_node(name, modulename, it->second);
			if(looked_for != NULL)
				return looked_for;
		}
	}
	/*Node not found*/
	return NULL;
}
void MibTree::print_tree(MibNode* from, int depth)
{
	if(from == NULL){
		from = &root;
	}
	std::cout << from->name << "(" << from->subid << ")" << std::endl;
	for(std::map<int,MibNode*>::iterator it = from->children.begin(); it != from->children.end(); ++it)
	{
		for (int i = 0; i<depth; i++ )
		{
			std::cout << "\t" ;
		}
		std::cout << "|-";
		print_tree(it->second,depth+1);
	}
} 
int MibModule::load_from_file(std::string filename)
{
	objects_order = std::vector<std::string>();
	objects = std::map<std::string, std::pair<std::string,int> >();
	
	std::ifstream mibfile(filename.c_str());
	if (!mibfile.is_open())
	{
		std::cerr << "Unable to open file " << filename << std::endl;
		return -1;
	}
	
	typedef boost::tokenizer< boost::char_separator<char> > Tokenizer;
	boost::char_separator<char> sep(" ");
	
	std::string line;
	Tokenizer::iterator it;
	
	/*get first line
	 * it must contain  MIB-BEGIN MIB-NAME */
	getline(mibfile,line);
	Tokenizer tok = Tokenizer(line,sep);
	it= tok.begin();
	if((*it) != "MIB-BEGIN" )
	{
		std::cerr << "Invalid mib file format.\n";
		mibfile.close();
		return -1;
	}
	name= *(++it);
	
	std::vector<std::string> tuples;
	
	while (getline(mibfile,line))
	{
	
		Tokenizer  tok(line,sep);
		tuples.assign(tok.begin(),tok.end());
		if(tuples.size() != 3)
		{
			std::cerr << "Invalid line. Skip.\n";
			continue;
		}
		
		if(tuples.at(0) == "IMPORTS")
		{
			imports[tuples.at(1)] = tuples.at(2);
		}
		else
		{
			int ii = strtol (tuples.at(2).c_str(),NULL,10);
			if(ii == 0 && *(tuples.at(2).c_str()) !='0')
			{
				std::cerr << "Error converting from string to int " << tuples.at(1) << tuples.at(2) << ".\n";
				continue;
			}
			else
			{
				objects[tuples.at(0)] = std::pair<std::string,int>(tuples.at(1),ii);
				objects_order.push_back(tuples.at(0));
				//std::cout << tuples.at(0) << " " << tuples.at(1) << " " << ii << std::endl;
			}
		}
		
	}
	/*
	for(std::vector<std::string>::iterator it = objects_order.begin(); it!=objects_order.end(); ++it)
	{
		std::cout << *it << " " << objects[*it].first << " " << objects[*it].second << std::endl;
	}
	for(std::map<std::string, std::string>::iterator it = imports.begin(); it!=imports.end(); ++it)
	{
		std::cout << (*it).first << " " << (*it).second << std::endl;
	}
	 */
	mibfile.close();
	return 0;
	
}
std::string MibTree::get_oid_description(std::vector<int>& oid)
{
	MibNode *curr_node = &root;
	std::string oid_description; 
	std::stringstream ss; 
	
	for(std::vector<int>::iterator it = oid.begin(); it != oid.end(); ++it){
		if(curr_node->children.count(*it) > 0){
			curr_node = curr_node->children[*it];
			oid_description = curr_node->mibname + "::" + curr_node->name;
		}
		else {
			ss << "." << *it ;
		}
	}
	oid_description = curr_node->mibname + "::" + curr_node->name;
	return oid_description + ss.str();
}
