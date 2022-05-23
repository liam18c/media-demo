#include "myvideowidget.h"

MyVideoWidget::MyVideoWidget(QWidget *parent)
    : QWidget{parent}
{

    videowidget_ = new QWidget(this);
    videowidget_->setMouseTracking(true);

    controlwidget_ = new QWidget();
    controlwidget_->setWindowFlags(Qt::Tool|Qt::FramelessWindowHint);

    controlwidget_->setStyleSheet("QWidget{background-color:transparent}");
    controlwidget_->setWindowOpacity(0.7);

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

QWidget *MyVideoWidget::GetVideoWidget()
{
    return videowidget_;
}

void MyVideoWidget::mouseMoveEvent(QMouseEvent *event) {
    event->accept();
    if(!this->controlwidget_->isVisible()){
        SetControlWidgetVisible();
        this->setCursor(Qt::ArrowCursor);
        emit mousemoved();
    }
}

void MyVideoWidget::resizeEvent(QResizeEvent* event)
{
    this->videowidget_->resize(this->size());
    ResizeControlWidget();
}

