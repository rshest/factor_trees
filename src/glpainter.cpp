#include <assert.h>
#include <stdlib.h>

#include <GL/glut.h>

#include "glpainter.h"
#include "image.h"

static GLPainter s_Painter;
GLPainter* g_pGLPainter = &s_Painter;

void* FONT = GLUT_BITMAP_TIMES_ROMAN_24;

void GLPainter::drawText(float x, float y, const char* text, Color color) const
{
    assert(text);
    glColor4ubv(reinterpret_cast<const GLubyte*>(&color));
    glRasterPos2f(x, y);
    
    const char* c = text;
    while (*c != '\0')
    {
        glutBitmapCharacter(FONT, *c);
        c++;
    }
}

void GLPainter::drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3, Color color) const
{
    glColor4ubv(reinterpret_cast<const GLubyte*>(&color));

    glBegin(GL_TRIANGLES); 
    glVertex2f(x1, y1);
    glVertex2f(x2, y2); 
    glVertex2f(x3, y3); 
    glVertex2f(x3, y3); 
    glEnd();
}

void GLPainter::drawQuad(float ax, float ay, float bx, float by, 
    float cx, float cy, float dx, float dy, Color color) const
{
    glColor4ubv(reinterpret_cast<const GLubyte*>(&color));

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(ax, ay);

    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(bx, by);

    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(cx, cy);
    
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(dx, dy);

    glEnd();
}

unsigned int GLPainter::loadTexture(const char* fileName) const
{
    Image image;
    if (!LoadTGA(fileName, image))
    {
        return -1;
    }

    GLuint texID;

    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, 
        GL_UNSIGNED_BYTE, image.rgba.size() ? &image.rgba[0] : 0);

    return texID;
}

void GLPainter::setTexture(unsigned int texID) const
{
    if (texID > 0)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texID);
    }
    else
    {
        glDisable(GL_TEXTURE_2D);
    }
}

