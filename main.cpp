#include <QCoreApplication>
#include <QTextStream>
#include <iostream>
#include "stdlib.h"
#include "QThread"
#include "QtCore"
#include "QCLoader.h"
#include "QtSerialPort/QSerialPort"
#include "QtSerialPort/QSerialPortInfo"

using namespace std;
//定义在头文件内会提示未定义函数，很奇怪
bool ConnectDevice(DiagInfo &);
void SetLibraryMode();
bool ConnectPort(DiagInfo &);
QString ReadIMEI(DiagInfo &,int);
bool SendSPC(DiagInfo &,QString SPC);
bool SetSIMDual(DiagInfo &,bool dual);
bool BackupQCN(DiagInfo &);
bool RestoreQCN(DiagInfo &);
bool SyncEFS(DiagInfo &);
bool RebootNormal(DiagInfo &);
void Disconnect();
void EnableQcnNvItemCallBacks(DiagInfo &info);


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    //添加命令行参数
    QCoreApplication::setApplicationName("QCNTool");
    QCoreApplication::setApplicationVersion("1.5.0");
    QCommandLineParser parser;
    parser.setApplicationDescription("a tool to download/flash qcn from/to your phone");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption writeqcn(QStringList() << "w" << "write",
                                "write qcn to your phone");
    QCommandLineOption readqcn(QStringList() << "r" << "read",
                               "backup qcn from your phone");
    QCommandLineOption targetport(QStringList() << "p" << "port",
                                  "Set 901D port num",
                                  "int");
    QCommandLineOption targetpath(QStringList() << "f" << "file",
                                  "qcn file path",
                                  "string");

    parser.addOption(targetport);
    parser.addOption(targetpath);

    parser.addOption(writeqcn);
    parser.addOption(readqcn);
    parser.process(a);
    int cport= parser.value(targetport).toInt();
    QString cpath = parser.value(targetpath);

    DiagInfo info;
    info.portnum = cport;

    if((!parser.isSet(writeqcn))&&(!parser.isSet(readqcn)))
    {
        std::cout << "Invalid function quest";
        QLIB_DisconnectServer(info.hndl);
        return 1;
    }

    if(parser.isSet(writeqcn)&&parser.isSet(readqcn))
    {
        std::cout << "Invalid function quest";
        QLIB_DisconnectServer(info.hndl);
        return 1;
    }

    std::cout << "a small free tool to flash/backup qcn into/from your phone\n"
              << "Modified by Zi_Cai\n";

    if (!ConnectDevice(info))
    {
        QLIB_DisconnectServer(info.hndl);
        return 1;
    }

    if (parser.isSet(writeqcn))
    {

        if (cpath == "")
        {
            std::cout << "please input file path";
            QLIB_DisconnectServer(info.hndl);
            return 1;

        }
        QFileInfo fi(cpath);
        if (!fi.isFile()){
            std::cout << "invalid file input";
            return 1;
        }
        std::cout << "\nLoading Data File...";
        int get1 = -1;
        int get2 = -1;
        if (!QLIB_NV_LoadNVsFromQCN(info.hndl,cpath.toLocal8Bit().data(),&get1,&get2))
        {
            std::cout << " error";
            QLIB_DisconnectServer(info.hndl);
            return 1;
        }else{
            std::cout << " OK";
            std::cout << "\nWriting Data File to phone...";
            int res2;
            if (!QLIB_NV_WriteNVsToMobile(info.hndl,&res2))
            {
                std::cout << " error";
                QLIB_DisconnectServer(info.hndl);
                return 1;
            }else{
                std::cout << " OK";
            }

        }
    }
    if (parser.isSet(readqcn))
    {
        QString path;
        if (cpath == "")
        {
            cpath = QDir().currentPath();
            QDateTime dteNow = QDateTime::currentDateTime();
            QString fnl = dteNow.toString("smh_d_M_yyyy").replace(" ","_").replace(":","_");
            path = QDir::cleanPath(cpath +QDir::separator()+ "QCN_"+fnl+".qcn");
        }
        else
        {
            std::cout <<"file path should be null";
            return 1;
        }

        int renas2;
        std::cout << "\nReading QCN from phone...";
        if (!QLIB_BackupNVFromMobileToQCN(info.hndl,path.toLocal8Bit().data(),&renas2))
        {
            std::cout <<" error";
            QLIB_DisconnectServer(info.hndl);
            return 1;
        }
        else
        {
            std::cout <<" OK";
            std::cout <<"\nBackup file : ";
            std::cout <<path.toStdString().data();
            std::cout <<"\n";
        }
    }
    QLIB_DisconnectServer(info.hndl);
    return 0;
}



