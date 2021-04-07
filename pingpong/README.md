# Ping Pong exercise
Simple exercise where a process sends messages to another process.
We plot the bandwidth obtained with different message sizes and different methods.

## Folder structure
* **normal**: contains the code to compare the performance across different message sizes and the related results.
* **1024**: slight variation of the original exercise with fixed message length. We compare the bandwidth obtained by the following two methods:
  * (*multiple*) we send multiple messages in the same execution
  * (*single*) we perform multiple executions where we send a single message

## Results
* **normal**:
  - *inter-node*: as expected, since we increase the message size exponentially, also the time chart looks exponential. As for the bandwidth, at a certain message size it caps out. We can also see that as the message size grows, also the bandwidth grows. This is probably due to the reduced relative overhead of message passing.
  - *intra-node*: what we said for the previous case holds also for this one, except for the fact that we get a bandwidth about 5-10 times larger. It is interesting to see that when we try to send messages larger than about 32 MB, which was problematic to test inter-node, the bandwidth is halved.  
* **1024**: 
  * (*multiple*)
    * *inter-node*: we can note a curious pattern in the bandwidth obtained, since the values align along some horizontal lines, though some outliers are present.
    * *intra-node*: no major difference, just better performance.
  * (*single*)
    * *inter-node*: here we observe a wilder distribution, with no major pattern we are able to detect.
    * *intra-node*: similar to the previous point, but with a strange behaviour in the last iterations, where the bandwidth clearly stabilizes in the lower range of the values.

