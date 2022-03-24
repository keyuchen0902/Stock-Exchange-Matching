#include "position.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
/* Add a new entry to the table */
void Position::addPosition(connection * C,
                           const string & symbol_name,
                           string account_id,
                           int num_share) {
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

bool Position::updateSymbolAmount(connection * C,string symbol_name,string account_id,int amount){
    work W(*C);

    // Read
    std::stringstream sql1;
    sql1 << "SELECT NUM_SHARE FROM POSITION WHERE ACCOUNT_ID = ";
    sql1 << W.quote(account_id) << " ";
    sql1 << "AND SYMBOL_NAME = " << W.quote(symbol_name) << " FOR UPDATE;";

    result R(W.exec(sql1.str()));
    if (R.size() == 0) {
      return false;
    }

    // Modify
    int num_share = R[0]["NUM_SHARE"].as<int>();
    int remain = num_share - amount;

    if (remain < 0) {
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