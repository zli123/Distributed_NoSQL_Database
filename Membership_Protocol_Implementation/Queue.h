/**********************************
 * FILE NAME: Queue.h
 *
 * DESCRIPTION: Header file for std::<queue> related functions (Revised 2020)
 **********************************/

#ifndef QUEUE_H_
#define QUEUE_H_

#include "stdincludes.h"
#include "Member.h"

/**
 * Class name: Queue
 *
 * Description: This function wraps std::queue related functions
 */
class Queue {
public:
	Queue() {}
	virtual ~Queue() {}
	static bool enqueue(queue<q_elt> *queue, void *buffer, int size) {
		q_elt element(buffer, size);
		queue->emplace(element);
		//cout<<endl<<"QUEUE size at this point is: "<<queue->size();
		return true;
	}
};

#endif /* QUEUE_H_ */
