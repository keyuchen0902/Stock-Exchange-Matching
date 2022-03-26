#include "transcation.h"

#include <fstream>
#include <sstream>
#include <string>

using namespace std;

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
              node3->SetAttribute("price", c["TIME"].as<long>());
              usernode->InsertEndChild(node3);
       }

       W.commit();
}

/* Get the limited price of the given trans_id */
double Transaction::getLimited(connection *C, int trans_id)
{
       /* Create a non-transactional object. */
       nontransaction N(*C);

       /* Create SQL statement */
       std::stringstream sql;
       sql << "SELECT LIMITED FROM TRANSACTION WHERE TRANSACTION_ID=";
       sql << N.quote(trans_id) << ";";

       /* Execute SQL query */
       result R(N.exec(sql.str()));

       return R[0][0].as<double>();
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
       bool flag = true;
       if (num_open == 0)
       {
              W1.commit();
              XMLElement *node1 = response->NewElement("error");
              string msg = "the num of open shsare is 0. Cannot be canceled";
              node1->InsertFirstChild(response->NewText(msg.c_str()));
              usernode->InsertEndChild(node1);
              flag = false;
       }
       else
       {
              /* Create SQL statement */
              std::stringstream sql2;
              sql2 << "UPDATE TRANSACTION SET ";
              sql2 << "NUM_CANCEL = TRANSACTION.NUM_OPEN, ";
              sql2 << "NUM_OPEN = 0, ";
              sql2 << "CANCEL_TIME = " << W1.quote(getCurrTime()) << ";";

              W1.exec(sql2);
              W1.commit();

              /* Create a non-transactional object. */
              nontransaction N1(*C); // Q需不需要每次都创建
              /* Create SQL statement */
              std::stringstream sql;
              sql << "SELECT ACCOUNT_ID FROM TRANSACTION WHERE TRANSACTION_ID=";
              sql << N1.quote(trans_id) << ";";

              /* Execute SQL query */
              result R1(N1.exec(sql.str()));
              string account_id = R1[0][0].as<string>();

              work W2(*C);
              if (num_open > 0)
              { // buyer,need to return money
                     double limiited = Transaction::getLimited(C, trans_id);
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
                     nontransaction N2(*C);
                     std::stringstream sql5;
                     sql5 << "SELECT SYMBOL_NAME FROM TRANSACTION WHERE TRANSACTION_ID=";
                     sql5 << N2.quote(trans_id) << ";";
                     result R2(N2.exec(sql5.str()));
                     string symbol_name = R2[0][0].as<string>();
                     std::stringstream sql6;
                     sql6 << "UPDATE POSITION SET ";
                     sql6 << "NUM_SHARE = POSITION.NUM_SHARE - " << W2.quote(num_open) << " ";
                     sql6 << "WHERE ACCOUNT_ID = ";
                     sql6 << W2.quote(account_id) << " ";
                     sql6 << "AND SYMBOL_NAME = " << W2.quote(symbol_name) << ";";

                     /* Execute SQL query */
                     W2.exec(sql6.str());
                     W2.commit();
              }
       }
       if (flag)
       {
              nontransaction N3(*C);

              /* Create SQL statement */
              std::stringstream sql7;
              sql7 << "SELECT NUM_CANCELED FROM TRANSACTION WHERE TRANSACTION_ID=";
              sql7 << N3.quote(trans_id) << ";";
              /* Execute SQL query */
              result R3(N3.exec(sql7.str()));
              int canceledShares = R3[0][0].as<int>();
              long canceledTime = Transaction::getCanceledTime(C, trans_id);
              XMLElement *node2 = response->NewElement("canceled");
              node2->SetAttribute("shares", canceledShares);
              node2->SetAttribute("time", canceledTime);
              usernode->InsertEndChild(node2);

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
                            node2->SetAttribute("shares", c[2].as<int>());
                     }
                     else
                     {
                            node2->SetAttribute("shares", 0 - c[2].as<int>());
                     }
                     node2->SetAttribute("price", c[3].as<double>());
                     node2->SetAttribute("time", c[4].as<long>());
                     usernode->InsertEndChild(node2);
              }
       }
}