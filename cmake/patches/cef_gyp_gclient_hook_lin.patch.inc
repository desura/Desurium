diff '--exclude=.svn' -aurN cef.ori/tools/gclient_hook.py cef/tools/gclient_hook.py
--- tools/gclient_hook.py	2012-05-09 11:27:16.865674327 +0200
+++ tools/gclient_hook.py	2012-05-09 11:32:01.703658217 +0200
@@ -54,5 +54,28 @@
 
 print "\nGenerating CEF project files..."
 os.environ['CEF_DIRECTORY'] = os.path.basename(cef_dir);
-gyper = [ 'python', 'tools/gyp_cef', 'cef.gyp', '-I', 'cef.gypi' ]
+# for windows we have to build all deps
+# gyper = [ 'python', 'tools/gyp_cef', 'cef.gyp', '-I', 'cef.gypi', '-Dwerror=']
+gyper = [ 'python', 'tools/gyp_cef', 'cef.gyp',
+	'-I', 'cef.gypi',
+	'-Dwerror=',
+	'-Duse_cups=0',
+	'-Duse_gconf=0',
+	'-Duse_gnome_keyring=0',
+	'-Duse_kerberos=0',
+	'-Dproprietary_codecs=1',
+	'-Dffmpeg_branding=@CEF_FFMPEG_BRANDING@',
+	'-Duse_system_bzip2=1',
+	'-Duse_system_flac=1',
+	'-Duse_system_icu=0',
+	'-Duse_system_libevent=1',
+	'-Duse_system_libjpeg=1',
+	'-Duse_system_libpng=1',
+	'-Duse_system_libwebp=0',
+	'-Duse_system_libxml=1',
+	'-Duse_system_speex=1',
+	'-Duse_system_v8=0',
+	'-Duse_system_xdg_utils=1',
+	'-Duse_system_yasm=1',
+	'-Duse_system_zlib=1']
 RunAction(cef_dir, gyper);