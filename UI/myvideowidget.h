#ifndef MYVIDEOWIDGET_H
#define MYVIDEOWIDGET_H

#include <QWidget>
#include <QToolButton>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QSlider>
#include <QMouseEvent>
#include <QTimer>
#include <QBoxLayout>
#include <QVBoxLayout>

class MyVideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MyVideoWidget(QWidget *parent = nullptr);
    ~MyVideoWidget();
    QWidget* GetControlWidget();
    void SetControlWidgetVisible();
    void SetControlWidgetUnVisible();
    void ResizeControlWidget();
    QWidget* GetVideoWidget() ;
signals:
    void mousemoved();

private:
    QWidget* videowidget_;
    QWidget* controlwidget_;

protected:
    void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent* event)Q_DECL_OVERRIDE;
};

#endif // MYVIDEOWIDGET_H
