#include "database.h"

database::database(QString path) {
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  this->db_name = path;
  db.setDatabaseName(path);

  if (!db.open()) {
    throw std::runtime_error("Could not open Database");
  }
  QString query_create_person{
      QString::fromStdString("CREATE TABLE IF NOT EXISTS person "
                             "(id INTEGER PRIMARY KEY AUTOINCREMENT, "
                             "firstname TEXT NOT NULL, "
                             "surname TEXT NOT NULL"
                             ");")};
  QSqlQuery query;
  if (!query.exec(query_create_person)) {
    throw std::runtime_error("Error creating table");
  }

  QString query_create_uses_mailaddress{
      QString::fromStdString("CREATE TABLE IF NOT EXISTS uses_mailaddress "
                             "(id INTEGER, "
                             "mail TEXT NOT NULL PRIMARY KEY"
                             ");")};
  if (!query.exec(query_create_uses_mailaddress)) {
    throw std::runtime_error("Error creating table");
  }
}

bool database::insert_into_db(const QString& firstname,
                              const QString& surname) {
  QSqlQuery query;

  query.prepare(
      "SELECT EXISTS(SELECT 1 FROM person WHERE firstname = ? and "
      "surname = "
      "?);");
  query.addBindValue(firstname);
  query.addBindValue(surname);
  if (!query.exec()) {
    return false;
  }
  query.first();
  if (query.value(0) == "0") {
    query.prepare(
        "INSERT INTO person(firstname, surname) "
        "VALUES(?, ?);");
    query.addBindValue(firstname);
    query.addBindValue(surname);
    if (!query.exec()) {
      return false;
    }
  }
  return true;
}

std::optional<QString> database::get_id(const QString& firstname,
                                        const QString& surname) {
  QSqlQuery query;

  query.prepare("SELECT id FROM person WHERE firstname = ? and surname = ?");
  query.addBindValue(firstname);
  query.addBindValue(surname);

  if (!query.exec()) {
    {};
  }

  if (query.first()) {
    return std::optional<QString>(query.value(0).toString());
  }

  return {};
}

bool database::insert_into_db(const QString& firstname, const QString& surname,
                              const QString& email) {
  std::optional<QString> id{get_id(firstname, surname)};

  if (!id.has_value() && insert_into_db(firstname, surname)) {
    id = get_id(firstname, surname);
  }

  // insert_into_db most likely failed
  if (!id.has_value()) {
    return false;
  }

  QSqlQuery query;

  query.prepare(
      "INSERT INTO uses_mailaddress(id, mail) "
      "values(?, ?);");
  query.addBindValue(id.value());
  query.addBindValue(email);

  return query.exec();
}

std::vector<std::array<QString, database::QUERY_COLUMN::SIZE>>
database::extract_query(QSqlQuery& query) {
  std::vector<std::array<QString, QUERY_COLUMN::SIZE>> to_return;

  while (query.next()) {
    std::array<QString, QUERY_COLUMN::SIZE> to_push{
        query.value(QUERY_COLUMN::FIRSTNAME).toString(),
        query.value(QUERY_COLUMN::SURNAME).toString(),
        query.value(QUERY_COLUMN::EMAIL).toString()};
    to_return.push_back(to_push);
  }

  return to_return;
}

std::vector<std::array<QString, database::QUERY_COLUMN::SIZE>>
database::find_by_full_name(const QString& firstname, const QString& surname) {
  QSqlQuery query;
  query.prepare(
      "SELECT p.firstname, p.surname, m.mail FROM uses_mailaddress m, person p "
      "WHERE p.firstname LIKE "
      "? and p.surname LIKE ? and m.id = p.id;");
  query.addBindValue(firstname);
  query.addBindValue(surname);
  query.exec();

  return extract_query(query);
}

std::vector<std::array<QString, database::QUERY_COLUMN::SIZE>>
database::find_by_one_name(const QString& name, bool is_firstname) {
  QSqlQuery query;
  QString query_string{
      "SELECT p.firstname, p.surname, m.mail FROM uses_mailaddress m, person p "
      "WHERE " +
      (is_firstname ? QString::fromStdString("p.firstname ")
                    : QString::fromStdString("p.surname ")) +
      "LIKE ? and m.id = p.id;"};
  query.prepare(query_string);
  query.addBindValue(name);
  query.exec();

  return extract_query(query);
}

std::vector<std::array<QString, database::QUERY_COLUMN::SIZE>>
database::find_by_email(const QString& email) {
  QSqlQuery query;
  QString query_string{
      "SELECT p.firstname, p.surname, m.mail FROM person p, uses_mailaddress m "
      "WHERE m.mail LIKE ? and "
      "m.id = p.id;"};
  query.prepare(query_string);
  query.addBindValue(email);
  query.exec();

  return extract_query(query);
}

std::optional<QString> database::get_id_by_email(const QString& email) {
  QSqlQuery query;
  QString query_string("SELECT id FROM uses_mailaddress WHERE mail = ?");
  query.prepare(query_string);
  query.addBindValue(email);

  if (!query.exec() || !query.first()) {
    return {};
  }

  return query.value(0).toString();
}

std::vector<std::array<QString, database::QUERY_COLUMN::SIZE>>
database::find_by_id(const QString& id) {
  QSqlQuery query;
  QString query_string(
      "SELECT p.firstname, p.surname, m.mail FROM uses_mailaddress m, person p "
      "WHERE p.id = ? and m.id = ?;");
  query.prepare(query_string);
  query.addBindValue(id);
  query.addBindValue(id);
  if (!query.exec()) {
    return {};
  }

  return extract_query(query);
}

bool database::remove_by_email(const QString& email) {
  QSqlQuery query;
  QString query_string("DELETE FROM uses_mailaddress WHERE mail = ?;");
  query.prepare(query_string);
  query.addBindValue(email);

  return query.exec();
}

bool database::remove_person_by_email(const QString& email) {
  QSqlQuery query;
  QString query_find_id("SELECT id FROM uses_mailaddress WHERE mail = ?;");
  query.prepare(query_find_id);
  query.addBindValue(email);

  if (!query.exec()) {
    return false;
  }

  if (!query.first()) {
    return false;
  }

  QString id{query.value(0).toString()};

  QString query_delete_from_uses_mailaddress(
      "DELETE FROM uses_mailaddress WHERE id = ?;");
  query.prepare(query_delete_from_uses_mailaddress);
  query.addBindValue(id);

  if (!query.exec()) {
    return false;
  }

  QString query_delete_from_person("DELETE FROM person WHERE id = ?;");
  query.prepare(query_delete_from_person);
  query.addBindValue(id);

  return query.exec();
}
