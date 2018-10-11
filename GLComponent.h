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

class GLComponent : public OpenGLAppComponent
{
protected:
	std::unique_ptr<GLGraphics> graphics;
	std::unique_ptr<Shader> shader;

public:
	GLComponent();

	~GLComponent();

	void initialise() override;

	void shutdown() override;

	void render() override;

	void paint(Graphics& g) override;

	void resized() override;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GLComponent)
};