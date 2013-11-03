///
/// @file   FFmpegMovie.cpp
/// @author Kosuke Imoji <imoji@imoji-pc>
/// @date   Sat Jun 15 23:57:58 2013
/// 
/// @brief  
/// 
/// 
///
#include <FFmpegMovie.h>
#include <QThread>
#include <QImage>
#include <iostream>

extern "C" {
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/avcodec.h>
#include <libavutil/imgutils.h>
}

/// 
/// @brief constructor
///
FFmpegMovie::FFmpegMovie()
:m_codecContext(NULL)
,m_frameNumber(0)
,m_duration(0)
,m_videoStreamIndex(-1)
{
  avcodec_register_all();
  av_register_all();
  avfilter_register_all();

  m_thread = new QThread(this);
  connect(m_thread, SIGNAL(started()), this, SLOT(procThread()));
  connect(m_thread, SIGNAL(finished()), this, SLOT(deleteThread()));
}

/// 
///
/// @brief destructor
/// @return 
///
FFmpegMovie::~FFmpegMovie()
{
  delete m_thread;
}

/// 
///
/// @param name 
///
void FFmpegMovie::setFileName(const QString& name)
{
  int ret;

  m_formatContext = avformat_alloc_context();
  if (m_formatContext == NULL) {
    std::cout << "can't alloc AVFormatContext" << std::endl;
    return;
  }

  // read a file
  ret = avformat_open_input(&m_formatContext, name.toLocal8Bit().data(), NULL, NULL);
  if (ret < 0) {
    std::cout << "can't open input file" << std::endl;
    return;
  }

  ret = avformat_find_stream_info(m_formatContext, NULL);
  if (ret < 0) {
      std::cout << "can't find stream info" << std::endl;
      return;
  }

  AVCodec* dec;

  /// can't find video stream
  ret = av_find_best_stream(m_formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
  if (ret < 0) {
    std::cout << "can't find video stream" << std::endl;
    return;
  }

  m_videoStreamIndex = ret;
  std::cout << "stream : " << m_videoStreamIndex << std::endl;

  m_codecContext = m_formatContext->streams[m_videoStreamIndex]->codec;

  if ((ret = avcodec_open2(m_codecContext, dec, NULL)) < 0) {
    std::cout << "can't open codec(decoder)" << std::endl;
    return;
  }

  ret = initFilters();
  if (ret < 0) {
    std::cout << "can't initialize context" << std::endl;
    return;
  }

  m_duration = (unsigned long)(m_formatContext->duration / AV_TIME_BASE);
  std::cout << "Duration : " << m_duration << std::endl;

  moveToThread(m_thread);
  m_thread->start();
}

/// 
///
/// @brief 
/// @return 
///
int FFmpegMovie::frameCount() const
{
  return m_frameNumber;
}

bool FFmpegMovie::jumpToFrame(int frameNumber)
{
  if (frameNumber){};
  return true;
}

/// 
///
/// @brief  
/// @return 
/// 
FFmpegMovie::MovieState FFmpegMovie::state() const
{
  return FFmpegMovie::NotRunning;
}

/// 
/// @brief
///
void FFmpegMovie::start()
{
}

/// 
///
/// @param paused 
/// 
void FFmpegMovie::setPaused(bool paused)
{
}

/// 
///
///
void FFmpegMovie::stop()
{
}

/// 
/// @brief procedure thread
///
void FFmpegMovie::procThread()
{
  AVPacket           packet;
  AVFrame            inputFrame;
  int                isFinish;
  int                ret;

  m_frameNumber = 1;

  /// decode data
  while(1) {
    AVFilterBufferRef *picref;

    if ((ret = av_read_frame(m_formatContext, &packet)) < 0) {
      break;
    }
    
    if (packet.stream_index != m_videoStreamIndex) {
      av_free_packet(&packet);
      continue;
    }

    avcodec_get_frame_defaults(&inputFrame);
    isFinish = 0;

    ret = avcodec_decode_video2(m_codecContext, &inputFrame, &isFinish, &packet);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "error decoding video !!\n");
        break;
    }

    if (!isFinish) {
      continue;
    }

    inputFrame.pts = av_frame_get_best_effort_timestamp(&inputFrame);

    // push decoded frame into the filtergraph
    if (av_buffersrc_add_frame(m_sourceContext, &inputFrame, 0) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
        break;
    }

    // pull filtered pictures from the filtergraph
    while(1) {
      ret = av_buffersink_get_buffer_ref(m_sinkContext, &picref, 0);

      if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
          break;
      }

      if (ret < 0) {
          return;
      }

      if (picref) {
          displayFrame(picref, m_sinkContext->inputs[0]->time_base);
          avfilter_unref_bufferp(&picref);
      }
    }
  }

FFmepMovie_End:
  avfilter_graph_free(&m_filterGraph);
  if (m_codecContext) {
    avcodec_close(m_codecContext);
  }

  avformat_close_input(&m_formatContext);

  if (ret < 0 && ret != AVERROR_EOF) {
    char buf[1024];
    av_strerror(ret, buf, sizeof(buf));
    fprintf(stderr, "Error occurred: %s\n", buf);
  }
}

/// 
///
/// @brief 
/// @return 
///
int FFmpegMovie::searchVideoStream()
{
  /// search video stream
  for(int i = 0; m_formatContext->nb_streams; i++) {
    if (m_formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
      /// find video stream
      return i;
    }
  }

  return -1;
}

///
///
/// @param context
/// @param index
///
/// @return stream index
/// @retval 0 means error
/// @retval non 0 is OK
///
int FFmpegMovie::initFilters()
{
  char args[512];
  int ret;
  AVFilter *buffersrc           = avfilter_get_by_name("buffer");
  AVFilter *buffersink          = avfilter_get_by_name("ffbuffersink");
  AVFilterInOut *outputs        = avfilter_inout_alloc();
  AVFilterInOut *inputs         = avfilter_inout_alloc();
  enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_GRAY8, AV_PIX_FMT_NONE };
  AVBufferSinkParams *buffersink_params;
  char filter_descr[60];

  m_filterGraph = avfilter_graph_alloc();

  /* buffer video source: the decoded frames from the decoder will be inserted here. */
  snprintf(args, sizeof(args),
           "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
           m_codecContext->width, m_codecContext->height, m_codecContext->pix_fmt,
           m_codecContext->time_base.num, m_codecContext->time_base.den,
           m_codecContext->sample_aspect_ratio.num, m_codecContext->sample_aspect_ratio.den);

  std::cout << "video_size=" << m_codecContext->width << "x" << m_codecContext->height 
	    << ":pix_fmt="   << m_codecContext->pix_fmt 
	    << ":time_base=" << m_codecContext->time_base.num << "/" << m_codecContext->time_base.den
	    << ":pixel_aspect=" << m_codecContext->sample_aspect_ratio.num 
	    << "/" << m_codecContext->sample_aspect_ratio.den << std::endl;

  ret = avfilter_graph_create_filter(&m_sourceContext, buffersrc, "in",
                                     args, NULL, m_filterGraph);
  if (ret < 0) {
      av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source\n");
      return ret;
  }

  /* buffer video sink: to terminate the filter chain. */
  buffersink_params = av_buffersink_params_alloc();

  buffersink_params->pixel_fmts = pix_fmts;
  ret = avfilter_graph_create_filter(&m_sinkContext, buffersink, "out",
                                     NULL, buffersink_params, m_filterGraph);

  av_free(buffersink_params);

  if (ret < 0) {
      av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink\n");
      return ret;
  }

  /* Endpoints for the filter graph. */
  outputs->name       = av_strdup("in");
  outputs->filter_ctx = m_sourceContext;
  outputs->pad_idx    = 0;
  outputs->next       = NULL;
  inputs->name       = av_strdup("out");
  inputs->filter_ctx = m_sinkContext;
  inputs->pad_idx    = 0;
  inputs->next       = NULL;

  snprintf(filter_descr, sizeof(filter_descr), "scale=%d:%d",
	   m_scaledSize.width(), m_scaledSize.height());

  if ((ret = avfilter_graph_parse(m_filterGraph, filter_descr,
                                  &inputs, &outputs, NULL)) < 0)
      return ret;

  if ((ret = avfilter_graph_config(m_filterGraph, NULL)) < 0)
      return ret;

  return 0;
}

/// 
///
/// @param AVPixelFormat 
///
/// @return 
///
QImage::Format FFmpegMovie::convertFormat(const enum AVPixelFormat format)
{
  switch(format) {
  case AV_PIX_FMT_ARGB:
    return QImage::Format_ARGB32;
  case AV_PIX_FMT_RGB24:
    return QImage::Format_RGB888;
  case AV_PIX_FMT_GRAY8:
    return QImage::Format_Mono;
  default:
    return QImage::Format_Invalid;
  }
}

/// 
/// @brief delete thread
///
void FFmpegMovie::deleteThread()
{
}

/// 
/// @brief
/// @param state 
///
QImage FFmpegMovie::currentImage() const
{
  return m_currentImage;
}
 
/// 
/// @brief 
/// @param frameNumber 
void FFmpegMovie::stateChangedEvent(FFmpegMovie::MovieState state)
{
    emit stateChanged(state);
}

/// 
/// @brief
///
void FFmpegMovie::displayFrame(AVFilterBufferRef *picref, AVRational time_base)
{
  static int64_t last_pts = AV_NOPTS_VALUE;
  int64_t        delay;

  if (picref->pts != AV_NOPTS_VALUE) {
    if (last_pts != AV_NOPTS_VALUE) {
      delay = av_rescale_q(picref->pts - last_pts, time_base, AV_TIME_BASE_Q);

      if (delay > 0 && delay < 1000000)
	usleep(delay);
    }

    last_pts = picref->pts;
  }

  int w = picref->video->w;
  int h = picref->video->h;
  int linesize = picref->linesize[0];

  QImage *image = new QImage(reinterpret_cast<uchar*>(picref->data[0]), w, h, linesize, QImage::Format_Mono);

  m_currentImage = QImage(*image);

  emit frameChanged(m_frameNumber);
  m_frameNumber++;

  delete image;
}

/// 
/// @brief set scaled size
/// @param size 
///
void FFmpegMovie::setScaledSize(const QSize& size)
{
  m_scaledSize = size;
}
