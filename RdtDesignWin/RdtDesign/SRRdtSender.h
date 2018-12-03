#pragma once
#include "RdtSender.h"
//#include "SeqStateEnum.h"
class SRRdtSender :
	public RdtSender
{
private:
	const int seqsize;
	const int wndsize;
	Packet *const sendBuf;//���ͻ����������ⷴ����������
	bool *const bufStatus;
	int base, nextSeqnum;

private:
	void Init();
	void printSlideWindow();
	bool isInWindow(int seqnum);

public:
	SRRdtSender(int sSize, int wsize);
	SRRdtSender();
	bool send(Message &message);
	bool getWaitingState();
	void receive(Packet &ackPkt);
	void timeoutHandler(int seqnum);
	virtual ~SRRdtSender();
};

