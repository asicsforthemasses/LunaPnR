#pragma once
#include <QWizard>
#include <QString>
#include <QProgressBar>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <iostream>
#include <string>
#include <vector>
#include "downloader.h"

namespace PDKInstall
{

    struct UrlWithDir
    {
        std::string m_url;
        std::string m_installdir;
    };

    struct ExtractInfo
    {
        std::string m_filename;
        std::string m_compressor;   ///< "zip" or "tgz"
    };

    struct Info
    {
        std::string m_title;
        std::string m_name;
        std::string m_version;
        std::string m_date;
        std::string m_copyright;
        std::string m_description;
        std::string m_layerfile;
        std::string m_installdir;
        std::vector<UrlWithDir> m_url;
        std::vector<ExtractInfo> m_extract;
        std::vector<std::string> m_lefs;
        std::vector<std::string> m_libs;
    };

    struct ReadResult
    {
        std::optional<Info> m_info;
        std::string m_errortxt;
    };

    ReadResult readInstallFile(const std::string &tomlFilename);
    ReadResult readInstallFile(std::istream &toml);

};

namespace GUI
{

class PDKInstallDialog : public QDialog
{
    Q_OBJECT
public:
    PDKInstallDialog(const std::filesystem::path &PDKRoot, QWidget *parent = nullptr);

private slots:
    void onOpen();
    void onInstall();
    void onProgress(int percent);
    void onDownloaded(QString filename);

protected:
    void updatePDKDisplay();

    QProgressBar    *m_progress{nullptr};
    QLineEdit       *m_installDirDisplay{nullptr};
    QLineEdit       *m_pdkTitle{nullptr};
    QLineEdit       *m_pdkVersion{nullptr};
    QPlainTextEdit  *m_pdkDescription{nullptr};

    std::optional<PDKInstall::Info> m_installInfo;
    std::filesystem::path m_installpath;
    std::filesystem::path m_pdkroot;
    Downloader  m_downloader;
};

};
