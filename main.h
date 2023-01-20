#ifndef MAIN_H
#define MAIN_H
#include "QThread"
#include "QtCore"
#include "QCLoader.h"
#include "QtSerialPort/QSerialPort"
#include "QtSerialPort/QSerialPortInfo"
class QCNRead
{
public:
    bool ConnectDevice(DiagInfo&);
    void SetLibraryMode();
    bool ConnectPort(DiagInfo&);
    QString ReadIMEI(DiagInfo&,int);
    bool SendSPC(DiagInfo&,QString SPC);
    bool SetSIMDual(DiagInfo&,bool dual);
    bool BackupQCN(DiagInfo&);
    bool RestoreQCN(DiagInfo&);
    bool SyncEFS(DiagInfo&);
    bool RebootNormal(DiagInfo&);
    void Disconnect();
    void EnableQcnNvItemCallBacks(DiagInfo & info);
};

#endif // MAIN_H
