#include "timerbtn.h"
#include <QMouseEvent>

TimerBtn::TimerBtn(QWidget *parent):QPushButton(parent),_counter(10)
{
    _timer = new QTimer(this);

    connect(_timer,&QTimer::timeout,this,[this](){
        _counter--;
        if(_counter<=0){
            _timer->stop();
            _counter = 10;
            this->setText("获取");
            this->setEnabled(true);
            return;
        }
        this->setText(QString::number(_counter));
    });
}

TimerBtn::~TimerBtn()
{
    _timer->stop();
}

void TimerBtn::mouseReleaseEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton){
        qDebug()<<"mybutton was released!";
        this->setEnabled(false);
        this->setText(QString::number(_counter));
        _timer->start(1000);
        //抛出按下事件
        emit clicked();
    }

    //调用基类的mouspevent处理其它按下事件
    QPushButton::mouseReleaseEvent(e);
}
