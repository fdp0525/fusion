#ifndef POINT_H
#define POINT_H

class point_t {
public:
    // constructors
    point_t();
    point_t(float x, float y, float z);
    point_t(const point_t& p);
    
    // operator overrides
    point_t operator*(float scale);
    point_t operator+(point_t other);
    void operator-=(point_t other);

    // vector length
    float length();

private:
    float x;
    float y;
    float z;
};

#endif
