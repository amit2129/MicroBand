# **Prologue:**

The OSI model of a networking system has 7 layers, from "lowest" to "highest", 1. Physical, 2. Link, 3. Network, 4. **Transport** and the rest of the application layers.
Since we don't have osciloscopes, we will not implement the physical layer, we also will not be implementing the Link layer due to our possible applications either not allowing
us to do this (Linux) or it being less performant than the existing solution (NRF24L01) for microcontrollers.
Under the assumption that the network layer supplies us with relevant packets, not necessarily in order we will define the implmentation
of the transport layer.


## The Transport Layer


### The goals of the implementation:

* Should be process independent, decentralized, each process carries out its own packet management.
* Should discard a packet if the target qp is unknown to the process.
* Should have a basic fixed length header which will allow dispatch to the correct QP.
* Should have a *central* object id allocation scheme as to not have id collisions.

### The basic implmentation scheme (for Linux)

	   	       64 bits
	.________.________.________.________.
	|		  |		    |
	|    Source QP    |	Dest QP	    |
	|_________________|_________________|
	|		  | 		    |
	| Sequence Number |    Ack Number   |
	|_________________|_________________|
	|        |                          |
	|DataLen |        Reserved          |
	|________|__________________________|
	|                                   |
	|                                   |
	|                                   |
	|               DATA                |
	|                                   |
	|                                   |
	|___________________________________|

QP IDs are 32 bits
Sequence and Acknowledgment number are 32 bits
DataLen is 16 bits, limiting us to 65536 byte packets and we keep another 48 bits reserved.

### The basic implmentation scheme (for MCUs NRF24L01 limitations)

	   	       8 bits
	.___________________________________.
	|	 |	  |        |        |
	| Src QP | Dst QP | SeqNum | AckNum |
	|________|________|________|________|
	|                 |                 |
	|     DataLen     |     Reserved    |
	|_________________|_________________|
	|                                   |
	|                                   |
	|               DATA                |
	|                                   |
	|                                   |
	|___________________________________|

QP IDs are 2 bits meaning we can have 4 QPs on each side
Sequence and Acknowledgmnt numbers are 2 bits each
DataLen is 4bits which means our max datalen is 8 bits.
Another 4 bits are reserved which leaves us with 16 bits for actual data.
perhaps we can implement a checksum in the reserved section
