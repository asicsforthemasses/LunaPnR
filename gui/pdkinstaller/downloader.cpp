// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <cassert>
#include "downloader.h"

namespace GUI
{

Downloader::Downloader(QObject *parent) : QObject(parent)
{
    m_webCtrl.setTransferTimeout(30000);

    connect(&m_webCtrl, &QNetworkAccessManager::finished,
        this, &Downloader::onFileDownloaded);
}

void Downloader::download(QUrl url, QString filename)
{
    m_queue.push_back(DownloadItem{.m_url = url, .m_filename = filename});

    // trigger a download is none is in progress.
    if (!busy())
    {
        startDownload(m_queue.front());
    }
}

void Downloader::startDownload(const DownloadItem &item)
{
    assert(m_reply == nullptr);

    auto &dlItem = m_queue.front();

    m_currentFile.setFileName(dlItem.m_filename);

    if (!m_currentFile.open(QIODevice::WriteOnly))
    {
        QString err = "Cannot open ";
        err.append(dlItem.m_filename);
        err.append(" for writing");
        emit error(err);

        m_queue.pop_front();
        return;
    }

    QNetworkRequest request(item.m_url);
    m_reply = m_webCtrl.get(request);

    connect(m_reply, &QNetworkReply::downloadProgress, this,
        &Downloader::onDownloadProgress);

    connect(m_reply, &QNetworkReply::readyRead, this, &Downloader::onReadReady);
}

void Downloader::onDownloadProgress(int64_t bytes, int64_t total)
{
    if ((bytes >= 0) && (total > 0))
    {
        emit downloadProgress(static_cast<int>(bytes * 100 / total));
    }
}

void Downloader::onReadReady()
{
    m_currentFile.write(m_reply->readAll());
}

void Downloader::onFileDownloaded(QNetworkReply *reply)
{
    auto dlItem = m_queue.front();
    m_queue.pop_front();

    if (m_currentFile.isOpen())
    {
        m_currentFile.close();
    }

    if (reply->error() == QNetworkReply::NetworkError::NoError)
    {
        emit downloaded(dlItem.m_filename);
    }
    else
    {
        emit error(reply->errorString());
    }

    reply->deleteLater();
    m_reply = nullptr;

    if (!m_queue.empty())
    {
        startDownload(m_queue.front());
    }
}

};
