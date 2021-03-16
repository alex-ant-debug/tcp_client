#ifndef MYCLIENT_H
#define MYCLIENT_H

#include <QTcpSocket>
#include <QObject>
#include <QTime>
#include <QTimer>
#include <QDir>
#include <QTextStream>
#include <QDataStream>
#include <QEventLoop>

class MyClient : public QObject
{
    Q_OBJECT

public:
    explicit MyClient(QObject * obj = 0);
    bool checkSettings(void);
    inline void delay(int millisecondsWait);
    ~MyClient();

private:
    QTcpSocket* m_pTcpSocket;
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
