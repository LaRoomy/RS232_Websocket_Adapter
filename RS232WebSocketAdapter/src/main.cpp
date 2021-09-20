#include "root.h"

// create root instance
RootComponent rootC;

// valid access to root instance from all locations
RootComponent* getRootClass(){
  return &rootC;
}

void setup(){
  // initialize component
  rootC.init();
}

void loop(){
  // run
  rootC.onLoop();
}