/*
  ==============================================================================

    VideoTexture.h
    Created: 10 Oct 2018 8:22:19pm
    Author:  0214t

  ==============================================================================
*/

#pragma once

#include "FFmpegWrapper.h"
#include "GLComponent.h"

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
			if ((uint8_t*)nullptr != video.front()) {
				texture.loadARGB(
					(PixelARGB*)video.front(),
					video.getFrameWidth(),
					video.getFrameHeight()
				);
				video.pop();
			}
		}
	};

	class VideoGraphics : public GLGraphics {
	private:
		std::unique_ptr<Shader> shader;
		OpenGLTexture videoTex;
		FF::VideoTexture ffVideoTex;

	public:
		void setup() override {
			ffVideoTex.open("G:\\Žv‚¢o\\Dropbox\\Dropbox\\C++\\ffmpeg_test\\ffmpeg_test\\sync-test-doubleNTSC30min.mp4");
			shader.reset(new Shader(*openGLContextPtr));
			shader->loadShader(
				R"(
	varying vec2 vUv;
	varying vec4 vColor;

	uniform sampler2D texture;

	void main()
	{
		vec3 color = texture2D(texture, vUv).rgb;
		gl_FragColor = vec4(color, 1.0);
	};
)"
);
		}

		void draw() {
			setNoShader();
			fill(1.0, 1.0, 1.0, 1.0);
			rect(-1.0, -1.0, 1.0, 1.0);

			if (shader->getErrorMessage() == "")
				setShader(*shader.get());

			ffVideoTex.next(videoTex);
			OpenGLShaderProgram::Uniform *tmpPtr = shader->getUniform("texture");
			if (tmpPtr != nullptr) tmpPtr->set((GLint)videoTex.getTextureID());

			videoTex.bind();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			fill(0.0, 0.0, 0.0, 1.0);
			noStroke();
			rect(-0.5, -0.5, 0.5, 0.5);
		}
	};
}