#include "execution.h"

#include <fstream>
#include <sstream>
#include <string>

using std::string;
/* Add a new entry to the table */
void Execution::addExecution(work & W,
                                   int buyer_trans_id,
                                   int seller_trans_id,
                                   int amount,
                                   double price) {
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