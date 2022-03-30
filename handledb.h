#define ACCOUNT_H

#include <pqxx/pqxx>
#include <string>
#include <mutex>
#include "database.h"
#include "tinyxml2.h"

using namespace std;
using namespace pqxx;
using namespace tinyxml2;

extern std::mutex mymutex;

class MyDB{
  public:
  static bool addAccount(connection *C, string &account_id, double balance);
  static bool idExists(connection *C, string &account_id);
  static bool enoughBalance(connection *C, string &account_id, double requiredBalance);
  static void addPosition(connection *C,string &symbol_name,string &account_id,int num_share);
  static bool updateSymbolAmount(connection *C, string &symbol_name, string &account_id, int amount);
static int addTransaction(connection * C,const string & account_id,const string & symbol_name,double limited,int num_open);
   static bool isTransExists(connection * C, int trans_id);
   static void handleQuery(connection *C,int trans_id,XMLDocument* response,XMLElement* usernode);
   static void handleCancel(connection *C,int trans_id,XMLDocument* response,XMLElement* usernode);
   static double getLimited(connection * C, int trans_id);
   static long getCanceledTime(connection * C, int trans_id);
    static bool matchOrder(connection * C, int trans_id);
    static void addExecution(work & W,int buyer_trans_id,int seller_trans_id,int amount,double price);

};


class MyLock
{
 private:
  std::mutex * mtx;

 public:
  explicit MyLock(std::mutex * temp) {
    temp->lock();
    mtx = temp;
  }

  ~MyLock() { mtx->unlock(); }
};






