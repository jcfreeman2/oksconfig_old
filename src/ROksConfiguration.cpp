#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdexcept>

#include <CoralBase/Exception.h>
#include <CoralKernel/Context.h>
#include "RelationalAccess/ConnectionService.h"
#include "RelationalAccess/ISessionProxy.h"
#include <RelationalAccess/ITransaction.h>

#include <oks/kernel.h>
#include <oks/ral.h>

#include <config/Configuration.h>
#include "oksconfig/ROksConfiguration.h"


  // to be used as plug-in

extern "C" ConfigurationImpl * _roksconfig_creator_ (const std::string& spec) {
  try {
    std::unique_ptr<ROksConfiguration> impl(new ROksConfiguration());
    if(!spec.empty()) { impl->open_db(spec); }
    return impl.release();
  }
  catch(daq::config::Exception & ex) {
    throw daq::config::Generic(ERS_HERE, "roksconfig initialization error", ex);
  }
  catch(...) {
    throw daq::config::Generic(ERS_HERE, "roksconfig initialization error:\n***** caught unknown exception *****");
  }
}

void
ROksConfiguration::bad_open_db(const std::string& text)
{
  close_db();
  throw ( daq::config::Generic( ERS_HERE, text.c_str() ) );
}

void
ROksConfiguration::open_db(const std::string& data)
{
    // create OKS kernel if it was not created

  if(m_kernel == 0) {
    m_kernel = new OksKernel(true);
  }

  Oks::Tokenizer t(data, ":");
  std::string tokens[5];

  for(size_t i=0; i<5; ++i) {
    if((tokens[i] = t.next()).empty() && i != 4) {
      std::ostringstream text;
      text << "failed to find in \"" << data << '\"'
           << ( i<2 ? "ral-connect-string" : i<3 ? "db-schema" : i<4 ? "oks-schema-version" : "oks-data-version" )
	   <<  "; expected parameter formats are:\n"
               "   - \"ral-connect-string:db-schema:oks-schema-version:oks-data-version\"\n"
               "   - \"ral-connect-string:db-schema:oks-data-tag\"\n";
      bad_open_db(text.str());
    }
  }

  m_schema_file = m_kernel->get_tmp_file("/tmp/roks.schema.xml");

  if(m_kernel->new_schema(m_schema_file) == 0) {
    std::ostringstream text;
    text << "failed to create file \'" << m_schema_file << '\'';
    bad_open_db(text.str());
  }
  else {
    m_data_file = m_kernel->get_tmp_file("/tmp/roks.data.xml");

    if(m_kernel->new_data(m_data_file) == 0) {
      std::ostringstream text;
      text << "failed to create file \'" << m_data_file << '\'';
      bad_open_db(text.str());
    }
    else {
      try {
        char * env = getenv("ROKSCONFIG_VERBOSITY");
        int verbose_level = (env ? atoi(env) : 0);
        std::unique_ptr<coral::ConnectionService> connection;
        {
          std::unique_ptr<coral::ISessionProxy> session (oks::ral::start_coral_session(tokens[0] + ':' + tokens[1], coral::ReadOnly, connection, verbose_level));

          int schema_version = -1;
          int data_version = -1;
 
            // try format "ral-connect-string:db-schema:oks-data-tag" 
          if(tokens[4].empty()) {
            if(verbose_level > 1) {
              std::cout << "ROksConfiguration::open_db(\'" << data << "\'): looking for schema and data versions for data tag \'" << tokens[3] << "\'..." << std::endl;
            }
            oks::ral::get_data_version(session.get(), tokens[2], tokens[3], schema_version, data_version, 0, verbose_level);
          }
            // try format "ral-connect-string:db-schema:oks-schema-version:oks-data-version"
          else {
            schema_version=atoi(tokens[3].c_str());
            data_version=atoi(tokens[4].c_str());
            if(data_version <= 0 || schema_version < 0) {
              if(verbose_level > 1) {
                std::cout << "ROksConfiguration::open_db(\'" << data << "\'): looking for head version..." << std::endl;
              }
              oks::ral::get_data_version(session.get(), tokens[2], "", schema_version, data_version, 0, verbose_level);
            }
          }

          if(verbose_level > 1) {
            std::cout << "ROksConfiguration::open_db(\'" << data << "\'): will use schema version " << schema_version << " and data version " << data_version << std::endl;
          }

          if(data_version <= 0 || schema_version < 0) {
            std::ostringstream text;
            text << "failed to open \'" << data << "\' (cannot get " << ((data_version == 0) ? "head" : "given") << " data)";
            bad_open_db(text.str());
          }

          oks::ral::get_data(*m_kernel, session.get(), tokens[2], schema_version, data_version, verbose_level);
          session->transaction().commit();
	}
      }
      catch ( coral::Exception& e ) {
        std::ostringstream text;
        text << "failed to open \'" << data << "\' (got CORAL exception " << e.what() << ')';
        bad_open_db(text.str());
      }
      catch ( std::exception& e ) {
        std::ostringstream text;
        text << "failed to open \'" << data << "\' (got standard C++ exception " << e.what() << ')';
        bad_open_db(text.str());
      }
    }
  }
}

void
ROksConfiguration::close_db()
{
  unlink(m_schema_file.c_str());
  unlink(m_data_file.c_str());
  OksConfiguration::close_db();
}
