#include "mainwindow.h"

#include <QAction>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QTableView>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

// ============================================================
// 构造函数：整个窗口在这里被"搭"出来
// ============================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    createActions();        // 先造动作对象
    createMenus();          // 子步骤 3
    createToolBars();       // 子步骤 4
    createCentralWidget();  // 子步骤 2 + 5
    createStatusBar();      // 子步骤 6

    createConnections();          // 阶段 2 新增
    updateActionEnableState();    // 初始状态：导航按钮全禁用

    setWindowTitle(tr("小小小压缩工具_igp"));
    resize(960, 620);
}

MainWindow::~MainWindow() = default;   // 对象树自动回收，不需要手写 delete

// ============================================================
// createActions：造 QAction
// QAction 不是可视控件，它只是"一个行为"的描述，
// 同一份 QAction 可以同时挂到菜单、工具栏、快捷键上。
// 阶段 1 只挂 UI，不写 connect()。
// ============================================================
void MainWindow::createActions()
{
    // ---- 文件 ----
    actOpen       = new QAction(tr("打开(&O)..."), this);
    actNewArchive = new QAction(tr("新建压缩包(&N)..."), this);
    actExtract    = new QAction(tr("提取(&X)..."), this);
    actTest       = new QAction(tr("测试(&T)"), this);
    actExit       = new QAction(tr("退出(&Q)"), this);

    // ---- 导航 / 查看 ----
    actUp         = new QAction(tr("向上(&U)"), this);
    actRefresh    = new QAction(tr("刷新(&R)"), this);
    actBack    = new QAction(tr("后退(&B)"), this);
    actForward = new QAction(tr("前进(&F)"), this);
    actBack->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Left));
    actForward->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Right));

    // ---- 编辑 ----
    actSelectAll  = new QAction(tr("全选(&A)"), this);
    actInvertSel  = new QAction(tr("反向选择(&I)"), this);
    actCopy       = new QAction(tr("复制(&C)"), this);

    // ---- 工具 / 帮助 ----
    actOptions    = new QAction(tr("选项(&O)..."), this);
    actAbout      = new QAction(tr("关于 7-Zip(&A)..."), this);

    // 注意：new QAction(..., this) 里父对象是 MainWindow，
    // Qt 对象树会在 MainWindow 析构时自动 delete 这些 Action，不会内存泄漏。
}

// ============================================================
// 子步骤 3：菜单栏
// QMainWindow 自带 menuBar()，千万不要 new QMenuBar！
// ============================================================
void MainWindow::createMenus()
{
    QMenuBar *bar = menuBar();   // 拿到框架自带的菜单栏

    // ---- 文件(&F) ----
    menuFile = bar->addMenu(tr("文件(&F)"));
    menuFile->addAction(actOpen);
    menuFile->addAction(actNewArchive);
    menuFile->addSeparator();          // 分隔线
    menuFile->addAction(actExtract);
    menuFile->addAction(actTest);
    menuFile->addSeparator();
    menuFile->addAction(actExit);

    // ---- 编辑(&E) ----
    menuEdit = bar->addMenu(tr("编辑(&E)"));
    menuEdit->addAction(actSelectAll);
    menuEdit->addAction(actInvertSel);
    menuEdit->addSeparator();
    menuEdit->addAction(actCopy);

    // ---- 查看(&V) ----
    menuView = bar->addMenu(tr("查看(&V)"));
    menuView->addAction(actBack);
    menuView->addAction(actForward);
    menuView->addSeparator();
    menuView->addAction(actUp);
    menuView->addAction(actRefresh);

    // ---- 工具(&T) ----
    menuTools = bar->addMenu(tr("工具(&T)"));
    menuTools->addAction(actOptions);

    // ---- 帮助(&H) ----
    menuHelp = bar->addMenu(tr("帮助(&H)"));
    menuHelp->addAction(actAbout);
}

// ============================================================
// 子步骤 4：工具栏
// addToolBar() 是 QMainWindow 的成员函数，工具栏归框架管，
// 不要自己 new QToolBar 再塞进 central widget 的布局里。
// ============================================================
void MainWindow::createToolBars()
{
    mainToolBar = addToolBar(tr("主工具栏"));
    mainToolBar->setMovable(false);                          // 阶段1 先固定，别让用户拖跑
    mainToolBar->setToolButtonStyle(Qt::ToolButtonTextOnly); // 本阶段不用图标，纯文字按钮

    // 复用菜单里已经建好的 QAction —— 这就是 QAction 存在的意义
    mainToolBar->addAction(actBack);
    mainToolBar->addAction(actForward);
    mainToolBar->addAction(actUp);
    mainToolBar->addSeparator();

    mainToolBar->addAction(actOpen);
    mainToolBar->addAction(actNewArchive);
    mainToolBar->addAction(actExtract);
    mainToolBar->addAction(actTest);

    mainToolBar->addSeparator();
    mainToolBar->addAction(actRefresh);
}

// ============================================================
// 子步骤 2 + 5：central widget（地址栏 + 文件列表占位）
// QMainWindow 中间是空的，必须先 setCentralWidget，
// 然后给 central widget 自己设布局。
// 千万不要给 QMainWindow 本体 setLayout()！这是大坑。
// ============================================================
void MainWindow::createCentralWidget()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *vLayout = new QVBoxLayout(central);
    vLayout->setContentsMargins(4, 4, 4, 4);
    vLayout->setSpacing(4);

    // ---- 地址栏 ----
    addressEdit = new QLineEdit(central);
    addressEdit->setPlaceholderText(tr("当前路径"));
    addressEdit->setClearButtonEnabled(true);
    vLayout->addWidget(addressEdit);

    // ---- 文件列表 ----
    fileListView = new QTableView(central);

    // 阶段 2：用 QFileSystemModel 替换 QStandardItemModel
    fileSystemModel = new QFileSystemModel(this);
    fileSystemModel->setReadOnly(true);
    fileSystemModel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    // 初始空路径：模型根为空，表格显示为空
    fileSystemModel->setRootPath(QString());

    fileListView->setModel(fileSystemModel);

    fileListView->setSelectionBehavior(QAbstractItemView::SelectRows);
    fileListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    fileListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    fileListView->verticalHeader()->setVisible(false);
    fileListView->horizontalHeader()->setStretchLastSection(true);

    vLayout->addWidget(fileListView, 1);
}

// ============================================================
// 子步骤 6：状态栏
// statusBar() 也是 QMainWindow 自带，不要 new QStatusBar！
// ============================================================
void MainWindow::createStatusBar()
{
    // ---- 先放一条临时消息（会显示在左侧区，被下面的永久 widget 挤到前面）----
    statusBar()->showMessage(tr("就绪"));

    // ---- 左侧：选中信息（stretch = 1，占满左侧剩余空间，把右侧挤到最右）----
    selectionLabel = new QLabel(tr("0 个对象"), statusBar());
    statusBar()->addPermanentWidget(selectionLabel, 1);

    // ---- 右侧：磁盘信息（stretch = 0，只占自己需要的宽度）----
    diskLabel = new QLabel(tr("可用空间 --"), statusBar());
    statusBar()->addPermanentWidget(diskLabel, 0);

    // 视觉上让两个标签和状态栏边缘有点呼吸感（纯装饰）
    // statusBar()->setStyleSheet(
    //     "QStatusBar { padding: 0 4px; }"
    //     "QStatusBar QLabel { padding: 0 6px; }"
    //     );
}

void MainWindow::createConnections()
{
    connect(actUp,      &QAction::triggered, this, &MainWindow::navGoUp);
    connect(actBack,    &QAction::triggered, this, &MainWindow::navGoBack);
    connect(actForward, &QAction::triggered, this, &MainWindow::navGoForward);
    connect(actRefresh, &QAction::triggered, this, &MainWindow::onRefresh);

    connect(fileListView, &QTableView::doubleClicked,
            this, &MainWindow::onFileDoubleClicked);

    connect(addressEdit, &QLineEdit::returnPressed,
            this, &MainWindow::onAddressReturnPressed);
}

void MainWindow::changeDirectory(const QString &newPath, bool recordHistory)
{
    if (newPath.trimmed().isEmpty()) {
        return;
    }

    QFileInfo info(newPath);
    if (!info.exists() || !info.isDir()) {
        return;
    }

    const QString cleanPath = QDir::cleanPath(info.absoluteFilePath());

    // 让模型监控该目录，并把视图根设为该目录
    fileSystemModel->setRootPath(cleanPath);
    const QModelIndex rootIndex = fileSystemModel->index(cleanPath);
    if (!rootIndex.isValid()) {
        return;
    }
    fileListView->setRootIndex(rootIndex);

    m_currentPath = cleanPath;
    addressEdit->setText(cleanPath);

    // ---- 历史栈处理 ----
    if (recordHistory) {
        const bool sameAsCurrent =
            m_navIndex >= 0
            && m_navIndex < m_navHistory.size()
            && m_navHistory.at(m_navIndex) == cleanPath;

        if (!sameAsCurrent) {
            // 如果当前不在历史末尾，先截断后面的分支
            while (m_navHistory.size() > m_navIndex + 1) {
                m_navHistory.removeLast();
            }
            m_navHistory.append(cleanPath);
            m_navIndex = m_navHistory.size() - 1;
        }
    }

    updateActionEnableState();
}

void MainWindow::updateActionEnableState()
{
    if (actBack) {
        actBack->setEnabled(m_navIndex > 0);
    }
    if (actForward) {
        actForward->setEnabled(m_navIndex >= 0
                               && m_navIndex < m_navHistory.size() - 1);
    }
    if (actUp) {
        if (m_currentPath.isEmpty()) {
            actUp->setEnabled(false);
        } else {
            QDir dir(m_currentPath);
            actUp->setEnabled(dir.cdUp());
        }
    }
    if (actRefresh) {
        actRefresh->setEnabled(!m_currentPath.isEmpty());
    }
    // 选中状态相关逻辑留到阶段 3
}

void MainWindow::navGoUp()
{
    if (m_currentPath.isEmpty()) {
        return;
    }
    QDir dir(m_currentPath);
    if (dir.cdUp()) {
        changeDirectory(dir.absolutePath());
    }
}

void MainWindow::onFileDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }
    if (fileSystemModel->isDir(index)) {
        const QString path = fileSystemModel->filePath(index);
        changeDirectory(path);
    }
    // 双击文件：阶段 2 不做任何事
}

void MainWindow::onAddressReturnPressed()
{
    const QString path = addressEdit->text().trimmed();
    if (path.isEmpty()) {
        return;
    }
    QFileInfo info(path);
    if (info.exists() && info.isDir()) {
        changeDirectory(path);
    }
    // 非法路径：阶段 2 简单忽略，不弹窗
}

void MainWindow::navGoBack()
{
    if (m_navIndex <= 0) {
        return;
    }
    --m_navIndex;
    changeDirectory(m_navHistory.at(m_navIndex), false);
}

void MainWindow::navGoForward()
{
    if (m_navIndex < 0 || m_navIndex >= m_navHistory.size() - 1) {
        return;
    }
    ++m_navIndex;
    changeDirectory(m_navHistory.at(m_navIndex), false);
}

void MainWindow::onRefresh()
{
    if (m_currentPath.isEmpty()) {
        return;
    }

    const QString path = m_currentPath;
    // 先清空 rootPath 触发模型重新枚举，再设回当前目录
    fileSystemModel->setRootPath(QString());
    fileSystemModel->setRootPath(path);
    fileListView->setRootIndex(fileSystemModel->index(path));
    addressEdit->setText(path);
    updateActionEnableState();
}