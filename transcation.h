#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <pqxx/pqxx>
#include <string>

#include "functions.h"
#include "database.h"
#include "tinyxml2.h"

using std::string;
using namespace pqxx;
using namespace tinyxml2;

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
   static bool isTransExists(connection * C, int trans_id);
   static void handleQuery(connection *C,int trans_id,XMLDocument* response,XMLElement* usernode);
   static void handleCancel(connection *C,int trans_id,XMLDocument* response,XMLElement* usernode);
   static double getLimited(connection * C, int trans_id);
   static long getCanceledTime(connection * C, int trans_id);
    static bool matchOrder(connection * C, int trans_id);
}; 
#endif