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

#include "sockethandler.h"

SocketHandler::SocketHandler()
{
}

SocketHandler::SocketHandler(int port,DataHandler *dhl)
{
	if(init_socket(port)<0){
		listen_fd=-1;
	}
	data_handler = dhl;
} 

SocketHandler::~SocketHandler()
{
}

int SocketHandler::init_socket(int port)
{
	if( (listen_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
#ifdef DEBUG
		std::cerr << "socket() error: " << strerror(errno) << std::endl;
#endif
		return -1;
	}
	
	sockaddr_in sock_addr;
	memset( &sock_addr, 0, sizeof(sockaddr_in) );
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_addr.s_addr = INADDR_ANY;
	sock_addr.sin_port = htons(port);
	
	if( bind(listen_fd, reinterpret_cast<struct sockaddr *> (&sock_addr), sizeof(sock_addr)) < 0)
	{
#ifdef DEBUG
		std::cerr << "bind() error: " << strerror(errno) << std::endl;
#endif
		listen_fd = -1;
		return -1;
	}
	else
	{
#ifdef DEBUG
		std::cout << "Started listen server on port " << port << std::endl;
#endif
	}
	return 0;
}
int SocketHandler::start_queue(int workers_num)
{
	pthread_attr_t attr;
	/* For portability, explicitly create threads in a joinable state */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	for(int i=0; i<workers_num; i++) {
		thr *tt = new thr;
		tt->evh = this;
		tt->thr_id = i;
		workers.push_back(new pthread_t);
		pthread_create(workers[i], &attr, start_worker, (void *)tt);
	}
	
	for (std::vector<pthread_t *>::iterator it = workers.begin(); it != workers.end(); ++it) 
		pthread_join(*(*it),NULL);
}

void* SocketHandler::start_worker(void* arg) {
	thr *tt = static_cast<thr*> (arg);
	int res = tt->evh->worker(tt->thr_id);
	if(res<0) {
#ifdef DEBUG
		std::cerr << "Abnormal thread termination\n";
#endif
	}
}

int SocketHandler::worker(int thr_id)
{
	struct sockaddr_in si_client;
	int s, i;
	socklen_t slen=sizeof(si_client);
	u_char buf[BUFLEN];
	
	memset(static_cast<void*>(&si_client), 0, slen);
	
	while (1)
	{
		int recvb = recvfrom(listen_fd, buf, BUFLEN, 0, reinterpret_cast<struct sockaddr *>(&si_client), &slen);
		if (recvb==-1)
#ifdef DEBUG
			std::cerr << "recvfrom() error\n";
#endif

#ifdef DEBUG
		printf("Thread %d. Received packet from %s:%d\n\n", thr_id,
		       inet_ntoa(si_client.sin_addr), ntohs(si_client.sin_port));
#endif
		data_handler->handle_message(buf,si_client.sin_addr);
	}
	
}
