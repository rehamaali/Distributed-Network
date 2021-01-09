
#include "Master.h"
using namespace std;

Define_Module(Master);

void Master::initialize()
{
    //self reminder
    double interval = exponential(1 / par("lambda").doubleValue());
    scheduleAt(simTime() + interval, new cMessage(""));
}

void Master::handleMessage(cMessage *msg)
{
    //always recieve self msg (till now)

    int rand_src,rand_dest,dest;
    rand_src = uniform(0, gateSize("outs"));

    do { //Avoid sending to the same node
        rand_dest = uniform(0, gateSize("outs"));
    } while(rand_dest == rand_src);

    //Calculate appropriate dest gate number
    dest = rand_dest;
    if (rand_dest > rand_src)
        dest--;

    //send msg to src (containing dest gate index) to inform it to send a msg to this dest
    delete msg;
    msg = new cMessage(to_string(dest).c_str());
    send(msg,"outs",rand_src);

    //self reminder
    double interval = exponential(1 / par("lambda").doubleValue());
    scheduleAt(simTime() + interval, new cMessage(""));
}
