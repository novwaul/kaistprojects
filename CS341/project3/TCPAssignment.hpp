/*
 * E_TCPAssignment.hpp
 *
 *  Created on: 2014. 11. 20.
 *      Author: Keunhong Lee
 */

#ifndef E_TCPASSIGNMENT_HPP_
#define E_TCPASSIGNMENT_HPP_


#include <E/Networking/E_Networking.hpp>
#include <E/Networking/E_Host.hpp>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <netinet/in.h>


#include <E/E_TimerModule.hpp>

namespace E
{

class TCPAssignment : public HostModule, public NetworkModule, public SystemCallInterface, private NetworkLog, private TimerModule
{
private:

private:
	enum state_s{ NOTREADYCONNECT,SYN_SENT ,READYCONNECT, CONNECTING, LISTENING, FIN_WAITING_1, FIN_WAITING_2,
		TIME_WAITING, CLOSE_WAITING, LASTACK_WAITNG, INCLOSING, SYN_RCV};
	struct cell // information of socket
	{
		int isbind; // indicate it has source IP and source port num
		int state;//store state
		int sockfd;// socket file descriptor
		char srcPortNum[2];// source port number
		char srcAddr[4];// source IP
		char destPortNum[2];// destination port number
		char destAddr[4];// destination IP
		ushort family;//type
		UUID waitUUID;//store syscallUUID used in return systemcall
		UUID timerUUID;//store timer UUID
		unsigned int backLog;
		std::vector<Packet* > listenList; //store connection request
		std::vector<Packet* > waitAcceptList;
		int pid; // store pid
		void *ptr; //store any pointer
	};
	std::vector<struct cell *> fdList; // store socket fd pointer
	virtual void timerCallback(void* payload) final;
	void syscall_socket(UUID syscallUUID, int pid, int param1_int, int param2_int);
	void syscall_close(UUID syscallUUID, int pid, int param1_int);
	void syscall_bind(UUID syscallUUID, int pid, int param1_int, struct sockaddr *param2_ptr,socklen_t param3_int);
	void syscall_getsockname(UUID syscallUUID, int pid, int param1_int, struct sockaddr *param2_ptr, socklen_t* param3_ptr);
	void syscall_connect(UUID syscallUUID, int pid, int param1_int, struct sockaddr* param2_ptr, socklen_t param3_int);
	void syscall_getpeername(UUID syscallUUID, int pid, int param1_int, struct sockaddr *param2_ptr, socklen_t* param3_ptr);
	void syscall_listen(UUID syscallUUID, int pid, int param1_int, int param2_int);
	void syscall_accept(UUID syscallUUID, int pid, int param1_int, struct sockaddr *param2_ptr,socklen_t* param3_int);
public:
	TCPAssignment(Host* host);
	virtual void initialize();
	virtual void finalize();
	virtual ~TCPAssignment();
protected:
	virtual void systemCallback(UUID syscallUUID, int pid, const SystemCallParameter& param) final;
	virtual void packetArrived(std::string fromModule, Packet* packet) final;
};

class TCPAssignmentProvider
{
private:
	TCPAssignmentProvider() {}
	~TCPAssignmentProvider() {}
public:
	static HostModule* allocate(Host* host) { return new TCPAssignment(host); }
};

}


#endif /* E_TCPASSIGNMENT_HPP_ */
