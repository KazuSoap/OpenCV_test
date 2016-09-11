// gcc -O2 test_eof.c -lm -o test_eof -lcomdlg32 -municode

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <fcntl.h>
#include <conio.h>
#include <io.h>

#define MAX_PATT_LEN 32
#define MAX_MDATA_NUM 100

typedef struct matchData {
    wchar_t pattern[MAX_PATT_LEN];
    int pat_len;
    long n;
    long pos[100];
} mData;

FILE* wfopen_r_InGui ( wchar_t* path, wchar_t* fname );
int wgetPatternStr ( wchar_t* pattern );
int patternMatchInFile (FILE* fp, wchar_t* pattern, mData* md);
int parseMatchDataCSV ( );
int comp( const void *c1, const void *c2 );

int wmain(void) {
    // _O_TEXT _O_U8TEXT _O_U16TEXT _O_WTEXT _O_BINARY
    _setmode( _fileno(stdout ), _O_U16TEXT);

    wchar_t path[MAX_PATH+1] = {0};
    wchar_t fname[129];
    FILE *fp;
    mData mds[MAX_MDATA_NUM];

    if( (fp = wfopen_r_InGui(path, fname)) == NULL ) {
        wprintf(L"%sのオープンに失敗\n", path);
        return EXIT_FAILURE;;
    }

    /* parseMatchDataCSV(); */

    for (int i=0; i < MAX_MDATA_NUM;) {
        wchar_t pattern[MAX_PATT_LEN] = {0};

        system("cls"); // clear screen
        switch ( wgetPatternStr(pattern) ) {
        case 1:
            patternMatchInFile( fp, pattern , &mds[i]);
            if ( mds[i].pat_len ){
                if( mds[i].n ){
                    wprintf(L"マッチしました : マッチ数 : %d個, 最初にマッチした位置 : %d文字目,\n", mds[i].n, mds[i].pos[i]);
                }
                i++;
            }
            getchar();
            break;
        case 2: {
            FILE *fptmp;
            if ((fptmp = _wfopen(L"./output.csv", L"w")) == NULL) {
                wprintf(L"オープンに失敗\n");
                return -1;
            }

            // 結果表示 & 保存
            qsort( mds, i, sizeof(mData), comp); // 降順ソート
            for(int j=0; j < i; j++){
                wprintf(L"--------------------------------\n");
                wprintf(L"pattern : %s\n個数 : %d\n位置 : ", mds[j].pattern, mds[j].n);
                for( int k=0; k < mds[j].n; k++){
                    wprintf(L"%d, ", mds[j].pos[k]);
                }
                wprintf(L"\n");
            }

            for(int j=0; j < i; j++){
                fwprintf(fptmp, L" %d,", mds[j].pat_len);
                for( int k=0; k < mds[j].pat_len; k++){
                    fwprintf(fptmp, L" %x", mds[j].pattern[k]);
                }
                fwprintf(fptmp, L",");

                fwprintf(fptmp, L"%ld,", mds[j].n);
                for( int k=0; k < mds[j].n; k++){
                    fwprintf(fptmp, L"%ld,", mds[j].pos[k]);
                }
                fwprintf(fptmp, L"\n");
            }
            fclose(fptmp);

            wprintf(L"結果のファイル出力をする予定\n");
            getchar();

            break;
        }
        case 0:
            fclose(fp);

            getchar();
            return EXIT_SUCCESS;
            break;
        default:
            break;
        }
    }
    fclose(fp);

    getchar();
    return EXIT_SUCCESS;
}

FILE* wfopen_r_InGui (wchar_t* path, wchar_t* fname) {

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

    // ファイルを UNICODE mode で開く
    // ファイルから読み取られたデータは wchar_t 型として格納された UTF-16LE データに変換される
    // (BOM付：BOMに基づいてエンコード、BOM無：UTF-16LE でエンコード)
    if ((fp = _wfreopen(path, L"r, ccs=UNICODE", fp)) == NULL) {
        wprintf(L"%sのオープンに失敗\n", path);
        return NULL;
    }

    return fp;
}

int wgetPatternStr (wchar_t* pattern) {
mode1:
    system("cls"); // clear screen
    wprintf(L"検索パターンを入力してください(最大 %d 文字まで入力可)\n", MAX_PATT_LEN);
    wprintf(L"\"Enter\" or \"EOF(Ctr + z)\" で入力確定\n");
    wprintf(L"\"Esc\" で別モードに変わります\n\n");
    wprintf(L"パターン : ");

    wint_t c = '\0';
    int i=0, special_key_f = 0;
    while ( (c = _getwch()) != 0x1a /*Ctr + z*/ && c != 0xd /*Enter*/ && i < MAX_PATT_LEN ) {
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
    wprintf(L"以下のコマンドを入力してください\n", MAX_PATT_LEN);
    wprintf(L"1 : 保存した結果を表示する\nEsc : 文字列検索モードに戻る\n9999 : プログラムの終了\n\n");
    wprintf(L"コマンド : ");

    c = '\0';
    i=0;
    wchar_t command[5] = {0};
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

int patternMatchInFile (FILE* fp, wchar_t* pattern, mData* md) {
    fpos_t pos_first;
    fgetpos( fp, &pos_first ); // 初期位置を取得

    wint_t c = fgetwc(fp); // 初回時にファイルバッファが読み込まれる
    fsetpos( fp, &pos_first ); // 初期位置に戻す

    wchar_t tmpstr[MAX_PATT_LEN] = {0};
    md->pat_len = wcslen(pattern);

    memcpy( md->pattern, pattern, sizeof(wchar_t)*md->pat_len);
    md->pattern[md->pat_len] = L'\0';

    // マッチング処理
    int cnt = 0;
    for ( long i=1; (c = fgetwc(fp)) != WEOF && fp->_cnt >= md->pat_len*sizeof(wchar_t); i++ ) {
        if ( c == pattern[0] ) {
            memcpy( tmpstr, (wchar_t*)fp->_ptr-1, sizeof(wchar_t)*md->pat_len);
            if ( !wcscmp(pattern, tmpstr) ) {
                md->pos[cnt] = i;
                cnt++;
            }
        }
    }
    fsetpos( fp, &pos_first ); // 初期位置に戻す
    md->n = cnt;

    return 0;
}

int parseMatchDataCSV ( ){
    FILE *fp;
    if ((fp = _wfopen(L"./output.csv", L"r")) == NULL) {
        wprintf(L"オープンに失敗\n");
        return -1;
    }

    wchar_t c;
    wchar_t tmpstr1[MAX_PATT_LEN] = {0};
    wchar_t tmpstr2[MAX_PATT_LEN] = {0};
    for ( int cnt=0; (c = fgetwc(fp)) != WEOF; ) {
        if( c == L' ' ){
            int i = 0;
            while ( *(fp->_ptr+i) != L' ' && *(fp->_ptr+i) != L',' ) {
                tmpstr1[i] = *(fp->_ptr+i);
                i++;
            }
            tmpstr1[i] = L'\0';

            wchar_t *endptr;
            tmpstr2[cnt] = wcstol( tmpstr1, &endptr, 16);
            cnt++;
        } else if ( c == L',' ) {
            int i = 0;
            while ( *(fp->_ptr+i) != L',' && *(fp->_ptr+i) != L'\n' ) {
                tmpstr1[i] = *(fp->_ptr+i);
                i++;
            }
            tmpstr1[i] = L'\0';

            if( wcslen(tmpstr1) ){
                wchar_t *endptr;
                long x = wcstol( tmpstr1, &endptr, 10);

                tmpstr2[cnt] = L'\0';
                wprintf(L"%s, %ld", tmpstr2, x);
            } else {
                tmpstr2[cnt] = L'\0';
                wprintf(L"%s,", tmpstr2);
            }
            cnt=0;
        } else if ( c == L'\n' ) {
            wprintf(L"\n", tmpstr2);
        }
    }

    fclose(fp);

    return 0;
}

/* 比較関数 */
int comp( const void *c1, const void *c2 ) {
    return ((mData *)c2)->n - ((mData *)c1)->n;
}
