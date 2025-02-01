Welcome to the chat project
===========================

This is a project for the course system programming where we implemented a chat server.
The following notes will help you getting started.

Cloning the repository
======================

In case you forgot how to get a local copy of the repository from the server to your development machine, here is
a little reminder:

1. Copy the repository address to the clipboard using the corresponding button.
2. Open a terminal and change to the directory where you want your local repository to be stored.
   Remember that git will create a subdirectory with the name of the project.
3. Use `git clone <Repository>`

Using CMake to build the project
================================

You will use CMake to build the project, so you do not have to write your own Makefile.
Even better, you do not even have to deal a lot with CMake yourself, as the
[required `CMakeLists.txt` control file](CMakeLists.txt) is already included.

To build the project, change to the build subdirectory, execute `cmake` and then `make`:
```
cd chat/build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```
In the lines above, the Debug build type was selected, so you can use `gdb` or another debugger during development.

If you decide to introduce additional modules, you can add them to `SERVER_MODULES` in
[`CMakeLists.txt`](CMakeLists.txt).

Description of the modules
==========================

You will see several modules in the [`src/` subdirectory of the project](src/).
Each module has a given purpose as described below.

`broadcastagent`
----------------

Here is the broadcast agent implemented which handle and send messages to clients.

`clientthread`
--------------

Here is the thread function for the client thread implemented, heavily using the functions provided by the
`network` and `user` modules.

`connectionhandler`
-------------------

This is where incoming client connections are handled and where the `clientthread` is started.
Of course, to make this work, you will have to create the server socket first.

`main`
------

Pretty obvious, isn't it? Here the command line arguments are evaluated and other modules are initialized.

`network`
----------

This is the module dealing with the network messages. Here the message strucures are defined and sending and
receiving them are implemented.

`user`
------

Here the double-linked list is implemented, containing a node for every connected and logged in user.
As this data is shared by multiple threads, proper locking is used here.

`util`
------

Several utility functions, hopefully making your lives a little bit easier:

* `utilInit()`: Initialize the util module, setting the program name given.
   Must be called before any of the output functions are used.
* `normalPrint()`, `debugPrint()`, `infoPrint()` `errorPrint()`: These are `printf()`-like output functions to
  pretty-print regular, debug, informational or error messages.
  They use colors (unless disabled via `styleDisable()`) and also print the program name in front.
  Debug messages are only printed if enabled with `debugEnable()` before.
* `errnoPrint()`: This is `perror()` on steroids, using colors and a `printf()`-like prefix.
* `debugHexdump()`, `hexdump()`, `vhexdump()`: Use these to dump data in a nice hexadecimal form.
  Great for debugging or for a nice Matrix effect.
* `getProgName()`: Get the program name used by the output functions.
* `debugEnable()`, `debugDisable()`, `debugEnabled()`: Enable or disable `debugPrint()` and `debugHexdump()`, or
  get the status.
* `styleEnable()`, `styleDisable()`, `styleEnabled()`: Enable or disable colorful output, or get the status.
* `nameBytesValidate()`: Checks if a given buffer only contains bytes that are valid in user or server names.
* `ntoh64u()`, `hton64u()`: POSIX lacks a portable way to convert 64 bit values from network into host byte order and
  vice versa. Luckily, filling this gap is not too hard.

Client for the Chat
===================

The program of the client was given.
To start the client open a terminal in the directory of the program and use `./client -n USERNAME`.

What I have learned
===================

* Working with pointers, structures and unions in C
* Using Linux / POSIX system features
* Saving data in memory (dynamic allocation)
* Multitasking with processes and threads
* Synchronizing global data between asynchronous threads with POSIX mutexobjects and semaphores
* Modular programming in C
* Establish connections with sockets
* Implementing client-server systems using TCP sockets
* Working with program parameters
* Implementing a [custom protocol](protocol.txt) for the chat
* Sending and receiving data packets of variable length considering the byteorder and memory gaps
* Using POSIX-Message Queues