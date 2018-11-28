#include "stdafx.h"
#include "GBNRdtReceiver.h"
#include "Global.h"


GBNRdtReceiver::GBNRdtReceiver():
	seqSize(8)
{
	initState();
}

void GBNRdtReceiver::initState()
{
	expectedSeqNum = 0;
	lastAckPkt.acknum = 0; //��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ0��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ0
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//���Ը��ֶ�
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}

GBNRdtReceiver::GBNRdtReceiver(int sSize) :
	seqSize(sSize)
{
	initState();
}


GBNRdtReceiver::~GBNRdtReceiver()
{
}

void GBNRdtReceiver::receive(Packet & packet)
{
	if (packet.checksum != pUtils->calculateCheckSum(packet))
	{//���ݰ��𻵣�������Ӧ��
		pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,����У�����", packet);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);//�����ϴ�ȷ�Ϲ���ack
		return;
	}
	if (packet.seqnum != expectedSeqNum)
	{//������Ҫ�����ݰ���������Ӧ��
		pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,������Ų���", packet);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);
		return;
	}
	Message msg;
	memcpy(msg.data, packet.payload, sizeof(packet.payload));
	pns->delivertoAppLayer(RECEIVER, msg);
	lastAckPkt.acknum = packet.seqnum + 1; //ȷ����ŵ����յ��ı������
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
	pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);
	pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
	expectedSeqNum = (expectedSeqNum + 1) % seqSize;
}
