#pragma once
#include <list>
#include <QString>
#include "replwidget.hpp"

namespace GUI
{

struct LunaCommandCompleter : public ReplWidget::ICompleter
{
    std::list<QString> tryComplete(const QString &str);

    std::vector<QString> m_words;
};

};