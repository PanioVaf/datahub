/*
  Kavala Institute of Technology
  Copyright (C) 2012-2013 Panagiotis Vafiadis
  http://vafiadis-ultrasonic.org
 */
#include "infopacket.h"

InfoPacket::InfoPacket(QObject *parent) :
    QObject(parent)
{
}


void InfoPacket::setSMS(const QString & sms)
{
  this->sms = sms;
}


void InfoPacket::extract()
{
  const QChar sep = ' ';
  QString sms = this->sms + sep;
  const int & length = sms.size();
  QString word;

  fields.clear();

  for(int i = 0; i < length; ++i)
    {
      const QChar & c = sms[i];

      if(c == sep)
      {
          if( !word.isEmpty() )
            {
               QString key;
               QString value;
               int j;

               for(j = 0; j < word.size(); ++j)
                  if( word[j] == ':' )
                           break;
                  else
                      key.append(word[j]);

               for(++j; j < word.size(); ++j)
                      value.append(word[j]);

               fields[key] = value;
               word.clear();
            }
        }
      else
         word.append(c);
    }
}

QString InfoPacket::operator[](const QString & key)
{
        return fields[key];
}
