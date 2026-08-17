// mainwindow.cpp - Qt6 UI shell (optional, BT_ENABLE_UI=ON)
#include "ui/mainwindow.h"

#include <QStandardItemModel>
#include <QStatusBar>
#include <QStringList>
#include <QTableView>

namespace bt {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("busmon - 汽车总线测试平台"));
    resize(960, 600);

    table_ = new QTableView(this);
    model_ = new QStandardItemModel(0, 5, this);
    model_->setHorizontalHeaderLabels(QStringList()
                                      << QStringLiteral("时间戳")
                                      << QStringLiteral("通道")
                                      << QStringLiteral("方向")
                                      << QStringLiteral("ID")
                                      << QStringLiteral("数据"));
    table_->setModel(model_);
    table_->horizontalHeader()->setStretchLastSection(true);
    setCentralWidget(table_);

    statusBar()->showMessage(QStringLiteral("未连接设备 (Null 自测模式)"));
}

MainWindow::~MainWindow() = default;

}  // namespace bt
