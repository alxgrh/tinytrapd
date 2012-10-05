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

#include "logger.h"

Logger::Logger()
{
	use_syslog = false;
	use_main_file = false;
	facility = 0;
	priority = 0;
	
	pthread_mutex_init(&message_queue.lock, NULL);
	pthread_cond_init (&message_queue.not_full, NULL);
	pthread_cond_init (&message_queue.not_empty, NULL);
	message_queue.full = false;
	message_queue.empty = true;
	
}

Logger::~Logger()
{
}

int Logger::init_logger(int s_facility, int s_priority, std::string filename)
{
	use_syslog = true;
	facility = s_facility;
	priority = s_priority;
	openlog("tinytrapd", LOG_PID|LOG_CONS, facility);
	syslog(priority, "Start syslogging ... ");
	
	use_main_file = true;
	fds = new std::fstream(filename.c_str(),std::ios_base::out | std::ios_base::app);
	if(!(*fds))
	{
		return -1; 
	}
	else
	{
		*fds << now() << " Start logging to file..."  << std::endl;
		return 0;
	}
	
}
int Logger::init_logger(int s_facility, int s_priority)
{
	use_syslog = true;
	facility = s_facility;
	priority = s_priority;
	openlog("tinytrapd", LOG_PID|LOG_CONS, facility);
	syslog(priority, "Start syslogging ... ");
	return 0;
}
int Logger::init_logger(std::string filename)
{
	use_main_file = true;
	fds = new std::fstream(filename.c_str(),std::ios_base::out | std::ios_base::app);
	if(!(*fds))
	{
#ifdef DEBUG
		std::cerr << "Unable to open file " << filename << "\n";
#endif
		return -1;
	}
	else
	{
		*fds << now() << " Start logging to file..." << std::endl;
		return 0;
	}
	
}
int Logger::init_logger()
{
	return 0;
}


int Logger::dispatch()
{
	logger_unit *msg_obj;
	while (1)
	{
		pthread_mutex_lock (&message_queue.lock);
		while (message_queue.empty)
		{
#ifdef DEBUG
			std::cout << "dispatch : queue EMPTY. Wait condition.\n";
#endif
			pthread_cond_wait (&message_queue.not_empty, &message_queue.lock);
#ifdef DEBUG
			std::cout << "dispatch : condition not empty occured\n";
#endif
		}
		
#ifdef DEBUG
		std::cout << "dispatch : queue size - " << message_queue.objs.size() << std::endl;
#endif
		msg_obj = message_queue.objs.front();
		message_queue.objs.pop();
		message_queue.full = false;
		if(message_queue.objs.size() == 0)
		{
			message_queue.empty = true;
		}
		
		pthread_mutex_unlock (&message_queue.lock);
		pthread_cond_signal (&message_queue.not_full);
		
		/* add \n to end of message if it is not already there */
		if(msg_obj->message.at(msg_obj->message.size()-1) != '\n'){
			msg_obj->message+='\n';
		}
		if(msg_obj->log_type == LOGGER_SYSLOG)
		{
			/* when syslogging enabled globally we won't change log facility */
			if(use_syslog)
			{
				syslog(msg_obj->syslog_priority,"%s",msg_obj->message.c_str());
			}
			else
			{
				openlog("tinytrapd", LOG_PID|LOG_CONS, msg_obj->syslog_facility);
				syslog(msg_obj->syslog_priority,"%s",msg_obj->message.c_str());
				closelog();
			}
		}
		if(msg_obj->log_type == LOGGER_MAIN_FILE)
		{ 
			*fds << msg_obj->message.c_str() << std::flush;
		}
		
		if(msg_obj->log_type == LOGGER_SEPARATE_FILE)
		{
			std::fstream custom_file(msg_obj->filename.c_str(), std::ios_base::out | std::ios_base::app);
			if(!custom_file)
			{
#ifdef DEBUG
				std::cerr << "Unable to open file " << msg_obj->filename << "\n";
#endif
				//return -1;
			}
			else
			{
				custom_file << msg_obj->message.c_str() << std::flush;
			}
			custom_file.close();
		}
		delete msg_obj;
		
	}
}
int Logger::start_queue_thread()
{
	pthread_attr_t attr;
	/* For portability, explicitly create threads in a joinable state */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_t dispatch_thread;
	return pthread_create(&dispatch_thread, &attr, start_dispatcher, static_cast<void*>(this));
}

void* Logger::start_dispatcher(void* logger_object)
{
	Logger *l = static_cast<Logger*>(logger_object);
	l->dispatch();
}
int Logger::feed_queue(logger_unit *lu)
{
	
	pthread_mutex_lock (&message_queue.lock);
	while (message_queue.full)
	{
#ifdef DEBUG
		std::cout << "feed: queue FULL. Wait condition.\n";
#endif
		pthread_cond_wait (&message_queue.not_full, &message_queue.lock);
#ifdef DEBUG
		std::cout << "feed: condition not full realeased\n ";
#endif
	}
#ifdef DEBUG
	std::cout << "feed queue message lu " << lu << "\n";
#endif
	message_queue.objs.push(lu);
	message_queue.empty = false;
	if(message_queue.objs.size() == LOGGER_QUEUE_SIZE)
	{
		message_queue.full = true;
	}
	pthread_mutex_unlock (&message_queue.lock);
	pthread_cond_signal (&message_queue.not_empty);
}

int Logger::log_message_global(std::string &message)
{
	logger_unit *lu;
	
	if(use_syslog)
	{
		log_message(facility,priority,message);
	}
	/* checking if log to main file enabled is performed inside function itself */
	log_message(message);
	
}

int Logger::log_message(std::string &message)
{
	/* log to main file only if it globally enabled*/
	if(use_main_file)
	{
		logger_unit *lu = new logger_unit;
		lu->log_type = LOGGER_MAIN_FILE;
		lu->message = now() + std::string(" ") + message;
		feed_queue(lu);
	}
}
int Logger::log_message(int s_facility, int s_priority, std::string &message)
{
	logger_unit *lu = new logger_unit;
	
	lu->syslog_facility = s_facility;
	lu->syslog_priority = s_priority;
	lu->log_type = LOGGER_SYSLOG;
	lu->message = message;
	feed_queue(lu);
}
int Logger::log_message(std::string &message, std::string &filename)
{
	logger_unit *lu = new logger_unit;
	lu->log_type = LOGGER_SEPARATE_FILE;
	lu->filename = filename;
	lu->message = now() + std::string(" ") + message;
	
	feed_queue(lu);
}

std::string Logger::now()
{
	return now("%d.%m.%Y %H:%M:%S");
}
std::string Logger::now(const char* format)
{
	time_t ts;
	time( &ts );   // get the calendar time
	tm *t = localtime( &ts );  // convert to local
	char timestr[20];
	strftime( timestr, 200, format, t );
	return timestr;
}
