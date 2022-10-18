#include <ers/ers.h>

#include <oks/file.h>
#include <oks/relationship.h>

#include <config/ConfigObject.h>

#include "oksconfig/OksConfigObject.h"
#include "oksconfig/OksConfiguration.h"


static std::string
mk_error_text(const char *op, const char * what, const std::string& name, const OksObject * o, const char * error)
{
  std::ostringstream text;
  text << "failed to " << op << ' ' << what << " \"" << name << "\" of object " << o;
  if (error)
    text << ": " << error;
  return text.str();
}

static std::string
mk_get_error_text(const char * what, const std::string& name, const OksObject * o, const char * error = nullptr)
{
  return mk_error_text("read", what, name, o, error);
}

static std::string
mk_set_error_text(const char * what, const std::string& name, const OksObject * o, const char * error = nullptr)
{
  return mk_error_text("set value", what, name, o, error);
}


OksData::Type
OksConfigObject::get_type(const std::string& name) const
{
  try
    {
      return m_obj->GetAttributeValue(name)->type;
    }
  catch (oks::exception& ex)
    {
      throw daq::config::Generic(ERS_HERE, mk_get_error_text("attribute", name, m_obj).c_str(), ex);
    }
}

// helper function to extract value out of OKS database
template<class T>
  void
  OksConfigObject::get_value(const std::string& name, T& value)
  {
    std::lock_guard<std::mutex> scoped_lock(m_mutex);

    try
      {
        throw_if_deleted();
        OksData * data(m_obj->GetAttributeValue(name));
        value = *reinterpret_cast<T*>(&data->data); // really ugly stuff here, but should come out right
      }
    catch (oks::exception& ex)
      {
        throw daq::config::Generic(ERS_HERE, mk_get_error_text("attribute", name, m_obj).c_str(), ex);
      }
  }


// helper function to extract vector of values out of OKS database
template<class T>
  void
  OksConfigObject::get_vector(const std::string& name, std::vector<T>& value)
  {
    OksData * data = 0;

    std::lock_guard<std::mutex> scoped_lock(m_mutex);

    try
      {
        throw_if_deleted();
        data = m_obj->GetAttributeValue(name);
      }
    catch (oks::exception& ex)
      {
        throw daq::config::Generic( ERS_HERE, mk_get_error_text("attribute", name, m_obj).c_str(), ex);
      }

    if (data->type != OksData::list_type)
      {
        std::ostringstream text;
        text << "attribute \"" << name << "\" of object " << m_obj << " is not of a multi-value";
        throw daq::config::Generic( ERS_HERE, text.str().c_str() );
      }

    value.clear();
    for (const auto& it : *data->data.LIST)
      {
        value.push_back(*reinterpret_cast<T*>(&it->data));
      }
  }


OksConfigObject::OksConfigObject(OksObject *obj, ConfigurationImpl * impl) noexcept :
  ConfigObjectImpl (impl, obj->GetId()),
  m_obj            (obj)
{
}

OksConfigObject::~OksConfigObject() noexcept
{
}

const std::string
OksConfigObject::contained_in() const
{
  std::lock_guard<std::mutex> scoped_lock(m_mutex);
  throw_if_deleted();
  return m_obj->get_file()->get_full_file_name();
}

void
OksConfigObject::get(const std::string& name, bool& value)
{
  get_value(name, value);
}

void
OksConfigObject::get(const std::string& name, uint8_t& value)
{
  get_value(name, value);
}

void
OksConfigObject::get(const std::string& name, int8_t& value)
{
  get_value(name, value);
}

void
OksConfigObject::get(const std::string& name, uint16_t& value)
{
  get_value(name, value);
}

void
OksConfigObject::get(const std::string& name, int16_t& value)
{
  get_value(name, value);
}

void
OksConfigObject::get(const std::string& name, uint32_t& value)
{
  get_value(name, value);
}

void
OksConfigObject::get(const std::string& name, int32_t& value)
{
  get_value(name, value);
}

void
OksConfigObject::get(const std::string& name, uint64_t& value)
{
  get_value(name, value);
}

void
OksConfigObject::get(const std::string& name, int64_t& value)
{
  get_value(name, value);
}

void
OksConfigObject::get(const std::string& name, float& value)
{
  get_value(name, value);
}

void
OksConfigObject::get(const std::string& name, double& value)
{
  get_value(name, value);
}

void
OksConfigObject::get(const std::string& name, std::string& value)
{
  OksData * data = 0;

  std::lock_guard<std::mutex> scoped_lock(m_mutex);

  try
    {
      throw_if_deleted();
      data = m_obj->GetAttributeValue(name);
    }
  catch (oks::exception& ex)
    {
      throw daq::config::Generic( ERS_HERE, mk_get_error_text("attribute", name, m_obj).c_str(), ex);
    }

  if ((data->type == OksData::string_type) || (data->type == OksData::enum_type))
    {
      value = *data->data.STRING;
      return;
    }
  else if (data->type == OksData::date_type || data->type == OksData::time_type)
    {
      value = data->str();
      return;
    }
  else if (data->type == OksData::class_type)
    {
      value = data->data.CLASS->get_name();
      return;
    }
  else
    {
      std::ostringstream text;
      text << "read wrong attribute type instead of expected \'string\' for attribute \"" << name << "\" of object " << m_obj;
      throw daq::config::Generic( ERS_HERE, text.str().c_str() );
    }
}

void
OksConfigObject::get(const std::string& name, ConfigObject& value)
{
  OksData * data = 0;

  std::lock_guard<std::mutex> scoped_lock(m_mutex);

  try
    {
      throw_if_deleted();
      data = m_obj->GetRelationshipValue(name);
    }
  catch (oks::exception& ex)
    {
      throw daq::config::Generic(ERS_HERE, mk_get_error_text("relationship", name, m_obj).c_str(), ex);
    }

  if (data->type != OksData::object_type)
    {
      if (data->type == OksData::uid_type || data->type == OksData::uid2_type)
        {
          std::ostringstream text;
          text << "referenced object " << *data << " is not loaded";
          throw daq::config::Generic(ERS_HERE, mk_get_error_text("relationship", name, m_obj, text.str().c_str()).c_str());
        }

      std::ostringstream text;
      text << "bad value of relationship \"" << name << "\" of object " << m_obj << ": ";

      if(data->type == OksData::list_type)
        {
          text << "the relationship is multi-value";
        }
      else
        {
          text << "non-object type was read";
        }
      throw daq::config::Generic(ERS_HERE, text.str().c_str());
    }

  if (data->data.OBJECT != nullptr)
    {
      value = static_cast<OksConfiguration *>(m_impl)->new_object(data->data.OBJECT);
    }
  else
    {
      value = nullptr;
    }
}

void
OksConfigObject::get(const std::string& name, std::vector<bool>& value)
{
  get_vector(name, value);
}

void
OksConfigObject::get(const std::string& name, std::vector<uint8_t>& value)
{
  get_vector(name, value);
}

void
OksConfigObject::get(const std::string& name, std::vector<int8_t>& value)
{
  get_vector(name, value);
}

void
OksConfigObject::get(const std::string& name, std::vector<uint16_t>& value)
{
  get_vector(name, value);
}

void
OksConfigObject::get(const std::string& name, std::vector<int16_t>& value)
{
  get_vector(name, value);
}

void
OksConfigObject::get(const std::string& name, std::vector<uint32_t>& value)
{
  get_vector(name, value);
}

void
OksConfigObject::get(const std::string& name, std::vector<int32_t>& value)
{
  get_vector(name, value);
}

void
OksConfigObject::get(const std::string& name, std::vector<uint64_t>& value)
{
  get_vector(name, value);
}

void
OksConfigObject::get(const std::string& name, std::vector<int64_t>& value)
{
  get_vector(name, value);
}

void
OksConfigObject::get(const std::string& name, std::vector<float>& value)
{
  get_vector(name, value);
}

void
OksConfigObject::get(const std::string& name, std::vector<double>& value)
{
  get_vector(name, value);
}

void
OksConfigObject::get(const std::string& name, std::vector<std::string>& value)
{
  OksData * data = 0;

  std::lock_guard<std::mutex> scoped_lock(m_mutex);

  try
    {
      throw_if_deleted();
      data = m_obj->GetAttributeValue(name);
    }
  catch (oks::exception& ex)
    {
      throw daq::config::Generic(ERS_HERE, mk_get_error_text("attribute", name, m_obj).c_str(), ex);
    }

  if (data->type != OksData::list_type)
    {
      std::ostringstream text;
      text << "attribute \"" << name << "\" of object " << m_obj << " is not of a multi-value";
      throw daq::config::Generic( ERS_HERE, text.str().c_str());
    }

  value.clear();

  for (const auto & it : *data->data.LIST)
    {
      if ((it->type == OksData::string_type) || (it->type == OksData::enum_type))
        {
          value.emplace_back(*it->data.STRING);
        }
      else if (it->type == OksData::class_type)
        {
          value.emplace_back(it->data.CLASS->get_name());
        }
      else if (it->type == OksData::date_type || it->type == OksData::time_type)
        {
          value.emplace_back(it->str());
        }
      else
        {
          std::ostringstream text;
          text << "wrong type of attribute \"" << name << "\" of object " << m_obj << " (instead of expected string)";
          throw daq::config::Generic( ERS_HERE, text.str().c_str() );
        }
    }
}


void
OksConfigObject::get(const std::string& name, std::vector<ConfigObject>& value)
{
  OksData * data = 0;

  std::lock_guard<std::mutex> scoped_lock(m_mutex);

  try
    {
      throw_if_deleted();
      data = m_obj->GetRelationshipValue(name);
    }
  catch (oks::exception& ex)
    {
      throw daq::config::Generic(ERS_HERE, mk_get_error_text("relationship", name, m_obj).c_str(), ex);
    }

  if (data->type != OksData::list_type)
    {
      std::ostringstream text;
      text << "the value of relationship \"" << name << "\" of object " << m_obj << " is not multi-value";
      throw daq::config::Generic( ERS_HERE, text.str().c_str());
    }

  value.clear();

  for (const auto& it : *data->data.LIST)
    {
      if (it->type != OksData::object_type)
        {
          ERS_DEBUG(1, "object " << *it << " is not loaded (relationship \'" << name << "\' of object " << m_obj << ')');
        }
      else if (!it->data.OBJECT)
        {
          ERS_DEBUG(1, "skip (null) object in relationship \'" << name << "\' of object " << m_obj);
        }
      else
        {
          value.push_back(ConfigObject(static_cast<OksConfiguration *>(m_impl)->new_object(it->data.OBJECT)));
        }
    }
}

bool
OksConfigObject::rel(const std::string& name, std::vector<ConfigObject>& value)
{
  std::lock_guard<std::mutex> scoped_lock(m_mutex);

  try
    {
      throw_if_deleted();

      if (OksDataInfo * di = m_obj->GetClass()->get_data_info(name))
        {
          if (di->relationship)
            {
              OksData * data = m_obj->GetRelationshipValue(di);

              if (data->type == OksData::object_type)
                {
                  if (data->data.OBJECT != nullptr)
                    {
                      value.push_back(ConfigObject(static_cast<OksConfiguration *>(m_impl)->new_object(data->data.OBJECT)));
                    }
                }
              else if (data->type == OksData::list_type)
                {
                  for (const auto& it : *data->data.LIST)
                    {
                      if (it->type == OksData::object_type)
                        {
                          value.push_back(ConfigObject(static_cast<OksConfiguration *>(m_impl)->new_object(it->data.OBJECT)));
                        }
                    }
                }

              return true;
            }
        }

      return false;
    }
  catch (oks::exception& ex)
    {
      throw daq::config::Generic(ERS_HERE, mk_get_error_text("relationship", name, m_obj).c_str(), ex);
    }
}

void
OksConfigObject::referenced_by(std::vector<ConfigObject>& value, const std::string& rname, bool check_composite_only, unsigned long /* rlevel */, const std::vector<std::string> * /* rclasses */) const
{
  value.clear();

  std::lock_guard<std::mutex> scoped_lock(m_mutex);
  throw_if_deleted();

  if (check_composite_only)
    {
      bool any_name = (rname == "*");

      if (const std::list<OksRCR *> * rcr = m_obj->reverse_composite_rels())
        {
          for (const auto& i : *rcr)
            {
              if (any_name || rname == i->relationship->get_name())
                {
                  value.push_back(ConfigObject(static_cast<OksConfiguration *>(m_impl)->new_object(i->obj)));
                }
            }
        }
    }
  else
    {
      if (OksObject::FList * objs = m_obj->get_all_rels(rname))
        {
          for (const auto& i : *objs)
            {
              value.push_back(ConfigObject(static_cast<OksConfiguration *>(m_impl)->new_object(i)));
            }

          delete objs;
        }
    }
}

   /////////////////////////////////////////////////////////////////////////////
   //
   //  set functions
   //
   /////////////////////////////////////////////////////////////////////////////

void
OksConfigObject::set_attr_value(const std::string& name, OksData & data)
{
  std::lock_guard<std::mutex> scoped_lock(m_mutex);

  try
    {
      throw_if_deleted();
      //test_checkout_needs();
      m_obj->SetAttributeValue(name, &data);
    }
  catch (daq::config::Generic& ex)
    {
      throw daq::config::Generic( ERS_HERE, mk_set_error_text("attribute", name, m_obj).c_str(), ex);
    }
  catch(oks::exception& ex)
    {
      throw daq::config::Generic( ERS_HERE, mk_set_error_text("attribute", name, m_obj).c_str(), ex);
    }
}

void
OksConfigObject::set_rel_value(const std::string& name, OksData & data, bool skip_non_null_check)
{
  std::lock_guard<std::mutex> scoped_lock(m_mutex);

  try
    {
      throw_if_deleted();
      //test_checkout_needs();
      m_obj->SetRelationshipValue(name, &data, skip_non_null_check);
    }
  catch (daq::config::Generic& ex)
    {
      throw daq::config::Generic(ERS_HERE, mk_set_error_text("relationship", name, m_obj).c_str(), ex );
    }
  catch(oks::exception& ex)
    {
      throw daq::config::Generic(ERS_HERE, mk_set_error_text("relationship", name, m_obj).c_str(), ex);
    }
}


template<class T>
  void
  OksConfigObject::set_value(const std::string& name, const T& value)
  {
    OksData d(value);
    set_attr_value(name, d);
  }

template<class T>
  void
  OksConfigObject::set_vector(const std::string& name, const std::vector<T>& value)
  {
    OksData d(new OksData::List());

    for (const auto& i : value)
      {
        d.data.LIST->push_back(new OksData(i));
      }

    set_attr_value(name, d);
  }


void OksConfigObject::set(const std::string& name, bool value)
{
  set_value(name, value);
}

void OksConfigObject::set(const std::string& name, uint8_t value)
{
  set_value(name, value);
}

void OksConfigObject::set(const std::string& name, int8_t value)
{
  set_value(name, value);
}

void OksConfigObject::set(const std::string& name, uint16_t value)
{
  set_value(name, value);
}

void OksConfigObject::set(const std::string& name, int16_t value)
{
  set_value(name, value);
}

void OksConfigObject::set(const std::string& name, uint32_t value)
{
  set_value(name, value);
}

void OksConfigObject::set(const std::string& name, int32_t value)
{
  set_value(name, value);
}

void OksConfigObject::set(const std::string& name, uint64_t value)
{
  set_value(name, value);
}

void OksConfigObject::set(const std::string& name, int64_t value)
{
  set_value(name, value);
}

void OksConfigObject::set(const std::string& name, float value)
{
  set_value(name, value);
}

void OksConfigObject::set(const std::string& name, double value)
{
  set_value(name, value);
}

void OksConfigObject::set(const std::string& name, const std::string& value)
{
  set_value(name, value);
}

void
OksConfigObject::set_enum(const std::string& name, const std::string& value)
{
  OksData d;

    {
      std::lock_guard<std::mutex> scoped_lock(m_mutex);
      throw_if_deleted();

      if (OksAttribute * a = m_obj->GetClass()->find_attribute(name))
        {
          try
            {
              d.data.ENUMERATION = a->get_enum_value(value);
              d.type = OksData::enum_type;
            }
          catch (std::exception& ex)
            {
              throw(daq::config::Generic( ERS_HERE, mk_set_error_text("attribute", name, m_obj, ex.what()).c_str()) );
            }
        }
      else
        {
          throw ( daq::config::Generic( ERS_HERE, mk_set_error_text("attribute", name, m_obj, "no such attribute").c_str()) );
        }
    }

  set_attr_value(name, d);
}

void OksConfigObject::set_class(const std::string& name, const std::string& value)
{
  OksData d;

  d.data.CLASS = nullptr;
  d.type = OksData::class_type;

    // try to find any attribute in this class; throw exception "no such attribute", if the class has no attributes at all
    {
      std::lock_guard<std::mutex> scoped_lock(m_mutex);
      throw_if_deleted();

      const std::list<OksAttribute *> * attrs = m_obj->GetClass()->all_attributes();

      if (!attrs || attrs->empty())
        {
          throw daq::config::Generic(ERS_HERE, mk_set_error_text("attribute", name, m_obj, "no such attribute").c_str());
        }

      try
        {
          d.SetValue(value.c_str(), attrs->front());
        }
      catch (oks::exception & ex)
        {
          throw daq::config::Generic(ERS_HERE, mk_set_error_text("attribute", name, m_obj).c_str(), ex);
        }
    }

  set_attr_value(name, d);
}

void
OksConfigObject::set_date(const std::string& name, const std::string& value)
{
  OksData d;
  d.type = OksData::date_type;

  try
    {
      d.SetValue(value.c_str(), nullptr);
    }
  catch (oks::exception& ex)
    {
      throw daq::config::Generic(ERS_HERE, mk_set_error_text("attribute", name, m_obj).c_str(), ex);
    }

  set_attr_value(name, d);
}

void
OksConfigObject::set_time(const std::string& name, const std::string& value)
{
  OksData d;
  d.type = OksData::time_type;

  try
    {
      d.SetValue(value.c_str(), nullptr);
    }
  catch (oks::exception& ex)
    {
      throw daq::config::Generic( ERS_HERE, mk_set_error_text("attribute", name, m_obj).c_str(), ex);
    }

  set_attr_value(name, d);
}


void
OksConfigObject::set(const std::string& name, const std::vector<bool>& value)
{
  set_vector(name, value);
}

void
OksConfigObject::set(const std::string& name, const std::vector<uint8_t>& value)
{
  set_vector(name, value);
}

void
OksConfigObject::set(const std::string& name, const std::vector<int8_t>& value)
{
  set_vector(name, value);
}

void
OksConfigObject::set(const std::string& name, const std::vector<uint16_t>& value)
{
  set_vector(name, value);
}

void
OksConfigObject::set(const std::string& name, const std::vector<int16_t>& value)
{
  set_vector(name, value);
}

void
OksConfigObject::set(const std::string& name, const std::vector<uint32_t>& value)
{
  set_vector(name, value);
}

void
OksConfigObject::set(const std::string& name, const std::vector<int32_t>& value)
{
  set_vector(name, value);
}

void
OksConfigObject::set(const std::string& name, const std::vector<uint64_t>& value)
{
  set_vector(name, value);
}

void
OksConfigObject::set(const std::string& name, const std::vector<int64_t>& value)
{
  set_vector(name, value);
}

void
OksConfigObject::set(const std::string& name, const std::vector<float>& value)
{
  set_vector(name, value);
}

void
OksConfigObject::set(const std::string& name, const std::vector<double>& value)
{
  set_vector(name, value);
}

void
OksConfigObject::set(const std::string& name, const std::vector<std::string>& value)
{
  set_vector(name, value);
}

void
OksConfigObject::set_enum(const std::string& name, const std::vector<std::string>& value)
{
  OksData d(new OksData::List());

    {
      std::lock_guard<std::mutex> scoped_lock(m_mutex);
      throw_if_deleted();

      if (OksAttribute * a = m_obj->GetClass()->find_attribute(name))
        {
          for (const auto& i : value)
            {
              OksData * d2 = new OksData();
              try
                {
                  d2->data.ENUMERATION = a->get_enum_value(i); // new OksString(i);
                  d2->type = OksData::enum_type;
                  d.data.LIST->push_back(d2);
                }
              catch (std::exception& ex)
                {
                  throw daq::config::Generic( ERS_HERE, mk_set_error_text("attribute", name, m_obj).c_str(), ex);
                }

            }
        }
      else
        {
          throw daq::config::Generic( ERS_HERE, mk_set_error_text("attribute", name, m_obj, "no such attribute").c_str());
        }
    }

  set_attr_value(name, d);
}

void
OksConfigObject::set_class(const std::string& name, const std::vector<std::string>& value)
{
  OksData d(new OksData::List());

  // try to find any attribute in this class; throw exception "no such attribute", if the class has no attributes at all

    {
      std::lock_guard<std::mutex> scoped_lock(m_mutex);
      throw_if_deleted();

      const std::list<OksAttribute *> * attrs = m_obj->GetClass()->all_attributes();

      if (!attrs || attrs->empty())
        {
          throw daq::config::Generic( ERS_HERE, mk_set_error_text("attribute", name, m_obj, "no such attribute").c_str());
        }

      for (const auto& i : value)
        {
          OksData * d2 = new OksData();
          d2->data.CLASS = 0;
          d2->type = OksData::class_type;

          try
            {
              d2->SetValue(i.c_str(), attrs->front());
            }
          catch (oks::exception & ex)
            {
              throw daq::config::Generic( ERS_HERE, mk_set_error_text("attribute", name, m_obj).c_str(), ex);
            }

          d.data.LIST->push_back(d2);
        }
    }

  set_attr_value(name, d);
}

void
OksConfigObject::set_date(const std::string& name, const std::vector<std::string>& value)
{
  OksData d(new OksData::List());

  for (const auto& i : value)
    {
      OksData * d2 = new OksData();
      d2->type = OksData::date_type;
      try
        {
          d2->SetValue(i.c_str(), nullptr);
        }
      catch (oks::exception& ex)
        {
          throw daq::config::Generic( ERS_HERE, mk_set_error_text("attribute", name, m_obj).c_str(), ex);
        }
      d.data.LIST->push_back(d2);
    }

  set_attr_value(name, d);
}

void
OksConfigObject::set_time(const std::string& name, const std::vector<std::string>& value)
{
  OksData d(new OksData::List());

  for (const auto& i : value)
    {
      OksData * d2 = new OksData();
      d2->type = OksData::time_type;
      try
        {
          d2->SetValue(i.c_str(), 0);
        }
      catch (oks::exception& ex)
        {
          throw daq::config::Generic( ERS_HERE, mk_set_error_text("attribute", name, m_obj).c_str(), ex);
        }
      d.data.LIST->push_back(d2);
    }

  set_attr_value(name, d);
}

void
OksConfigObject::set(const std::string& name, const ConfigObject * value, bool skip_non_null_check)
{
  OksData d((value != nullptr) ? static_cast<const OksConfigObject*>(value->implementation())->m_obj : (OksObject *) nullptr);
  set_rel_value(name, d, skip_non_null_check);
}

void
OksConfigObject::set(const std::string& name, const std::vector<const ConfigObject*>& value, bool skip_non_null_check)
{
  OksData d(new OksData::List());

  for (const auto& i : value)
    {
      d.data.LIST->push_back(new OksData(static_cast<const OksConfigObject*>(i->implementation())->m_obj));
    }

  set_rel_value(name, d, skip_non_null_check);
}

//void OksConfigObject::test_checkout_needs()
//{
////  static_cast<OksConfiguration *>(m_impl)->test_and_checkout(m_obj->get_file());
//}

void
OksConfigObject::move(const std::string& at)
{
  std::lock_guard<std::mutex> scoped_lock(m_mutex);

  throw_if_deleted();

  if (OksFile * new_file_h = m_obj->GetClass()->get_kernel()->find_data_file(at))
    {
      OksFile * old_file_h = m_obj->get_file();
      if (old_file_h != new_file_h)
        {
          try
            {
//              static_cast<OksConfiguration *>(m_impl)->test_and_checkout(old_file_h);
//              static_cast<OksConfiguration *>(m_impl)->test_and_checkout(new_file_h);
              m_obj->set_file(new_file_h, false);
            }
          catch (oks::exception& ex)
            {
              throw daq::config::Generic(ERS_HERE, ex.what());
            }
        }
    }
  else
    {
      std::ostringstream text;
      text << "file \"" << at << "\" is not loaded";
      throw daq::config::Generic(ERS_HERE, text.str().c_str());
    }
}


void
OksConfigObject::rename(const std::string& new_id)
{
  try
    {
      throw_if_deleted();

//      std::set<OksFile *> files;
//
//      files.insert(m_obj->get_file());
//
//      if (OksObject::FList * objs = m_obj->get_all_rels())
//        {
//          for (auto & f : *objs)
//            {
//              files.insert(f->get_file());
//            }
//
//          delete objs;
//        }
//
//      for (auto & f : files)
//        {
//          static_cast<OksConfiguration *>(m_impl)->test_and_checkout(f);
//        }

      m_obj->set_id(new_id);
    }
  catch (oks::exception& ex)
    {
      std::ostringstream text;
      text << "cannot rename object " << m_obj << " to new name \'" << new_id << "\' because " << ex.what();
      throw daq::config::Generic(ERS_HERE, text.str().c_str());
    }
}

void
OksConfigObject::reset()
{
  if (OksClass * c = static_cast<OksConfiguration *>(m_impl)->m_kernel->find_class(*m_class_name))
    {
      m_obj = c->get_object(m_id);
    }
  else
    {
      m_obj = nullptr;
    }

  m_state = (m_obj ? daq::config::Valid : daq::config::Deleted);
}
