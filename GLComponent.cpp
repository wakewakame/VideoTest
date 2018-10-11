/*
  ==============================================================================

    GLComponent.cpp
    Created: 12 Oct 2018 12:24:44am
    Author:  0214t

  ==============================================================================
*/

#include "GLComponent.h"

GLComponent::GLComponent()
{
	setSize(800, 600);
}

GLComponent::~GLComponent()
{
	shutdownOpenGL();
}

void GLComponent::initialise()
{
	graphics.reset(new GLGraphics(openGLContext));
	ffVideoTex.open("G:\\Žv‚¢o\\Dropbox\\Dropbox\\C++\\ffmpeg_test\\ffmpeg_test\\sync-test-doubleNTSC30min.mp4");
	shader.reset(new Shader(openGLContext));
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
	if (shader->getErrorMessage() == "")
		graphics->filter(*shader.get());
}

void GLComponent::shutdown()
{
	videoTex.release();
	shader.reset();
	graphics.reset();
}

void GLComponent::render()
{
	jassert(OpenGLHelpers::isContextActive());

	auto desktopScale = (float)openGLContext.getRenderingScale();
	OpenGLHelpers::clear(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glViewport(0, 0, roundToInt(desktopScale * getWidth()), roundToInt(desktopScale * getHeight()));

	ffVideoTex.next(videoTex);
	OpenGLShaderProgram::Uniform *tmpPtr = shader->getUniform("texture");
	if (tmpPtr != nullptr) tmpPtr->set((GLint)videoTex.getTextureID());

	videoTex.bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	graphics->fill(0.0, 0.0, 0.0, 1.0);
	graphics->noStroke();
	graphics->rect(-0.5, -0.5, 0.5, 0.5);
}

void GLComponent::paint(Graphics& g)
{
	// You can add your component specific drawing code here!
	// This will draw over the top of the openGL background.

	g.setColour(getLookAndFeel().findColour(Label::textColourId));
	g.setFont(20);
	g.drawText("OpenGL Example", 25, 20, 300, 30, Justification::left);
	g.drawLine(20, 20, 170, 20);
	g.drawLine(20, 50, 170, 50);
}

void GLComponent::resized()
{
	// This is called when this component is resized.
	// If you add any child components, this is where you should
	// update their positions.
}