
#ifndef mainwindow_h
#define mainwindow_h

#include <vector>
#include <string>

#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QTableWidget>
#include <QSplitter>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();

public slots:
    void onQuit();
    void onAbout();
    
protected:
    QMenuBar    *m_menuBar;
    QSplitter   *m_splitter;
};

#endif