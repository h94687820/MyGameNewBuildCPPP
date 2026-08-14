# Third-Person Cube Prototype

نموذج أولي مستقل بلغة C++ باستخدام raylib.

## التشغيل

```bash
cmake -S . -B build
cmake --build build
./build/third_person_cube
```

## التحكم

- `W A S D` أو الأسهم: تحريك المكعب
- أزرار `W A S D` داخل النافذة: تحريك بالماوس أو اللمس
- `ESC`: إغلاق اللعبة

الكاميرا تتبع المكعب من الخلف بمنظور الشخص الثالث، والمكعب حجمه `1 × 1 × 1`.

## Android APK

مشروع Android موجود داخل `android/`. يستخدم CMake وGradle وraylib، ويبني
نسخة `arm64-v8a` لهواتف Android الحديثة. أزرار W A S D داخل اللعبة تقرأ
اللمس الحقيقي عبر raylib، مع بقاء لوحة المفاتيح والماوس متاحين على الكمبيوتر.

بعد تثبيت Android SDK وNDK، اضبط المسارين ثم شغّل:

```bash
export ANDROID_SDK_ROOT="$HOME/Android/Sdk"
export ANDROID_NDK_ROOT="$ANDROID_SDK_ROOT/ndk/25.2.9519653"
./build_android.sh
```

ملف APK الناتج:

```text
android/app/build/outputs/apk/debug/app-debug.apk
```

لتثبيته على هاتف متصل مع تفعيل USB debugging:

```bash
adb install -r android/app/build/outputs/apk/debug/app-debug.apk
```

ملف `android/local.properties` يُنشأ تلقائيًا ولا يجب حفظه في Git.