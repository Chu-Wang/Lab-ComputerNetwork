#pragma once
#include "RdtSender.h"
#include "DataStructure.h"
class GBNRdtSender :
	public RdtSender
{

private:
	bool waitingState;
	int base;							//����ţ������δȷ�Ϸ�������
	int nextSeqNum;						//��һ��������������
	//int sentNum;						//�Ѿ����͵ķ�����Ŀ
	const int wndSize;					//�������ڴ�С��ʵ�齨��Ϊ4
	const int seqSize;					//��Ŵ�С��ʵ�齨��λ��Ϊ3λ����0~7
	//int *const stateArray;				//��ŵ�״̬�������СӦ����seqSize
	Packet *sendBuf;					//���ͻ����������淢�͵ı��ģ������ش�����СӦ����seqSize

private:
	void InitState();

public:
	GBNRdtSender();
	GBNRdtSender(int wSize, int sSize);
	virtual ~GBNRdtSender();

	bool getWaitingState();
	bool send(Message &message);
	void receive(Packet &ackPkt);
	void timeoutHandler(int seqNum);

};

