Firstly, creat a database called postgres with user postgres and password is passw0rd.  
alter user postgres with password 'passw0rd';  
create database postgres owner postgres;  

To start the server,please change the servername to your own machine in server.cpp,port number is 12345.  
To test the functionality, please change the servername first in client.cpp then run ./client textX.txt.   
To test the scalability, please change the servername first in client.cpp the run ./client num of threads like ./client 4
