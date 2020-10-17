#pragma once

// Structs
typedef struct {
    float x;
    float y;
} V2;

// Scientific
float v_norm(V2 * v);
V2 v_add(V2 * a, V2 * b);
V2 v_substract(V2 * a, V2 * b);
void v_normalize(V2 * v);
void v_muliply(V2 * v, float b);
float v_scalar(V2 * a, V2 * b);

// Utils
void v_print(char buffer[], V2 v);