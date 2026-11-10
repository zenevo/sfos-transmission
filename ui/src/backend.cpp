#include "backend.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QDir>
#include <QTimer>
#include <QProcessEnvironment>

Backend::Backend(QObject *parent) : QObject(parent) {}

void Backend::updateStatus(bool running, const QString &version, int count,
                           const QString &down, const QString &up, const QString &err)
{
    m_running = running;
    m_version = version;
    m_torrentCount = count;
    m_downSpeed = down;
    m_upSpeed = up;
    m_lastError = err;
    emit statusChanged();
}

void Backend::refresh()
{
    queryIsActive();
    queryRpc();
}

void Backend::queryIsActive()
{
    QProcess *p = new QProcess(this);
    connect(p, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this,
        [this, p](int code, QProcess::ExitStatus) {
            const bool active = (code == 0);
            p->deleteLater();
            if (m_running != active)
                updateStatus(active, m_version, m_torrentCount, m_downSpeed, m_upSpeed, m_lastError);
        });
    p->start(QStringLiteral("systemctl"), { QStringLiteral("--user"), QStringLiteral("is-active"),
                                            QStringLiteral("transmission-daemon") });
}

static QString formatSpeed(qint64 bps)
{
    if (bps < 1024)
        return QStringLiteral("%1 B/s").arg(bps);
    if (bps < 1024 * 1024)
        return QStringLiteral("%1 kB/s").arg(bps / 1024.0, 0, 'f', 1);
    return QStringLiteral("%1 MB/s").arg(bps / (1024.0 * 1024.0), 0, 'f', 2);
}

void Backend::queryRpc()
{
    QNetworkRequest req(QUrl(QStringLiteral("http://127.0.0.1:9091/transmission/rpc")));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    if (!m_sessionId.isEmpty())
        req.setRawHeader("X-Transmission-Session-Id", m_sessionId);

    const QByteArray body = QByteArrayLiteral("{\"method\":\"session-stats\"}");
    QNetworkReply *reply = m_nam.post(req, body);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        const int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (code == 409) {
            m_sessionId = reply->rawHeader("X-Transmission-Session-Id");
            return;
        }
        if (reply->error() != QNetworkReply::NoError) {
            updateStatus(false, m_version, m_torrentCount, m_downSpeed, m_upSpeed,
                         reply->errorString());
            return;
        }
        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        const QJsonObject args = doc.object().value(QStringLiteral("arguments")).toObject();
        const QString version = args.value(QStringLiteral("version")).toString();
        const int count = args.value(QStringLiteral("torrentCount")).toInt();
        const qint64 down = static_cast<qint64>(args.value(QStringLiteral("downloadSpeed")).toDouble());
        const qint64 up = static_cast<qint64>(args.value(QStringLiteral("uploadSpeed")).toDouble());
        updateStatus(true, version, count, formatSpeed(down), formatSpeed(up), QString());
    });
}

void Backend::setRunning(bool run)
{
    if (!run) {
        // Stop through the RPC (works inside the sailjail sandbox)
        QNetworkRequest req(QUrl(QStringLiteral("http://127.0.0.1:9091/transmission/rpc")));
        req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
        if (!m_sessionId.isEmpty())
            req.setRawHeader("X-Transmission-Session-Id", m_sessionId);
        QNetworkReply *reply = m_nam.post(req, QByteArrayLiteral("{\"method\":\"session-close\"}"));
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            reply->deleteLater();
            const int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (code == 409) {
                m_sessionId = reply->rawHeader("X-Transmission-Session-Id");
                setRunning(false);
                return;
            }
            updateStatus(false, m_version, 0, QString(), QString(), QString());
        });
        return;
    }

    // Start: try the systemd user service first (with an explicit session environment)
    QProcess *p = new QProcess(this);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (!env.contains(QStringLiteral("XDG_RUNTIME_DIR")))
        env.insert(QStringLiteral("XDG_RUNTIME_DIR"), QStringLiteral("/run/user/100000"));
    if (!env.contains(QStringLiteral("DBUS_SESSION_BUS_ADDRESS")))
        env.insert(QStringLiteral("DBUS_SESSION_BUS_ADDRESS"),
                   QStringLiteral("unix:path=/run/user/100000/dbus/user_bus_socket"));
    p->setProcessEnvironment(env);
    connect(p, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this,
        [this, p, run](int code, QProcess::ExitStatus) {
            p->deleteLater();
            if (code == 0) {
                m_running = run;
                emit statusChanged();
                refresh();
                return;
            }
            // Fallback: spawn the daemon directly as the current user
            const QString cfg = QDir::homePath() + QStringLiteral("/.config/transmission-daemon");
            QProcess::startDetached(QStringLiteral("transmission-daemon"),
                { QStringLiteral("-g"), cfg,
                  QStringLiteral("-w"), QDir::homePath() + QStringLiteral("/Downloads") });
            QTimer::singleShot(1500, this, [this]() { refresh(); });
        });
    p->start(QStringLiteral("systemctl"), { QStringLiteral("--user"),
                                            QStringLiteral("start"),
                                            QStringLiteral("transmission-daemon") });
}
