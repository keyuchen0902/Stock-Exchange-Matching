#define ACCOUNT_H

#include <pqxx/pqxx>
#include <string>

#include "database.h"
#include "tinyxml2.h"

using std::string;
using namespace pqxx;
using namespace tinyxml2;

class Account
{
public:
    /* Add a new entry to the table, return true if succeed */
    static bool addAccount(connection *C, string account_id, double balance);
    /* Check if the given account already exists */
    static bool idExists(connection *C, string account_id);
    static bool enoughBalance(connection *C, string account_id, double requiredBalance);
};

class Position
{
public:
    /* Add a new entry to the table */
    static void addPosition(connection *C,
                            const string &symbol_name,
                            string account_id,
                            int num_share);
    static bool updateSymbolAmount(connection *C, string symbol_name, string account_id, int amount);
};

class Transaction
{
 public:
  /* Add a new entry to the table, return the transaction_id */
  static int addTransaction(connection * C,
                            const string & account_id,
                            const string & symbol_name,
                            double limited,
                            int num_open);

   static bool isTransExists(connection * C, int trans_id);
   static void handleQuery(connection *C,int trans_id,XMLDocument* response,XMLElement* usernode);
   static void handleCancel(connection *C,int trans_id,XMLDocument* response,XMLElement* usernode);
   static double getLimited(connection * C, int trans_id);
   static long getCanceledTime(connection * C, int trans_id);
    static bool matchOrder(connection * C, int trans_id);
}; 

class Execution
{
 public:
  /* Add a new entry to the table
     The parameter is work &, so it's part of another transaction */
  static void addExecution(work & W,
                                 int buyer_trans_id,
                                 int seller_trans_id,
                                 int amount,
                                 double price);
};