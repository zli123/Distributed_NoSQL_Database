/**********************************
 * FILE NAME: Log.h
 *
 * DESCRIPTION: Header file of Log class (Revised 2020)
 **********************************/

#ifndef _LOG_H_
#define _LOG_H_

#include "stdincludes.h"
#include "Params.h"
#include "Member.h"

/*
 * Macros
 */
// number of writes after which to flush file
#define MAXWRITES 1
#define MAGIC_NUMBER "CS425"
#define DBG_LOG "dbg.log"
#define STATS_LOG "stats.log"

/**
 * CLASS NAME: Log
 *
 * DESCRIPTION: Functions to log messages in a debug log
 */
class Log{
private:
	Params *par;
	bool firstTime;
public:
	Log(Params *p);
	Log(const Log &anotherLog);
	Log& operator = (const Log &anotherLog);
	virtual ~Log();
	void LOG(Address *, const char * str, ...);
	void logNodeAdd(Address *, Address *);
	void logNodeRemove(Address *, Address *);
};

#endif /* _LOG_H_ */
