/*
  Kavala Institute of Technology
  Copyright (C) 2012-2013 Panagiotis Vafiadis
  http://vafiadis-ultrasonic.org
 */
#include "gateway.h"

Gateway::Gateway(QObject *parent)
{
  manager = 0;
  status = SENSORAS_UKNOWN;
  connect(&timeGuard, SIGNAL(timeout()), this, SLOT(timeout()) );
}

Gateway::SENSORAS_STATUS Gateway::startRequest(QUrl url, Parameters params, int timeout)
{
       if(manager == 0)  manager = new QNetworkAccessManager();

        QNetworkRequest postRequest = QNetworkRequest(url);        
        QMapIterator<QString, QString> iter(params);
        QUrl query;

        while(iter.hasNext())
         {
            iter.next();
            query.addEncodedQueryItem(QUrl::toPercentEncoding(iter.key()), QUrl::toPercentEncoding(iter.value()));
         }

        connect(manager, SIGNAL( finished(QNetworkReply *) ),
                   this,   SLOT( finished(QNetworkReply *)) );

        status = SENSORAS_SUCCESS;
        timeGuard.start(timeout);

        reply = manager->post(postRequest, query.encodedQuery() );

        connect(reply, SIGNAL( error(QNetworkReply::NetworkError) ),
                this,    SLOT( error(QNetworkReply::NetworkError) ) );

        //eLoop.exec(QEventLoop::ExcludeUserInputEvents);
        eLoop.exec();
        manager->deleteLater();
        manager = 0;
        reply->deleteLater();
        reply = 0;

        return status;
}


void Gateway::finished(QNetworkReply * r)
{
  timeGuard.stop();
  eLoop.quit();
}

void Gateway::error(QNetworkReply::NetworkError err)
{
  timeGuard.stop();
  status = SENSORAS_FAILED;
  eLoop.quit();
}

void Gateway::timeout()
{
  status = SENSORAS_TIMEOUT;
  eLoop.quit();
}
