﻿#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QKeyEvent>
#include "ui_login.h"

namespace Ui {
class Login;
}

class Login : public QWidget, public Ui::Login
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = 0);
    ~Login();

    void signInSignalSend() {
        emit userSignInSignal();
    }
    void keyPressEvent(QKeyEvent *event);

private slots:
    void on_LoginBtn_clicked();
    void on_RegisterBtn_clicked();

signals:
    void userSignInSignal();
};

#endif // LOGIN_H
