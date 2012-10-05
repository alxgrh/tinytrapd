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

#ifndef SOCKETHANDLER_H
#define SOCKETHANDLER_H

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h> 
#include <arpa/inet.h>
#include <iostream>
#include <errno.h>
#include <pthread.h>
#include <string.h>

#include "mibtree.h"
#include "datahandler.h"
#define WORKERS_NUM 4
#define BUFLEN 1500

class SocketHandler {

	int listen_fd;
	std::vector<pthread_t *> workers;
	DataHandler *data_handler;
	
public:
	SocketHandler();
	SocketHandler(int port, DataHandler *dhl);
	
	~SocketHandler();
	
	/* Static function that starts worker process. 
	 * Used due to impossibility of calling pthread_create() with non-static member function in arguments */
	static void* start_worker(void *arg);
	int worker(int thr_id);
	int init_socket(int port);	
	int get_listen_fd() const {return listen_fd;};
	int start_queue(int workers_num);
};

struct thr {
	SocketHandler * evh;
	int thr_id;
};

#endif // SOCKETHANDLER_H
