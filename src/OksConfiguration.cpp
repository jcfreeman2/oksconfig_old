#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include <algorithm>
#include <memory>

#include <ers/ers.h>

#include <oks/file.h>
#include <oks/kernel.h>
#include <oks/object.h>
#include <oks/query.h>
#include <oks/relationship.h>
#include <oks/access.h>

#include <config/Configuration.h>
#include <config/ConfigObject.h>
#include <config/Change.h>
#include <config/Schema.h>

#include "oksconfig/OksConfiguration.h"
#include "oksconfig/OksConfigObject.h"

ERS_DECLARE_ISSUE( oksconfig, Exception, , )

  // to be used as plug-in

extern "C" ConfigurationImpl * _oksconfig_creator_ (const std::string& spec) {
  try {
    std::unique_ptr<OksConfiguration> impl(new OksConfiguration());
    if(!spec.empty()) { impl->open_db(spec); }
    return impl.release();
  }
  catch(daq::config::Exception & ex) {
    throw daq::config::Generic(ERS_HERE, "oksconfig initialization error", ex);
  }
  catch(...) {
    throw daq::config::Generic(ERS_HERE, "oksconfig initialization error:\n***** caught unknown exception *****");
  }
}


struct OksConfigurationCheckDB {

  OksConfiguration * m_db;
  bool m_run;

  OksConfigurationCheckDB(OksConfiguration * db) : m_db(db), m_run(true) { ; }

  ~OksConfigurationCheckDB() {
    ERS_DEBUG( 3 , "Call destructor of OksConfigurationCheckDB object" );
    m_db = nullptr;
  }

  void
  operator()()
  {
    ERS_DEBUG(2, "Call user notification");

    while (m_run)
      {
        sleep(1);
        try
          {
            m_db->check_db();
          }
        catch (daq::config::Generic& ex)
          {
            m_db->m_check_db_thread = nullptr;
            m_db->m_check_db_obj = nullptr;

            if (getenv("OKSCONFIG_NO_RELOAD_ABORT") != nullptr)
              {
                ers::fatal(daq::config::Generic( ERS_HERE, "database reload has failed, unsubscribing...", ex ) );
                return;
              }
            else
              {
                ers::fatal ( daq::config::Generic( ERS_HERE, "database reload has failed, aborting...", ex ) );
                abort();
              }
          }
      }

    ERS_DEBUG( 4 , "Destroy OksConfigurationCheckDB object = " << (void *)this );

    delete this;

    ERS_DEBUG( 2 , "Exit user notification" );
  }

};


bool
is_repo_name(const std::string& name)
{
  if (!OksKernel::get_repository_root().empty())
    {
      std::string s(name);

      Oks::substitute_variables(s);

      bool is_absolute_path = (s[0] == '/');

      if (!is_absolute_path)
        s = std::string(OksKernel::get_cwd()) + '/' + s;

      // if file system path exists
      if (Oks::real_path(s, true))
        {
          if (!OksKernel::get_repository_mapping_dir().empty() && s.find(OksKernel::get_repository_mapping_dir()) == 0)
            return true;

          static std::string user_repo_dir;
          static std::once_flag flag;

          std::call_once(flag, []()
            {
              if (const char * s = getenv("TDAQ_DB_USER_REPOSITORY"))
                {
                  user_repo_dir = s;

                  try
                    {
                      Oks::real_path(user_repo_dir, false);
                    }
                  catch(oks::exception& ex)
                    {
                      ERS_DEBUG( 0, "Failed to read TDAQ_DB_USER_REPOSITORY = \'" << s << "\':\n\tcaused by: " << ex.what() );
                    }
                }
            }
          );

          if (!user_repo_dir.empty() && s.find(user_repo_dir) == 0)
            return true;

          return false;
        }
      else
        return true;
    }

  return false;
}

void
OksConfiguration::open_db(const std::string& spec_params)
{
    // separate parameters first

  std::string::size_type idx = spec_params.find_first_of('&');

  std::string data, params;

  if (idx == std::string::npos)
    {
      data = spec_params;
    }
  else
    {
      data = spec_params.substr(0, idx);
      params = spec_params.substr(idx + 1);
    }

  std::string token;

  // guess about oks kernel mode based on the file names

    {
      Oks::Tokenizer t(data, ":");

      while (!(token = t.next()).empty())
        {
          if (is_repo_name(token) == false)
            m_oks_kernel_no_repo = true;
        }
    }

    // create OKS kernel if it was not created

  if (m_kernel == nullptr)
    {
      if (!params.empty())
        {
          Oks::Tokenizer t(params, ";");

          while (!(token = t.next()).empty())
            {
              if(token == "norepo")
                m_oks_kernel_no_repo = true;
            }
        }

      m_kernel = new OksKernel(m_oks_kernel_silence, false, false, !m_oks_kernel_no_repo);
    }

  Oks::Tokenizer t(data, ":");

  while(!(token = t.next()).empty()) {
    try {
      m_kernel->load_file(token);
    }
    catch (oks::exception & e) {
      std::ostringstream text;
      text << "cannot load file \'" << token << "\':\n" << e.what();
      close_db();
      throw ( daq::config::Generic( ERS_HERE, text.str().c_str() ) );
    }
    catch (...) {
      std::ostringstream text;
      text << "cannot load file \'" << token << '\'';
      close_db();
      throw ( daq::config::Generic( ERS_HERE, text.str().c_str() ) );
    }
  }
}

void
OksConfiguration::close_database(bool call_unsubscribe)
{
  clean(); // clean implementation cache

  if(m_kernel) try {
    if(m_check_db_obj) {
      if(call_unsubscribe)
        unsubscribe();
      else
        m_check_db_obj->m_run = false;
    }

    m_kernel->subscribe_create_object(0, 0);
    m_kernel->subscribe_change_object(0, 0);
    m_kernel->subscribe_delete_object(0, 0);

    m_kernel->close_all_data();
    m_kernel->close_all_schema();

    delete m_kernel;
    m_kernel = 0;
  }
  catch(oksconfig::Exception& ex) {
    throw ( daq::config::Generic( ERS_HERE, "failed to close database", ex ) );
  }
  catch(std::exception& ex) {
    throw ( daq::config::Generic( ERS_HERE, "failed to close database", ex ) );
  }
}


void
OksConfiguration::create(const std::string& db_name, const std::list<std::string>& includes)
{
    // create OKS kernel if it was not created

  if (m_kernel == nullptr)
    {
      if (is_repo_name(db_name) == false)
        m_oks_kernel_no_repo = true;

      m_kernel = new OksKernel(m_oks_kernel_silence, false, false, !m_oks_kernel_no_repo);
    }


    // create new data file

  try {
    OksFile * h = m_kernel->new_data(db_name);
    m_created_files.insert(h);

      // add includes

    for(std::list<std::string>::const_iterator i = includes.begin(); i != includes.end(); ++i) {
      try {
        h->add_include_file(*i);
      }
      catch (oks::exception & e) {
        std::ostringstream text;
        text << "cannot add and load include file \'" << *i << "\' to \'" << db_name << "\':\n" << e.what();
        close_db();
        throw ( daq::config::Generic( ERS_HERE, text.str().c_str() ) );
      }
      catch (...) {
        std::ostringstream text;
        text << "cannot add and load include file \'" << *i << "\' to \'" << db_name << '\'';
        close_db();
        throw ( daq::config::Generic( ERS_HERE, text.str().c_str() ) );
      }
    }

    m_kernel->bind_objects();
  }
  catch (oks::exception & ex) {
    close_db();
    std::ostringstream text;
    text << "cannot create new data file \'" << db_name << "\': " << ex.what();
    throw ( daq::config::Generic( ERS_HERE, text.str().c_str() ) );
  }
  catch (daq::config::Generic & ex) {
    close_db();
    std::ostringstream text;
    text << "cannot create new data file \'" << db_name << "\'";
    throw ( daq::config::Generic( ERS_HERE, text.str().c_str(), ex ) );
  }
  catch (...) {
    close_db();
    std::ostringstream text;
    text << "cannot create new data file \'" << db_name << '\'';
    throw ( daq::config::Generic( ERS_HERE, text.str().c_str() ) );
  }
}

static std::string
mk_no_file_error_text(const std::string &db)
{
  std::ostringstream text;
  text << "cannot find file \'" << db << '\'';
  return text.str();
}

static std::string
mk_add_include_error_text(const std::string &db, const std::string &include, const char *error = nullptr)
{
  std::ostringstream text;
  text << "cannot add and load include file \'" << include << "\' to \'" << db << '\'';
  if (error)
      text << ":\n" << error;
  return text.str();
}


void
OksConfiguration::get_updated_dbs(std::list<std::string>& dbs) const
{
  for (const auto& i : m_kernel->data_files())
    {
      if (i.second->is_updated())
        dbs.push_back(i.second->get_full_file_name());
    }
}


void
OksConfiguration::set_commit_credentials(const std::string& user, const std::string& password)
{
  try {
    OksKernel::validate_credentials(user.c_str(), password.c_str());
  }
  catch(oks::exception& ex) {
    throw daq::config::Generic( ERS_HERE, "authentication failed", ex );
  }

  m_user = user;
  m_password = password;
}

static void throw_update_exception(const OksFile * file_h, const char * action, const char * reason)
{
  std::ostringstream text;
  text << "cannot " << action << " file \'" << file_h->get_full_file_name() << "\' since " << reason;
  throw daq::config::Generic( ERS_HERE, text.str().c_str() );
}

void
OksConfiguration::commit(const std::string& log_message)
{
  try
    {
      m_kernel->set_active_data(nullptr);
    }
  catch (oks::exception &ex)
    {
      throw(daq::config::Generic( ERS_HERE, ex.what()));
    }

  const bool commit2repo(!m_kernel->get_user_repository_root().empty() && m_kernel->is_user_repository_created());
  unsigned int count(0);

  for (const auto &i : m_kernel->data_files())
    {
      OksFile *file_h = i.second;

      if (file_h->is_updated())
        {
          const OksFile::FileStatus file_status = file_h->get_status_of_file();

          if (file_status == OksFile::FileModified)
            throw_update_exception(file_h, "save", "it was externally modified");
          else if (file_status == OksFile::FileRemoved)
            throw_update_exception(file_h, "save", "it was externally removed");

          try
            {
              if (!commit2repo && !log_message.empty())
                file_h->add_comment(log_message, m_kernel->get_user_name());

              m_kernel->save_data(file_h);

              count++;
            }
          catch (oks::exception &ex)
            {
              std::ostringstream text;
              text << "cannot save updated data file \'" << *(i.first) << "\':\n" << ex.what();
              throw(daq::config::Generic( ERS_HERE, text.str().c_str()));
            }
        }
    }

  if (commit2repo && count)
    {
      try
        {
          m_kernel->commit_repository(log_message);
        }
      catch (oks::exception &ex)
        {
          throw(daq::config::Generic( ERS_HERE, ex.what()));
        }
    }

  m_created_files.clear();
}

class ResubscribeGuard {

  public:

    ResubscribeGuard(OksConfiguration & db) : m_db(db) {
      if(m_db.m_check_db_obj) {
        m_db.unsubscribe();
	m_restart = true;
      }
      else {
	m_restart = false;
      }

      m_db.m_created.clear();
      m_db.m_modified.clear();
      m_db.m_removed.clear();
    }

    ~ResubscribeGuard() {
      if(m_restart) {
        m_db.subscribe();
      }
    }

  private:

   OksConfiguration& m_db;
   bool m_restart;

};


void
OksConfiguration::abort()
{
    // unsubscribe changes; if there are any, re-subscribe on return from this method

  ResubscribeGuard __rg__(*this);


    // destroy any newly created files
  for (const auto& i : m_created_files)
    {
      const std::string file_name(i->get_full_file_name());

      ERS_DEBUG(1, "destroy created file \'" << file_name << "\')");
      m_kernel->close_data(i);

      if (int result = unlink(file_name.c_str()))
        {
          std::ostringstream text;
          text << "abort changes failed since cannot erase created file \'" << file_name << "\'; unlink failed with code " << result << ": " << strerror(errno);
          throw(daq::config::Generic( ERS_HERE, text.str().c_str() ) );
        }
    }

  m_created_files.clear();



  // add to "updated" any modified file
  std::set<OksFile *> updated;

  for (const auto& i : m_kernel->data_files())
    {
      if (i.second->is_updated())
        {
          updated.insert(i.second);
          ERS_DEBUG(2, "file \'" << i.second->get_full_file_name() << "\' was updated, to be reloaded...");
        }
    }

  if (!m_kernel->get_user_repository_root().empty())
    {
      std::set<OksFile *> mfs, rfs;
      try
        {
          m_kernel->get_modified_files(mfs, rfs, "origin/master");
        }
      catch (const oks::exception& ex)
        {
          std::ostringstream text;
          text << "failed to get differences between revisions " << m_kernel->get_repository_version() << " and origin/master: " << ex.what() << '\n';
          throw(daq::config::Generic(ERS_HERE, text.str().c_str(), ex));
        }
    }

  if (!updated.empty())
    {
      try
        {
          if (!m_kernel->get_user_repository_root().empty())
            m_kernel->update_repository("origin/master", OksKernel::RepositoryUpdateType::DiscardChanges);

          m_kernel->reload_data(updated);
        }
      catch (oks::exception & ex)
        {
          throw(daq::config::Generic(ERS_HERE, "cannot reload updated data files", ex));
        }
      catch (...)
        {
          throw(daq::config::Generic(ERS_HERE, "cannot reload updated data files"));
        }
    }
}


static std::vector<daq::config::Version>
oks2config(const std::vector<OksRepositoryVersion>& in)
{
  std::vector<daq::config::Version> out;
  out.reserve(in.size());

  for (auto& x : in)
    out.emplace_back(x.m_commit_hash, x.m_user, x.m_date, x.m_comment, x.m_files);

  return out;
}

std::vector<daq::config::Version>
OksConfiguration::get_changes()
{
  if (!m_kernel->get_repository_root().empty())
    {
      try
        {
          return oks2config(m_kernel->get_repository_versions_by_hash(true, m_kernel->get_repository_version()));
        }
      catch (const oks::exception& ex)
        {
          throw(daq::config::Generic(ERS_HERE, "cannot get new versions", ex));
        }
    }
  else
    {
      std::vector<daq::config::Version> out;

      std::set<OksFile *> oks_files;
      m_kernel->get_modified_files(oks_files, oks_files, "");

      if (!oks_files.empty())
        {
          std::vector<std::string> files;

          for(const auto& x : oks_files)
          files.push_back(x->get_well_formed_name());

          out.emplace_back("", "", 0, "", files);
        }

      return out;
    }
}


std::vector<daq::config::Version>
OksConfiguration::get_versions(const std::string& since, const std::string& until, daq::config::Version::QueryType type, bool skip_irrelevant)
{
  if (!m_kernel->get_repository_root().empty())
    {
      try
        {
          return oks2config(type == daq::config::Version::query_by_date ? m_kernel->get_repository_versions_by_date(skip_irrelevant, since, until) : m_kernel->get_repository_versions_by_hash(skip_irrelevant, since, until));
        }
      catch (const oks::exception& ex)
        {
          throw(daq::config::Generic(ERS_HERE, "cannot get repository versions", ex));
        }
    }
  else
    {
      throw(daq::config::Generic(ERS_HERE, "cannot get versions, repository is not set"));
    }
}


OksConfigObject *
OksConfiguration::new_object(OksObject * obj) noexcept
{
  return insert_object<OksConfigObject>(obj, obj->GetId(), obj->GetClass()->get_name());
}

bool
OksConfiguration::test_object(const std::string& class_name, const std::string& name, unsigned long /*rlevel*/, const std::vector<std::string> * /*rclasses*/)
{
  // Looking for the class. If it does not exist, we return false.
  OksClass * cl = m_kernel->find_class(class_name);
  if (!cl)
    return false;

  // The object exist for this class?
  if (cl->get_object(name))
    return true;

  // Trying all subclasses
  if (const OksClass::FList * subclasses = cl->all_sub_classes())
    {
      for (OksClass::FList::const_iterator it = subclasses->begin(); it != subclasses->end(); ++it)
        {
          if ((*it)->get_object(name))
            return true;
        }
    }
  // If we get here, we found the class, but not the object.
  return false;
}

void
OksConfiguration::get(const std::string &class_name, const std::string &name, ConfigObject &object, unsigned long /*rlevel*/, const std::vector<std::string>* /* rclasses */)
{
  if (OksClass * cl = m_kernel->find_class(class_name))
    {
      OksObject * obj = cl->get_object(name);
      if (obj == nullptr)
        {
          // try all subclasses
          if (const OksClass::FList *subclasses = cl->all_sub_classes())
            {
              for (const auto& it : *subclasses)
                {
                  obj = it->get_object(name);
                  if (obj != nullptr)
                    break;
                }
            }
          if (obj == nullptr)
            {
              const std::string id(name + '@' + class_name);
              throw daq::config::NotFound(ERS_HERE, "object", id.c_str());
            }
        }

      object = new_object(obj);
    }
  else
    {
      throw daq::config::NotFound(ERS_HERE, "class", class_name.c_str());
    }
}

void
OksConfiguration::get(const std::string& class_name, std::vector<ConfigObject>& objects, const std::string& query, unsigned long /*rlevel*/, const std::vector<std::string> * /* rclasses */)
{
  if(OksClass * cl = m_kernel->find_class(class_name)) {
    objects.clear();

    OksObject::List * objs = 0;

    if(query.empty()) {
      objs = cl->create_list_of_all_objects();
    }
    else {
      try {
        std::unique_ptr<OksQuery> qe(new OksQuery(cl, query.c_str()));
        if(qe->good() == false) {
          std::ostringstream text;
          text << "bad query syntax \"" << query << "\" in scope of class \"" << class_name << '\"';
          throw daq::config::Generic( ERS_HERE, text.str().c_str());
        }
        objs = cl->execute_query(qe.get());  // FIXME: check .get()
      }
      catch(oks::exception& ex) {
        std::ostringstream text;
        text << "failed to execute query:\n" << ex.what();
        throw daq::config::Generic(ERS_HERE, text.str().c_str());
      }
    }

    if(objs) {
      for(OksObject::List::iterator i = objs->begin(); i != objs->end(); ++i) {
        objects.push_back(new_object(*i));
      }
      delete objs;
    }
  }
  else {
    throw daq::config::NotFound(ERS_HERE, "class", class_name.c_str());
  }
}

void
OksConfiguration::get(const ConfigObject& obj_from, const std::string& query, std::vector<ConfigObject>& objects, unsigned long /*rlevel*/, const std::vector<std::string> * /* rclasses */)
{
  if(obj_from.is_null()) {
    throw ( daq::config::Generic( ERS_HERE, "parameter \'obj_from\' is (null)" ) );
  }

  OksObject * o = (static_cast<const OksConfigObject *>(obj_from.implementation()))->m_obj;

  try {
    oks::QueryPath q(query, *m_kernel);
    if(OksObject::List * objs = o->find_path(q)) {
      for(OksObject::List::iterator i = objs->begin(); i != objs->end(); ++i) {
        objects.push_back(new_object(*i));
      }
      delete objs;
    }
  }
  catch ( oks::bad_query_syntax& e ) {
    std::ostringstream text;
    text << "bad path-query \"" << query << "\" to object " << o;
    throw ( daq::config::Generic( ERS_HERE, text.str().c_str() ) );
  }
}


daq::config::class_t *
OksConfiguration::get(const std::string& class_name, bool direct_only)
{
  OksClass * c = m_kernel->find_class(class_name);

  if(c == 0) {
    throw daq::config::NotFound(ERS_HERE, "class", class_name.c_str());
  }

  daq::config::class_t * d = new daq::config::class_t(c->get_name(), c->get_description(), c->get_is_abstract());

  if(direct_only) {
    if(const std::list<std::string *> * classes = c->direct_super_classes()) {
      for(const auto& i : *classes) {
        const_cast< std::vector<std::string>& >(d->p_superclasses).push_back(*i);
      }
    }
  }
  else {
    if(const OksClass::FList * classes = c->all_super_classes()) {
      for(const auto& i : *classes) {
        const_cast< std::vector<std::string>& >(d->p_superclasses).push_back(i->get_name());
      }
    }
  }

  if(const OksClass::FList * subclasses = c->all_sub_classes()) {
    for(const auto& i : *subclasses) {
      if(direct_only == false || i->has_direct_super_class(c->get_name()) == true) {
        const_cast< std::vector<std::string>& >(d->p_subclasses).push_back(i->get_name());
      }
    }
  }

  if(const std::list<OksAttribute *> * attrs = (direct_only ? c->direct_attributes() : c->all_attributes())) {
    for(const auto& i : *attrs) {
      OksAttribute::Format format = i->get_format();
      OksData::Type type = i->get_data_type();

      const_cast< std::vector<daq::config::attribute_t>& >(d->p_attributes).push_back(
        daq::config::attribute_t(
	  i->get_name(),
          (
	    type == OksData::string_type  ? daq::config::string_type :
	    type == OksData::enum_type    ? daq::config::enum_type   :
	    type == OksData::bool_type    ? daq::config::bool_type   :
	    type == OksData::s8_int_type  ? daq::config::s8_type     :
	    type == OksData::u8_int_type  ? daq::config::u8_type     :
	    type == OksData::s16_int_type ? daq::config::s16_type    :
	    type == OksData::u16_int_type ? daq::config::u16_type    :
	    type == OksData::s32_int_type ? daq::config::s32_type    :
	    type == OksData::u32_int_type ? daq::config::u32_type    :
	    type == OksData::s64_int_type ? daq::config::s64_type    :
	    type == OksData::u64_int_type ? daq::config::u64_type    :
	    type == OksData::float_type   ? daq::config::float_type  :
	    type == OksData::double_type  ? daq::config::double_type :
	    type == OksData::date_type    ? daq::config::date_type   :
	    type == OksData::time_type    ? daq::config::time_type   :
	    daq::config::class_type
	  ),
          i->get_range(),
	  (
	    i->is_integer() == false ? daq::config::na_int_format :
	      (
	        format == OksAttribute::Dec ? daq::config::dec_int_format :
	        format == OksAttribute::Hex ? daq::config::hex_int_format :
	        daq::config::oct_int_format
	      )
	  ),
          i->get_is_no_null(),
	  i->get_is_multi_values(),
          i->get_init_value(),
	  i->get_description()
	)
      );

    }
  }

  if(const std::list<OksRelationship *> * rels = (direct_only ? c->direct_relationships() : c->all_relationships())) {
    for(const auto& i : *rels) {
      const_cast< std::vector<daq::config::relationship_t>& >(d->p_relationships).push_back(
        daq::config::relationship_t(
          i->get_name(),
          i->get_type(),
          (i->get_low_cardinality_constraint() == OksRelationship::Zero),
          (i->get_high_cardinality_constraint() == OksRelationship::Many),
          i->get_is_composite(),
          i->get_description()
        )
      );
    }
  }

  return d;
}

void
OksConfiguration::get_superclasses(config::fmap<config::fset>& schema)
{
  schema.clear();

  if(!m_kernel) {
    return; // throw ( daq::config::Generic( ERS_HERE, "database is not loaded" ) );
  }

  schema.reserve(m_kernel->classes().size() * 3);

  for(OksClass::Map::const_iterator i = m_kernel->classes().begin(); i != m_kernel->classes().end(); ++i) {
    if(const OksClass::FList * scl = i->second->all_super_classes()) {
      const std::string* name = &DalFactory::instance().get_known_class_name_ref(i->second->get_name());

      auto& subclasses = schema[name];

      if(!scl->empty()) {
        subclasses.reserve(scl->size() * 3);
        for(OksClass::FList::const_iterator j = scl->begin(); j != scl->end(); ++j) {
          subclasses.insert(&DalFactory::instance().get_known_class_name_ref((*j)->get_name()));
        }
      }
    }
  }
}


void
OksConfiguration::create(OksFile * at, const std::string& class_name, const std::string& id, ConfigObject& object)
{
  try {
    m_kernel->set_active_data(at);
  }
  catch(oks::exception& ex) {
    throw ( daq::config::Generic( ERS_HERE, ex.what()) );
  }

  OksClass * c = m_kernel->find_class(class_name);

  if(c == nullptr) {
    std::ostringstream text;
    text << "cannot find class \"" << class_name << '\"';
    throw ( daq::config::Generic( ERS_HERE, text.str().c_str() ) );
  }
  else if(id.empty() == false) {
    if(OksObject * obj = c->get_object(id)) {
      std::ostringstream text;
      text << "object \"" << id << '@' << class_name << "\" already exists in file \"" << obj->get_file()->get_full_file_name() << '\"';
      throw ( daq::config::Generic( ERS_HERE, text.str().c_str() ) );
    }
  }

  if(m_kernel->get_test_duplicated_objects_via_inheritance_mode() == false)
    {
      m_kernel->set_test_duplicated_objects_via_inheritance_mode(true);
      m_kernel->registrate_all_classes();
    }

  try {
    object = new_object(new OksObject(c, id.c_str()));
  }
  catch(oks::exception& ex)
  {
    throw ( daq::config::Generic( ERS_HERE, "cannot create configuration object", ex ) );
  }
}

void
OksConfiguration::create(const std::string& at, const std::string& class_name, const std::string& id, ::ConfigObject& object)
{
  if(at.empty() == true) {
    throw ( daq::config::Generic( ERS_HERE, "parameter \'at\' (i.e. filename) cannot be empty" ) );
  }

  if(OksFile * h = m_kernel->find_data_file(at)) {
    return create(h, class_name, id, object);
  }
  else {
    std::ostringstream text;
    text << "file \"" << at << "\" is not loaded";
    throw ( daq::config::Generic( ERS_HERE, text.str().c_str() ) );
  }
}

void
OksConfiguration::create(const ConfigObject& at, const std::string& class_name, const std::string& id, ConfigObject& object)
{
  create((static_cast<const OksConfigObject *>(at.implementation()))->m_obj->get_file(), class_name, id, object);
}

void
OksConfiguration::create_notify(OksObject * o, void * p) noexcept
{
  reinterpret_cast<OksConfiguration *>(p)->m_created.push_back(o);
}

void
OksConfiguration::change_notify(OksObject * o, void * p) noexcept
{
  reinterpret_cast<OksConfiguration *>(p)->m_modified.insert(o);
}

void
OksConfiguration::delete_notify(OksObject * o, void * p) noexcept
{
  reinterpret_cast<OksConfiguration *>(p)->m_removed[o->GetClass()->get_name()].insert(o->GetId());
}


struct InheritanceData {

  InheritanceData(const OksKernel&);

  const OksConfiguration::SMap& data() const { return m_data; }

  OksConfiguration::SMap m_data;

};

InheritanceData::InheritanceData(const OksKernel &kernel)
{
  for (OksClass::Map::const_iterator i = kernel.classes().begin(); i != kernel.classes().end(); ++i)
    if (const OksClass::FList *slist = i->second->all_super_classes())
      {
        std::set<std::string> &data(m_data[i->second->get_name()]);
        for (const auto &j : *slist)
          data.insert(j->get_name());
      }
}


  /**
   *  The function checks if the object satisfies the subscription
   *  and adds to changes.
   *  The actions could be:
   *   '+' - created
   *   '~' - changed
   *   '-' - deleted
   */

static void
check(std::vector<ConfigurationChange *>& changes,
      const InheritanceData & inheritance,
      const std::set<std::string>& class_names,
      const OksConfiguration::SMap & objects,
      const std::string& obj_class,
      const std::string& obj_id,
      const char action
) {

  bool any = (class_names.empty() && objects.empty());


    // check subscription on objects
    // 
    // omi iterator in non-null, if on an object of class 'obj_class'
    // there is explicit subscription

  OksConfiguration::SMap::const_iterator omi = objects.end();

  if(action != '+') {
    omi = objects.find(obj_class);
    if(omi != objects.end()) {
      std::set<std::string>::const_iterator i = omi->second.find(obj_id);

      if(i != omi->second.end()) {
        ConfigurationChange::add(changes, obj_class, obj_id, action);
      }
    }
  }


    // process the class

  if(action == '+' || omi == objects.end()) {
    if(any || class_names.find(obj_class) != class_names.end()) {
      ConfigurationChange::add(changes, obj_class, obj_id, action);
    }
  }


    // process subclasses

  OksConfiguration::SMap::const_iterator it = inheritance.data().find(obj_class);
  if(it != inheritance.data().end()) {
    for(std::set<std::string>::const_iterator j = it->second.begin(); j != it->second.end(); ++j) {
      omi = (action != '+' ? objects.find(*j) : objects.end());

        // 1. add objects if criteria is 'any'
        // 2. add object of class which has subscription and has no explicit object subscription
        // 3. add explicitly subscribed object

      if(
	any ||
	(
	  omi == objects.end() &&
	  class_names.find(*j) != class_names.end()
	) ||
	(
	  omi != objects.end() &&
	  omi->second.find(obj_id) != omi->second.end()
	)
      ) {
        ConfigurationChange::add(changes, *j, obj_id, action);
      }
    }
  }
}


class DestroyGuard1 {

  public:

    DestroyGuard1(OksKernel & kernel) : m_kernel(kernel) { ; }

    ~DestroyGuard1() {
      m_kernel.subscribe_delete_object(0, 0);
    }

  private:

    OksKernel& m_kernel;

};

class DestroyGuard2 {

  public:

    DestroyGuard2(OksConfiguration::SMap& removed) : m_removed(removed) { ; }

    ~DestroyGuard2() {
      m_removed.clear();
    }

  private:

    OksConfiguration::SMap& m_removed;

};


void
OksConfiguration::destroy(::ConfigObject& obj)
{
  OksObject * o((static_cast<const OksConfigObject *>(obj.implementation()))->m_obj);

  DestroyGuard2 dg2(m_removed);  // => on exit from method, call m_removed.clear();

  InheritanceData inheritance(*m_kernel);

  try {
    m_kernel->subscribe_delete_object(delete_notify, reinterpret_cast<void *>(this));
    DestroyGuard1 dg1(*m_kernel); // => on exit from try block, call m_kernel->subscribe_delete_object(0, 0);
    OksObject::destroy(o);
  }
  catch(oks::exception& ex) {
    throw ( daq::config::Generic( ERS_HERE, ex.what()) );
  }

  if(m_conf) {
    std::vector<ConfigurationChange *> changes;

    for(SMap::iterator i = m_removed.begin(); i != m_removed.end(); ++i) {
      for(std::set<std::string>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
        check(changes, inheritance, m_class_names, m_objects, i->first, *j, '-');
      }
    }

    if(!changes.empty()) {
      m_conf->update_cache(changes);
      ConfigurationChange::clear(changes);
    }
  }
}

bool
OksConfiguration::is_writable(const std::string& db_name)
{
  if (OksFile * h = m_kernel->find_data_file(db_name))
    {
      try
        {
          if (!m_kernel->get_user_repository_root().empty())
            {
              return oks::access::is_writable(*h, OksKernel::get_user_name());
            }
          else
            {
              return !OksKernel::check_read_only(h);
            }
        }
      catch (const std::exception& ex)
        {
          std::ostringstream text;
          text << "cannot check access status of file \'" << db_name << "\': " << ex.what();
          throw ( daq::config::Generic( ERS_HERE, text.str().c_str() ) );
        }
    }
  else
    {
      throw ( daq::config::Generic( ERS_HERE, mk_no_file_error_text(db_name).c_str() ) );
    }
}

static std::string
mk_rm_include_error_text(const std::string &db, const std::string &include, const char *error = 0)
{
  std::ostringstream text;
  text << "cannot remove include file \'" << include << "\' from \'" << db << '\'';
  if (error)
    text << ":\n" << error;
  return text.str();
}

void
OksConfiguration::add_include(const std::string& db_name, const std::string& include)
{
  if(OksFile * h = m_kernel->find_data_file(db_name)) {
    try {
      h->add_include_file(include);
    }
    catch(daq::config::Generic& ex) {
      throw ( daq::config::Generic( ERS_HERE, mk_add_include_error_text(db_name, include).c_str() ), ex );
    }
    catch (oks::exception & ex) {
      throw ( daq::config::Generic( ERS_HERE, mk_add_include_error_text(db_name, include, ex.what()).c_str() ) );
    }
    catch (...) {
      throw ( daq::config::Generic( ERS_HERE, mk_add_include_error_text(db_name, include).c_str() ) );
    }
  }
  else {
    throw ( daq::config::Generic( ERS_HERE, mk_no_file_error_text(db_name).c_str() ) );
  }
}

void
OksConfiguration::remove_include(const std::string& db_name, const std::string& include)
{
  InheritanceData inheritance(*m_kernel);

  DestroyGuard2 dg2(m_removed);  // => on exit from method, call m_removed.clear();

  if(OksFile * h = m_kernel->find_data_file(db_name)) {
    try {
      m_kernel->subscribe_delete_object(delete_notify, reinterpret_cast<void *>(this));
      DestroyGuard1 dg1(*m_kernel); // => on exit from try block, call m_kernel->subscribe_delete_object(0, 0);
      h->remove_include_file(include);
    }
    catch(daq::config::Generic& ex) {
      throw ( daq::config::Generic( ERS_HERE, mk_rm_include_error_text(db_name, include).c_str() ), ex );
    }
    catch (oks::exception & ex) {
      throw ( daq::config::Generic( ERS_HERE, mk_rm_include_error_text(db_name, include, ex.what()).c_str() ) );
    }
    catch (...) {
      throw ( daq::config::Generic( ERS_HERE, mk_rm_include_error_text(db_name, include).c_str() ) );
    }
  }
  else {
    throw ( daq::config::Generic( ERS_HERE, mk_no_file_error_text(db_name).c_str() ) );
  }

  std::vector<ConfigurationChange *> changes;

  for(SMap::iterator i = m_removed.begin(); i != m_removed.end(); ++i) {
    for(std::set<std::string>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
      check(changes, inheritance, m_class_names, m_objects, i->first, *j, '-');
    }
  }

  if(!changes.empty()) {
    m_conf->update_cache(changes);
    ConfigurationChange::clear(changes);
  }
}


void
OksConfiguration::get_includes(const std::string& db_name, std::list<std::string>& includes) const
{
  if (db_name.empty())
    {
      for (auto& i : m_kernel->data_files())
        {
          if(i.second->get_parent() == nullptr)
            includes.push_back(*i.first);
        }
    }
  else
    {
      if (OksFile * h = m_kernel->find_data_file(db_name))
        {
          for (auto& i : h->get_include_files())
            {
              includes.push_back(i);
            }
        }
      else
        {
          throw(daq::config::Generic( ERS_HERE, mk_no_file_error_text(db_name).c_str() ) );
        }
    }
}


void
OksConfiguration::check_db()
{
  static const std::string version("origin/master");

  if (m_fn)
    {
      std::set<OksFile *> ufs, rfs;

      try
        {
          m_kernel->get_modified_files(ufs, rfs, version);
          m_repo_error_count = 0;
        }
      catch(const oks::RepositoryOperationFailed& ex)
        {
          std::ostringstream text;
          text << "cannot get modified repository files (attempt " << ++m_repo_error_count << "): " << ex.what();
          ers::error(daq::config::Generic( ERS_HERE, text.str().c_str() ) );
        }
      catch(const oks::exception& ex)
        {
          std::ostringstream text;
          text << "cannot get modified files: " << ex.what();
          throw(daq::config::Generic( ERS_HERE, text.str().c_str() ) );
        }

      for (const auto& x : ufs)
        {
          if (x->get_oks_format() == "schema")
            {
              std::ostringstream text;
              text << "reload of schema files is not supported (\'" << x->get_well_formed_name() << "\')";
              throw(daq::config::Generic( ERS_HERE, text.str().c_str() ) );
            }
        }

      if (ufs.empty() == false)
        {
          InheritanceData inheritance(*m_kernel);

          m_kernel->subscribe_create_object(create_notify, reinterpret_cast<void *>(this));
          m_kernel->subscribe_change_object(change_notify, reinterpret_cast<void *>(this));
          m_kernel->subscribe_delete_object(delete_notify, reinterpret_cast<void *>(this));

          (*m_pre_fn)(m_conf);

          try
            {
              if(!m_kernel->get_user_repository_root().empty())
                m_kernel->update_repository(version, OksKernel::RepositoryUpdateType::DiscardChanges);

              m_kernel->reload_data(ufs, false);
            }
          catch (oks::exception & ex)
            {
              close_database(false);
              m_fn = 0;
              std::ostringstream text;
              text << "failed to reload updated files:\n" << ex.what();
              throw(daq::config::Generic( ERS_HERE, text.str().c_str() ) );
            }
          catch (...)
            {
              close_database(false);
              m_fn = 0;
              throw ( daq::config::Generic( ERS_HERE, "failed to reload updated files" ) );
            }


          m_kernel->subscribe_create_object(0, 0);
          m_kernel->subscribe_change_object(0, 0);
          m_kernel->subscribe_delete_object(0, 0);

          std::vector<ConfigurationChange *> changes;

            {
              for (auto& i : m_created)
                {
                  check(changes, inheritance, m_class_names, m_objects, i->GetClass()->get_name(), i->GetId(), '+');

                  std::set<OksObject *>::iterator j = m_modified.find(i);
                  if (j != m_modified.end())
                    {
                      ERS_DEBUG(1, "created object " << i << " appears in \'modified\' set, removing...");
                      m_modified.erase(j);
                    }
                }
            }

            {
              for (auto& i : m_modified)
                {
                  if (m_kernel->is_dangling(i))
                    {
                      ERS_DEBUG(1, "object " << (void *)(i) << " is dangling, ignore...");
                    }
                  else
                    {
                      check(changes, inheritance, m_class_names, m_objects, i->GetClass()->get_name(), i->GetId(), '~');
                    }
                }
            }

            {
              for (auto& i : m_removed)
                {
                  for (auto& j : i.second)
                    {
                      check(changes, inheritance, m_class_names, m_objects, i.first, j, '-');
                    }
                }
            }

          if (!changes.empty())
            {
              (*m_fn)(changes, m_conf);
              ConfigurationChange::clear(changes);
            }

          m_created.clear();
          m_modified.clear();
          m_removed.clear();
        }
    }
  else
    {
      throw(daq::config::Generic( ERS_HERE, "no subscription has been done" ) );
    }
}


  //
  // Subscribe on changes by class name
  //

void
OksConfiguration::subscribe(const std::set<std::string>& class_names,
                            const SMap& objs,
			    ConfigurationImpl::notify cb,
			    ConfigurationImpl::pre_notify pre_cb)
{
  m_fn = cb;
  m_pre_fn = pre_cb;
  m_class_names = class_names;
  m_objects = objs;

  subscribe();
}

  // Start subscription thread

void
OksConfiguration::subscribe()
{
  if(m_check_db_obj == 0) {
    ERS_DEBUG( 2, "starting CheckDB thread ..." );
    m_check_db_obj = new OksConfigurationCheckDB(this);
    m_check_db_thread = new std::thread(std::ref(*m_check_db_obj));
  }
}


  //
  // Unsubscribe
  //

void
OksConfiguration::unsubscribe()
{
  if(m_check_db_obj) {
    ERS_DEBUG( 2, "stopping CheckDB thread ..." );
    m_check_db_obj->m_run = false;
    m_check_db_thread->join(); // wait thread termination
    delete m_check_db_thread;
    m_check_db_thread = 0;
    m_check_db_obj = 0;
    ERS_DEBUG( 2, "the CheckDB thread has been terminated" );
  }
}


void
OksConfiguration::print_profiling_info() noexcept
{
  std::cout <<
    "OksConfiguration profiler report:\n"
    "  number of loaded schema files: " << (m_kernel ? m_kernel->schema_files().size() : 0) << "\n"
    "  number of loaded data files: " << (m_kernel ? m_kernel->data_files().size() : 0) << "\n"
    "  number of loaded classes: " << (m_kernel ? m_kernel->classes().size() : 0) << "\n"
    "  number of loaded objects: " << (m_kernel ? m_kernel->objects().size() : 0) << std::endl;
}


  // read OksKernel parameters from environment (silence mode)

void
OksConfiguration::init_env()
{
  m_oks_kernel_silence = (getenv("OKSCONFIG_NO_KERNEL_SILENCE") == nullptr);
}
