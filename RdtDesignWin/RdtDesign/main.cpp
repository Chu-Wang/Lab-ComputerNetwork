// main.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"
#include "StopWaitRdtSender.h"
#include "StopWaitRdtReceiver.h"
#include "GBNRdtReceiver.h"
#include "GBNRdtSender.h"


int main(int argc, char* argv[])
{
	RdtSender *pStopWaitSender = new StopWaitRdtSender();
	RdtReceiver * pStopWaitReceiver = new StopWaitRdtReceiver();
	RdtSender *pGBNSender = new GBNRdtSender();
	RdtReceiver *pGBNReceiver = new GBNRdtReceiver();
	ofstream stopWatiLog;
	ofstream gbnLog;
	streambuf *coutBackup = cout.rdbuf();

	pns->init();
	pns->setRtdSender(pStopWaitSender);
	pns->setRtdReceiver(pStopWaitReceiver);
	pns->setInputFile("..\\data\\input.txt");
	pns->setOutputFile("..\\data\\StopWaitOutput.txt");
	stopWatiLog.open("..\\data\\StopWait.log");
	cout.rdbuf(stopWatiLog.rdbuf());
	pns->start();
	stopWatiLog.close();
	
	pns->init();
	pns->setRtdSender(pGBNSender);
	pns->setRtdReceiver(pGBNReceiver);
	pns->setInputFile("..\\data\\input.txt");
	pns->setOutputFile("..\\data\\GBNOutput.txt");
	gbnLog.open("..\\data\\GBN.log");
	cout.rdbuf(gbnLog.rdbuf());
	pns->start();
	gbnLog.close();

	cout.rdbuf(coutBackup);

	delete pStopWaitSender;
	delete pStopWaitReceiver;
	delete pGBNSender;
	delete pGBNReceiver;
	delete pUtils;									//ָ��Ψһ�Ĺ�����ʵ����ֻ��main��������ǰdelete
	delete pns;										//ָ��Ψһ��ģ�����绷����ʵ����ֻ��main��������ǰdelete
	
	return 0;
}

