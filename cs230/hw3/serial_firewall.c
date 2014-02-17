#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Utils/generators.h"
#include "Utils/stopwatch.h"
#include "Utils/fingerprint.h"
#include "Utils/packetsource.h"

void serialFirewall (int, int, long, int, short);

int main(int argc, char *argv[]) {

	if(argc >= 6) {
        const int numPackets = atoi(argv[1]);
		const int numSources = atoi(argv[2]);
		const long mean = atol(argv[3]);
		const int uniformFlag = atoi(argv[4]);
		const short experimentNumber = (short) atoi(argv[5]);

        serialFirewall(numPackets, numSources, mean, uniformFlag, experimentNumber);
    }
    return 0;
}

void serialFirewall (int numPackets,
					 int numSources,
					 long mean,
					 int uniformFlag,
					 short experimentNumber) {

    PacketSource_t *packetSource = createPacketSource(mean, numSources, experimentNumber);
    Packet_t **rcvd_pkts = malloc (numSources * numPackets * sizeof(Packet_t *));
    int packet_ctr = 0;

    StopWatch_t watch;
    long fingerprint = 0;

    if(uniformFlag) {
        startTimer(&watch);
        for(int i = 0; i < numSources; i++) {
            for(int j = 0; j < numPackets; j++) {
                Packet_t *tmp = getUniformPacket(packetSource,i);
                fingerprint += getFingerprint(tmp->iterations, tmp->seed);
                rcvd_pkts[packet_ctr] = tmp;
                packet_ctr += 1;
            }
        }
        stopTimer(&watch);
    }

    else {
        startTimer(&watch);
        for(int i = 0; i < numSources; i++) {
            for(int j = 0; j < numPackets; j++) {
                Packet_t *tmp = getExponentialPacket(packetSource,i);
                fingerprint += getFingerprint(tmp->iterations, tmp->seed);
                rcvd_pkts[packet_ctr] = tmp;
                packet_ctr += 1;
            }
        }
        stopTimer(&watch);
    }
    for (int i = 0; i < packet_ctr; i++) {
        free(rcvd_pkts[i]);
    }
    free (rcvd_pkts);
    
#ifdef PERF
    printf("%f\n",getElapsedTime(&watch));
#else
    printf("%ld\n", fingerprint);
#endif
}
