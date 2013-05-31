#include <vector>
#include <ios>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <ctime>

void printInvoiceTable(unsigned int numRows);
void printItemTable(unsigned int numRows, unsigned factor);

int main(int argc, char *argv[])
{
  unsigned numRows = 100;
  unsigned factor = 5;


  
  if (argc > 1) {
    if ( (strncmp(argv[1], "--help", 6)==0) or (strncmp(argv[1], "-h", 2)==0 ) ) {
      //std::cout << "Generate a two column table for DefaultDictVector. First column will contain random values, second column nearly only the default value." << std::endl;
      std::cout << "Usage: create INVOICES AVGITEMSPERINVOICE" << std::endl;
      std::cout << "Default is: create 100 5" << std::endl;
      exit(0);
    }

    if (argc > 1) {   // number of rows (invoice)
      std::stringstream str;
      str << argv[1];
      str >> numRows;
      if (argc > 2) {   // avg number of items per invoice
        std::stringstream str;
        str << argv[2];
        str >> factor;
       }
     }
  }

  printInvoiceTable(numRows);

  printItemTable(numRows, factor);

  return 0;
}


void printInvoiceTable(unsigned int numRows)
{
  std::ofstream out("InvoiceTable.tbl", std::fstream::trunc | std::fstream::out);
  if (out.fail()) {
    std::cerr << "Cannot open output file \"InvoiceTable.tbl\"." << std::endl;
    exit(2);
  }

  // print header
  out << "invoiceId|date|daysSinceToday|customerId" << std::endl 
      << "INTEGER|STRING|INTEGER|INTEGER" << std::endl 
      << "0_R|1_R|2_R|2_R" << std::endl 
      << "===" << std::endl;

  // create "today" as maximal possible date value
  time_t today_raw = (42*365 + 199)*24*60*60;

  for (int id = 0; id < numRows; ++id) {
    time_t rawtime=(42*365 + (id*id) % 200)*24*60*60;
    int daysSinceToday = (today_raw - rawtime) / (60*60*24);
    struct tm *timeptr = localtime(&rawtime);
    char timeString[11];
    strftime(timeString, 11, "%F", timeptr);

    std::string date (timeString);

    out << id << "|" << date << "|" << daysSinceToday << "|" << id%5 << std::endl;
  }

  out.close();
}

void printItemTable(unsigned int numRows, unsigned factor)
{
  std::ofstream out("ItemTable.tbl", std::fstream::trunc | std::fstream::out);
  if (out.fail()) {
    std::cerr << "Cannot open output file \"ItemTable.tbl\"." << std::endl;
    exit(2);
  }

  // print header
  out << "invoiceId|itemId|count|price|currency|itemCanceled" << std::endl
      << "INTEGER|INTEGER|INTEGER|FLOAT|STRING|INTEGER" << std::endl
      << "0_R|1_R|2_R|3_R|4_R|5_R" << std::endl
      << "===" << std::endl;


  std::string currencies[] = {"euro", "dollar", "yen", "pound"};
  int numCurrencies = sizeof(currencies) / sizeof(std::string);

  unsigned numItemsPerInvoice;
  unsigned itemId;
  unsigned itemCount;
  float price;
  std::string currency;
  unsigned itemCanceled;
  int x;
  unsigned long count = 0;

  for (int invId = 0; invId < numRows; ++invId) {
    numItemsPerInvoice = invId % (2 * factor - 1) + 1;
    for (int i = 1; i <= numItemsPerInvoice; ++i) {    // AVG should be =factor (= Erwartungswert)
      itemId = i % numItemsPerInvoice;
      itemCount = (count % 100) + 1;
      price = (invId % 10000) / 100.0f;
      x = count % 100;
      if (x < 99) {
        currency = currencies[0];
      }
      else
        currency = currencies[(count % (numCurrencies-1)) + 1];

      itemCanceled = (count % (100/3)) ? 0 : 1;    // 3% items canceled

      out << invId << "|" << itemId << "|" << itemCount << "|" << price << "|" << currency << "|" << itemCanceled << std::endl;

      ++ count;      
    }
  }

  out.close();
}
