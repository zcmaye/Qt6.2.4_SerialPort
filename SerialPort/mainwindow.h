#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QSerialPort>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void init();
private slots:
    void on_openPortBtn_released();

    void on_sendBtn_released();

    void onReadyRead();

    void on_openFileBtn_released();

    void on_sendFileBtn_released();

    void on_hexDisplayChx_toggled(bool checked);

    void on_timerSendChx_toggled(bool checked);

    void on_sendStopBtn_released();


private:
    void displayHex();
    void displayText();

    Ui::MainWindow *ui;
    QSerialPort serialPort_;
    QTimer timer_;
};
#endif // MAINWINDOW_H
