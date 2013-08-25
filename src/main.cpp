#include <stdio.h> 
#include <math.h>

#include <GL/glut.h>

#include <assert.h>
#include <ctime>
#include <sstream>

#include <geom.h>
#include <glpainter.h>
#include <image.h>

#include "factorization.h"

const float PI          = 3.1415926535897932384626433832795f;
const float GOLDEN_PHI  = 137.5f/180.0f*PI;

const int   BUD_THRESHOLD            = 47;          //  max number before becoming a "is_bud"
const float SPREAD_ANGLE             = PI*0.5f;     //  child branches spread angle, from the vertical
const float STEM_LENGTH              = 0.618f;
const float STEM_BASE_WIDTH          = 0.4f;
const float STEM_TOP_WIDTH           = 0.25f;
const float STEM_SCALEY              = 0.8f;        //  the y-axis scale when going one branch level up
const int   STEP_TIME_SECONDS        = 2;           //  animation time step
const Color TRUNK_COLOR              = 0xFF142A44;

int          screenWidth  = 1024;
int          screenHeight = 768;
int          img_fruit_bw = -1;
int          img_fruit    = -1;
bool         is_paused    = false;
bool         is_colored   = true;

unsigned int cur_number   = 2*2*3*3*53;
std::vector<unsigned int> primes_cache;

//  gets a fruit color from predefined palette, based on weight (in range [0, 1])
inline BGRA get_fruit_color(float weight)
{
    static const BGRA palette[] =
    {
        { 43,168, 87,  255},
        {115,215,169,  255},
        {223,223,223,  255},
        {214,154,191,  255},
        {151, 99,172,  255}
    };

    assert(weight >= 0.0f && weight <= 1.0f);
    int nEntries = sizeof(palette)/sizeof(BGRA);
    float fidx = weight*float(nEntries - 1);
    int idx = (int)fidx;
    if (idx == nEntries - 1) idx--;
    float t = fidx - float(idx);
    const BGRA& c1 = palette[idx];
    const BGRA& c2 = palette[idx + 1];
    BGRA res;
    res.r = (unsigned char)(c1.b*(1.0f - t) + c2.b*t);
    res.g = (unsigned char)(c1.g*(1.0f - t) + c2.g*t);
    res.b = (unsigned char)(c1.r*(1.0f - t) + c2.r*t);
    res.a = (unsigned char)(c1.a*(1.0f - t) + c2.a*t);
    return res;
}   

struct Branch
{
    Branch() : parent_offset(0), is_bud(false), has_fruit(false) {}

    Point beg1, beg2;               //  branch base coordinates
    Point end1, end2;               //  branch top coordinates
    int parent_offset;              //  index of the parent branch in the tree branches array
    float radius;                   //  fruit radius
    bool has_fruit;                 //  the branch is "terminal", will be drawn with a fruit
    bool is_bud;                    //  drawn as a part of a bud
};

void build_tree(const unsigned int* factors, int num_factors, std::vector<Branch>& tree)
{
    if (num_factors == 0) return;
    const int factor = factors[num_factors - 1];
    const bool is_bud = (factor > BUD_THRESHOLD);

    //  the new common branch at the root
    Branch b;
    b.has_fruit = (factor == 0);
    b.radius = 0.1f;

    b.beg1 = Point(-STEM_BASE_WIDTH*0.5f, 0.0f);
    b.beg2 = Point( STEM_BASE_WIDTH*0.5f, 0.0f);

    const float top_width = (is_bud || (factor == 0)) ? 0.0f : STEM_TOP_WIDTH*0.5f;
    b.end1 = Point(-top_width, STEM_LENGTH);
    b.end2 = Point( top_width, STEM_LENGTH);

    if (!tree.empty())
    {
        Branch& root = tree.back();
        float da = SPREAD_ANGLE*2.0f/float(factor);
        const float s_ang = SPREAD_ANGLE*2.0f/float(factor);

        const float r = b.end1.dist(b.end2)*0.5f;
        const float rs = root.beg1.dist(root.beg2)*0.5f;
        const float rs_new = sinf(s_ang*0.5f)*r;
        const float s = rs_new/rs;
        Point scale(s, STEM_SCALEY);

        const Point root_offs(0.0f, STEM_LENGTH*(is_bud ? 1.5f : 1.0f));
        Point offs0(0.0f, r*cosf(s_ang*0.5f));

        const int bsz = tree.size();
        // clone the whole subtree factor times
        for (int i = 1; i < factor; i++)
        {
            tree.insert(tree.end(), tree.begin(), tree.begin() + bsz);
        }
        const int new_bsz = bsz*factor;
        for (int i = bsz - 1; i < new_bsz; i += bsz)
        {
            tree[i].parent_offset = new_bsz - i;
            tree[i].is_bud = is_bud;
        }

        float rot_ang = GOLDEN_PHI;
        const float r_scale = 0.9f/sqrtf(GOLDEN_PHI*factor);
        const float fr = 0.9f*sqrtf(0.9f*0.9f/factor);

        for (int i = 0; i < factor; i++)
        {
            if (is_bud)
            {
                // in case of a is_bud do Fermat spiral layout
                const float r = sqrtf(rot_ang)*r_scale;
                rot_ang += GOLDEN_PHI;
                offs0 = Point(0.0f, r - STEM_LENGTH);
            }
            else
            {
                //  otherwise radial layout along the arc
                rot_ang = -SPREAD_ANGLE + da*(0.5f + i);
            }

            float ca = cosf(rot_ang);
            float sa = sinf(rot_ang);

            for (int j = 0; j < bsz; j++)
            {
                Branch& cb = tree[j + i*bsz];

                cb.beg1 *= scale; 
                cb.end1 *= scale;
                cb.beg2 *= scale;
                cb.end2 *= scale;

                cb.beg1 += offs0;
                cb.end1 += offs0;
                cb.beg2 += offs0;
                cb.end2 += offs0;

                cb.beg1.rotate(ca, sa);
                cb.end1.rotate(ca, sa);
                cb.beg2.rotate(ca, sa);
                cb.end2.rotate(ca, sa);

                cb.beg1 += root_offs;
                cb.end1 += root_offs;
                cb.beg2 += root_offs;
                cb.end2 += root_offs;

                cb.radius = is_bud ? 
                    fr : 
                    (cb.is_bud ? 
                        (0.5f*cb.radius*scale.x +  0.5f*cb.radius*scale.y) : 
                        cb.radius*scale.y);
            }
        }
    }
    tree.push_back(b);

    build_tree(factors, num_factors - 1, tree);
}

void draw_branch(const Point& offs, const Point& scale, const std::vector<Branch>& tree, int idx)
{
    const Branch& b = tree[idx];

    Point beg1 = b.beg1*scale + offs;
    Point beg2 = b.beg2*scale + offs;

    Point end1 = b.end1*scale + offs;
    Point end2 = b.end2*scale + offs;

    Point root(0.0f, 0.0f);
    if (b.parent_offset > 0)
    {
        const Branch& parent = tree[idx + b.parent_offset];
        root = (parent.end1 + parent.end2)*0.5f;
    }

    root = root*scale + offs;

    if (!b.is_bud)
    {
        // the base cap
        g_pGLPainter->setTexture(0);
        g_pGLPainter->drawTriangle(root.x, root.y, beg1.x, beg1.y, beg2.x, beg2.y, TRUNK_COLOR);
    }
    g_pGLPainter->drawQuad(end1.x, end1.y, end2.x, end2.y, beg2.x, beg2.y, beg1.x, beg1.y, TRUNK_COLOR); 

    if (b.has_fruit)
    {
        g_pGLPainter->setTexture(is_colored ? img_fruit_bw : img_fruit);
        float r = scale.x*b.radius;
        BGRA fruitColor = get_fruit_color(float(rand())/RAND_MAX);
        g_pGLPainter->drawQuad(end1.x - r, end1.y - r, end1.x + r, end1.y - r,
            end1.x + r, end1.y + r, end1.x - r, end1.y + r, 
            is_colored ? *((Color*)&fruitColor) : 0xFFFFFFFF);
    }
}

void draw_tree(const Rect& ext, const std::vector<Branch>& tree)
{    
    srand(cur_number);
    const size_t nb = tree.size();
    const float sc = ext.h()*0.4f;
    for (int i = nb - 1; i >= 0; i--)
    {
        draw_branch(Point(ext.w()*0.5f, ext.h()), Point(sc, -sc), tree, i);
    }
}

void handle_resize(int width, int height)
{
    screenWidth = width;
    screenHeight = height;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(1, -1, 1);
    glTranslatef(0.5f, -height - 0.5f, 0); // half pixel offset

    glViewport(0, 0, width, height);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    img_fruit = g_pGLPainter->loadTexture("data/fruit.tga");
    img_fruit_bw = g_pGLPainter->loadTexture("data/fruit_bw.tga");
} 

void handle_update(int value)
{
    double curTime = ((double)glutGet(GLUT_ELAPSED_TIME))/1000.0;
    static double lastTime = curTime;
    double dt = curTime - lastTime;
    lastTime = curTime;

    glutTimerFunc(10, handle_update, 0);
    glutPostRedisplay();
} 

void handle_display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static std::time_t last_time = std::time(NULL);
    std::time_t cur_time = std::time(NULL);
    if (cur_time - last_time >= STEP_TIME_SECONDS && !is_paused)
    {
        last_time = cur_time;
        cur_number++;
    }

    Rect ext(0.0f, 0.0f, float(screenWidth), float(screenHeight));
    g_pGLPainter->setTexture(0);

    std::vector<unsigned int> factors;
    prime_factors(cur_number, factors, &primes_cache);

    std::stringstream ss;
    ss << cur_number << " = ";
    if (factors.size() <= 1) 
    {
        ss << "Prime" << (factors.size() == 1 ? "!" : "?..");
    }
    else
    {
        for (size_t i = 0; i < factors.size(); i++)
        {
            ss << factors[i] << (i < factors.size() - 1 ? "x" : ".");
        }
    }
    if (is_paused) ss << " [PAUSED]";

    g_pGLPainter->drawText(15, 25, ss.str().c_str(), 0xFFFFFFFF);

    factors.push_back(0);

    std::vector<Branch> tree;
    build_tree(&factors[0], factors.size(), tree);

    draw_tree(ext, tree);

    glutSwapBuffers();
}

void handle_keydown(unsigned char key, int x, int y)
{
    switch (key)
    {
    case ' ': is_paused = !is_paused; break;
    case '-': cur_number--; break;
    case '+': cur_number++; break;
    case 'c': is_colored = !is_colored; break;
    }
}

int main(int argc, char *argv[])
{
    printf("Space - toggle pause;\n'+' - next number;\n"
        "'-' - previous number;\n'c' - toggle fruit coloring;");
    fflush(stdout);

    glutInit(&argc, argv);
    glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH) - screenWidth)/2, 
        (glutGet(GLUT_SCREEN_HEIGHT) - screenHeight)/2);
    glutInitWindowSize(screenWidth, screenHeight);    

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutCreateWindow("Growing primes");

    glutReshapeFunc(handle_resize);
    glutDisplayFunc(handle_display);
    glutTimerFunc(30, handle_update, 0);
    glutKeyboardFunc(handle_keydown);
    glutMainLoop();

    return 0;
}


