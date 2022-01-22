#pragma once

#include <QWidget>

namespace Ui
{
    class About;
}

class About : public QWidget
{
    Q_OBJECT
public:
    explicit About(QWidget* parent = 0);
    ~About();
private:
    Ui::About* ui;
};
