#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtWidgets>
QSerialPort *myPort;

//bool is_port_open = false;
void MainWindow::sendSignal(QString instruction){
    if(myPort->isWritable()){

       myPort->write(instruction.toStdString().c_str());
    }
    else{
        qDebug() << "Could not write to serial";
    }
//    myPort->setPortName("COM19");

//    if(myPort->open(QIODevice::ReadWrite)){
////        Now the serial port is open try to set configuration
////        myPort->setBaudRate(QSerialPort::Baud9600);

//        if(!myPort->setBaudRate(QSerialPort::Baud9600))
//            qDebug()<<myPort->errorString();

//        if(!myPort->setDataBits(QSerialPort::Data8))
//            qDebug()<<myPort->errorString();

//        if(!myPort->setParity(QSerialPort::NoParity))
//            qDebug()<<myPort->errorString();

//        if(!myPort->setStopBits(QSerialPort::OneStop))
//            qDebug()<<myPort->errorString();

//        if(!myPort->setFlowControl(QSerialPort::NoFlowControl))
//            qDebug()<<myPort->errorString();

//        //If any error was returned the serial il corrctly configured

//        myPort->write(instruction.toStdString().c_str());
//        //the serial must remain opened

//        /*
//We will need this code when recieving message from zumo
//*/
//        if(myPort->waitForReadyRead(100)){
//            //Data was returned
//            qDebug()<<"Response: "<<myPort->readAll();
//        }else{
//            //No data
//            qDebug()<<"Time out";
//        }

//    }else{
//        qDebug()<<"Serial COM19 not opened. Error: "<<myPort->errorString();
//    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    myPort = new QSerialPort;
//    serialBuffer = "";

    qDebug() << "Number of Ports: " << QSerialPortInfo::availablePorts().length();
    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
        qDebug() << "Has Vender ID: " << serialPortInfo.hasVendorIdentifier();
        if (serialPortInfo.hasVendorIdentifier()){
            qDebug() << "Vender ID: " << serialPortInfo.vendorIdentifier();
        }

        qDebug() << "Has Product ID: " << serialPortInfo.hasProductIdentifier();
        if (serialPortInfo.hasProductIdentifier()){
            qDebug() << "Product ID: " << serialPortInfo.productIdentifier();
        }
    }

    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
        if(serialPortInfo.hasVendorIdentifier() && serialPortInfo.hasProductIdentifier()){
            if(serialPortInfo.vendorIdentifier() == zumo_vender_id){
                if(serialPortInfo.productIdentifier() == zumo_product_id){
                    qDebug() << "Found XBEE!! ";
                    zumo_port_name = serialPortInfo.portName();
                    qDebug() << "Port name" << zumo_port_name;
                    zumo_port_available = true;
                }
            }
        }
    }

    if(zumo_port_available){
        myPort->setPortName(zumo_port_name);
        bool is_port_open = myPort->open(QSerialPort::ReadWrite);
        if (is_port_open){
            qDebug() << "Port open successfully";
        } else{
            qDebug() << "Can Not open Port";
            qDebug() << myPort->error();
        }
        myPort->setBaudRate(QSerialPort::Baud9600);
        myPort->setDataBits(QSerialPort::Data8);
        myPort->setParity(QSerialPort::NoParity);
        myPort->setStopBits(QSerialPort::OneStop);
        myPort->setFlowControl(QSerialPort::NoFlowControl);
        QObject::connect(myPort, SIGNAL(readyRead()), this, SLOT(readSerial()));
    }
    else{
        QMessageBox::warning(this, "Error message", "Could not find XBEE");
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    if(myPort->isOpen()){
    myPort->close();
    }
}

void MainWindow::readSerial(){
    QByteArray serialData = myPort->readAll();
    QString message = QString::fromStdString(serialData.toStdString());
    ui->textEdit->append(message);
//ui->textEdit->setPlainText("Hello world");
//ui->textEdit->append("Dash");
   //ui->textEdit->setReadOnly()

}
void MainWindow::on_pushButton_clicked()
{
    //Move forward
//    Not sure if "QSerialPort.write();" works
//    myPort.write("w");
    sendSignal("w");
}


void MainWindow::on_pushButton_2_clicked()
{

//    myPort.write("x");
    sendSignal("a");
}


void MainWindow::on_pushButton_3_clicked()
{
    sendSignal("r");
}


void MainWindow::on_pushButton_4_clicked()
{
    sendSignal("l");
}


void MainWindow::on_pushButton_5_clicked()
{
    sendSignal("g");
}


void MainWindow::on_pushButton_6_clicked()
{
    sendSignal("c");
}


void MainWindow::on_pushButton_7_clicked()
{
    sendSignal("p");

}


void MainWindow::on_pushButton_8_clicked()
{    //Stop the Robot
    sendSignal("x");
//    readSerial();

}


void MainWindow::on_pushButton_10_clicked()
{
    sendSignal("m");
}


void MainWindow::on_pushButton_9_clicked()
{
    sendSignal("t");
}


void MainWindow::on_pushButton_11_clicked()
{
    sendSignal("z");
}


void MainWindow::on_pushButton_12_clicked()
{
    sendSignal("j");
}


void MainWindow::on_pushButton_13_clicked()
{
    sendSignal("o");
}

