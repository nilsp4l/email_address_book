#ifndef DATABASE_H
#define DATABASE_H
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <array>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

class database {
 public:
  enum QUERY_COLUMN { FIRSTNAME = 0, SURNAME, EMAIL, SIZE };

  database(QString path);
  QString query(QString& q);
  bool insert_into_db(const QString& first_name, const QString& last_name);
  bool insert_into_db(const QString& first_name, const QString& last_name,
                      const QString& email);
  std::vector<std::array<QString, QUERY_COLUMN::SIZE>> find_by_full_name(
      const QString& first_name, const QString& last_name);
  std::vector<std::array<QString, QUERY_COLUMN::SIZE>> find_by_one_name(
      const QString& name, bool is_firstname);
  std::vector<std::array<QString, QUERY_COLUMN::SIZE>> find_by_email(
      const QString& email);
  std::vector<std::array<QString, database::QUERY_COLUMN::SIZE>> find_by_id(
      const QString& id);

  std::optional<QString> get_id_by_email(const QString& email);

  bool remove_by_email(const QString& email);
  bool remove_by_id(long id);
  bool remove_person_by_email(const QString& email);

 private:
  std::vector<std::array<QString, QUERY_COLUMN::SIZE>> extract_query(
      QSqlQuery& query);
  std::optional<QString> get_id(const QString& firstname,
                                const QString& surname);
  QString db_name;
};

#endif  // DATABASE_H
