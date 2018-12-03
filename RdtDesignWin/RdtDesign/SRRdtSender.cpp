#include "stdafx.h"
#include "SRRdtSender.h"
#include "Tool.h"
#include "Global.h"

void SRRdtSender::Init()
{
	base = nextSeqnum = 0;
	for (int i = 0; i < seqsize; i++)
	{
		bufStatus[i] = false;
	}
}

void SRRdtSender::printSlideWindow()
{
	int i;
	for (i = 0; i < seqsize; i++)
	{
		if (i == base)
			std::cout << "[";
		if (isInWindow(i) && i >= nextSeqnum)			//���ã�δ����
			std::cout << "����";
		else if (isInWindow(i) && bufStatus[i] == true)		//���ͣ���ȷ��
			std::cout << "��ȷ��";
		else if (isInWindow(i))						//�ѷ��ͣ�δȷ��
			std::cout << "�ѷ���";
		if (i == (base + wndsize) % seqsize)
			std::cout << "]";
		if (isInWindow(i) == false)
			std::cout << "������";						//�����ã�������
		std::cout << " ";
	}
	std::cout << std::endl;
}

//�ж�����Ƿ��ڴ�����
bool SRRdtSender::isInWindow(int seqnum)
{
	//return seqnum >= base && seqnum <= (base + wndsize) % seqsize;
	if (base < (base + wndsize) % seqsize)
	{
		return seqnum >= base && seqnum < (base + wndsize) % seqsize;
	}
	else
	{
		return seqnum >= base || seqnum < (base + wndsize) % seqsize;
	}
}

SRRdtSender::SRRdtSender(int sSize, int wsize):
	seqsize(sSize),wndsize(wsize),sendBuf(new Packet[sSize]),bufStatus(new bool[sSize])
{
	Init();
}

SRRdtSender::SRRdtSender():
	seqsize(8),wndsize(4), sendBuf(new Packet[8]), bufStatus(new bool[8])
{
	Init();
}

bool SRRdtSender::send(Message & message)
{
	if (getWaitingState())
	{//�����������ȴ�ack
		std::cout << "[SENDER]���������������������Ե�\n\n";
		return false;
	}
	sendBuf[nextSeqnum].acknum = -1;
	sendBuf[nextSeqnum].seqnum = nextSeqnum;
	memcpy(sendBuf[nextSeqnum].payload, message.data, sizeof(message.data));
	sendBuf[nextSeqnum].checksum = pUtils->calculateCheckSum(sendBuf[nextSeqnum]);
	pUtils->printPacket("[Debug]���ͷ����ͱ���", sendBuf[nextSeqnum]);
	//��ʾ��������
	std::cout << "\n[SENDER]����ǰ���ڣ�";
	printSlideWindow();
	//���ͱ���
	pns->sendToNetworkLayer(RECEIVER, sendBuf[nextSeqnum]);
	//������ʱ����srЭ����ÿ�������Ӧһ����ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, nextSeqnum);
	//������ϣ�����״̬
	nextSeqnum = (nextSeqnum + 1) % seqsize;//������ȡģ
	std::cout << "[SENDER]���ͺ󴰿ڣ�";
	printSlideWindow();
	std::cout << std::endl;
	return true;
}


bool SRRdtSender::getWaitingState()
{
	return (base + wndsize) % seqsize == (nextSeqnum) % seqsize;
}

void SRRdtSender::receive(Packet & ackPkt)
{
	int checksum = pUtils->calculateCheckSum(ackPkt);
	if (checksum != ackPkt.checksum)
	{
		pUtils->printPacket("[Debug]���յ�ack�𻵣�У��Ͳ����", ackPkt);
		return;
	}
	else
	{
		pns->stopTimer(SENDER, ackPkt.acknum);
		if (isInWindow(ackPkt.acknum))
		{//���´���
			bufStatus[ackPkt.acknum] = true;
			while (bufStatus[base] == true)
			{//�ƶ�base
				bufStatus[base] = false;
				base = (base + 1) % seqsize;
			}
			std::cout << "\n[SENDER]�յ�ack�����������ƶ���";
			printSlideWindow();
			std::cout << std::endl;
		}
	}
}

void SRRdtSender::timeoutHandler(int seqnum)
{
	std::cout << "\n[Debug]���ݰ���ʱ��" << seqnum << "\n\n";
	//���ù���������ţ����շ��л���������ʹʧ��Ҳ��Ҫ��
	pns->sendToNetworkLayer(RECEIVER, sendBuf[seqnum]);
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqnum);
	std::cout << "\n[Debug]�ط����ݰ����:" << seqnum << "\n\n";
}

SRRdtSender::~SRRdtSender()
{
	delete[] sendBuf;
	delete[] bufStatus;
}
