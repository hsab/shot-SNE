#include "vid.h"

Vid::Vid()
{
}

Vid::~Vid()
{
    threadedObject.stop();
}

void Vid::setup()
{
    x = ofRandom(0, ofGetWidth()); // give some random positioning
    y = ofRandom(0, ofGetHeight());

    speedX = ofRandom(-1, 1); // and random speed and direction
    speedY = ofRandom(-1, 1);

    dim = 20;

    color.set(threadedObject.getProcessed() * 255);
}

void Vid::update()
{
    if (x < 0) {
        x = 0;
        speedX *= -1;
    } else if (x > ofGetWidth()) {
        x = ofGetWidth();
        speedX *= -1;
    }

    if (y < 0) {
        y = 0;
        speedY *= -1;
    } else if (y > ofGetHeight()) {
        y = ofGetHeight();
        speedY *= -1;
    }

    x += speedX;
    y += speedY;

    threadedObject.updateNoLock();
}

void Vid::draw()
{
    ofSetColor(threadedObject.getProcessed() * 255);
    ofDrawCircle(x, y, dim);

    threadedObject.draw();
}

void Vid::hecate(string cmd)
{
    threadedObject.setup(cmd);
}
