# Lockless Producer-Consumer Queue
A custom implementation of a lockless producer/consumer queue

*Work in progress, haven't been tested thoughtfully yet. Use at your own risks.*

The adventage of the lockless design is that it doesn't use mutexes or any knid of syncronisation between threads and is therefore faster. It also doesn't block 
the producer thread (usually the main thread) while inserting in the queue. (although, the producer thread will do the memory allocation 
so there might be some slowdown comming from that, but you would have that with any kind of queue anyway) 
(but maybe I'll implement some sort of bucket allocation mechanism at some point to make it faster)

Currently supports only one producer thread and one consumer thread at a time (so maximum 2 thread using the queue, but can also work in single threaded
applications). Also, the queue's lifetime should be higher than the threads using it (meaning that it should only be destroyed after at least one of the threads finishes)
(or one thread might call the destructor while the other one is still accessing it).

This queue is perfect for a threaded logger implementation for instance, where the main thread will produce messages to be logged and the consumer
will log them to a file, send them to a server or do any kind of heavy work on it that would usually slow down the main thread.
Or basicly any application where you want to produce fast (not slow down the producer thread), and where you are using a dedicated thread to process 
the consumed data.

# Known limitations
Since this is only designed to work with one producer and one consumer thread there is some issues that will araise if you try to ignore this restriction :
- If used with more than one producer thread, race conditions will make you loose some data from the consumers (and thus having memory leaks).
- If used with more than one consumer thread, race conditions will make you consuming the same information twice, 
and more likely crash from double free memory violation before you can actually get one of thoses duplicate value.

# Contributing
Contributing is welcomed, feel free to open issues / feature requests or pull requests as you wish
