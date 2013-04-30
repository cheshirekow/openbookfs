#ifndef INFO_H
#define INFO_H

#include <QPushButton>
#include <QPixmap>
#include <QIcon>
#include <QDebug>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

class Info : public QPushButton
{
    Q_OBJECT
public:
    explicit Info(QWidget *parent = 0);

    void SetText(QString value){
        this->text->setText(value);
    }
    
signals:
    
public slots:

protected:
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);

private:
    QWidget *i;

    QLabel *text;
};

#endif // INFO_H
