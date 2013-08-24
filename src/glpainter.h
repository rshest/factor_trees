#ifndef __GLPAINTER_H__
#define __GLPAINTER_H__

typedef unsigned int Color;

//  Interface to OpenGL painting facilities.
//  Note that it's not OpenGL-specific, while the implementation is
class GLPainter
{
public:
    void drawQuad(float ax, float ay, float bx, float by, float cx, float cy, float dx, float dy, Color color) const;
    void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3, Color color) const;
    void drawText(float x, float y, const char* text, Color color) const;

    unsigned int loadTexture(const char* fileName) const;
    void setTexture(unsigned int texID) const;
};

extern GLPainter* g_pGLPainter;

#endif
