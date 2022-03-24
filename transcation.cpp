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