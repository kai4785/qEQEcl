#ifndef QEQECL_QTFILEDOWNLOAD_H
#define QEQECL_QTFILEDOWNLOAD_H
#include <QObject>
#include <QString>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>


class QtDownload : public QObject {
    Q_OBJECT
public:
    explicit QtDownload();
    ~QtDownload();
    QString url;
    QString file;

private:
    QNetworkAccessManager manager;

signals:
    void done();

public slots:
    void download();
    void downloadFinished(QNetworkReply* data);
    void downloadProgress(qint64 recieved, qint64 total);
};
#endif // QEQECL_QTFILEDOWNLOAD_H
