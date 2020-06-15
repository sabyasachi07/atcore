#pragma once
#include <QSslSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>
#include <QObject>
#include <QSslKey>
#include <QSslCertificate>
#include <QFile>
#include <QSslSocket>
#include "atcore_export.h"


class QSslSocket;
class QFile;

class  ATCORE_EXPORT ServerPart :public QTcpServer
{
    Q_OBJECT

public:
  ServerPart(QObject *parent = nullptr);


signals :
    void gotNewCommand(QString str);

public slots:
    void sslErrors(const QList<QSslError> &errors);
     //void startserver();
     virtual void newConnection();
    void readClient(QTcpSocket* m_clientSocket);
    void disconnect(QTcpSocket* m_clientSocket);

private:
    quint16 m_nNextBlockSize;
    QSslKey key;
    QSslCertificate cert;

protected:
    void incomingConnection(qintptr sslSocketDescriptor);
};
