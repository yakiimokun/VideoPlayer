///
/// @file   videosurface.cpp
/// @author Kosuke Imoji <imoji@imoji-pc>
/// @date   Sat May 25 21:46:19 2013
/// 
/// @brief  
/// 
/// 
///
#include "videowidgetsurface.h"

#include <QtMultimedia>

/// 
/// @brief constructor
/// @param widget 
/// @param parent 
///
VideoWidgetSurface::VideoWidgetSurface(QWidget *widget, QObject *parent)
  :QAbstractVideoSurface(parent)
  ,m_widget(widget)
  ,m_imageFormat(QImage::Format_Invalid)
{
}

/// 
/// @brief destructor
///
VideoWidgetSurface::~VideoWidgetSurface()
{
}

/// 
/// @brief
/// @param handleType 
///
/// @return 
///
QList<QVideoFrame::PixelFormat> VideoWidgetSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
  if (handleType == QAbstractVideoBuffer::NoHandle) {
    return QList<QVideoFrame::PixelFormat>()
            << QVideoFrame::Format_RGB32
            << QVideoFrame::Format_ARGB32
            << QVideoFrame::Format_ARGB32_Premultiplied
            << QVideoFrame::Format_RGB565
            << QVideoFrame::Format_RGB555
            << QVideoFrame::Format_RGB24
            << QVideoFrame::Format_Y8
            << QVideoFrame::Format_Y16;
  }
   
  return QList<QVideoFrame::PixelFormat>();
}

/// 
/// @brief whether 
/// @param format 
/// @param similar 
///
/// @return 
///
bool VideoWidgetSurface::isFormatSupported(const QVideoSurfaceFormat &format, QVideoSurfaceFormat *similar) const
{
  Q_UNUSED(similar);

  const QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());
  const QSize size = format.frameSize();

  return imageFormat != QImage::Format_Invalid && !size.isEmpty() && format.handleType() == QAbstractVideoBuffer::NoHandle;
}

/// 
/// @brief start video surface presenting
/// @param format 
///
/// @return 
///
bool VideoWidgetSurface::start(const QVideoSurfaceFormat &format)
{
  const QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());
  const QSize size                 = format.frameSize();

  if (imageFormat == QImage::Format_Invalid || size.isEmpty()) {
    return false;
  }

  this->m_imageFormat = imageFormat;
  m_imageSize         = size;
  m_sourceRect        = format.viewport();

  QAbstractVideoSurface::start(format);    ///< start video surface presenting as a format frames

  m_widget->updateGeometry();	           ///< Notifies the layout system that this widget has changed and may need to change geometry
  updateVideoRect();

  return true;
}

/// 
/// @brief
///
void VideoWidgetSurface::stop()
{
  m_currentFrame = QVideoFrame(); ///< get presentation of a frame of video data
  m_targetRect   = QRect();

  QAbstractVideoSurface::stop();  ///< Stops a video surface presenting frames and 
                                  ///< releases any resources acquired in start(). 
  m_widget->update();		  ///< Updates the widget unless updates are disabled or the widget is hidden
}

/// 
/// @brief repaint the video frame
/// @param frame 
///
/// @return 
///
bool VideoWidgetSurface::present(const QVideoFrame &frame)
{
  if (surfaceFormat().pixelFormat() != frame.pixelFormat()
      || surfaceFormat().frameSize() != frame.size()) {
    setError(IncorrectFormatError);
    stop();

    return false;
  }
   
  m_currentFrame = frame;	  ///< get current frame
  m_widget->repaint(m_targetRect);  ///< repaint the data
  return true;
}

/// 
/// @brief modify video rectangle 
///
void VideoWidgetSurface::updateVideoRect()
{
  ///< Returns a suggested size in pixels for the video stream
  QSize size = surfaceFormat().sizeHint();
  
  size.scale(m_widget->size().boundedTo(size), Qt::KeepAspectRatio); ///< scaling

  m_targetRect = QRect(QPoint(0, 0), size);                        ///< set rectangle
  m_targetRect.moveCenter(m_widget->rect().center());
}

/// 
/// @brief 
/// @param painter 
///
void VideoWidgetSurface::paint(QPainter *painter)
{
  if (m_currentFrame.map(QAbstractVideoBuffer::ReadOnly)) {
    ///< check whether video frame memory mode is in without writing 
    const QTransform oldTransform = painter->transform();

    if (surfaceFormat().scanLineDirection() == QVideoSurfaceFormat::BottomToTop) {
      painter->scale(1, -1);
      painter->translate(0, -m_widget->height());
    }

    /// draw image
    QImage image(m_currentFrame.bits(), m_currentFrame.width(), m_currentFrame.height(),
                 m_currentFrame.bytesPerLine(), m_imageFormat);

    painter->drawImage(m_targetRect, image, m_sourceRect);
    painter->setTransform(oldTransform);
    m_currentFrame.unmap();
  }
}
