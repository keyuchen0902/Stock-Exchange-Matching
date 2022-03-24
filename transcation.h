#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <pqxx/pqxx>
#include <string>

#include "functions.h"
#include "database.h"

using std::string;
using namespace pqxx;

class Transaction
{
 public:
  /* Add a new entry to the table, return the transaction_id */
  static int addTransaction(connection * C,
                            const string & account_id,
                            const string & symbol_name,
                            double limited,
                            int num_open);

  /* Try to match a transaction,
     return true if another matching can still be achieved */
  //static bool tryMatch(connection * C, int trans_id);
};
#endif