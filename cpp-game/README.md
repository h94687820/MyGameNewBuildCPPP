# Downtown City — C++ Android Prototype

لعبة Native مستقلة بلغة C++ باستخدام raylib، تعرض مدينة Downtown أولية
من أصول glTF خفيفة مناسبة للنسخة الأولى على Android. الشخصية الحالية مكعب
مؤقت، والأنيميشن مؤجل حتى إضافة شخصية فعالة.

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

الكاميرا تتبع اللاعب من الخلف بمنظور الشخص الثالث، واللاعب حجمه `1 × 1 × 1`.
تُحمّل نماذج المباني والشوارع من `android/app/src/main/assets/city/`، وتبقى
مربعات المدينة القريبة فقط مرئية حول اللاعب مع تصادمات مبسطة للمباني.

## Android APK

مشروع Android موجود داخل `android/`. يستخدم CMake وGradle وraylib، ويبني
نسخة `arm64-v8a` لهواتف Android الحديثة. أزرار W A S D داخل اللعبة تقرأ
اللمس الحقيقي عبر raylib، مع بقاء لوحة المفاتيح والماوس متاحين على الكمبيوتر.

خطة متابعة إضافة المدينة والتحميل التدريجي موجودة في
[`NEXT_STEPS.md`](NEXT_STEPS.md).

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