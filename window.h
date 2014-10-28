/*
  Kavala Institute of Technology
  Copyright (C) 2012-2013 Panagiotis Vafiadis
  http://vafiadis-ultrasonic.org
 */
#ifndef WINDOW_H
#define WINDOW_H

#include <QSystemTrayIcon>
#include <QDialog>
#include <QSettings>
#include "serialdeviceenumerator.h"
#include "monitor.h"

QT_BEGIN_NAMESPACE
class QAction;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QPushButton;
class QSpinBox;
class QTextEdit;
QT_END_NAMESPACE


class Window : public QDialog
{
    Q_OBJECT

public:
    Window();

    void setVisible(bool visible);

protected:
    void closeEvent(QCloseEvent *event);

public slots:    
    void addLog(QString);

private slots:
    void connectSlot();
    void disconnectSlot();
    void populateComPorts(QStringList list);
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void showMessage();       

private:
    void restartModem();
    void createIconGroupBox();
    void createMessageGroupBox();
    void createActions();
    void createTrayIcon();
    void saveSettings();
    QString decodeSMS(QString sms);

private:
    SerialDeviceEnumerator *devEnum;
    Monitor * monitor;
    int numLogLines;

    QStringList   comPorts;
    QString       trayToolTip;
    QLabel      * comPortLabel;
    QComboBox   * comPortComboBox;
    QGroupBox   * gsmModemGroupBox;
    QPushButton * connectButton;
    QPushButton * disconnectButton;
    QGroupBox * serverSettingsGroupBox;
    QGroupBox * loggingGroupBox;
    QLineEdit * serverURLEdit;
    QLineEdit * serverDevKeyEdit;
    QLineEdit * usernameEdit;
    QTextEdit * loggingTextEdit;
    QSettings * settings;
    QComboBox * iconComboBox;
    QCheckBox * showIconCheckBox;
    QGroupBox * messageGroupBox;
    QLabel    * typeLabel;
    QLabel    * durationLabel;
    QLabel    * durationWarningLabel;
    QLabel    * titleLabel;
    QLabel    * bodyLabel;
    QComboBox * typeComboBox;
    QSpinBox  * durationSpinBox;
    QLineEdit * titleEdit;
    QTextEdit * bodyEdit;
    QPushButton * showMessageButton;
    QPushButton * tmpMsg;
    QAction     * minimizeAction;
    QAction     * maximizeAction;
    QAction     * restoreAction;
    QAction     * quitAction;
    QSystemTrayIcon * trayIcon;
    QMenu           * trayIconMenu;
};


#endif
