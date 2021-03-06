#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <direct.h>

#include "Implement/LogImpl.h"
#include "Implement/LogUserImpl.h"
#include "API/Yap/ModuleManager.h"

#include "datamanager.h"
#include "Processors/Display1D.h"
#include "Processors/Display2D.h"
#include "Processors/ReceiveData.h"
#include "Processors/PSliceIterator.h"
#include "Processors/PFft2D.h"
#include "DataSample/reconserver.h"
#include "imagelayoutwidget.h"
#include "Interface/Interfaces.h"
#include "Implement/ProcessorImpl.h"
#include "Client/DataHelper.h"
#include "datamanager.h"
#include <QFileDialog>

#include "DataSample/SampleDataProtocol.h"

#include <complex>
#include<QThread>
#include"globalvariable.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    SetImportedProcessors();

    DataManager::GetHandle().Demo1D();

    ui->buttonFinish->setEnabled(false);

   // _mythread= std::thread(buttonStart_thread,this);//创建线程

 /*   qDebug()<<"Main thread id:"<<this->thread()->currentThreadId();
    QThread * t2=new QThread();
    t2->start();

    MyObject * obj1=new MyObject();
    //把对象移动到新线程
    obj1->moveToThread(t2);

    //连接到对象
    QObject::connect(this->ui->pushButton_4,SIGNAL(clicked()),obj1,SLOT(on_btn_click()));
*/
/*
    qDebug()<<"main thread id:"<<this->thread()->currentThreadId();
    QThread * t2=new QThread();
    t2->start();

    QString port = ui->editListeningPort->text();
    reconServer = std::shared_ptr<ReconServer>(new ReconServer(this, port.toInt()));
    //connect(reconServer.get(), &ReconServer::signalDataReceived, this, &MainWindow::slotDataReceived);
    //connect(reconServer.get(), &ReconServer::signalDisconnected,this,&MainWindow::slotDisconnected);

    //把对象移动到新线程
    //reconServer.get()->setParent(0);
    reconServer.get()->moveToThread(t2);

    ui->buttonStart->setText("Waiting");
*/
    //on_buttonStart_clicked();
}

MainWindow::~MainWindow()
{
    //_mythread.join();
    //delete reconServer;
    reconServer.get()->destroyed();
    //reconServer.get()=nullptr;
    delete ui;
}


bool MainWindow::event(QEvent *event)
{
    switch (event->type())
    {
        case QEvent::User + finished://子线程已经重建完毕，进行数据的重置
        {
            ui->editInfo->appendPlainText(tr("FinishedReconstructionThread"));
            DataManager::GetHandle().JoinThread();
            ui->buttonStart->setText("Start");
            ui->buttonStart->setEnabled(true);
            ui->buttonFinish->setText("Finish");
            //ui->buttonFinish->setEnabled(false);
            std::unique_lock<std::mutex> lck(gv_data_repopsitory.gv_mtx);
            gv_data_repopsitory.gv_ready=false;
            //gv_data_repopsitory.gv_is_finished=true;
            gv_data_repopsitory.gv_channel_count=0;
            gv_data_repopsitory._all_data.clear();
            SampleDataStart temp;
            gv_data_repopsitory._gv_sample_start=temp;
            lck.unlock();
            reconServer.get()->close();//断开连接，然后服务器不再监听任何连接
            reconServer.get()->destroyed();
            gv_data_repopsitory._pwnd=nullptr;
            break;
        }
        default:
            break;
    }
   //return QWidget::event(event);
   return QMainWindow::event(event);
}


/*
void MainWindow::buttonStart_thread(MainWindow *mw)
{
    MainWindow* mw1=mw;
    DataManager::GetHandle().setPipelinPath(mw1->ui->editPipeline->text());
    DataManager::GetHandle().Pipeline2DforNewScan();
    //std::shared_ptr<ReconServer> reconServer1;
    if (!gv_data_repopsitory.reconServer1)
    {
        QString port = mw1->ui->editListeningPort->text();
        gv_data_repopsitory.reconServer1= std::shared_ptr<ReconServer>(new ReconServer(mw1, port.toInt()));//出错?
        //connect(reconServer.get(), &ReconServer::signalDataReceived, mw, &MainWindow::slotDataReceived);
        //connect(reconServer.get(), &ReconServer::signalDisconnected,mw,&MainWindow::slotDisconnected);
        //listen(QHostAddress::Any, port.toInt());
        mw1->ui->buttonStart->setText("Waiting");
    }
}
*/

void MainWindow::SetImportedProcessors()
{
    Yap::ImportedProcessorManager::RegisterProcessor(L"ReceiveData", new ReceiveData());
    Yap::ImportedProcessorManager::RegisterProcessor(L"Display1D", new Display1D(*ui->plot1d_widget));
    Yap::ImportedProcessorManager::RegisterProcessor(L"Display2D", new Display2D(*ui->imagelayout_widget));
    Yap::ImportedProcessorManager::RegisterProcessor(L"PSliceIterator", new PSliceIterator());
    Yap::ImportedProcessorManager::RegisterProcessor(L"PFft2D", new PFft2D());
}




void MainWindow::on_actionOpen_triggered()
{

    bool demo = false;

    if (!demo){
        QString start_folder2 = "D:\\projects\\ReconDemo\\Data";
        char   buffer[MAX_PATH];
        getcwd(buffer, MAX_PATH);

        QString start_folder(buffer);
        start_folder +="\\Data";



        auto file_path = QFileDialog::getOpenFileName(this, tr("Load File"), start_folder,
                  "Raw Data (*.fid);;NiuMriImage Files (*.niuimg);;Image Files (*.img);;All Files (*.*)", nullptr);
        if (!file_path.isEmpty())
        {
            DataManager::GetHandle().Load(file_path);

        }
    }
    else
    {

        //流水线的特点：数据驱动，数据不会停止，如果中间有停止机制（停在内存里），将造成拥堵。
        //流水线末端：显示，或者存储等数据堆积则是可以理解的。
        //auto output_data = CreateDemoIData();
        //ui->imagelayout_widget->AddImage(output_data.get(), nullptr);

        DataManager::GetHandle().Demo2D();
    }


}

void MainWindow::on_buttonStart_clicked()
{
    DataManager::GetHandle().setPipelinPath(ui->editPipeline->text());
    DataManager::GetHandle().Pipeline2DforNewScan();
    DataManager::GetHandle().SetPwnd(this);

//    if (!reconServer.get())
//    {
        QString port = ui->editListeningPort->text();
        reconServer = std::shared_ptr<ReconServer>(new ReconServer(this, port.toInt()));
        connect(reconServer.get(), &ReconServer::signalDataReceived, this, &MainWindow::slotDataReceived);
        connect(reconServer.get(), &ReconServer::signalDisconnected,this,&MainWindow::slotDisconnected);
//        reconServer = new ReconServer(this, port.toInt());
//        connect(reconServer, &ReconServer::signalDataReceived, this, &MainWindow::slotDataReceived);
//        connect(reconServer, &ReconServer::signalDisconnected,this,&MainWindow::slotDisconnected);
        //listen(QHostAddress::Any, port.toInt());
        ui->buttonStart->setText("Waiting");
        ui->buttonStart->setEnabled(false);
        ui->buttonFinish->setEnabled(true);
//    }
}
// void MainWindow::buttonStart_thread(MainWindow *mw0)
// {
//     //std::shared_ptr<MainWindow> mw=std::shared_ptr<MainWindow>(new MainWindow(mw0));
//        MainWindow* mw=mw0;
//     {
//         QString port = mw->ui->editListeningPort->text();
//         mw->reconServer = std::shared_ptr<ReconServer>(new ReconServer(mw, port.toInt()));
//         connect(mw->reconServer.get(), &ReconServer::signalDataReceived, mw, &MainWindow::slotDataReceived);
//         connect(mw->reconServer.get(), &ReconServer::signalDisconnected,mw,&MainWindow::slotDisconnected);
//        // listen(QHostAddress::Any, port.toInt());
//         mw->ui->buttonStart->setText("Waiting");
//     }
// }


void MainWindow::slotDataReceived(int length)
{
    QString len=QString::number(length);
    ui->editInfo->appendPlainText(len);
    //

    //assert(0);
    return;

}

void MainWindow::slotDisconnected(int socketDescriptor)//主线程接收数据完毕
{
    std::unique_lock <std::mutex> lck(gv_data_repopsitory.gv_mtx);
    gv_data_repopsitory.gv_is_finished=true;//告知子线程结束数据的重建
    lck.unlock();
    DataManager::GetHandle().SetSampleDataDataCount(0);//重置唤醒子线程的判断条件_sampleDataData_count为0

    ui->buttonFinish->setText("Finishing");
    ui->buttonFinish->setEnabled(false);

    ui->editInfo->appendPlainText(tr("FinishReceiveDta"));
    QString len=QString::number(socketDescriptor);
    ui->editInfo->appendPlainText(len);
/*
    ui->buttonStart->setText("start");
    ui->buttonStart->setEnabled(true);
    std::unique_lock<std::mutex> lck(gv_data_repopsitory.gv_mtx);
    gv_data_repopsitory.gv_ready=false;
    //gv_data_repopsitory.gv_is_finished=true;
    gv_data_repopsitory.gv_channel_count=0;
    gv_data_repopsitory._all_data.clear();
    SampleDataStart temp;
    gv_data_repopsitory._gv_sample_start=temp;
    lck.unlock();


//    reconServer->close();//断开连接，然后服务器不再监听任何连接
//    reconServer->deleteLater();

    reconServer.get()->close();//断开连接，然后服务器不再监听任何连接
    reconServer.get()->destroyed();


    //std::shared_ptr<ReconServer> sp2;
    //reconServer=sp2;
*/
    return;
}





void MainWindow::on_buttonBrowsePipeline_clicked()
{
    char   buffer[MAX_PATH];
    getcwd(buffer, MAX_PATH);
    QString start_folder(buffer);
    start_folder +="/config/pipelines";
    QString pipeline_path = QFileDialog::getOpenFileName(this, tr("Get pipeline path"), start_folder,
              "Pipeline Path (*.pipeline);;All Files (*.*)", nullptr);
    ui->editPipeline->setText(pipeline_path);
}

void MainWindow::on_buttonFinish_clicked()
{
    //reconServer.get()->clientSocket->disconnectFromHost();
    //reconServer.get()->close();//断开连接，然后服务器不再监听任何连接
    //reconServer.get()->destroyed();

    //ui->buttonFinish->setText("Finishing");
    //ui->buttonFinish->setEnabled(false);
    reconServer.get()->slotDisconnected(-2);//主线程断开连接，结束数据的接收
//    std::unique_lock <std::mutex> lck(gv_data_repopsitory.gv_mtx);
//    gv_data_repopsitory.gv_is_finished=true;//告知子线程结束数据的重建
//    lck.unlock();

}
