///
/// @file   videowidget.h
/// @author Kosuke Imoji <imoji@imoji-pc>
/// @date   Sun Oct 20 23:15:27 2013
/// 
/// @brief  
/// 
/// 
///

#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>

class QAbstractVideoSurface;
class VideoWidgetSurface;

class VideoWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit VideoWidget(QWidget *parent = 0);
    ~VideoWidget();
    QAbstractVideoSurface* videoSurface() const;

protected:
    void paintEvent(QPaintEvent* event);
    void resizeEvent(QResizeEvent* event);

private:
    VideoWidgetSurface* m_surface;
};

#endif // VIDEOWIDGET_H
