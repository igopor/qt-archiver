#include "mainwindow.h"

#include <QAction>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QStandardItemModel>
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
    mainToolBar->addAction(actUp);
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
    // ---- 子步骤 2：中央部件 + 垂直布局 ----
    QWidget *central = new QWidget(this);
    setCentralWidget(central);          // ← 忘了这句，里面所有控件都看不见

    QVBoxLayout *vLayout = new QVBoxLayout(central);
    vLayout->setContentsMargins(4, 4, 4, 4);   // 四周留白
    vLayout->setSpacing(4);                    // 控件之间的缝隙

    // ---- 子步骤 5：地址栏 ----
    addressEdit = new QLineEdit(central);
    addressEdit->setPlaceholderText(tr("当前路径"));
    addressEdit->setClearButtonEnabled(true);
    vLayout->addWidget(addressEdit);    // stretch 默认 0：只占自己需要的高度

    // ---- 子步骤 5：文件列表占位 ----
    fileListView = new QTableView(central);

    // 只是为了让占位看起来像 7-Zip，给个空表头；不接任何数据逻辑
    fileModel = new QStandardItemModel(0, 4, this);   // 0 行 4 列 → 表格是空的
    fileModel->setHorizontalHeaderLabels({ tr("名称"), tr("大小"),
                                          tr("修改日期"), tr("属性") });
    fileListView->setModel(fileModel);

    // 一些"看起来像样"的静态属性（都不涉及业务逻辑）
    fileListView->setSelectionBehavior(QAbstractItemView::SelectRows);
    fileListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    fileListView->setEditTriggers(QAbstractItemView::NoEditTriggers);  // 静态界面，禁止编辑
    fileListView->verticalHeader()->setVisible(false);                 // 隐藏左侧行号
    fileListView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch); // 名称列自动撑开

    vLayout->addWidget(fileListView, 1);  // stretch = 1 → 吃掉剩余全部空间
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
    statusBar()->setStyleSheet(
        "QStatusBar { padding: 0 4px; }"
        "QStatusBar QLabel { padding: 0 6px; }"
        );
}

void MainWindow::updateActionEnableState()
{
    // TODO 阶段3实现：根据选中项状态更新各个QAction启用/禁用
}
