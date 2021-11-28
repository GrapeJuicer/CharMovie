#include "char_movie.hpp"

// CharAnalyze �̃R���X�g���N�^
// args:
//      text: ��͂���摜�Ŏg�p����Ă��镶����
//      file_path: �摜�̃t�@�C���p�X
//      threshold: ��͂���摜�̓�l����臒l
CharAnalyze::CharAnalyze(string text, string file_path, unsigned char threshold)
{
	this->text = text;
	this->image = cv::imread(file_path);
	// �G���[����
	if (this->image.empty())
	{
		throw runtime_error("Image is empty");
	}
	this->threshold = threshold;
}

// CharAnalyze �̃f�X�g���N�^
CharAnalyze::~CharAnalyze()
{
	this->image.release();
	this->work_image.release();
}

// �摜�̉�͂��s��
// return:
//      0: ����I�� / -1: �G���[
int CharAnalyze::analyze()
{
	int x, y;

	// �O���[�X�P�[���� & ��l��
	cv::cvtColor(this->image, this->work_image, cv::COLOR_BGR2GRAY);
	// 2�l��
	cv::threshold(this->work_image, this->work_image, this->threshold, 255, cv::THRESH_BINARY);

	// ��_��T��
	this->locateBasePosition(x, y);
	// �̈�̕��ƍ������擾
	this->calItemSize(x, y);
	// �g�̕��ƍ������擾
	this->calFrameSize(x, y);

	// �����ƔZ�x�̊֌W���v�Z���ēo�^
	return this->setCharList(x, y);
}

// ��_��T��
// args:
//      x: ��_��x���W
//      y: ��_��y���W
// return:
//      0: ����I�� / -1: �G���[
int CharAnalyze::locateBasePosition(int& x, int& y)
{
	int base_x, base_y;

	// ��_���v�Z
	// (y, x) = (0,0), (0,1), (1,0), (0,2), (1,1), (2,0), ...
	base_x = 0;
	base_y = 0;
	while (this->work_image.at<uchar>(base_y, base_x) == 255)
	{
		if (base_x == this->work_image.cols - 1 && base_y == this->work_image.rows - 1)
		{
			// ������Ȃ�����
			return -1;
		}

		if (base_x == 0 || base_y == this->work_image.rows - 1)
		{
			base_x = base_x + base_y + 1;
			base_y = 0;

			if (base_x >= this->work_image.cols)
			{
				int d = base_x - this->work_image.cols + 1;
				base_x -= d;
				base_y += d;
			}
		}
		else
		{
			base_y++;
			base_x--;
		}
	}

	// ���̂Ƃ��Cbase_x, base_y �̃s�N�Z�����F���قȂ�D����_
	x = base_x;
	y = base_y;

	return 0;
}

// �����̈�̑傫�����v�Z
// args:
//      x: ��_��x���W
//      y: ��_��y���W
// return:
//      0: ����I�� / -1: �G���[
int CharAnalyze::calItemSize(int x, int y)
{
	// �����v�Z
	// ���ł͕������̈悩��͂ݏo�����Ƃ�����̂ŁC�̈�̈�ԏ�̃s�N�Z���Ǝg�p���Čv�Z
	int cnt = 0;
	while (this->work_image.at<uchar>(y, x + cnt) != 255)
	{
		cnt++;
		if (x + cnt >= this->work_image.cols)
		{
			// �񐔂𒴂���
			return -1;
		}
	}
	this->item_width = cnt; // cnt ����

	// �������v�Z
	// �����ł͗̈���ɏ����ꂽ�����̉e����
	cnt = 1;
	while (1)
	{
		int i;
		for (i = 0; i < this->work_image.cols; i++)
		{
			if (y + cnt >= this->work_image.rows)
			{
				// �s�����z����
				return -1;
			}
			if (this->work_image.at<uchar>(y + cnt, i) != 255)
			{
				break;
			}
		}
		// ��s���ׂĂ� white �̂Ƃ�
		if (i == this->work_image.cols)
		{
			break;
		}

		// ���̍s��
		cnt++;
	}
	this->item_height = cnt; // cnt ������

	return 0;
}

// �g�̑傫�����v�Z
// args:
//      x: ��_��x���W
//      y: ��_��y���W
// return:
//      void
void CharAnalyze::calFrameSize(int x, int y)
{
	//�g�̑傫�����擾
	int fx = x + this->item_width, fy = y + this->item_height;
	while (this->work_image.at<uchar>(y, fx) == 255)
	{
		fx++;
		if (fx >= this->work_image.cols)
		{
			// �������ɂȂ� = �񐔂� 1
			break;
		}
	}
	while (this->work_image.at<uchar>(fy, x) == 255)
	{
		fy++;
		if (fy >= this->work_image.rows)
		{
			// �������ɂȂ� = �s���� 1
			break;
		}
	}
	// �g�̕�
	this->frame_width = fx >= this->work_image.cols ? this->work_image.cols : fx - x - this->item_width;
	// �g�̍���
	this->frame_height = fy >= this->work_image.rows ? this->work_image.cols : fy - y - this->item_height;
}

// �����ƔZ�x�̊֌W���v�Z���ēo�^
// args:
//      x: ��_��x���W
//      y: ��_��y���W
// return:
//      0: ����I�� / -1: �G���[
int CharAnalyze::setCharList(int x, int y)
{
	int c_index = 0;

	for (int tmpy = y; tmpy < this->work_image.rows; tmpy += this->item_height + this->frame_height)
	{
		for (int tmpx = x; tmpx < this->work_image.cols; tmpx += this->item_width + this->frame_width)
		{
			// �s�N�Z���̐F���`�F�b�N
			if (this->work_image.at<uchar>(tmpy, tmpx) == 255)
			{
				// �������Ȃ��D��{�I�ɂ͂�����for�����I������D
				continue;
			}

			// �g���~���O�͈�
			// (tmpx, tmpy), w: item_width, h: item_height

			// ���ϔZ�x�̃`�F�b�N
			double con_ratio = CharAnalyze::averageConcentration(this->work_image, tmpx, tmpy, this->item_width, this->item_height) / 255.0;

			// �o�^
			if (c_index >= this->text.size())
			{
				return -1;
			}
			this->char_list.push_back({ con_ratio, this->text[c_index] });
			c_index++;
		}
	}

	return 0;
}

// �͈͓��̉�f�̕��ϔZ�x���v�Z
// args:
//      image: �摜
//      x: x���W
//      y: y���W
//      w: ��
//      h: ����
// return:
//      ���ϔZ�x
double CharAnalyze::averageConcentration(const cv::Mat& image, int x, int y, int w, int h)
{
	// �l�̃`�F�b�N
	if (x < 0 || x >= image.cols || y < 0 || y >= image.rows || w <= 0 || w > image.cols || h <= 0 || h > image.rows)
	{
		throw runtime_error("Invalid position");
	}

	long line_sum;
	double sum = 0;

	// �e�s�̕��ς̘a�����߂�
	for (int i = y; i < y + h; i++)
	{
		// ������
		line_sum = 0;
		// ��s�̒l�̘a�����߂�
		for (int j = x; j < x + w; j++)
		{
			line_sum += image.at<uchar>(i, j);
		}
		// ���ς����߂�
		sum += (double)line_sum / w;
	}

	// ���ς����߂�
	return sum / h;
}

// �����ƔZ�x�̊֌W���t�@�C���ɏo�͂���
// args:
//      path: �o�͐�̃p�X
// return:
//      0: ����I�� / -1: �G���[
int CharAnalyze::exportCharList(string path)
{
	// �t�@�C�����J��
	fstream ofs(path, ios::out);
	if (!ofs.is_open())
	{
		return -1;
	}

	// ���@���o��
	ofs << this->item_width << " " << this->item_height << endl;

	// �������w��
	ofs << fixed << std::setprecision(6);

	// ���X�g���o��
	for (cmap::iterator it = this->char_list.begin(); it != this->char_list.end(); it++)
	{
		ofs << it->second << it->first << endl;
	}

	// �������f�t�H���g�ɖ߂�
	ofs << defaultfloat;

	ofs.close();

	return 0;
}

// �����ƔZ�x�̊֌W����ʂɏo�͂���
// args:
//      none
// return:
//      void
void CharAnalyze::printCharList()
{
	// ���@���o��
	cout << this->item_width << " " << this->item_height << endl;

	cout << fixed << std::setprecision(6);

	// ���X�g���o��
	for (int i = 0; i < this->char_list.size(); i++)
	{
		cout << '\'' << this->char_list[i].second << '\'' << ": " << this->char_list[i].first << "\t" << ends;

		if ((i + 1) % 8 == 0)
		{
			cout << endl;
		}
	}
	cout << defaultfloat << endl;
}
