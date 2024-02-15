#include "query.h"

void query::update_last_query(const QString &firstname, const QString &surname,
                              const QString &mail) {
  last_query[database::QUERY_COLUMN::FIRSTNAME] = firstname;
  last_query[database::QUERY_COLUMN::SURNAME] = surname;
  last_query[database::QUERY_COLUMN::EMAIL] = mail;
}

std::vector<std::array<QString, database::QUERY_COLUMN::SIZE>>
query::execute_search_query(const QString &firstname, const QString &surname,
                            const QString &mail) {
  update_last_query(firstname, surname, mail);

  return this->refresh_query();
}

bool query::execute_insertion_query(const QString &firstname,
                                    const QString &surname,
                                    const QString &mail) {
  update_last_query(firstname, surname, mail);

  return db.insert_into_db(firstname, surname, mail);
}

bool query::execute_remove_query(const QString &mail) {
  // last_query does not need to be updated, as a search must have happened
  // beforehand´
  return db.remove_by_email(mail);
}

std::vector<std::array<QString, database::QUERY_COLUMN::SIZE>>
query::refresh_query() {
  const QString &firstname = last_query[database::QUERY_COLUMN::FIRSTNAME];
  const QString &surname = last_query[database::QUERY_COLUMN::SURNAME];
  const QString &mail = last_query[database::QUERY_COLUMN::EMAIL];

  if (mail.isEmpty() && firstname.isEmpty() && surname.isEmpty()) {
    return {};
  }

  if (!mail.isEmpty() && firstname.isEmpty() && surname.isEmpty()) {
    return db.find_by_email(mail);
  }

  if (!firstname.isEmpty() && !surname.isEmpty()) {
    return db.find_by_full_name(firstname, surname);
  }

  if (surname.isEmpty()) {
    return db.find_by_one_name(firstname, true);
  }

  if (firstname.isEmpty()) {
    return db.find_by_one_name(surname, false);
  }

  return {};
}
