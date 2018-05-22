#ifndef FUNCTION_H
#define FUNCTION_H

#include <functional>
#include <glm/glm.hpp>

typedef glm::vec3 point_t;

template<class T>
class function_t {
private:
    // target function
    std::function<T(point_t)> f;

public:
    function_t(std::function<T(point_t)> f){
        this->f = f;
    }

    T operator()(point_t p){
        return f(p);
    }

    function_t differentiate(int axis){
        const float delta = 8;

        point_t axes[3] = {
            point_t(delta, 0, 0),
            point_t(0, delta, 0),
            point_t(0, 0, delta)
        };
        point_t u = axes[axis];

	return function_t([=](point_t x){
            return (this->f(x + u) - this->f(x - u)) / (2.0f * delta);
        });
    }
};

#endif
