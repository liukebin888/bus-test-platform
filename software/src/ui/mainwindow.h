// mainwindow.h - Qt6 UI shell (optional, BT_ENABLE_UI=ON)
#pragma once

#include <QMainWindow>

class QStandardItemModel;
class QTableView;

namespace bt {

// Placeholder trace/statistics window. Phase B wires the L2 pipeline and
// L4 engines into this view (v3.0 section 10 UI).
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    QTableView* table_ = nullptr;
    QStandardItemModel* model_ = nullptr;
};

}  // namespace bt
