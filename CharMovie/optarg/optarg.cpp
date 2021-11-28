#include "optarg.hpp"

// summary:
//      parse the arguments.
// arg:
//      argc: number of arguments.
//      argv: arguments
//      shortopts: short options
//      longopts: long options
//      findopts: array to store the found options
//      len_findopts: size of findopts array
int getopt_once(int argc, char* argv[], const char* shortopts, const struct option* longopts, struct optarg findopts[], int len_findopts)
{
	int opt;
	int i;
	int cnt = 0;
	int loptindex;

	// short option にて禁止オプション '?' を使用している．
	if (strchr(shortopts, '?') != NULL)
	{
		return -1;
	}

	i = 0;
	while (!(longopts[i].name == 0 && longopts[i].has_arg == 0 && longopts[i].flag == 0 && longopts[i].val == 0))
	{
		// long option にて禁止オプション '?' を使用している．
		if (longopts[i].val == '?')
		{
			return -1;
		}
		i++;
	}

	// 結果を格納する配列の初期化
	for (i = 0; i < len_findopts; i++)
	{
		findopts[i].opt = -1;
	}

	while ((opt = getopt_long(argc, argv, shortopts, longopts, &loptindex)) != -1)
	{
		// 見つからなかった場合は何もしない
		if (opt == '?')
		{
			continue;
		}

		// 既に見つかっているかチェック
		for (i = 0; i < cnt; i++)
		{
			if (opt == findopts[i].opt)
			{
				break;
			}
		}

		// 既に一度見つかっているなら何もしない
		if (i < cnt)
		{
			continue;
		}

		// 配列のサイズを超えたらエラー
		if (cnt >= len_findopts)
		{
			return -1;
		}

		// 追加
		findopts[cnt].opt = opt;
		findopts[cnt].arg = optarg;

		cnt++;
	}

	return cnt;
}
