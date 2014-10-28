/*
  Kavala Institute of Technology
  Copyright (C) 2012-2013 Panagiotis Vafiadis
  http://vafiadis-ultrasonic.org
 */
#ifndef INFOPACKET_H
#define INFOPACKET_H

#include <QObject>
#include <QString>
#include <QMap>

class InfoPacket : public QObject
{
    Q_OBJECT
public:
    explicit InfoPacket(QObject *parent = 0);

    void setSMS(const QString & sms);
    void extract();
    QString operator[](const QString & key);

   private:
    QString sms;
    QMap<QString, QString> fields;

signals:

public slots:

};

#endif // INFOPACKET_H
