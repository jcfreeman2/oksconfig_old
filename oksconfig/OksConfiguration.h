// this is -*- c++ -*-
#ifndef OKSCONFIG_OKSCONFIGURATION_H_
#define OKSCONFIG_OKSCONFIGURATION_H_

//
// $Id$
//

#include <string>
#include <list>
#include <set>
#include <thread>

#include "config/ConfigurationImpl.h"

  // forward declarations for OKS classes
class OksKernel;
class OksObject;
class OksClass;
class OksFile;

  // forward declaration 
class OksConfigObject;
struct OksConfigurationCheckDB;


class OksConfiguration : public ConfigurationImpl {

  friend class OksConfigObject;
  friend struct OksConfigurationCheckDB;
  friend class ResubscribeGuard;

  public:

    OksConfiguration() noexcept : m_kernel (0), m_fn (0), m_oks_kernel_no_repo(false), m_check_db_obj (0), m_check_db_thread (0), m_repo_error_count(0) { init_env(); }
    virtual ~OksConfiguration() { close_db(); }

    typedef std::map< std::string , std::set<std::string> > SMap;

  public:

    virtual bool test_object(const std::string& class_name, const std::string& name, unsigned long rlevel, const std::vector<std::string> * rclasses);
    virtual void get(const std::string& class_name, const std::string& name, ConfigObject& object, unsigned long rlevel, const std::vector<std::string> * rclasses);
    virtual void get(const std::string& class_name, std::vector<ConfigObject>& objects, const std::string& query, unsigned long rlevel, const std::vector<std::string> * rclasses);
    virtual void get(const ConfigObject& obj_from, const std::string& query, std::vector<ConfigObject>& objects, unsigned long rlevel, const std::vector<std::string> * rclasses);
    virtual daq::config::class_t * get(const std::string& class_name, bool direct_only);
    virtual void get_superclasses(config::fmap<config::fset>& schema);

    virtual void create(const std::string& at, const std::string& class_name, const std::string& id, ConfigObject& object);
    virtual void create(const ConfigObject& at, const std::string& class_name, const std::string& id, ConfigObject& object);
    virtual void destroy(ConfigObject& object);

    virtual void open_db(const std::string& db_name);
    virtual void close_db() { close_database(true); }
    virtual bool loaded() const noexcept { return (m_kernel != 0); }
    virtual void create(const std::string& db_name, const std::list<std::string>& includes);
    virtual bool is_writable(const std::string& db_name);
    virtual void add_include(const std::string& db_name, const std::string& include);
    virtual void remove_include(const std::string& db_name, const std::string& include);
    virtual void get_includes(const std::string& db_name, std::list<std::string>& includes) const;
    virtual void get_updated_dbs(std::list<std::string>& dbs) const;
    virtual void set_commit_credentials(const std::string& user, const std::string& password);
    virtual void commit(const std::string& why);
    virtual void abort();
    virtual void prefetch_all_data() { ; } // nothing to do with OKS data already in-memory
    virtual std::vector<daq::config::Version> get_changes();
    virtual std::vector<daq::config::Version> get_versions(const std::string& since, const std::string& until, daq::config::Version::QueryType type, bool skip_irrelevant);

    virtual void subscribe(const std::set<std::string>& class_names, const SMap& objs, ConfigurationImpl::notify cb, ConfigurationImpl::pre_notify pre_cb);
    virtual void unsubscribe();

    virtual void print_profiling_info() noexcept;


  protected:

    void create(OksFile * at, const std::string& class_name, const std::string& id, ConfigObject& object);

  private:

    virtual void check_db();

    void close_database(bool unsubscribe);


  public:

    const OksKernel& get_oks_kernel() const { return *m_kernel; }  // required by Java oksconfig


  protected:

    OksKernel * m_kernel;
    ConfigurationImpl::notify m_fn;
    ConfigurationImpl::pre_notify m_pre_fn;
    std::set<std::string> m_class_names;
    SMap m_objects;
    bool m_oks_kernel_silence;
    bool m_oks_kernel_no_repo;

      // read OksKernel parameters from environment (silence mode)

    void init_env();


    OksConfigObject * new_object(OksObject * obj) noexcept;



      // keep information about changed objects

    std::list<OksObject *> m_created;
    std::set<OksObject *> m_modified;
    std::map<std::string, std::set<std::string> > m_removed;

    void subscribe();

    static void create_notify(OksObject *, void *) noexcept;
    static void change_notify(OksObject *, void *) noexcept;
    static void delete_notify(OksObject *, void *) noexcept;

    OksConfigurationCheckDB * m_check_db_obj;
    std::thread * m_check_db_thread;

    std::string m_user;
    std::string m_password;


      // keep information about created files

    std::set<OksFile *> m_created_files;


      //

    unsigned int m_repo_error_count;

};

#endif // OKSCONFIG_OKSCONFIGURATION_H_
