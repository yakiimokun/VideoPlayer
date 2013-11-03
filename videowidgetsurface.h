///
/// @file   videosurface.h
/// @author Kosuke Imoji <imoji@imoji-pc>
/// @date   Sat May 25 19:48:38 2013
/// 
/// @brief  
/// 
/// 
///
#pragma once

#include <QtCore/QRect> 
#include <QtGui/QImage>
#include <QtMultimedia/QAbstractVideoSurface>
#include <QtMultimedia/QVideoFrame>

/// 
/// @class 
/// @brief video surface class 
///
class VideoWidgetSurface : public QAbstractVideoSurface
{
   Q_OBJECT

public:
  VideoWidgetSurface(QWidget *widget, QObject *parent = 0);
  ~VideoWidgetSurface();

  QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;
  bool isFormatSupported(const QVideoSurfaceFormat &format, QVideoSurfaceFormat *similar) const;

  bool start(const QVideoSurfaceFormat &format);
  void stop();

  bool present(const QVideoFrame &frame);

  QRect videoRect() const { return m_targetRect; }
  void updateVideoRect();

  void paint(QPainter *painter);

private:
  QWidget        *m_widget;
  QImage::Format  m_imageFormat;
  QRect           m_targetRect;
  QSize           m_imageSize;
  QRect           m_sourceRect;
  QVideoFrame     m_currentFrame;
};
