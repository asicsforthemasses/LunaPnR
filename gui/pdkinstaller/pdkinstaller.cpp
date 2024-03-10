// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <filesystem>
#include <toml++/toml.h>
#include <strutilspp.hpp>

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QDir>
#include <QPushButton>

#include "pdkinstaller.h"
#include "downloader.h"
#include "common/pdkinfo.h"
#include "common/logging.h"
#include "common/imagemanip.h"

namespace PDKInstall
{

ReadResult readInstallFile(const std::string &tomlFilename)
{
    std::ifstream infile(tomlFilename);
    if (infile)
    {
        return readInstallFile(infile);
    }

    std::stringstream ss;
    ss << "Cannot open file " << tomlFilename;

    return ReadResult{.m_errortxt = ss.str()};
}

ReadResult readInstallFile(std::istream &toml)
{
    Info info;
    try
    {
        auto tbl = toml::parse(toml);
        info.m_installdir   = tbl["installdir"].value_or("");
        info.m_title        = tbl["title"].value_or("");
        info.m_name         = tbl["name"].value_or("");
        info.m_version      = tbl["version"].value_or("");
        info.m_description  = tbl["description"].value_or("");
        info.m_date         = tbl["date"].value_or("");
        info.m_copyright    = tbl["copyright"].value_or("");
        info.m_layerfile    = tbl["layerfile"].value_or("");
        info.m_icon         = tbl["icon"].value_or("");

        // sanity checks
        if (info.m_installdir.empty())
        {
            return ReadResult{.m_errortxt = "Missing installdir"};
        }

        std::filesystem::path installDir = info.m_installdir;
        if (!installDir.is_relative())
        {
            return ReadResult{.m_errortxt = "Specified installdir is not a relative path"};
        }

        if (info.m_title.empty())
        {
            return ReadResult{.m_errortxt = "Missing title"};
        }

        if (info.m_name.empty())
        {
            return ReadResult{.m_errortxt = "Missing name"};
        }

        auto extractArr = tbl["extract"].as_array();
        if (extractArr != nullptr)
        {
            extractArr->for_each(
                [&](toml::array& cmd)
                {
                    ExtractInfo extractInfo;
                    extractInfo.m_filename = cmd[0].value<std::string>().value();
                    extractInfo.m_compressor = cmd[1].value<std::string>().value();
                    info.m_extract.emplace_back(extractInfo);
                }
            );
        }

        auto urlArr = tbl["url"].as_array();
        if (urlArr != nullptr)
        {
            urlArr->for_each(
                [&](toml::array& url)
                {
                    UrlWithDir urlWithDir;
                    urlWithDir.m_url = url[0].value<std::string>().value();
                    urlWithDir.m_installdir = url[1].value<std::string>().value();
                    info.m_url.emplace_back(urlWithDir);
                }
            );
        }

        auto lefArr = tbl["lef"].as_array();
        if (lefArr != nullptr)
        {
            lefArr->for_each(
                [&](toml::value<std::string>& lef)
                {
                    info.m_lefs.emplace_back(lef);
                }
            );
        }

        auto libArr = tbl["lib"].as_array();
        if (libArr != nullptr)
        {
            libArr->for_each(
                [&](toml::value<std::string>& lib)
                {
                    info.m_libs.emplace_back(lib);
                }
            );
        }

        if (info.m_lefs.empty())
        {
            return ReadResult{.m_errortxt = "Missing lef files"};
        }

        if (info.m_libs.empty())
        {
            return ReadResult{.m_errortxt = "Missing lib files"};
        }

    }
    catch (const toml::parse_error& err)
    {
        std::stringstream ss;
        ss  << "Parsing error: " << err.description() << "\n"
            << "   (" << err.source().begin << ")\n";
        return ReadResult{.m_errortxt = ss.str()};
    }

    return ReadResult{.m_info = info};
}

};


namespace GUI
{

PDKInstallDialog::PDKInstallDialog(const std::filesystem::path &PDKRoot,
    QWidget *parent) : QDialog(parent)
{
    m_pdkroot = PDKRoot;

    if (!std::filesystem::exists(m_pdkroot))
    {
        std::filesystem::create_directory(m_pdkroot);
    }

    setWindowTitle("Install PDK");

    auto layout = new QGridLayout();

    auto openButton = new QPushButton("Open");
    layout->addWidget(openButton, 0, 0, 1, 2);

    QFrame* line1 = new QFrame();
    line1->setFrameShape(QFrame::HLine);
    line1->setFrameShadow(QFrame::Sunken);
    line1->setMinimumHeight(10);

    layout->addWidget(line1, 1, 0, 1, 2);

    m_pdkTitle = new QLineEdit("");
    m_pdkTitle->setReadOnly(true);
    m_pdkVersion = new QLineEdit("");
    m_pdkVersion->setReadOnly(true);
    m_pdkDescription = new QPlainTextEdit();
    m_pdkDescription->setReadOnly(true);
    m_installDirDisplay = new QLineEdit("");
    m_installDirDisplay->setReadOnly(true);

    layout->addWidget(new QLabel("Title:"), 2, 0);
    layout->addWidget(m_pdkTitle, 2, 1);
    layout->addWidget(new QLabel("Version:"), 3, 0);
    layout->addWidget(m_pdkVersion, 3, 1);
    layout->addWidget(new QLabel("Description:"), 4, 0);
    layout->addWidget(m_pdkDescription, 4, 1);
    layout->addWidget(new QLabel("Install path:"), 5, 0);
    layout->addWidget(m_installDirDisplay, 5, 1);

    QFrame* line2 = new QFrame();
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);
    line2->setMinimumHeight(10);
    layout->addWidget(line2, 6, 0, 1, 2);

    connect(openButton, &QPushButton::clicked, this, &PDKInstallDialog::onOpen);

    m_progress = new QProgressBar();
    m_progress->setEnabled(false);
    m_progress->setValue(0);
    m_progress->setMinimum(0);
    m_progress->setMaximum(100);

    layout->addWidget(m_progress, 7, 0, 1, 2);

    connect(&m_downloader, &Downloader::downloadProgress, this, &PDKInstallDialog::onProgress);
    connect(&m_downloader, &Downloader::downloaded, this, &PDKInstallDialog::onDownloaded);

    // OK / Cancel buttons
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    layout->addWidget(buttonBox, 8,0, 1,2);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    updatePDKDisplay();

    setLayout(layout);
};

void PDKInstallDialog::updatePDKDisplay()
{
    if (!m_installInfo)
    {
        m_pdkTitle->setText("");
        m_pdkTitle->setEnabled(false);
        m_pdkDescription->setPlainText("");
        m_pdkDescription->setEnabled(false);
        m_installDirDisplay->setText("");
        m_installDirDisplay->setEnabled(false);
        m_pdkVersion->setText("");
        m_pdkVersion->setEnabled(false);
    }
    else
    {
        m_pdkTitle->setText(QString::fromStdString(m_installInfo->m_title));
        m_pdkTitle->setEnabled(true);
        m_pdkDescription->setPlainText(QString::fromStdString(m_installInfo->m_description));
        m_pdkDescription->setEnabled(true);
        m_installDirDisplay->setText(QString::fromStdString(m_installpath));
        m_installDirDisplay->setEnabled(true);
        m_pdkVersion->setText(QString::fromStdString(m_installInfo->m_version));
        m_pdkVersion->setEnabled(true);
    }
}

void PDKInstallDialog::onOpen()
{
    auto filename = QFileDialog::getOpenFileName(this,
        tr("PDK setup file"), "", tr("Setup Files (*.toml)"));

    if (!filename.isEmpty())
    {
        auto result = PDKInstall::readInstallFile(filename.toStdString());
        if (result.m_info)
        {
            // ok
            m_installInfo = result.m_info;
            m_installpath = m_pdkroot / m_installInfo->m_installdir;

            updatePDKDisplay();
            std::cout << result.m_info->m_title << "\n";
            onInstall();
        }
        else
        {
            // fail
            m_installInfo.reset();
            updatePDKDisplay();
            std::cerr << result.m_errortxt << "\n";
        }
    }
}

void PDKInstallDialog::onInstall()
{
    if (!m_installInfo) return; // cannot install.

    m_progress->setEnabled(true);

    for(auto const& urlAndDir : m_installInfo->m_url)
    {
        auto filepath = std::filesystem::absolute(m_installpath / urlAndDir.m_installdir);
        auto filedir  = filepath;
        filedir.remove_filename();

#if 0
        std::cout << "url     : " << urlAndDir.m_url << "\n";
        std::cout << "dir     : " << urlAndDir.m_installdir << "\n";
        std::cout << "filepath: " << filepath << "\n";
        std::cout << "filedir : " << filedir << "\n";
#endif
        if (!std::filesystem::exists(filedir))
        {
            // make the directory if it doesn't exist
            if (!std::filesystem::create_directories(filedir))
            {
                // dir create error
                std::cerr << "Cannot create dir " << filedir << "\n";
                return;
            }
        }

        m_downloader.download(QUrl(QString::fromStdString(urlAndDir.m_url)), filepath.c_str());
    }

    // if there is a logo, download it
    if (!m_installInfo->m_icon.empty())
    {
        auto iconPath = std::filesystem::absolute(m_installpath / "_icon.png");
        m_downloader.download(QUrl(QString::fromStdString(m_installInfo->m_icon)), iconPath.c_str());
    }

    // write _pdkinfo.toml in the install dir
    PDKInfo pdkinfo;
    pdkinfo.m_title = m_installInfo->m_title;
    pdkinfo.m_name = m_installInfo->m_name;
    pdkinfo.m_copyright = m_installInfo->m_copyright;
    pdkinfo.m_date = m_installInfo->m_date;
    pdkinfo.m_description = m_installInfo->m_description;
    pdkinfo.m_lefs = m_installInfo->m_lefs;
    pdkinfo.m_libs = m_installInfo->m_libs;
    pdkinfo.m_layerfile = m_installInfo->m_layerfile;
    pdkinfo.m_version = m_installInfo->m_version;

    // write .toml file
    auto tomlPath = std::filesystem::absolute(m_installpath / "_pdkinfo.toml");
    std::ofstream ofile(tomlPath);
    if (!ofile)
    {
        return;
    }

    ofile << "# LunaPnR PDK info file\n";
    ofile << "# Installed on " << StrUtils::date() << "\n\n";
    ofile << toToml(pdkinfo);
}

void PDKInstallDialog::onProgress(int percent)
{
    if (percent >= 0)
    {
        m_progress->setValue(percent);
    }
}

void PDKInstallDialog::onDownloaded(QString filename)
{
    std::stringstream ss;
    ss << "Downloaded " << filename.toStdString() << "\n";
    Logging::doLog(Logging::LogType::INFO, ss.str());

    if (m_downloader.empty())
    {
        m_progress->setTextVisible(true);
        m_progress->setFormat("Decompressing...");
        m_progress->update();

        for(auto const& extractInfo : m_installInfo->m_extract)
        {
            auto filepath = std::filesystem::absolute(m_installpath / extractInfo.m_filename);
            auto filedir  = filepath;
            filedir.remove_filename();

            auto oldPath = std::filesystem::current_path();
            std::filesystem::current_path(filedir);

            // FIXME: platform specific
            //        and handle return code!
            if (extractInfo.m_compressor == "tgz")
            {
                std::stringstream ss;
                ss << "tar -xzf " << filepath;
                std::system(ss.str().c_str());
            }
            else if (extractInfo.m_compressor == "zip")
            {
                std::stringstream ss;
                ss << "unzip -q -o " << filepath;
                std::system(ss.str().c_str());
            }
            else if (extractInfo.m_compressor == "bz2")
            {
                std::stringstream ss;
                ss << "tar -xjf " << filepath;
                std::system(ss.str().c_str());
            }
            else if (extractInfo.m_compressor == "xz")
            {
                std::stringstream ss;
                ss << "tar -xJf " << filepath;
                std::system(ss.str().c_str());
            }
            else
            {
                // unsupported compression
            }
            std::filesystem::current_path(oldPath);
        }

        // check if there is an _icon.png
        // if there is, resize it to 64x64
        auto iconPath = std::filesystem::absolute(m_installpath / "_icon.png");
        if (std::filesystem::exists(iconPath))
        {
            auto iconImage = makePDKIcon(iconPath.string());
            if (iconImage)
            {
                iconImage->save(QString::fromStdString(iconPath.string()));
            }
        }

        m_progress->setFormat("Done.");
        m_progress->setEnabled(false);
        m_progress->update();
    }
}

};
