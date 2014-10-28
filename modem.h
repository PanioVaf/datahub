/*
  Kavala Institute of Technology
  Copyright (C) 2012-2013 Panagiotis Vafiadis
  http://vafiadis-ultrasonic.org
 */
#ifndef GSM_MODEM_H
#define GSM_MODEM_H

#include <serialdeviceinfo.h>
#include <abstractserial.h>
#include <QtCore>

/**
 * SMSMessage holds a single SMS message
 *
 */
struct SMSMessage
{
  QString   from;
  QString   message;
  QDateTime dateTime;
};

/**
 * GSM Modem Connection Settings
 *
 */

struct GSMConnectionSettings
{
   QString                  deviceName;
   AbstractSerial::BaudRate baudrate;
   AbstractSerial::DataBits dataBits;
   AbstractSerial::Parity   parity;
   AbstractSerial::StopBits stopBits;
   AbstractSerial::Flow     flow;
};


class Modem : public QObject
{
    Q_OBJECT
public:

    enum ModemStatus
    {
        MODEM_INVALID_BAUD_RATE,
        MODEM_INVALID_DATA_BITS,
        MODEM_INVALID_PARITY,
        MODEM_INVALID_STOP_BITS,
        MODEM_INVALID_FLOW_CONTROL,
        MODEM_CONNECTION_SUCCESS,
        MODEM_CONNECTION_FAILED,
        MODEM_CLOSED_SUCCESS,
        MODEM_CLOSED_FAILED,
        MODEM_COMMAND_SUCCESS,
        MODEM_COMMAND_FAILED,
        MODEM_COMMAND_TIMEOUT

    };

    enum RetrieveParameter
    {
      GET_ALL_SMS,
      GET_READ_SMS,
      GET_UNREAD_SMS,
    };

    /// Constructor
    explicit Modem(QObject *parent = 0);

    /// Set connection settings
    void setConnectionSettings(const GSMConnectionSettings & settings);

    /// Get connection settings
    GSMConnectionSettings connectionSettings() const;

    /// Get port information
    const SerialDeviceInfo & getInfo() const;

    /// Open Connection with serial port
    bool open();

    /// Close Connection with serial port
    bool close();

    /// Check if the connection with the modem is open
    bool isOpen() const;

    /// AT command to set current time and date
    bool setDateTime();

    /// AT command send SMS
    bool sendSMS(const QString & phone, const QString & message);

    /// AT command retrieve sms
    QList<SMSMessage> retrieveSMS(RetrieveParameter param);

    /// AT command delete all read sms
    bool deleteReadSMS();

    /// Return current modem status
    ModemStatus getStatus() const;

private:

    /// Set status
    bool statusSuccess(Modem::ModemStatus newStatus = Modem::MODEM_COMMAND_SUCCESS);
    bool statusFailure(Modem::ModemStatus newStatus = Modem::MODEM_COMMAND_FAILED);
    void setStatus(Modem::ModemStatus newStatus);

    /// Filter invalid characters
    QString filterQuationMarks(const QString & s);
    QString filterChars(const QString & msg);

    /// Extract SMS messages from the received data
    SMSMessage extractSMS(QByteArray & data);
    QList<SMSMessage> extractSMSList(QByteArray & buffer);

private slots:
   void receivedData();
   void commandTimeOut();

private:
    bool running;
    QEventLoop eLoop;
    QByteArray readBuffer;
    QTimer watchDog;
    ModemStatus status;
    GSMConnectionSettings settings;
    AbstractSerial * port;
    SerialDeviceInfo info;    

};

#endif // MODEM_H
