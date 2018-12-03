#include "stdafx.h"
#include "SRRdtReceiver.h"
#include "Global.h"

void SRRdtReceiver::Init()
{
	base = 0;
	//nextSeqnum = 0;
	for (int i = 0; i < seqsize; i++)
	{
		bufStatus[i] = false;
	}
	lastAckPkt.acknum = -1; //��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ0��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ0
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//���Ը��ֶ�
	memset(lastAckPkt.payload, '.', Configuration::PAYLOAD_SIZE);
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}

void SRRdtReceiver::printSlideWindow()
{
	int i;
	for (i = 0; i < seqsize; i++)
	{
		if (i == base)
			std::cout << "[";
		if (isInWindow(i) && bufStatus[i] == true)
			std::cout << "�ѻ���";
		else if (isInWindow(i))
			std::cout << "�ڴ�";
		if (i == (base + wndsize) % seqsize)
			std::cout << "]";
		if (isInWindow(i) == false)
			std::cout << "������";
		std::cout << " ";
	}
	std::cout << std::endl;
}

bool SRRdtReceiver::isInWindow(int seqnum)
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

SRRdtReceiver::SRRdtReceiver():
	seqsize(8),wndsize(4),recvBuf(new Packet[seqsize]),bufStatus(new bool[seqsize])
{
	Init();
}

SRRdtReceiver::SRRdtReceiver(int sSize, int wsize):
	seqsize(sSize),wndsize(wsize), recvBuf(new Packet[seqsize]), bufStatus(new bool[seqsize])
{
	Init();
}

void SRRdtReceiver::receive(Packet & packet)
{
	int checksum = pUtils->calculateCheckSum(packet);
	if (checksum != packet.checksum)
	{//���ݰ��𻵣�������Ӧ��
		pUtils->printPacket("[Debug]���շ�û����ȷ�յ����ͷ��ı���,����У�����", packet);
		//std::cout << "\n\n���յ����ݷ���У��ʹ���\n\n";
		return;
	}
	else
	{
		if (isInWindow(packet.seqnum) == false)
		{//������Ҫ�����ݰ���������Ӧ��
			pUtils->printPacket("[Debug]�������������ݷ���", packet);
			//std::cout << "\n\n�������������ݷ���\n\n";
			//lastAckPkt.acknum = base;
			lastAckPkt.acknum = packet.seqnum;
			lastAckPkt.seqnum = -1;//����
			memset(lastAckPkt.payload, '.', Configuration::PAYLOAD_SIZE);
			lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt);
			return;
		}
		else
		{//���Լ���Ҫ�����ݰ�������ack�����»������ͻ�������
			recvBuf[packet.seqnum] = packet;//Packet�������˸�ֵ�����
			bufStatus[packet.seqnum] = true;
			lastAckPkt.acknum = packet.seqnum;
			lastAckPkt.seqnum = 0;//����
			memset(lastAckPkt.payload, '.', sizeof(lastAckPkt.payload));//����
			pUtils->printPacket("[Debug]���շ�����ack", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt);
			while (bufStatus[base] == true)
			{
				Message msg;
				memcpy(msg.data, recvBuf[base].payload, sizeof(recvBuf[base].payload));
				pns->delivertoAppLayer(RECEIVER, msg);
				pUtils->printPacket("[Debug]�ݽ��ϲ�:", recvBuf[base]);
				bufStatus[base] = false;
				base = (base + 1) % seqsize;
			}
			std::cout << "\n[RECEIVER]�յ����ݰ��������ƶ���";
			printSlideWindow();
			std::cout << std::endl;
		}
	}
}


SRRdtReceiver::~SRRdtReceiver()
{
	delete[] recvBuf;
	delete[] bufStatus;
}
