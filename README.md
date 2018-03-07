# Network-Programming-MiniProjects
Network Programming MiniProjects : 

# MiniProject1
An HTTP client program that queries Microsoft’s Bing search engine for a search term.

# MiniProject2
A program that discovers and lists all computers that are online (i.e., up running) in a given IP address range.

# MiniProject3
An concurrent HTTP server. It is able to interact with a full-fledged browser, e.g. Firefox. The server serves clients using the standard port 80. The server serves only two types of files: html and jpg. The server is concurrent along two axes: It can serve multiple clients concurrently, and it can serve multiple parallel connections from each client concurrently.

This code needs no command line arguments.
It will bind a socket with wildecard for IP address and port 80
Please compile it, and run the executable file in the root of your website or where ever you have the test files.
This code asssumes that each directory you request will contain "index.html" file, otherwise it will return NOT_FOUND error.
Also that the only types of files you will request are with "html" and "jpg" extensions,otherwise it will return NOT_FOUND error.
for the (POST) method: please move files (p.html and x.html ) in the root where the server is running . 
then request the p.html by typing 127.0.01/p.html on the browser
fill your ID in the box and click on submit.
you can see that your ID will apear in the log.out file in the field that is called (Data entered(ID)).

# MiniProject4
A UDP-based client-server ping application using socket API.
Our program don't have any consideration required to compile and run, in server code: just you need to use "MiniProject4-server.c " to compile it and " ./a.out  PORT# RATIO " to run it and, in client code: just you need to use "MiniProject4-client.c " to compile it and " ./a.out  IP# PORT# #OF PACKETS " to run it .

# MiniProject5
An Web proxy works as follows:
1. It accepts a TCP connection from the browser.
2. It accepts a HTTP request from the browser for some content hosted by some Web server.
3. It performs a DNS lookup of the Web server.
4. It establishes a TCP connection to the Web server.
5. It sends the client’s HTTP request to the Web server.
6. It forwards the response it receives from the Web server to the browser.

Those MiniProjects are done by: Yousef Hadder, Alaa Abu Hantash and Mais Tawalbeh.
