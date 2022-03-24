#include "position.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
/* Add a new entry to the table */
void Position::addPosition(connection * C,
                           const string & symbol_name,
                           int account_id,
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