/*
  Kavala Institute of Technology
  Copyright (C) 2012-2013 Panagiotis Vafiadis
  http://vafiadis-ultrasonic.org
 */
#include "monitor.h"
#include "infopacket.h"

Monitor::Monitor() : QThread()
{

}

Monitor::~Monitor()
{

}

bool Monitor::openSession(const QString & port)
{
  if( modem.isOpen() )
      modem.close();

  comPort = port;  
  GSMConnectionSettings modemSettings;
  modemSettings.deviceName = port;
  modemSettings.baudrate = AbstractSerial::BaudRate9600;
  modemSettings.dataBits = AbstractSerial::DataBits8;
  modemSettings.parity   = AbstractSerial::ParityNone;
  modemSettings.stopBits = AbstractSerial::StopBits1;
  modemSettings.flow     = AbstractSerial::FlowControlHardware;

  modem.setConnectionSettings(modemSettings);

  if( modem.open() )
  {     
      emit logMessage(tr("Connection with modem established!"));

      bool result = modem.setDateTime();
      if( result )
          emit logMessage("Synchronized GSM modem with date and time.");
      else
          emit logMessage("Synchronization GSM modem failed.");

      connect(&checkTimer, SIGNAL(timeout()), this, SLOT(check()));      
      checkTimer.setSingleShot(true);
      checkTimer.start(checkCycle);
     return true;  
  }
  else  
  {
     emit logMessage(tr("Error: Connection with the modem cannot be established!"));
     return false;  
  }
}

void Monitor::setGateway(QUrl url, QString username, QString key)
{
    account_username = username;
    account_key = key;
    account_url = url;
}

void Monitor::transmitData(const QList<SMSMessage> & smsList)
{
   InfoPacket info;

   emit logMessage(QString("New SMS = %1").arg(smsList.size() ));

   foreach(SMSMessage sms, smsList)
    {
       Gateway::Parameters params;

       info.setSMS(sms.message);
       info.extract();

       if( info["u"] == "" ) continue;

       params["username"]   = account_username;
       params["key"]        = account_key;
       params["phone"]      = sms.from;
       params["waterlevel"] = info["u"];
       params["timestamp"]  = sms.dateTime.toString("yyyy-MM-dd hh:mm:ss");

       emit logMessage(QString("SMS> %1, [%2 cm], %3").arg(sms.from).arg(info["u"]).arg(sms.dateTime.toString("yyyy-MM-dd hh:mm:ss")));

       Gateway::SENSORAS_STATUS result = gateway.startRequest(account_url, params);

       if( Gateway::SENSORAS_SUCCESS == result )
       {
          emit logMessage("Data transmission finished successfully...");        
       }
       else if( Gateway::SENSORAS_FAILED  == result )
       {
          emit logMessage("Data transmmission failed...");
       }
       else if( Gateway::SENSORAS_TIMEOUT == result )
       {
          emit logMessage("Data transmmission failed due to timeout...");
       }
    }
}

void Monitor::restart()
{

}

void Monitor::closeSession()
{
    checkTimer.stop();

    if( modem.close() )
        emit logMessage(tr("Connection with modem closed!"));
    else
        emit logMessage(tr("Error: Closing connection with modem"));
}

void Monitor::setCheckInterval(int msec)
{   
  checkCycle = msec;
}

void Monitor::run()
{
  connect(this, SIGNAL(shutdownRequest()), this, SLOT(quit()));
  QThread::exec();
}

void Monitor::check()
{
 modem.open();

 emit logMessage("Checking for new sms...");

 QList<SMSMessage> smsList = modem.retrieveSMS(Modem::GET_ALL_SMS);

 transmitData(smsList);

 if( !smsList.empty() )
    modem.deleteReadSMS();

modem.close();
 checkTimer.start(checkCycle);
}
