#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QTcpServer>
#include <QVector>
class QThread;
class Worker;
class Server : public QTcpServer
{
    Q_OBJECT
    Q_DISABLE_COPY(Server)
public:
    explicit Server(QObject *parent = nullptr);
protected:
    void incomingConnection(qintptr socketDescriptor) override;
signals:
    void logMessage(const QString &msg);
public slots:
    void stopServer();
private slots:
    void broadcast(const QJsonObject &message, Worker *exclude);
    void jsonReceived(Worker *sender, const QJsonObject &doc);
    void userDisconnected(Worker *sender);
    void userError(Worker *sender);
private:
    void jsonFromLoggedOut(Worker *sender, const QJsonObject &doc);
    void jsonFromLoggedIn(Worker *sender, const QJsonObject &doc);
    void sendJson(Worker *destination, const QJsonObject &message);
    QVector<Worker *> m_clients;
};

#endif // CHATSERVER_H
