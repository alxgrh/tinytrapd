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

#ifndef LOGGER_H
#define LOGGER_H

#include <pthread.h>
#include <syslog.h>
#include <queue>
#include <string>
#include <fstream>
#include <iostream>
#include <ctime>


#define LOGGER_NONE 0 
#define LOGGER_MAIN_FILE 1
#define LOGGER_SEPARATE_FILE 2
#define LOGGER_SYSLOG 3
#define LOGGER_ALL 5

#define LOGGER_DEFAULT_FILE "/var/log/tinytrapd.log"
#define LOGGER_QUEUE_SIZE 10

//struct for object that should be logged
struct logger_unit {
	int log_type;
	int syslog_facility;
	int syslog_priority;
	std::string message;
	std::string filename;
};

//struct for queue of objects that should be logged
struct logger_queue {
	std::queue<logger_unit*> objs;
	pthread_mutex_t lock;
	bool empty, full;
	pthread_cond_t not_empty, not_full;

};



class Logger {
	bool use_syslog;
	bool use_main_file;
	int facility;
	int priority;
	logger_queue message_queue;
	std::fstream *fds;
	
	

public:
	
	static std::string now(const char* format);
	static std::string now();
	
	int start_queue_thread();
	/*static function for callback from pthread_create(). it runs dispatch() function*/
	static void* start_dispatcher(void*);
	/* queue dispatcher thread*/
	int dispatch();
	
	/*send message to logger queue */
	int feed_queue(logger_unit *lu);
	
	/* log message to all global enbled channels */
	int log_message_global(std::string &message);
	/* try to log message to syslog channel */
	int log_message(int facility, int priority, std::string &message);
	/* try to log message to separate file */
	int log_message(std::string &message, std::string &filename);
	/* try to log to main log file*/
	int log_message(std::string &message);
	
	
	Logger();
	int init_logger(int s_facility, int s_priority);
	int init_logger(std::string filename);
	int init_logger(int s_facility, int s_priority, std::string filename);
	int init_logger();
	
	~Logger();

};

#endif // LOGGER_H
