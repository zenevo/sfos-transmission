#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QProcess>

class Backend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ running NOTIFY statusChanged)
    Q_PROPERTY(QString version READ version NOTIFY statusChanged)
    Q_PROPERTY(int torrentCount READ torrentCount NOTIFY statusChanged)
    Q_PROPERTY(QString downSpeed READ downSpeed NOTIFY statusChanged)
    Q_PROPERTY(QString upSpeed READ upSpeed NOTIFY statusChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY statusChanged)
public:
    explicit Backend(QObject *parent = nullptr);

    bool running() const { return m_running; }
    QString version() const { return m_version; }
    int torrentCount() const { return m_torrentCount; }
    QString downSpeed() const { return m_downSpeed; }
    QString upSpeed() const { return m_upSpeed; }
    QString lastError() const { return m_lastError; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setRunning(bool run);

signals:
    void statusChanged();

private:
    void queryIsActive();
    void queryRpc();
    void updateStatus(bool running, const QString &version, int count,
                      const QString &down, const QString &up, const QString &err);

    QNetworkAccessManager m_nam;
    QByteArray m_sessionId;
    bool m_running = false;
    QString m_version;
    int m_torrentCount = 0;
    QString m_downSpeed;
    QString m_upSpeed;
    QString m_lastError;
};

#endif
