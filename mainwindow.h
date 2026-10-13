#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QStringList>
#include <QModelIndex>

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QLineEdit;
class QMenu;
class QFileSystemModel;
class QTableView;
class QToolBar;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    // ---- 阶段 2：导航相关槽函数 ----
    void navGoUp();
    void navGoBack();
    void navGoForward();
    void onRefresh();
    void onFileDoubleClicked(const QModelIndex &index);
    void onAddressReturnPressed();

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createCentralWidget();
    void createStatusBar();
    void createConnections();          // 阶段 2 新增：集中连接信号槽
    void updateActionEnableState();

    // 阶段 2 新增：统一的目录切换入口
    void changeDirectory(const QString &newPath, bool recordHistory = true);

    // ---------- QAction ----------
    QAction *actOpen        = nullptr;
    QAction *actNewArchive  = nullptr;
    QAction *actExtract     = nullptr;
    QAction *actTest        = nullptr;
    QAction *actExit        = nullptr;

    QAction *actUp          = nullptr;
    QAction *actRefresh     = nullptr;
    QAction *actBack        = nullptr;   // 阶段 2 新增
    QAction *actForward     = nullptr;   // 阶段 2 新增

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
    QLineEdit        *addressEdit     = nullptr;
    QTableView       *fileListView    = nullptr;
    QFileSystemModel *fileSystemModel = nullptr;   // 阶段 2：替换 QStandardItemModel

    // ---------- 状态栏 ----------
    QLabel *selectionLabel = nullptr;
    QLabel *diskLabel      = nullptr;

    // ---------- 阶段 2：导航状态 ----------
    QStringList m_navHistory;     // 导航历史栈
    int         m_navIndex = -1;  // 当前在历史栈的位置；-1 表示空历史
    QString     m_currentPath;    // 当前浏览路径
};

#endif // MAINWINDOW_H