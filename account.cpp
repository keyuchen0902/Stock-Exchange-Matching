#include "account.h"

#include <fstream>
#include <sstream>
#include <string>

using namespace std;

/* Add a new entry to the table, return true if succeed */
bool Account::addAccount(connection * C, string account_id, double balance) {
  /* Create a transactional object. */
  work W(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "Insert INTO ACCOUNT (ACCOUNT_ID, BALANCE) ";
  sql << "VALUES (";
  sql << W.quote(account_id) << ", ";
  sql << W.quote(balance) << ");";

  /* Execute SQL query */
  try {
    W.exec(sql.str());
    W.commit();
  }
  catch (const std::exception & e) {
    W.abort();
    return false;
  }

  return true;
}

/* Check if the given account already exists */
bool Account::idExists(connection * C,string account_id) {
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

bool Account::enoughBalance(connection *C,string account_id, double requiredBalance){
    work W(*C);

    std::stringstream sql1;
    sql1 << "SELECT BALANCE FROM ACCOUNT WHERE ACCOUNT_ID=";
    sql1 << W.quote(account_id) << " FOR UPDATE;";

    result R(W.exec(sql1.str()));

    double amount = R[0]["BALANCE"].as<double>();
    double remain = amount - requiredBalance;
    if (remain < 0) {
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