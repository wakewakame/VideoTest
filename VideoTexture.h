/*
  ==============================================================================

    VideoTexture.h
    Created: 10 Oct 2018 8:22:19pm
    Author:  0214t

  ==============================================================================
*/

#pragma once

#include "FFmpegWrapper.h"

namespace FF {
	class VideoTexture {
		static_assert(std::is_standard_layout<PixelARGB>::value, "PixelARGB is not POD");
		static_assert(std::is_same<juce::uint8, uint8_t>::value, "juce::uint8 is not equal uint8_t");

	private:
		FF::Video video;

	public:
		VideoTexture() {
			video.setPixelFormat(video.BGRA32);
		}
		virtual ~VideoTexture() {}
		void open(std::string path) {
			video.open(path);
		}
		void seek(int64_t time) { video.seek(time); }
		void next(OpenGLTexture &texture) {
			video.next();
			texture.loadARGB(
				(PixelARGB*)video.getFrameData(),
				video.getFrameWidth(),
				video.getFrameHeight()
			);
		}
	};
}