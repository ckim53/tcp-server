# Multithreaded TCP Server 

My implementation of a multithreaded TCP server built in C++ with CMake for build automation. Includes support for concurrent client connections, a custom application layer protocol, and shared state management.

# How to build the project
``cmake -B build``
``cmake --build build``

### Navigate to build directory:
``cd build``
### Run the server executable:
``./server``
### Run the client executable in a separate terminal:
``./client``

### Send a message from the client and see it received by the server and broadcasted to other clients created!

# Features in Action
Concurrency: The server uses a thread pool to manage multiple clients simultaneously without blocking.
Broadcasting: Messages sent from one client are processed by the server and broadcasted to all other connected clients in real-time.
Shared State: Demonstrates thread-safe data handling across the server's worker threads.
