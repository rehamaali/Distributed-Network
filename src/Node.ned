
package deathlyhallows_sf2020;

simple Node
{
    parameters:
        double lambdaNode = default (2.0);
        double propgationDelay = default(0.1);
        double frameTimeout = default(0.3);
        double ackTimeout = default(0.2);
        double networkReadyDelay = default(0.1);
        
    
    gates:
        input ins [];
        output outs [];
}
