#include "respondthread.h"
#include <QDebug>
#include "config.h"
#include <sstream>
#include <string>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <string>
#include <regex>
#include <QDebug>
#include <windows.h>//ϵͳ�ײ㣬�ж�Ŀ¼�Ƿ����
#include <QTime>

RespondThread::RespondThread(QObject *parent)
    :QThread(parent)
{
    qDebug()<<"recv thread:"<<QThread::currentThreadId();
}
RespondThread::~RespondThread()
{
    requestInterruption();
    terminate();
    quit();
    wait();
    qDebug()<<"recv thread����";
    closesocket(this->acceptSocket);
    qDebug()<<"recv thread ������socket�ر�";
    qDebug()<<"recv thread running:"<<isRunning();
}
void RespondThread::setAcceptSocket(SOCKET s)
{
    this->acceptSocket=s;
}
void RespondThread::setClientAddr(sockaddr_in addr)
{
    this->clientSock=addr;
}
void RespondThread::setMainPath(string str)
{
    this->mainPath=str;
}

//���Ĵ���
void RespondThread::run()
{
    char recvBuf[Config::BUF_LENGTH];
    char sendBuf[Config::BUF_LENGTH];
    std::string method,url,path,httpHeader;
    std::string httpStatus;
    std::string httpVersion;
    std::string tempStr,acceptStr;
    std::stringstream recvSS;
    QStringList msg;
    QString clientAddrStr(inet_ntoa(this->clientSock.sin_addr));
    int clientPort=this->clientSock.sin_port;

    int rtn;
    int i,j;
    long fileLength;

    DWORD ftyp;

    while(!isInterruptionRequested())
    {
        //��������
        j=0;
        httpHeader="";
        httpStatus=Config::OK_STATUS_STR;
        httpVersion=Config::DEFAULT_HTTP_VERSION_STR;

        memset(recvBuf,0,sizeof(recvBuf));
        rtn=recv(this->acceptSocket,recvBuf,sizeof(recvBuf),0);
        if(rtn==SOCKET_ERROR)
        {//����ʧ��
            msg.clear();
            msg<<QTime::currentTime().toString("hh:mm:ss")
              <<clientAddrStr
              <<QString::fromStdString(std::to_string(clientPort))
              <<QString("")//method
             <<QString("")//url
             <<QString("recv failed");
            emit respondDone(msg);
            return;
        }
        if(rtn==0)
        {//�Է��Ͽ�����
            return;
        }
//        qDebug()<<"recv buf:"<<endl<<QString(recvBuf);

        recvSS.clear();recvSS<<recvBuf;
        qDebug()<<"recvSS str:"<<recvSS.str().c_str();
        while(true)
        {
            std::getline(recvSS,tempStr);
            qDebug()<<"��"<<j++<<":"<<tempStr.c_str();
            if(tempStr.find("HTTP",0)!=tempStr.npos)
            {
                std::stringstream ssTemp(tempStr);
                ssTemp>>method>>url>>httpVersion;
                break;
            }
        }
        while(true)
        {
            std::getline(recvSS,tempStr);
            qDebug()<<"��"<<j++<<":"<<tempStr.c_str();
            if(tempStr.find("Accept:",0)!=tempStr.npos)
            {
                acceptStr=tempStr;
                break;
            }
        }
        acceptStr.replace(0,6,"Conten-Type");//��Accept�滻ΪConten-Type

        for(i=0;i<url.size();i++)
        {//�滻��Windows·��
            if(url[i]=='/')
                url[i]='\\';
        }
        path=this->mainPath+url;//����·��

        ftyp=GetFileAttributesA(path.c_str());
        if(ftyp&FILE_ATTRIBUTE_DIRECTORY)
        {//��һ��Ŀ¼���򿪸�Ŀ¼�µ�����index.html
            path=path+"\\index.html";
        }

        FILE* infile=fopen(path.c_str(),"rb");

        if(!infile)
        {//�ļ�������
            fclose(infile);
            //�����Զ���404ҳ��
            infile=fopen("404.html","rb");
            httpStatus=Config::NOT_FOUND_STR;
        }
        else if(method!="GET")
        {//δʵ��
            fclose(infile);
            //�����Զ���501ҳ��
            infile=fopen("501.html","rb");
            httpStatus=Config::ERROR_METHOD_STR;
        }
        //��ȡ�ļ���С
        fseek(infile,0,SEEK_SET);
        fseek(infile,0,SEEK_END);
        fileLength=ftell(infile);
        //�ļ�ָ���λ
        fseek(infile,0,SEEK_SET);

        qDebug()<<"file open:"<<infile;

        httpHeader=httpVersion+" "
                +httpStatus+"\r\n"
//                +"Content-Type: text/html"+"\r\n"
                +"Content-Length: "+std::to_string(fileLength)+"\r\n"
                +acceptStr+"\r\n"
                +"Server: csr_http1.1\r\n"
                +"Connection: keep-alive\r\n"
                +"\r\n";

        qDebug()<<"http header:"<<httpHeader.c_str();
        //���ͱ�ͷ
        rtn=send(this->acceptSocket,
                 httpHeader.c_str(),
                 (int)(httpHeader.length()),
                 0);
        if(rtn==SOCKET_ERROR)
        {
            msg.clear();
            msg<<QTime::currentTime().toString("hh:mm:ss")
              <<clientAddrStr
              <<QString::fromStdString(std::to_string(clientPort))
             <<QString::fromStdString(method)
            <<QString::fromStdString(url)
             <<QString("Send status code failed");
            emit respondDone(msg);
            return;
        }
        //����������ļ�
        while(true)
        {
            //��������
            memset(sendBuf,0,sizeof(sendBuf));
            fgets(sendBuf,sizeof(sendBuf),infile);
            if(SOCKET_ERROR==(send(this->acceptSocket,sendBuf,strlen(sendBuf),0)))
            {
                msg.clear();
                msg<<QTime::currentTime().toString("hh:mm:ss")
                  <<clientAddrStr
                  <<QString::fromStdString(std::to_string(clientPort))
                 <<QString::fromStdString(method)
                <<QString::fromStdString(url)
                 <<QString("Send file failed");
                emit respondDone(msg);
                return;
            }
            if(feof(infile))
                break;
        }
        fclose(infile);
        msg.clear();
        msg<<QTime::currentTime().toString("hh:mm:ss")
          <<clientAddrStr
          <<QString::fromStdString(std::to_string(clientPort))
         <<QString::fromStdString(method)
        <<QString::fromStdString(url)
         <<QString::fromStdString(httpStatus);
        emit respondDone(msg);
    }
}
