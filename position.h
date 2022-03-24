#ifndef POSITION_H
#define POSITION_H

#include <pqxx/pqxx>
#include <string>

#include "database.h"

using std::string;
using namespace pqxx;

class Position
{
 private:
  /* The fields of this table:
     
     string symbol_name;
     string account_id;
     int num_share;
  */

 public:

  /* Add a new entry to the table */
  static void addPosition(connection * C,
                          const string & symbol_name,
                          int account_id,
                          int num_share);
};
#endif