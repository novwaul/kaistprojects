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
		//this->syscall_read(syscallUUID, pid, param.param1_int, param.param2_ptr, param.param3_int);
		break;
	case WRITE:
		//this->syscall_write(syscallUUID, pid, param.param1_int, param.param2_ptr, param.param3_int);
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

void TCPAssignment::packetArrived(std::string fromModule, Packet* packet)
{
	char srcAddr[4], destAddr[4], srcPortNum[2], destPortNum[2], signBit[1], seq[4], ack[4];
  if(fromModule == "IPv4")
	{
		/* get src IP, src Port num, dest IP, dest Port num */
		packet->readData(14+12, destAddr, 4);
		packet->readData(14+16, srcAddr, 4);
		packet->readData(34+0, destPortNum, 2);
		packet->readData(34+2, srcPortNum,2);
		/* get sequence number */
		packet->readData(34+4, ack, 4);
		((uint32_t*)ack)[0] = htonl(ntohl(((uint32_t*)ack)[0]) + 1);
		/* get ack number */
		packet->readData(34+8, seq, 4);
		/* get sign bits */
		packet->readData(34+13, signBit, 1);

/*** DETERMIN RECEIVED PACKET  ***/
/* SYN-ACK PACKET */
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
			/* make ACK packet */
			Packet* myPacket = allocatePacket((size_t)54);
			uint8_t flag = 1<<4; //ack bit
			uint8_t headerlen = 5<<4;
			uint16_t rcvw = 15200;
			rcvw = htons(rcvw);
			uint16_t checksum = 0;
			myPacket->writeData(14+12,srcAddr,4);
			myPacket->writeData(14+16,(*iter)->destAddr,4);
			myPacket->writeData(34+0,srcPortNum,2);
			myPacket->writeData(34+2,destPortNum,2);
			myPacket->writeData(34+4,seq,4);
			myPacket->writeData(34+8,ack,4);
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
			param2_ptr->sa_family = (*iter)->family;
			((uint16_t*)(param2_ptr->sa_data))[0] = ((uint16_t*)((*iter)->destPortNum))[0];
			((uint32_t*)(param2_ptr->sa_data + 2))[0] = ((uint32_t*)((*iter)->destAddr))[0];
			this->returnSystemCall((*iter)->waitUUID, 0);// return connect success value, which is 0
			return;
		}
/* SYN PACKET */
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
				Packet* myPacket2 = allocatePacket(54); // TCP 20B + IP 20B + Etype 14B
				uint8_t flag[1] ; // syn-ack bit
				flag[0] = 18;
				uint8_t headerlen = 5<<4;// (5*32)/8 = 20
				uint16_t rcvw = 51200; // window size
				rcvw = htons(rcvw);
				uint16_t checksum = 0;
				uint16_t urg = 0;
				myPacket2->writeData(14+12,srcAddr,4);
				myPacket2->writeData(14+16,(*iter)->destAddr,4); // already stored in Big-endian
				myPacket2->writeData(34+0,srcPortNum,2);// already stored in Big-endian
				myPacket2->writeData(34+2,destPortNum,2);// already stored in Big-endian
				myPacket2->writeData(34+4,seq,4);//sequence num
				myPacket2->writeData(34+8,ack,4);// ack num
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
					if( ( ((uint32_t*)((*iter)->srcAddr))[0] == ((uint32_t*)srcAddr)[0] ||
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
				uint16_t rcvw = 51200; // window size
				rcvw = htons(rcvw);
				uint16_t checksum = 0;
				srand(time(NULL));
				uint32_t seq_start = rand();// make random number
				uint16_t urg = 0;
				if(((uint32_t*)((*iter)->srcAddr))[0] == 0) myPacket2->writeData(14+12,srcAddr,4); //already stored in Big-endian
				else myPacket2->writeData(14+12,((uint32_t*)((*iter)->srcAddr)),4);
				myPacket2->writeData(14+16,destAddr,4); // already stored in Big-endian
				myPacket2->writeData(34+0,srcPortNum,2);// already stored in Big-endian
				myPacket2->writeData(34+2,destPortNum,2);// already stored in Big-endian
				myPacket2->writeData(34+4,(uint8_t*)&seq_start,4);//sequence num
				myPacket2->writeData(34+8,(uint8_t*)ack,4);// ack num
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
/* FIN PACKET */
		else if(signBit[0] == 1)
		{

			std::vector<struct cell*>::iterator iter;
			/* Find CONNECTING, FIN_WAITING_1, FIN_WAITING_2 Socket */
	    for ( iter = fdList.begin() ; iter != fdList.end(); ++iter)
			{
				int state = (*iter)->state;
				if((state == CONNECTING)||(state == FIN_WAITING_1)||(state == FIN_WAITING_2))
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
			Packet* myPacket = allocatePacket(54);
			uint8_t flag = 1<<4; //ack bit, 10000
			uint8_t headerlen = 5<<4;
			uint16_t rcvw = 51200;
			rcvw = htons(rcvw);
			uint16_t checksum = 0;
			srand(time(NULL));
			uint32_t seq = rand();
			myPacket->writeData(14+12,srcAddr,4);
			myPacket->writeData(14+16,destAddr,4);
			myPacket->writeData(34+0,srcPortNum,2);
			myPacket->writeData(34+2,destPortNum,2);
			myPacket->writeData(34+4,(uint8_t*)&seq,4);
			myPacket->writeData(34+8,ack,4);
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
			}
			/* FIN_WAITING_2 to TIME_WAITING */
			else if((*iter)->state == FIN_WAITING_2)
			{
				(*iter)->state = TIME_WAITING;
				(*iter)->timerUUID = addTimer((*iter), 10);
				fdList.erase(iter);
			}
			/* FIN_WAITING_1 to INCLOSING */
			else if((*iter)->state == FIN_WAITING_1)
			{
				(*iter)->state = INCLOSING;
			}
			else assert(0);
			return;
		}
/* ACK PACKET */
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
		/* Normal(No Transition) */
		if((*iter)->state == CONNECTING)
		{
			this->freePacket(packet);
			return;
		}
		/* Transition to CONNECTING 1 : NO LISTENING SOCKET(SYN_SENT->READYCONNECT case) */
		else if((*iter)->state == SYN_RCV)
		{
			this->freePacket(packet);
			struct sockaddr * param2_ptr = (struct sockaddr *)((*iter)->ptr);
			((uint16_t*)param2_ptr->sa_data)[0] = ((uint16_t*)((*iter)->destPortNum))[0];
			((uint32_t*)(param2_ptr->sa_data + 2))[0] = ((uint32_t*)((*iter)->destAddr))[0];
			param2_ptr->sa_family = (*iter)->family;
			returnSystemCall((*iter)->waitUUID, (*iter)->sockfd);// return Connect
			(*iter)->state = CONNECTING;
			return;
		}
		/* Transition to CONNECTING 2 : WITH LISENING SOCKET */
		else if((*iter)->state == LISTENING)
		{
			/* Find Packet in listenList*/
			int found = -1;
			for(std::vector<Packet *>::iterator liter = (*iter)->listenList.begin() ; liter != (*iter)->listenList.end(); liter++)
			{
				uint32_t srcIP, destIP;
				uint16_t srcPN, destPN;
				Packet* waitingPack = (*liter);
				waitingPack->readData(14+12, &destIP, 4);
				waitingPack->readData(14+16, &srcIP, 4);
				waitingPack->readData(34+0, &destPN, 2);
				waitingPack->readData(34+2, &srcPN, 2);
				if((((uint32_t*)(srcAddr))[0] == srcIP /*|| srcIP == 0*/ )&& ((uint32_t*)(destAddr))[0] == destIP &&
				((uint16_t*)(srcPortNum))[0] == srcPN && ((uint16_t*)(destPortNum))[0] == destPN)
				{
					found = 0;
					this->freePacket(*liter);
					(*iter)->listenList.erase(liter);
					break;
				}
			}
			if(found == -1) assert(0);
			Packet *myPacket = clonePacket(packet);
			this->freePacket(packet);
			(*iter)->waitAcceptList.push_back(myPacket);

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
			/* socket is found*/
			else
			{
				this->freePacket((*iter)->waitAcceptList[0]);
				(*iter)->waitAcceptList.erase((*iter)->waitAcceptList.begin());
				(*it)->state = CONNECTING;
				((uint32_t*)((*it)->srcAddr))[0] = src_IP;
				((uint32_t*)((*it)->destAddr))[0] = dest_IP;
				((uint16_t*)((*it)->destPortNum))[0] = dest_PN;
				((uint16_t*)((*it)->srcPortNum))[0] = src_PN;
				(*it)->isbind = 0;
				struct sockaddr * param2_ptr = (struct sockaddr *)((*it)->ptr);
				((uint16_t*)param2_ptr->sa_data)[0] = ((uint16_t*)((*it)->destPortNum))[0];
				((uint32_t*)(param2_ptr->sa_data + 2))[0] = ((uint32_t*)((*it)->destAddr))[0];
				param2_ptr->sa_family = (*it)->family;
				returnSystemCall((*it)->waitUUID, (*it)->sockfd);// return Accept
				return;
			}
		}
		/* Transition to FIN_WAITING_2*/
		else if((*iter)->state == FIN_WAITING_1)
		{
			(*iter)->state = FIN_WAITING_2;
			this->freePacket(packet);
			return;
		}
		/* CLOSE SOCKET */
		else if((*iter)->state == LASTACK_WAITNG)
		{
			this->freePacket(packet);
			this->removeFileDescriptor((*iter)->pid, (*iter)->sockfd);
			returnSystemCall((*iter)->waitUUID,0 );
			delete *iter;
			fdList.erase(iter);
			return;
		}
		/* Transition to TIME_WAITING*/
		else if((*iter)->state == INCLOSING)
		{
			this->freePacket(packet);
			(*iter)->state = TIME_WAITING;
			(*iter)->timerUUID = addTimer((*iter), 10);
			fdList.erase(iter);
			return;
		}
		else assert(0);
	}
/* FIN-ACK PACKET */
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
			Packet* myPacket = allocatePacket(54);
			uint8_t flag = 1<<4; //ack bit, 10000
			uint8_t headerlen = 5<<4;
			uint16_t rcvw = 51200;
			rcvw = htons(rcvw);
			uint16_t checksum = 0;
			srand(time(NULL));
			uint32_t seq = rand();
			myPacket->writeData(14+12,srcAddr,4);
			myPacket->writeData(14+16,destAddr,4);
			myPacket->writeData(34+0,srcPortNum,2);
			myPacket->writeData(34+2,destPortNum,2);
			myPacket->writeData(34+4,(uint8_t*)&seq,4);
			myPacket->writeData(34+8,ack,4);
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
			/* send packet*/
			this->sendPacket("IPv4", myPacket);

			(*iter)->state = TIME_WAITING;
			(*iter)->timerUUID = addTimer((*iter), 10);
			fdList.erase(iter);
			return;
		}
	}
	else assert(0);
}

void TCPAssignment::timerCallback(void* payload)
{
	/* Close Connection*/
	struct cell * ptr = (struct cell *) payload;
	returnSystemCall(ptr->waitUUID,0);
	this->removeFileDescriptor(ptr->pid, ptr->sockfd);
	delete ptr;
	return;
}

void TCPAssignment::syscall_socket(UUID syscallUUID, int pid, int param1_int, int param2_int)
{

	int sockfd = this->createFileDescriptor(pid);
	struct cell *cellptr = new struct cell;//make file descriptor list
	cellptr->sockfd = sockfd;
	cellptr->isbind = -1; // -1 means not binded
	cellptr->state = NOTREADYCONNECT;
	cellptr->pid = pid;
	((uint32_t*)cellptr->srcAddr)[0] = ((uint32_t*)cellptr->destAddr)[0] =((uint16_t*)cellptr->srcPortNum)[0] = ((uint16_t*)cellptr->destPortNum)[0] = -1;
	fdList.push_back(cellptr);
  this->returnSystemCall(syscallUUID, sockfd);
	return;
}
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
	/* Close Pure & SYN_SENT & LISENING Socket */
	if((*iter)->state == NOTREADYCONNECT || (*iter)->state == SYN_SENT || (*iter)->state == LISTENING )
	{
		delete *iter;
		fdList.erase(iter);
		this->removeFileDescriptor(pid, param1_int);
		this->returnSystemCall(syscallUUID, 0); //0 means success
		return;
	}
	else if((*iter)->state == CONNECTING || (*iter)->state == READYCONNECT)
		(*iter)->state = FIN_WAITING_1;
	else if((*iter)->state == CLOSE_WAITING)
		(*iter)->state = LASTACK_WAITNG;
	else assert(0);
	/* Store PID and syscall UUID */
	(*iter)->waitUUID = syscallUUID;
	/* Send FIN Packet */
	uint8_t flag = 1; //FIN bit
	uint8_t headerlen = 5<<4;
	uint16_t checksum = 0;
	uint16_t rcvw = 51200; // window size
	rcvw = htons(rcvw);
	srand(time(NULL));
	uint32_t seq = rand();// make random number
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
	/* send packet */
	this->sendPacket("IPv4", myPacket);
	return;// blocking(waiting ACK PACKET)
}
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
		((uint16_t*)srcPortNum)[0] = 0;
		do{
		 found = 0; // check whether srcPortNum is changed
		 for (iter = fdList.begin() ; iter != fdList.end(); ++iter)
		  {
			  if((*iter)->isbind == -1) continue;
			  else if(((uint16_t*)((*iter)->srcPortNum))[0] == ((uint16_t*)srcPortNum)[0])//find same port num
			  {
				  if(((uint32_t*)((*iter)->srcAddr))[0] == ((uint32_t*)srcAddr)[0]
						|| ((uint32_t*)((*iter)->srcAddr))[0] == 0)
					  {
							found = -1;
							((uint16_t*)srcPortNum)[0]++;
							//break;
					  }
				 }
				 else continue;
			 }
		 }while(found != 0);
		 /* Implicit binding*/
		 ((uint32_t*)((*socket)->srcAddr))[0] = ((uint32_t*)srcAddr)[0];
		 ((uint16_t*)((*socket)->srcPortNum))[0] = ((uint16_t*)srcPortNum)[0];
		 (*socket)->isbind = 0;
	}
  /* store connectUUID*/
	(*socket)->waitUUID = syscallUUID;
	(*socket)->ptr = param2_ptr;
	/* store destIP and dest Port num*/
	((uint32_t*)((*socket)->destAddr))[0] = ((uint32_t*)destAddr)[0];
	((uint16_t*)((*socket)->destPortNum))[0] = ((uint16_t*)destPortNum)[0];
	(*socket)->state = SYN_SENT;
	/* make SYN packet */
	Packet* myPacket = allocatePacket(54); // TCP 20B + IP 20B + Etype 14B
	uint8_t flag = 1<<1; // syn bit
	srand(time(NULL));
	uint32_t seq = rand();
	uint8_t headerlen = 5<<4;
  uint16_t rcvw = 15200;// receive window size = 15200
	rcvw = htons(rcvw);
	uint16_t checksum = 0;
	myPacket->writeData(14+12,srcAddr,4); // already stored in Big-endian
	myPacket->writeData(14+16,destAddr,4); // already stored in Big-endian
	myPacket->writeData(34+0,srcPortNum,2);// already stored in Big-endian
	myPacket->writeData(34+2,destPortNum,2);// already stored in Big-endian
	myPacket->writeData(34+4,(uint8_t*)&seq,4);
	myPacket->writeData(34+12,&headerlen,1);
	myPacket->writeData(34+13,&flag,1);
	myPacket->writeData(34+14,(uint8_t*)&rcvw,2);
	myPacket->writeData(34+16, (uint8_t*)&checksum, 2);
	uint8_t header_buffer[20];
	myPacket->readData(34, header_buffer, 20);
	checksum = NetworkUtil::tcp_sum(((uint32_t*)((*socket)->srcAddr))[0],((uint32_t*)((*socket)->destAddr))[0],header_buffer,20);
	checksum = ~checksum;
	checksum = htons(checksum);
	myPacket->writeData(34+16, (uint8_t*)&checksum, 2);
	/* send packet*/
	this->sendPacket("IPv4", myPacket);
	return; //blocking
}
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
	it = fdList[i]->waitAcceptList.begin();
	freePacket(*it);
	fdList[i]->waitAcceptList.erase(it);
	/* Return Data */
	((uint16_t*)param2_ptr->sa_data)[0] = ((uint16_t*)new_cell->destPortNum)[0];
	((uint32_t*)(param2_ptr->sa_data + 2))[0] = ((uint32_t*)new_cell->destAddr)[0];
	param2_ptr->sa_family = new_cell->family;
	returnSystemCall(syscallUUID, new_cell->sockfd);
	return;
}
}
