diff '--exclude=.svn' -aurN cef.ori/tools/gclient_hook.py cef/tools/gclient_hook.py
--- tools/gclient_hook.py	2012-05-09 11:27:16.865674327 +0200
+++ tools/gclient_hook.py	2012-05-09 11:32:01.703658217 +0200
@@ -54,5 +54,13 @@
 
 print "\nGenerating CEF project files..."
 os.environ['CEF_DIRECTORY'] = os.path.basename(cef_dir);
-gyper = [ 'python', 'tools/gyp_cef', 'cef.gyp', '-I', 'cef.gypi' ]
+# for windows we have to build all deps
+# gyper = [ 'python', 'tools/gyp_cef', 'cef.gyp', '-I', 'cef.gypi', '-Dwerror=']
+gyper = [ 'python', 'tools/gyp_cef', 'cef.gyp',
+	'-I', 'cef.gypi',
+	'-Dwerror=',
+	'-Dchromium_win_pch=1',
+	'-Dmsvs_multi_core_compile=1',
+	'-Dproprietary_codecs=1',
+	'-Dffmpeg_branding=@CEF_FFMPEG_BRANDING@']
 RunAction(cef_dir, gyper);