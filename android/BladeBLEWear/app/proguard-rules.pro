# Add project specific ProGuard rules here.
# You can control the set of applied configuration files using the
# proguardFiles setting in build.gradle.
#
# For more details, see
#   http://developer.android.com/guide/developing/tools/proguard.html

# If your project uses WebView with JS, uncomment the following
# and specify the fully qualified class name to the JavaScript interface
# class:
#-keepclassmembers class fqcn.of.javascript.interface.for.webview {
#   public *;
#}

-keepclassmembers class pw.rfg.bladeblewear.menu.SubMenuModel { *; }
-keepclassmembers class pw.rfg.bladeblewear.menu.ButtonModel { *; }
-keepclassmembers class pw.rfg.bladeblewear.activity.MainActivity { *; }
-keepclassmembers class pw.rfg.bladeblewear.ble.BleMaster { *; }

# Uncomment this to preserve the line number information for
# debugging stack traces.
-keepattributes SourceFile,LineNumberTable

# If you keep the line number information, uncomment this to
# hide the original source file name.
#-renamesourcefileattribute SourceFile
-keep class org.apache.commons.logging.**               { *; }
-dontwarn javax.script.**
-dontwarn org.apache.commons.logging.**
-dontwarn java.beans.**
-dontwarn org.apache.commons.jexl3.**

-dontobfuscate