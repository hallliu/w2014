#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Utils/generators.h"
#include "Utils/stopwatch.h"
#include "Utils/fingerprint.h"
#include "Utils/packetsource.h"
#include "queue.h"
#include "parallel_firewall.h"

#define DEFAULT_NUMBER_OF_ARGS 7

void serialFirewall(const int,
					const int,
					const long,
					const int,
					const short);

void serial_queue(int n_packets, int n_src, int q_depth, long mean, int seed, int distr);

int main(int argc, char *argv[]) {

	if(argc >= DEFAULT_NUMBER_OF_ARGS) {
        const int numPackets = atoi(argv[2]);
		const int numSources = atoi(argv[3]);
		const long mean = atol(argv[4]);
		const int uniformFlag = atoi(argv[5]);
		const short experimentNumber = (short) atoi(argv[6]);
        int q_depth = 32;
        if (argc > 7)
            q_depth = atoi(argv[7]);

#ifdef PERF
        double p_time;
#else
        long fp;
#endif
        switch(atoi(argv[1])) {
            case 0:
                serialFirewall(numPackets, numSources, mean, uniformFlag, experimentNumber);
                break;
            case 1:
                serial_queue(numPackets, numSources, q_depth, mean, experimentNumber, uniformFlag);
                break;
            case 2:
#ifdef PERF
                p_time = parallel_dispatcher(numPackets, numSources, q_depth, mean, experimentNumber, uniformFlag);
                printf("%f\n", p_time);
#else
                fp = parallel_dispatcher(numPackets, numSources, q_depth, mean, experimentNumber, 0, uniformFlag);
                printf("%ld\n", fp);
#endif
        }

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

void serial_queue(int n_packets, int n_src, int q_depth, long mean, int seed, int distr) {
    Packet_t **rcvd_packets = malloc (n_packets * n_src * sizeof(Packet_t *));
    PacketSource_t *source = createPacketSource (mean, n_src, seed);
    int packet_ctr = 0;
    long fp_sum = 0;

    StopWatch_t watch;
    startTimer(&watch);

    Packet_t *(*pkt_fn)(PacketSource_t *, int) = distr ? getUniformPacket : getExponentialPacket;
    struct l_queue *queues = create_queues (n_src, q_depth);

    for (int i = 0; i < n_packets; i++) {
        for (int j = 0; j < n_src; j++) {
            Packet_t *pkt = pkt_fn (source, j);
            rcvd_packets[packet_ctr] = pkt;
            enq (queues + j, (void *) pkt);
            packet_ctr++;
            deq (queues + j, (void **) &pkt);
            fp_sum += getFingerprint (pkt->iterations, pkt->seed);
        }
    }
    destroy_queues (n_src, queues);
    stopTimer(&watch);

    deletePacketSource (source);
    // Free all the packets...
    for (int i = 0; i < packet_ctr * n_src; i++)
        free (rcvd_packets[i]);
    free (rcvd_packets);

#ifdef PERF
    printf("%f\n", getElapsedTime(&watch));
#else
    printf("%ld\n", fp_sum);
#endif
    return;
}
