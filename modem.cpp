/*
  Kavala Institute of Technology
  Copyright (C) 2012-2013 Panagiotis Vafiadis
  http://vafiadis-ultrasonic.org
 */
#include "modem.h"

static const char OK[] = {0x0D, 0x0A, 0x4F, 0x4B, 0x0D, 0x0A};

//_________________________________________________________________________________________

Modem::Modem(QObject *parent) :
    QObject(parent), port(0)
{
  watchDog.setSingleShot(true);
  connect(&watchDog, SIGNAL(timeout()), this, SLOT(commandTimeOut()));
  running = false;
}

//_________________________________________________________________________________________

void Modem::setStatus(ModemStatus newStatus)
{
  status = newStatus;
}

//_________________________________________________________________________________________

void Modem::setConnectionSettings(const GSMConnectionSettings & settings)
{
  this->settings = settings;
}

//_________________________________________________________________________________________

GSMConnectionSettings Modem::connectionSettings() const
{
  return this->settings;
}

//_________________________________________________________________________________________

const SerialDeviceInfo & Modem::getInfo() const
{
   return info;
}

//_________________________________________________________________________________________

bool Modem::open()
{
   if(port) close();

   watchDog.stop();
   port = new AbstractSerial();
   port->setDeviceName(settings.deviceName);

   if(port->open(AbstractSerial::ReadWrite) )
     {
        if(!port->setBaudRate(settings.baudrate))
            return statusFailure(MODEM_INVALID_BAUD_RATE);

         if(!port->setDataBits(settings.dataBits))                  
            return statusFailure(MODEM_INVALID_DATA_BITS);

         if(!port->setParity(settings.parity))         
            return statusFailure(MODEM_INVALID_PARITY);

         if(!port->setStopBits(settings.stopBits))         
            return statusFailure(MODEM_INVALID_STOP_BITS);

         if(!port->setFlowControl(settings.flow))         
            return statusFailure(MODEM_INVALID_FLOW_CONTROL);

          running = false;
          port->setCharIntervalTimeout(50);
          info.setName(settings.deviceName);
          connect(port, SIGNAL(readyRead()), this, SLOT(receivedData()));
    }
   else         
       return statusFailure(MODEM_CONNECTION_FAILED);

  return statusSuccess(MODEM_CONNECTION_SUCCESS);
}

//_________________________________________________________________________________________

Modem::ModemStatus Modem::getStatus() const
{
  return status;
}

//_________________________________________________________________________________________

bool Modem::statusSuccess(ModemStatus newStatus)
{
  setStatus(newStatus);
  return true;
}

//_________________________________________________________________________________________

bool Modem::statusFailure(ModemStatus newStatus)
{
  running = false;
  setStatus(newStatus);
  return false;
}

//_________________________________________________________________________________________

bool Modem::close()
{
  if(port)
    {
      readBuffer.clear();
      port->close();
      delete port;
      port = 0;      
      running = false;
      return statusSuccess(MODEM_CLOSED_SUCCESS);
    }

  return statusFailure(MODEM_CLOSED_FAILED);
}

//_________________________________________________________________________________________

bool Modem::isOpen() const
{
  if(port)    
      return port->isOpen();
  return false;
}

//_________________________________________________________________________________________

void Modem::receivedData()
{


  if( !isOpen() ) return;

  QByteArray bytes = port->readAll();
  readBuffer.push_back(bytes);

  qDebug() << "recievedData:" << readBuffer;

  if( !running ) return;

  if( readBuffer.contains(OK))
  {
      watchDog.stop();
      eLoop.quit();
      return;
  }
}

//_________________________________________________________________________________________

QList<SMSMessage> Modem::retrieveSMS(RetrieveParameter param)
{    
    const int read_timeout = 2000;
    const int finish_timeout = 8000;

    QList<SMSMessage> smsList;
    QString cmd;

    if( GET_ALL_SMS == param )
       cmd = "AT+CMGL=\"ALL\"\n";
    else if( GET_READ_SMS == param )
       cmd = "AT+CMGL=\"READ\"\n";
    else if( GET_UNREAD_SMS == param )
       cmd = "AT+CMGL=\"UNREAD\"\n";

    QByteArray buffer;
    buffer.append(cmd);

    if(!isOpen())
    {
      setStatus(MODEM_CONNECTION_FAILED);
      return smsList;
    }

    running = true;
    port->reset();
    setStatus(MODEM_COMMAND_SUCCESS);
    readBuffer.clear();
    port->write(buffer);

    if( !port->waitForReadyRead(read_timeout) )
    {
      running = false;
      setStatus(MODEM_CONNECTION_FAILED);
      return smsList;
    }

    eLoop.exec();

    running = false;

    if( MODEM_COMMAND_SUCCESS == status )
       smsList = extractSMSList(readBuffer);

    return smsList;
}

//_________________________________________________________________________________________

bool Modem::deleteReadSMS()
{
    const int read_timeout = 2000;
    const int finish_timeout = 8000;

    QByteArray buffer;
    buffer.append("AT+CMGD= 1,2\n");

    if(!isOpen())
      return statusFailure(MODEM_CONNECTION_FAILED);

    running = true;
    port->reset();
    setStatus(MODEM_COMMAND_SUCCESS);
    readBuffer.clear();
    port->write(buffer);

    if( !port->waitForReadyRead(read_timeout) )
      return statusFailure();

    eLoop.exec();
    running = false;

    if( MODEM_COMMAND_SUCCESS == status )
        return statusSuccess();
    else
        return false;
}

//_________________________________________________________________________________________

bool Modem::sendSMS(const QString & phone, const QString & message)
{
   const int read_timeout = 2000;
   const int finish_timeout = 8000;

   QString cmd;
   cmd.append("AT+CMGS=").append('\"').append(phone).append('\"').append("\n");
   cmd.append(message).append(QChar(26));

   QByteArray buffer(cmd.toLatin1().data());

   if(!isOpen())
     return statusFailure(MODEM_CONNECTION_FAILED);

   running = true;
   port->reset();
   setStatus(MODEM_COMMAND_SUCCESS);
   readBuffer.clear();
   port->write(buffer);

   if( !port->waitForReadyRead(read_timeout) )
     return statusFailure();

   eLoop.exec();
   running = false;

   if( MODEM_COMMAND_SUCCESS == status )
       return statusSuccess();
   else
       return false;
}

//_________________________________________________________________________________________

bool Modem::setDateTime()
{
  const int read_timeout = 8000;
  const int finish_timeout = 20000;

  QString dateTime = QDateTime::currentDateTime().toString("yy/MM/dd,hh:mm:ss");
  QString cmd = QString("AT+CCLK=\"%1\"\n").arg(dateTime);

  QByteArray buffer;
  buffer.append(cmd);

  if(!isOpen())
    return statusFailure(MODEM_CONNECTION_FAILED);

  running = true;
  port->reset();
  setStatus(MODEM_COMMAND_SUCCESS);
  readBuffer.clear();
  port->write(buffer);

  if( !port->waitForReadyRead(read_timeout) )
    return statusFailure();

  watchDog.start(finish_timeout);

  eLoop.exec();
  running = false;

  if( MODEM_COMMAND_SUCCESS == status )
      return statusSuccess();
  else
      return false;
}

//_________________________________________________________________________________________

void Modem::commandTimeOut()
{
  running = false;
  setStatus(MODEM_COMMAND_TIMEOUT);
  readBuffer.clear();
  qDebug() << "TIMEOUT!!!!!!!!";
  eLoop.quit();
}

//_________________________________________________________________________________________

QString Modem::filterChars(const QString & msg)
{
 QString cleanString;

 for(int i = 0; i < msg.length(); ++i)
 {
   QChar c = msg.at(i);
   if( c == '\n' || c == '\r' || c == '\t' )
           c = ',';

   if( c.isSymbol() || c.isLetterOrNumber() || c.isSpace() ||
           c == '%' || c == '$' || c == '.' || c == '+' || c == '-' ||
           c == '(' || c == ')' || c == '$' || c == '#' || c == '@' ||
           c == '?' || c == '-' || c == '*' || c == '/' || c == '\\' ||
           c == '<' || c == '>' || c == ',' || c == '\"' || c == '\'' ||
           c == '!' || c == ':' || c == ';' || c == '|' || c == '[' ||
           c == ']' )
           cleanString.append(c);
 }

 return cleanString.trimmed();
}

//_________________________________________________________________________________________

QString Modem::filterQuationMarks(const QString & s)
{
 QString filtered;

  for(int i = 0; i < s.length(); ++i)
          if( s[i] != '\"' )
        filtered.append(s[i]);

  return filtered;
}

//_________________________________________________________________________________________

SMSMessage Modem::extractSMS(QByteArray & data)
{
    const int MIN_NUM_FIELDS = 7;
    QString packet = filterChars(data.data());
    QList<QString> items = packet.split(',');
    SMSMessage sms;

    if( items.length() < MIN_NUM_FIELDS) return sms;

    QString sms_phone = filterQuationMarks(items.at(2));
    QString sms_date  = filterQuationMarks(items.at(4));
    QString sms_time  = filterQuationMarks(items.at(5)).left(8);
    QString sms_date_time = QDateTime::currentDateTime().toString("yyyy").left(2) + sms_date + "," + sms_time;
    QString sms_msg   = filterQuationMarks(items.at(7)).trimmed();

    sms.from = sms_phone;
    sms.message = sms_msg;
    sms.dateTime = QDateTime::fromString(sms_date_time, "yyyy/MM/dd,hh:mm:ss") ;

    return sms;
}

//_________________________________________________________________________________________

/*

----------
0 :  "+CMGL: 37"
1 :  ""REC READ""
2 :  ""+306936117266""
3 :  ""
4 :  ""10/08/12"
5 :  "13:05:22+12""
6 :  ""
7 :  "Message-19"
8 :  ""
9 :  ""
----------
0 :  "+CMGL: 38"
1 :  ""REC READ""
2 :  ""+306936117266""
3 :  ""
4 :  ""10/08/12"
5 :  "13:05:26+12""
6 :  ""
7 :  "Message-20"
8 :  ""
9 :  ""
 */
QList<SMSMessage> Modem::extractSMSList(QByteArray & buffer)
{
 int start = 0;
 QList<SMSMessage> smslist;

 while(start < buffer.length() && start != -1)
 {
    int pos = buffer.indexOf("+CMGL", start);

    if(pos == -1) break;

    int nextpos = buffer.indexOf("+CMGL", pos + 1);
    if( nextpos == -1 ) nextpos = buffer.length();

    int numChars = nextpos - pos;

    start = pos + 1;
    QByteArray b = buffer.mid(pos, numChars);

    SMSMessage sms = extractSMS(b);

    if( sms.from == "" ) continue;

    smslist.append(sms);    
 }

  return smslist;
}

//_________________________________________________________________________________________
