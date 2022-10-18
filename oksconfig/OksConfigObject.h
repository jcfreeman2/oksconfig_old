// this is -*- c++ -*-
#ifndef OKSCONFIG_OKSCONFIGOBJECT_H_
#define OKSCONFIG_OKSCONFIGOBJECT_H_

#include <string>
#include <vector>

#include <oks/kernel.h>
#include <oks/object.h>

#include "config/ConfigObjectImpl.h"

class ConfigurationImpl;
class OksConfiguration;

class OksConfigObject :  public ConfigObjectImpl {

  friend class ConfigurationImpl;
  friend class OksConfiguration;

  public:

    OksConfigObject(OksObject *obj, ConfigurationImpl * impl) noexcept;
    virtual ~OksConfigObject() noexcept;


  public:

    virtual const std::string contained_in() const;

    virtual void get(const std::string& name, bool&           value);
    virtual void get(const std::string& name, uint8_t&        value);
    virtual void get(const std::string& name, int8_t&         value);
    virtual void get(const std::string& name, uint16_t&       value);
    virtual void get(const std::string& name, int16_t&        value);
    virtual void get(const std::string& name, uint32_t&       value);
    virtual void get(const std::string& name, int32_t&        value);
    virtual void get(const std::string& name, uint64_t&       value);
    virtual void get(const std::string& name, int64_t&        value);
    virtual void get(const std::string& name, float&          value);
    virtual void get(const std::string& name, double&         value);
    virtual void get(const std::string& name, std::string&    value);
    virtual void get(const std::string& name, ConfigObject&   value);

    virtual void get(const std::string& name, std::vector<bool>&           value);
    virtual void get(const std::string& name, std::vector<uint8_t>&        value);
    virtual void get(const std::string& name, std::vector<int8_t>&         value);
    virtual void get(const std::string& name, std::vector<uint16_t>&       value);
    virtual void get(const std::string& name, std::vector<int16_t>&        value);
    virtual void get(const std::string& name, std::vector<uint32_t>&       value);
    virtual void get(const std::string& name, std::vector<int32_t>&        value);
    virtual void get(const std::string& name, std::vector<uint64_t>&       value);
    virtual void get(const std::string& name, std::vector<int64_t>&        value);
    virtual void get(const std::string& name, std::vector<float>&          value);
    virtual void get(const std::string& name, std::vector<double>&         value);
    virtual void get(const std::string& name, std::vector<std::string>&    value);
    virtual void get(const std::string& name, std::vector<ConfigObject>&   value);

    virtual bool rel(const std::string& name, std::vector<ConfigObject>& value);
    virtual void referenced_by(std::vector<ConfigObject>& value, const std::string& association, bool check_composite_only, unsigned long rlevel, const std::vector<std::string> * rclasses) const;


    virtual void set(const std::string& name, bool                value);
    virtual void set(const std::string& name, uint8_t             value);
    virtual void set(const std::string& name, int8_t              value);
    virtual void set(const std::string& name, uint16_t            value);
    virtual void set(const std::string& name, int16_t             value);
    virtual void set(const std::string& name, uint32_t            value);
    virtual void set(const std::string& name, int32_t             value);
    virtual void set(const std::string& name, uint64_t            value);
    virtual void set(const std::string& name, int64_t             value);
    virtual void set(const std::string& name, float               value);
    virtual void set(const std::string& name, double              value);
    virtual void set(const std::string& name, const std::string&  value);

    virtual void set_enum(const std::string& attribute, const std::string& value);
    virtual void set_date(const std::string& attribute, const std::string& value);
    virtual void set_time(const std::string& attribute, const std::string& value);

    virtual void set_class(const std::string& attribute, const std::string& value);

    virtual void set(const std::string& name, const std::vector<bool>&           value);
    virtual void set(const std::string& name, const std::vector<uint8_t>&        value);
    virtual void set(const std::string& name, const std::vector<int8_t>&         value);
    virtual void set(const std::string& name, const std::vector<uint16_t>&       value);
    virtual void set(const std::string& name, const std::vector<int16_t>&        value);
    virtual void set(const std::string& name, const std::vector<uint32_t>&       value);
    virtual void set(const std::string& name, const std::vector<int32_t>&        value);
    virtual void set(const std::string& name, const std::vector<uint64_t>&       value);
    virtual void set(const std::string& name, const std::vector<int64_t>&        value);
    virtual void set(const std::string& name, const std::vector<float>&          value);
    virtual void set(const std::string& name, const std::vector<double>&         value);
    virtual void set(const std::string& name, const std::vector<std::string>&    value);

    virtual void set_enum(const std::string& attribute, const std::vector<std::string>& value);
    virtual void set_date(const std::string& attribute, const std::vector<std::string>& value);
    virtual void set_time(const std::string& attribute, const std::vector<std::string>& value);

    virtual void set_class(const std::string& attribute, const std::vector<std::string>& value);

    virtual void set(const std::string& name, const ConfigObject*                     value, bool skip_non_null_check);
    virtual void set(const std::string& name, const std::vector<const ConfigObject*>& value, bool skip_non_null_check);

    virtual void move(const std::string& at);
    virtual void rename(const std::string& new_id);

    virtual void reset();

  public:

      // required by JNI

    OksData::Type get_type(const std::string& attribute) const;


    // helper functions

  private:

      // extract values out of OKS database

    template<class T> void get_value(const std::string& name, T& value);
    template<class T> void get_vector(const std::string& name, std::vector<T>& value);


      // set values in OKS database

    void set_attr_value(const std::string& name, OksData& value);
    void set_rel_value(const std::string& name, OksData& value, bool skip_non_null_check);

    template<class T> void set_value(const std::string& name, const T& value);
    template<class T> void set_vector(const std::string& name, const std::vector<T>& value);


      // checkout file, if is needed for update

//    void test_checkout_needs();


    // required by template method insert_object
    void set(OksObject *obj)
      {
        m_obj = obj;
      }

    // representation

  private:

    OksObject *m_obj;
};

#endif // OKSCONFIG_OKSCONFIGOBJECT_H_
