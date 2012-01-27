svn co http://src.chromium.org/svn/trunk/tools/depot_tools depot_tools
svn co http://gyp.googlecode.com/svn/trunk gyp
svn co http://google-breakpad.googlecode.com/svn/trunk -r 699  breakpad
svn co http://svn.boost.org/svn/boost/tags/release/Boost_1_47_0 boost
svn co http://v8.googlecode.com/svn/tags/3.7.7/ v8
svn co http://svn.wxwidgets.org/svn/wx/wxWidgets/tags/WX_2_9_0/ wxWidgets

rm -fr curl-shared
rm -fr curl-static
rm -fr c-ares

#lets use the working version of curl
cp -rp curl curl-shared
cp -rp curl-shared curl-static

git clone git://github.com/bagder/c-ares.git c-ares
cd c-ares
git checkout cares-1_7_5

cd ..
