# Programming Assignment 3: SSL
The task was to design a communication protocol and implement a server and a client program that implements the protocol. I use SSL to enforce privacy and authenticity.

## Setup
To setup the application, run the following from the root of this program:
> make -C ./src

## Run
The server needs to be started before the client. To start the server, run the following:
> ./src/chatd [port]

To start the client, run the following:
> ./src/chat [hostname] [port]

## Implementation
When a client has established a secure encrypted connection, a `Welcome to the chat server!` message is displayed. The secure connection is established with TCP and OpenSSL.<br/>

After the connection, the client is asked for his/her username and password. The user gets three tries to enter the correct password, else s/he will need to enter the username again.<br/>

All logs are logged to a file named `chatd.log` located in `./log`. The log contains a timestamp, the client IP and port, and a descriptive message. The timestamp is in ISO 8601 format, precise up to seconds.<br/>

The connection is closed and cleaned up when the user either disconnects, or issues the */bye* command. If a user has been inactive for *60* seconds, then s/he will be removed from the server.<br/>

Both the server and the client are using TSLv1 methods for the SSL_CTX. <br/>

All clients and chatrooms are stored in a GTree at the server. <br/>

A client can only be a member of one chatroom at a time. A subsequent join will make a client to leave a chatroom and join the next one.

## Spec
* Maximum simultaneous users are 256.
* Each message that is sent from/to server/client is 1024 bytes.

## Protocol
> /bye

User will issue the server the aforementioned command.<br/>
  PARAMETERS:<br/>
    none<br/>
  RETURN:<br/>
    none<br/>
  DESCRIPTION:<br/>
    This command will clean up and close the connection on the server.

-------------------------------------;

> /who

User will issue the server the aforementioned command.<br/>
  PARAMETERS:<br/>
    none<br/>
  RETURN:<br/>
    list of all users<br/>
  DESCRIPTION:<br/>
    This command will list the names of all users that are available/authenticated on the system.

-------------------------------------;

> /list

User will issue the server the aforementioned command.<br/>
  PARAMETERS:<br/>
    none<br/>
  RETURN:<br/>
    list of all chatrooms<br/>
  DESCRIPTION:<br/>
    This command will list the names of all available public chat rooms.


-------------------------------------;

> /join [name]

User will issue the server the aforementioned command.<br/>
  PARAMETERS:<br/>
    name: the name of chatroom<br/>
  RETURN:<br/>
    joined and created chatroom: [chatroom]<br/>
    joined chatroom: [chatroom]<br/>
    Already a member of chatroom: [chatroom]<br/>
  DESCRIPTION:<br/>
    This command will join the client to the desired chatroom, if it's available

-------------------------------------;

> /user [username]

User will issue the server the aforementioned command.<br/>
  PARAMETERS:<br/>
    username: the name of the client<br/>
  RETURN:<br/>
    Welcome [username]<br/>
  DESCRIPTION:<br/>
    This command will authenticate him/herself to the server. The server responds with the salt of the client. The client enters a password that is hashed 100.000x with SHA256. The hash is sent over to the server and is validated there to be correct or not. If it's accepted, then the server responds with `1`, else `0`.

## Questions
#### 6.5
The hashed passwords and salts are stored in the keyfile, called `keyfile.ini` "on the server". Hashing is just a one way encryption and cannot be decrypted. If someone would be able to obtain the hashed password, s/he would not be able to decrypt it to plain text. By using a salt, the hashing becomes better and more secure.

#### 7.2
Yes, private messages should be logged. What could be logged is who sent it, when and so on. Private messages should be encrypted using RSA for example. The consequences are that the messages can be decrypted by a possible attacker that can circumvent this.
