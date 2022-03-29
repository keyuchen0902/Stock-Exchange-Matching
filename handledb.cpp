#include "handledb.h"
#include <chrono>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

long getCurrTime()
{
    stringstream ss;
    ss << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::_V2::system_clock::now().time_since_epoch()).count();
    long time;
    ss >> time;
    return time;
}
/* Add a new entry to the table, return true if succeed */
bool Account::addAccount(connection *C, string &account_id, double balance)
{
    /* Create a transactional object. */
    work W(*C);

    /* Create SQL statement */
    std::stringstream sql;
    sql << "INSERT INTO ACCOUNT (ACCOUNT_ID, BALANCE) ";
    sql << "VALUES (";
    sql << W.quote(account_id) << ", ";
    sql << W.quote(balance) << ");";

    /* Execute SQL query */
    try
    {
        W.exec(sql.str());
        W.commit();
    }
    catch (const std::exception &e)
    {
        W.abort();
        return false;
    }

    return true;
}

/* Check if the given account already exists */
bool Account::idExists(connection *C, string &account_id)
{
    /* Create a non-transactional object. */
    nontransaction N(*C);

    /* Create SQL statement */
    std::stringstream sql;
    sql << "SELECT * FROM ACCOUNT WHERE ACCOUNT_ID=";
    sql << N.quote(account_id) << ";";

    /* Execute SQL query */
    result R(N.exec(sql.str()));

    return R.size() != 0;
}

bool Account::enoughBalance(connection *C, string &account_id, double requiredBalance)
{
    work W(*C);

    std::stringstream sql1;
    sql1 << "SELECT BALANCE FROM ACCOUNT WHERE ACCOUNT_ID=";
    sql1 << W.quote(account_id) << " FOR UPDATE;";

    result R(W.exec(sql1.str()));

    double amount = R[0]["BALANCE"].as<double>();
    double remain = amount - requiredBalance;
    if (remain < 0)
    {
        W.commit();
        return false;
    }

    std::stringstream sql2;
    sql2 << "UPDATE ACCOUNT SET BALANCE = ";
    sql2 << W.quote(remain) << " ";
    sql2 << "WHERE ACCOUNT_ID=";
    sql2 << W.quote(account_id) << ";";

    /* Execute SQL query */
    W.exec(sql2.str());
    W.commit();

    return true;
}

/* Add a new entry to the table */
void Execution::addExecution(work &W,
                             int buyer_trans_id,
                             int seller_trans_id,
                             int amount,
                             double price)
{
    /* Create SQL statement */
    std::stringstream sql;
    sql << "Insert INTO EXECUTION (EXECUTION_ID, BUYER_TRANS_ID, SELLER_TRANS_ID, ";
    sql << "AMOUNT, PRICE, TIME) ";
    sql << "VALUES (DEFAULT, ";
    sql << W.quote(buyer_trans_id) << ", ";
    sql << W.quote(seller_trans_id) << ", ";
    sql << W.quote(amount) << ", ";
    sql << W.quote(price) << ", ";
    sql << W.quote(getCurrTime()) << ");";

    W.exec(sql.str());
}

void Position::addPosition(connection *C,
                            string &symbol_name,
                           string &account_id,
                           int num_share)
{
    /* Create a transactional object. */
    work W(*C);

    /* Create SQL statement */

    std::stringstream sql;
    sql << "Insert INTO POSITION (SYMBOL_NAME, ACCOUNT_ID, NUM_SHARE) ";
    sql << "VALUES (";
    sql << W.quote(symbol_name) << ", ";
    sql << W.quote(account_id) << ", ";
    sql << W.quote(num_share) << ") ";
    sql << "ON CONFLICT ON CONSTRAINT POSITION_PKEY ";
    sql << "DO UPDATE SET NUM_SHARE = POSITION.NUM_SHARE + ";
    sql << W.quote(num_share) << ";";

    W.exec(sql.str());
    W.commit();
}

bool Position::updateSymbolAmount(connection *C, string &symbol_name, string &account_id, int amount)
{
    work W(*C);

    // Read
    std::stringstream sql1;
    sql1 << "SELECT NUM_SHARE FROM POSITION WHERE ACCOUNT_ID = ";
    sql1 << W.quote(account_id) << " ";
    sql1 << "AND SYMBOL_NAME = " << W.quote(symbol_name) << " FOR UPDATE;";

    result R(W.exec(sql1.str()));
    if (R.size() == 0)
    {
        return false;
    }

    // Modify
    int num_share = R[0]["NUM_SHARE"].as<int>();
    int remain = num_share - amount;

    if (remain < 0)
    {
        W.commit();
        return false;
    }

    // Write
    std::stringstream sql2;
    sql2 << "UPDATE POSITION SET NUM_SHARE = ";
    sql2 << W.quote(remain) << " ";
    sql2 << "WHERE ACCOUNT_ID = ";
    sql2 << W.quote(account_id) << " ";
    sql2 << "AND SYMBOL_NAME = " << W.quote(symbol_name) << ";";

    W.exec(sql2.str());
    W.commit();

    return true;
}

/* Add a new entry to the table, return the transaction_id */
int Transaction::addTransaction(connection *C,
                                const string &account_id,
                                const string &symbol_name,
                                double limited,
                                int num_open)
{
    /* Create a transactional object. */
    work W(*C);

    /* Create SQL statement */
    std::stringstream sql;
    sql << "Insert INTO TRANSACTION (TRANSACTION_ID, ACCOUNT_ID, SYMBOL_NAME, "
           "LIMITED, NUM_OPEN, NUM_CANCELED, OPEN_TIME, CANCEL_TIME) ";
    sql << "VALUES (DEFAULT, ";
    sql << W.quote(account_id) << ", ";
    sql << W.quote(symbol_name) << ", ";
    sql << W.quote(limited) << ", ";
    sql << W.quote(num_open) << ", ";
    sql << W.quote(0) << ", ";
    sql << W.quote(getCurrTime()) << ", DEFAULT) RETURNING TRANSACTION_ID;";

    result R(W.exec(sql.str()));
    W.commit();

    return R[0][0].as<int>();
}

bool Transaction::isTransExists(connection *C, int trans_id)
{
    /* Create a non-transactional object. */
    nontransaction N(*C);

    /* Create SQL statement */
    std::stringstream sql;
    sql << "SELECT * FROM TRANSACTION WHERE TRANSACTION_ID=";
    sql << N.quote(trans_id) << ";";

    /* Execute SQL query */
    result R(N.exec(sql.str()));

    return R.size() != 0;
}

/* Get the cancel_time of the given trans_id */
long Transaction::getCanceledTime(connection *C, int trans_id)
{
    /* Create a non-transactional object. */
    nontransaction N(*C);

    /* Create SQL statement */
    std::stringstream sql;
    sql << "SELECT CANCEL_TIME FROM TRANSACTION WHERE TRANSACTION_ID=";
    sql << N.quote(trans_id) << ";";

    /* Execute SQL query */
    result R(N.exec(sql.str()));

    return R[0][0].as<long>();
}

void Transaction::handleQuery(connection *C, int trans_id, XMLDocument *response, XMLElement *usernode)
{
    work W(*C);

    // Lock the transaction and execution table
    std::stringstream sql1;
    sql1 << "SELECT * FROM TRANSACTION, EXECUTION ";
    sql1 << "WHERE TRANSACTION.TRANSACTION_ID = " << W.quote(trans_id);
    sql1 << " AND (EXECUTION.BUYER_TRANS_ID = " << W.quote(trans_id);
    sql1 << " OR EXECUTION.SELLER_TRANS_ID = " << W.quote(trans_id);
    sql1 << ") FOR UPDATE;";
    W.exec(sql1.str());

    // Get open and canceled shares
    std::stringstream sql2;
    sql2 << "SELECT NUM_OPEN, NUM_CANCELED, CANCEL_TIME FROM TRANSACTION WHERE TRANSACTION_ID=";
    sql2 << W.quote(trans_id) << ";";

    result R1(W.exec(sql2.str()));

    int openShares = R1[0]["NUM_OPEN"].as<int>();
    int canceledShares = R1[0]["NUM_CANCELED"].as<int>();
    long canceledTime = R1[0]["CANCEL_TIME"].as<long>();

    if (openShares != 0)
    {
        XMLElement *node1 = response->NewElement("open");
        node1->SetAttribute("shares", openShares);
        usernode->InsertEndChild(node1);
    }
    if (canceledShares != 0)
    {
        XMLElement *node2 = response->NewElement("canceled");
        node2->SetAttribute("shares", openShares);
        node2->SetAttribute("time", canceledTime);
        usernode->InsertEndChild(node2);
    }
    // Get response for execution
    std::stringstream sql3;
    sql3 << "SELECT BUYER_TRANS_ID, SELLER_TRANS_ID, AMOUNT, PRICE, TIME FROM EXECUTION WHERE ";
    sql3 << "BUYER_TRANS_ID = " << W.quote(trans_id) << " ";
    sql3 << "OR SELLER_TRANS_ID = " << W.quote(trans_id) << ";";

    /* Execute SQL query */
    result R2(W.exec(sql3.str()));
    for (result::const_iterator c = R2.begin(); c != R2.end(); ++c)
    {
        XMLElement *node3 = response->NewElement("executed");
        if (trans_id == c["BUYER_TRANS_ID"].as<int>())
        {
            node3->SetAttribute("shares", c["AMOUNT"].as<int>());
        }
        else
        {
            node3->SetAttribute("shares", 0 - c["AMOUNT"].as<int>());
        }
        node3->SetAttribute("price", c["PRICE"].as<double>());
        node3->SetAttribute("time", c["TIME"].as<long>());
        usernode->InsertEndChild(node3);
    }

    W.commit();
}

/* Get the limited price of the given trans_id */
double Transaction::getLimited(connection *C, int trans_id)
{
    nontransaction N(*C);
    std::stringstream sql;
    sql << "SELECT LIMITED FROM TRANSACTION WHERE TRANSACTION_ID=";
    sql << N.quote(trans_id) << ";";
    result R(N.exec(sql.str()));

    return R[0][0].as<double>();
}

string getAccountID(connection *C, int trans_id)
{
    nontransaction N1(*C); // Q需不需要每次都创建
    /* Create SQL statement */
    std::stringstream sql;
    sql << "SELECT ACCOUNT_ID FROM TRANSACTION WHERE TRANSACTION_ID=";
    sql << N1.quote(trans_id) << ";";

    /* Execute SQL query */
    result R1(N1.exec(sql.str()));
    string account_id = R1[0][0].as<string>();
    N1.commit();
    return account_id;
}

string getSymbolName(connection *C, int trans_id)
{
    nontransaction N2(*C);
    std::stringstream sql5;
    sql5 << "SELECT SYMBOL_NAME FROM TRANSACTION WHERE TRANSACTION_ID=";
    sql5 << N2.quote(trans_id) << ";";
    result R2(N2.exec(sql5.str()));
    string symbol_name = R2[0][0].as<string>();
    return symbol_name;
}

int getCanceledNum(connection *C, int trans_id)
{
    nontransaction N3(*C);

    /* Create SQL statement */
    std::stringstream sql7;
    sql7 << "SELECT NUM_CANCELED FROM TRANSACTION WHERE TRANSACTION_ID=";
    sql7 << N3.quote(trans_id) << ";";
    /* Execute SQL query */
    result R3(N3.exec(sql7.str()));
    int canceledShares = R3[0][0].as<int>();
    return canceledShares;
}

void creatExecutedPrinted(connection *C, int trans_id, XMLDocument *response, XMLElement *usernode)
{
    nontransaction N4(*C);
    /* Create SQL statement */
    std::stringstream sql8;
    sql8 << "SELECT BUYER_TRANS_ID, SELLER_TRANS_ID, AMOUNT, PRICE, TIME FROM EXECUTION WHERE "
            "BUYER_TRANS_ID=";
    sql8 << N4.quote(trans_id) << " ";
    sql8 << "OR SELLER_TRANS_ID=" << N4.quote(trans_id) << ";";

    /* Execute SQL query */
    result R4(N4.exec(sql8.str()));
    for (result::const_iterator c = R4.begin(); c != R4.end(); ++c)
    {
        XMLElement *node3 = response->NewElement("executed");
        if (trans_id == c[0].as<int>())
        {
            node3->SetAttribute("shares", c[2].as<int>());
        }
        else
        {
            node3->SetAttribute("shares", 0 - c[2].as<int>());
        }
        node3->SetAttribute("price", c[3].as<double>());
        node3->SetAttribute("time", c[4].as<long>());
        usernode->InsertEndChild(node3);
    }
}
void Transaction::handleCancel(connection *C, int trans_id, XMLDocument *response, XMLElement *usernode)
{
    /* Create a transactional object. */
    work W1(*C);

    /* Create SQL statement */
    std::stringstream sql1;
    sql1 << "SELECT NUM_OPEN FROM TRANSACTION WHERE TRANSACTION_ID=";
    sql1 << W1.quote(trans_id) << " FOR UPDATE;";

    /* Execute SQL query */
    result R(W1.exec(sql1.str()));

    int num_open = R[0]["NUM_OPEN"].as<int>();
    // bool flag = true;
    if (num_open == 0)
    {
        W1.commit();
        XMLElement *node1 = response->NewElement("error");
        string msg = "the num of open share is 0. Cannot be canceled";
        node1->InsertFirstChild(response->NewText(msg.c_str()));
        usernode->InsertEndChild(node1);
        return;
    }
    cout << "enter here1" << endl;

    /* Create SQL statement */
    std::stringstream sql2;
    sql2 << "UPDATE TRANSACTION SET ";
    sql2 << "NUM_CANCELED = TRANSACTION.NUM_OPEN, ";
    sql2 << "NUM_OPEN = 0, ";
    sql2 << "CANCEL_TIME = " << W1.quote(getCurrTime()) << ";";

    W1.exec(sql2);
    W1.commit();

    cout << "enter here2" << endl;
    /* Get the account_id of the given trans_id */
    string account_id = getAccountID(C, trans_id);
    cout << "enter here3" << endl;
    if (num_open > 0)
    { // buyer,need to return money
        cout<<"enter here4"<<endl;
        double limiited = Transaction::getLimited(C, trans_id);
        cout<<"enter here5"<<endl;
        work W2(*C);
        std::stringstream sql3;
        sql3 << "SELECT BALANCE FROM ACCOUNT WHERE ACCOUNT_ID = ";
        sql3 << W2.quote(account_id) << " FOR UPDATE;";

        result R2(W2.exec(sql3.str()));
        double balance = R2[0]["BALANCE"].as<double>();

        balance = balance + num_open * limiited;

        std::stringstream sql4;
        sql4 << "UPDATE ACCOUNT SET BALANCE = ";
        sql4 << W2.quote(balance) << " ";
        sql4 << "WHERE ACCOUNT_ID = ";
        sql4 << W2.quote(account_id) << ";";

        W2.exec(sql4);
        W2.commit();
   
    }
    else
    { // num <0,sell, return symbol
        string symbol_name = getSymbolName(C, trans_id);
        std::stringstream sql6;
        work W2(*C);
        sql6 << "UPDATE POSITION SET ";
        sql6 << "NUM_SHARE = POSITION.NUM_SHARE - " << W2.quote(num_open) << " ";
        sql6 << "WHERE ACCOUNT_ID = ";
        sql6 << W2.quote(account_id) << " ";
        sql6 << "AND SYMBOL_NAME = " << W2.quote(symbol_name) << ";";

        /* Execute SQL query */
        W2.exec(sql6.str());
        W2.commit();
    }
    int canceledShares = getCanceledNum(C, trans_id);
    long canceledTime = Transaction::getCanceledTime(C, trans_id);
    XMLElement *node2 = response->NewElement("canceled");
    node2->SetAttribute("shares", canceledShares);
    node2->SetAttribute("time", canceledTime);
    usernode->InsertEndChild(node2);

    creatExecutedPrinted(C, trans_id, response, usernode);
}

bool Transaction::matchOrder(connection *C, int trans_id)
{
    work W(*C);

    // Lock all rows related to current transaction's owner
    std::stringstream ownerSql;
    ownerSql << "SELECT * FROM ACCOUNT, POSITION, TRANSACTION WHERE ";
    ownerSql << "TRANSACTION.TRANSACTION_ID = " << W.quote(trans_id) << " ";
    ownerSql << "AND ACCOUNT.ACCOUNT_ID = TRANSACTION.ACCOUNT_ID ";
    ownerSql << "AND POSITION.ACCOUNT_ID = TRANSACTION.ACCOUNT_ID ";
    ownerSql << "AND POSITION.SYMBOL_NAME = TRANSACTION.SYMBOL_NAME ";
    ownerSql << "FOR UPDATE;"; // lock

    W.exec(ownerSql.str());
    // get info of trans_id
    std::stringstream getinfo;
    getinfo << "SELECT * FROM TRANSACTION WHERE TRANSACTION_ID = ";
    getinfo << W.quote(trans_id) << ";";

    result R1(W.exec(getinfo.str()));

    string account_id = R1[0]["ACCOUNT_ID"].as<string>();
    string symbol_name = R1[0]["SYMBOL_NAME"].as<string>();
    double limited = R1[0]["LIMITED"].as<double>();
    int num_open = R1[0]["NUM_OPEN"].as<int>();

    // Get the first matching transactions,使用lock
    std::stringstream getTrans;
    getTrans << "SELECT * FROM TRANSACTION WHERE ACCOUNT_ID != ";
    getTrans << W.quote(account_id) << " ";
    getTrans << "AND SYMBOL_NAME = " << W.quote(symbol_name) << " ";

    int final_amount;
    double final_price;

    if (num_open > 0) //找seller
    {
        cout << "finding seller..." << endl;
        getTrans << "AND LIMITED <= " << W.quote(limited) << " ";
        getTrans << "AND NUM_OPEN < 0 ";
        getTrans << "ORDER BY LIMITED ASC, OPEN_TIME ASC LIMIT 1 ";
        getTrans << "FOR UPDATE;";
    }
    else
    {
        cout << "finding buyer" << endl;
        getTrans << "AND LIMITED >= " << W.quote(limited) << " ";
        getTrans << "AND NUM_OPEN > 0 ";
        getTrans << "ORDER BY LIMITED DESC, OPEN_TIME ASC LIMIT 1 ";
        getTrans << "FOR UPDATE;";
    }

    /* Execute SQL query */
    result R2(W.exec(getTrans.str()));
    cout << "the size of result = " << R2.size() << endl;
    if (R2.size() == 0)
    {
        return false;
    }

    int other_trans_id = R2[0]["TRANSACTION_ID"].as<int>();
    string other_account_id = R2[0]["ACCOUNT_ID"].as<string>();
    double other_limited = R2[0]["LIMITED"].as<double>();
    int other_num_open = R2[0]["NUM_OPEN"].as<int>();

    final_price = other_limited; // Q execution price?

    if (num_open > 0)
    {
        if (num_open <= 0 - other_num_open)
        {
            final_amount = num_open;
            std::stringstream sql;
            sql << "UPDATE TRANSACTION SET NUM_OPEN = ";
            sql << W.quote(0) << " ";
            sql << "WHERE TRANSACTION_ID = ";
            sql << W.quote(trans_id) << ";";
            /* Execute SQL query */
            W.exec(sql.str());
            std::stringstream sql2;
            sql2 << "UPDATE TRANSACTION SET NUM_OPEN = ";
            sql2 << W.quote(other_num_open + num_open) << " ";
            sql2 << "WHERE TRANSACTION_ID = ";
            sql2 << W.quote(other_trans_id) << ";";
            /* Execute SQL query */
            W.exec(sql2.str());
        }
        else
        {
            final_amount = 0 - other_num_open;
            std::stringstream sql;
            sql << "UPDATE TRANSACTION SET NUM_OPEN = ";
            sql << W.quote(num_open + other_num_open) << " ";
            sql << "WHERE TRANSACTION_ID = ";
            sql << W.quote(trans_id) << ";";
            /* Execute SQL query */
            W.exec(sql.str());
            std::stringstream sql2;
            sql2 << "UPDATE TRANSACTION SET NUM_OPEN = ";
            sql2 << W.quote(0) << " ";
            sql2 << "WHERE TRANSACTION_ID = ";
            sql2 << W.quote(other_trans_id) << ";";
            /* Execute SQL query */
            W.exec(sql2.str());
        }
        // Return money to buyer
        std::stringstream sql;
        sql << "UPDATE ACCOUNT SET BALANCE = ACCOUNT.BALANCE + ";
        sql << W.quote(final_amount * (limited - other_limited)) << " ";
        sql << "WHERE ACCOUNT_ID = ";
        sql << W.quote(account_id) << ";";
        /* Execute SQL query */
        W.exec(sql.str());

        // give money to seller
        std::stringstream sql2;
        sql2 << "UPDATE ACCOUNT SET BALANCE = ACCOUNT.BALANCE + ";
        sql2 << W.quote(final_amount * final_price) << " ";
        sql2 << "WHERE ACCOUNT_ID = ";
        sql2 << W.quote(other_account_id) << ";";
        W.exec(sql2.str());

        // give symbol to buyer ???实现未成功
        std::stringstream sql3;
        sql3 << "Insert INTO POSITION (SYMBOL_NAME, ACCOUNT_ID, NUM_SHARE) ";
        sql3 << "VALUES (";
        sql3 << W.quote(symbol_name) << ", ";
        sql3 << W.quote(account_id) << ", ";
        sql3 << W.quote(final_amount) << ") ";
        sql3 << "ON CONFLICT ON CONSTRAINT POSITION_PKEY ";
        sql3 << "DO UPDATE SET NUM_SHARE = POSITION.NUM_SHARE + ";
        sql3 << W.quote(final_amount) << ";";

        W.exec(sql3.str());
        // Create execution
        Execution::addExecution(W, trans_id, other_trans_id, final_amount, final_price);
    }
    else
    {
        if (0 - num_open <= other_num_open)
        {
            final_amount = -num_open;
            std::stringstream sql;
            sql << "UPDATE TRANSACTION SET NUM_OPEN = ";
            sql << W.quote(0) << " ";
            sql << "WHERE TRANSACTION_ID = ";
            sql << W.quote(trans_id) << ";";
            W.exec(sql.str());

            std::stringstream sql2;
            sql2 << "UPDATE TRANSACTION SET NUM_OPEN = ";
            sql2 << W.quote(other_num_open + num_open) << " ";
            sql2 << "WHERE TRANSACTION_ID = ";
            sql2 << W.quote(other_trans_id) << ";";
            W.exec(sql2.str());
        }
        else
        {
            final_amount = other_num_open;
            std::stringstream sql;
            sql << "UPDATE TRANSACTION SET NUM_OPEN = ";
            sql << W.quote(num_open + other_num_open) << " ";
            sql << "WHERE TRANSACTION_ID = ";
            sql << W.quote(trans_id) << ";";
            W.exec(sql.str());

            std::stringstream sql2;
            sql2 << "UPDATE TRANSACTION SET NUM_OPEN = ";
            sql2 << W.quote(0) << " ";
            sql2 << "WHERE TRANSACTION_ID = ";
            sql2 << W.quote(other_trans_id) << ";";
            W.exec(sql2.str());
        }
        // give money to seller
        std::stringstream sql2;
        sql2 << "UPDATE ACCOUNT SET BALANCE = ACCOUNT.BALANCE + ";
        sql2 << W.quote(final_amount * final_price) << " ";
        sql2 << "WHERE ACCOUNT_ID = ";
        sql2 << W.quote(other_account_id) << ";";
        W.exec(sql2.str());

        // give symbol to buyer
        std::stringstream sql3;
        sql3 << "Insert INTO POSITION (SYMBOL_NAME, ACCOUNT_ID, NUM_SHARE) ";
        sql3 << "VALUES (";
        sql3 << W.quote(symbol_name) << ", ";
        sql3 << W.quote(other_account_id) << ", ";
        sql3 << W.quote(final_amount) << ") ";
        sql3 << "ON CONFLICT ON CONSTRAINT POSITION_PKEY ";
        sql3 << "DO UPDATE SET NUM_SHARE = POSITION.NUM_SHARE + ";
        sql3 << W.quote(final_amount) << ";";

        Execution::addExecution(W, other_trans_id, trans_id, final_amount, final_price);
    }
    std::stringstream openshare_sql;
    openshare_sql << "SELECT NUM_OPEN FROM TRANSACTION WHERE TRANSACTION_ID=";
    openshare_sql << W.quote(trans_id) << ";";

    /* Execute SQL query */
    result R(W.exec(openshare_sql.str()));

    W.commit();
    if (R[0][0].as<int>() == 0)
    { // open_num now equals 0
        return false;
    }
    return true;
}