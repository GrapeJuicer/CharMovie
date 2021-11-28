#include "optarg.hpp"

// summary:
//      parse the arguments. more intuitive to use than 'getopt_once'
// arg:
//      argc: number of arguments
//      argv: arguments
//      docopts: option list
//      findopts: array to store the found options
//      findopt_size: size of findopts array
// return:
//      1~: index of the first non-optional argument
//      -1: invalid option
int getopt_flex(int argc, char** argv, const struct docoption* docopts, struct optarg* findopts, size_t findopts_size)
{
	int i;
	int opt;
	int cnt = 0;
	int loptindex;
	struct option* longopts;
	char* shortopts;
	int docopts_size = __optSize(docopts) + 1;

	// convert 'struct docoption' to 'struct option'(long option) and 'char *'(short option)
	if (__convertOption(docopts, &longopts, &shortopts) == -1)
	{
		return -1;
	}

	// short option にて禁止オプション '?' を使用している．
	if (strchr(shortopts, '?') != NULL)
	{
		__flushOpts(shortopts, longopts);
		return -1;
	}

	for (i = 0; i < docopts_size; i++)
	{
		if (docopts[i].val == '?')
		{
			__flushOpts(shortopts, longopts);
			return -1;
		}
	}

	// 結果を格納する配列の初期化
	for (i = 0; i < findopts_size; i++)
	{
		findopts[i].opt = -1;
		findopts[i].arg = NULL;
	}

	while ((opt = getopt_long(argc, argv, shortopts, longopts, &loptindex)) != -1)
	{
		// 見つからなかった場合は何もしない
		if (opt == '?')
		{
			continue;
		}

		// short option を val に変換
		for (i = 0; i < docopts_size; i++)
		{
			if (opt == docopts[i].short_name)
			{
				opt = docopts[i].val;
				break;
			}
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
		if (cnt >= findopts_size)
		{
			__flushOpts(shortopts, longopts);
			return -1;
		}

		findopts[cnt].opt = opt;
		findopts[cnt].arg = optarg;
		cnt++;
	}

	// flush options
	__flushOpts(shortopts, longopts);

	return cnt;
}

// summary:
//      returns option list size.
// arg:
//      opts: option list
// return:
//      0~: option list size without DOCOPTION_END.
int __optSize(const struct docoption* opts)
{
	int cnt = 0;

	while (!__isEnd(opts[cnt]))
	{
		cnt++;
	}
	return cnt;
}

// summary:
//      check if the option is DOCOPT_END.
// arg:
//      opt: option
// return:
//      0: not DOCOPT_END / not 0: DOCOPT_END
int __isEnd(const struct docoption opt)
{
	struct docoption nullopt = DOCOPT_END;
	return opt.val == nullopt.val && opt.short_name == nullopt.short_name && opt.long_name == nullopt.long_name && opt.has_arg == nullopt.has_arg && opt.help_msg == nullopt.help_msg;
}

// summary:
//      dynamically allocate an area to store longoption and shortoption.
// arg:
//      shortopts: short option's pointer x2
//      shortopts_size: short option's array size
//      longopts: long option's pointer x2
//      longopts_size: long option's array size
// return:
//      0: success / -1: failure
int __initOpts(char** shortopts, size_t shortopts_size, struct option** longopts, size_t longopts_size)
{
	*shortopts = NULL;
	*longopts = NULL;

	if ((*shortopts = (char*)malloc(sizeof(char) * shortopts_size)) == NULL)
	{
		// error
		return -1;
	}

	if ((*longopts = (struct option*)malloc(sizeof(struct option) * longopts_size)) == NULL)
	{
		// error
		__flushOpts(*shortopts, *longopts);
		return -1;
	}

	return 0;
}

// summary:
//      free up space for long option and short option
// arg:
//      shortopts: short option's pointer
//      longopts: long option's pointer
// return:
//      void
void __flushOpts(char* shortopts, struct option* longopts)
{
	SFREE(shortopts);
	SFREE(longopts);
}

// summary:
//      convert options from 'struct docoption' to 'struct option'
// arg:
//      docopts: option list
//      docopts_size: option list size
//      longopts: long option's pointer
//      longopts_size: long option's array size
// return:
//      0: success / -1: failure
int __generateLongOption(const struct docoption* docopts, size_t docopts_size, struct option* longopts, size_t longopts_size)
{
	int i, cnt = 0;

	// check array size
	if (longopts_size < docopts_size + 1)
	{
		return -1;
	}

	// generate long option
	for (i = 0; i <= docopts_size; i++)
	{
		if (docopts[i].long_name != 0)
		{
			longopts[cnt].name = docopts[i].long_name;
			longopts[cnt].has_arg = docopts[i].has_arg;
			longopts[cnt].flag = NULL;
			longopts[cnt].val = docopts[i].val;
			cnt++;
		}
	}

	longopts[cnt].name = 0;
	longopts[cnt].has_arg = 0;
	longopts[cnt].flag = 0;
	longopts[cnt].val = 0;

	return 0;
}

// summary:
//      convert options from 'struct docoption' to 'char'
// arg:
//      docopts: option list
//      docopts_size: option list size
//      shortopts: short option's pointer
//      shortopts_size: short option's array size
// return:
//      0: success / -1: failure
int __generateShortOption(const struct docoption* docopts, size_t docopts_size, char* shortopts, size_t shortopts_size)
{
	char buf[4];
	int i;

	// check array size
	if (shortopts_size < __calShortOptsLength(docopts, docopts_size))
	{
		return -1;
	}

	// initialize shortopts
	for (i = 0; i < shortopts_size; i++)
	{
		shortopts[i] = '\0';
	}

	// generate short option
	for (i = 0; i < docopts_size; i++)
	{
		switch (docopts[i].has_arg)
		{
		case no_argument:
			sprintf(buf, "%c", docopts[i].short_name);
			break;
		case required_argument:
			sprintf(buf, "%c:", docopts[i].short_name);
			break;
		case optional_argument:
			sprintf(buf, "%c::", docopts[i].short_name);
			break;
		default:
			return -1;
		}
		strcat_s(shortopts, shortopts_size, buf);
	}

	return 0;
}

// summary:
//      calculate short option's length
// arg:
//      docopts: option list
//      docopts_size: option list size
// return:
//      short option's length
int __calShortOptsLength(const struct docoption* docopts, size_t docopts_size)
{
	int i;
	int len = 0;

	for (i = 0; i < docopts_size; i++)
	{
		switch (docopts[i].has_arg)
		{
		case no_argument:
			len += 1; // Denoted by "x"
			break;
		case required_argument:
			len += 2; // Denoted by "x:"
			break;
		case optional_argument:
			len += 3; // Denoted by "x::"
			break;
		default:
			return -1;
		}
	}

	// add '\0'
	len++;

	return len;
}

// summary:
//      convert options from 'struct docoption' to 'struct option' and 'char'
// arg:
//      docopts: option list
//      longopts: long option's pointer x2
//      shortopts: short option's pointer x2
// return:
//      0: success / -1: failure
int __convertOption(const struct docoption* docopts, struct option** longopts, char** shortopts)
{
	// get docopts size
	int docopts_size = __optSize(docopts);

	// get short opts array length
	int shortopts_len = __calShortOptsLength(docopts, docopts_size);
	int longopts_len = docopts_size + 1;

	// initialize long opts and short opts
	if (__initOpts(shortopts, shortopts_len, longopts, longopts_len) == -1)
	{
		return -1;
	}

	// generate short option
	if (__generateShortOption(docopts, docopts_size, *shortopts, shortopts_len) == -1)
	{
		__flushOpts(*shortopts, *longopts);
		return -1;
	}

	// generate long option
	if (__generateLongOption(docopts, docopts_size, *longopts, longopts_len) == -1)
	{
		__flushOpts(*shortopts, *longopts);
		return -1;
	}

	return 0;
}
