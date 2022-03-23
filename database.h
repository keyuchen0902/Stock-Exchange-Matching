#ifndef DATABASE_H
#define DATABASE_H

#include <exception>
#include <iostream>
#include <pqxx/pqxx>
#include <string>

using std::string;
using namespace pqxx;

class Database
{
 public:
  /* Create table in the database connected by W
     If it already exists, delete it first */
  static void createTable(connection * C) {
    /* Create a transactional object. */
    string dropAccount= "DROP TABLE IF EXISTS ACCOUNT CASCADE;";

    string createTableAccount = "CREATE TABLE ACCOUNT ("
                          "ACCOUNT_ID      INT    PRIMARY KEY NOT NULL, "
                          "BALANCE         REAL    NOT NULL);";
    string dropPosition = "DROP TABLE IF EXISTS POSITION CASCADE;";

    string createTablePosition = "CREATE TABLE POSITION ("
                          "SYMBOL_NAME     TEXT    NOT NULL, "
                          "ACCOUNT_ID      TEXT    NOT NULL, "
                          "NUM_SHARE       INT     NOT NULL, "
                          "CONSTRAINT POSITION_PKEY PRIMARY KEY (SYMBOL_NAME, ACCOUNT_ID));";
    
    string dropTransaction = "DROP TABLE IF EXISTS TRANSACTION CASCADE;";

    string createTableTranscation = "CREATE TABLE TRANSACTION ("
                          "TRANSACTION_ID  SERIAL  PRIMARY KEY NOT NULL, "
                          "ACCOUNT_ID      TEXT    NOT NULL, "
                          "SYMBOL_NAME     TEXT    NOT NULL, "
                          "LIMITED         REAL    NOT NULL, "
                          "NUM_OPEN        INT     NOT NULL, "
                          "NUM_CANCELED    INT     NOT NULL, "
                          "OPEN_TIME       BIGINT  NOT NULL, "
                          "CANCEL_TIME     BIGINT  NOT NULL DEFAULT 0);";
    
    string dropExecution = "DROP TABLE IF EXISTS EXECUTION CASCADE;";

    string createTableExecution = "CREATE TABLE EXECUTION ("
                          "EXECUTION_ID    SERIAL  PRIMARY KEY NOT NULL, "
                          "BUYER_TRANS_ID  INT     NOT NULL, "
                          "SELLER_TRANS_ID INT     NOT NULL, "
                          "AMOUNT          INT     NOT NULL, "
                          "PRICE           REAL    NOT NULL, "
                          "TIME            BIGINT  NOT NULL);";
    
    work W(*C);


    // Drop existing table
    W.exec(dropAccount);
    W.exec(dropPosition);
    W.exec(dropTransaction);
    W.exec(dropExecution);

    // Create new table
    W.exec(createTableAccount);
    W.exec(createTablePosition);
    W.exec(createTableTranscation);
    W.exec(createTableExecution);

    W.commit();
  }

  /* Build foreign keys */
  static void buildForeignKeys(connection * C) {
    /* Create a transactional object. */
    string buildForeignKeysAccount = ";";
    string buildForeignKeysPosition = "ALTER TABLE POSITION "
                               "ADD CONSTRAINT POSITION_ACCOUNT_ID_FKEY FOREIGN KEY "
                               "(ACCOUNT_ID) REFERENCES ACCOUNT(ACCOUNT_ID);";

    string buildForeignKeysTranscation = "ALTER TABLE TRANSACTION "
                               "ADD CONSTRAINT TRANSACTION_ACCOUNT_ID_FKEY FOREIGN KEY "
                               "(ACCOUNT_ID) REFERENCES ACCOUNT(ACCOUNT_ID);";

    string buildForeignKeysExecution = "ALTER TABLE EXECUTION "
                               "ADD CONSTRAINT EXECUTION_TRANSACTION_ID_FKEY_1 FOREIGN KEY "
                               "(BUYER_TRANS_ID) REFERENCES TRANSACTION(TRANSACTION_ID); "
                               "ALTER TABLE EXECUTION "
                               "ADD CONSTRAINT EXECUTION_TRANSACTION_ID_FKEY_2 FOREIGN KEY "
                               "(SELLER_TRANS_ID) REFERENCES TRANSACTION(TRANSACTION_ID);";
    work W(*C);
    
    W.exec(buildForeignKeysAccount);
    W.exec(buildForeignKeysPosition);
    W.exec(buildForeignKeysTranscation);
    W.exec(buildForeignKeysExecution);
    W.commit();
  }
};

#endif