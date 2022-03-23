#define ACCOUNT_H

#include <pqxx/pqxx>
#include <string>

#include "database.h"

using std::string;
using namespace pqxx;

class Account
{
 private:
  /* The fields of this table:
     
     string account_id;
     double balance;
  */

 public:
  /* Add a new entry to the table, return true if succeed */
  static bool addAccount(connection * C, int account_id, double balance);

  /* Check if the given account already exists */
  static bool idExists(connection * C, int account_id);
};

