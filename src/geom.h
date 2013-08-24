#ifndef __GEOM_H__
#define __GEOM_H__
#include <vector>

struct Point
{
    float x;
    float y;

    Point() : x(0.0f), y(0.0f) {}
    Point(float x_, float y_) : x(x_), y(y_) {}

    void rotate(float cosa, float sina)
    {
        float nx = x*cosa - y*sina;
        y = x*sina + y*cosa;
        x = nx;
    }

    inline float dist(const Point& p) const { return sqrtf((x - p.x)*(x - p.x) + (y - p.y)*(y - p.y)); }
    
    inline void operator *=(float v) { x *= v; y *= v; }
    inline void operator *=(const Point& p) { x *= p.x; y *= p.y; }
    inline void operator +=(const Point& p) { x += p.x; y += p.y; }

    friend inline Point operator *(const Point& a, float mul) { return Point(a.x*mul, a.y*mul); }
    friend inline Point operator *(const Point& a, const Point& b) { return Point(a.x*b.x, a.y*b.y); }
    friend inline Point operator *(float mul, const Point& a) { return Point(a.x*mul, a.y*mul); }

    friend inline Point operator +(const Point& a, const Point& b) { return Point(a.x + b.x, a.y + b.y); }
    friend inline Point operator -(const Point& a, const Point& b) { return Point(a.x - b.x, a.y - b.y); }

};

struct Rect
{
    float l;    // left
    float t;    // top
    float r;    // right
    float b;    // bottom

    Rect() : l(0.0f), t(0.0f), r(0.0f), b(0.0f) {}
    Rect(float l_, float t_, float r_, float b_) : l(l_), t(t_), r(r_), b(b_) {}

    inline float w() const { return r - l; }
    inline float h() const { return b - t; }

    inline bool contains(float x, float y) const { return x >= l && x <= r && y >= t && y <= b; }

    inline Rect inflated(float d) const { return Rect(l - d, t - d, r + d, b + d); }
    inline Rect inflated(float dl, float dt, float dr, float db) const { return Rect(l - dl, t - dl, r + dr, b + db); }
    inline Rect shifted(float dx, float dy) const { return Rect(l + dx, t + dy, r + dx, b + dy); }

    inline void inflate(float d) { l -= d; t -= d; r += d; b += d; }
    inline void inflate(float dl, float dt, float dr, float db) { l -= dl; t -= dt; r += dr; b += db; }
    inline void shift(float dx, float dy) { l += dx; t += dy; r += dx; b += dy; }
    
    //  maps point's coordinates from unit rectangle into this one
    inline Point mapFromUnit(const Point& pt) const
    {
        return Point(l + pt.x*(r - l), t + pt.y*(b - t));
    }
};


#endif
