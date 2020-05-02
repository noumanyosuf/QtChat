#include "worker.h"
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>

Worker::Worker(QObject *parent)
    : QObject(parent)
    , m_serverSocket(new QTcpSocket(this))
{
    connect(m_serverSocket, &QTcpSocket::readyRead, this, &Worker::receiveJson);
    connect(m_serverSocket, &QTcpSocket::disconnected, this, &Worker::disconnectedFromClient);
    connect(m_serverSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &Worker::error);
}

bool Worker::setSocketDescriptor(qintptr socketDescriptor)
{
    return m_serverSocket->setSocketDescriptor(socketDescriptor);
}

void Worker::sendJson(const QJsonObject &json)
{
    const QByteArray jsonData = QJsonDocument(json).toJson(QJsonDocument::Compact);
    emit logMessage("Sending to " + userName() + " - " + QString::fromUtf8(jsonData));
    QDataStream socketStream(m_serverSocket);
    socketStream.setVersion(QDataStream::Qt_5_7);
    socketStream << jsonData;
}

void Worker::disconnectFromClient()
{
    m_serverSocket->disconnectFromHost();
}

QString Worker::userName() const
{
    return m_userName;
}

void Worker::setUserName(const QString &userName)
{
    m_userName = userName;
}

void Worker::receiveJson()
{
    QByteArray jsonData;
    QDataStream socketStream(m_serverSocket);
    socketStream.setVersion(QDataStream::Qt_5_7);

    for (;;) {
        socketStream.startTransaction();
        socketStream >> jsonData;
        if (socketStream.commitTransaction()) {
            QJsonParseError parseError;
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
            if (parseError.error == QJsonParseError::NoError) {
                if (jsonDoc.isObject())
                    emit jsonReceived(jsonDoc.object());
                else
                    emit logMessage("Invalid message: " + QString::fromUtf8(jsonData));
            } else {
                emit logMessage("Invalid message: " + QString::fromUtf8(jsonData));
            }
        } else {
            break;
        }
    }
}


