#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QLineEdit;
class QMenu;
class QStandardItemModel;
class QTableView;
class QToolBar;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    // 把搭建过程拆成 5 个小函数，方便和子步骤一一对照
    void createActions();          // 子步骤 3 的前置：先把 QAction 造出来
    void createMenus();            // 子步骤 3：菜单栏
    void createToolBars();         // 子步骤 4：工具栏
    void createCentralWidget();    // 子步骤 2 + 5：中央部件（地址栏 + 文件列表占位）
    void createStatusBar();        // 子步骤 6：状态栏
    void updateActionEnableState();

    // ---------- QAction（动作对象，不是控件！）----------
    QAction *actOpen        = nullptr;
    QAction *actNewArchive  = nullptr;
    QAction *actExtract     = nullptr;
    QAction *actTest        = nullptr;
    QAction *actExit        = nullptr;

    QAction *actUp          = nullptr;
    QAction *actRefresh     = nullptr;

    QAction *actSelectAll   = nullptr;
    QAction *actInvertSel   = nullptr;
    QAction *actCopy        = nullptr;

    QAction *actOptions     = nullptr;
    QAction *actAbout       = nullptr;

    // ---------- 菜单 ----------
    QMenu *menuFile  = nullptr;
    QMenu *menuEdit  = nullptr;
    QMenu *menuView  = nullptr;
    QMenu *menuTools = nullptr;
    QMenu *menuHelp  = nullptr;

    // ---------- 工具栏 ----------
    QToolBar *mainToolBar = nullptr;

    // ---------- 中央部件内部控件 ----------
    QLineEdit          *addressEdit   = nullptr;
    QTableView         *fileListView  = nullptr;
    QStandardItemModel *fileModel     = nullptr;

    // ---------- 状态栏 ----------
    QLabel *selectionLabel = nullptr;   // 左侧：选中信息
    QLabel *diskLabel      = nullptr;   // 右侧：磁盘信息
};

#endif // MAINWINDOW_H