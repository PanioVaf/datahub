/*
  Kavala Institute of Technology
  Copyright (C) 2012-2013 Panagiotis Vafiadis
  http://vafiadis-ultrasonic.org
 */
#ifndef GATEWAY_H
#define GATEWAY_H

#include <QObject>
#include <QtGui>
#include <QtCore>
#include <QtNetwork>
#include <QNetworkAccessManager>
#include <QUrl>

class Gateway : QObject
{
    Q_OBJECT
public:

    typedef QMap<QString, QString> Parameters;

    enum SENSORAS_STATUS
    {
      SENSORAS_UKNOWN,
      SENSORAS_SUCCESS,
      SENSORAS_FAILED,
      SENSORAS_TIMEOUT
    };

    /// constructor
    explicit Gateway(QObject *parent = 0);

    /// Contact sensoras gateway
    SENSORAS_STATUS startRequest(QUrl url, Parameters params, int timeout = 10000);

private slots:
    void finished(QNetworkReply *);
    void error(QNetworkReply::NetworkError err);
    void timeout();

private:        
    QTimer timeGuard;
    QEventLoop eLoop;
    SENSORAS_STATUS status;
    QNetworkAccessManager * manager;
    QNetworkReply *reply;
};

#endif // GATEWAY_H
