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
- أزرار `W A S D` داخل النافذة: تحريك بالماوس
- `ESC`: إغلاق اللعبة

الكاميرا تتبع المكعب من الخلف بمنظور الشخص الثالث، والمكعب حجمه `1 × 1 × 1`.