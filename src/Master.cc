
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

    srand(time(0));
}

void Master::handleMessage(cMessage *msg)
{
    if(!(msg->isSelfMessage()))
    {
        int firstNode = msg->getArrivalGate()->getIndex();
        int secondNode = nodesConnection[firstNode];
        nodesConnection[firstNode] = nodesConnection[secondNode] = -1;
        freeNodes += 2;
        EV << "pair(" << firstNode << ", " << secondNode << ") finished" << endl;
        cout << "Release: " << firstNode << " " << secondNode << endl;
        delete msg;
        return;
    }
    int choose = rand() % 2;           // choose if it will choose two nodes or not.

    if(choose == 0 && freeNodes)                   //will choose
    {

        int rand_src,rand_dest;

        do{
            rand_src = uniform(0, gateSize("outs"));

        } while(nodesConnection[rand_src] != -1);
        do { //Avoid sending to the same node
            rand_dest = uniform(0, gateSize("outs"));
        } while((nodesConnection[rand_dest] != -1) || (rand_dest == rand_src));   // to be modified (check that both nodes are free)
        EV << "pair(" << rand_src << ", " << rand_dest << ") started" << endl;
        nodesConnection[rand_src] = rand_dest;
        nodesConnection[rand_dest] = rand_src;
        freeNodes -= 2;

        //Calculate appropriate dest gate number
        if (rand_dest > rand_src)
            rand_dest--;

        //send msg to src (containing dest gate index) to inform it to send a msg to this dest
        cMessage *msg2 = new cMessage(to_string(rand_dest).c_str());
        send(msg2,"outs",rand_src);
    }
    delete msg;

    //self reminder
    scheduleAt(simTime() + 30, new cMessage(""));
}
