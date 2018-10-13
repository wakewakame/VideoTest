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
		std::thread decode;

		bool isOpen = false;
		size_t frameQueueMaxSize = 5;
		int streamIndex;
		int response;
		int64_t seek_pos = -1;

		void close() {
			if (isOpen) {
				isOpen = false;
				decode.join();
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

			if (frameQueue.size() >= frameQueueMaxSize) return 0;

			if (seek_pos >= 0) {
				std::lock_guard<std::mutex> lock(frameQueueMutex);
				if (av_seek_frame(pFormatContext.get(), -1, seek_pos, 0) < 0)
					return -1;
				avcodec_flush_buffers(pCodecContext.get());
				seek_pos = -1;
			}

			while (av_read_frame(pFormatContext.get(), pPacket.get()) >= 0) {
				if (pPacket->stream_index == streamIndex) {
					response = decodePacket();
					if (response >= 0) {
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
		class FFmpegException : std::exception {
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
			for (int i = 0; i < pFormatContext->nb_streams; i++)
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

			response = 0;

			isOpen = true;

			decode = std::thread([&]() {
				while (decodeStream() >= 0);
			});
		}

		spAVFrame nextFrame() {
			spAVFrame result{ nullptr };
			if (0 < frameQueue.size()) {
				std::lock_guard<std::mutex> lock(frameQueueMutex);
				result = std::move(frameQueue.front());
				frameQueue.pop();
			}
			return result;
		}

		inline void seek(int64_t time) { seek_pos = time; }
	};

	class Video : public FFmpeg {
	private:
		using spSwsContext = std::unique_ptr<SwsContext, std::function<void(SwsContext*)>>;

		spSwsContext pSWScaleContext;

		AVPixelFormat pixelFormat;
		std::unique_ptr<uint8_t[]> frameData;
		uint8_t frameChannelSize;
		uint32_t frameWidth;
		uint32_t frameHeight;

		bool changeFlag;

		AVMediaType getAVMediaType() { return AVMediaType::AVMEDIA_TYPE_VIDEO; }

		int convert(AVFrame *pFrame)
		{

			if (pixelFormat == (AVPixelFormat)PixelFormat::NONE) return -1;

			if (
				frameWidth != pFrame->width ||
				frameHeight != pFrame->height ||
				changeFlag
				) {
				frameWidth = pFrame->width;
				frameHeight = pFrame->height;
				frameData.reset(new uint8_t[frameWidth * frameHeight * frameChannelSize]);

				pSWScaleContext.reset(
					sws_getContext(
						pFrame->width, pFrame->height, (AVPixelFormat)(pFrame->format),
						pFrame->width, pFrame->height, pixelFormat,
						0, 0, 0, 0
					)
				);
				if (!pSWScaleContext) return -1;
				changeFlag = false;
			}

			uint8_t* inData[1] = { frameData.get() };
			int inStride[1] = { frameWidth * frameChannelSize };
			int ret = sws_scale(
				pSWScaleContext.get(),
				pFrame->data, pFrame->linesize, 0, pFrame->height,
				inData, inStride
			);
			if (ret < 0) return -1;

			return 0;
		}

	public:
		enum PixelFormat {
			NONE = AV_PIX_FMT_NONE,
			RGB24 = AV_PIX_FMT_RGB24,
			BGR24 = AV_PIX_FMT_BGR24,
			ARGB32 = AV_PIX_FMT_ARGB,
			ABGR32 = AV_PIX_FMT_ABGR,
			RGBA32 = AV_PIX_FMT_RGBA,
			BGRA32 = AV_PIX_FMT_BGRA
		};

		Video() :
			pixelFormat(AV_PIX_FMT_RGB24),
			frameChannelSize(0),
			frameWidth(0),
			frameHeight(0),
			changeFlag(true)
		{
			pSWScaleContext = spSwsContext(
				nullptr,
				[](SwsContext *ptr) {
				sws_freeContext(ptr);
			}
			);
		}

		virtual ~Video() {

		}

		uint8_t *nextFrame() {
			spAVFrame tmpFrame = this->FFmpeg::nextFrame();
			if (tmpFrame.get() == nullptr) return nullptr;
			convert(tmpFrame.get());
			return frameData.get();
		}

		int setPixelFormat(PixelFormat pixelFormat) {
			this->pixelFormat = (AVPixelFormat)pixelFormat;
			switch (pixelFormat) {
			case PixelFormat::NONE:
				frameChannelSize = 0;
				break;
			case PixelFormat::RGB24:
			case PixelFormat::BGR24:
				frameChannelSize = 3;
				break;
			case PixelFormat::ARGB32:
			case PixelFormat::ABGR32:
			case PixelFormat::RGBA32:
			case PixelFormat::BGRA32:
				frameChannelSize = 4;
				break;
			default:
				this->pixelFormat = (AVPixelFormat)PixelFormat::NONE;
				frameChannelSize = 0;
				return -1;
				break;
			}

			changeFlag = true;

			return 0;
		}

		inline PixelFormat getPixelFormat() const { return (PixelFormat)pixelFormat; }
		inline uint8_t getFrameChannelSize() const { return frameChannelSize; }
		inline uint32_t getFrameWidth() const { return frameWidth; }
		inline uint32_t getFrameHeight() const { return frameHeight; }
	};


	class Audio : public FFmpeg {
	private:
		using spSwrContext = std::unique_ptr<SwrContext, std::function<void(SwrContext*)>>;

		spSwrContext pSwrContext;

		std::unique_ptr<float_t[]> frameData;
		uint32_t frameLength;
		uint8_t frameChannelSize;
		double_t sampleRate;

		bool changeFlag;

		AVMediaType getAVMediaType() { return AVMediaType::AVMEDIA_TYPE_AUDIO; }

		int convert(AVFrame *pFrame)
		{
			if (
				frameLength != pFrame->nb_samples ||
				frameChannelSize != pFrame->channels ||
				changeFlag
				) {
				frameLength = pFrame->nb_samples;
				frameChannelSize = pFrame->channels;
				frameData.reset(new float_t[frameLength * frameChannelSize]);

				pSwrContext.reset(swr_alloc());
				if (!pSwrContext) return -1;
				av_opt_set_int(pSwrContext.get(), "in_channel_layout", pFrame->channel_layout, 0);
				av_opt_set_int(pSwrContext.get(), "out_channel_layout", pFrame->channel_layout, 0);
				av_opt_set_int(pSwrContext.get(), "in_sample_rate", pFrame->sample_rate, 0);
				av_opt_set_int(pSwrContext.get(), "out_sample_rate", sampleRate, 0);
				av_opt_set_sample_fmt(pSwrContext.get(), "in_sample_fmt", (AVSampleFormat)pFrame->format, 0);
				av_opt_set_sample_fmt(pSwrContext.get(), "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0);
				int ret = swr_init(pSwrContext.get());
				if (ret < 0) return -1;

				changeFlag = false;
			}

			float_t *tmpFrameData = frameData.release();
			int ret = swr_convert(
				pSwrContext.get(),
				(uint8_t**)&tmpFrameData, pFrame->nb_samples,
				(const uint8_t**)pFrame->extended_data, pFrame->nb_samples
			);
			if (ret < 0) return -1;
			frameData.reset(tmpFrameData);

			return 0;
		}

	public:
		Audio() :
			frameChannelSize(0),
			frameLength(0),
			sampleRate(44100),
			changeFlag(true)
		{
			pSwrContext = spSwrContext(
				nullptr,
				[](SwrContext *ptr) {
				swr_free(&ptr);
			}
			);
		}

		virtual ~Audio() {

		}

		float_t *nextFrame() {
			spAVFrame tmpFrame = this->FFmpeg::nextFrame();
			if (tmpFrame.get() == nullptr) return nullptr;
			convert(tmpFrame.get());
			return frameData.get();
		}

		inline void setSampleRate(double sampleRate) {
			this->sampleRate = sampleRate;
			changeFlag = true;
		}

		inline uint32_t getFrameLength() const { return frameLength; }
		inline uint8_t getFrameChannelSize() const { return frameChannelSize; }
		inline double_t getSampleRate() const { return sampleRate; }
	};
}