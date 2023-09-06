#include "pdkdialog.h"
#include <QBoxLayout>
#include "pdktile.h"

namespace GUI
{

PDKDialog::PDKDialog(std::vector<PDKInfo> &pdks) : m_pdks(pdks)
{
    setWindowTitle("Select PDK");
    
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    auto mainLayout = new QVBoxLayout();    
    m_tileList = new QScrollArea();
    m_tileList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    
    auto tileLayout = new QVBoxLayout();
    tileLayout->setSizeConstraint(QLayout::SetFixedSize);
    for(auto const &pdk : pdks)
    {
        auto tile = new PDKTile(pdk);
        tileLayout->addWidget(tile);
    }

    m_tileList->setLayout(tileLayout);
    mainLayout->addWidget(m_tileList);
    setLayout(mainLayout);
}

};
