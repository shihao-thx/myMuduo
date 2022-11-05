# myMuduo
An online library modeled after muduo.
# First of all
In the project, I try to write a network lib from the ground up with reference to muduo. The 'from ground' means I want to explain why we need the lib, how to design it and why design it this way.

If you are new to network programing, I recommend you first learn how network works and coding based on socket interface. For example, when the data sended from another computer coming, what happened during the process until we see the content from the screen. In the socket coding, you should know the key API like socket(), bind(), listen(), and so on. I also recommend you try to use IO multiplexing to write your own concurrent server program to feel the effect of IO multiplexing on improving your server's ability to serve more clients. Here are some very good articles in Chinese to somewhat help you. [Package in Network](https://mp.weixin.qq.com/s/iSZp41SRmh5b2bXIvzemIw), [Epoll Source Code Analysis](https://mp.weixin.qq.com/s/OmRdUgO1guMX76EdZn11UQ), and others the authors wrote.
# How to start
When we talk about network lib, we are talking about shielding the details of building a network connecting and make it possible to only focus on what we want to transfer through the Internet. For a server, it's obvious that we expect it can handle many connection requests and response the requests immediately. The requests, we call them events. How to make the server services concurrent? The easy way we can get is we have a listen event loop in the main thread, and the work threads are responsible for read/write event. If we assign every connection to each thread, it's no doubt that we have a huge context switch burden. It's not a good choice. The problem now is how we can use the limited thread to reply events. 

Muduo uses reactor model and follow the principle "one (events)loop per thread". The number of the threads is fixed, usually equals to CPUs. The listen event loop in the main thread, and read/write event loop in the other threads. The key point is how the connected fd from the listen loop transfer to the other loops and then read/write using the connected fd. Maybe we should make a **Channel** to do that. And another question is what other things of event to transfer as a channel. which thread we should choose. 

The core of reactor model in muduo is that one reactor have a IO multiplexing to accept new connection, a sub-reactor is assigned to read/write loop. Why is this design efficient? Because a lot of connections do not mean all of them are active. We only pay attention to ready events in a loop. The select/poll/epoll can give us the ready events. If a event needs the loop thread work hard, we can also put it into a special thread, in case blocking the loop thread. How to make it come true?
# Further on
You may notify the above every paragraph raised a question and the answer of the question is in the class, **Channel** and **Poller**, designed well in muduo. In the source code, I gave more details about the classes, please refer to it. 



# Deep into

# Changes
