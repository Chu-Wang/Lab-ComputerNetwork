#include "stdafx.h"
#include "GBNRdtReceiver.h"
#include "Global.h"


GBNRdtReceiver::GBNRdtReceiver():
	seqsize(8)
{
	Init();
}

void GBNRdtReceiver::Init()
{
	expectedSeqNum = 0;
	lastAckPkt.acknum = -1; //��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ0��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ0
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//���Ը��ֶ�
	memset(lastAckPkt.payload, '.', Configuration::PAYLOAD_SIZE);
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}

GBNRdtReceiver::GBNRdtReceiver(int sSize) :
	seqsize(sSize)
{
	Init();
}


GBNRdtReceiver::~GBNRdtReceiver()
{
}

void GBNRdtReceiver::receive(Packet & packet)
{
	if (packet.checksum != pUtils->calculateCheckSum(packet))
	{//���ݰ��𻵣�������Ӧ��
		pUtils->printPacket("[Debug]���շ�û����ȷ�յ����ͷ��ı���,����У�����", packet);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);//�����ϴε�
	}
	if (packet.seqnum != expectedSeqNum)
	{//������Ҫ�����ݰ���������Ӧ��
		pUtils->printPacket("[Debug]�������������ݷ���", packet);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);//�����ϴε�
	}
	else
	{
		Message msg;
		memcpy(msg.data, packet.payload, sizeof(packet.payload));
		pns->delivertoAppLayer(RECEIVER, msg);
		pUtils->printPacket("[Debug]���շ���ȷ�����͵��ϲ�APP��", packet);
		//std::cout << "�ݽ���Ӧ�����ݷ��飺" << packet.seqnum << "\n\n";
		lastAckPkt.acknum = packet.seqnum; //ȷ����ŵ����յ��ı������
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("[Debug]���շ�����ȷ�ϱ���", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
		expectedSeqNum = (expectedSeqNum + 1) % seqsize;//������ȡģ
	}
	std::cout << "\n[RECEIVER]ȷ�Ϻ�ack��" << lastAckPkt.acknum << "\n\n";
}
