#include "stdafx.h"
#include "GBNRdtSender.h"
#include "SeqStateEnum.h"
#include "Global.h"

//��ʼ����������״̬
void GBNRdtSender::Init()
{
	//sendBuf = new Packet[seqsize];
	base = 0;
	nextSeqnum = 0;
}

bool GBNRdtSender::isInWindow(int seqnum)
{
	if (base < (base + wndsize) % seqsize)
	{
		return seqnum >= base && seqnum < (base + wndsize) % seqsize;
	}
	else
	{
		return seqnum >= base || seqnum < (base + wndsize) % seqsize;
	}
}

GBNRdtSender::GBNRdtSender():
	wndsize(4),seqsize(8),sendBuf(new Packet[8])
{
	Init();
}

GBNRdtSender::GBNRdtSender(int wsize, int sSize):
	wndsize(wsize),seqsize(sSize),sendBuf(new Packet[sSize])
{
	Init();
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
		std::cout << "[SENDER]���������������������Ե�\n\n";
		return false;
	}
	else
	{
		sendBuf[nextSeqnum].acknum = -1;//����ط���̫���������䷢�����շ���ackӦ��û�ð�
		sendBuf[nextSeqnum].seqnum = nextSeqnum;//���к�
		memcpy(sendBuf[nextSeqnum].payload, message.data, sizeof(message.data));
		sendBuf[nextSeqnum].checksum = pUtils->calculateCheckSum(sendBuf[nextSeqnum]);
		pUtils->printPacket("[Debug]���ͷ����ͱ���", sendBuf[nextSeqnum]);
		//��ʾ��������
		std::cout << "\n[SENDER]����ǰ���ڣ�"; 
		printSlideWindow();
		if (base == nextSeqnum)
		{//gbnЭ����ֻ��һ����ʱ�����������ĸ����鳬ʱ�������ش�����δȷ�ϵķ���
			pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
		}
		pns->sendToNetworkLayer(RECEIVER, sendBuf[nextSeqnum]);
		//������ϣ�����״̬
		nextSeqnum = (nextSeqnum + 1) % seqsize;//��Ҫ����ȡģ����
		//��ʾ��������
		std::cout << "[SENDER]���ͺ󴰿ڣ�";
		printSlideWindow();
		std::cout << std::endl;
		return true;
	}
}

//�����Ƿ��ڵȴ�״̬������������true�����򷵻�false
bool GBNRdtSender::getWaitingState()
{
	//����ʵ���ĵ���gbnЭ���л������������޷������ϲ�Ӧ������
	return (base + wndsize) % seqsize == (nextSeqnum) % seqsize;
}

//����ack
void GBNRdtSender::receive(Packet &ackPkt)
{
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	if (checkSum != ackPkt.checksum) 
	{
		pUtils->printPacket("[Debug]���յ�ack�𻵣�У��Ͳ����", ackPkt);
	}
	else
	{
		base = (ackPkt.acknum + 1) % seqsize;//�ۻ�ȷ�ϣ�������ȡģ
		if (base == nextSeqnum)
		{
			pns->stopTimer(SENDER, 0);
		}
		else
		{//������ʱ��
			pns->stopTimer(SENDER, 0);
			pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
		}
		std::cout << "\n[SENDER]�յ�ack�����������ƶ���";
		printSlideWindow();
		std::cout << std::endl;
	}
}

//����ʱ
void GBNRdtSender::timeoutHandler(int seqNum)
{
	std::cout << "\n[Debug]���ͳ�ʱ\n\n";
	//ĳ�����鷢�ͳ�ʱ��
	//��Ϊ���շ�û�л��治�ᱣ��˳����˱����ط������Ѿ�������δȷ�ϵķ���
	if (nextSeqnum == base)
	{//��ʱ��������������
		return;
	}
	else
	{
		pns->startTimer(SENDER, Configuration::TIME_OUT, 0);//������ʱ�������¼�ʱ
		int i;
		for (i = base; i != nextSeqnum; i = (i + 1) % seqsize)
		{//��������ȡģ�ˣ����Ƕ���
			pns->sendToNetworkLayer(RECEIVER, sendBuf[i]);
			pUtils->printPacket("[Debug]��ʱ�ش��ķ���:", sendBuf[i]);
		}
	}
	std::cout << "\n[Debug]�ط����ݰ����\n\n";
}

void GBNRdtSender::printSlideWindow()
{
	int i;
	for (i = 0; i < seqsize; i++)
	{
		if (i == base)
			std::cout << "[";
		std::cout << i;
		if (i == nextSeqnum)
			std::cout << "*";
		if (i == (base + wndsize - 1) % seqsize)
			std::cout << "]";
		std::cout << " ";
	}
	std::cout << std::endl;
}

