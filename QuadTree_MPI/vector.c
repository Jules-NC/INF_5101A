#include <math.h>
#include <stdio.h>
#include "vector.h"

// Scientific
float v_norm(V2 * v){
    return sqrt(powf(v->x, 2.f) + powf(v->y, 2.f));
}

V2 v_add(V2 * a, V2 * b){
    V2 result;
    result.x = a->x + b->x;
    result.y = a->y + b->y;
    return result;
}

V2 v_substract(V2 * a, V2 * b){
    V2 result;
    result.x = a->x - b->x;
    result.y = a->y - b->y;
    return result;
}

void v_normalize(V2 * v){
    float norm = v_norm(v);
    v->x /= norm;
    v->y /= norm;
}

void v_muliply(V2 * v, float e){
    v->x *= e;
    v->y *= e;
}

float v_scalar(V2 * a, V2 * b){
    return (a->x * b->x) + (a->y * b->y);
}

// Utils

void v_print(char buffer[], V2 v){
    sprintf(buffer, "V2(%f, %f)", v.x, v.y);
}