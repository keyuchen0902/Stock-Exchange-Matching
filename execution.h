#ifndef EXECUTION_H
#define EXECUTION_H

#include <pqxx/pqxx>
#include <string>

#include "database.h"
#include "functions.h"

using std::string;
using namespace pqxx;

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
#endif