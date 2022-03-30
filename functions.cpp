#include "functions.h"

void handleRequest(int client_fd)
{
    connection *C;
    try
    {
        // Establish a connection to the database
        // Parameters: database name, user name, user password
        C = new connection(
            "dbname=postgres user=postgres password=passw0rd");
        if (C->is_open())
        {
            // cout << "Opened database successfully: " << C->dbname() << endl;
        }
        else
        {
            cout << "Can't open database" << endl;
            exit(EXIT_FAILURE);
        }
    }
    catch (const std::exception &e)
    {
        cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    if (C == NULL)
    {
        return;
    }
    // loop and dispatch
    char buffer[40960];
   
    XMLDocument *response = new XMLDocument(); // Q
    // string response;

    // Reset buffer
    memset(buffer, '\0', sizeof(buffer));

    // Recv
    int totalsize = recv(client_fd, buffer, 40960, 0);
    if (totalsize < 0)
        cout << "Error receive buffer from client." << std::endl;

    string xml(buffer);
    // conver stringxml to a XMLDocument object
    /*XMLDocument *doc = new XMLDocument();
    doc->Parse(xml.c_str());*/
    if (xml.find("<create>") != std::string::npos)
    {
        response = handleCreat(C, xml); // Q
    }
    else if (xml.find("<transactions") != std::string::npos)
    {
        response = handleTranscation(C, xml);
    }

    XMLPrinter printer;
    response->Print(&printer);   //将Print打印到Xmlprint类中 即保存在内存中
    string buf = printer.CStr(); //转换成const char*类型

    send(client_fd, buf.c_str(), buf.length(), 0);
    // Close database connection
    C->disconnect();
    delete C;

    // Close connection
    close(client_fd);
}

bool checkAlpha(string sym)
{
    if (sym.empty())
    {
        return false;
    }
    for (std::string::const_iterator it = sym.begin(); it != sym.end(); ++it)
    {
        if (!std::isalpha(*it) && !std::isdigit(*it))
        {
            return false;
        }
    }
    return true;
}

bool checkDigits(string id)
{
    if (id.empty())
    {
        return false;
    }

    for (std::string::const_iterator it = id.begin(); it != id.end(); ++it)
    {
        if (!std::isdigit(*it))
        {
            return false;
        }
    }

    return true;
}

void getAccountError(XMLDocument *response, XMLElement *root, string id, string msg)
{
    XMLElement *usernode = response->NewElement("error");
    usernode->SetAttribute("id", id.c_str());
    usernode->InsertFirstChild(response->NewText(msg.c_str()));
    root->InsertEndChild(usernode);
}

void getSymbolError(XMLDocument *response, XMLElement *root, string sym, string id, string msg)
{
    XMLElement *usernode = response->NewElement("error");
    usernode->SetAttribute("sym", sym.c_str()); // Q 加两个对象
    usernode->SetAttribute("id", id.c_str());
    usernode->InsertFirstChild(response->NewText(msg.c_str()));
    root->InsertEndChild(usernode);
}

XMLDocument *handleCreat(connection *C, string &request)
{
    XMLDocument *response = new XMLDocument();

    XMLDocument *xml = new XMLDocument(); // Q
    xml->Parse(request.c_str());

    XMLElement *root = response->NewElement("results");
    response->InsertEndChild(root);

    XMLElement *xml_root = xml->RootElement();
    XMLElement *currElem = xml_root->FirstChildElement();
    while (currElem != NULL)
    {
        if (strncmp(currElem->Name(), "account", 7) == 0)
        {
            string id = currElem->FirstAttribute()->Value(); // Q检查id是否为string
            double balance = currElem->FindAttribute("balance")->DoubleValue();

            if (Account::idExists(C, id) == true || checkDigits(id) == false)
            {
                //写error 是否要直接return
                getAccountError(response, root, id, "account id has existed");
            }
            else if (balance <= 0)
            {
                getAccountError(response, root, id, "balance is not positive");
            }
            else if (checkDigits(id) == false)
            {
                getAccountError(response, root, id, "account id's format is wrong");
            }
            else
            { //创建一个account
                Account::addAccount(C, id, balance);
                XMLElement *usernode = response->NewElement("created");
                usernode->SetAttribute("id", id.c_str());
                root->InsertEndChild(usernode);
            }
        }
        else if (strncmp(currElem->Name(), "symbol", 6) == 0)
        {
            // to do
            string sym = currElem->FirstAttribute()->Value();
            XMLElement *currAccount = currElem->FirstChildElement();
            while (currAccount != NULL)
            {
                string id = currAccount->FirstAttribute()->Value();
                int amount = stoi(currAccount->GetText()); // Q 检查text是否是数字
                currAccount = currAccount->NextSiblingElement();
                if (!checkAlpha(sym))
                {
                    getSymbolError(response, root, sym, id, "sym format is wrong");
                }
                else if (amount < 0)
                {
                    getSymbolError(response, root, sym, id, "amount should be positive");
                }
                else if (Account::idExists(C, id) == false)
                {
                    getSymbolError(response, root, sym, id, "account id doesn't exist");
                }
                else if (checkDigits(id) == false)
                {
                    getSymbolError(response, root, sym, id, "id format is wrong");
                }
                else
                {
                    Position::addPosition(C, sym, id, amount);
                    XMLElement *usernode = response->NewElement("created");
                    usernode->SetAttribute("sym", sym.c_str());
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
    response->Print(&printer);   //将Print打印到Xmlprint类中 即保存在内存中
    string buf = printer.CStr(); //转换成const char*类型
    cout << buf << endl;         // buf即为创建后的XML 字符串。
    return response;
}
void getTransError(XMLDocument *response, XMLElement *root, string sym, string amount, string limit, string msg)
{
    XMLElement *usernode = response->NewElement("error");
    usernode->SetAttribute("sym", sym.c_str());
    usernode->SetAttribute("amount", amount.c_str());
    usernode->SetAttribute("limit", limit.c_str());
    usernode->InsertFirstChild(response->NewText(msg.c_str()));
    root->InsertEndChild(usernode);
}

XMLDocument *handleTranscation(connection *C, string &request)
{
    XMLDocument *response = new XMLDocument();

    XMLDocument *xml = new XMLDocument(); // Q
    xml->Parse(request.c_str());

    XMLElement *root = response->NewElement("results");
    response->InsertEndChild(root);

    XMLElement *xml_root = xml->RootElement();
    string id = xml_root->FindAttribute("id")->Value();
    if (Account::idExists(C, id) == false || !checkDigits(id))
    {
        getAccountError(response, root, id, "account id doesn't exist");
        response->SaveFile("test1.xml");
        // XML 写到字符串
        XMLPrinter printer;
        response->Print(&printer);   //将Print打印到Xmlprint类中 即保存在内存中
        string buf = printer.CStr(); //转换成const char*类型
        cout << buf << endl;         // buf即为创建后的XML 字符串。
        return response;
    }
    else if (!checkDigits(id))
    {
        getAccountError(response, root, id, "id format is wrong");
        response->SaveFile("test1.xml");
        // XML 写到字符串
        XMLPrinter printer;
        response->Print(&printer);   //将Print打印到Xmlprint类中 即保存在内存中
        string buf = printer.CStr(); //转换成const char*类型
        cout << buf << endl;         // buf即为创建后的XML 字符串。
        return response;
    }
    XMLElement *currElem = xml_root->FirstChildElement();

    while (currElem != NULL)
    {
        if (strncmp(currElem->Name(), "order", 5) == 0)
        {
            string sym = currElem->FirstAttribute()->Value();
            string amount = currElem->FindAttribute("amount")->Value();
            string limit = currElem->FindAttribute("limit")->Value();
            if (!checkAlpha(sym))
            {
                getTransError(response, root, sym, amount, limit, "symblo format is wrong");
            }
            else if (stoi(amount) == 0)
            {
                getTransError(response, root, sym, amount, limit, "amount should be non-zero");
            }
            else if (stod(limit) < 0)
            {
                getTransError(response, root, sym, amount, limit, "limit should be positive");
            }
            else
            {
                if (stoi(amount) > 0)
                { //检查账户余额是否充足，并更新账户余额 买
                    double requiredBalance = stoi(amount) * stod(limit);
                    if (!Account::enoughBalance(C, id, requiredBalance))
                    {
                        getTransError(response, root, sym, amount, limit, "account balance is not enough");
                        currElem = currElem->NextSiblingElement();
                        continue;
                    }
                }
                else if (stoi(amount) < 0)
                { // amount<0 means sell 检查symbol的amount是否足够并更新
                    if (!Position::updateSymbolAmount(C, sym, id, 0 - stoi(amount)))
                    {
                        getTransError(response, root, sym, amount, limit, "symbol amount is not enough");
                        currElem = currElem->NextSiblingElement();
                        continue;
                    }
                }
                // now creat a transaction
                int id_trans = Transaction::addTransaction(C, id, sym, stod(limit), stoi(amount));
                // Match one possible at a time ??
                while (Transaction::matchOrder(C, id_trans))
                {
                }
                XMLElement *usernode = response->NewElement("opened");
                usernode->SetAttribute("sym", sym.c_str());
                usernode->SetAttribute("amount", amount.c_str());
                usernode->SetAttribute("limit", limit.c_str());
                usernode->SetAttribute("id", id_trans);
                root->InsertEndChild(usernode);
            }
        }
        else if (strncmp(currElem->Name(), "cancel", 6) == 0)
        {
            string trans_id = currElem->FindAttribute("id")->Value();
            if (!checkDigits(trans_id))
            {
                getAccountError(response, root, id, "transcation id format is wrong");
            }
            else if (!Transaction::isTransExists(C, stoi(trans_id)))
            {
                getAccountError(response, root, id, "transcation id does not exist");
            }
            else
            {
                XMLElement *usernode = response->NewElement("canceled");
                usernode->SetAttribute("id", trans_id.c_str());
                Transaction::handleCancel(C, stoi(trans_id), response, usernode); // Q内存问题 对response进行修改
                root->InsertEndChild(usernode);
            }
        }
        else if (strcmp(currElem->Name(), "query") == 0)
        {

            string trans_id = currElem->FindAttribute("id")->Value();
            if (!checkDigits(trans_id))
            {
                getAccountError(response, root, id, "transcation id format is wrong");
            }
            else if (!Transaction::isTransExists(C, stoi(trans_id)))
            {
                getAccountError(response, root, id, "transcation id does not exist");
            }
            else
            {
                XMLElement *usernode = response->NewElement("status");
                usernode->SetAttribute("id", trans_id.c_str());
                Transaction::handleQuery(C, stoi(trans_id), response, usernode);
                root->InsertEndChild(usernode);
            }
        }
        currElem = currElem->NextSiblingElement();
    }
    response->SaveFile("test2.xml");
    XMLPrinter printer;
    response->Print(&printer);   //将Print打印到Xmlprint类中 即保存在内存中
    string buf = printer.CStr(); //转换成const char*类型
    cout << buf << endl;         // buf即为创建后的XML 字符串。
    return response;
}
