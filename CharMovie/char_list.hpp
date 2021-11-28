#include <iostream>
#include <algorithm>

using namespace std;

namespace clist
{
    namespace ccolor // �F
    {
        namespace fg // �O�i�F
        {
            const string FG_Black   = "\033[30m"; // ��
            const string FG_Red     = "\033[31m"; // ��
            const string FG_Green   = "\033[32m"; // ��
            const string FG_Yellow  = "\033[33m"; // ��
            const string FG_Blue    = "\033[34m"; // ��
            const string FG_Magenta = "\033[35m"; // �}�[���^
            const string FG_Cyan    = "\033[36m"; // �V�A��
            const string FG_White   = "\033[37m"; // ��
            const string FG_Default = "\033[39m"; // �f�t�H���g
        }

        namespace bg // �w�i�F
        {
            const string BG_Black   = "\033[40m"; // ��
            const string BG_Red     = "\033[41m"; // ��
            const string BG_Green   = "\033[42m"; // ��
            const string BG_Yellow  = "\033[43m"; // ��
            const string BG_Blue    = "\033[44m"; // ��
            const string BG_Magenta = "\033[45m"; // �}�[���^
            const string BG_Cyan    = "\033[46m"; // �V�A��
            const string BG_White   = "\033[47m"; // ��
            const string BG_Default = "\033[49m"; // �f�t�H���g
        }

        namespace deco // ����
        {
            const string Deco_DefaultAll = "\033[0m"; // �f�t�H���g
            const string Deco_Bold       = "\033[1m"; // ����
            const string Deco_Underline  = "\033[4m"; // ����
            const string Deco_rvserse    = "\033[7m"; // ���]
        }
    }

    namespace control // ����
    {
        const string Control_ClearScreen          = "\033[2J"; // ��ʃN���A
        const string Control_ClearScreenAndBuffer = "\033[3J"; // ��ʃN���A + �o�b�t�@�N���A
        const string Control_ClearLine            = "\033[K";  // �s�N���A
        const string Control_MoveUp               = "\033[1A"; // ��ړ�
        const string Control_MoveDown             = "\033[1B"; // ���ړ�
        const string Control_MoveRight            = "\033[1C"; // �E�ړ�
        const string Control_MoveLeft             = "\033[1D"; // ���ړ�
        const string Control_MoveToHead           = "\033[H";  // �擪�ֈړ�
    }

    bool isDupChar (string str);
    int  charList  (unsigned int line_char_num, string characters, bool do_check = true);
}
