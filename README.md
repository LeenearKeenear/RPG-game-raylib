tutor cara masang raylib

1. download filenya
2. kalo udah extract
3. kalo udah kalian buka raylibnya
4. lalu cari file header raylibnya di path ini "C:\raylib-5.5_win32_mingw-w64\include\raylib.h"
5. sekarang copy filenya terus paste di folder include compiler kalian. contoh : "C:\MinGW\include\raylib.h"
6. lakukan untuk semua header yang ada pada folder include raylib (ada 3)
7. lakukan langkah 3-5 untuk file dengan nama "C:\raylib-5.5_win32_mingw-w64\lib\libraylib.a". bedanya ini di taro di folder lib compiler kalian. contoh : "C:\MinGW\lib\libraylib.a"
8. kalo mau compile di linking pake flag ini
    -IC:\\raylib-5.5_win32_mingw-w64\\include -LC:\\raylib-5.5_win32_mingw-w64\\lib -lraylib -lopengl32 -lgdi32 -lwinmm


cara untuk ngerun program ini :
- make app

cara hapus file build dan exe :
- make cln
