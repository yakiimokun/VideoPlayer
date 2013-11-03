///
/// @file   mainwindow.h
/// @author Kosuke Imoji <imoji@imoji-pc>
/// @date   Sat May 25 23:21:07 2013
/// 
/// @brief  
/// 
/// 
///
#pragma once

#include <QtGui/QMainWindow>
#include <QtGui/QWidget>
#include "FFmpegMovie.h"

namespace Ui {
class VideoPlayer;
}

class QPushButton;
class QSlider;
class VideoWidget;
class QAbstractVideoSurface;

class VideoPlayer : public QMainWindow
{
  Q_OBJECT
    
public:
  explicit VideoPlayer(QWidget *parent = 0);
  virtual ~VideoPlayer();

public slots:
  void openFile();
  void play();

private slots:
  void movieStateChanged(FFmpegMovie::MovieState state);
  void frameChanged(int frame);
  void setPosition(int frame);
    
private:
  bool presentImage(const QImage &image);

  Ui::VideoPlayer*        ui;
  VideoWidget*            m_videoWidget;
  QAbstractVideoSurface*  m_surface;
  FFmpegMovie             m_movie;
  QPushButton*            m_openButton;
  QPushButton*            m_playButton;
  QPushButton*            m_fastforwardButton;
  QPushButton*            m_rewindButton;
  QSlider*                m_progressSlider;
};
