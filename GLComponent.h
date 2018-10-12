/*
  ==============================================================================

    GLComponent.h
    Created: 12 Oct 2018 12:24:21am
    Author:  0214t

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "GLGraphics.h"

template<class G>
class GLComponent : public OpenGLAppComponent
{
static_assert(std::is_base_of<GLGraphics, G>::value, "type parameter of this class must derive from GLGraphics");

protected:
	std::unique_ptr<GLGraphics> graphics;

public:
	GLComponent() {
		setSize(640, 480);
	}

	virtual ~GLComponent() {
		shutdownOpenGL();
	}

	inline G *getGraphics() { return (G*)graphics.get(); }

	void initialise() override {
		graphics.reset(new G());
		graphics->initialise(openGLContext);
		graphics->setup();
	}

	void shutdown() override {
		graphics.reset();
	}

	void render() override {
		assert(OpenGLHelpers::isContextActive());

		auto desktopScale = (float)openGLContext.getRenderingScale();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glViewport(0, 0, roundToInt(desktopScale * getWidth()), roundToInt(desktopScale * getHeight()));

		graphics->draw();
	}

	void paint(Graphics& g) override {
		// You can add your component specific drawing code here!
		// This will draw over the top of the openGL background.

		g.setColour(getLookAndFeel().findColour(Label::textColourId));
		g.setFont(20);
		g.drawText("OpenGL Example", 25, 20, 300, 30, Justification::left);
		g.drawLine(20, 20, 170, 20);
		g.drawLine(20, 50, 170, 50);
	}

	void resized() override {
		// This is called when this component is resized.
		// If you add any child components, this is where you should
		// update their positions.
	}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GLComponent)
};