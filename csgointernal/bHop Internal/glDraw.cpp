#include "glDraw.h"


void GL::SetupOrtho() {
	// set up 2d projection matrix to draw on
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushMatrix();
	GLint viewport[4];
	// get viewport data and save into viewport
	glGetIntegerv(GL_VIEWPORT, viewport);
	// set viewport width and height
	glViewport(0, 0, viewport[2], viewport[3]);
	// set mode = projection mode
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// ortho matrix for 2D drawings
	glOrtho(0, viewport[2], viewport[3], 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);
};
void GL::RestoreGL() {
	glPopMatrix();
	glPopAttrib();
};

void GL::DrawFilledRect(float x, float y, float width, float height, const GLubyte color[3]) {
	glColor3ub(color[0], color[1], color[2]);
	glBegin(GL_QUADS);
	glVertex2f(x, y); // top left
	glVertex2f(x + width, y); // top right
	glVertex2f(x + width, y + height); // bottom right
	glVertex2f(x, y + height); // bottom left
	glEnd();
}
void GL::DrawOutline(float x, float y, float width, float height, float lineWidth, const GLubyte color[3]) {
	glLineWidth(lineWidth);
	glBegin(GL_LINE_STRIP);
	glColor3ub(color[0], color[1], color[2]);
	glVertex2f(x - 0.5f, y - 0.5f);
	glVertex2f(x + 0.5f + width, y + width + 0.5f);
	glVertex2f(x + 0.5f + width, y + width - 0.5f);
	glVertex2f(x - 0.5f , y + height + 0.5f);
	glVertex2f(x - 0.5f, y + height - 0.5f);
	glEnd();
}