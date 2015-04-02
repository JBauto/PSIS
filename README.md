# PSIS
# Webserver 
This project was developed for Systems Programming course (2ºSemester 2013/2014).
Main goal was to design a web-server that was able to respond to multiple concurrent request at the same time. To accomplish this two techniques were used.
* Pool of Threads - a group of slave threads that analyze a request that is send to them by a master thread that read the requests from a FIFO queue.
* Process - allowing to create multiple replicas of thread pools.

This project was tested with ab, Apache HTTP server benchmarking tool (http://httpd.apache.org/docs/2.2/programs/ab.html), and was capable to respond to up to 1500 concurrent request. 

Known Problems:
* Since each master thread reads a request from a FIFO queue this creates a bottleneck and if a request arrives late and all threads are occupied this increases the respond time to that request.
* Memory management -  A dynamic system was created in case of an increase number of request and all thread pools are occupied. In low specs systems this may crash your system.
* Number of threads per pool -  Since there wasn't much time to do proper testing of the number of threads per pool, an optimized solution wasn't achieved.

This project was developed with João Graça.
