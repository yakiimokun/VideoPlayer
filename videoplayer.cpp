///
/// @file   mainwindow.cpp
/// @author Kosuke Imoji <imoji@imoji-pc>
/// @date   Sun May 19 16:15:12 2013
/// 
/// @brief  
/// 
/// 
///
#include "videoplayer.h"
#include "ui_videoplayer.h"
#include "videowidget.h"

#include <QtMultimedia/QVideoSurfaceFormat>
#include <QFileDialog>
#include <QDir>
#include <QAbstractVideoSurface>

#include <iostream>

/// 
/// @brief constructor
/// @param parent 
///
VideoPlayer::VideoPlayer(QWidget *parent) 
  :QMainWindow(parent)
  ,ui(new Ui::VideoPlayer)
  ,m_surface(0) 
{
  ui->setupUi(this);

  QObject::connect(&m_movie, SIGNAL(stateChanged(FFmpegMovie::MovieState)), 
           ui->centralWidget, SLOT(movieStateChanged(FFmpegMovie::MovieState)));
  QObject::connect(&m_movie, SIGNAL(frameChanged(int)), ui->centralWidget, SLOT(frameChanged(int)));

  m_videoWidget       = ui->m_VideoWidget;
  m_surface           = m_videoWidget->videoSurface();
  m_openButton        = ui->m_openButton;
  m_playButton        = ui->m_playButton;
  m_fastforwardButton = ui->m_fastforwardButton;
  m_rewindButton      = ui->m_rewindButton;
  m_progressSlider    = ui->m_progressSlider;

  m_progressSlider->setRange(0, 0);

  m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
  m_fastforwardButton->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
  m_rewindButton->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));

  QSize size(m_videoWidget->width(), m_videoWidget->height());
  m_movie.setScaledSize(size);
}

/// 
/// @brief destructor
///
VideoPlayer::~VideoPlayer()
{
  delete ui;
}

/// 
/// @brief open the file
///
void VideoPlayer::openFile()
{
    QStringList supportedFormats;

//    foreach (QString fmt, QMovie::supportedFormats()) {
//        supportedFormats << fmt;
//    }
//    supportedFormats << "mpg";

//    QString filter = "Images (";
//    foreach ( QString fmt, supportedFormats) {
//        qDebug() << fmt;
//        filter.append(QString("*. ").arg(fmt));
//    }

//    filter.append(")");

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Movie"),
                                                   QDir::homePath(), 0);

    if (!fileName.isEmpty()) {
        m_surface->stop();

        m_movie.setFileName(fileName);

        m_progressSlider->setMaximum(m_movie.frameCount());
        m_movie.jumpToFrame(0);

        m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        m_playButton->setEnabled(true);
        m_fastforwardButton->setEnabled(true);
        m_rewindButton->setEnabled(true);
    }
}
 
/// 
/// @brief play the video stream
///
void VideoPlayer::play()
{
  switch(m_movie.state()) {
  case QMovie::NotRunning:
    m_movie.start();
    break;
  case QMovie::Paused:
    m_movie.setPaused(false);
    break;
  case QMovie::Running:
    m_movie.setPaused(true);
    break;
  }
}

/// 
///
/// @param state 
///
void VideoPlayer::movieStateChanged(FFmpegMovie::MovieState state)
{
  switch(state) {
  case FFmpegMovie::NotRunning:
  case FFmpegMovie::Paused:
    m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    break;
  case FFmpegMovie::Running:
    m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    break;
  }
}

/// 
/// 
/// @param frame 
///
void VideoPlayer::frameChanged(int frame)
{
  if (!presentImage(m_movie.currentImage())) {
    m_movie.stop();
    m_playButton->setEnabled(false);
    m_progressSlider->setMaximum(0);
  } else {
    m_progressSlider->setValue(frame);
    std::cout << "frame : " << frame << std::endl;
  }
}

/// 
/// @brief 
/// @param frame 
///
void VideoPlayer::setPosition(int frame)
{
  m_movie.jumpToFrame(frame);
}

/// 
/// @brief 
/// @param image 
///
/// @return 
///
bool VideoPlayer::presentImage(const QImage &image)
{
  QVideoFrame frame(image);

  if (!frame.isValid()) {
    std::cout << "Frame is invalid !!" << std::endl;
    return false;
  }

  QVideoSurfaceFormat currentFormat = m_surface->surfaceFormat();

  if (frame.pixelFormat() != currentFormat.pixelFormat() || frame.size() != currentFormat.frameSize()) {
    QVideoSurfaceFormat format(frame.size(), frame.pixelFormat());

    if (!m_surface->start(format)) {
      return false;
    }
  }

  if (!m_surface->present(frame)) {
    m_surface->stop();
    return false;
  }

  return true;
}
