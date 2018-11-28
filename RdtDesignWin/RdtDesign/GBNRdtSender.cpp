#include "stdafx.h"
#include "GBNRdtSender.h"
#include "SeqStateEnum.h"
#include "Global.h"
#include "CommonFunction.h"


GBNRdtSender::GBNRdtSender():
	wndSize(4),seqSize(8)
{
	InitState();
}

GBNRdtSender::GBNRdtSender(int wSize, int sSize):
	wndSize(wSize),seqSize(sSize)
{
	InitState();
}


GBNRdtSender::~GBNRdtSender()
{
	delete[] sendBuf;
}

//�ϲ����send������������
bool GBNRdtSender::send(Message & message)
{
	if (nextSeqNum < (base + wndSize) % seqSize)
	{
		sendBuf[nextSeqNum].acknum = -1;//����ط���̫��
		sendBuf[nextSeqNum].seqnum = nextSeqNum;//���к�
		memcpy(sendBuf[nextSeqNum].payload, message.data, sizeof(message.data));
		sendBuf[nextSeqNum].checksum = pUtils->calculateCheckSum(sendBuf[nextSeqNum]);
		pUtils->printPacket("���ͷ����ͱ���", sendBuf[nextSeqNum]);
		//��ʾ��������
		std::cout << "\n����ǰ��"; 
		printSlideWindow(seqSize, wndSize, base, nextSeqNum);
		if (base == nextSeqNum)//������ʱ��
			pns->startTimer(SENDER, Configuration::TIME_OUT, sendBuf[nextSeqNum].seqnum);
		pns->sendToNetworkLayer(RECEIVER, sendBuf[nextSeqNum]);
		//������ϣ�����״̬
		nextSeqNum = (nextSeqNum + 1) % seqSize;
		//��ʾ��������
		std::cout << "\n���ͺ�";
		printSlideWindow(seqSize, wndSize, base, nextSeqNum);
		return true;
	}
	else
	{//������������
		std::cout << "\n������������\n\n";
		waitingState = true;//���������ȴ�ȷ��
		return false;
	}
}

//�����Ƿ��ڵȴ�״̬
bool GBNRdtSender::getWaitingState()
{
	/*if (waitingState == true)
	{
		std::cout << "for debug, waiting state:true\n\n";
	}*/
	return waitingState;
}

//����ack
void GBNRdtSender::receive(Packet &ackPkt)
{
	if (ackPkt.checksum != pUtils->calculateCheckSum(ackPkt))
	{//ack�𻵣���������
		return;
	}
	base = ackPkt.acknum;
	if (base == nextSeqNum)
	{
		pns->stopTimer(SENDER, ackPkt.acknum);
	}
	else
	{
		pns->startTimer(SENDER, Configuration::TIME_OUT, (ackPkt.acknum + 1) % seqSize);
	}
	waitingState = false;
}

//����ʱ
void GBNRdtSender::timeoutHandler(int seqNum)
{
	if ((seqNum<base || seqNum>nextSeqNum) && (base <= nextSeqNum))
	{
		return;
	}
	if ((base > nextSeqNum) && (seqNum<base && seqNum>nextSeqNum))
	{
		return;
	}
	int i;
	pns->startTimer(SENDER,Configuration::TIME_OUT,seqNum);
	//�����ش�
	std::cout << "\n�����ش�!\n\n";
	if (base <= seqNum)
	{
		for (i = base; i <= seqNum; i++)
		{
			pns->sendToNetworkLayer(RECEIVER, sendBuf[i]);
		}
	}
	else
	{
		for (i = base; i < seqSize; i++)
		{
			pns->sendToNetworkLayer(RECEIVER, sendBuf[i]);
		}
		for (i = 0; i <= seqNum; i++)
		{
			pns->sendToNetworkLayer(RECEIVER, sendBuf[i]);
		}
	}
}

//��ʼ����������״̬
void GBNRdtSender::InitState()
{
	sendBuf = new Packet[seqSize];
	base = nextSeqNum = 0;
	waitingState = false;
}

