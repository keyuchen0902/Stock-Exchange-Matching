#include "account.h"

#include <fstream>
#include <sstream>
#include <string>

using namespace std;

/* Add a new entry to the table, return true if succeed */
bool Account::addAccount(connection * C, int account_id, double balance) {
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
bool Account::idExists(connection * C,int account_id) {
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