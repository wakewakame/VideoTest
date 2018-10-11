/*
  ==============================================================================

    Shader.h
    Created: 11 Oct 2018 4:09:36pm
    Author:  0214t

  ==============================================================================
*/

#pragma once

#include <map>
#include <string>
#include <regex>
#include "../JuceLibraryCode/JuceHeader.h"

class Shader {
private:
	// current OpenGLContext
	OpenGLContext &openGLContextRef;
	// program
	std::unique_ptr<OpenGLShaderProgram> shader;
	// default shader source codes
	static const std::string defaultVertexShader;
	static const std::string defaultFragmentShader;
	// attributes and uniforms
	std::map<std::string, std::string> attributesType;
	std::map<std::string, std::string> uniformsType;
	std::map<std::string, std::unique_ptr<OpenGLShaderProgram::Attribute>> attributes;
	std::map<std::string, std::unique_ptr<OpenGLShaderProgram::Uniform>> uniforms;
	// shader's error message
	std::string errorMessage;

	void findAttributes(std::string src);
	void findUniforms(std::string src);

public:
	Shader(OpenGLContext &openGLContext);
	virtual ~Shader();
	void loadShader(std::string vertexShader, std::string fragmentShader);
	void loadShader(std::string fragmentShader);
	void loadDefaultShader();
	void use();
	std::vector<std::string> getAttributeNames();
	std::vector<std::string> getUniformNames();
	inline OpenGLShaderProgram::Attribute *getAttribute(std::string name) {
		if (attributes.count(name) == 0) return (OpenGLShaderProgram::Attribute*)nullptr;
		return attributes[name].get();
	}
	inline OpenGLShaderProgram::Uniform *getUniform(std::string name) {
		if (uniforms.count(name) == 0) return (OpenGLShaderProgram::Uniform*)nullptr;
		return uniforms[name].get();
	}
	inline std::string getAttributeType(std::string name) {
		if (attributesType.count(name) == 0) return "";
		return attributesType[name];
	}
	inline std::string getUniformType(std::string name) {
		if (uniformsType.count(name) == 0) return "";
		return uniformsType[name];
	}
	inline std::string getErrorMessage() {
		return errorMessage;
	}
	inline std::string getDefaultVertexShader() {
		return errorMessage;
	}
	inline std::string getDefaultFragmentShader() {
		return errorMessage;
	}
};