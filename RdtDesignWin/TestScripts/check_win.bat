@echo off
:: appname ������
:: inputname �����ļ���
:: outputname ����ļ���
:: resultname �������̨����ض����ļ���

set appname="..\Debug\RdtDesign.exe"
set inputname="..\data\input.txt"
set gbnoutputname="..\data\GBNOutput.txt"
set gbnresultname="..\data\GBNLog.txt"
set sroutputname="..\data\SROutput.txt"
set srresultname="..\data\SRLog.txt"
set tcpoutputname="..\data\TcpOutput.txt"
set tcpresultname="..\data\TcpLog.txt"

for /l %%i in (1,1,10) do (
    echo Test %appname% %%i:
    fc /N %inputname% %GBNOutputname%
    fc /N %inputname% %SROutputname%
    fc /N %inputname% %TcpOutputname%
)
pause