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
		//this->syscall_connect(syscallUUID, pid, param.param1_int,
		//		static_cast<struct sockaddr*>(param.param2_ptr), (socklen_t)param.param3_int);
		break;
	case LISTEN:
		//this->syscall_listen(syscallUUID, pid, param.param1_int, param.param2_int);
		break;
	case ACCEPT:
		//this->syscall_accept(syscallUUID, pid, param.param1_int,
		//		static_cast<struct sockaddr*>(param.param2_ptr),
		//		static_cast<socklen_t*>(param.param3_ptr));
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
		//this->syscall_getpeername(syscallUUID, pid, param.param1_int,
		//		static_cast<struct sockaddr *>(param.param2_ptr),
		//		static_cast<socklen_t*>(param.param3_ptr));
		break;
	default:
		assert(0);
	}
}

void TCPAssignment::packetArrived(std::string fromModule, Packet* packet)
{

}

void TCPAssignment::timerCallback(void* payload)
{

}

/*---------------------------------------------------------------------------*/
void TCPAssignment::syscall_socket(UUID syscallUUID, int pid, int param1_int, int param2_int)
{

	int sockfd = this->createFileDescriptor(pid);
	struct cell *cellptr = new struct cell;//make file descriptor list
	cellptr->sockfd = sockfd;
	cellptr->isbind = -1; // -1 means not binded
	fdList.push_back(cellptr);
  this->returnSystemCall(syscallUUID, sockfd);
	return;
}
void TCPAssignment::syscall_close(UUID syscallUUID, int pid, int param1_int)
{
	this->removeFileDescriptor(pid, param1_int);
	for (std::vector<struct cell *>::iterator iter = fdList.begin() ; iter != fdList.end(); ++iter)//erase socket fd from fdList
	{
		if((*iter)->sockfd == param1_int)
		{
			delete *iter;
			fdList.erase(iter);
			break;
		}
	}
	this->returnSystemCall(syscallUUID, 0); //0 means success
	return;
}
void TCPAssignment::syscall_bind(UUID syscallUUID, int pid, int param1_int, struct sockaddr *param2_ptr,socklen_t param3_int)
{
	/* error check */
	 std::vector<struct cell*>::iterator temp = fdList.end();
	 for (std::vector<struct cell*>::iterator iter = fdList.begin() ; iter != fdList.end(); ++iter)
	 {
		 if((*iter)->sockfd == param1_int)//find socket fd
		 {
			 if((*iter)->isbind == 0) return;//erorr: already binded
			 else temp = iter;
		 }
		 if((*iter)->isbind == -1) continue;
		 else if(((*iter)->portnum[0] == param2_ptr->sa_data[0])&&((*iter)->portnum[1] == param2_ptr->sa_data[1]))//find same port num
		 {
			 if((
				 ((*iter)->addr[0] == param2_ptr->sa_data[2])&&
			   ((*iter)->addr[1] == param2_ptr->sa_data[3])&&
		     ((*iter)->addr[2] == param2_ptr->sa_data[4])&&
				 ((*iter)->addr[3] == param2_ptr->sa_data[5]))//error: same address
				 ||
				 ((param2_ptr->sa_data[2]||param2_ptr->sa_data[3]||param2_ptr->sa_data[4]||param2_ptr->sa_data[5]) == 0)//error
			   ||
			   (((*iter)->addr[0]||(*iter)->addr[1]||(*iter)->addr[2]||(*iter)->addr[3]) == 0)) return; //error
			 else continue;
		 }
	 }
	 if(temp == fdList.end()) return;//error: cannot find socket fd
	 /* make return value */
	 (*temp)->portnum[0] = param2_ptr->sa_data[0];
	 (*temp)->portnum[1] = param2_ptr->sa_data[1];
	 (*temp)->addr[0] = param2_ptr->sa_data[2];
	 (*temp)->addr[1] = param2_ptr->sa_data[3];
	 (*temp)->addr[2] = param2_ptr->sa_data[4];
	 (*temp)->addr[3] = param2_ptr->sa_data[5];
	 (*temp)->family = param2_ptr->sa_family;
	 (*temp)->isbind = 0;
	 this->returnSystemCall(syscallUUID, 0); //0 means success
	 return;
}
void TCPAssignment::syscall_getsockname(UUID syscallUUID, int pid, int param1_int, struct sockaddr *param2_ptr, socklen_t* param3_ptr)
{
	std::vector<struct cell*>::iterator iter;
  for (iter = fdList.begin() ; iter != fdList.end(); ++iter)
	{
		if((*iter)->sockfd == param1_int) break;
	}
	if(iter == fdList.end()) return; //error: cannot find socket fd
	param2_ptr->sa_family = (*iter)->family;
	param2_ptr->sa_data[0] = (*iter)->portnum[0] ;
	param2_ptr->sa_data[1] = (*iter)->portnum[1];
	param2_ptr->sa_data[2] = (*iter)->addr[0] ;
	param2_ptr->sa_data[3] = (*iter)->addr[1] ;
	param2_ptr->sa_data[4] = (*iter)->addr[2] ;
	param2_ptr->sa_data[5] = (*iter)->addr[3] ;
	this->returnSystemCall(syscallUUID, 0);//0 means success
	return;
}
}
