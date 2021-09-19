#include "root.h"

RootComponent rootC;

RootComponent* getRootClass(){
  return &rootC;
}


void setup(){
  rootC.init();
}

void loop(){
  rootC.onLoop();
}