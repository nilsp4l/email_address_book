#ifndef QUERY_H
#define QUERY_H

#include "database.h"

class query {
 public:
  query(QString database_path) : db(database_path) {}

  std::vector<std::array<QString, database::QUERY_COLUMN::SIZE>>
  execute_search_query(const QString &firstname, const QString &surname,
                       const QString &email);

  // execute the last query executed
  std::vector<std::array<QString, database::QUERY_COLUMN::SIZE>>
  refresh_query();

  bool execute_insertion_query(const QString &firstname, const QString &surname,
                               const QString &mail);
  bool execute_remove_query(const QString &mail);

 private:
  void update_last_query(const QString &firstname, const QString &surname,
                         const QString &mail);
  std::array<QString, database::QUERY_COLUMN::SIZE> last_query = {"", "", ""};
  database db;
};

#endif  // QUERY_H
