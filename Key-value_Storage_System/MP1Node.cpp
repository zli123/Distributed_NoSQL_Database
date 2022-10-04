/**********************************
 * FILE NAME: MP1Node.cpp
 *
 * DESCRIPTION: Membership protocol run by this Node.
 * 				Definition of MP1Node class functions. (Revised 2020)
 *
 *  Starter code template
 **********************************/

#include "MP1Node.h"

/*
 * Note: You can change/add any functions in MP1Node.{h,cpp}
 */

/**
 * Overloaded Constructor of the MP1Node class
 * You can add new members to the class if you think it
 * is necessary for your logic to work
 */
MP1Node::MP1Node( Params *params, EmulNet *emul, Log *log, Address *address) {
	for( int i = 0; i < 6; i++ ) {
		NULLADDR[i] = 0;
	}
	this->memberNode = new Member;
    this->shouldDeleteMember = true;
	memberNode->inited = false;
	this->emulNet = emul;
	this->log = log;
	this->par = params;
	this->memberNode->addr = *address;
}

/**
 * Overloaded Constructor of the MP1Node class
 * You can add new members to the class if you think it
 * is necessary for your logic to work
 */
MP1Node::MP1Node(Member* member, Params *params, EmulNet *emul, Log *log, Address *address) {
    for( int i = 0; i < 6; i++ ) {
        NULLADDR[i] = 0;
    }
    this->memberNode = member;
    this->shouldDeleteMember = false;
    this->emulNet = emul;
    this->log = log;
    this->par = params;
    this->memberNode->addr = *address;
}

/**
 * Destructor of the MP1Node class
 */
MP1Node::~MP1Node() {
    if (shouldDeleteMember) {
        delete this->memberNode;
    }
}

/**
* FUNCTION NAME: recvLoop
*
* DESCRIPTION: This function receives message from the network and pushes into the queue
*                 This function is called by a node to receive messages currently waiting for it
*/
int MP1Node::recvLoop() {
    if ( memberNode->bFailed ) {
    	return false;
    }
    else {
    	return emulNet->ENrecv(&(memberNode->addr), enqueueWrapper, NULL, 1, &(memberNode->mp1q));
    }
}

/**
 * FUNCTION NAME: enqueueWrapper
 *
 * DESCRIPTION: Enqueue the message from Emulnet into the queue
 */
int MP1Node::enqueueWrapper(void *env, char *buff, int size) {
	Queue q;
	return q.enqueue((queue<q_elt> *)env, (void *)buff, size);
}

/**
* FUNCTION NAME: nodeStart
*
* DESCRIPTION: This function bootstraps the node
*                 All initializations routines for a member.
*                 Called by the application layer.
*/
void MP1Node::nodeStart(char *servaddrstr, short servport) {
    Address joinaddr;
    joinaddr = getJoinAddress();

    // Self booting routines
    if( initThisNode(&joinaddr) == -1 ) {
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "init_thisnode failed. Exit.");
#endif
        exit(1);
    }

    if( !introduceSelfToGroup(&joinaddr) ) {
        finishUpThisNode();
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Unable to join self to group. Exiting.");
#endif
        exit(1);
    }

    return;
}

/**
 * FUNCTION NAME: initThisNode
 *
 * DESCRIPTION: Find out who I am and start up
 */
int MP1Node::initThisNode(Address *joinaddr) {
    /*
    * This function is partially implemented and may require changes
    */
	int id = *(int*)(&memberNode->addr.addr);
	int port = *(short*)(&memberNode->addr.addr[4]);

	memberNode->bFailed = false;
	memberNode->inited = true;
	memberNode->inGroup = false;
	// node is up!
	memberNode->nnb = 0;
	memberNode->heartbeat = 0;
	memberNode->pingCounter = TFAIL;
	memberNode->timeOutCounter = -1;
	initMemberListTable(memberNode);

	addToMembershipList(id, port, memberNode->heartbeat);
    //memberNode->myPos = memberNode->memberList.begin();

    return 0;
}

/**
 * FUNCTION NAME: introduceSelfToGroup
 *
 * DESCRIPTION: Join the distributed system
 */
int MP1Node::introduceSelfToGroup(Address *joinaddr) {
	MessageHdr *msg;
#ifdef DEBUGLOG
    static char s[1024];
#endif

    if ( 0 == strcmp((char *)&(memberNode->addr.addr), (char *)&(joinaddr->addr))) {
        // I am the group booter (first process to join the group). Boot up the group
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Starting up group...");
#endif
        memberNode->inGroup = true;
    }
    else {
        size_t msgsize = sizeof(MessageHdr) + sizeof(joinaddr->addr) + sizeof(long) + 1;
        msg = (MessageHdr *) malloc(msgsize * sizeof(char));

        // create JOINREQ message: format of data is {struct Address myaddr}
        msg->msgType = JOINREQ;
        memcpy((char *)(msg+1), &memberNode->addr.addr, sizeof(memberNode->addr.addr));
        memcpy((char *)(msg+1) + 1 + sizeof(memberNode->addr.addr), &memberNode->heartbeat, sizeof(long));

#ifdef DEBUGLOG
        sprintf(s, "Trying to join...");
        log->LOG(&memberNode->addr, s);
#endif

        // send JOINREQ message to introducer member
        emulNet->ENsend(&memberNode->addr, joinaddr, (char *)msg, msgsize);

        free(msg);
    }

    return 1;

}

/**
* FUNCTION NAME: finishUpThisNode
*
* DESCRIPTION: Wind up this node and clean up state
*/
int MP1Node::finishUpThisNode(){
    /*
     * Your code goes here
     */
     return 0;
}

/**
* FUNCTION NAME: nodeLoop
*
* DESCRIPTION: Executed periodically at each member
*                 Check your messages in queue and perform membership protocol duties
*/
void MP1Node::nodeLoop() {
    if (memberNode->bFailed) {
    	return;
    }

    // Check my messages
    checkMessages();

    // Wait until you're in the group...
    if( !memberNode->inGroup ) {
    	return;
    }

    // ...then jump in and share your responsibilites!
    nodeLoopOps();

    return;
}

/**
 * FUNCTION NAME: checkMessages
 *
 * DESCRIPTION: Check messages in the queue and call the respective message handler
 */
void MP1Node::checkMessages() {
    void *ptr;
    int size;

    // Pop waiting messages from memberNode's mp1q
    while ( !memberNode->mp1q.empty() ) {
    	ptr = memberNode->mp1q.front().elt;
    	size = memberNode->mp1q.front().size;
    	memberNode->mp1q.pop();
    	recvCallBack((void *)memberNode, (char *)ptr, size);
    }
    return;
}

/**
 * FUNCTION NAME: recvCallBack
 *
 * DESCRIPTION: Message handler for different message types
 */
bool MP1Node::recvCallBack(void *env, char *data, int size ) {
    /*
    * Your code goes here
    */
	MessageHdr *msg = (MessageHdr *)data;
	char *membershipListString = (char *)(msg + 1);

    if(msg->msgType == JOINREQ){
        Address from;
        long heartbeat;
        memcpy(from.addr, membershipListString, sizeof(from.addr));
        memcpy(&heartbeat, membershipListString + sizeof(from.addr) + 1, sizeof(long));

        int id = *(int*)(&from.addr);
        short port = *(short*)(&from.addr[4]);

        char *table = membershipListToString();

        int size = sizeof(MessageHdr) + strlen(table);
        MessageHdr *message = (MessageHdr *) malloc(size * sizeof(char));
        message->msgType = JOINREP;
        char * ptr = (char *) (message + 1);
        memcpy(ptr, table, strlen(table));

        emulNet->ENsend(&memberNode->addr, &from, (char *)message, size);
        free(message);
        free(table);

        vector<MemberListEntry>::iterator it = search(id);
        if(it == memberNode->memberList.end()){
            addToMembershipList(id, port, heartbeat);
        }
    }
    else if(msg->msgType == JOINREP){
        memberNode->inGroup = true;
        readMessage(membershipListString);
    }
    else if(msg->msgType == GOSSIP){
        readMessage(membershipListString);
	}

    return true;
}

/**
* FUNCTION NAME: nodeLoopOps
*
* DESCRIPTION: Check if any node hasn't responded within a timeout period and then delete
*                 the nodes
*                 Propagate your membership list
*/
void MP1Node::nodeLoopOps() {
    
    /*
     * Your code goes here
     */
	memberNode->heartbeat++;
    //memberNode->myPos->setheartbeat(memberNode->heartbeat);
    if(!memberNode->memberList.empty()){
        string address = memberNode->addr.getAddress();
        int id = stoi(address);
        for(int i = 0; i < memberNode->memberList.size(); i++){
            if(id != memberNode->memberList[i].id && (par->getcurrtime() - (memberNode->memberList[i].timestamp)) > TREMOVE){
                string addr = to_string(memberNode->memberList[i].id) + ":" + to_string(memberNode->memberList[i].port);
                Address toBeDeleted(addr);
                memberNode->memberList.erase(memberNode->memberList.begin() + i);
                log->logNodeRemove(&memberNode->addr, &toBeDeleted);
            }else if(id == memberNode->memberList[i].id){
    			memberNode->memberList[i].setheartbeat(memberNode->heartbeat);
            }
        }
    }

	if(memberNode->pingCounter == 0){
		char *table = membershipListToString();

		int size = sizeof(MessageHdr) + strlen(table) + 1;
		MessageHdr *msg = (MessageHdr *)malloc(size * sizeof(char));
		msg->msgType = GOSSIP;
		char *ptr = (char *)(msg + 1);
		memcpy(ptr, table, strlen(table) + 1);
        string address = memberNode->addr.getAddress();
        int id = stoi(address);
		for(int i = 0; i < memberNode->memberList.size(); i++){
            string addr = to_string(memberNode->memberList[i].id) + ":" + to_string(memberNode->memberList[i].port);
			Address gossip(addr);
			if (memberNode->memberList[i].id != id) {
				emulNet->ENsend(&memberNode->addr, &gossip, (char *)msg, size);
			}
		}
		free(msg);
		free(table);

		memberNode->pingCounter = TGOSSIP;
	}else{
        memberNode->pingCounter--;
    }

	return;
}

void MP1Node::readMessage(char *data){
    char *temp;
    temp = strtok(data, ">");
    while(temp != NULL){
        char *nodeString = NULL;
        asprintf(&nodeString, "%s", temp);
        int id;
        short port;
        long heartbeat;
        long timestamp;

        sscanf(nodeString, "%d.%hi.%ld.%ld", &id, &port, &heartbeat, &timestamp);
        vector<MemberListEntry>::iterator loc = search(id);
        if(loc != memberNode->memberList.end()){
            if((*(loc)).getheartbeat() < heartbeat){
            	(*(loc)).setheartbeat(heartbeat);
            	(*(loc)).settimestamp(par->getcurrtime());
            }
        }else{
            addToMembershipList(id, port, heartbeat);
        }
        temp = strtok(NULL, ">");
        free(nodeString);
    }
}

void MP1Node::addToMembershipList(int id, short port, long heartbeat){
	MemberListEntry newEntry(id, port, heartbeat, par->getcurrtime());
	memberNode->memberList.push_back(newEntry);
    string addr = to_string(id) + ":" + to_string(port);
    Address newAddress(addr);
	log->logNodeAdd(&memberNode->addr, &newAddress);
}

char* MP1Node::membershipListToString(){
    char *result = NULL;
    for(int i = 0; i < memberNode->memberList.size(); i++){
        char *nodeString = NULL;
        asprintf(&nodeString, "%d.%hi.%ld.%ld", memberNode->memberList[i].id, memberNode->memberList[i].port, memberNode->memberList[i].heartbeat, memberNode->memberList[i].timestamp);
        if(result == NULL) {
            asprintf(&result, "%s>", nodeString);
        }
        else {
            asprintf(&result, "%s%s>", result, nodeString);
        }
        if(nodeString) {
            free(nodeString);
        }
    }
    return result;
}

vector<MemberListEntry>::iterator MP1Node::search(int id){
	vector<MemberListEntry>::iterator it;
	for(it = memberNode->memberList.begin(); it != memberNode->memberList.end(); ++it){
		if((*(it)).getid() == id){
			return it;
		}
	}
	return it;
}

/**
 * FUNCTION NAME: isNullAddress
 *
 * DESCRIPTION: Function checks if the address is NULL
 */
int MP1Node::isNullAddress(Address *addr) {
	return (memcmp(addr->addr, NULLADDR, 6) == 0 ? 1 : 0);
}

/**
 * FUNCTION NAME: getJoinAddress
 *
 * DESCRIPTION: Returns the Address of the coordinator
 */
Address MP1Node::getJoinAddress() {
    Address joinaddr;

    *(int *)(&joinaddr.addr) = 1;
    *(short *)(&joinaddr.addr[4]) = 0;

    return joinaddr;
}

/**
 * FUNCTION NAME: initMemberListTable
 *
 * DESCRIPTION: Initialize the membership list
 */
void MP1Node::initMemberListTable(Member *memberNode) {
	memberNode->memberList.clear();
}

/**
 * FUNCTION NAME: printAddress
 *
 * DESCRIPTION: Print the Address
 */
void MP1Node::printAddress(Address *addr)
{
    printf("%d.%d.%d.%d:%d \n",  addr->addr[0],addr->addr[1],addr->addr[2],
                                                       addr->addr[3], *(short*)&addr->addr[4]) ;    
}
