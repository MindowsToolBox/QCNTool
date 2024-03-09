#include "QThread"
#include "QtCore"
#include "QCLoader.h"
#include "QtSerialPort/QSerialPort"
#include "QtSerialPort/QSerialPortInfo"
#include <iostream>
using namespace std;


void SetLibraryMode()
{
    std::cout << "\nSetting QPST mode...";
    QLIB_SetLibraryMode(0);
    std::cout << " OK";
}

bool ConnectPort(DiagInfo & info)
{
    std::cout << "\nConnecting to phone...";
    info.hndl = QLIB_ConnectServer(info.portnum);
    unsigned int a = QLIB_IsPhoneConnected(info.hndl);
    if ( a )
    {
        std::cout << " OK";
        unsigned long _iMSM_HW_Version = 0;
        unsigned long _iMobModel = 0;
        char _sMobSwRev[512];
        char _sModelStr[512];
        unsigned char bOk;
        bOk = QLIB_DIAG_EXT_BUILD_ID_F(info.hndl, &_iMSM_HW_Version,&_iMobModel, _sMobSwRev, _sModelStr );
        if (bOk)
        {
            std::cout << "\nMSM-HW ver : ";
            std::cout << QString::number(_iMSM_HW_Version).toStdString().data();
            std::cout << "\nMobModel : ";
            std::cout << QString::number(_iMobModel).toStdString().data();
            //std::cout << "\nSoftware : ";
            // std::cout << QString::number(_sMobSwRev).toStdString().data();
        }
        return true;
    }
    else
    {
        std::cout << " error";
        std::cout << "\nPlease select currect diag port!";
        return false;
    }
}
bool SetSIMDual(DiagInfo & info,bool dual)
{
    std::cout << "\nSet Multi Sim...";
    if (dual)
    {
        unsigned char res = QLIB_NV_SetTargetSupportMultiSIM(info.hndl,true);
        if (res)
        {
            std::cout << " 2";
            return true;
        }else{
            std::cout << " error";
            return false;
        }
    }else{
        if (QLIB_NV_SetTargetSupportMultiSIM(info.hndl,false))
        {
            std::cout << " 1";
            return true;
        }else{
            std::cout << " error";
            return false;
        }
    }
}


bool SyncEFS(DiagInfo & info)
{
    std::cout << "\nSyncing EFS...";
    unsigned char b = 0;
    unsigned char b2 = 0;
    b = 47;
    b2 = 4;
    try
    {
        QLIB_EFS2_SyncWithWait(info.hndl, & b, 2000, & b2);
    }
    catch (...)
    {
        std::cout << " error";
        std::cout << "\nCan not sync EFS";
        return false;
    }
    std::cout << " OK";
    return true;
}
enum mode_enum_type {
    MODE_OFFLINE_A_F,
    MODE_OFFLINE_D_F,
    MODE_RESET_F,
    MODE_FTM_F,
    MODE_ONLINE_F,
    MODE_LPM_F,
    MODE_POWER_OFF_F,
    MODE_MAX_F};
bool RebootNormal(DiagInfo & info)
{
    std::cout << "Rebooting phone...";
    QLIB_DIAG_CONTROL_F(info.hndl,MODE_OFFLINE_D_F);
    QThread::sleep(2000);
    QLIB_DIAG_CONTROL_F(info.hndl,MODE_RESET_F);
    std::cout << " OK";
    return true;
}

typedef struct{
    std::wstring imei;
    std::wstring tac;
    std::wstring fac;
    std::wstring snr;
    std::wstring svn;
    std::wstring luhnCode;
} Imei_Info;

QString ReadIMEI(DiagInfo & info,int index)
{
    unsigned char array[128];
    std::wstring array2[15];
    int array3[15];
    unsigned short num1 = 4;
    unsigned char res = QLIB_DIAG_NV_READ_EXT_F(info.hndl, NV_UE_IMEI_I, array, index, 128, &num1);
    if (!res)
        return "000000000000000";
    int num = 0;
    for (int i = 1; i <= 8; i++)
    {
        if (i != 8)
        {
            array3[num] = static_cast<int>(array[i]);
            array3[num] &= 240;
            array3[num] >>= 4;
            array3[num + 1] = static_cast<int>(array[i + 1] & 15);
        }
        else
        {
            array3[num] = static_cast<int>(array[i]);
            array3[num] &= 240;
            array3[num] >>= 4;
        }
        num += 2;
    }
    Imei_Info imeiinfo;
    for (int j = 0; j < 15; j++)
    {
        array2[j] = std::to_wstring(array3[j]);
        if (j < 6)
        {
            imeiinfo.tac += array2[j];
        }
        else if (j >= 6 && j <= 7)
        {
            imeiinfo.fac += array2[j];
        }
        else if (j >= 7 && j <= 13)
        {
            imeiinfo.snr += array2[j];
        }
    }
    imeiinfo.luhnCode = array2[14];
    imeiinfo.imei = imeiinfo.tac + imeiinfo.fac + imeiinfo.snr + imeiinfo.luhnCode;
    if (imeiinfo.imei.size() != 15)
    {
        return "000000000000000";
    }
    return QString::fromWCharArray(imeiinfo.imei.c_str());
}



bool SendSPC(DiagInfo & info,QString SPC)
{
    QString test = SPC;
    unsigned char test2[6];
    memcpy( test2, test.toStdString().c_str() ,test.size());
    std::cout << "\nSending SPC...";
    int piSPC_Result;

    if(QLIB_DIAG_SPC_F(info.hndl,test2,&piSPC_Result))
    {
        std::cout << " OK";
        QString IMEI1=ReadIMEI(info,0);
        QString IMEI2=ReadIMEI(info,1);
        std::cout << "\nPhone IMEI1 : ";
        std::cout << IMEI1.toStdString().data();
        std::cout << "\nPhone IMEI2 : ";
        std::cout << IMEI2.toStdString().data();
        return true;
    }else{
        std::cout << " error";
    }
    return true;
}

bool ConnectDevice(DiagInfo &info)
{
    SetLibraryMode();
    if (!ConnectPort(info))
    {
        return false;
    }
    if (!SendSPC(info,"000000"))
    {
        return false;
    }

    if (!SetSIMDual(info,(ReadIMEI(info,0) == ReadIMEI(info,1))))
    {
        return false;
    }
    return true;
}
