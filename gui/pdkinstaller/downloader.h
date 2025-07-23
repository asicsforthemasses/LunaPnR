// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <deque>
#include <QObject>
#include <QByteArray>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStringList>

namespace GUI
{

class Downloader : public QObject
{
    Q_OBJECT
public:
    Downloader(QObject *parent = nullptr);
    virtual ~Downloader() = default;

    /** add a download to the queue */
    void download(QUrl url, QString filename);

    /** returns true if the download queue is empty */
    [[nodiscard]] bool empty() const noexcept
    {
        return m_queue.empty();
    }

signals:
    void downloaded(QString filename);
    void downloadProgress(int percent);
    void error(QString error);

private slots:
    void onReadReady();
    void onFileDownloaded(QNetworkReply *reply);
    void onDownloadProgress(int64_t bytes, int64_t total);

protected:
    constexpr bool busy() const noexcept
    {
        return m_reply != nullptr;
    }

    struct DownloadItem
    {
        QUrl    m_url;
        QString m_filename;
    };

    void startDownload(const DownloadItem &item);

    std::deque<DownloadItem> m_queue;

    QNetworkReply *m_reply{nullptr};
    QNetworkAccessManager m_webCtrl;

    QFile   m_currentFile;
};

};