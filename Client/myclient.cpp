#include "myclient.h"
#include <QFile>

MyClient::MyClient(QObject * obj):
    QObject(obj),
    nextBlockSize(0)
{
    readSettings();
    if(!isSettingsCorrect)
    {
        return;
    }
    tcpSocket = new QTcpSocket(this);

    tcpSocket->connectToHost(host, port);
    connect(tcpSocket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(tcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotError(QAbstractSocket::SocketError)));
}

void MyClient::slotReadyRead()
{
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_5_9);
    for (;;) {
        if (!nextBlockSize) {
            if (tcpSocket->bytesAvailable() < sizeof(quint32)) {
                break;
            }
            in >> nextBlockSize;
        }

        if (tcpSocket->bytesAvailable() < nextBlockSize) {
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
            writeToLog(time.toString() + " " + "Incremented value is received");
            QFile *myBinaryFile = new QFile(QDir::currentPath() + "/dataFile.dat");
            myBinaryFile->open(QIODevice::WriteOnly);
            QDataStream binaryDataStream(myBinaryFile);
            binaryDataStream.setVersion(QDataStream::Qt_5_9);

            binaryDataStream << incrementedValue;

            myBinaryFile->close();
            myBinaryFile->deleteLater();
            delete myBinaryFile;

            tcpSocket->disconnectFromHost();

            QString closedConnection = time.toString() + " " + "Disconnect from host";
            writeToLog(closedConnection);
        }
        else if(messageType == "error")
        {
            QString messageText;
            QString logMessage;
            int error;
            in >> error>>messageText;
            logMessage = time.toString() + " Error " + QString::number(error) + ": " + messageText;

            writeToLog(logMessage);
        }
        else
        {
            QString logMessage;
            logMessage = time.toString() + " Error unknown message type";

            writeToLog(logMessage);
        }

        nextBlockSize = 0;
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
                     QString(tcpSocket->errorString())
                    );

    writeToLog(strError);
}

void MyClient::slotSendToServer()
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_9);

    out << quint32(0) << protocolVersion << QTime::currentTime() << valueToSend;

    out.device()->seek(0);
    out << quint32(arrBlock.size() - sizeof(quint32));

    tcpSocket->write(arrBlock);
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
    tcpSocket->disconnectFromHost();
    tcpSocket->close();
    delete tcpSocket;

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
