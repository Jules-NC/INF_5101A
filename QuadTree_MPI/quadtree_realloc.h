#pragma once

// TEMPORARY, to define tater
/*
 * TODO: define the body object
 * (do i really need to do this (if leaf then same data))
*/
typedef struct Body {} Body;

// STRUCTS
// QT Node
typedef struct QT {
    // name
    char name[255];
    // Offsets in memory
    int father;
    int A;
    int B;
    int C;
    int D;
    // real important data
    bool leaf;
    float half_length;
    V2 origin;
    // scientific thing, can be changed
    float total_mass;
    V2 center_of_mass;
    Body body;
} QT;

// QT Nodes Orchestrator
typedef struct {
    int QT_total;
    QT * QTList;
} QTMaster;


// GLOBALS
// because QT_MASTER is unique (even in multithread context) and i don't want to pass him all over the galaxy
QTMaster QT_MASTER;

// FUNCTIONS
// essential
QT * getQt(QTMaster * qtmaster, int offset);
int addQT(QTMaster * qtmaster, V2 origin, float half_length, int father, char name[]);
void divideQT(QTMaster * qtmaster, int offset);

// utils
void printQT(QT * qt);
void printOrchestrator(QTMaster * qtmaster);
