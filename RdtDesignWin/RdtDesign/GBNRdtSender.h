#pragma once
#include "RdtSender.h"
#include "DataStructure.h"
class GBNRdtSender :
	public RdtSender
{

private:
	//bool waitingState;
	int base;							//����ţ������δȷ�Ϸ�������
	int nextSeqnum;						//��һ��������������
	//int sentNum;						//�Ѿ����͵ķ�����Ŀ
	const int wndsize;					//�������ڴ�С��ʵ�齨��Ϊ4
	const int seqsize;					//��Ŵ�С��ʵ�齨��λ��Ϊ3λ����0~7
	//int *const stateArray;				//��ŵ�״̬�������СӦ����seqsize
	Packet *const sendBuf;					//���ͻ����������淢�͵ı��ģ������ش�����СӦ����seqsize

private:
	void Init();
	void printSlideWindow();

public:
	GBNRdtSender();
	GBNRdtSender(int wsize, int sSize);
	virtual ~GBNRdtSender();

	bool getWaitingState();
	bool send(Message &message);
	void timeoutHandler(int seqNum);
	void receive(Packet &ackPkt);

};

