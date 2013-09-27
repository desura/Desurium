
cd build_out
desura.exe -unittests --gtest_output=xml:gtest_results.xml
echo ##teamcity[importData type='gtest' path='build_out\gtest_results.xml']
cd ..