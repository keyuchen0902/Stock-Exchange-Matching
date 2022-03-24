#include "functions.h"


/*
XMLDocument *convertToXML(string xml){
    XMLDocument *doc = new XMLDocument();
    doc->Parse(xml.c_str());
    return doc;

}*/

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
        std::cout << "Can't open database" << std::endl;
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
      response = handleTranscation(C,xml);
  }

}

bool checkAlpha(string sym){
    if (sym.empty()) {
    return false;
    }
  for (std::string::const_iterator it = sym.begin(); it != sym.end(); ++it) {
    if (!std::isalpha(*it) && !std::isdigit(*it)) {
      return false;
    }
  }
  return true;
}

bool checkDigits(string id){
    if (id.empty()) {
    return false;
  }

  for (std::string::const_iterator it = id.begin(); it != id.end(); ++it) {
    if (!std::isdigit(*it)) {
      return false;
    }
  }

  return true;
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
            string id = currElem->FirstAttribute()->Value();//Q检查id是否为string
            double balance = currElem->FindAttribute("balance")->DoubleValue();
            if(balance <= 0 || Account::idExists(C,id) == true || checkDigits(id) == false){
                //写error 是否要直接return
                XMLElement* usernode = response->NewElement("error");
                usernode->SetAttribute("id", id.c_str());
                usernode->InsertFirstChild(response->NewText("id is not digits or balance is wrong or account id has existed"));
                root->InsertEndChild(usernode);
                /*response->SaveFile("test.xml");
                // XML 写到字符串
                XMLPrinter printer;
                response->Print(&printer);         //将Print打印到Xmlprint类中 即保存在内存中
                string buf = printer.CStr(); //转换成const char*类型
                cout << buf << endl;         // buf即为创建后的XML 字符串。
                return response;*/
            }else{//创建一个account
                Account::addAccount(C,id,balance);
                XMLElement* usernode = response->NewElement("created");
                usernode->SetAttribute("id", id.c_str());
                root->InsertEndChild(usernode);
            }
        }else if(strncmp(currElem->Name(),"symbol",6) == 0){
             //to do
            string sym = currElem->FirstAttribute()->Value();
            XMLElement *currAccount = currElem->FirstChildElement();
            while(currAccount != NULL){
                string id = currAccount->FirstAttribute()->Value();
                int amount = stoi(currAccount->GetText());//Q 检查text是否是数字
                currAccount = currAccount->NextSiblingElement();
                if(!checkAlpha(sym)|| amount <0 || Account::idExists(C,id) == false || checkDigits(id) == false){
                    XMLElement* usernode = response->NewElement("error");
                    usernode->SetAttribute("sym",sym.c_str());//Q 加两个对象
                    usernode->SetAttribute("id", id.c_str());
                    usernode->InsertFirstChild(response->NewText("symbol or id format is wrong or account id has not existed or the num of share is negative"));
                    root->InsertEndChild(usernode);
                    /*response->SaveFile("test.xml");
                    // XML 写到字符串
                    XMLPrinter printer;
                    response->Print(&printer);         //将Print打印到Xmlprint类中 即保存在内存中
                    string buf = printer.CStr(); //转换成const char*类型
                    cout << buf << endl;         // buf即为创建后的XML 字符串。

                    return response;*/
                }else{
                    Position::addPosition(C,sym,id,amount);
                    XMLElement* usernode = response->NewElement("created");
                    usernode->SetAttribute("sym",sym.c_str());
                    usernode->SetAttribute("id", id.c_str());
                    root->InsertEndChild(usernode);

                }
            }
        }
        currElem = currElem->NextSiblingElement();
    }
    //是否要save 怎么save 似乎需要一个文件路径
     response->SaveFile("test.xml");
    // XML 写到字符串
    XMLPrinter printer;
    response->Print(&printer);         //将Print打印到Xmlprint类中 即保存在内存中
    string buf = printer.CStr(); //转换成const char*类型
    cout << buf << endl;         // buf即为创建后的XML 字符串。
    return response;

}

XMLDocument* handleTranscation(connection *C, string request){
    XMLDocument* response = new XMLDocument();

    XMLDocument *xml = new XMLDocument();//Q
    xml->Parse(request.c_str());

    XMLElement* root = response->NewElement("results");
    response->InsertEndChild(root);

    XMLElement* xml_root  =xml->RootElement();
    string id = xml_root->FindAttribute("id")->Value();
    if(Account::idExists(C,id) ==false || !checkDigits(id)){
        //写error 是否要直接return
        XMLElement* usernode = response->NewElement("error");
        usernode->SetAttribute("id", id.c_str());
        usernode->InsertFirstChild(response->NewText("account id has not existed or id format wrong"));
        root->InsertEndChild(usernode);
        response->SaveFile("test1.xml");
        // XML 写到字符串
        XMLPrinter printer;
        response->Print(&printer);         //将Print打印到Xmlprint类中 即保存在内存中
        string buf = printer.CStr(); //转换成const char*类型
        cout << buf << endl;         // buf即为创建后的XML 字符串。
        return response;
    }
    XMLElement* currElem = xml_root->FirstChildElement();
    while(currElem != NULL){
        if(strncmp(currElem->Name(),"order",5) == 0){
            string sym = currElem->FirstAttribute()->Value();
            string amount = currElem->FindAttribute("amount")->Value();
            string limit = currElem->FindAttribute("limit")->Value();
            if(!checkAlpha(sym)|| stoi(amount) == 0 || stod(limit) < 0){
                cout << stoi(amount) <<endl;
                cout << stod(limit) <<endl;
                XMLElement* usernode = response->NewElement("error");
                usernode->SetAttribute("sym", sym.c_str());
                usernode->SetAttribute("amount", amount.c_str());
                usernode->SetAttribute("limit", limit.c_str());
                usernode->InsertFirstChild(response->NewText("sym or amount or limit format wrong"));
                root->InsertEndChild(usernode);
                response->SaveFile("test.xml");
                // XML 写到字符串
                XMLPrinter printer;
                response->Print(&printer);         //将Print打印到Xmlprint类中 即保存在内存中
                string buf = printer.CStr(); //转换成const char*类型
                cout << buf << endl;         // buf即为创建后的XML 字符串。
                return response;
            }
            if(stoi(amount) > 0){//检查账户余额是否充足，并更新账户余额
                double requiredBalance = stoi(amount)*stod(limit);
                
                if(!Account::enoughBalance(C,id,requiredBalance)){
                    XMLElement* usernode = response->NewElement("error");
                    usernode->SetAttribute("sym", sym.c_str());
                    usernode->SetAttribute("amount", amount.c_str());
                    usernode->SetAttribute("limit", limit.c_str());
                    usernode->InsertFirstChild(response->NewText("account balance is not enough"));
                    root->InsertEndChild(usernode);
                    response->SaveFile("test.xml");
                    XMLPrinter printer;
                    response->Print(&printer);         //将Print打印到Xmlprint类中 即保存在内存中
                    string buf = printer.CStr(); //转换成const char*类型
                    cout << buf << endl;         // buf即为创建后的XML 字符串。
                    return response;
                }
            }else{//amount<0 means sell 检查symbol的amount是否足够
                if(!Position::updateSymbolAmount(C,sym,id,0-stoi(amount))){
                    XMLElement* usernode = response->NewElement("error");
                    usernode->SetAttribute("sym", sym.c_str());
                    usernode->SetAttribute("amount", amount.c_str());
                    usernode->SetAttribute("limit", limit.c_str());
                    usernode->InsertFirstChild(response->NewText("symbol amount is not enough"));
                    root->InsertEndChild(usernode);
                    response->SaveFile("test.xml");
                    XMLPrinter printer;
                    response->Print(&printer);         //将Print打印到Xmlprint类中 即保存在内存中
                    string buf = printer.CStr(); //转换成const char*类型
                    cout << buf << endl;         // buf即为创建后的XML 字符串。
                    return response;
                }
            }
            //now creat a transaction
            int id_trans = Transaction::addTransaction(C,id,sym,stod(limit),stoi(amount));
            //Match one possible at a time ??
            XMLElement* usernode = response->NewElement("opened");
            usernode->SetAttribute("sym",sym.c_str());
            usernode->SetAttribute("amount", amount.c_str());
            usernode->SetAttribute("limit", limit.c_str());
            usernode->SetAttribute("id", id_trans);
            root->InsertEndChild(usernode);
        }
        /*if(strncmp(currElem->Name(),"cancel",6) == 0){

        }
        if(strncmp(currElem->Name(),"query",5) == 0){

        }*/
        currElem = currElem->NextSiblingElement();
    }
    response->SaveFile("test2.xml");
    XMLPrinter printer;
    response->Print(&printer);         //将Print打印到Xmlprint类中 即保存在内存中
    string buf = printer.CStr(); //转换成const char*类型
    cout << buf << endl;         // buf即为创建后的XML 字符串。
    return response;
}

long getCurrTime() {
  stringstream ss;
  ss << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::_V2::system_clock::now().time_since_epoch()).count();
  long time;
  ss >> time;
  return time;
}