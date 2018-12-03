#pragma once
#include "RdtSender.h"
class TcpRdtSender :
	public RdtSender
{
private:
	//bool waitingState;
	int base;							//����ţ������δȷ�Ϸ�������
	int nextSeqnum;						//��һ��������������
	const int wndsize;					//�������ڴ�С��ʵ�齨��Ϊ4
	const int seqsize;					//��Ŵ�С��ʵ�齨��λ��Ϊ3λ����0~7
	Packet *const sendBuf;				//���ͻ����������淢�͵ı��ģ������ش�����СӦ����seqsize
	//int dupAck[3];						//����ack
	int dupAckNum;							//�յ�3������ack�����ش�

private:
	void Init();
	bool isInWindow(int seqnum);
	void printSlideWindow();

public:
	TcpRdtSender();
	TcpRdtSender(int wsize, int ssize);
	virtual ~TcpRdtSender();

public:
	bool getWaitingState();
	bool send(Message &message);
	void timeoutHandler(int seqNum);
	void receive(Packet &ackPkt);
};

