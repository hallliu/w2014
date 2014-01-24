
#ifndef HASHPACKETTEST_H_
#define HASHPACKETTEST_H_

void serialHashPacketTest(int numMilliseconds,
							float fractionAdd,
							float fractionRemove,
							float hitRate,
							int maxBucketSize,
							long mean,
							int initSize);


void parallelHashPacketTest(int numMilliseconds,
							float fractionAdd,
							float fractionRemove,
							float hitRate,
							int maxBucketSize,
							long mean,
							int initSize,
							int numWorkers);



#endif /* HASHPACKETTEST_H_ */
