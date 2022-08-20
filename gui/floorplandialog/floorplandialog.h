#pragma once

#include <QDialog>
#include <QGridLayout>
#include <QTableWidget>
#include "common/database.h"
#include "lunacore.h"

namespace GUI
{

class FloorplanDialog : public QDialog
{
    Q_OBJECT
public:
    FloorplanDialog(Database &db, QWidget *parent = nullptr);
    virtual ~FloorplanDialog() = default;

protected slots:
    void onAddRegionRow();

protected:
    void createTableRow(size_t row, const RegionSetup &region);
    
    Database &m_db;
    QTableWidget *m_regionTable;
};

};