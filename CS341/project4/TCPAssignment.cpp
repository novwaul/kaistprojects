/*
 * E_TCPAssignment.cpp
 *
 *  Created on: 2014. 11. 20.
 *      Author: Keunhong Lee
 */


#include <E/E_Common.hpp>
#include <E/Networking/E_Host.hpp>
#include <E/Networking/E_Networking.hpp>
#include <cerrno>
#include <E/Networking/E_Packet.hpp>
#include <E/Networking/E_NetworkUtil.hpp>
#include "TCPAssignment.hpp"

namespace E
{

TCPAssignment::TCPAssignment(Host* host) : HostModule("TCP", host),
		NetworkModule(this->getHostModuleName(), host->getNetworkSystem()),
		SystemCallInterface(AF_INET, IPPROTO_TCP, host),
		NetworkLog(host->getNetworkSystem()),
		TimerModule(host->getSystem())
{

}

TCPAssignment::~TCPAssignment()
{

}

void TCPAssignment::initialize()
{

}

void TCPAssignment::finalize()
{

}

void TCPAssignment::systemCallback(UUID syscallUUID, int pid, const SystemCallParameter& param)
{
	switch(param.syscallNumber)
	{
	case SOCKET:
		this->syscall_socket(syscallUUID, pid, param.param1_int, param.param2_int);
		break;
	case CLOSE:
		this->syscall_close(syscallUUID, pid, param.param1_int);
		break;
	case READ:
		this->syscall_read(syscallUUID, pid, param.param1_int, param.param2_ptr, param.param3_int);
		break;
	case WRITE:
		this->syscall_write(syscallUUID, pid, param.param1_int, param.param2_ptr, param.param3_int);
		break;
	case CONNECT:
		this->syscall_connect(syscallUUID, pid, param.param1_int,
				static_cast<struct sockaddr*>(param.param2_ptr),
				(socklen_t)param.param3_int);
		break;
	case LISTEN:
		this->syscall_listen(syscallUUID, pid, param.param1_int, param.param2_int);
		break;
	case ACCEPT:
		this->syscall_accept(syscallUUID, pid, param.param1_int,
				static_cast<struct sockaddr*>(param.param2_ptr),
				static_cast<socklen_t*>(param.param3_ptr));
		break;
	case BIND:
		this->syscall_bind(syscallUUID, pid, param.param1_int,
				static_cast<struct sockaddr *>(param.param2_ptr),
				(socklen_t) param.param3_int);
		break;
	case GETSOCKNAME:
		this->syscall_getsockname(syscallUUID, pid, param.param1_int,
				static_cast<struct sockaddr *>(param.param2_ptr),
				static_cast<socklen_t*>(param.param3_ptr));
		break;
	case GETPEERNAME:
		this->syscall_getpeername(syscallUUID, pid, param.param1_int,
				static_cast<struct sockaddr *>(param.param2_ptr),
				static_cast<socklen_t*>(param.param3_ptr));
		break;
	default:
		assert(0);
	}
}
void TCPAssignment::syscall_write(UUID syscallUUID, int pid, int param1_int, void* param2_ptr, int param3_int)
{
	std::vector<struct cell*>::iterator iter;
	for(iter = fdList.begin(); iter != fdList.end(); iter++)
	{
		if((*iter)->sockfd == param1_int && pid == (*iter)->pid ) break;
	}
	if(iter == fdList.end())
	{
		returnSystemCall(syscallUUID, -1);
		return;
	}
	if((*iter)->state != CONNECTING)
	{
		returnSystemCall(syscallUUID,-1);
		return;
	}
	(*iter)->curSend = 0;
	(*iter)->totalSend = param3_int;
	uint16_t swnd, inBuf = 0;
	Time curTime;
	char*payload = (char*)param2_ptr;
	int size, remain, first = true;
	for(std::vector<S_Packet *>::iterator packet=(*iter)->internalBuf.begin();packet!=(*iter)->internalBuf.end();packet++)
	{
		inBuf+=(*packet)->size;
	}
	swnd = (*iter)->cwnd - inBuf;
	if(param3_int < swnd) remain = param3_int;
	else remain = swnd;
	while(remain!=0)
	{
		if(remain < MSS) size = remain;
		else size = MSS;
		/* make ACK PACKET */
		uint8_t flag = 1<<4; //ack bit
		uint8_t headerlen = 5<<4;
		uint16_t r_temp, rcvw = (*iter)->maxRecvBufSize;
		for(std::vector<Packet*>::iterator it = (*iter)->recvBuf.begin();it!=(*iter)->recvBuf.end();it++)
		{
			(*it)->readData(14+2,(uint8_t*)&r_temp,2);
			rcvw -= (ntohs(r_temp)-40);
		}
		if((*iter)->readOffset!=0) rcvw+=(*iter)->readOffset;
		rcvw = htons(rcvw);
		Packet *packetToSend = allocatePacket(54+size);
		packetToSend->writeData(14+12,(*iter)->srcAddr,4);
		packetToSend->writeData(14+16,(*iter)->destAddr,4);
		packetToSend->writeData(34+0,(*iter)->srcPortNum,2);
		packetToSend->writeData(34+2,(*iter)->destPortNum,2);
		packetToSend->writeData(34+4,(uint8_t*)&(*iter)->presentSeq,4);
		packetToSend->writeData(34+8,(uint8_t*)&(*iter)->presentAck,4);
		packetToSend->writeData(34+12,&headerlen,1);
		packetToSend->writeData(34+13,&flag,1);
		packetToSend->writeData(34+14,&rcvw,2);
		packetToSend->writeData(34+20,payload,size);
		uint16_t checksum = 0;
		uint8_t buffer[20+size];
		packetToSend->readData(34+0,buffer,20+size);
		checksum = NetworkUtil::tcp_sum(((uint32_t*)((*iter)->srcAddr))[0],((uint32_t*)((*iter)->destAddr))[0],buffer,20+size);
		checksum = ~checksum;
		checksum = htons(checksum);
		packetToSend->writeData(34+16,(uint8_t*)&checksum,2);
		this->sendPacket("IPv4",packetToSend);
		if(first == true)
		{
			(*iter)->timerUUID = addTimer((*iter), (*iter)->RTO);
			first = false;
		}
		/* update information */
		curTime = this->getHost()->getSystem()->getCurrentTime();
		(*iter)->sendTime.push_back(curTime);
		Packet *packetToStore = clonePacket(packetToSend);
		struct S_Packet* Store = new struct S_Packet;
		Store->size = size;
		Store->packet = packetToStore;
		(*iter)->internalBuf.push_back(Store);
		(*iter)->presentSeq = htonl(ntohl((*iter)->presentSeq)+size);
		(*iter)->curSend+=size;
		payload += size;
		if(remain < MSS) remain = 0;
		else remain -= MSS;
	}
	if((*iter)->totalSend == (*iter)->curSend)
	{
		(*iter)->ptr = NULL;
		returnSystemCall(syscallUUID,param3_int);
	}
	else
	{
		(*iter)->ptr = payload;
		(*iter)->waitUUID = syscallUUID;
	}
	return;
}
/*-------------------------------------------------------------------------------------------------------------------------------------*/
void TCPAssignment::syscall_read(UUID syscallUUID, int pid, int param1_int, void* param2_ptr, int param3_int)
{
	std::vector<struct cell*>::iterator iter;
	for(iter=fdList.begin();iter!=fdList.end();iter++)
	{
		if((*iter)->sockfd == param1_int && (*iter)->pid == pid) break;
	}
	if(iter==fdList.end())
	{
		returnSystemCall(syscallUUID,-1);
		return;
	}
	if((*iter)->state != CONNECTING)
	{
		returnSystemCall(syscallUUID,-1);
		return;
	}
	(*iter)->waitUUID = syscallUUID;
	(*iter)->ptr = param2_ptr;
	if((*iter)->pending != READING)
	{
		(*iter)->startAck = (*iter)->presentAck;
		(*iter)->readOffset = 0;
	}
	(*iter)->pending = READING;
	(*iter)->totalRead = param3_int;
	(*iter)->curRead = 0;
	if((*iter)->recvBuf.size()==0) return;
	char* payload = (char*)param2_ptr;
	int pos = -1;
	uint32_t cur,next;
	uint16_t len;
	cur = ntohl((*iter)->startAck)+(*iter)->curRead;
	for(std::vector<Packet*>::iterator rcvp = (*iter)->recvBuf.begin();rcvp!=(*iter)->recvBuf.end();rcvp++)
	{
		if((*iter)->curRead == (*iter)->totalRead) break;
		(*rcvp)->readData(34+4,(uint8_t*)&next,4);
		next = ntohl(next);
		if(cur!=next) break;
		(*rcvp)->readData(14+2,(uint8_t*)&len,2);
		len = ntohs(len)-40;
		/* continue reading from offset */
		if((*iter)->readOffset!=0)
		{
			if(len-(*iter)->readOffset > (*iter)->totalRead - (*iter)->curRead)
			{
				len = (*iter)->totalRead-(*iter)->curRead;
				(*rcvp)->readData(54+(*iter)->readOffset,payload,len);
				(*iter)->readOffset+=len;
				(*iter)->curRead+=len;
				payload+=len;
			}
			else
			{
				(*rcvp)->readData(54+(*iter)->readOffset,payload,len-(*iter)->readOffset);
				cur+=len;
				(*iter)->startAck = htonl(ntohl((*iter)->startAck)+(*iter)->readOffset);
				payload+=len-(*iter)->readOffset;
				(*iter)->curRead+=len-(*iter)->readOffset;
				(*iter)->readOffset = 0;
				pos++;
			}
		}
		/* not enough user buffer */
		else if(len>(*iter)->totalRead-(*iter)->curRead)
		{
			len = (*iter)->totalRead-(*iter)->curRead;
			(*rcvp)->readData(54,payload,len);
			(*iter)->curRead+=len;
			(*iter)->readOffset = len;
			payload+=len;
		}
		/* normal */
		else
		{
			cur+=len;
			(*rcvp)->readData(54,payload,len);
			payload+=len;
			(*iter)->curRead+=len;
			pos++;
		}
	}
	while(pos!=-1)
	{
		freePacket((*iter)->recvBuf[pos]);
		(*iter)->recvBuf.erase((*iter)->recvBuf.begin()+pos);
		pos--;
	}
	if((*iter)->curRead == param3_int)
	{
		if((*iter)->readOffset==0)
		{
			(*iter)->pending = NONE;
		}
		(*iter)->startAck = htonl(cur);
		(*iter)->ptr = NULL;
		returnSystemCall(syscallUUID,param3_int);
		return;
	}
	else if((*iter)->curRead < param3_int)
	{
		(*iter)->ptr = payload;
		return;
	}
	else assert(0);
}
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/
void TCPAssignment::packetArrived(std::string fromModule, Packet* packet)
{
	char srcAddr[4], destAddr[4], srcPortNum[2], destPortNum[2], signBit[1], seq[4], ack[4];
	uint16_t peerRwnd, packetSize;
  if(fromModule == "IPv4")
	{
		packet->readData(14+2, (uint8_t*)&packetSize, 2);
		packetSize = ntohs(packetSize) - 40;
		/* get src IP, src Port num, dest IP, dest Port num */
		packet->readData(14+12, destAddr, 4);
		packet->readData(14+16, srcAddr, 4);
		packet->readData(34+0, destPortNum, 2);
		packet->readData(34+2, srcPortNum,2);
		/* get sequence number */
		packet->readData(34+4, ack, 4);
		/* get ack number */
		packet->readData(34+8, seq, 4);
		/* get sign bits */
		packet->readData(34+13, signBit, 1);
		packet->readData(34+14,(uint8_t*)&peerRwnd,2);
		peerRwnd = ntohs(peerRwnd);


/*** DETERMIN RECEIVED PACKET  ***/
/*---------------------------------------------------------------------------------------------------------------*/
/* SYN-ACK PACKET received */
		if(signBit[0] == 18)
		{
			this->freePacket(packet);//free
			std::vector<struct cell*>::iterator iter;
	    for ( iter = fdList.begin() ; iter != fdList.end(); ++iter)
			{
				if((*iter)->state == SYN_SENT )
				{
					if((((uint32_t*)((*iter)->srcAddr))[0] == ((uint32_t*)(srcAddr))[0]) &&
					((uint16_t*)((*iter)->srcPortNum))[0] == ((uint16_t*)(srcPortNum))[0] &&
					(((uint32_t*)((*iter)->destAddr))[0] == ((uint32_t*)(destAddr))[0] )&&
					((uint16_t*)((*iter)->destPortNum))[0] == ((uint16_t*)(destPortNum))[0])
					break;
				}
				else continue;
			}
			if(iter == fdList.end()) return;
			(*iter)->state = CONNECTING;
			(*iter)->presentAck = htonl(ntohl(((uint32_t*)ack)[0])+1);
			/* make ACK packet */
			uint8_t flag = 1<<4; //ack bit
			uint8_t headerlen = 5<<4;
			uint16_t checksum = 0;
			uint16_t rcvw = (*iter)->maxRecvBufSize;
			rcvw = htons(rcvw);
			Packet* myPacket = allocatePacket((size_t)54);
			myPacket->writeData(14+12,srcAddr,4);
			myPacket->writeData(14+16,(*iter)->destAddr,4);
			myPacket->writeData(34+0,srcPortNum,2);
			myPacket->writeData(34+2,destPortNum,2);
			myPacket->writeData(34+4,(uint8_t*)&(*iter)->presentSeq,4);
			myPacket->writeData(34+8,(uint8_t*)&(*iter)->presentAck,4);
			myPacket->writeData(34+12, &headerlen, 1);
			myPacket->writeData(34+13,&flag,1);
			myPacket->writeData(34+14,(uint8_t*)&rcvw, 2);
			myPacket->writeData(34+16, (uint8_t*)&checksum, 2);
			uint8_t header_buffer[20];
			myPacket->readData(34, header_buffer, 20);
			checksum = NetworkUtil::tcp_sum(((uint32_t*)((*iter)->srcAddr))[0],((uint32_t*)((*iter)->destAddr))[0],header_buffer,20);
			checksum = ~checksum;
			checksum = htons(checksum);
			myPacket->writeData(34+16, (uint8_t*)&checksum, 2);
			/* send packet*/
			this->sendPacket("IPv4", myPacket);
			struct sockaddr* param2_ptr = (struct sockaddr*)(*iter)->ptr;
			(*iter)->ptr = NULL;
			(*iter)->LastByteRcvd = ntohl((*iter)->presentSeq)-1;
			param2_ptr->sa_family = (*iter)->family;
			((uint16_t*)(param2_ptr->sa_data))[0] = ((uint16_t*)((*iter)->destPortNum))[0];
			((uint32_t*)(param2_ptr->sa_data + 2))[0] = ((uint32_t*)((*iter)->destAddr))[0];
			this->returnSystemCall((*iter)->waitUUID, 0);// return connect success value, which is 0
			return;
		}
/*--------------------------------------------------------------------------------------------------------------------------------------*/
/* SYN PACKET received */
		else if(signBit[0] == 1<<1)
		{
			std::vector<struct cell*>::iterator iter;
			/* Find SYN_SENT Socket */
			for ( iter = fdList.begin() ; iter != fdList.end(); ++iter)
			{
				if((*iter)->state == SYN_SENT )
				{
					if((((uint32_t*)((*iter)->srcAddr))[0] == ((uint32_t*)(srcAddr))[0] ) &&
					((uint16_t*)((*iter)->srcPortNum))[0] == ((uint16_t*)(srcPortNum))[0] &&
					(((uint32_t*)((*iter)->destAddr))[0] == ((uint32_t*)(destAddr))[0] )&&
					((uint16_t*)((*iter)->destPortNum))[0] == ((uint16_t*)(destPortNum))[0])
					break;
				}
				else continue;
			}
			if(iter != fdList.end())//SYN_SENT Socket is found
			{
				(*iter)->state = SYN_RCV;//Transition to SYN_RCV
				this->freePacket(packet);
				/* make syn-ack packet */
				(*iter)->presentAck = htonl(ntohl(((uint32_t*)ack)[0])+1);
				uint8_t flag[1] ; // syn-ack bit
				flag[0] = 18;
				uint8_t headerlen = 5<<4;// (5*32)/8 = 20
				uint16_t urg = 0;
				uint32_t temp_seq = (*iter)->presentSeq;
				uint16_t checksum = 0;
				uint16_t rcvw = (*iter)->maxRecvBufSize; // window size
				rcvw = htons(rcvw);
				temp_seq = htonl(ntohl(temp_seq) -1);
				Packet* myPacket2 = allocatePacket(54); // TCP 20B + IP 20B + Etype 14B
				myPacket2->writeData(14+12,srcAddr,4);
				myPacket2->writeData(14+16,(*iter)->destAddr,4); // already stored in Big-endian
				myPacket2->writeData(34+0,srcPortNum,2);// already stored in Big-endian
				myPacket2->writeData(34+2,destPortNum,2);// already stored in Big-endian
				myPacket2->writeData(34+4,(uint8_t*)&temp_seq,4);//sequence num
				myPacket2->writeData(34+8,(uint8_t*)&(*iter)->presentAck,4);// ack num
				myPacket2->writeData(34+12,&headerlen,1);// header len
				myPacket2->writeData(34+13,flag,1);//syn-ack
				myPacket2->writeData(34+14,(uint8_t*)&rcvw, 2);// rcvw size
				myPacket2->writeData(34+16, (uint8_t*)&checksum, 2);
				myPacket2->writeData(34+18, (uint8_t*)&urg,2);
				uint8_t header_buffer[20];
				myPacket2->readData(34, header_buffer, 20);
				checksum = NetworkUtil::tcp_sum(((uint32_t*)((*iter)->srcAddr))[0],((uint32_t*)((*iter)->destAddr))[0],header_buffer,20);
				checksum = ~checksum;
				checksum = htons(checksum);
				myPacket2->writeData(34+16, (uint8_t*)&checksum, 2);//checksum
				/* send packet*/
				this->sendPacket("IPv4", myPacket2);
				return;
			}
			/* Find Listening Socket */
	    for ( iter = fdList.begin() ; iter != fdList.end(); ++iter)
			{
				if((*iter)->state == LISTENING)
				{
					if((((uint32_t*)((*iter)->srcAddr))[0] == ((uint32_t*)srcAddr)[0] ||
					  ((uint32_t*)((*iter)->srcAddr))[0] == 0) && // Get packet from Any Client
				    (((uint16_t*)((*iter)->srcPortNum))[0] == ((uint16_t*)srcPortNum)[0]))
							{
								break;
							}
				}
				else continue;
			}
			if(iter == fdList.end())
			{
				this->freePacket(packet);
				return;
			}
      /* send SYN-ACK Packet */
			if((*iter)->listenList.size() < (*iter)->backLog)
			{
				Packet * myPacket = clonePacket(packet);
				this->freePacket(packet);
				(*iter)->listenList.push_back(myPacket);//just store
				/* make syn-ack packet */
				Packet* myPacket2 = allocatePacket(54); // TCP 20B + IP 20B + Etype 14B
				uint8_t flag[1] ; // syn-ack bit
				flag[0] = 18;
				uint8_t headerlen = 5<<4;// (5*32)/8 = 20
				uint16_t rcvw = (*iter)->maxRecvBufSize; // window size
				rcvw = htons(rcvw);
				uint16_t checksum = 0;
				uint32_t seq_start = rand();// make random number
				uint32_t seq_next = htonl(ntohl(seq_start) + 1);
				(*iter)->seqList.push_back(seq_next);
				uint32_t ack_next = htonl(ntohl(((uint32_t*)ack)[0])+1);
				(*iter)->ackList.push_back(ack_next);
				uint16_t urg = 0;
				if(((uint32_t*)((*iter)->srcAddr))[0] == 0) myPacket2->writeData(14+12,srcAddr,4); //already stored in Big-endian
				else myPacket2->writeData(14+12,((uint32_t*)((*iter)->srcAddr)),4);
				myPacket2->writeData(14+16,destAddr,4); // already stored in Big-endian
				myPacket2->writeData(34+0,srcPortNum,2);// already stored in Big-endian
				myPacket2->writeData(34+2,destPortNum,2);// already stored in Big-endian
				myPacket2->writeData(34+4,(uint8_t*)&seq_start,4);//sequence num
				myPacket2->writeData(34+8,(uint8_t*)&ack_next,4);// ack num
				myPacket2->writeData(34+12,&headerlen,1);// header len
				myPacket2->writeData(34+13,flag,1);//syn-ack
				myPacket2->writeData(34+14,(uint8_t*)&rcvw, 2);// rcvw size
				myPacket2->writeData(34+16, (uint8_t*)&checksum, 2);
				myPacket2->writeData(34+18, (uint8_t*)&urg,2);
				uint8_t header_buffer[20];
				myPacket2->readData(34, header_buffer, 20);
				if(((uint32_t*)((*iter)->srcAddr))[0] == 0)
				checksum = NetworkUtil::tcp_sum(((uint32_t*)srcAddr)[0],((uint32_t*)destAddr)[0],header_buffer,20);
				else
				checksum = NetworkUtil::tcp_sum(((uint32_t*)((*iter)->srcAddr))[0],((uint32_t*)destAddr)[0],header_buffer,20);
				checksum = ~checksum;
				checksum = htons(checksum);
				myPacket2->writeData(34+16, (uint8_t*)&checksum, 2);//checksum
				/* send packet*/
				this->sendPacket("IPv4", myPacket2);
				return;
			}
			else // pending connection is full
			{
				this->freePacket(packet);
				return;
			}
		}
/*---------------------------------------------------------------------------------------------------------------------------*/
/* FIN PACKET received */
		else if(signBit[0] == 1)
		{
			std::vector<struct cell*>::iterator iter;
			/* Find CONNECTING, FIN_WAITING_1, FIN_WAITING_2 Socket */
	    for ( iter = fdList.begin() ; iter != fdList.end(); ++iter)
			{
				int state = (*iter)->state;
				if((state == CONNECTING)||(state == FIN_WAITING_1)||(state == FIN_WAITING_2)||(state == TIME_WAITING))
				{
					if((((uint32_t*)((*iter)->srcAddr))[0] == ((uint32_t*)(srcAddr))[0]) &&
					((uint16_t*)((*iter)->srcPortNum))[0] == ((uint16_t*)(srcPortNum))[0] &&
					(((uint32_t*)((*iter)->destAddr))[0] == ((uint32_t*)(destAddr))[0])&&
					((uint16_t*)((*iter)->destPortNum))[0] == ((uint16_t*)(destPortNum))[0])
					break;
				}
				else continue;
			}
			if(iter == fdList.end()) return;
			/* make ACK packet */
			(*iter)->presentAck = htonl(ntohl(((uint32_t*)ack)[0])+1);
			Packet* myPacket = allocatePacket(54);
			uint8_t flag = 1<<4; //ack bit, 10000
			uint8_t headerlen = 5<<4;
			uint16_t temp, rcvw = (*iter)->maxRecvBufSize;
			for(std::vector<Packet*>::iterator it = (*iter)->recvBuf.begin();it!=(*iter)->recvBuf.end();it++)
			{
				(*it)->readData(14+2,(uint8_t*)&temp,2);
				rcvw -= (ntohs(temp)-40);
			}
			if((*iter)->readOffset!=0) rcvw+=(*iter)->readOffset;
			rcvw = htons(rcvw);
			uint16_t checksum = 0;
			myPacket->writeData(14+12,srcAddr,4);
			myPacket->writeData(14+16,destAddr,4);
			myPacket->writeData(34+0,srcPortNum,2);
			myPacket->writeData(34+2,destPortNum,2);
			myPacket->writeData(34+4,(uint8_t*)&(*iter)->presentSeq,4);
			myPacket->writeData(34+8,(uint8_t*)&(*iter)->presentAck,4);
			myPacket->writeData(34+12, &headerlen, 1);
			myPacket->writeData(34+13,&flag,1);
			myPacket->writeData(34+14,(uint8_t*)&rcvw,2);
			myPacket->writeData(34+16, (uint8_t*)&checksum, 2);
			uint8_t header_buffer[20];
			myPacket->readData(34, header_buffer, 20);
			checksum = NetworkUtil::tcp_sum(((uint32_t*)srcAddr)[0],((uint32_t*)destAddr)[0],header_buffer,20);
			checksum = ~checksum;
			checksum = htons(checksum);
			myPacket->writeData(34+16, (uint8_t*)&checksum, 2);
			this->sendPacket("IPv4", myPacket);
			this->freePacket(packet);//free
			/* CONNECTING to CLOSE_WAITING */
			if((*iter)->state == CONNECTING)
			{
				(*iter)->state = CLOSE_WAITING;
				if((*iter)->pending == READING)
				{
					returnSystemCall((*iter)->waitUUID, (*iter)->curRead);
				}
				while((*iter)->recvBuf.size()!=0)
				{
					freePacket(*((*iter)->recvBuf.begin()));
					(*iter)->recvBuf.erase((*iter)->recvBuf.begin());
				}
			}
			/* FIN_WAITING_2 to TIME_WAITING */
			else if((*iter)->state == FIN_WAITING_2)
			{
				(*iter)->state = TIME_WAITING;
				(*iter)->timerUUID = addTimer((*iter), 2*(*iter)->RTO);
			}
			/* FIN_WAITING_1 to INCLOSING */
			else if((*iter)->state == FIN_WAITING_1)
			{
				(*iter)->state = INCLOSING;
			}
			else if((*iter)->state == TIME_WAITING)
			{
				cancelTimer((*iter)->timerUUID);
				(*iter)->cancel++;
				(*iter)->timerUUID = addTimer((*iter),2*(*iter)->RTO);
			}
			else assert(0);
			return;
		}
/*------------------------------------------------------------------------------------------------------------------------------*/
/* ACK PACKET received */
	else if(signBit[0] == 16)
	{
		/* Find CONNECTING, FIN_WAITING_1, LASTACK_WAITNG, INCLOSING Socket */
		std::vector<struct cell*>::iterator iter;
		for ( iter = fdList.begin() ; iter != fdList.end(); ++iter)
		{
			int state = (*iter)->state;
			if((state == CONNECTING)||(state == FIN_WAITING_1)||(state == LASTACK_WAITNG)||(state == INCLOSING)||(state == SYN_RCV))
			{
				if((((uint32_t*)((*iter)->srcAddr))[0] == ((uint32_t*)(srcAddr))[0] ) &&
				((uint16_t*)((*iter)->srcPortNum))[0] == ((uint16_t*)(srcPortNum))[0] &&
				(((uint32_t*)((*iter)->destAddr))[0] == ((uint32_t*)(destAddr))[0])&&
				((uint16_t*)((*iter)->destPortNum))[0] == ((uint16_t*)(destPortNum))[0])
				break;
			}
			else continue;
		}
		if(iter == fdList.end())
		{
			/* Find READYCONNECT, LISTENING Socket */
			for ( iter = fdList.begin() ; iter != fdList.end(); ++iter)
			{
				int state = (*iter)->state;
				if(state == LISTENING)
				{
					if((((uint32_t*)((*iter)->srcAddr))[0] == ((uint32_t*)(srcAddr))[0] ||((uint32_t*)((*iter)->srcAddr))[0] == 0) &&
					((uint16_t*)((*iter)->srcPortNum))[0] == ((uint16_t*)(srcPortNum))[0])
					{
 						break;
					}
				}
				else continue;
			}
		}
		if(iter == fdList.end())
		{
			this->freePacket(packet);
			return;
		}
/* Normal(No Transition) ----------------------------------------------------------------------------------------------------------------*/
		if((*iter)->state == CONNECTING)
		{
			/* WRITE part: Check matched ACK packet arrived-------------------------------------------------------------------------------------------------------------------*/
			if((*iter)->internalBuf.size()!=0)
			{
				/* already processed(don't care) */
				if(ntohl(((uint32_t*)seq)[0])-1 < (*iter)->LastByteRcvd)
				{
					freePacket(packet);
					return;
				}
				/* Find matched packet */
				int count = 0;
				uint32_t checkAck, firstSeq;
				std::vector<struct S_Packet*>::iterator internal;
				for(internal=(*iter)->internalBuf.begin();internal!=(*iter)->internalBuf.end();internal++)
				{
					(*internal)->packet->readData(34+4,(uint8_t*)&checkAck,4);
					if(internal == (*iter)->internalBuf.begin()) firstSeq = checkAck;
					if(ntohl(((uint32_t*)seq)[0]) == ntohl(checkAck)+(*internal)->size)
					{
						count++;
						break;
					}
					else count++;
				}
				/* Cannot find & receive duplicate ACK PACKET */
				if(internal == (*iter)->internalBuf.end() && firstSeq == ((uint32_t*)seq)[0])
				{
					count = 0;
					freePacket(packet);

					(*iter)->duplicate++;
					/* calculate RTT for not retransmitted packets */
					if(((*iter)->totalSendDupPacket ==0 && (*iter)->recovery == false)|| ((*iter)->totalSendDupPacket !=0  && (*iter)->recovery == true))
					{
						Time RTT;
						if((*iter)->totalSendDupPacket != 0) RTT = this->getHost()->getSystem()->getCurrentTime() - (*iter)->sendTime[(*iter)->duplicate + 2];
						else RTT = this->getHost()->getSystem()->getCurrentTime() - (*iter)->sendTime[(*iter)->duplicate -1 ];
						(*iter)->SRTT = 0.875*(*iter)->SRTT+0.125*RTT;
						if((*iter)->SRTT>RTT) (*iter)->RTTVAR = 0.75*(*iter)->RTTVAR+0.25*((*iter)->SRTT-RTT);
						else (*iter)->RTTVAR = 0.75*(*iter)->RTTVAR+0.25*(RTT-(*iter)->SRTT);
						(*iter)->RTO = (*iter)->SRTT+4*(*iter)->RTTVAR;
					}
					/* ignore packtes after Fast transmit until receive normal packet */
					if((*iter)->totalSendDupPacket !=0 ) return;
					/* Fast Retransmit */
					if((*iter)->duplicate == 3)
					{
						cancelTimer((*iter)->timerUUID);
						(*iter)->cancel++;
						(*iter)->duplicate = 0;
						(*iter)->timerUUID = addTimer((*iter),(*iter)->RTO);
						for(std::vector<struct S_Packet*>::iterator resend=(*iter)->internalBuf.begin();resend!=(*iter)->internalBuf.end();resend++)
						{
							Packet* packetToStore = clonePacket((*resend)->packet);
							this->sendPacket("IPv4",(*resend)->packet);
							(*resend)->packet = packetToStore;
							(*iter)->totalSendDupPacket++;
							(*iter)->recovery = true;
						}
					}
					return;
				}
				/* Find matched packet */
				else if(internal != (*iter)->internalBuf.end())
				{
					int diff = (size_t)count - (*iter)->internalBuf.size();
					cancelTimer((*iter)->timerUUID);
					(*iter)->cancel++;
					freePacket(packet);
					(*iter)->LastByteRcvd = ntohl(checkAck)+(*internal)->size-1;
					(*iter)->duplicate = 0;
					if((*iter)->totalSendDupPacket!=0)
					{
						(*iter)->totalSendDupPacket = 0;
						if(diff == 0)
						{
							int Q = (*iter)->cwnd/MSS;
							Q = Q/2;
							if(Q<1) (*iter)->cwnd = MSS;
							else (*iter)->cwnd = Q*MSS;
							(*iter)->cwnd -= count*MSS;
							(*iter)->recovery = false;
						}
						else return;
					}
					/* Caculate RTT */
					else
					{
						Time RTT = this->getHost()->getSystem()->getCurrentTime() - (*iter)->sendTime[count-1];
						(*iter)->SRTT = 0.875*(*iter)->SRTT+0.125*RTT;
						if((*iter)->SRTT>RTT) (*iter)->RTTVAR = 0.75*(*iter)->RTTVAR+0.25*((*iter)->SRTT-RTT);
						else (*iter)->RTTVAR = 0.75*(*iter)->RTTVAR+0.25*(RTT-(*iter)->SRTT);
						(*iter)->RTO = (*iter)->SRTT+4*(*iter)->RTTVAR;
					}
					/* Cleaning */
					while(count!=0)
					{
						/* clean sendTime */
						(*iter)->sendTime.erase((*iter)->sendTime.begin());
						/* clean internal buffer */
						freePacket((*(*iter)->internalBuf.begin())->packet);
						free(*((*iter)->internalBuf.begin()));
						(*iter)->internalBuf.erase((*iter)->internalBuf.begin());
						count--;
						/* update congestion window size */
						if((*iter)->cwnd+MSS<=(*iter)->maxIntBufSize &&(*iter)->totalSendDupPacket == 0)
						{
							(*iter)->cwnd+=MSS;
						}
					}
					uint16_t inBuf = 0;
					for(std::vector<struct S_Packet*>::iterator internal=(*iter)->internalBuf.begin();internal!=(*iter)->internalBuf.end();internal++)
					{
						inBuf+=(*internal)->size;
					}
					/* Send more data */
					if((*iter)->ptr != NULL && (*iter)->recovery == false)
					{
						int size, remain,totalRemain = (*iter)->totalSend - (*iter)->curSend;
						Time curTime;
						uint16_t swnd = (*iter)->cwnd - inBuf;
						if(peerRwnd<swnd) swnd = peerRwnd;
						char*payload = (char*)(*iter)->ptr;
						if(totalRemain<swnd) remain = totalRemain;
						else remain = swnd;
						while(remain!=0)
						{
							if(remain < MSS) size = remain;
							else size = MSS;
							/* make ACK PACKET */
							uint8_t flag = 1<<4; //ack bit
							uint8_t headerlen = 5<<4;
							uint16_t r_temp, rcvw = (*iter)->maxRecvBufSize;
							for(std::vector<Packet*>::iterator it = (*iter)->recvBuf.begin();it!=(*iter)->recvBuf.end();it++)
							{
								(*it)->readData(14+2,(uint8_t*)&r_temp,2);
								rcvw -= (ntohs(r_temp)-40);
							}
							if((*iter)->readOffset!=0) rcvw+=(*iter)->readOffset;
							rcvw = htons(rcvw);
							Packet *packetToSend = allocatePacket(54+size);
							packetToSend->writeData(14+12,(*iter)->srcAddr,4);
							packetToSend->writeData(14+16,(*iter)->destAddr,4);
							packetToSend->writeData(34+0,(*iter)->srcPortNum,2);
							packetToSend->writeData(34+2,(*iter)->destPortNum,2);
							packetToSend->writeData(34+4,(uint8_t*)&(*iter)->presentSeq,4);
							packetToSend->writeData(34+8,(uint8_t*)&(*iter)->presentAck,4);
							packetToSend->writeData(34+12,&headerlen,1);
							packetToSend->writeData(34+13,&flag,1);
							packetToSend->writeData(34+14,&rcvw,2);
							packetToSend->writeData(34+20,payload,size);
							uint16_t checksum = 0;
							uint8_t buffer[20+size];
							packetToSend->readData(34+0,buffer,20+size);
							checksum = NetworkUtil::tcp_sum(((uint32_t*)((*iter)->srcAddr))[0],((uint32_t*)((*iter)->destAddr))[0],buffer,20+size);
							checksum = ~checksum;
							checksum = htons(checksum);
							packetToSend->writeData(34+16,(uint8_t*)&checksum,2);
							this->sendPacket("IPv4",packetToSend);
							curTime = this->getHost()->getSystem()->getCurrentTime();
							/* update information */
							curTime = this->getHost()->getSystem()->getCurrentTime();
							(*iter)->sendTime.push_back(curTime);
							Packet *packetToStore = clonePacket(packetToSend);
							struct S_Packet* Store = new struct S_Packet;
							Store->size = size;
							Store->packet = packetToStore;
							(*iter)->internalBuf.push_back(Store);
							(*iter)->presentSeq = htonl(ntohl((*iter)->presentSeq)+size);
							(*iter)->curSend+=size;
							payload += size;
							if(remain < MSS) remain = 0;
							else remain -= MSS;
						}
						if((*iter)->curSend == (*iter)->totalSend)
						{
							(*iter)->ptr = NULL;
							returnSystemCall((*iter)->waitUUID,(*iter)->totalSend);
						}
						else
						{
							/* restart the timer*/
							(*iter)->timerUUID = addTimer((*iter),(*iter)->RTO - curTime + (*iter)->sendTime[0]);
							(*iter)->ptr = payload;
						}
						return;
					}
					/* no more data to send */
					else
					{
						(*iter)->timerUUID = addTimer((*iter),(*iter)->RTO - this->getHost()->getSystem()->getCurrentTime() + (*iter)->sendTime[0]);
						return;
					}
				}
				else assert(0);
			}
			/* READ part:-------------------------------------------------------------------------------------------------------------------------------------*/
			/* make ACK PACKET */
			uint16_t dataLen, new_rcvw ,pre_rcvw = (*iter)->maxRecvBufSize;
			uint32_t cur, next;
			Packet *packetToStore = clonePacket(packet);
			freePacket(packet);
			for(std::vector<Packet*>::iterator rcvp=(*iter)->recvBuf.begin();rcvp!=(*iter)->recvBuf.end();rcvp++)
			{
				(*rcvp)->readData(14+2,(uint8_t*)&dataLen,2);
				pre_rcvw -= (ntohs(dataLen)-40);
			}
			if((*iter)->readOffset!=0) pre_rcvw+=(*iter)->readOffset;
			/* store PACKET */
			if(ntohl(((uint32_t*)ack)[0]) >= ntohl((*iter)->presentAck) && pre_rcvw >= packetSize)
			{
				std::vector<Packet*>::iterator rcvp;
				for(rcvp=(*iter)->recvBuf.begin();rcvp!=(*iter)->recvBuf.end();rcvp++)
				{
					(*rcvp)->readData(34+4,(uint8_t*)&cur,4);
					cur = ntohl(cur);
					if( cur >= ntohl(((uint32_t*)ack)[0])) break;
					else continue;
				}
				if(rcvp == (*iter)->recvBuf.end())
				{
					(*iter)->recvBuf.push_back(packetToStore);
					new_rcvw = pre_rcvw - packetSize;
				}
				else
				{
					if(cur == ntohl(((uint32_t*)ack)[0]))
					{
						freePacket(packetToStore);
						new_rcvw = pre_rcvw;
					}
					else
					{
						(*iter)->recvBuf.insert(rcvp,packetToStore);
						new_rcvw = pre_rcvw - packetSize;
					}
				}
			}
			else if(ntohl(((uint32_t*)ack)[0]) < ntohl((*iter)->presentAck) )
			{
				freePacket(packetToStore);
				new_rcvw = pre_rcvw;
			}
			else if(pre_rcvw<packetSize)
			{
				freePacket(packetToStore);
				new_rcvw = pre_rcvw;
			}
			else assert(0);
			/* Detemine presentAck */
			std::vector<Packet*>::iterator rbi;
			for( rbi = (*iter)->recvBuf.begin();rbi!=(*iter)->recvBuf.end();rbi++)
			{
				(*rbi)->readData(34+4,(uint8_t*)&cur,4);
				if((cur = ntohl(cur)) == ntohl((*iter)->presentAck)) break;
			}
			while(rbi!=(*iter)->recvBuf.end())
			{
				(*rbi)->readData(14+2,(uint8_t*)&dataLen,2);
			 	dataLen = ntohs(dataLen)-40;
				(*iter)->presentAck = htonl(ntohl((*iter)->presentAck) + dataLen);
				rbi++;
				if(rbi == (*iter)->recvBuf.end()) break;
				(*rbi)->readData(34+4,(uint8_t*)&next,4);
				if((next = ntohl(next)) == (cur = ntohl((*iter)->presentAck))) continue;
				else if(next > cur) break;
				else assert(0);
			}
			Packet *packetToSend = allocatePacket(54);
			uint8_t headerlen = 5<<4;
			uint8_t flag = 1<<4;
			new_rcvw = htons(new_rcvw);
			packetToSend->writeData(14+12,(*iter)->srcAddr,4);
			packetToSend->writeData(14+16,(*iter)->destAddr,4);
			packetToSend->writeData(34+0,(*iter)->srcPortNum,2);
			packetToSend->writeData(34+2,(*iter)->destPortNum,2);
			packetToSend->writeData(34+4,(uint8_t*)&(*iter)->presentSeq,4);
			packetToSend->writeData(34+8,(uint8_t*)&(*iter)->presentAck,4);
			packetToSend->writeData(34+12,&headerlen,1);
			packetToSend->writeData(34+13,&flag,1);
			/* send ACK PACKET */
			if((*iter)->pending != READING)
			{
				packetToSend->writeData(34+14,&new_rcvw,2);
				uint16_t checksum = 0;
				uint8_t header_buffer[20];
				packetToSend->readData(34+0,header_buffer,20);
				checksum = NetworkUtil::tcp_sum(((uint32_t*)((*iter)->srcAddr))[0],((uint32_t*)((*iter)->destAddr))[0],header_buffer,20);
				checksum = ~checksum;
				checksum = htons(checksum);
				packetToSend->writeData(34+16,(uint8_t*)&checksum,2);
				sendPacket("IPv4",packetToSend);
				return;
			}
			/* check READ is pended or not */
			else if((*iter)->pending == READING)
			{
				char* payload = (char*)(*iter)->ptr;
				uint16_t len;
				int pos = -1;
				cur = ntohl((*iter)->startAck)+(*iter)->curRead;
				for(std::vector<Packet*>::iterator rcvp = (*iter)->recvBuf.begin();rcvp!=(*iter)->recvBuf.end();rcvp++)
				{
					if((*iter)->curRead == (*iter)->totalRead) break;
					(*rcvp)->readData(34+4,(uint8_t*)&next,4);
					next = ntohl(next);
					if(cur!=next) break;
					(*rcvp)->readData(14+2,(uint8_t*)&len,2);
					len = ntohs(len)-40;
					/* continue reading from offset */
					if((*iter)->readOffset!=0)
					{
						if(len-(*iter)->readOffset > (*iter)->totalRead - (*iter)->curRead)
						{
							len = (*iter)->totalRead-(*iter)->curRead;
							(*rcvp)->readData(54+(*iter)->readOffset,payload,len);
							(*iter)->readOffset+=len;
							(*iter)->curRead+=len;
							payload+=len;
						}
						else
						{
							(*rcvp)->readData(54+(*iter)->readOffset,payload,len-(*iter)->readOffset);
							(*iter)->startAck = htonl(ntohl((*iter)->startAck)+(*iter)->readOffset);
							cur+=len;
							payload+=len-(*iter)->readOffset;
							(*iter)->curRead+=len-(*iter)->readOffset;
							(*iter)->readOffset = 0;
							pos++;
						}
					}
					/* not enough user buffer */
					else if((*iter)->totalRead-(*iter)->curRead<len)
					{
						len = (*iter)->totalRead-(*iter)->curRead;
						(*rcvp)->readData(54,payload,len);
						(*iter)->curRead+=len;
						(*iter)->readOffset = len;
						payload+=len;
					}
					/* normal */
					else
					{
						cur+=len;
						(*rcvp)->readData(54,payload,len);
						payload+=len;
						(*iter)->curRead+=len;
						pos++;
					}
				}
				while(pos!=-1)
				{
					freePacket((*iter)->recvBuf[pos]);
					(*iter)->recvBuf.erase((*iter)->recvBuf.begin()+pos);
					pos--;
				}
				uint16_t read_len ,read_rcvw = (*iter)->maxRecvBufSize;
				for(std::vector<Packet*>::iterator rcvp=(*iter)->recvBuf.begin();rcvp!=(*iter)->recvBuf.end();rcvp++)
				{
					(*rcvp)->readData(14+2,(uint8_t*)&read_len,2);
					read_rcvw -= (ntohs(read_len)-40);
				}
				if((*iter)->readOffset!=0) read_rcvw+=(*iter)->readOffset;
				read_rcvw = htons(read_rcvw);
				packetToSend->writeData(34+14,(uint8_t*)&read_rcvw,2);
				uint16_t checksum = 0;
				uint8_t header_buffer[20];
				packetToSend->readData(34+0,header_buffer,20);
				checksum = NetworkUtil::tcp_sum(((uint32_t*)((*iter)->srcAddr))[0],((uint32_t*)((*iter)->destAddr))[0],header_buffer,20);
				checksum = ~checksum;
				checksum = htons(checksum);
				packetToSend->writeData(34+16,(uint8_t*)&checksum,2);
				sendPacket("IPv4", packetToSend);
				if((*iter)->curRead == (*iter)->totalRead)
				{
					if((*iter)->readOffset==0)
					{
						(*iter)->pending = NONE;
					}
					(*iter)->startAck = htonl(cur);
					(*iter)->ptr = NULL;
					returnSystemCall((*iter)->waitUUID,(*iter)->totalRead);
					return;
				}
				else if((*iter)->curRead < (*iter)->totalRead)
				{
					(*iter)->ptr = payload;
					return;
				}
				else assert(0);
			}
			else assert(0);
		}
/* Transition to CONNECTING 1 : NO LISTENING SOCKET(SYN_SENT->SYN_RCV case) ------------------------------------------------------------------------*/
		else if((*iter)->state == SYN_RCV)
		{
			this->freePacket(packet);
			(*iter)->state = CONNECTING;
			struct sockaddr * param2_ptr = (struct sockaddr *)((*iter)->ptr);
			(*iter)->ptr = NULL;
			(*iter)->LastByteRcvd = ntohl((*iter)->presentSeq)-1;
			((uint16_t*)param2_ptr->sa_data)[0] = ((uint16_t*)((*iter)->destPortNum))[0];
			((uint32_t*)(param2_ptr->sa_data + 2))[0] = ((uint32_t*)((*iter)->destAddr))[0];
			param2_ptr->sa_family = (*iter)->family;
			returnSystemCall((*iter)->waitUUID, (*iter)->sockfd);// return Connect
			return;
		}
/* Transition to CONNECTING 2 : WITH LISENING SOCKET ------------------------------------------------------------------------------------------------*/
		else if((*iter)->state == LISTENING)
		{
			/* Find Packet in listenList*/
			int found = -1;
			int pos = 0;
			for(std::vector<Packet *>::iterator liter = (*iter)->listenList.begin() ; liter != (*iter)->listenList.end(); liter++)
			{
				uint32_t srcIP, destIP;
				uint16_t srcPN, destPN;
				Packet* waitingPack = (*liter);
				waitingPack->readData(14+12, &destIP, 4);
				waitingPack->readData(14+16, &srcIP, 4);
				waitingPack->readData(34+0, &destPN, 2);
				waitingPack->readData(34+2, &srcPN, 2);
				if((((uint32_t*)(srcAddr))[0] == srcIP)&& ((uint32_t*)(destAddr))[0] == destIP &&
				((uint16_t*)(srcPortNum))[0] == srcPN && ((uint16_t*)(destPortNum))[0] == destPN)
				{
					found = 0;
					this->freePacket(*liter);
					(*iter)->listenList.erase(liter);
					break;
				}
				pos++;
			}
			if(found == -1) assert(0);
			Packet *myPacket = clonePacket(packet);
			this->freePacket(packet);
			(*iter)->waitAcceptList.push_back(myPacket);
			(*iter)->waitAcceptSeqList.push_back((*iter)->seqList[pos]);
			(*iter)->waitAcceptAckList.push_back((*iter)->ackList[pos]);
			(*iter)->seqList.erase((*iter)->seqList.begin()+pos);
			(*iter)->ackList.erase((*iter)->ackList.begin()+pos);
			/* Find READYCONNECT Socket(Normal READYCONNECT Sockets are in behind of the coressponding LISENING SOCKET)*/
			uint32_t src_IP, dest_IP;
			uint16_t src_PN, dest_PN;
			Packet *pck =(*iter)->waitAcceptList[0];
			pck->readData(14+12, &dest_IP, 4);
			pck->readData(14+16, &src_IP, 4);
			pck->readData(34+0, &dest_PN, 2);
			pck->readData(34+2, &src_PN, 2);
			std::vector<struct cell *>::iterator it;
			for (it = fdList.begin() ; it != fdList.end(); ++it)
			{
				int state = (*it)->state;
				if( state == READYCONNECT )
				{
					if((((uint32_t*)((*it)->srcAddr))[0] == ((uint32_t*)((*iter)->srcAddr))[0]) &&
					((uint16_t*)((*it)->srcPortNum))[0] ==  ((uint16_t*)((*iter)->srcPortNum))[0])
					break;
				}
				else continue;
			}
			if(it == fdList.end()) return;
			/* socket is found */
			else
			{
				this->freePacket((*iter)->waitAcceptList[0]);
				(*iter)->waitAcceptList.erase((*iter)->waitAcceptList.begin());
				(*it)->state = CONNECTING;
				(*it)->presentSeq = (*iter)->waitAcceptSeqList[0];
				(*it)->presentAck = (*iter)->waitAcceptAckList[0];
				(*iter)->waitAcceptSeqList.erase((*iter)->waitAcceptSeqList.begin());
				(*iter)->waitAcceptAckList.erase((*iter)->waitAcceptAckList.begin());
				((uint32_t*)((*it)->srcAddr))[0] = src_IP;
				((uint32_t*)((*it)->destAddr))[0] = dest_IP;
				((uint16_t*)((*it)->destPortNum))[0] = dest_PN;
				((uint16_t*)((*it)->srcPortNum))[0] = src_PN;
				(*it)->isbind = 0;
				struct sockaddr * param2_ptr = (struct sockaddr *)((*it)->ptr);
				(*it)->ptr=NULL;
				(*it)->LastByteRcvd = ntohl((*it)->presentSeq)-1;
				((uint16_t*)param2_ptr->sa_data)[0] = ((uint16_t*)((*it)->destPortNum))[0];
				((uint32_t*)(param2_ptr->sa_data + 2))[0] = ((uint32_t*)((*it)->destAddr))[0];
				param2_ptr->sa_family = (*it)->family;
				returnSystemCall((*it)->waitUUID, (*it)->sockfd);// return Accept
				return;
			}
		}
/* Transition to FIN_WAITING_2 -----------------------------------------------------------------------------------------------------------*/
		else if((*iter)->state == FIN_WAITING_1)
		{
			if((*iter)->presentAck == ((uint32_t*)seq)[0])
			{
				assert((*iter)->internalBuf.size()==0);
				(*iter)->state = FIN_WAITING_2;
				this->freePacket(packet);
				return;
			}
			else if((*iter)->internalBuf.size()!=0)
			{
				Time curTime = this->getHost()->getSystem()->getCurrentTime();
				this->freePacket(packet);
				int count = 0;
				uint32_t checkAck;
				for(std::vector<S_Packet*>::iterator internal=(*iter)->internalBuf.begin();internal!=(*iter)->internalBuf.end();internal++)
				{
					(*internal)->packet->readData(34+4,(uint8_t*)&checkAck,4);
					if(ntohl(((uint32_t*)seq)[0]) == ntohl(checkAck)+(*internal)->size)
					{
						count++;
						break;
					}
					else count++;
				}
				if(count==0) return;
				cancelTimer((*iter)->timerUUID);
				(*iter)->cancel++;
				Time RTT = curTime - (*iter)->sendTime[count-1];
				(*iter)->SRTT = 0.875*(*iter)->SRTT + 0.125*RTT;
				if((*iter)->SRTT>RTT) (*iter)->RTTVAR = 0.75*(*iter)->RTTVAR + 0.25*((*iter)->SRTT-RTT);
				else (*iter)->RTTVAR = 0.75*(*iter)->RTTVAR + 0.25*(RTT-(*iter)->SRTT);
				(*iter)->RTO = (*iter)->SRTT+4*(*iter)->RTTVAR;
				while(count!=0)
				{
					freePacket((*(*iter)->internalBuf.begin())->packet);
					free(*((*iter)->internalBuf.begin()));
					(*iter)->internalBuf.erase((*iter)->internalBuf.begin());
					count--;
					(*iter)->cwnd+=MSS;
				}
				if((*iter)->internalBuf.size()!=0)
				{
					(*iter)->timerUUID = addTimer((*iter),(*iter)->RTO -curTime + (*iter)->sendTime[0]);
				}
				return;
			}
			else assert(0);
		}
/* CLOSE SOCKET -------------------------------------------------------------------------------------------------------------------------------*/
		else if((*iter)->state == LASTACK_WAITNG)
		{
			this->freePacket(packet);
			returnSystemCall((*iter)->waitUUID,0 );
			delete *iter;
			fdList.erase(iter);
			return;
		}
/* Transition to TIME_WAITING-------------------------------------------------------------------------------------------------------------------*/
		else if((*iter)->state == INCLOSING && (*iter)->presentSeq == ((uint32_t*)seq)[0])
		{
			this->freePacket(packet);
			(*iter)->state = TIME_WAITING;
			(*iter)->timerUUID = addTimer((*iter),2*(*iter)->RTO);
			return;
		}
/*error--------------------------------------------------------------------------------------------------------------------------------------------*/
		else assert(0);
	}
/*----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/* FIN-ACK PACKET received */
		else if(signBit[0] == 17)
		{
			this->freePacket(packet);//free
			std::vector<struct cell*>::iterator iter;
	    for ( iter = fdList.begin() ; iter != fdList.end(); ++iter)
			{
				int state = (*iter)->state;
				if(state == FIN_WAITING_1) // find timerUUID and syscallUUID for close
				{
					if( ((*iter)->srcAddr[0] == srcAddr[0])&&((*iter)->srcAddr[1] == srcAddr[1])&&((*iter)->srcAddr[2] == srcAddr[3])&&((*iter)->srcAddr[3] ==srcAddr[3])&&
							((*iter)->srcPortNum[0] == srcPortNum[0])&&((*iter)->srcPortNum[1] == srcPortNum[1])&&
							((*iter)->destAddr[0] == destAddr[0])&&((*iter)->destAddr[1] == destAddr[1])&&((*iter)->destAddr[2] == destAddr[3])&&((*iter)->destAddr[3] ==destAddr[3])&&
							((*iter)->destPortNum[0] == destPortNum[0])&&((*iter)->destPortNum[1] == destPortNum[1]) )
							break;
				}
				else continue;
			}
			if(iter == fdList.end()) return;
			/* make ACK packet */
			(*iter)->presentAck = htonl(ntohl(((uint32_t*)ack)[0]+1));
			Packet* myPacket = allocatePacket(54);
			uint8_t flag = 1<<4; //ack bit, 10000
			uint8_t headerlen = 5<<4;
			uint16_t temp, rcvw = (*iter)->maxRecvBufSize;
			for(std::vector<Packet*>::iterator it = (*iter)->recvBuf.begin();it!=(*iter)->recvBuf.end();it++)
			{
				(*it)->readData(14+2,(uint8_t*)&temp,2);
				rcvw -= (ntohs(temp)-40);
			}
			rcvw = htons(rcvw);
			uint16_t checksum = 0;
			myPacket->writeData(14+12,srcAddr,4);
			myPacket->writeData(14+16,destAddr,4);
			myPacket->writeData(34+0,srcPortNum,2);
			myPacket->writeData(34+2,destPortNum,2);
			myPacket->writeData(34+4,(uint8_t*)&(*iter)->presentSeq,4);
			myPacket->writeData(34+8,(uint8_t*)&(*iter)->presentAck,4);
			myPacket->writeData(34+12, &headerlen, 1);
			myPacket->writeData(34+13,&flag,1);
			myPacket->writeData(34+14,(uint8_t*)&rcvw,2);
			myPacket->writeData(34+16, (uint8_t*)&checksum, 2);
			uint8_t header_buffer[20];
			myPacket->readData(34, header_buffer, 20);
			checksum = NetworkUtil::tcp_sum(((uint32_t*)srcAddr)[0],((uint32_t*)destAddr)[0],header_buffer,20);
			checksum = ~checksum;
			checksum = htons(checksum);
			myPacket->writeData(34+16, (uint8_t*)&checksum, 2);
			(*iter)->presentSeq = htonl(ntohl((*iter)->presentSeq) + 1);
			/* send packet*/
			this->sendPacket("IPv4", myPacket);

			(*iter)->state = TIME_WAITING;
			(*iter)->timerUUID = addTimer((*iter),2*(*iter)->RTO);
			return;
		}
	}
	else assert(0);
}
/*-------------------------------------------------------------------------------------------------------------------------------*/

void TCPAssignment::timerCallback(void* payload)
{
	if(payload == NULL) assert(0);
	struct cell * ptr = (struct cell *) payload;
	if(ptr->cancel!=0)
	{
		ptr->cancel--;
		return;
	}
	/* Close Connection*/
	std::vector<struct cell*>::iterator iter;
	for(iter = fdList.begin(); iter != fdList.end(); iter++)
	{
		if((*iter) == ptr) break;
	}
	if(iter == fdList.end()) assert(0);
	if((*iter)->state == TIME_WAITING)
	{
		fdList.erase(iter);
		returnSystemCall(ptr->waitUUID,0);
		delete ptr;
		return;
	}
	else if((*iter)->state == CONNECTING)
	{
		(*iter)->cwnd = MSS;
		//(*iter)->expand = 0;
		(*iter)->RTO = 2*(*iter)->RTO;
		(*iter)->timerUUID = addTimer((*iter),(*iter)->RTO);
		for(std::vector<struct S_Packet*>::iterator resend=(*iter)->internalBuf.begin();resend!=(*iter)->internalBuf.end();resend++)
		{
			Packet* packetToStore = clonePacket((*resend)->packet);
			sendPacket("IPv4",(*resend)->packet);
			(*resend)->packet = packetToStore;
		}
		return;
	}
	else assert(0);
}
/*-------------------------------------------------------------------------------------------------------------------------------------*/

void TCPAssignment::syscall_socket(UUID syscallUUID, int pid, int param1_int, int param2_int)
{
	int sockfd = this->createFileDescriptor(pid);
	struct cell *cellptr = new struct cell;//make file descriptor list
	cellptr->pid = pid;
	cellptr->sockfd = sockfd;
	cellptr->isbind = -1; // -1 means not binded
	cellptr->state = NOTREADYCONNECT;
	cellptr->pending = NONE;
	cellptr->maxIntBufSize = 100*MSS;
	cellptr->maxRecvBufSize = 100*MSS;
	cellptr->ptr = NULL;
	cellptr->duplicate = 0;
	cellptr->totalSendDupPacket = 0;
	cellptr->readOffset = 0;
	cellptr->cancel = 0;
	cellptr->recovery = false;
	cellptr->SRTT = 100000000;//100msec
	cellptr->RTTVAR = 50000000;//50msec
	cellptr->RTO = 60000000000;//60sec
	cellptr->cwnd = MSS;
	((uint32_t*)cellptr->srcAddr)[0] = ((uint32_t*)cellptr->destAddr)[0] =((uint16_t*)cellptr->srcPortNum)[0] = ((uint16_t*)cellptr->destPortNum)[0] = -1;
	fdList.push_back(cellptr);
  this->returnSystemCall(syscallUUID, sockfd);
	return;
}
/*----------------------------------------------------------------------------------------------------------------------------------------------*/
void TCPAssignment::syscall_close(UUID syscallUUID, int pid, int param1_int)
{
	std::vector<struct cell*>::iterator iter;
	for (iter = fdList.begin() ; iter != fdList.end(); ++iter)
	{
		if((*iter)->sockfd == param1_int && pid == (*iter)->pid) break;
	}
	if(iter == fdList.end())
	{
		this->returnSystemCall(syscallUUID, -1);
		return;
	}
	this->removeFileDescriptor(pid, param1_int);
	(*iter)->sockfd = -1; // -1 means this socket does not have file descriptor
	/* Close Pure & SYN_SENT & LISENING Socket */
	if((*iter)->state == NOTREADYCONNECT || (*iter)->state == SYN_SENT || (*iter)->state == LISTENING ||(*iter)->state == READYCONNECT)
	{
		delete *iter;
		fdList.erase(iter);
		this->returnSystemCall(syscallUUID, 0); //0 means success
		return;
	}
	else if((*iter)->state == CONNECTING ||(*iter)->state == SYN_RCV)
	{
		(*iter)->state = FIN_WAITING_1;
		if((*iter)->pending == READING)
		{
			returnSystemCall((*iter)->waitUUID,-1);
		}
	}
	else if((*iter)->state == CLOSE_WAITING)
	{
		(*iter)->state = LASTACK_WAITNG;
		if((*iter)->pending == READING)
		{
			returnSystemCall((*iter)->waitUUID, -1);
		}
	}
	else assert(0);
	/* Store PID and syscall UUID */
	(*iter)->waitUUID = syscallUUID;
	/* Send FIN Packet */
	uint8_t flag = 1; //FIN bit
	uint8_t headerlen = 5<<4;
	uint16_t checksum = 0;
	uint16_t temp, rcvw = (*iter)->maxRecvBufSize; // window size
	for(std::vector<Packet*>::iterator it = (*iter)->recvBuf.begin();it!=(*iter)->recvBuf.end();it++)
	{
		(*it)->readData(14+2,(uint8_t*)&temp,2);
		rcvw -= (ntohs(temp)-40);
	}
	if((*iter)->readOffset!=0) rcvw+=(*iter)->readOffset;
	rcvw = htons(rcvw);
	uint32_t seq = (*iter)->presentSeq;
	Packet* myPacket = allocatePacket(54);
	myPacket->writeData(14+12,(*iter)->srcAddr,4); // already stored in Big-endian
	myPacket->writeData(14+16,(*iter)->destAddr,4); // already stored in Big-endian
	myPacket->writeData(34+0,(*iter)->srcPortNum,2);// already stored in Big-endian
	myPacket->writeData(34+2,(*iter)->destPortNum,2);// already stored in Big-endian
	myPacket->writeData(34+4,(uint8_t*)&seq,4);
	myPacket->writeData(34+12,&headerlen,1);
	myPacket->writeData(34+13,&flag,1);
	myPacket->writeData(34+14,(uint8_t*)&rcvw,2);
	myPacket->writeData(34+16, (uint8_t*)&checksum, 2);
	uint8_t header_buffer[20];
	myPacket->readData(34, header_buffer, 20);
	checksum = NetworkUtil::tcp_sum(((uint32_t*)((*iter)->srcAddr))[0],((uint32_t*)((*iter)->destAddr))[0],header_buffer,20);
	checksum = ~checksum;
	checksum = htons(checksum);
	myPacket->writeData(34+16, (uint8_t*)&checksum, 2);
	/* update presentSeq */
	(*iter)->presentSeq = htonl(ntohl((*iter)->presentSeq) + 1);
	(*iter)->presentAck = (*iter)->presentSeq;
	/* send packet */
	this->sendPacket("IPv4", myPacket);
	return;// blocking(waiting ACK PACKET)
}
/*--------------------------------------------------------------------------------------------------------------------------------------*/
void TCPAssignment::syscall_bind(UUID syscallUUID, int pid, int param1_int, struct sockaddr *param2_ptr,socklen_t param3_int)
{
	/* error check */
	 std::vector<struct cell*>::iterator temp = fdList.end();
	 for (std::vector<struct cell*>::iterator iter = fdList.begin() ; iter != fdList.end(); ++iter)
	 {
		 if((*iter)->sockfd == param1_int && pid == (*iter)->pid)//find socket fd
		 {
			 if((*iter)->isbind == 0)
			 {
				 this->returnSystemCall(syscallUUID, -1); //-1 means failure
			   return;//erorr: already binded
		   }
			 else temp = iter;
		 }
		 if((*iter)->isbind == -1) continue;
		 else if(((*iter)->srcPortNum[0] == param2_ptr->sa_data[0])&&((*iter)->srcPortNum[1] == param2_ptr->sa_data[1]))//find same port num
		 {
			 if((
				 ((*iter)->srcAddr[0] == param2_ptr->sa_data[2])&&
			   ((*iter)->srcAddr[1] == param2_ptr->sa_data[3])&&
		     ((*iter)->srcAddr[2] == param2_ptr->sa_data[4])&&
				 ((*iter)->srcAddr[3] == param2_ptr->sa_data[5]))//error: same address
				 ||
				 ((param2_ptr->sa_data[2]||param2_ptr->sa_data[3]||param2_ptr->sa_data[4]||param2_ptr->sa_data[5]) == 0)//error
			   ||
			   (((*iter)->srcAddr[0]||(*iter)->srcAddr[1]||(*iter)->srcAddr[2]||(*iter)->srcAddr[3]) == 0))
				 {
					 this->returnSystemCall(syscallUUID, -1);
					 return; //error
				 }
			 else continue;
		 }
	 }
	 if(temp == fdList.end())
	 {
		 this->returnSystemCall(syscallUUID, -1);
		 return;//error: cannot find socket fd
	 }
	 /* make return value */
	 (*temp)->srcPortNum[0] = param2_ptr->sa_data[0];
	 (*temp)->srcPortNum[1] = param2_ptr->sa_data[1];
	 (*temp)->srcAddr[0] = param2_ptr->sa_data[2];
	 (*temp)->srcAddr[1] = param2_ptr->sa_data[3];
	 (*temp)->srcAddr[2] = param2_ptr->sa_data[4];
	 (*temp)->srcAddr[3] = param2_ptr->sa_data[5];
	 (*temp)->family = param2_ptr->sa_family;
	 (*temp)->isbind = 0; // 0 means it is now binded
	 this->returnSystemCall(syscallUUID, 0); //0 means success
	 return;
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------*/
void TCPAssignment::syscall_getsockname(UUID syscallUUID, int pid, int param1_int, struct sockaddr *param2_ptr, socklen_t* param3_ptr)
{
	std::vector<struct cell*>::iterator iter;
  for (iter = fdList.begin() ; iter != fdList.end(); ++iter)
	{
		if((*iter)->sockfd == param1_int && pid == (*iter)->pid )
		{
			if((*iter)->isbind == 0) break;
			else continue;
		}
	}
	if(iter == fdList.end())
	{
		returnSystemCall(syscallUUID, -1);
		return; //error: cannot find socket fd
	}
	param2_ptr->sa_family = (*iter)->family;
	param2_ptr->sa_data[0] = (*iter)->srcPortNum[0] ;
	param2_ptr->sa_data[1] = (*iter)->srcPortNum[1];
	param2_ptr->sa_data[2] = (*iter)->srcAddr[0] ;
	param2_ptr->sa_data[3] = (*iter)->srcAddr[1] ;
	param2_ptr->sa_data[4] = (*iter)->srcAddr[2] ;
	param2_ptr->sa_data[5] = (*iter)->srcAddr[3] ;
	this->returnSystemCall(syscallUUID, 0);//0 means success
	return;
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------------*/
void TCPAssignment::syscall_getpeername(UUID syscallUUID, int pid, int param1_int, struct sockaddr *param2_ptr, socklen_t* param3_ptr)
{
	std::vector<struct cell*>::iterator iter;
	for (iter = fdList.begin() ; iter != fdList.end(); ++iter)
	{
		if((*iter)->sockfd == param1_int && pid == (*iter)->pid && (*iter)->state == CONNECTING)
		{
			break;
		}
	}
	if(iter == fdList.end())
	{
		this->returnSystemCall(syscallUUID, -1);
		return;
	}
	param2_ptr->sa_family = (*iter)->family;
	param2_ptr->sa_data[0] = (*iter)->destPortNum[0] ;
	param2_ptr->sa_data[1] = (*iter)->destPortNum[1];
	param2_ptr->sa_data[2] = (*iter)->destAddr[0] ;
	param2_ptr->sa_data[3] = (*iter)->destAddr[1] ;
	param2_ptr->sa_data[4] = (*iter)->destAddr[2] ;
	param2_ptr->sa_data[5] = (*iter)->destAddr[3] ;
	this->returnSystemCall(syscallUUID, 0);
	return;
}
/*-----------------------------------------------------------------------------------------------------------------------------------------------------*/
void TCPAssignment::syscall_connect(UUID syscallUUID, int pid, int param1_int, struct sockaddr* param2_ptr, socklen_t param3_int)
{
	char srcAddr[4], destAddr[4], srcPortNum[2], destPortNum[2];
	int found;
	/* get dest IP and dest Port num*/
	((uint16_t*)destPortNum)[0] = ((uint16_t*)param2_ptr->sa_data)[0];
	((uint32_t*)destAddr)[0] = ((uint32_t*)(param2_ptr->sa_data + 2))[0];
  /* get src IP and src Port num*/
  std::vector<struct cell*>::iterator iter, socket;
	for (iter = fdList.begin() ; iter != fdList.end(); ++iter)
	{
		if((*iter)->sockfd == param1_int && pid == (*iter)->pid ) break;
		else continue;
	}
	socket = iter; //store socket positon
	if(iter == fdList.end())
	{
		returnSystemCall(syscallUUID, -1);
		return; //error: cannot find socket fd
	}
	/* Already Binded */
	else if((*socket)->isbind == 0)
	{
		((uint16_t*)srcPortNum)[0] = ((uint16_t*)(*socket)->srcPortNum)[0];
		if(((uint32_t*)(*socket)->srcAddr)[0] != 0)
			((uint32_t*)srcAddr)[0] = ((uint32_t*)(*socket)->srcAddr)[0];
		else
		{
			this->getHost()->getIPAddr((uint8_t*)srcAddr,this->getHost()->getRoutingTable((uint8_t*)destAddr));
			((uint32_t*)((*socket)->srcAddr))[0] = ((uint32_t*)srcAddr)[0];
		}
	}
	/* Not Yet Binded */
	else
	{
		/* get source IP*/
	  if(this->getHost()->getIPAddr((uint8_t*)srcAddr,this->getHost()->getRoutingTable((uint8_t*)destAddr)) == false)
		{
			returnSystemCall(syscallUUID, -1);
			return;
		}
		/* get source Port num */
		do{
			((uint16_t*)srcPortNum)[0] = rand()%(65535+1);
			found = 0; // check whether srcPortNum is changed
			for(iter=fdList.begin();iter!=fdList.end();iter++)
			{
				if((*iter)->isbind == 0 && ((uint16_t*)((*iter)->srcPortNum))[0] == ((uint16_t*)srcPortNum)[0] &&
				(((uint32_t*)((*iter)->srcAddr))[0] == ((uint32_t*)srcAddr)[0] || ((uint32_t*)((*iter)->srcAddr))[0] == 0))
				{
					found = -1;
					break;
				}
			}
		 }while(found != 0);
		 /* Implicit binding */
		 ((uint32_t*)((*socket)->srcAddr))[0] = ((uint32_t*)srcAddr)[0];
		 ((uint16_t*)((*socket)->srcPortNum))[0] = ((uint16_t*)srcPortNum)[0];
	 }
  /* store connectUUID */
	(*socket)->isbind = 0;
	(*socket)->waitUUID = syscallUUID;
	(*socket)->ptr = param2_ptr;
	/* store destIP and dest Port num*/
	((uint32_t*)((*socket)->destAddr))[0] = ((uint32_t*)destAddr)[0];
	((uint16_t*)((*socket)->destPortNum))[0] = ((uint16_t*)destPortNum)[0];
	(*socket)->state = SYN_SENT;
	/* make SYN packet */
	uint8_t flag = 1<<1; // syn bit
	uint32_t seq = rand();
	uint8_t headerlen = 5<<4;
	uint16_t checksum = 0;
	uint8_t header_buffer[20];
  uint16_t rcvw = (*socket)->maxRecvBufSize;// receive window size = 15200
	rcvw = htons(rcvw);
	Packet* myPacket = allocatePacket(54); // TCP 20B + IP 20B + Etype 14B
	myPacket->writeData(14+12,srcAddr,4); // already stored in Big-endian
	myPacket->writeData(14+16,destAddr,4); // already stored in Big-endian
	myPacket->writeData(34+0,srcPortNum,2);// already stored in Big-endian
	myPacket->writeData(34+2,destPortNum,2);// already stored in Big-endian
	myPacket->writeData(34+4,(uint8_t*)&seq,4);
	myPacket->writeData(34+12,&headerlen,1);
	myPacket->writeData(34+13,&flag,1);
	myPacket->writeData(34+14,(uint8_t*)&rcvw,2);
	myPacket->writeData(34+16, (uint8_t*)&checksum, 2);
	myPacket->readData(34, header_buffer, 20);
	checksum = NetworkUtil::tcp_sum(((uint32_t*)((*socket)->srcAddr))[0],((uint32_t*)((*socket)->destAddr))[0],header_buffer,20);
	checksum = ~checksum;
	checksum = htons(checksum);
	myPacket->writeData(34+16, (uint8_t*)&checksum, 2);
	/* update presentSeq */
	(*socket)->presentSeq = htonl(ntohl(seq) + 1);
	/* send packet*/
	this->sendPacket("IPv4", myPacket);
	return; //blocking
}
/*---------------------------------------------------------------------------------------------------------------------------------------------*/
void TCPAssignment::syscall_listen(UUID syscallUUID, int pid, int param1_int, int param2_int)
{
   int findmatch = -1;
   int i;
   if(param2_int < 0)
   {
      returnSystemCall(syscallUUID,-1);
      return;
   }
	 assert(fdList.size() != 0);
   for (i = 0 ; (unsigned int)i < fdList.size(); i++)
   {
    if (fdList[i]->sockfd == param1_int && fdList[i]->state == NOTREADYCONNECT && fdList[i]->isbind == 0 && pid == fdList[i]->pid)
       {
        findmatch = 0;
        fdList[i]->state = LISTENING; // Change state
        fdList[i]->backLog = (unsigned int)param2_int;
				break;
       }
    }
   if(findmatch != 0)
   {
      returnSystemCall(syscallUUID,-1);
      return;
   }
   returnSystemCall(syscallUUID,0);
   return;
}

/*----------------------------------------------------------------------------------------------------------------------------------------------------------*/
void TCPAssignment::syscall_accept(UUID syscallUUID, int pid, int param1_int, struct sockaddr* param2_ptr, socklen_t* param3_ptr)
{
  std::vector<Packet *>::iterator it;
	struct cell *new_cell = new struct cell;
  int i;
  for(i = 0;(unsigned)i < fdList.size();i++)// find listening socket
  {
    if(fdList[i]->sockfd == param1_int && fdList[i]->state == LISTENING && pid == fdList[i]->pid)
		{
			new_cell->sockfd = createFileDescriptor(pid);// make new fd
			break;
		}
	}
	if( (unsigned)i == fdList.size())
	{
		delete new_cell;
		returnSystemCall(syscallUUID, -1);
		return;
	}
	/* store information of new socket */
	((uint32_t*)(new_cell->srcAddr))[0] = ((uint32_t*)(fdList[i]->srcAddr))[0];
	((uint16_t*)(new_cell->srcPortNum))[0] = ((uint16_t*)(fdList[i]->srcPortNum))[0];
	new_cell->family = fdList[i]->family;
	new_cell->pid = pid;
	new_cell->waitUUID = syscallUUID;
	new_cell->state = READYCONNECT;//Waiting Established Connection
	new_cell->pending = NONE;
	new_cell->maxIntBufSize = 100*MSS;
	new_cell->maxRecvBufSize = 100*MSS;
	new_cell->ptr = NULL;
	new_cell->duplicate = 0;
	new_cell->totalSendDupPacket = 0;
	new_cell->readOffset = 0;
	new_cell->cancel = 0;
	new_cell->recovery = false;
	new_cell->SRTT = 100000000;//100msec
	new_cell->RTTVAR = 50000000;//50msec
	new_cell->RTO = 60000000000;//60sec
	new_cell->cwnd = MSS;
	fdList.push_back(new_cell);
	/* Check Connecting request */
	if(fdList[i]->waitAcceptList.size() == 0)
	{
		new_cell->ptr = param2_ptr;
		return;
	}
	/* Get Data */
	uint32_t packetDestIP[1];
	new_cell->state = CONNECTING;
	fdList[i]->waitAcceptList[0]->readData(14+12, new_cell->destAddr, 4);
	fdList[i]->waitAcceptList[0]->readData(14+16, (uint8_t*)packetDestIP, 4);
	fdList[i]->waitAcceptList[0]->readData(34+0, new_cell->destPortNum, 2);
	if( ((uint32_t*)new_cell->srcAddr)[0] == 0) ((uint32_t*)new_cell->srcAddr)[0] = packetDestIP[0]; //0.0.0.0 -> a.b.c.d
	new_cell->isbind = 0;
	new_cell->ptr = NULL;
	new_cell->LastByteRcvd = ntohl(new_cell->presentSeq)-1;
	new_cell->presentSeq = fdList[i]->waitAcceptSeqList[0];
	new_cell->presentAck = fdList[i]->waitAcceptAckList[0];
	it = fdList[i]->waitAcceptList.begin();
	freePacket(*it);
	fdList[i]->waitAcceptList.erase(it);
	fdList[i]->waitAcceptSeqList.erase(fdList[i]->waitAcceptSeqList.begin());
	fdList[i]->waitAcceptAckList.erase(fdList[i]->waitAcceptAckList.begin());
	/* Return Data */
	((uint16_t*)param2_ptr->sa_data)[0] = ((uint16_t*)new_cell->destPortNum)[0];
	((uint32_t*)(param2_ptr->sa_data + 2))[0] = ((uint32_t*)new_cell->destAddr)[0];
	param2_ptr->sa_family = new_cell->family;
	returnSystemCall(syscallUUID, new_cell->sockfd);
	return;
}
}
