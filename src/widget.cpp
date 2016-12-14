﻿#include "ui_widget.h"
#include "../inc/widget.h"
#include <algorithm>
#include <sstream>
#include <QLabel>
#include <QString>
#include <QDebug>
#include <QMessageBox>
#include <iostream>
#include <QStandardItem>

unsigned short Widget::runtime;

Widget::Widget(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);
//    setWindowTitle("Jobs Scheduler");
    setFixedSize(this->width(), this->height());
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [=](){
        CurTimeClock->display(QString::number(runtime++));
    } );
    CurTimeClock->setPalette(Qt::red);

    isPSA = isPM = isRun = isMethodFixed = false;
    scheduleMethod = "";
    radioBtnVec = { FCFS, EDF, SJF, HRRN, MFQ, RR };
    initMap();
}

void Widget::initMap()
{
    radioBtnMap[std::string("FCFS")] = 0;
    radioBtnMap[std::string("EDF")] = 1;
    radioBtnMap[std::string("SJF")] = 2;
    radioBtnMap[std::string("HRRN")] = 3;
    radioBtnMap[std::string("MFQ")] = 4;
    radioBtnMap[std::string("RR")] = 5;
}

Widget::~Widget()
{

}

class Widget::ValidJob{
private:
    std::string jobName;
    std::string joinTimeStr;
    std::string lastTimeStr;
    std::string deadLineStr;
    std::string fault;

    unsigned short joinTime;
    unsigned short lastTime;
    unsigned short deadLine;
public:
    ValidJob() {}
    ~ValidJob() {}

    void setJobName(std::string _jobName) { jobName = _jobName; }
    void setJoinTimeStr(std::string _joinTimeStr) { joinTimeStr = _joinTimeStr; }
    void setLastTimeStr(std::string _lastTimeStr) { lastTimeStr = _lastTimeStr; }
    void setDeadLineStr(std::string _deadLineStr) { deadLineStr = _deadLineStr; }
    void setJoinTime(unsigned short _joinTime) { joinTime = _joinTime; }
    void setLastTime(unsigned short _lastTime) { lastTime = _lastTime; }
    void setDeadLine(unsigned short _deadLine) { deadLine = _deadLine; }
    void setFault(std::string faultStr) { fault += faultStr; }

    std::string getFault() { return fault; }
    std::string getJobName() { return jobName; }
    std::string getJoinTimeStr() { return joinTimeStr; }
    std::string getLastTimeStr() { return lastTimeStr; }
    std::string getDeadLineStr() { return deadLineStr; }
    unsigned short getJoinTime() { return joinTime; }
    unsigned short getLastTime() { return lastTime; }
    unsigned short getDeadLine() { return deadLine; }

    bool checkJobValid();   //check if the committed job is valid
};

void Widget::on_ClearAllDataBtn_clicked()
{
    if(!isRun){
        isMethodFixed = false;
        EnableRadioBtn();
    }
}

void Widget::on_RunBtn_clicked()
{
    /* if it is not running, that initialize the operation and start the timer; */
    if(isMethodFixed && !isRun){
        runtime = 0;
        CurTimeClock->setPalette(Qt::green);
        timer->start(500);
        isRun = true;
        RunBtn->setDisabled(true);
    }
}

void Widget::on_PauseBtn_clicked()
{
    /* it only work when the jobs are running */
    if(isRun){
        static bool isPause = false;
        if(isPause){
            CurTimeClock->setPalette(Qt::green);
            timer->start(500);
            PauseBtn->setText(tr("Pause"));     //change the text show on the button
            isPause = false;
        }else{
            timer->stop();
            CurTimeClock->setPalette(Qt::red);
            PauseBtn->setText(tr("Continue"));  //change the text show on the button
            isPause = true;
        }
    }

}

void Widget::on_StopBtn_clicked()
{
    timer->stop();
    CurTimeClock->display(0);
    isRun = false;
    isMethodFixed = false;
    EnableRadioBtn();

}

/* select schedule type */
void Widget::on_PSA_clicked(bool checked) { isPSA = checked; }
void Widget::on_PM_clicked(bool checked) { isPM = checked; }

/* select schedule method */
void Widget::on_FCFS_clicked() { scheduleMethod = "FCFS"; }
void Widget::on_MFQ_clicked() { scheduleMethod = "MFQ"; }
void Widget::on_RR_clicked() { scheduleMethod = "RR"; }
void Widget::on_SJF_clicked() { scheduleMethod = "SJF"; }
void Widget::on_HRRN_clicked() { scheduleMethod = "HRRN"; }
void Widget::on_EDF_clicked() { scheduleMethod = "EDF"; }

void Widget::DisableRadioBtn(std::string exception)
{
    for(auto it = radioBtnMap.begin(); it != radioBtnMap.end(); ++it){
        if(it->first != exception){
            radioBtnVec[it->second]->setDisabled(true);
        }
    }
}

void Widget::EnableRadioBtn()
{
    for(size_t i = 0; i < radioBtnVec.size(); ++i){
        radioBtnVec[i]->setEnabled(true);
    }
}

void Widget::on_PriorityCombo_currentIndexChanged(int index)
{

}

void Widget::on_ClearInputBtn_clicked()
{
    /* clear input */
    JobNameEdit->clear();
    JoinTimeEdit->clear();
    LastTimeEdit->clear();
    DeadLineEdit->clear();
    PriorityCombo->setCurrentIndex(0);
    qDebug() << "clear data" ;
}

void Widget::methodMsgSend()
{
    emit methodFixedSignal(scheduleMethod, isPM, isPSA);
}

void Widget::on_CommitInputBtn_clicked()
{
    validJob = new ValidJob;
    /* set attribute of job */
    validJob->setJobName(std::string((const char*)JobNameEdit->text().toLocal8Bit()));
    validJob->setJoinTimeStr(std::string((const char*)JoinTimeEdit->text().toLocal8Bit()));
    validJob->setLastTimeStr(std::string((const char*)LastTimeEdit->text().toLocal8Bit()));
    validJob->setDeadLineStr(std::string((const char*)DeadLineEdit->text().toLocal8Bit()));

    if(false == validJob->checkJobValid()){
        std::string warning =
                std::string("The job you committed is not valid!\n") +
                "The follow may be the error:\n" +
                validJob->getFault();

        QMessageBox::warning(this, tr("Warning"), tr(warning.c_str()));
    }else{
        qDebug() << "commit success" << endl;
        if(!isMethodFixed){
            methodMsgSend();
            isMethodFixed = true;
            DisableRadioBtn(scheduleMethod);
        }



        TableAddJobItem(PreInputTbl);
        Job *job = new Job(validJob->getJobName(),
                                     validJob->getJoinTime(),
                                     validJob->getLastTime(),
                                     validJob->getDeadLine(),
                                     PriorityCombo->currentIndex());

        jobSend(job);
    }
}

void Widget::TableAddJobItem(QTableWidget *table)
{
    int rowCount = table->rowCount() + 1;
    PreInputTbl->setRowCount(rowCount);
    QTableWidgetItem *jobNameItem = new QTableWidgetItem();
    jobNameItem->setText(JobNameEdit->text());
    QTableWidgetItem *joinTimeItem = new QTableWidgetItem();
    joinTimeItem->setText(JoinTimeEdit->text());
    QTableWidgetItem *lastTimeItem = new QTableWidgetItem();
    lastTimeItem->setText(LastTimeEdit->text());
    QTableWidgetItem *deadLineItem = new QTableWidgetItem();
    deadLineItem->setText(DeadLineEdit->text());
    QTableWidgetItem *currentIndexItem = new QTableWidgetItem();
    currentIndexItem->setText(PriorityCombo->currentText());

    table->setItem(rowCount - 1, 0, jobNameItem);
    table->setItem(rowCount - 1, 1, joinTimeItem);
    table->setItem(rowCount - 1, 2, lastTimeItem);
    table->setItem(rowCount - 1, 3, deadLineItem);
    table->setItem(rowCount - 1, 4, currentIndexItem);
}

void Widget::jobSend(Job *job)
{
    emit jobCommingSignal(job);
}

void Widget::on_OpenFile_clicked()
{

}



void Widget::on_PauseBtn_pressed()
{
    PauseBtn->setStyleSheet("QPushButton:hover{background: red;}");//setting background to be red when hover
    PauseBtn->setToolTip(tr("Transfer File(s) from Left to Right"));//setting tool tip when hover
}


bool Widget::ValidJob::checkJobValid()
{
    bool flag = true;
    /* check job name */
    if("" == jobName){
        this->setFault("The job is without name!\n");
        flag = false;
    }

    std::istringstream ss;
    unsigned short time = -1;
    std::cout << "time is " << time << std::endl;

    /* check join time */
    ss.str(joinTimeStr);
    ss >> time;
    if(ss.fail() || time < 0){
        this->setFault("The number about the join time is invalid!\n");
        qDebug() << "join time is " << time << endl;
        qDebug() << ss.failbit;
        std::cout << joinTimeStr << std::endl;
        flag = false;
    }else{
        joinTime = time;
    }
    ss.clear();

    /* check last time */
    ss.str(lastTimeStr);
    ss >> time;
    if(ss.fail() || time < 0){
        this->setFault("The number about the last time is invalid!\n");
        qDebug() << "last time is " << time << endl;
        qDebug() << ss.failbit;
        std::cout << lastTimeStr << std::endl;
        flag = false;
    }else{
        lastTime = time;
    }
    ss.clear();

    /* check deadine */
    ss.str(deadLineStr);
    ss >> time;
    if(ss.fail() || time < joinTime + lastTime){
        this->setFault("The number about the deadline is invalid!\n");
        qDebug() << "deadline is " << time << endl;
        qDebug() << ss.failbit;
        std::cout << deadLineStr << std::endl;
        flag = false;
    }else{
        deadLine = time;
    }

    return flag;
}


