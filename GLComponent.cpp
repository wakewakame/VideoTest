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
	shape.reset(new Shape(openGLContext));
}

void GLComponent::shutdown()
{
	shape.reset();
}

void GLComponent::render()
{
	jassert(OpenGLHelpers::isContextActive());

	auto desktopScale = (float)openGLContext.getRenderingScale();
	OpenGLHelpers::clear(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glViewport(0, 0, roundToInt(desktopScale * getWidth()), roundToInt(desktopScale * getHeight()));


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