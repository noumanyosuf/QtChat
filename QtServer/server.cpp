#include "server.h"
#include "worker.h"
#include <QThread>
#include <functional>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTimer>
Server::Server(QObject *parent)
    : QTcpServer(parent)
{}

void Server::incomingConnection(qintptr socketDescriptor)
{
    Worker *worker = new Worker(this);
    if (!worker->setSocketDescriptor(socketDescriptor)) {
        worker->deleteLater();
        return;
    }
    connect(worker, &Worker::disconnectedFromClient, this, std::bind(&Server::userDisconnected, this, worker));
    connect(worker, &Worker::error, this, std::bind(&Server::userError, this, worker));
    connect(worker, &Worker::jsonReceived, this, std::bind(&Server::jsonReceived, this, worker, std::placeholders::_1));
    connect(worker, &Worker::logMessage, this, &Server::logMessage);
    m_clients.append(worker);
    emit logMessage(QStringLiteral("New client Connected"));
}
void Server::sendJson(Worker *destination, const QJsonObject &message)
{
    Q_ASSERT(destination);
    destination->sendJson(message);
}
void Server::broadcast(const QJsonObject &message, Worker *exclude)
{
    for (Worker *worker : m_clients) {
        Q_ASSERT(worker);
        if (worker == exclude)
            continue;
        sendJson(worker, message);
    }
}

void Server::jsonReceived(Worker *sender, const QJsonObject &doc)
{
    Q_ASSERT(sender);
    emit logMessage("JSON received " + QString::fromUtf8(QJsonDocument(doc).toJson()));
    if (sender->userName().isEmpty())
        return jsonFromLoggedOut(sender, doc);
    jsonFromLoggedIn(sender, doc);
}

void Server::userDisconnected(Worker *sender)
{
    m_clients.removeAll(sender);
    const QString userName = sender->userName();
    if (!userName.isEmpty()) {
        QJsonObject disconnectedMessage;
        disconnectedMessage["type"] = QStringLiteral("userdisconnected");
        disconnectedMessage["username"] = userName;
        broadcast(disconnectedMessage, nullptr);
        emit logMessage(userName + " disconnected");
    }
    sender->deleteLater();
}

void Server::userError(Worker *sender)
{
    Q_UNUSED(sender)
    emit logMessage("Error from " + sender->userName());
}

void Server::stopServer()
{
    for (Worker *worker : m_clients) {
        worker->disconnectFromClient();
    }
    close();
}

void Server::jsonFromLoggedOut(Worker *sender, const QJsonObject &docObj)
{
    Q_ASSERT(sender);
    const QJsonValue typeVal = docObj.value(QLatin1String("type"));
    if (typeVal.isNull() || !typeVal.isString())
        return;
    if (typeVal.toString().compare(QLatin1String("login"), Qt::CaseInsensitive) != 0)
        return;
    const QJsonValue usernameVal = docObj.value(QLatin1String("username"));
    if (usernameVal.isNull() || !usernameVal.isString())
        return;
    const QString newUserName = usernameVal.toString().simplified();
    if (newUserName.isEmpty())
        return;
    for (Worker *worker : qAsConst(m_clients)) {
        if (worker == sender)
            continue;
        if (worker->userName().compare(newUserName, Qt::CaseInsensitive) == 0) {
            QJsonObject message;
            message["type"] = QStringLiteral("login");
            message["success"] = false;
            message["reason"] = QStringLiteral("duplicate username");
            sendJson(sender, message);
            return;
        }
    }
    sender->setUserName(newUserName);
    QJsonObject successMessage;
    successMessage["type"] = QStringLiteral("login");
    successMessage["success"] = true;
    sendJson(sender, successMessage);
    QJsonObject connectedMessage;
    connectedMessage["type"] = QStringLiteral("newuser");
    connectedMessage["username"] = newUserName;
    broadcast(connectedMessage, sender);
}

void Server::jsonFromLoggedIn(Worker *sender, const QJsonObject &docObj)
{
    Q_ASSERT(sender);
    const QJsonValue typeVal = docObj.value(QLatin1String("type"));
    if (typeVal.isNull() || !typeVal.isString())
        return;
    if (typeVal.toString().compare(QLatin1String("message"), Qt::CaseInsensitive) != 0)
        return;
    const QJsonValue textVal = docObj.value(QLatin1String("text"));
    if (textVal.isNull() || !textVal.isString())
        return;
    const QString text = textVal.toString().trimmed();
    if (text.isEmpty())
        return;
    QJsonObject message;
    message["type"] = QStringLiteral("message");
    message["text"] = text;
    message["sender"] = sender->userName();
    broadcast(message, sender);
}


