/*
  Kavala Institute of Technology
  Copyright (C) 2012-2013 Panagiotis Vafiadis
  http://vafiadis-ultrasonic.org
 */
#ifndef MONITOR_H
#define MONITOR_H

#include <QObject>
#include <QtCore>
#include "modem.h"
#include "gateway.h"

class Monitor : public QThread
{
    Q_OBJECT
public:

    // Constructor
    explicit Monitor();

    // Destructor
    ~Monitor();

    // Open connection with the modem
    bool openSession(const QString & port);

    // Close connection with the Modem
    void closeSession();

    // Restart modem
    void restart();

    // Set Sensoras API Gateway
    void setGateway(QUrl url, QString username, QString key);

    // Set the sms checking interval
    void setCheckInterval(int msec);

    // Run event loop
    void run();

signals:    
    void logMessage(QString);
    void shutdownRequest();

private slots:
    void check();

private:
    void transmitData(const QList<SMSMessage> & smsList);

private:    
    QString comPort;
    QTimer  checkTimer;

    Gateway gateway;
    Modem   modem;

    QString account_username;
    QString account_key;
    QUrl    account_url;

    int     checkCycle;
    QEventLoop eLoop;
};

#endif // MONITOR_H
