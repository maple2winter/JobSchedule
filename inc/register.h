﻿#ifndef REGISTER_H
#define REGISTER_H

#include <QWidget>
#include "ui_register.h"

namespace Ui {
class Register;
}

class Register : public QWidget, public Ui::Register
{
    Q_OBJECT

public:
    explicit Register(QWidget *parent = 0);
    ~Register();

private:
    Ui::Register *ui;
};

#endif // REGISTER_H