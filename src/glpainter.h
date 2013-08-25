#ifndef __GLPAINTER_H__
#define __GLPAINTER_H__

typedef unsigned int Color;

//  OpenGL painting helpers
void drawQuad(float ax, float ay, float bx, float by, float cx, float cy, float dx, float dy, Color color);
void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3, Color color);
void drawText(float x, float y, const char* text, Color color);

unsigned int loadTexture(const char* fileName);
void setTexture(unsigned int texID);

#endif
