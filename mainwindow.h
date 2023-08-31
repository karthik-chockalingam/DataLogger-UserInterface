#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QMessageBox>
#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#define LOG_OPERATION_START 2
#define LOG_OPERATION_STOP  1
#define PAYLOAD_OFFSET      1
#define ENABLE_DEBUG        0

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    Ui::MainWindow *ui;
    QTcpSocket *clientsocket;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButtonConnect_clicked();
    void on_pushButtonDisconnect_clicked();
    void on_pushButtonCapture_clicked();
    void on_pushButtonStop_clicked();
    void readyRead();
};
#endif // MAINWINDOW_H
