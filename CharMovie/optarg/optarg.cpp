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

	// short option �ɂċ֎~�I�v�V���� '?' ���g�p���Ă���D
	if (strchr(shortopts, '?') != NULL)
	{
		return -1;
	}

	i = 0;
	while (!(longopts[i].name == 0 && longopts[i].has_arg == 0 && longopts[i].flag == 0 && longopts[i].val == 0))
	{
		// long option �ɂċ֎~�I�v�V���� '?' ���g�p���Ă���D
		if (longopts[i].val == '?')
		{
			return -1;
		}
		i++;
	}

	// ���ʂ��i�[����z��̏�����
	for (i = 0; i < len_findopts; i++)
	{
		findopts[i].opt = -1;
	}

	while ((opt = getopt_long(argc, argv, shortopts, longopts, &loptindex)) != -1)
	{
		// ������Ȃ������ꍇ�͉������Ȃ�
		if (opt == '?')
		{
			continue;
		}

		// ���Ɍ������Ă��邩�`�F�b�N
		for (i = 0; i < cnt; i++)
		{
			if (opt == findopts[i].opt)
			{
				break;
			}
		}

		// ���Ɉ�x�������Ă���Ȃ牽�����Ȃ�
		if (i < cnt)
		{
			continue;
		}

		// �z��̃T�C�Y�𒴂�����G���[
		if (cnt >= len_findopts)
		{
			return -1;
		}

		// �ǉ�
		findopts[cnt].opt = opt;
		findopts[cnt].arg = optarg;

		cnt++;
	}

	return cnt;
}
