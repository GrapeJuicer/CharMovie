#include "optarg.hpp"

/* case 1
  -a      , --alpha         This is an option with an argument.
  -b [ARG], --beta [ARG]    This is another option with an argument.
  -c      , --charlie       This is a third option with an argument.
			--delta         This is a fourth option with an argument.
  -e                        This is a fifth option with an argument.
*/
/* case 2
  --alpha           This is an option with an argument.
  --beta [ARG]      This is another option with an argument.
  --charlie ARG     This is a third option with an argument.
  --delta           This is a fourth option with an argument.
  --echo            This is a fifth option with an argument.
*/
/* case 1
  -a          This is an option with an argument.
  -b [ARG]    This is another option with an argument.
  -c          This is a third option with an argument.
  -d ARG      This is a fourth option with an argument.
  -e          This is a fifth option with an argument.
*/

/*
2(  ) + 1(-) + 1(sopts) + 6([=ARG]) + 2(, ) + 2(--) + n(lopts) + 6([=ARG])
= 20 + n ... option list width
*/

// summary:
//      print help message.
// arg:
//      docopts: option list
//      progname: program name = argv[0]
//      usage_arg: format of command arguments to be displayed in USAGE.
//          If NULL, the default format is "[OPTION] ...".
//          else, write it like following: "[OPTION] ... source_file target_file" (note: Don't write prog name)
//          ex: "[OPTION] ... source_file target_file" --> "Usage: {executable} [OPTION] ... source_file target_file\n"
//      style: style of help message
//      header: header of help message. nullable.
//      footer: footer of help message. nullable.
int printHelp(const struct docoption* docopts, const char* progname, const char* usage_arg, const struct docstyle style, const char* header, const char* footer)
{
	int docopts_size = __optSize(docopts);
	size_t slen = __shortOptionCommandLength(docopts, docopts_size);
	size_t llen = __longOptionCommandLength(docopts, docopts_size);
	size_t doc_indent = __calIndentSize(slen, llen, style);

	if (slen < 0 || llen < 0)
	{
		return -1;
	}

	// print usage
	__printUsage(progname, usage_arg);

	// print header
	if (header != NULL)
	{
		printf("%s\n\n", header);
	}

	// print options
	printf("Options:\n");

	// loop for each option
	for (int i = 0; i < docopts_size; i++)
	{
		// print option command
		if (__printOptionCommand(docopts[i], slen, llen, style) == -1)
		{
			return -1;
		}

		// print option description
		__printOptionDocs(docopts[i].help_msg, doc_indent, style.doc_width);
	}

	// print supplementary explanation
	printf("\n");
	printf("It is also possible to specify the options in the following:\n");
	printf("  -o ARG        ---> -oARG\n");
	printf("  --option ARG  ---> --option=ARG\n");
	printf("\n");

	// print footer
	if (footer != NULL)
	{
		printf("%s\n", footer);
	}

	return 0;
}

// summary:
//      Calculating the size of the indent until the help message summary is displayed.
// arg:
//      slen: max size of short option
//      llen: max size of long option
//      style: style of help message
// return:
//      indent size
size_t __calIndentSize(size_t slen, size_t llen, const struct docstyle style)
{
	return strlen(style.indent) + slen + strlen(style.separator) + llen + strlen(style.margin);
}

// summary:
//      print usage message.
// arg:
//      progname: program name = argv[0]
//      usage_arg: Format of arguments to be displayed in USAGE.
// return:
//      void
void __printUsage(const char* progname, const char* usage_arg)
{
	printf("Usage: %s %s\n", progname, usage_arg);
	printf("\n");
}

// summary:
//      Calculate the maximum length of the short option.
// arg:
//      docopts: option list
//      docopts_size: size of option list
// return:
//      maxium length of short option
int __shortOptionCommandLength(const struct docoption* docopts, int docopts_size)
{
	int i;
	int slen = 0;
	int tmp;

	// short opts length
	for (i = 0; i < docopts_size; i++)
	{
		if (docopts[i].short_name != 0)
		{
			switch (docopts[i].has_arg)
			{
			case no_argument:
				tmp = 2; // -a
				break;
			case required_argument:
				tmp = 6; // -a ARG
				break;
			case optional_argument:
				tmp = 8; // -a [ARG]
				break;
			default:
				return -1;
			}

			// save larger one
			if (tmp > slen)
			{
				slen = tmp;
			}
		}
	}

	return slen;
}

// summary:
//      Calculate the maximum length of the long option.
// arg:
//      docopts: option list
//      docopts_size: size of option list
// return:
//      maxium length of long option
size_t __longOptionCommandLength(const struct docoption* docopts, int docopts_size)
{
	int i;
	size_t llen = 0;
	size_t tmp;

	// long opts length
	for (i = 0; i < docopts_size; i++)
	{
		if (docopts[i].long_name != 0)
		{
			// long opts length
			tmp = strlen(docopts[i].long_name);

			switch (docopts[i].has_arg)
			{
			case no_argument:
				tmp += 2; // --alpha
				break;
			case required_argument:
				tmp += 6; // --alpha ARG
				break;
			case optional_argument:
				tmp += 8; // --alpha [ARG]
				break;
			default:
				return -1;
			}

			// save larger one
			if (tmp > llen)
			{
				llen = tmp;
			}
		}
	}

	return llen;
}

// summary:
//      print option command. (onoly 1 line)
// arg:
//      opt: option
//      slen: maxium length of short option
//      llen: maxium length of long option
//      style: style of help message
// return:
//      0: success / -1: error
int __printOptionCommand(const struct docoption opt, size_t slen, size_t llen, struct docstyle style)
{
	if (slen == -1 || llen == -1)
	{
		return -1;
	}

	// print indent
	printf("%s", style.indent);

	// print short option
	if (opt.short_name != 0)
	{
		switch (opt.has_arg)
		{
		case no_argument:
			printf("-%-*c", (int)slen - 1, opt.short_name);
			break;
		case required_argument:
			printf("-%c ARG", opt.short_name);
			printf("%*s", (int)slen - 6, ""); // space * (slen - 6)
			break;
		case optional_argument:
			printf("-%c [ARG]", opt.short_name);
			printf("%*s", (int)slen - 8, ""); // space * (slen - 8)
			break;
		default:
			break;
		}
	}
	else
	{
		printf("%*s", (int)slen, ""); // space * slen
	}

	// print separator
	if (opt.short_name != 0 && opt.long_name != 0)
	{
		printf("%s", style.separator);
	}
	// if not exist short option or long option
	else if (slen > 0 && llen > 0)
	{
		printf("%*s", (int)strlen(style.separator), ""); // space * strlen(sep)
	}

	// print long option
	if (opt.long_name != 0)
	{
		switch (opt.has_arg)
		{
		case no_argument:
			printf("--%-*s", (int)llen - 2, opt.long_name);
			break;
		case required_argument:
			printf("--%s ARG", opt.long_name);
			printf("%*s", (int)llen - 6 - (int)strlen(opt.long_name), "");
			break;
		case optional_argument:
			printf("--%s [ARG]", opt.long_name);
			printf("%*s", (int)llen - 8 - (int)strlen(opt.long_name), "");
			break;
		default:
			return -1;
		}
	}
	else
	{
		printf("%*s", (int)llen, ""); // space * llen
	}

	// print margin
	printf("%s", style.margin);

	return 0;
}

// summary:
//      print option description. note: indent will not be applied to the first line.
// arg:
//      docs: description
//      indent: indent
//      min_line_size: minimum line size of description
// return:
//      void
void __printOptionDocs(const char* docs, size_t indent, size_t min_line_size)
{
	size_t base = 0;
	bool is_first = true;
	bool contain_return = false;
	bool contain_null = false;

	while (base < strlen(docs))
	{
		// initialize
		size_t cnt = 0;

		// count
		// proceed until it reaches '\0' or '\n' or ' '
		while (docs[base + cnt] != '\0' && docs[base + cnt] != '\n' && (docs[base + cnt] != ' ' || cnt < min_line_size - 1))
		{
			// print cnt
			cnt++;
		}

		// chck contain '\n' or '\0'
		switch (docs[base + cnt])
		{
		case '\n':
			contain_return = true;
			break;
		case '\0':
			contain_null = true;
			break;
		default:
			break;
		}

		// if string is not ended
		if (!contain_null && !contain_return)
		{
			int peak = 0;
			// proceed until it reaches '\0' or '\n' or 'a-zA-Z0-9'
			while (docs[base + cnt + peak] != '\0' && docs[base + cnt + peak] != '\n' && iswalnum(docs[base + cnt + peak]) == 0)
			{
				peak++;
			}

			if (docs[base + cnt + peak] == '\n')
			{
				contain_return = true;
			}
			cnt += peak;
		}

		// print help msg
		printf("%*.*s\n", (int)(cnt + (is_first ? 0 : indent)) /*don't indent only at the beginning*/, (int)cnt, &docs[base]);
		is_first = false;

		// update p
		base += (contain_return ? cnt + 1 : cnt);
	}
}

// summary:
//      print version message.
// arg:
//      progname: program name = argv[0]
//      version: version string
//      footer: footer string
// return:
//      void
void printVersion(const char* progname, const char* version, const char* footer)
{
	printf("%s %s\n", progname, version);
	if (footer != NULL)
	{
		printf("%s\n", footer);
	}
}
