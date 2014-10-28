/*
  Kavala Institute of Technology
  Copyright (C) 2012-2013 Panagiotis Vafiadis
  http://vafiadis-ultrasonic.org
 */
#include <QtGui>
#include "generic.h"
#include "window.h"
#include "gateway.h"
#include "serialdeviceenumerator.h"
#include <QFile>
#include "infopacket.h"
#include "modem.h"

Window::Window()
{
    comPortComboBox = 0;
    numLogLines = 0;

    settings = new QSettings("kavala", "DataHub", this);

    this->devEnum = SerialDeviceEnumerator::instance();
    connect(this->devEnum, SIGNAL(hasChanged(QStringList)),
            this, SLOT(populateComPorts(QStringList)));

    createIconGroupBox();
    createMessageGroupBox();
    createActions();
    createTrayIcon();

    connect(showMessageButton, SIGNAL(clicked()), this, SLOT(showMessage()));
    connect(showIconCheckBox, SIGNAL(toggled(bool)),
            trayIcon, SLOT(setVisible(bool)));    
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(gsmModemGroupBox);
    mainLayout->addWidget(serverSettingsGroupBox);
    mainLayout->addWidget(loggingGroupBox);

    setLayout(mainLayout);

    monitor = new Monitor();
    const int CHECK_CYCLE = 28000;
    monitor->setCheckInterval(CHECK_CYCLE);
    connect(monitor, SIGNAL(logMessage(QString)), this, SLOT(addLog(QString)));

    QIcon icon = QIcon(":/images/samovar.ico");
    setWindowIcon(icon);
    trayIcon->setIcon(icon);
    trayIcon->show();

    setWindowTitle(APP_NAME);
    resize(400, 300);
    this->layout()->setSizeConstraint( QLayout::SetFixedSize );
}

void Window::connectSlot()
{
   connectButton->setEnabled(false);
   disconnectButton->setEnabled(true);

   QString port = comPorts.at(comPortComboBox->currentIndex());

   monitor->setGateway( QUrl(serverURLEdit->text()),
                              usernameEdit->text(),
                          serverDevKeyEdit->text()   );

   if( monitor->openSession(port) )          
      monitor->start();    
   else
   {       
       connectButton->setEnabled(true);
       disconnectButton->setEnabled(false);
   }
}

void Window::disconnectSlot()
{
    monitor->closeSession();
    addLog("Disconnected!");
    connectButton->setEnabled(true);
    disconnectButton->setEnabled(false);
}

void Window::setVisible(bool visible)
{
    minimizeAction->setEnabled(visible);
    maximizeAction->setEnabled(!isMaximized());
    restoreAction->setEnabled(isMaximized() || !visible);
    QDialog::setVisible(visible);
}

void Window::saveSettings()
{
  if(settings)
    {      
      settings->setValue("username", usernameEdit->text());
      settings->setValue("serverUrl", serverURLEdit->text());
      settings->setValue("serverKey", serverDevKeyEdit->text());
    }
}

void Window::closeEvent(QCloseEvent *event)
{
    saveSettings();

    if(!disconnectButton->isEnabled())
    {
      monitor->closeSession();
      qApp->quit();
    }

    if (trayIcon->isVisible()) {
        QMessageBox::information(this, APP_NAME,
                                 tr("The program will keep running in the "
                                    "system tray. To terminate the program, "
                                    "choose <b>Quit</b> in the context menu "
                                    "of the system tray entry."));
        hide();
        event->ignore();
    }
}


void Window::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        iconComboBox->setCurrentIndex((iconComboBox->currentIndex() + 1)
                                      % iconComboBox->count());
        break;
    case QSystemTrayIcon::MiddleClick:
        showMessage();
        break;
    default:
        ;
    }
}

void Window::showMessage()
{
    QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(
            typeComboBox->itemData(typeComboBox->currentIndex()).toInt());
    trayIcon->showMessage(titleEdit->text(), bodyEdit->toPlainText(), icon,
                          durationSpinBox->value() * 1000);
}

void Window::populateComPorts(QStringList list)
{
   if(comPortComboBox == 0) return;

   comPortComboBox->clear();
   comPorts.clear();

   QStringList devList;

   if(list.empty())
       devList = this->devEnum->devicesAvailable();
   else
       devList = list;

  // QFile file("d://enum.txt");
   //file.open(QIODevice::WriteOnly | QIODevice::Text);
   //QTextStream out(&file);

   //out << "\n ===> All devices: \n";

    foreach (QString s, devList) {
      this->devEnum->setDeviceName(s);
      comPortComboBox->addItem(this->devEnum->name() + " (" + this->devEnum->friendlyName() + ")" );

      comPorts.append(this->devEnum->shortName());

      /*
      out << "\n <<< info about: " << this->devEnum->name() << " >>>";
             out << "\n-> description  : " << this->devEnum->description();
             out << "\n-> driver       : " << this->devEnum->driver();
             out << "\n-> friendlyName : " << this->devEnum->friendlyName();
             out << "\n-> hardwareID   : " << this->devEnum->hardwareID().join(",");
             out << "\n-> locationInfo : " << this->devEnum->locationInfo();
             out << "\n-> manufacturer : " << this->devEnum->manufacturer();
             out << "\n-> productID    : " << this->devEnum->productID();
             out << "\n-> service      : " << this->devEnum->service();
             out << "\n-> shortName    : " << this->devEnum->shortName();
             out << "\n-> subSystem    : " << this->devEnum->subSystem();
             out << "\n-> systemPath   : " << this->devEnum->systemPath();
             out << "\n-> vendorID     : " << this->devEnum->vendorID();
             out << "\n-> revision     : " << this->devEnum->revision();
             out << "\n-> bus          : " << this->devEnum->bus();
             out << "\n-> is exists    : " << this->devEnum->isExists();
             out << "\n-> is busy      : " << this->devEnum->isBusy();
       */
         }

          //file.close();


}

void Window::createIconGroupBox()
{
    gsmModemGroupBox = new QGroupBox(tr("GSM Modem"));
    serverSettingsGroupBox = new QGroupBox(tr("Server Settings"));
    loggingGroupBox = new QGroupBox(tr("Logging"));

    QGridLayout * serverSettingsLayout = new QGridLayout;

    QLabel * usernameLabel = new QLabel(tr("Username:"));
    QLabel * serverURLLabel = new QLabel(tr("URL:"));
    QLabel * serverDevKeyLabel = new QLabel(tr("Key:"));

    serverURLEdit = new QLineEdit();
    serverDevKeyEdit = new QLineEdit();
    usernameEdit  =  new QLineEdit();

    usernameEdit->setText(settings->value("username").toString());
    serverURLEdit->setText(settings->value("serverUrl").toString());
    serverDevKeyEdit->setText(settings->value("serverKey").toString());

    QVBoxLayout * logVBoxLayout = new QVBoxLayout;
    loggingTextEdit = new QTextEdit();
    loggingTextEdit->setReadOnly(true);
    logVBoxLayout->addWidget(loggingTextEdit);
    loggingGroupBox->setLayout(logVBoxLayout);

    serverSettingsLayout->addWidget(usernameLabel, 0, 0);
    serverSettingsLayout->addWidget(usernameEdit, 0, 1);

    serverSettingsLayout->addWidget(serverURLLabel, 1, 0);
    serverSettingsLayout->addWidget(serverURLEdit, 1, 1);

    serverSettingsLayout->addWidget(serverDevKeyLabel, 2, 0);
    serverSettingsLayout->addWidget(serverDevKeyEdit, 2, 1);

    serverSettingsGroupBox->setLayout(serverSettingsLayout);

    comPortLabel = new QLabel("COM PORT:");
    comPortComboBox = new QComboBox;

    populateComPorts(this->devEnum->devicesAvailable());

    connectButton = new QPushButton(tr("Connect..."));
    disconnectButton = new QPushButton(tr("Disconnect"));
    disconnectButton->setEnabled(false);

    connect(connectButton, SIGNAL(clicked()), this, SLOT(connectSlot()));
    connect(disconnectButton, SIGNAL(clicked()), this, SLOT(disconnectSlot()));

    iconComboBox = new QComboBox;
    iconComboBox->addItem(QIcon(":/images/bad.svg"), tr("Bad"));
    iconComboBox->addItem(QIcon(":/images/samovar.ico"), tr("Heart"));
    iconComboBox->addItem(QIcon(":/images/trash.svg"), tr("Trash"));

    showIconCheckBox = new QCheckBox(tr("Show icon"));
    showIconCheckBox->setChecked(true);

    QHBoxLayout *iconLayout = new QHBoxLayout;
    iconLayout->addWidget(comPortLabel);
    iconLayout->addWidget(comPortComboBox);
    iconLayout->addStretch();
    iconLayout->addWidget(connectButton);
    iconLayout->addWidget(disconnectButton);
    gsmModemGroupBox->setLayout(iconLayout);
}

void Window::createMessageGroupBox()
{
    messageGroupBox = new QGroupBox(tr("Balloon Message"));

    typeLabel = new QLabel(tr("Type:"));

    typeComboBox = new QComboBox;
    typeComboBox->addItem(tr("None"), QSystemTrayIcon::NoIcon);
    typeComboBox->addItem(style()->standardIcon(
            QStyle::SP_MessageBoxInformation), tr("Information"),
            QSystemTrayIcon::Information);
    typeComboBox->addItem(style()->standardIcon(
            QStyle::SP_MessageBoxWarning), tr("Warning"),
            QSystemTrayIcon::Warning);
    typeComboBox->addItem(style()->standardIcon(
            QStyle::SP_MessageBoxCritical), tr("Critical"),
            QSystemTrayIcon::Critical);
    typeComboBox->setCurrentIndex(1);

    durationLabel = new QLabel(tr("Duration:"));

    durationSpinBox = new QSpinBox;
    durationSpinBox->setRange(5, 60);
    durationSpinBox->setSuffix(" s");
    durationSpinBox->setValue(15);

    durationWarningLabel = new QLabel(tr("(some systems might ignore this "
                                         "hint)"));
    durationWarningLabel->setIndent(10);

    titleLabel = new QLabel(tr("Title:"));

    titleEdit = new QLineEdit(tr("Cannot connect to network"));

    bodyLabel = new QLabel(tr("Body:"));

    bodyEdit = new QTextEdit;
    bodyEdit->setPlainText(tr("Don't believe me. Honestly, I don't have a "
                              "clue.\nClick this balloon for details."));

    showMessageButton = new QPushButton(tr("Show Message"));
    showMessageButton->setDefault(true);

    tmpMsg = new QPushButton("DEVELOPER");


    QGridLayout *messageLayout = new QGridLayout;
    messageLayout->addWidget(typeLabel, 0, 0);
    messageLayout->addWidget(tmpMsg, 0, 0);
    messageLayout->addWidget(typeComboBox, 0, 1, 1, 2);
    messageLayout->addWidget(durationLabel, 1, 0);
    messageLayout->addWidget(durationSpinBox, 1, 1);
    messageLayout->addWidget(durationWarningLabel, 1, 2, 1, 3);
    messageLayout->addWidget(titleLabel, 2, 0);
    messageLayout->addWidget(titleEdit, 2, 1, 1, 4);
    messageLayout->addWidget(bodyLabel, 3, 0);
    messageLayout->addWidget(bodyEdit, 3, 1, 2, 4);
    messageLayout->addWidget(showMessageButton, 5, 4);
    messageLayout->setColumnStretch(3, 1);
    messageLayout->setRowStretch(4, 1);
    messageGroupBox->setLayout(messageLayout);
}


void Window::createActions()
{
    minimizeAction = new QAction(tr("Mi&nimize"), this);
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

    maximizeAction = new QAction(tr("Ma&ximize"), this);
    connect(maximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));

    restoreAction = new QAction(tr("&Restore"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}

void Window::addLog(QString msg)
{
   if( numLogLines >= MAX_LOG_LINES)
    {
      numLogLines = 0;
      loggingTextEdit->clear();
    }

   QDateTime now = QDateTime::currentDateTime();
   ++numLogLines;
   QString msgLogDate = now.toString("dd-MMM-yyyy, hh:mm:ss");
   QString msgLog = "[<b>" + msgLogDate +"</b>] " + msg;
   loggingTextEdit->append(msgLog);
   loggingTextEdit->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);

   QFile file(LOG_FILE);
   file.open(QIODevice::Append | QIODevice::Text);
   QTextStream out(&file);
   out << "[" << msgLogDate << "] " << msg << "\n";
   file.close();
}

void Window::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(minimizeAction);    
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
}
