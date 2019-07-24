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

#define MSS 512

namespace E
{

class TCPAssignment : public HostModule, public NetworkModule, public SystemCallInterface, private NetworkLog, private TimerModule
{
private:
	enum state_s{ NOTREADYCONNECT,SYN_SENT ,READYCONNECT, CONNECTING, LISTENING, FIN_WAITING_1, FIN_WAITING_2,
		TIME_WAITING, CLOSE_WAITING, LASTACK_WAITNG, INCLOSING, SYN_RCV};
	enum pending_s{ READING, NONE};
	struct S_Packet // information of sent packet
	{
		int size; // data size without header length
		Packet* packet; // sent packet
	};
	struct cell // information of socket
	{
		/* Socket Information */
		int isbind; // indicate it has source IP and source port num
		int state;//store state
		int pending;//indicate pending function(none or read)
		int sockfd;// socket file descriptor
		char srcPortNum[2];// source port number
		char srcAddr[4];// source IP
		char destPortNum[2];// destination port number
		char destAddr[4];// destination IP
		int pid; // store pid
		void *ptr; //store any pointer
		int duplicate; // check the number of received duplicate packets
		int totalSendDupPacket;// the number of sent packets by fast retransmission
		int totalRead;//the total byte that USER wants to read
		int curRead;// current read byte
		int totalSend;//the total byte that USER wants to send
		int curSend;// current sent byte
		int cancel; // check the number of cancelled timer
		int recovery;// indicate whether in fast retransmit or not
		Time SRTT, RTTVAR, RTO; // Smooth RTT, RTT Variance, Time out
		uint16_t readOffset; // store the position of next start to read in packet
		uint16_t LastByteRcvd; //the last byte ACKed by reciever
		uint16_t maxIntBufSize; // maximum internal buffer size
		uint16_t maxRecvBufSize;//maximum receive buffer size
		ushort family;//type
		uint32_t startAck;//initial ACK number
		uint32_t presentSeq;//present sequence number "to send"
		uint32_t presentAck;//present ack number "to check" whether packet is arrived well
		uint32_t cwnd;//congestion window size
		UUID waitUUID;//store any syscallUUID
		UUID timerUUID;//store only timer UUID
		std::vector<S_Packet* > internalBuf;//store packets to be sent by "write" function
		std::vector<Packet* > recvBuf;//store received packets to be read by "read" function
		std::vector<Time> sendTime;//store send time
		/* Information only for LISTENING Socket */
		unsigned int backLog;
		std::vector<Packet* > listenList; //store connection request
		std::vector<Packet* > waitAcceptList;//ACK-confirmed packets are stored
		std::vector<uint32_t> seqList;//store the sequence number of not yet established(connected) socket
		std::vector<uint32_t> ackList;//store the acknowledge number of not yet established(connected) socket
		std::vector<uint32_t> waitAcceptSeqList;//store the sequence number of established(connecter) socket
		std::vector<uint32_t> waitAcceptAckList;//store the acknowledge number of established(connected) socket
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
  void syscall_write(UUID syscallUUID, int pid, int param1_int, void* param2_ptr, int param3_int);
	void syscall_read(UUID syscallUUID, int pid, int param1_int, void* param2_ptr, int param3_int);
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
