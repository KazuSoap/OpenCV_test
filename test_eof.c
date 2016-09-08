// gcc -O2 test_eoffff.c -lm -o test_eoffff -lcomdlg32 -municode

#ifndef UNICODE
#define UNICODE
#endif

#define MAX_STR_LEN 32

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <fcntl.h>
#include <conio.h>

// #define cls printf("\x1b[2J")

FILE* wfopen_r_InGui(wchar_t* path, wchar_t* fname);
int wgetPatternStr(wchar_t* pattern);

int wmain(void) {
    // _O_TEXT _O_U8TEXT _O_U16TEXT _O_WTEXT _O_BINARY
    _setmode( _fileno(stdout ), _O_U16TEXT);

    wchar_t path[MAX_PATH+1] = {0};
    wchar_t fname[129];
    FILE *fp;
    int ch;

    if( (fp = wfopen_r_InGui(path, fname)) == NULL ) {
        wprintf(L"%sのオープンに失敗\n", path);
        return EXIT_FAILURE;;
    }

    while (1) {
        wchar_t pattern[MAX_STR_LEN] = {0};

        system("cls"); // clear screen
        switch ( wgetPatternStr(pattern) ) {
        case 1:
            wprintf(L"検索処理 & 結果のファイル出力をする予定\n");
            break;
        case 2:
            wprintf(L"結果を表示する予定\n");
            break;
        case 0:
            fclose(fp);

            getchar();
            return EXIT_SUCCESS;
            break;
        }
    }
    fclose(fp);

    getchar();
    return EXIT_SUCCESS;
}

FILE* wfopen_r_InGui(wchar_t* path, wchar_t* fname) {

    OPENFILENAME ofname;
    memset(&ofname, 0, sizeof(OPENFILENAME));
    ofname.lStructSize = sizeof(OPENFILENAME);
    ofname.hwndOwner = NULL;
    ofname.lpstrFilter = L"テキストファイル(*.txt)\0*.txt\0\0";
    ofname.lpstrFile = path;
    ofname.nMaxFile = MAX_PATH;
    ofname.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofname.lpstrDefExt = L"txt";
    ofname.lpstrFileTitle = fname;
    ofname.nMaxFileTitle = 128;
    ofname.lpstrTitle = L"テキストファイルの選択";

    if( !GetOpenFileName(&ofname) ){
        wprintf(L"ファイルの読み込みをキャンセルしました\n");
        getchar();
        return NULL;
    }

    FILE* fp;
    // BOM 確認のため ファイルを binary mode で開く
    if ((fp = _wfopen(path, L"rb")) == NULL) {
        wprintf(L"%sのオープンに失敗\n", path);
        return NULL;
    }
    // BOM の確認
    fgetwc(fp);
    if(*(wint_t*)(fp->_base) != 0xfeff && *(wint_t*)(fp->_base) != 0xbbef){
        wprintf(L"\"%s\"\nはBOM 付 UNICODE テキストファイルではありません\n", path);
        return NULL;
    }
    fclose(fp);

    // ファイルを UNICODE mode で開く
    // ファイルから読み取られたデータは wchar_t 型として格納された UTF-16 データに変換される
    // (BOM付：BOMに基づいてエンコード、BOM無：UTF-16LE でエンコード)
    if ((fp = _wfopen(path, L"r, ccs=UNICODE")) == NULL) {
        wprintf(L"%sのオープンに失敗\n", path);
        return NULL;
    }

    return fp;
}

int wgetPatternStr(wchar_t* pattern){
mode1:
    system("cls"); // clear screen
    wprintf(L"検索パターンを入力してください(最大 %d 文字まで入力可)\n", MAX_STR_LEN);
    wprintf(L"\"Enter\" or \"EOF(Ctr + z)\" で入力確定\n");
    wprintf(L"\"Esc\" で別モードに変わります\n\n");
    wprintf(L"パターン : ");

    wint_t c = '\0';
    int i=0, special_key_f = 0;
    while ( (c = _getwch()) != 0x1a /*Ctr + z*/ && c != 0xd /*Enter*/ && i < MAX_STR_LEN ) {
        // Esc キーの検出
        if (c == 0x1b /*Esc*/){
            pattern[0] = L'\0';
            goto mode2;
            // 先頭のスペース & 特殊キー回避
        } else if ( special_key_f == 0 && (c == 0x00 || c == 0xe0) ) {
            special_key_f = 1;
            continue;
        } else if ( special_key_f == 1 || (c > 0x00 && c < 0x20) ||  (i==0 && c == 0x20 )  || c == 0x7f ){
            special_key_f = 0;
            continue;
        }

        // パターンを格納
        pattern[i++] = c;
        wprintf(L"\rパターン : %s", pattern);
    }
    wprintf(L"\n");

    return 1;

mode2:
    system("cls"); // clear screen
    wprintf(L"以下のコマンドを入力してください\n", MAX_STR_LEN);
    wprintf(L"1 : 保存した結果を表示する | Esc : 文字列検索モードに戻る | 9999 : プログラムの終了\n\n");
    wprintf(L"コマンド : ");

    c = '\0';
    i=0, special_key_f = 0;
    wchar_t command[4] = {0};
    while ( 1 ) {
        c = _getwch();
        if ( i > 2 && c == 0x39){
            command[i++] = c;
            wprintf(L"\rコマンド : %s\n", command);
            wprintf(L"プログラムを終了します\n");
            return 0;
        } else if ( c == 0x39 /*9*/){
            command[i++] = c;
            wprintf(L"\rコマンド : %s", command);
        } else if ( c == 0x31 /*1*/ && i==0 ) {
            wprintf(L"%c\n", c);
            wprintf(L"保存した結果を表示する予定\n");
            break;
        } else if ( c == 0x1b /*Esc*/ && i==0 ){
            wprintf(L"%s\n", L"Esc");
            goto mode1;
        }
    }
    wprintf(L"\n");

    return 2;
}
