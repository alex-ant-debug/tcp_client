#include "myclient.h"
#include <QFile>

MyClient::MyClient(QObject * obj):
    QObject(obj),
    m_nNextBlockSize(0)
{
    readSettings();
    if(!isSettingsCorrect)
    {
        return;
    }
    m_pTcpSocket = new QTcpSocket(this);

    m_pTcpSocket->connectToHost(host, port);
    connect(m_pTcpSocket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(m_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotError(QAbstractSocket::SocketError)));
}

void MyClient::slotReadyRead()
{
    QDataStream in(m_pTcpSocket);
    in.setVersion(QDataStream::Qt_5_9);
    for (;;) {
        if (!m_nNextBlockSize) {
            if (m_pTcpSocket->bytesAvailable() < sizeof(quint16)) {
                break;
            }
            in >> m_nNextBlockSize;
        }

        if (m_pTcpSocket->bytesAvailable() < m_nNextBlockSize) {
            break;
        }
        QTime   time;
        QString messageType;
        in >> time >> messageType;

        if(messageType == "message")
        {
            QString messageText;
            in >> messageText;
            QString logMessage = time.toString() + " " + messageText;
            writeToLog(logMessage);
        }
        else if(messageType == "incremented_value")
        {
            QVector<double>  incrementedValue;
            in >> incrementedValue;
            QString incrementedValueString;
            for(int i = 0; i < incrementedValue.size(); i++)
            {
                incrementedValueString += " " + QString::number(incrementedValue.at(i));
            }

            writeToLog(time.toString() + " " + "Incremented value is received");
            QFile *myBinaryFile = new QFile(QDir::currentPath() + "/dataFile.dat");
            myBinaryFile->open(QIODevice::WriteOnly);
            QDataStream binaryDataStream(myBinaryFile);
            binaryDataStream.setVersion(QDataStream::Qt_5_9);

            binaryDataStream << incrementedValue;

            myBinaryFile->close();
            myBinaryFile->deleteLater();

            m_pTcpSocket->disconnectFromHost();

            QString closedConnection = time.toString() + " " + "Disconnect from host";
            writeToLog(closedConnection);
        }
        else if(messageType == "error")
        {
            QString messageText;
            QString logMessage;
            int error;
            in >> error>>messageText;
            logMessage = time.toString() + " " + "Error " + QString::number(error) + ": " + messageText;

            writeToLog(logMessage);
        }

        m_nNextBlockSize = 0;
    }
}

void MyClient::slotError(QAbstractSocket::SocketError err)
{
    QString strError =
        "Error: " + (err == QAbstractSocket::HostNotFoundError ?
                     "The host was not found." :
                     err == QAbstractSocket::RemoteHostClosedError ?
                     "The remote host is closed." :
                     err == QAbstractSocket::ConnectionRefusedError ?
                     "The connection was refused." :
                     QString(m_pTcpSocket->errorString())
                    );

    writeToLog(strError);
}

void MyClient::slotSendToServer()
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_9);

    out << quint16(0) << protocolVersion << QTime::currentTime() << valueToSend;

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    m_pTcpSocket->write(arrBlock);
}

void MyClient::slotConnected()
{
    delay(3000);
    slotSendToServer();
}

bool MyClient::readSettings(void)
{
    isSettingsCorrect = false;
    QFile file(QDir::currentPath() + "/settings");
    QStringList settings;

    if (!file.open(QIODevice::ReadOnly ))
    {
        writeToLog("No settings file");
        return false;
    }

    QTextStream stream(&file);
    QString line = stream.readLine();
    settings = line.split(":");
    stream.flush();
    file.close();

    host = settings.at(0);
    port = settings.at(1).toInt();
    valueToSend = settings.at(2).toDouble();

    if (host.isEmpty())
    {
        writeToLog("Host is empty");
        return false;
    }

    if (port <= 0)
    {
        writeToLog("Port has wrong value");
        return false;
    }

    isSettingsCorrect = true;
    return true;
}

bool MyClient::checkSettings(void)
{
    return isSettingsCorrect;
}

MyClient::~MyClient()
{
    m_pTcpSocket->close();
    delete m_pTcpSocket;
}

void MyClient::writeToLog(QString str)
{
    QFile file(QDir::currentPath() + "/log.txt");
    file.open(QIODevice:: Append | QIODevice::Text);
    QTextStream out(&file);
    out << str<<endl;
    file.close();
}

inline void MyClient::delay(int millisecondsWait)
{
    QEventLoop loop;
    QTimer t;
    t.connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
    t.start(millisecondsWait);
    loop.exec();
}
