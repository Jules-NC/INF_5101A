#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

//#include "mpi.h"
#include "vector.h"
#include "quadtree_realloc.h"

QT * getQt(QTMaster * qtmaster, int offset){
    QT* qt = qtmaster->QTList+offset;
    return qt;
}

void printQT(QT * qt){
    // vector print
    char vector_buffer[255];
    v_print(vector_buffer, qt->origin);

    puts("+========[node]========");
    printf("|[%s]\n", qt->name);
    puts("+----------------------");
    printf("|leaf: %d\n", qt->leaf);
    printf("|h_len: %f\n", qt->half_length);
    printf("|origin: %s\n", vector_buffer);
    puts("+======================");
    printf("\n");
}

void printOrchestrator(QTMaster * qtmaster){
    puts("+====[ORCHESTRATOR]====");
    printf("|bytes allocated: %ld\n", qtmaster->QT_total*sizeof(QT));
    printf("|number of QT: %d\n", qtmaster->QT_total);
    printf("|size of QT: %ld\n", sizeof(QT));
    puts("+======================");
    puts("");
}

int addQT(QTMaster * qtmaster, V2 origin, float half_length, int father, char name[]){
    qtmaster->QT_total++;
    qtmaster->QTList = (QT*) realloc(qtmaster->QTList, sizeof(QT)*qtmaster->QT_total);
    QT * qt = qtmaster->QTList+(qtmaster->QT_total-1);

    qt->leaf = true;
    qt->origin = origin;
    qt->half_length = half_length;
    qt->father = father;

    strcpy(qt->name, name);

    return qtmaster->QT_total-1;
}

void divideQT(QTMaster * qtmaster, int offset){
    QT * qt = getQt(qtmaster, offset);
    qt->leaf = false;
    float x0 = qt->origin.x;
    float y0 = qt->origin.y;
    float half_length = qt->half_length;

    char name_A[255];
    char name_B[255];
    char name_C[255];
    char name_D[255];
    strcpy(name_A, qt->name);
    strcat(name_A, "->A");

    strcpy(name_B, qt->name);
    strcat(name_B, "->B");

    strcpy(name_C, qt->name);
    strcat(name_C, "->C");

    strcpy(name_D, qt->name);
    strcat(name_D, "->D");

    half_length /= 2;

    V2 origin_A = {.x = x0, .y = y0+half_length};
    V2 origin_B = {.x = x0+half_length, .y = y0+half_length};
    V2 origin_C = {.x = x0, .y = y0};
    V2 origin_D = {.x = x0+half_length, .y = y0};


    qt->A = addQT(qtmaster, origin_A, half_length, offset ,name_A);
    qt->B = addQT(qtmaster, origin_B, half_length, offset, name_B);
    qt->C = addQT(qtmaster, origin_C, half_length, offset, name_C);
    qt->D = addQT(qtmaster, origin_D, half_length, offset, name_D);

}


int main(int argc, char *argv[])
{
    QTMaster qtmaster;
    qtmaster.QT_total = 0;  // next time realloc will not change the memory size
    qtmaster.QTList = (QT*) malloc(qtmaster.QT_total*sizeof(QT));  // TODO: malloc(0)

    V2 origin = {.x = 69, .y = 420};
    int root = addQT(&qtmaster, origin, 100, -1, "ROOT"); // ROOT
    divideQT(&qtmaster, root);

    printQT( getQt(&qtmaster, (getQt(&qtmaster, root)->C)));

    printOrchestrator(&QT_MASTER);

    V2 lol = {.x = 0, .y=42};
    v_normalize(&lol);
    float xd = v_norm(&lol);

    free(qtmaster.QTList);
}
