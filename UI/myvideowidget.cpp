#include "myvideowidget.h"

MyVideoWidget::MyVideoWidget(QWidget *parent)
    : QWidget{parent}
{

    controlwidget_ = new QWidget();
    controlwidget_->setWindowFlags(Qt::Tool|Qt::FramelessWindowHint);

    controlwidget_->setStyleSheet("QWidget{background-color:transparent}");
    controlwidget_->setWindowOpacity(0.7);
    ResizeControlWidget();

    SetControlWidgetVisible();
    SetControlWidgetUnVisible();
    this->setUpdatesEnabled(false);
    this->setMouseTracking(true);

    QTimer* timer = new QTimer();
    connect(this,&MyVideoWidget::mousemoved,timer,[=](){
       timer->start(5000);
       timer->setSingleShot(true);
    });
    connect(timer,&QTimer::timeout,this,[=](){
       this->setCursor(Qt::BlankCursor);
       SetControlWidgetUnVisible();
    });
}

MyVideoWidget::~MyVideoWidget()
{
     delete this->controlwidget_;
}

QWidget* MyVideoWidget::GetControlWidget() {
    return controlwidget_;
}

void MyVideoWidget::SetControlWidgetVisible() {
    ResizeControlWidget();
    controlwidget_->setVisible(true);
}

void MyVideoWidget::SetControlWidgetUnVisible() {
    controlwidget_->setVisible(false);
}

void MyVideoWidget::ResizeControlWidget() {
    QPoint pos = this->mapToGlobal(this->pos())-this->pos();
    this->controlwidget_->setGeometry(pos.x(),pos.y()+this->height()-controlwidget_->height(),this->width(),30);
}

void MyVideoWidget::mouseMoveEvent(QMouseEvent *event) {
    event->accept();
    if(!this->controlwidget_->isVisible()){
        SetControlWidgetVisible();
        this->setCursor(Qt::ArrowCursor);
        emit mousemoved();
    }
}
