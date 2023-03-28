#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QSerialPortInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();

    ui->linkLable->setText("<a href=\"https://www.dunkaiedu.com/\">顿开教育-顽石老师-版权所有</a>");
    ui->linkLable->setOpenExternalLinks(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{

    //设置改变
    connect(ui->baudRateCmb,&QComboBox::currentIndexChanged,this,[=]
    {
        auto br = ui->baudRateCmb->currentData().value<QSerialPort::BaudRate>();
        if(!serialPort_.setBaudRate(br))
        {
            QMessageBox::warning(this,"false","设置波特率失败:"+serialPort_.errorString());
        }
    });
    connect(ui->dataBitsCmb,&QComboBox::currentIndexChanged,this,[=]
    {
        auto value = ui->dataBitsCmb->currentData().value<QSerialPort::DataBits>();
        if(!serialPort_.setDataBits(value))
        {
            QMessageBox::warning(this,"false","设置数据位失败:"+serialPort_.errorString());
        }
    });
    connect(ui->stopBitsCmb,&QComboBox::currentIndexChanged,this,[=]
    {
        auto value = ui->stopBitsCmb->currentData().value<QSerialPort::StopBits>();
        if(!serialPort_.setStopBits(value))
        {
            QMessageBox::warning(this,"false","设置停止位失败:"+serialPort_.errorString());
        }
    });
    connect(ui->parityCmb,&QComboBox::currentIndexChanged,this,[=]
    {
        auto value = ui->parityCmb->currentData().value<QSerialPort::Parity>();
        if(!serialPort_.setParity(value))
        {
            QMessageBox::warning(this,"false","设置校验位失败:"+serialPort_.errorString());
        }
        qInfo()<<"sdflksjdfklsfkd";
    });


    //获取所有的可用的串口
    auto portsInfo = QSerialPortInfo::availablePorts();
    for(auto& info : portsInfo)
    {
        qInfo()<<info.description() << info.portName()<<info.systemLocation();
        ui->protsCmb->addItem(info.portName() +":" + info.description(),info.portName());
    }

    //获取标准的波特率
    auto baudRates =  QSerialPortInfo::standardBaudRates();
    for(auto br : baudRates)
    {
        ui->baudRateCmb->addItem(QString::number(br),br);
    }
    ui->baudRateCmb->setCurrentText("9600");

    //设置停止位
    ui->stopBitsCmb->addItem("1",QSerialPort::OneStop);
    ui->stopBitsCmb->addItem("1.5",QSerialPort::OneAndHalfStop);
    ui->stopBitsCmb->addItem("2",QSerialPort::TwoStop);

    //设置数据位
    ui->dataBitsCmb->addItem("5",QSerialPort::Data5);
    ui->dataBitsCmb->addItem("6",QSerialPort::Data6);
    ui->dataBitsCmb->addItem("7",QSerialPort::Data7);
    ui->dataBitsCmb->addItem("8",QSerialPort::Data8);
    ui->dataBitsCmb->setCurrentText("8");

    //设置校验位
    ui->parityCmb->addItem("NoParity",QSerialPort::NoParity);
    ui->parityCmb->addItem("EvenParity",QSerialPort::EvenParity);
    ui->parityCmb->addItem("OddParity",QSerialPort::OddParity);
    ui->parityCmb->addItem("SpaceParity",QSerialPort::SpaceParity);
    ui->parityCmb->addItem("MarkParity",QSerialPort::MarkParity);

    connect(&serialPort_,&QSerialPort::readyRead,this,&MainWindow::onReadyRead);

    timer_.callOnTimeout([=]
    {
        this->on_sendBtn_released();
    });

    connect(ui->clearRecvBtn,&QPushButton::clicked,ui->recvEdit,&QPlainTextEdit::clear);
    connect(ui->sendClearBtn,&QPushButton::clicked,ui->sendEdit,&QPlainTextEdit::clear);
}

void MainWindow::on_openPortBtn_released()
{
    //串口是否已经打开
    if(serialPort_.isOpen())
    {
        serialPort_.close();
        ui->openPortBtn->setText("打开串口");
        if(timer_.isActive())
            timer_.stop();
        return;
    }


    //获取串口名
    auto portName = ui->protsCmb->currentData().toString();
 /*   //获取波特率
    auto baudRate = ui->baudRateCmb->currentData().value<QSerialPort::BaudRate>();
    //获取数据位
    auto dataBits = ui->dataBitsCmb->currentData().value<QSerialPort::DataBits>();
    //获取停止位
    auto stopBits = ui->stopBitsCmb->currentData().value<QSerialPort::StopBits>();
    //获取校验位
    auto parity = ui->parityCmb->currentData().value<QSerialPort::Parity>();


    serialPort_.setBaudRate(baudRate);
    serialPort_.setDataBits(dataBits);
    serialPort_.setStopBits(stopBits);
    serialPort_.setParity(parity);
*/
    serialPort_.setPortName(portName);
    //打开串口
    if(!serialPort_.open(QIODevice::ReadWrite))
    {
        QMessageBox::warning(this,"warning",portName + " open failed:"+serialPort_.errorString());
        return;
    }
    else
    {
        ui->openPortBtn->setText("关闭串口");
    }
}

void MainWindow::on_sendBtn_released()
{
    auto dataStr = ui->sendEdit->toPlainText() + (ui->sendNewLineChx->isChecked() ? "\r\n":"");
    serialPort_.write(dataStr.toLocal8Bit());
}

void MainWindow::onReadyRead()
{
    auto data = serialPort_.readAll();
    ui->recvEdit->setPlainText(QString::fromLocal8Bit(data));
}

void MainWindow::on_openFileBtn_released()
{
    auto filename =QFileDialog::getOpenFileName(this,"选择文件",QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
                                 "txt(*.txt);;all(*.*)");
    if(!filename.isEmpty())
    {
       ui->fileNameEdit->setText(filename);
    }
}

void MainWindow::on_sendFileBtn_released()
{
    auto filename = ui->fileNameEdit->text();
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this,"warning",filename + " open failed:"+file.errorString());
        return;
    }
    //最好判断一下文件的编码
    serialPort_.write(QString::fromUtf8(file.readAll()).toLocal8Bit());
}

void MainWindow::on_hexDisplayChx_toggled(bool checked)
{
    if(checked)
        displayHex();
    else
        displayText();
}

void MainWindow::displayHex()
{
    //先把数据拿出来
    auto dataStr = ui->recvEdit->toPlainText();
    //转成十六进制
    auto hexData = dataStr.toLocal8Bit().toHex(' ').toUpper();
    //写回去
    ui->recvEdit->setPlainText(hexData);
}

void MainWindow::displayText()
{
    //先把数据拿出来
    auto dataStr = ui->recvEdit->toPlainText();
    //转成文本
    auto textData = QString::fromLocal8Bit(dataStr.toLocal8Bit());
    //写回去
    ui->recvEdit->setPlainText(textData);

}

void MainWindow::on_timerSendChx_toggled(bool checked)
{
    if(checked)
    {
        timer_.start(ui->timerPeriodEdit->text().toUInt());
        ui->timerPeriodEdit->setEnabled(false);
    }
    else
    {
        timer_.stop();
        ui->timerPeriodEdit->setEnabled(true);
    }
}

void MainWindow::on_sendStopBtn_released()
{
    serialPort_.clear();
    if(timer_.isActive())
        timer_.stop();
}


