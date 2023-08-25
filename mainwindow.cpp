#include "mainwindow.h"
#include "./ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    clientsocket = new QTcpSocket(this);
    connect(clientsocket, SIGNAL(readyRead()), this, SLOT(readyRead()));

    //Add combo items for sample rate
    ui->comboBoxSampleRate->addItem("1 Sample/s");
    ui->comboBoxSampleRate->addItem("5 Sample/s");
    ui->comboBoxSampleRate->addItem("10 Sample/s");

    // Disable channel and Log fields
    ui->groupBoxChannel->setDisabled(true);
}

MainWindow::~MainWindow()
{
    if(clientsocket->isOpen())
        clientsocket->close();
    delete ui;
}

void MainWindow::on_pushButtonConnect_clicked()
{
    QString Srv_IP_Addr_str = ui->lineEditIPAddress->text();
    QString Srv_Port_Num_str = ui->lineEditPortNum->text();
    QStringList Srv_IP_Addr_strlist;
    unsigned int Srv_IP_Addr[4];
    unsigned int Srv_Port_Num;

    // Check are there 4 parts of IP address
    Srv_IP_Addr_strlist = Srv_IP_Addr_str.split(".");
    int num_of_parts = Srv_IP_Addr_strlist.size();

    if(num_of_parts != 4)
    {
        QMessageBox::warning(this, "Connection", "Invalid IP Address");
        return;
    }
    else
    {
        // Check is there any alphabet in the IP address
        for(int i=0; i<4; i++)
        {
            if(Srv_IP_Addr_strlist[i].size() <= 0)
            {
                QMessageBox::warning(this, "Connection", "Invalid IP Address");
                return;
            }
            else
            {
                for(int j=0; j<Srv_IP_Addr_strlist[i].size(); j++)
                {
                    bool is_digit = Srv_IP_Addr_strlist[i][j].isDigit();
                    qInfo() << Srv_IP_Addr_strlist[i][j] << "is digit - " << is_digit;
                    if(!(is_digit))
                    {
                        QMessageBox::warning(this, "Connection", "Invalid IP Address");
                        return;
                    }
                }
            }
        }

        Srv_IP_Addr[0] = Srv_IP_Addr_strlist[0].toInt();
        Srv_IP_Addr[1] = Srv_IP_Addr_strlist[1].toInt();
        Srv_IP_Addr[2] = Srv_IP_Addr_strlist[2].toInt();
        Srv_IP_Addr[3] = Srv_IP_Addr_strlist[3].toInt();
    }

    // Check the IP addr range and not alpha
    for(int i=0; i<4; i++)
    {
        if(!((Srv_IP_Addr[i] <= 255) || (Srv_IP_Addr[i] >= 0)))
        {
            QMessageBox::warning(this, "Connection", "Invalid IP Address");
            return;
        }
    }

    // check is the port number alphabet

    for(int i=0; i<Srv_Port_Num_str.size(); i++)
    {
        bool is_alpha = Srv_Port_Num_str[i].isLetter();
        if(is_alpha)
        {
            QMessageBox::warning(this, "Connection", "Invalid Port Number");
            return;
        }
    }

    // check is the port number in range
    Srv_Port_Num = Srv_Port_Num_str.toInt();
    if(!((Srv_Port_Num <= 65535) || (Srv_Port_Num >= 0)))
    {
        QMessageBox::warning(this, "Connection", "Invalid Port Number");
        return;
    }

    // After all the check, initiate connection

    //qInfo() << "IP_Addr[0]:" << Srv_IP_Addr_strlist[0];
    clientsocket->connectToHost(Srv_IP_Addr_str, Srv_Port_Num);
    if(clientsocket->waitForConnected(60000))
    {
        QMessageBox::information(this, "Connection Status", "Connection Successful!!");

        // Disable connection configuration field
        ui->pushButtonConnect->setDisabled(true);
        ui->lineEditIPAddress->setDisabled(true);
        ui->lineEditPortNum->setDisabled(true);

        //Enable channel configuration fields
        ui->groupBoxChannel->setEnabled(true);
    }
    else
    {
        QMessageBox::critical(this, "Connection Status", "Connection Failed!!");
    }
}


void MainWindow::on_pushButtonDisconnect_clicked()
{
    if(clientsocket->isOpen())
    {
        //First, stop the capture process
        MainWindow::on_pushButtonStop_clicked();

        clientsocket->close();
        QMessageBox::information(this, "Connection Status", "Connection closed!");

        // Disable channel and Log fields
        ui->groupBoxChannel->setDisabled(true);

        // Enable connection configuration field
        ui->pushButtonConnect->setEnabled(true);
        ui->lineEditIPAddress->setEnabled(true);
        ui->lineEditPortNum->setEnabled(true);
    }
}


void MainWindow::on_pushButtonCapture_clicked()
{
    char log_config[3];
    log_config[0] = (char)ui->checkBoxAnalog->isChecked();
    log_config[1] = (char)ui->checkBoxDigital->isChecked();
    log_config[2] = ui->comboBoxSampleRate->currentIndex();

    qInfo() << "analog" << log_config[0];
    qInfo() << "samplerate:" << log_config[2];
    qInfo() << "digital" << log_config[1];

    // Disable channel configuration fields
    ui->checkBoxAnalog->setDisabled(true);
    ui->checkBoxDigital->setDisabled(true);
    ui->comboBoxSampleRate->setDisabled(true);
    ui->pushButtonCapture->setDisabled(true);

    clientsocket->write(log_config);
    clientsocket->waitForBytesWritten(1000);

}


void MainWindow::on_pushButtonStop_clicked()
{
    // Enable channel configuration fields
    ui->checkBoxAnalog->setEnabled(true);
    ui->checkBoxDigital->setEnabled(true);
    ui->comboBoxSampleRate->setEnabled(true);
    ui->pushButtonCapture->setEnabled(true);
}

void MainWindow::readyRead()
{
    QByteArray rcvData;
    rcvData = clientsocket->readAll();
    ui->plainTextEditLogs->appendPlainText(QString(rcvData));
}

