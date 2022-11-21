Program 1: Client using customized protocol on top of UDP protocol for sending information to the server.


Follow the below steps for Program Compilation: 

1. Launch the terminal and execute the command “gcc -o server server.c” to compile server.c file.   
2. Open another terminal and execute the command “gcc -o client  client.c” to compile client.c file.
3. Now, Run the server first by using the command "./server <port number>", for example "./server 3335".
4. Next, Run the client by using the command "./client <host name> <port number>", for example "./client localhost 3335".
5. Make sure to run the server first and then the client. Also, Make sure to run the server and client with the same port number.