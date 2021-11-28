#include <iostream>
#include <algorithm>

using namespace std;

namespace clist
{
    namespace ccolor // 色
    {
        namespace fg // 前景色
        {
            const string FG_Black   = "\033[30m"; // 黒
            const string FG_Red     = "\033[31m"; // 赤
            const string FG_Green   = "\033[32m"; // 緑
            const string FG_Yellow  = "\033[33m"; // 黄
            const string FG_Blue    = "\033[34m"; // 青
            const string FG_Magenta = "\033[35m"; // マゼンタ
            const string FG_Cyan    = "\033[36m"; // シアン
            const string FG_White   = "\033[37m"; // 白
            const string FG_Default = "\033[39m"; // デフォルト
        }

        namespace bg // 背景色
        {
            const string BG_Black   = "\033[40m"; // 黒
            const string BG_Red     = "\033[41m"; // 赤
            const string BG_Green   = "\033[42m"; // 緑
            const string BG_Yellow  = "\033[43m"; // 黄
            const string BG_Blue    = "\033[44m"; // 青
            const string BG_Magenta = "\033[45m"; // マゼンタ
            const string BG_Cyan    = "\033[46m"; // シアン
            const string BG_White   = "\033[47m"; // 白
            const string BG_Default = "\033[49m"; // デフォルト
        }

        namespace deco // 装飾
        {
            const string Deco_DefaultAll = "\033[0m"; // デフォルト
            const string Deco_Bold       = "\033[1m"; // 太字
            const string Deco_Underline  = "\033[4m"; // 下線
            const string Deco_rvserse    = "\033[7m"; // 反転
        }
    }

    namespace control // 制御
    {
        const string Control_ClearScreen          = "\033[2J"; // 画面クリア
        const string Control_ClearScreenAndBuffer = "\033[3J"; // 画面クリア + バッファクリア
        const string Control_ClearLine            = "\033[K";  // 行クリア
        const string Control_MoveUp               = "\033[1A"; // 上移動
        const string Control_MoveDown             = "\033[1B"; // 下移動
        const string Control_MoveRight            = "\033[1C"; // 右移動
        const string Control_MoveLeft             = "\033[1D"; // 左移動
        const string Control_MoveToHead           = "\033[H";  // 先頭へ移動
    }

    bool isDupChar (string str);
    int  charList  (unsigned int line_char_num, string characters, bool do_check = true);
}
