#include "stdafx.h"
#include "GBNRdtSender.h"
#include "SeqStateEnum.h"
#include "Global.h"


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
	if (getWaitingState())
	{//�����������޷�������������
		std::cout << "���������������������Ե�\n\n";
		return false;
	}
	else
	{
		sendBuf[nextSeqNum].acknum = -1;//����ط���̫���������䷢�����շ���ackӦ��û�ð�
		sendBuf[nextSeqNum].seqnum = nextSeqNum;//���к�
		memcpy(sendBuf[nextSeqNum].payload, message.data, sizeof(message.data));
		sendBuf[nextSeqNum].checksum = pUtils->calculateCheckSum(sendBuf[nextSeqNum]);
		pUtils->printPacket("���ͷ����ͱ���", sendBuf[nextSeqNum]);
		//��ʾ��������
		std::cout << "\n����ǰ���ڣ�"; 
		printSlideWindow();
		if (base == nextSeqNum)
		{//gbnЭ����ֻ��һ����ʱ�����������ĸ����鳬ʱ�������ش�����δȷ�ϵķ���
			pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
		}
		pns->sendToNetworkLayer(RECEIVER, sendBuf[nextSeqNum]);
		//������ϣ�����״̬
		nextSeqNum = (nextSeqNum + 1) % seqSize;//��Ҫ����ȡģ����
		//��ʾ��������
		std::cout << "���ͺ󴰿ڣ�";
		printSlideWindow();
		std::cout << std::endl;
		return true;
	}
}

//�����Ƿ��ڵȴ�״̬������������true�����򷵻�false
bool GBNRdtSender::getWaitingState()
{
	//����ʵ���ĵ���gbnЭ���л������������޷������ϲ�Ӧ������
	return (base + wndSize) % seqSize == (nextSeqNum) % seqSize;
}

//����ack
void GBNRdtSender::receive(Packet &ackPkt)
{
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	if (checkSum != ackPkt.checksum) 
	{
		pUtils->printPacket("���յ�ack�𻵣�У��Ͳ����", ackPkt);
	}
	else
	{
		base = (ackPkt.acknum + 1) % seqSize;//�ۻ�ȷ�ϣ�������ȡģ
		if (base == nextSeqNum)
		{
			pns->stopTimer(SENDER, 0);
		}
		else
		{//������ʱ��
			pns->stopTimer(SENDER, 0);
			pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
		}
		std::cout << "\n�յ�ack�����������ƶ���";
		printSlideWindow();
		std::cout << std::endl;
	}
}

//����ʱ
void GBNRdtSender::timeoutHandler(int seqNum)
{
	//ĳ�����鷢�ͳ�ʱ��
	//��Ϊ���շ�û�л��治�ᱣ��˳����˱����ط������Ѿ�������δȷ�ϵķ���
	if (nextSeqNum == base)
	{//��ʱ��������������
		return;
	}
	else
	{
		pns->startTimer(SENDER, Configuration::TIME_OUT, 0);//������ʱ�������¼�ʱ
		int i;
		for (i = base; i != nextSeqNum; i = (i + 1) % seqSize)
		{//��������ȡģ�ˣ����Ƕ���
			pns->sendToNetworkLayer(RECEIVER, sendBuf[i]);
			pUtils->printPacket("��ʱ�ش��ķ���:", sendBuf[i]);
		}
	}
}

//��ʼ����������״̬
void GBNRdtSender::InitState()
{
	sendBuf = new Packet[seqSize];
	base = nextSeqNum = 0;
}

void GBNRdtSender::printSlideWindow()
{
	int i;
	for (i = 0; i < seqSize; i++)
	{
		if (i == base)
			std::cout << "[";
		std::cout << i;
		if (i == nextSeqNum)
			std::cout << "* ";
		else
			std::cout << " ";
		if (i == (base + wndSize) % seqSize)
			std::cout << "]";
	}
	std::cout << std::endl;
}

