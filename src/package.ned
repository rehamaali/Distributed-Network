package deathlyhallows_sf2020;

@license(LGPL);
import ned.IdealChannel;

network Network
{
    parameters:
        //number of nodes:
        int nodesCount = default(6);
        
    submodules:
        //nodes: send and recieve from each others
        nodes[nodesCount]: Node;
        //master: randomly choose which src to send to which dest  
        master: Master;
        
    connections:
        //connect each node to the all other nodes(inout connection)
        for i=0..sizeof(nodes)-1, for j=i+1..sizeof(nodes)-1 {
            nodes[i].outs++ --> IdealChannel --> nodes[j].ins++;
            nodes[i].ins++ <-- IdealChannel <-- nodes[j].outs++;
        }
        //connect each node to the masters (master only send to nodes)
        for i=0..sizeof(nodes)-1 {
            master.outs++ --> IdealChannel --> nodes[i].ins++;
            master.ins++ <-- IdealChannel <-- nodes[i].outs++;
        }
}

