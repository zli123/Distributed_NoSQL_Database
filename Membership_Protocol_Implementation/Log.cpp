/**********************************
 * FILE NAME: Log.cpp
 *
 * DESCRIPTION: Log class definition (Revised 2020)
 **********************************/

#include "Log.h"
#include <iostream>
#include <string>
/**
 * Constructor
 */
Log::Log(Params *p) {
	par = p;
	firstTime = false;
}

/**
 * Copy constructor
 */
Log::Log(const Log &anotherLog) {
	this->par = anotherLog.par;
	this->firstTime = anotherLog.firstTime;
}

/**
 * Assignment Operator Overloading
 */
Log& Log::operator = (const Log& anotherLog) {
	this->par = anotherLog.par;
	this->firstTime = anotherLog.firstTime;
	return *this;
}

/**
 * Destructor
 */
Log::~Log() {}

/**
 * FUNCTION NAME: LOG
 *
 * DESCRIPTION: Print out to file dbg.log, along with Address of node.
 */
void Log::LOG(Address *addr, const char * str, ...) {

	static FILE *fp;
	static FILE *fp2;
	va_list vararglist;
	static char buffer[30000];
	static int numwrites;
	static char stdstring[1000];
	static char stdstring2[1000];
	static char stdstring3[1000]; 
	static int dbg_opened=0;

	if(dbg_opened != 639){
		numwrites=0;

		stdstring2[0]=0;

		strcpy(stdstring3, stdstring2);

		strcat(stdstring2, DBG_LOG);
		strcat(stdstring3, STATS_LOG);

		fp = fopen(stdstring2, "w");
		fp2 = fopen(stdstring3, "w");

		dbg_opened=639;
	}
	else 

	sprintf(stdstring, "%d.%d.%d.%d:%d ", addr->addr[0], addr->addr[1], addr->addr[2], addr->addr[3], *(short *)&addr->addr[4]);

	va_start(vararglist, str);
	vsprintf(buffer, str, vararglist);
	va_end(vararglist);

	if (!firstTime) {
		int magicNumber = 0;
		string magic = MAGIC_NUMBER;
		int len = magic.length();
		for ( int i = 0; i < len; i++ ) {
			magicNumber += (int)magic.at(i);
		}
		fprintf(fp, "%x\n", magicNumber);
		firstTime = true;
	}

	if(memcmp(buffer, "#STATSLOG#", 10)==0){
		fprintf(fp2, "\n %s", stdstring);
		fprintf(fp2, "[%d] ", par->getcurrtime());

		fprintf(fp2, buffer);
	}
	else{
		fprintf(fp, "\n %s", stdstring);
		fprintf(fp, "[%d] ", par->getcurrtime());
		fprintf(fp, buffer);

	}

	if(++numwrites >= MAXWRITES){
		fflush(fp);
		fflush(fp2);
		numwrites=0;
	}

}

/**
 * FUNCTION NAME: logNodeAdd
 *
 * DESCRIPTION: To Log a node add
 */
void Log::logNodeAdd(Address *thisNode, Address *addedAddr) {
	static char stdstring[1000];
	sprintf(stdstring, "Node %d.%d.%d.%d:%d joined at time %d", addedAddr->addr[0], addedAddr->addr[1], addedAddr->addr[2], addedAddr->addr[3], *(short *)&addedAddr->addr[4], par->getcurrtime());
    std::string string1(stdstring);
    std::cout << "stdstring1 length " << string1.length() << std::endl;
    LOG(thisNode, stdstring);
}

/**
 * FUNCTION NAME: logNodeRemove
 *
 * DESCRIPTION: To log a node remove
 */
void Log::logNodeRemove(Address *thisNode, Address *removedAddr) {
	static char stdstring[1000];
	sprintf(stdstring, "Node %d.%d.%d.%d:%d removed at time %d", removedAddr->addr[0], removedAddr->addr[1], removedAddr->addr[2], removedAddr->addr[3], *(short *)&removedAddr->addr[4], par->getcurrtime());
    std::string string2(stdstring);
    std::cout << "stdstring2 length " << string2.length() << std::endl;
    LOG(thisNode, stdstring);
}
