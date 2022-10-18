#ifndef OKSCONFIG_ROKSCONFIGURATION_H_
#define OKSCONFIG_ROKSCONFIGURATION_H_

#include "oksconfig/OksConfiguration.h"

class ROksConfiguration : public OksConfiguration {

  friend class OksConfigObject;

  public:

    ROksConfiguration() noexcept { ; }
    virtual ~ROksConfiguration() { close_db(); }


  public:

    virtual void open_db(const std::string& db_name);
    virtual void close_db();


  private:

    std::string m_schema_file;
    std::string m_data_file;

    void bad_open_db(const std::string& text);

};

#endif // OKSCONFIG_OKSCONFIGURATION_H_
