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

    // Disable all fields except TCP connection;
    ui->pushButtonDisconnect->setDisabled(true);
    ui->groupBoxChannel->setDisabled(true);
    ui->plainTextEditLogs->setDisabled(true);

    //set Log message field read only
    ui->plainTextEditLogs->setReadOnly(true);
}

MainWindow::~MainWindow()
{
    //stop the capture process -> Needed incase of abrupt disconnect during logging
    MainWindow::on_pushButtonStop_clicked();

    if(clientsocket->isOpen())
        clientsocket->close();
    delete ui;
}

// Slot methods
void MainWindow::on_pushButtonConnect_clicked()
{
    QString Srv_IP_Addr_str = ui->lineEditIPAddress->text();
    QString Srv_Port_Num_str = ui->lineEditPortNum->text();
    QStringList Srv_IP_Addr_strlist;
    unsigned int Srv_IP_Addr[4];
    unsigned int Srv_Port_Num;
    int i = 0, j = 0, k = 0, l = 0;

    // IP address check - 4 address part using "." delimiter
    Srv_IP_Addr_strlist = Srv_IP_Addr_str.split(".");
    int num_of_parts = Srv_IP_Addr_strlist.size();

    if(num_of_parts != 4)
    {
        QMessageBox::warning(this, "Connection", "Invalid IP Address");
        return;
    }
    else
    {
        // IP address check - Numbers only
        for(i=0; i<4; i++)
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
#if ENABLE_DEBUG
                    qInfo() << Srv_IP_Addr_strlist[i][j] << "is digit - " << is_digit;
#endif
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

    // IP address check - Range(0 to 255)
    for(k=0; k<4; k++)
    {
        if(((Srv_IP_Addr[k] > 255) || (Srv_IP_Addr[k] < 0)))
        {
            QMessageBox::warning(this, "Connection", "Invalid IP Address");
            return;
        }
    }

    // Port Number check - Numbers only
    if(!Srv_Port_Num_str.size())
    {
        QMessageBox::warning(this, "Connection", "Enter Port Number");
        return;
    }
    else
    {
        for(l=0; l<Srv_Port_Num_str.size(); l++)
        {
            bool is_alpha = Srv_Port_Num_str[l].isLetter();
            if(is_alpha)
            {
                QMessageBox::warning(this, "Connection", "Invalid Port Number");
                return;
            }
        }

        // Port Number check - Range(0 to 65535)
        Srv_Port_Num = Srv_Port_Num_str.toInt();
        if(((Srv_Port_Num > 65535) || (Srv_Port_Num < 0)))
        {
            QMessageBox::warning(this, "Connection", "Invalid Port Number");
            return;
        }
    }

    //Initiate TCP connection

    clientsocket->connectToHost(Srv_IP_Addr_str, Srv_Port_Num);
    if(clientsocket->waitForConnected(60000))       //Timeout - 1Minute
    {
        QMessageBox::information(this, "Connection Status", "Connection Successful!!");

        // Enable disconnect button
        ui->pushButtonDisconnect->setEnabled(true);

        // Disable connection configuration field
        ui->pushButtonConnect->setDisabled(true);
        ui->lineEditIPAddress->setDisabled(true);
        ui->lineEditPortNum->setDisabled(true);

        //Enable channel configuration fields
        ui->groupBoxChannel->setEnabled(true);

        //Enable log message field
        ui->plainTextEditLogs->setEnabled(true);
    }
    else
    {
        // on connection fail - capture error type and notify user
        QAbstractSocket::SocketError err = QAbstractSocket::SocketError::UnknownSocketError;
        err = clientsocket->error();
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
        ui->plainTextEditLogs->setDisabled(true);

        // Disable disconnect button
        ui->pushButtonDisconnect->setDisabled(true);

        // Enable connection configuration field
        ui->pushButtonConnect->setEnabled(true);
        ui->lineEditIPAddress->setEnabled(true);
        ui->lineEditPortNum->setEnabled(true);
    }
}


void MainWindow::on_pushButtonCapture_clicked()
{
    char log_config[4];
    // Offset 1 is added to avoid null bytes in TCP payload
    log_config[0] = (uint8_t)ui->checkBoxAnalog->isChecked() + PAYLOAD_OFFSET;
    log_config[1] = (uint8_t)ui->checkBoxDigital->isChecked() + PAYLOAD_OFFSET;
    log_config[2] = (uint8_t)ui->comboBoxSampleRate->currentIndex() + PAYLOAD_OFFSET;
    log_config[3] = LOG_OPERATION_START;

    if(!((uint8_t)ui->checkBoxDigital->isChecked() || (uint8_t)ui->checkBoxAnalog->isChecked()))
    {
        QMessageBox::critical(this, "Configuration", "Select channel!!");
        return;
    }

    ui->plainTextEditLogs->clear();

#if ENABLE_DEBUG
    qInfo() << "analog" << log_config[1];
    qInfo() << "digital" << log_config[2];
    qInfo() << "samplerate:" << log_config[3];
#endif

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
    char log_config[4];
    // Offset 1 is added to avoid null bytes in TCP payload
    log_config[0] = (uint8_t)ui->checkBoxAnalog->isChecked() + PAYLOAD_OFFSET;
    log_config[1] = (uint8_t)ui->checkBoxDigital->isChecked() + PAYLOAD_OFFSET;
    log_config[2] = (uint8_t)ui->comboBoxSampleRate->currentIndex() + PAYLOAD_OFFSET;
    log_config[3] = LOG_OPERATION_STOP;

    clientsocket->write(log_config);
    clientsocket->waitForBytesWritten(1000);

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

