Firstly, creat a database called postgres with user postgres and password is passw0rd. 

alter user postgres with password 'passw0rd';  
create database postgres owner postgres;  

or you can just change the information of the database to your own in line 75:server.cpp.     

To start the server,please cd into the folder "server" and do make clean && make. Then do ./server.  

To test the functionality in testing folder, please change the servername first in client.cpp then run ./client textX.txt.   

To test the scalability in testing folder , please change the servername first in client.cpp the run ./client (num of threads) like ./client 4
