#include "transcation.h"

#include <fstream>
#include <sstream>
#include <string>

using namespace std;

/* Add a new entry to the table, return the transaction_id */
int Transaction::addTransaction(connection * C,
                                const string & account_id,
                                const string & symbol_name,
                                double limited,
                                int num_open) {
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

bool Transaction::isTransExists(connection * C, int trans_id) {
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

void Transaction::handleQuery(connection *C,int trans_id,XMLDocument* response,XMLElement* usernode){
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


       if(openShares != 0){
              XMLElement* node1 = response->NewElement("open");
              node1->SetAttribute("shares",openShares);
              usernode->InsertEndChild(node1);
       }
       if (canceledShares != 0) {
              XMLElement* node2 = response->NewElement("canceled");
              node2->SetAttribute("shares",openShares);
              node2->SetAttribute("time",canceledTime);
              usernode->InsertEndChild(node2);      
       }
       // Get response for execution
       std::stringstream sql3;
       sql3 << "SELECT BUYER_TRANS_ID, SELLER_TRANS_ID, AMOUNT, PRICE, TIME FROM EXECUTION WHERE ";
       sql3 << "BUYER_TRANS_ID = " << W.quote(trans_id) << " ";
       sql3 << "OR SELLER_TRANS_ID = " << W.quote(trans_id) << ";";

       /* Execute SQL query */
       result R2(W.exec(sql3.str()));
       for (result::const_iterator c = R2.begin(); c != R2.end(); ++c) {
              XMLElement* node3 = response->NewElement("executed");
              if (trans_id == c["BUYER_TRANS_ID"].as<int>()) {
                     node3->SetAttribute("shares",c["AMOUNT"].as<int>());
              }
              else {
                     node3->SetAttribute("shares",0-c["AMOUNT"].as<int>());
                     
              }
              node3->SetAttribute("price",c["PRICE"].as<double>());
              node3->SetAttribute("price",c["TIME"].as<long>());
              usernode->InsertEndChild(node3);
       }

       W.commit();
}