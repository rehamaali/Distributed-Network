
#include "Master.h"


Define_Module(Master);

void Master::initialize()
{
    //self reminder
    double interval = exponential(1 / par("lambdaMaster").doubleValue());
    scheduleAt(simTime() + interval, new cMessage(""));
    freeNodes = getParentModule()->par("nodesCount").intValue();
    nodesConnection.resize(freeNodes);
    for(auto& e:nodesConnection)
        e=-1;
}

void Master::handleMessage(cMessage *msg)
{
    if(!(msg->isSelfMessage()))
    {
        int firstNode = msg->getArrivalGate()->getIndex();
        int secondNode = nodesConnection[firstNode];
        nodesConnection[firstNode] = nodesConnection[secondNode] = -1;
        freeNodes += 2;
        return;
    }

    int choose = rand() % 2;           // choose if it will choose two nodes or not.
    EV<<choose;


    if(choose == 0 && freeNodes)                   //will choose
    {

        int rand_src,rand_dest;

        do{
            rand_src = uniform(0, gateSize("outs"));

        } while(nodesConnection[rand_src] != -1);

        do { //Avoid sending to the same node
            rand_dest = uniform(0, gateSize("outs"));
        } while((nodesConnection[rand_dest] != -1) || (rand_dest == rand_src));   // to be modified (check that both nodes are free)

        nodesConnection[rand_src] = rand_dest;
        nodesConnection[rand_dest] = rand_src;
        freeNodes -= 2;

        //Calculate appropriate dest gate number
        if (rand_dest > rand_src)
            rand_dest--;

        //send msg to src (containing dest gate index) to inform it to send a msg to this dest
        delete msg;
        msg = new cMessage(to_string(rand_dest).c_str());
        send(msg,"outs",rand_src);
    }

    //self reminder
    scheduleAt(simTime() + 30, new cMessage(""));
}
