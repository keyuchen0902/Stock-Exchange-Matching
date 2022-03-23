#include "functions.h"
#include "tinyxml2.h"
using namespace tinyxml2;
using namespace std;

XMLDocument *convertToXML(string xml){
    XMLDocument *doc = new XMLDocument();
    doc->Parse(xml.c_str());
    return doc;

}

void handleRequest(int client_fd){
    pqxx::connection *C;
    try {
        // Establish a connection to the database
        // Parameters: database name, user name, user password
        C= new pqxx::connection(
            "dbname=postgres user=postgres password=passw0rd");
        if (C->is_open()) {
        // cout << "Opened database successfully: " << C->dbname() << endl;
        } else {
        cout << "Can't open database" << std::endl;
        exit(EXIT_FAILURE);
        }
    } catch (const std::exception &e) {
        cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    if (C == NULL) {
    return;
    }
    // loop and dispatch
    char buffer[40960];
    XMLDocument *response = new XMLDocument();//Q
    //string response;

  // Reset buffer
  memset(buffer, '\0', sizeof(buffer));

  // Recv
  int totalsize = recv(client_fd, buffer, 40960, 0);
  if (totalsize < 0)
    cout << "Error receive buffer from client." << std::endl;

  string xml(buffer);
  //conver stringxml to a XMLDocument object
  /*XMLDocument *doc = new XMLDocument();
  doc->Parse(xml.c_str());*/
  if (xml.find("<create>") != std::string::npos){
      response = handleCreat(C,xml);//Q
  }else if (xml.find("<transactions") != std::string::npos){
      response = handleTranscation(C,xml,response);
  }

}


XMLDocument* handleCreat(connection *C, string request){
    XMLDocument* response = new XMLDocument();

    XMLDocument *xml = new XMLDocument();//Q
    xml->Parse(request.c_str());

    XMLElement* root = response->NewElement("results");
    response->InsertEndChild(root);

    XMLElement* xml_root  =xml->RootElement();
    XMLElement* currElem = xml_root->FirstChildElement();
    while(currElem != NULL){
        if(strncmp(currElem->Name(),"account",7) == 0){
            int id = currElem->FirstAttribute()->IntValue();
            double balance = currElem->FindAttribute("balance")->DoubleValue();
            string id = to_string(id);
            if(balance <= 0 || Account::idExists(C,id) == true){
                //写error
            }else{//创建一个account
                Account::addAccount(C,id,balance);
            }
            XMLElement* usernode = response->NewElement("created");
            string id = to_string(id);
            usernode->SetAttribute("id", id);
            root->InsertEndChild(usernode);
            
        }else if(strncmp(currElem->Name(),"symbol",6) == 0){

        }
        currElem = currElem->NextSiblingElement();
    }
    return response;

}

XMLDocument* handleTranscation(connection *C, string request,XMLDocument response){
    
}