#include <iostream>
#include <cassert>
#include <fstream>
#include <list>
#include <string>
#include <map>
#include <sstream>
#include <optional>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

std::optional<std::list<std::string>> readFileAsStdListOfStrings(const std::string& fileName)
{
	std::ifstream in(fileName.c_str(), std::ios::in);
	if (!in.is_open()) return {};
	std::list<std::string> result;
	std::string string;
	while (std::getline(in, string)) result.emplace_back(string + "\n");
	in.close();
	return result;
}

std::string getErrorMessage()
{
	const GLenum error = glGetError();
	if (GL_NO_ERROR == error) return "";

	// English descriptions are from
	// https://www.opengl.org/sdk/docs/man/docbook4/xhtml/glGetError.xml
	const std::map<GLenum, std::string> errorStrings = {
		{GL_NO_ERROR, "No error has been recorded. THIS message is the error itself."},
		{GL_INVALID_ENUM, "An unacceptable value is specified for an enumerated argument."},
		{GL_INVALID_VALUE, "A numeric argument is out of range."},
		{GL_INVALID_OPERATION, "The specified operation is not allowed in the current state."},
		{GL_INVALID_FRAMEBUFFER_OPERATION, "The framebuffer object is not complete."},
		{GL_OUT_OF_MEMORY, "There is not enough memory left to execute the command."},
		{GL_STACK_UNDERFLOW, "An attempt has been made to perform an operation that would cause an internal stack to underflow."},
		{GL_STACK_OVERFLOW, "An attempt has been made to perform an operation that would cause an internal stack to overflow."}
	};
	std::stringstream ss;
	ss << "OpenGL error: " << static_cast<int>(error) << std::endl;
	ss << "Error string: ";
	const auto& found = errorStrings.find(error);
	if (found == errorStrings.end()) {
		ss << "No description available. Incompatible OpenGL version?";
	}
	else {
		ss << found->second;
	}
	ss << std::endl;
	return ss.str();
}

bool error()
{
	const auto message = getErrorMessage();
	if (message.length() == 0) return false;
	std::cerr << message;
	return true;
}

bool compileShader(const GLuint shader, const std::list<std::string>& source)
{
	std::vector<const GLchar*> vsSource;
	const auto vsLinesCount = source.size();
	vsSource.reserve(vsLinesCount);
	for (const auto& str: source) vsSource.emplace_back(str.c_str());
	glShaderSource(shader, GLsizei(vsLinesCount), vsSource.data(), nullptr);
	assert(!error());
	glCompileShader(shader);
	assert(!error());

	GLint logLength;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0) {
		auto* const log = new GLchar[logLength + 1];
		glGetShaderInfoLog(shader, logLength, nullptr, log);
		std::cout << "Log: " << std::endl;
		std::cout << log;
		delete[] log;
	}

	GLint compileStatus;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	return bool(compileStatus);
}

int main()
{
	// Init
	if (!glfwInit()) {
		std::cerr << "Error: glfw init failed." << std::endl;
		return 3;
	}
	GLFWwindow* window = nullptr;
	window = glfwCreateWindow(300, 200, "Shader test", nullptr, nullptr);
	if (window == nullptr) {
		std::cerr << "Error: window is null." << std::endl;
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);

	const auto glewOk = glewInit();
	if (glewOk != GLEW_OK) {
		std::cerr << "Error: glew not OK." << std::endl;
		glfwTerminate();
		return 2;
	}

	// Test max vectors
	GLint maxVertexUniformVectors = 0;
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &maxVertexUniformVectors);
	std::cout << "Max vectors: " << maxVertexUniformVectors << std::endl;

	// Load shader source
	std::cout << "Loading shader sources..." << std::endl;;
	const auto vertexSource = readFileAsStdListOfStrings("Shader.vert");
	const auto fragmentSource = readFileAsStdListOfStrings("Shader.frag");
	if (!vertexSource) {
		std::cerr << "Error: could not load vertex source." << std::endl;
		return 4;
	}
	if (!fragmentSource) {
		std::cerr << "Error: could not load fragment source." << std::endl;
		return 5;
	}

	// Create shader
	const auto vs = glCreateShader(GL_VERTEX_SHADER);
	if (vs == 0) {
		std::cerr << "Error: vertex shader is 0." << std::endl;
		return 2;
	}
	assert(!error());
	const auto fs = glCreateShader(GL_FRAGMENT_SHADER);
	if (fs == 0) {
		std::cerr << "Error: fragment shader is 0." << std::endl;
		return 2;
	}
	assert(!error());

	// Compile shaders
	std::cout << "Compiling vertex shader..." << std::endl;;
	if (!compileShader(vs, *vertexSource)) {
		std::cerr << "Error: could not compile vertex shader." << std::endl;
		return 5;
	}

	std::cout << "Compiling fragment shader..." << std::endl;;
	if (!compileShader(fs, *fragmentSource)) {
		std::cerr << "Error: could not compile fragment shader." << std::endl;
		return 5;
	}

	// Link program
	const auto program = glCreateProgram();
	if (program == 0) {
		std::cerr << "Error: program is 0." << std::endl;
		return 2;
	}
	assert(!error());
	glAttachShader(program, vs);
	assert(!error());
	glAttachShader(program, fs);
	assert(!error());
	glLinkProgram(program);
	assert(!error());

	// Get log
	GLint logLength = 0;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

	if (logLength > 0) {
		auto* const log = new GLchar[logLength + 1];
		glGetProgramInfoLog(program, logLength, nullptr, log);
		std::cout << "Log: " << std::endl;
		std::cout << log;
		delete[] log;
	}
	GLint linkStatus = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	if (!linkStatus) {
		std::cerr << "Error: could not link." << std::endl;
		return 2;
	}
	assert(!error());
	glDeleteShader(vs);
	glDeleteShader(fs);

	// Shutdown
	std::cout << "Finishing..." << std::endl;;
	glDeleteShader(program);
	glfwMakeContextCurrent(nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
