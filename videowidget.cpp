///
/// @file   videowidget.cpp
/// @author Kosuke Imoji <imoji@imoji-pc>
/// @date   Sun Oct 20 23:19:12 2013
/// 
/// @brief  
/// 
/// 
///
#include "videowidget.h"
#include "videowidgetsurface.h"

#include <QAbstractVideoSurface>
#include <QPainter>
#include <QPaintEvent>

/// 
/// @brief
/// @param parent 
///
VideoWidget::VideoWidget(QWidget *parent) 
    : QWidget(parent)
    , m_surface(0)
{
  setAutoFillBackground(false);
  setAttribute(Qt::WA_NoSystemBackground, true);
  setAttribute(Qt::WA_PaintOnScreen, true);

  QPalette palette = this->palette(); ///< get the current palette
  palette.setColor(QPalette::Background, Qt::black);
  setPalette(palette);

  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

  m_surface = new VideoWidgetSurface(this); ///< create video surface
}

VideoWidget::~VideoWidget()
{
  delete m_surface;
}

/// 
/// @brief trigger 
/// @param event 
///
void VideoWidget::paintEvent(QPaintEvent* event)
{
  QPainter painter(this);

  if (m_surface->isActive()) {
    const QRect videoRect = m_surface->videoRect();

    if (!videoRect.contains(event->rect())) {
      /// check whether update rectangle contains videoRect
      QRegion region = event->region();
      region.subtract(videoRect);

      QBrush brush = palette().background();

      foreach (const QRect &rect, region.rects())
          painter.fillRect(rect, brush);
    }

    m_surface->paint(&painter);
  } else {
    painter.fillRect(event->rect(), palette().background());
  }
}

/// 
/// @brief 
/// @param event 
///
void VideoWidget::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);
  m_surface->updateVideoRect();
}

/// 
/// @brief return video surface
///
/// @return QVideoFrame
///
QAbstractVideoSurface* VideoWidget::videoSurface() const 
{ 
  return m_surface; 
}

