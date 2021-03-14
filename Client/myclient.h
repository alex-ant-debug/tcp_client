#ifndef MYCLIENT_H
#define MYCLIENT_H

#include <QWidget>
#include <QTcpSocket>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QTime>
#include <QDir>
#include <QTextStream>

class QTextEdit;
class QLineEdit;

namespace Ui {
class MyClient;
}

class MyClient : public QWidget
{
    Q_OBJECT

public:
    explicit MyClient(QString settingsPath, QWidget *parent = 0);
    bool checkSettings(void);
    ~MyClient();

private:
    Ui::MyClient *ui;
    QTcpSocket* m_pTcpSocket;
    QTextEdit*  m_ptxtInfo;
    quint16     m_nNextBlockSize;
    bool readSettings(void);
    void writeToLog(QString str);

    bool isSettingsCorrect;
    QString host;
    int port;
    double valueToSend;
    const QString protocolVersion = "V1.0";

private slots:
    void slotReadyRead   (                            );
    void slotError       (QAbstractSocket::SocketError);
    void slotSendToServer(                            );
    void slotConnected   (                            );

};

#endif // MYCLIENT_H
