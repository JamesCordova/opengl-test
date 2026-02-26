#ifndef ERROR_REPORTING_H
#define ERROR_REPORTING_H

#include <glad/glad.h>

void GLAPIENTRY glDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam);

void enableReportGlErrors();

#endif