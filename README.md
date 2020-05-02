# QtChat
Qt based simple server client chat application communicating over TCP.

The application illustrate how a simple client server chat application works over a TCP connection. Few of the salient feature of the application are :
1. How a client can communication to another client via a server.
2. How a bulky message can be send over the socket. Where following scenario were taken care,
  - The socket did not receive enough data to be a complete message, we only received a partial
  - There is exactly enough data in the socket buffer to read a message
  - There is more than enough data in the socket buffer to read a message but not enough to read 2 messages
  - There is enough data in the socket buffer to read multiple messages
3. Also, to illustrate that the server needs to implement a multi-thread infrastructure to handle multiple clients.


Below is the system diagram of the application:
![](system%20diagram.PNG)
