#include "pdkinstaller.h"
#include "downloader.h"
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QDir>
#include <QPushButton>

#include <filesystem>
#include <toml++/toml.h>

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
        info.m_title    = tbl["title"].value_or("");
        info.m_name     = tbl["name"].value_or("");
        info.m_version  = tbl["version"].value_or("");
        info.m_description = tbl["description"].value_or("");
        info.m_date     = tbl["date"].value_or("");
        info.m_copyright = tbl["copyright"].value_or("");
        info.m_layerfile = tbl["layerfile"].value_or("");

        for(int idx = 0; idx < !!tbl["extract"]; idx++)
        {
            ExtractInfo extractInfo;
            extractInfo.m_filename = tbl["extract"][idx][0].value<std::string>().value();
            extractInfo.m_compressor = tbl["extract"][idx][1].value<std::string>().value();
            info.m_extract.emplace_back(extractInfo);
        }

        for(int idx = 0; idx < !!tbl["url"]; idx++)
        {
            UrlWithDir urlWithDir;
            urlWithDir.m_url = tbl["url"][idx][0].value<std::string>().value();
            urlWithDir.m_installdir = tbl["url"][idx][1].value<std::string>().value();
            info.m_url.emplace_back(urlWithDir);
        }

        for(int idx = 0; idx < !!tbl["lef"]; idx++)
        {
            auto lef = tbl["lef"][idx].value<std::string>().value();
            info.m_lefs.emplace_back(lef);
        }

        for(int idx = 0; idx < !!tbl["lib"]; idx++)
        {
            auto lib = tbl["lib"][idx].value<std::string>().value();
            info.m_libs.emplace_back(lib);
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

PDKInstallDialog::PDKInstallDialog(QWidget *parent) : QDialog(parent)
{
    const char* envHome = std::getenv("HOME");

    if(envHome != nullptr)
    {
        m_basepath = envHome;
        m_basepath /= "lunapnr";
    }
    else
    {
        //FIXME: make user configurable
        m_basepath = "/opt/lunapnr/pdks";
    }

    setWindowTitle("Install PDKs");

    auto layout = new QGridLayout();

    auto openButton = new QPushButton("Open");
    layout->addWidget(openButton);

    m_pdkTitle = new QLineEdit("");
    m_pdkTitle->setReadOnly(true);
    m_pdkDescription = new QPlainTextEdit();
    m_pdkDescription->setReadOnly(true);
    
    layout->addWidget(new QLabel("Title:"), 2, 0);
    layout->addWidget(m_pdkTitle, 2, 1);
    layout->addWidget(new QLabel("Description:"), 3, 0);
    layout->addWidget(m_pdkDescription, 3, 1);

    connect(openButton, &QPushButton::clicked, this, &PDKInstallDialog::onOpen);

    m_progress = new QProgressBar();
    m_progress->setEnabled(false);
    m_progress->setValue(0);
    m_progress->setMinimum(0);
    m_progress->setMaximum(100);

    layout->addWidget(m_progress, 4, 0, 1, 2);

    connect(&m_downloader, &Downloader::downloadProgress, this, &PDKInstallDialog::onProgress);
    connect(&m_downloader, &Downloader::downloaded, this, &PDKInstallDialog::onDownloaded);

    // OK / Cancel buttons
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    layout->addWidget(buttonBox);

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
    }
    else
    {
        m_pdkTitle->setText(QString::fromStdString(m_installInfo->m_title));
        m_pdkTitle->setEnabled(true);
        m_pdkDescription->setPlainText(QString::fromStdString(m_installInfo->m_description));
        m_pdkDescription->setEnabled(true);
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
        auto filepath = std::filesystem::absolute(m_basepath / urlAndDir.m_installdir);
        auto filedir  = filepath;
        filedir.remove_filename();

        std::cout << "url     : " << urlAndDir.m_url << "\n";
        std::cout << "dir     : " << urlAndDir.m_installdir << "\n";
        std::cout << "filepath: " << filepath << "\n";
        std::cout << "filedir : " << filedir << "\n";

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
}

void PDKInstallDialog::onProgress(int percent)
{
    m_progress->setValue(percent);
}

void PDKInstallDialog::onDownloaded(QString filename)
{
    std::cout << "Downloaded " << filename.toStdString() << "\n";

    if (m_downloader.empty())
    {
        m_progress->setTextVisible(true);
        m_progress->setFormat("Decompressing...");
        m_progress->update();

        for(auto const& extractInfo : m_installInfo->m_extract)
        {
            auto filepath = std::filesystem::absolute(m_basepath / extractInfo.m_filename);
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
                ss << "tar -xjvf " << filepath;
                std::system(ss.str().c_str());
            }            
            else
            {
                // unsupported compression
            }
            std::filesystem::current_path(oldPath);            
        }
        m_progress->setFormat("Done.");
        m_progress->setEnabled(false);
        m_progress->update();
    }
}

};
