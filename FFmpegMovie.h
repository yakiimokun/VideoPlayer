///
/// @file   FFmpegMovie.h
/// @author Kosuke Imoji <imoji@imoji-pc>
/// @date   Sat Jun 15 23:30:56 2013
/// 
/// @brief  
/// 
/// 
///
#ifndef __FFMPEGMOVIE_H__
#define __FFMPEGMOVIE_H__

#ifdef __cplusplus
#define __STDC_CONSTANT_MACROS
#ifdef _STDINT_H
#undef _STDINT_H
#endif
#include <stdint.h>
#endif

extern "C" {
#include <libavutil/avstring.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/avfiltergraph.h>
}

#include <QObject>
#include <QtGui/QMovie>
#include <QString>
#include <QSize>

class QThread;
class QImage;

class FFmpegMovie : public QObject
{
  Q_OBJECT

public:
  enum MovieState {
    NotRunning,
    Paused,
    Running
  };

  FFmpegMovie();
  ~FFmpegMovie();

  virtual void setFileName(const QString& name);
  virtual int frameCount() const;
  virtual bool jumpToFrame(int frameNumber);
  virtual MovieState state() const;
  virtual QImage currentImage() const;
  virtual void setScaledSize(const QSize& size);

public slots:
  void start();
  void setPaused(bool paused);
  void stop();
  void procThread();
  void deleteThread();

signals:
  void stateChanged(FFmpegMovie::MovieState state);
  void frameChanged(int frameNumber);

protected:
  virtual void stateChangedEvent(FFmpegMovie::MovieState state);

private:
  int searchVideoStream();
  int setAVCodec(int* index);
  int initFilters();
  QImage::Format convertFormat(const enum AVPixelFormat format);
  void displayFrame(AVFilterBufferRef *picref, AVRational time_base);

  static bool      m_initflag;
  AVFormatContext* m_formatContext;
  AVFilterContext* m_sourceContext;
  AVFilterContext* m_sinkContext;
  AVCodecContext*  m_codecContext;
  AVFilterGraph*   m_filterGraph;
  QImage           m_currentImage;
  QThread*         m_thread;
  int              m_frameNumber;
  unsigned long    m_duration;
  QSize            m_scaledSize;
  int              m_videoStreamIndex;
};

#endif
