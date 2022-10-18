#! /bin/sh

unset OKS_DB_ROOT

###############################################################################

  # database spec fro C++ and Java tests

db="oksconfig:${2}/test/okstest.data.xml:${2}/test/test2.data.xml"

###############################################################################

echo ''
echo 'TEST (1/3): C++ library test '
echo "run: \"$1/test_oksdb $db\""
if $1/test_oksdb $db
then
  echo '' 
  echo 'c++ library test passed' 
else
  echo '' 
  echo 'c++ library test failed'
  exit 1
fi

###############################################################################

echo ''
echo 'TEST (2/3): C++ config plugin test'
echo "run: \"config_dump -d "${db}" -c "TestPrimitive"\""
if config_dump -d "${db}" -c "TestPrimitive"
then
  echo ''
  echo 'c++ config plugin test passed'
else
  echo ''
  echo 'c++ config plugin test failed'
  exit 1
fi

###############################################################################

LD_LIBRARY_PATH=../$CMTCONFIG:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH

echo ''
echo 'TEST (3/3): Java test'
echo "run: \"$TDAQ_JAVA_HOME/bin/java -classpath $1/jtest.jar:$1/oksconfig.jar:\$CLASSPATH Test ${db}\""
if $TDAQ_JAVA_HOME/bin/java -classpath $1/jtest.jar:$1/oksconfig.jar:$CLASSPATH Test "${db}"
then
  echo '' 
  echo 'read java test passed' 
else
  echo '' 
  echo 'read java test failed'
  exit 1
fi

echo '' 
echo '*********************************************************************'
echo ''

###############################################################################
