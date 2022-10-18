
TDAQ_DB_SCHEMA 
     [ list of ] schema files to use  (required)

TDAQ_DB_PATH
     path prependend to TDAQ_DB_SCHEMA and data files

TDAQ_DB_DATA
     The top-level data file.


1. Test simple files:

    % setenv LD_LIBRARY_PATH   /.../oksconfig/vXrYpZ/${CMTCONFIG}

    % setenv TDAQ_DB_PATH      /.../oksconfig/vXrYpZ/test 

    % setenv TDAQ_DB_SCHEMA    okstest.schema.xml


  then
    
      % cd to test directory

      % ../Linux-i686/test_oksdb.exe oks:okstest.data.xml

    or
      % setenv TDAQ_DB_DATA    okstest.data.xml
      % ../Linux-i686/test_oksdb.exe oks:

2. Add test2.schema.xml to TDAQ_DB_SCHEMA
   Add test2.data.xml   to TDAQ_DB_DATA

3. Recursive Loading

      % setenv TDAQ_DB_SCHEMA  online.schema.xml
      % setenv TDAQ_DB_DATA    recursive.data.xml

      % ../Linux-i686/test_oksdb.exe oks:


    3a) Set AB and DC environment variable in such a way
        that they combine to TDAQ_DB_PATH

      % ../Linux-i686/test_oksdb.exe oks:recursive2.data.xml

