/*
  ==============================================================================

    FFmpegWrapper.h
    Created: 10 Oct 2018 8:20:09pm
    Author:  0214t

  ==============================================================================
*/

#pragma once

// Standerd Library
#include <iostream>
#include <functional>
#include <memory>
#include <queue>
#include <thread>
#include <mutex>
#include <exception>
#include <cassert>
#include <type_traits>

///debug
#include <Windows.h>
#include <sstream>
#include <string>
///debug

// FFmpeg library
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "swscale.lib")

namespace FF {
	class FFmpeg {
	public:
		struct deletorAVFormatContext {
			void operator()(AVFormatContext *ptr) const {
				avformat_free_context(ptr);
			}
		};
		struct deletorAVCodecContext {
			void operator()(AVCodecContext *ptr) const {
				avcodec_free_context(&ptr);
			}
		};
		struct deletorAVFrame {
			void operator()(AVFrame *ptr) const {
				av_frame_free(&ptr);
				assert(nullptr == ptr);
			}
		};
		struct deletorAVPacket {
			void operator()(AVPacket *ptr) const {
				av_packet_free(&ptr);
			}
		};

		using spAVFormatContext = std::unique_ptr<AVFormatContext, deletorAVFormatContext>;
		using spAVCodecContext = std::unique_ptr<AVCodecContext, deletorAVCodecContext>;
		using spAVFrame = std::unique_ptr<AVFrame, deletorAVFrame>;
		using spAVPacket = std::unique_ptr<AVPacket, deletorAVPacket>;

	protected:
		spAVFormatContext pFormatContext;
		spAVCodecContext pCodecContext;
		spAVFrame pFrame;
		spAVPacket pPacket;
		std::queue<spAVFrame> frameQueue;
		std::mutex frameQueueMutex;
		std::thread decoder;

		bool isOpen = false;
		size_t frameQueueMaxSize = 5;
		int streamIndex;
		int64_t seekPos = 0;
		bool seekReq = false;

		void close() {
			if (isOpen) {
				isOpen = false;
				decoder.join();
			}

			if (nullptr != pFormatContext.get()) {
				AVFormatContext *tmpPFormatContext = pFormatContext.release();
				avformat_close_input(&tmpPFormatContext);
				pFormatContext.reset(tmpPFormatContext);
			}
		}

		int decodeStream() {
			if (!isOpen)
				return -1;

			{
				std::lock_guard<std::mutex> lock(frameQueueMutex);

				if (frameQueue.size() >= frameQueueMaxSize) return 0;

				if (seekReq) {
					if (av_seek_frame(pFormatContext.get(), -1, seekPos, 0) < 0)
						return -1;
					avcodec_flush_buffers(pCodecContext.get());
					while (!frameQueue.empty()) frameQueue.pop();
					seekReq = false;
				}
			}

			while (av_read_frame(pFormatContext.get(), pPacket.get()) >= 0) {
				if (pPacket->stream_index == streamIndex) {
					if (decodePacket() >= 0) {
						av_packet_unref(pPacket.get());
						return 0;
					}
					else {
						return -1;
					}
				}
			}

			return -1;
		}

		int decodePacket()
		{
			int response = avcodec_send_packet(pCodecContext.get(), pPacket.get());
			if (response < 0) {
				return response;
			}

			while (response >= 0)
			{
				response = avcodec_receive_frame(pCodecContext.get(), pFrame.get());
				if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
					break;
				}
				else if (response < 0) {
					return response;
				}
				if (response >= 0) {
					std::lock_guard<std::mutex> lock(frameQueueMutex);
					AVFrame *tmpFrame = av_frame_alloc();
					av_frame_ref(tmpFrame, pFrame.get());
					frameQueue.push(spAVFrame(tmpFrame));
					av_frame_unref(pFrame.get());
				}
			}

			return 0;
		}

	public:
		class FFmpegException : public std::exception {
		public:
			enum ErrorType {
				UnknownError,
				AllocateError,
				PathError,
				FileError,
				ProcessError,
				UnexpectError
			};
			FFmpegException(ErrorType err_type, std::string err) :
				std::exception(),
				err(err),
				err_type(err_type)
			{

			}
			const char* what() const override {
				return err.c_str();
			}
			const ErrorType type() const {
				return err_type;
			}
		private:
			ErrorType err_type;
			std::string err;
		};

		virtual AVMediaType getAVMediaType() = 0;

		FFmpeg() {}

		virtual ~FFmpeg() {
			close();
		}

		void open(std::string path) {

			close();

			pFormatContext.reset(avformat_alloc_context());
			if (!pFormatContext)
				throw FFmpegException(FFmpegException::AllocateError, "ERROR could not allocate memory for Format Context");

			AVFormatContext *tmp_pFormatContext = pFormatContext.release();
			if (avformat_open_input(&tmp_pFormatContext, path.c_str(), NULL, NULL) != 0)
				throw FFmpegException(FFmpegException::PathError, "ERROR could not open the file");
			else pFormatContext.reset(tmp_pFormatContext);
			if (avformat_find_stream_info(pFormatContext.get(), NULL) < 0)
				throw FFmpegException(FFmpegException::FileError, "ERROR could not get the stream info");

			AVCodec *pCodec = NULL;
			AVCodecParameters *pCodecParameters = NULL;
			streamIndex = AVMEDIA_TYPE_UNKNOWN;
			for (unsigned int i = 0; i < pFormatContext->nb_streams; i++)
			{
				AVCodecParameters *pLocalCodecParameters = NULL;
				pLocalCodecParameters = pFormatContext->streams[i]->codecpar;
				AVCodec *pLocalCodec = NULL;
				pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);
				if (pLocalCodec == NULL) throw FFmpegException(FFmpegException::FileError, "ERROR unsupported codec!");
				if (pLocalCodecParameters->codec_type == getAVMediaType()) {
					streamIndex = i;
					pCodec = pLocalCodec;
					pCodecParameters = pLocalCodecParameters;
				}
			}
			if (streamIndex == AVMEDIA_TYPE_UNKNOWN)
				throw FFmpegException(FFmpegException::FileError, "ERROR could not get the stream index");

			pCodecContext.reset(avcodec_alloc_context3(pCodec));
			if (!pCodecContext) throw FFmpegException(FFmpegException::AllocateError, "failed to allocated memory for AVCodecContext");
			if (avcodec_parameters_to_context(pCodecContext.get(), pCodecParameters) < 0)
				throw FFmpegException(FFmpegException::ProcessError, "failed to copy codec params to codec context");
			if (avcodec_open2(pCodecContext.get(), pCodec, NULL) < 0)
				throw FFmpegException(FFmpegException::ProcessError, "failed to open codec through avcodec_open2");

			pFrame.reset(av_frame_alloc());
			if (!pFrame)
				throw FFmpegException(FFmpegException::AllocateError, "failed to allocated memory for AVFrame");
			pPacket.reset(av_packet_alloc());
			if (!pPacket)
				throw FFmpegException(FFmpegException::AllocateError, "failed to allocated memory for AVPacket");

			isOpen = true;

			decoder = std::thread([&]() {
				while (decodeStream() >= 0);
			});
		}

		spAVFrame nextFrame() {
			std::lock_guard<std::mutex> lock(frameQueueMutex);
			spAVFrame result{ nullptr };
			if (!frameQueue.empty()) {
				result = std::move(frameQueue.front());
				frameQueue.pop();
			}
			return result;
		}

		inline void seek(int64_t time) { 
			std::lock_guard<std::mutex> lock(frameQueueMutex); 
			seekReq = true;
			seekPos = time;
		}
	};

	class Video : public FFmpeg {
	private:
		struct deletorSwsContext {
			void operator()(SwsContext *ptr) const {
				sws_freeContext(ptr);
			}
		};

		using spSwsContext = std::unique_ptr<SwsContext, deletorSwsContext>;

		spAVFrame convertedFrame;
		spSwsContext pSWScaleContext;

		bool changeFlag;

		AVMediaType getAVMediaType() override { return AVMediaType::AVMEDIA_TYPE_VIDEO; }

		int convert(AVFrame *frame)
		{
			if (
				changeFlag || (!pSWScaleContext) ||
				convertedFrame->width != frame->width ||
				convertedFrame->height != frame->height
				) {
				av_frame_copy_props(convertedFrame.get(), frame);
				convertedFrame->width = frame->width;
				convertedFrame->height = frame->height;

				if (av_frame_get_buffer(convertedFrame.get(), 0) != 0) return -1;

				pSWScaleContext.reset(
					sws_getContext(
						frame->width, frame->height, (AVPixelFormat)(frame->format),
						convertedFrame->width, convertedFrame->height, (AVPixelFormat)(convertedFrame->format),
						0, 0, 0, 0
					)
				);
				if (!pSWScaleContext) return -1;
				changeFlag = false;
			}

			int ret = sws_scale(
				pSWScaleContext.get(),
				frame->data, frame->linesize, 0, frame->height,
				convertedFrame->data, convertedFrame->linesize
			);
			if (ret < 0) return -1;

			return 0;
		}

	public:
		Video()
		{
			convertedFrame.reset(av_frame_alloc());
			convertedFrame->format = AVPixelFormat::AV_PIX_FMT_RGB24;
			changeFlag = true;
		}

		virtual ~Video() {

		}

		uint8_t *nextFrame() {
			spAVFrame tmpFrame = this->FFmpeg::nextFrame();
			if (tmpFrame.get() == nullptr) return nullptr;
			if (convert(tmpFrame.get()) < 0) return nullptr;
			return convertedFrame->data[0];
		}

		void setPixelFormat(AVPixelFormat pixelFormat) {
			convertedFrame->format = (AVPixelFormat)pixelFormat;
			changeFlag = true;
		}

		inline AVPixelFormat getPixelFormat() const { return (AVPixelFormat)convertedFrame->format; }
		inline int32_t getFrameWidth() const { return convertedFrame->width; }
		inline int32_t getFrameHeight() const { return convertedFrame->height; }
	};


	class Audio : public FFmpeg {
	private:
		struct deletorSwrContext {
			void operator()(SwrContext *ptr) const {
				swr_free(&ptr);
			}
		};

		using spSwrContext = std::unique_ptr<SwrContext, deletorSwrContext>;

		spAVFrame convertedFrame;
		spSwrContext pSwrContext;

		bool changeFlag;

		AVMediaType getAVMediaType() { return AVMediaType::AVMEDIA_TYPE_AUDIO; }

		int convert(AVFrame *frame)
		{
			if (
				changeFlag || (!pSwrContext) ||
				convertedFrame->nb_samples != frame->nb_samples ||
				convertedFrame->channel_layout != frame->channel_layout
				) {
				av_frame_copy_props(convertedFrame.get(), frame);
				convertedFrame->nb_samples = frame->nb_samples;
				convertedFrame->channel_layout = frame->channel_layout;

				if (av_frame_get_buffer(convertedFrame.get(), 0) != 0) return -1;

				pSwrContext.reset(swr_alloc());
				if (!pSwrContext) return -1;
				av_opt_set_int(pSwrContext.get(), "in_channel_layout", frame->channel_layout, 0);
				av_opt_set_int(pSwrContext.get(), "out_channel_layout", convertedFrame->channel_layout, 0);
				av_opt_set_int(pSwrContext.get(), "in_sample_rate", frame->sample_rate, 0);
				av_opt_set_int(pSwrContext.get(), "out_sample_rate", convertedFrame->sample_rate, 0);
				av_opt_set_sample_fmt(pSwrContext.get(), "in_sample_fmt", (AVSampleFormat)frame->format, 0);
				av_opt_set_sample_fmt(pSwrContext.get(), "out_sample_fmt", (AVSampleFormat)convertedFrame->format, 0);
				int ret = swr_init(pSwrContext.get());
				if (ret < 0) return -1;

				changeFlag = false;
			}

			int ret = swr_convert(
				pSwrContext.get(),
				(uint8_t**)convertedFrame->extended_data, convertedFrame->nb_samples,
				(const uint8_t**)frame->extended_data, frame->nb_samples
			);
			if (ret < 0) return -1;

			return 0;
		}

	public:
		Audio()
		{
			convertedFrame.reset(av_frame_alloc());
			convertedFrame->format = AVSampleFormat::AV_SAMPLE_FMT_FLT;
			changeFlag = true;
		}

		virtual ~Audio() {

		}

		uint8_t *nextFrame() {
			spAVFrame tmpFrame = this->FFmpeg::nextFrame();
			if (tmpFrame.get() == nullptr) return nullptr;
			if (convert(tmpFrame.get()) < 0) return nullptr;
			return (*(convertedFrame->extended_data));
		}

		inline void setSampleFormat(AVSampleFormat sampleFormat) {
			convertedFrame->format = sampleFormat;
			changeFlag = true;
		}

		inline int32_t getFrameLength() const { return convertedFrame->nb_samples; }
		inline int32_t getFrameChannelSize() const { return convertedFrame->channels; }
		inline int32_t getSampleRate() const { return convertedFrame->sample_rate; }
	};
}