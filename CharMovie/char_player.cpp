#include "char_movie.hpp"
#include "char_list.hpp"

namespace cm
{
	// CharImageViewer�̃R���X�g���N�^
	// arg:
	//      char_style_path: �X�^�C���̃p�X
	//      characters: �A�X�L�[�A�[�g�Ɏg�p���镶����
	//      ratio: �A�X�L�[�A�[�g�̃T�C�Y�䗦
	CharImageViewer::CharImageViewer(string char_style_path, string characters, double ratio)
	{
		// �����}�b�v�̓ǂݍ���
		// ������ width �� height ���ݒ肳���
		this->loadCharMap(char_style_path, characters);

		// ratio�̐ݒ� (�ŏ��l����������ꍇ�͍ŏ��l�ɏC��)
		double min_val = 1.0 / min<int>(this->width, this->height);
		this->ratio = max<double>(ratio, min_val);
	}

	// CharImageViewer�̃f�X�g���N�^
	CharImageViewer::~CharImageViewer()
	{
	}

	// �X�^�C���̓ǂݍ���
	// arg:
	//      char_style_path: �X�^�C���t�@�C���̃p�X
	//      characters: �A�X�L�[�A�[�g�Ɏg�p���镶����
	// return:
	//      void
	void CharImageViewer::loadCharMap(string char_style_path, string characters)
	{
		char c;
		double val;
		string line;
		// �X�^�C���t�@�C���̓ǂݍ���
		fstream fin(char_style_path, ios::in);
		if (!fin.is_open())
		{
			throw runtime_error("Can't open the file: " + char_style_path);
		}

		// ���@���擾
		fin >> this->width >> this->height;
		// �ǂݎ̂Ă�
		getline(fin, line);
		// �e�s��ǂݍ���
		while (getline(fin, line))
		{
			//�v�f�𕪂���
			c = line[0];
			val = stod(line.substr(1));

			// c �� characters �Ɋ܂܂�Ă��邩�`�F�b�N
			if (characters.find(c) != string::npos)
			{
				// �܂܂�Ă�����ǉ�
				this->char_list.push_back({ val, c });
			}
		}

		// ���������������Ƀ\�[�g
		sort(this->char_list.begin(), this->char_list.end());

		fin.close();
	}

	// �A�X�L�[�A�[�g��`��
	// arg:
	//      image: �A�X�L�[�A�[�g�ŕ`�悷��摜
	// return:
	//      void
	void CharImageViewer::show(const cv::Mat& image)
	{
		cv::Mat gray_image;
		int pxw, pxh;     // 1�����ŕ\������͈͂̃s�N�Z����
		int w, h;         // �C����̕��ƍ���
		double con_ratio; // �Z�x�̔䗦
		string chimg;

		// �O���[�X�P�[���ɕϊ�
		cv::cvtColor(image, gray_image, cv::COLOR_BGR2GRAY);

		// 1����������̃s�N�Z�������v�Z
		pxw = max<int>((int)(this->width * this->ratio), 1);
		pxh = max<int>((int)(this->height * this->ratio), 1);

		// �e�͈͂ŌJ��Ԃ�
		for (int i = 0; i < gray_image.rows; i += pxh)
		{
			for (int j = 0; j < gray_image.cols; j += pxw)
			{
				// �g���~���O����T�C�Y������
				w = min<int>(pxw, gray_image.cols - j);
				h = min<int>(pxh, gray_image.rows - i);

				// �Ώ۔͈� -> ������W: (j, i), width: w, height: h

				// �Z�x�̔䗦���v�Z
				// 0: ����, 255: ���� --> �� 1----0 ��
				con_ratio = CharAnalyze::averageConcentration(gray_image, j, i, w, h) / 255.0;

				// ����������
				chimg += this->selectChar(con_ratio);
			}
			chimg += '\n';
		}

		// �\��
		cout << chimg << ends;

		gray_image.release();
	}

	// Web�J�����̉摜��`��
	void CharImageViewer::show(int webcam_id)
	{
		cv::Mat img;
		// web�J�����ɐڑ�
		cv::VideoCapture cam(webcam_id);
		if (!cam.isOpened())
		{
			throw runtime_error("failed to cennect to web camera.");
		}

		// 1�t���[���擾
		cam.read(img);

		// �\��
		this->show(img);

		img.release();
		cam.release();
	}


	// ����������
	// arg:
	//      con_ratio: �Z�x�̔䗦
	// return:
	//      ����
	char CharImageViewer::selectChar(double con_ratio)
	{
		// ��O����
		if (con_ratio < 0 || con_ratio > 1)
		{
			throw runtime_error("con_ratio must be between 0 and 1.");
		}

		// ���X�g�̏��
		static double range = this->char_list.back().first - this->char_list.front().first;
		static pair<double, char> min_val = this->char_list.front();
		static size_t lsize = this->char_list.size();

		// ���ΓI�ɒl������
		double con_ratio_norm = (range * con_ratio) /*���ΓI�Ȓl*/ + min_val.first /*�ŏ��l*/;

		// ���������������Ɍ���
		for (int i = 0; i < lsize; i++)
		{
			// ���X�g�̒l�� con_ratio_norm �ȏ�ɂȂ� -> i�Ԗڂ�i-1�Ԗڂ̂ǂ��炩���ł��߂��l
			if (this->char_list[i].first >= con_ratio_norm)
			{
				// �擪�̎�
				if (i == 0)
				{
					// 1�O���Ȃ��̂Ō��݂̒l������
					return min_val.second;
				}
				else
				{
					// 1�O�̒l�Ɣ�r���āC���߂��ق��Ɍ���
					double diff1 = this->char_list[i].first - con_ratio_norm;
					double diff2 = con_ratio_norm - this->char_list[i - 1].first;
					return diff1 < diff2 ? this->char_list[i].second : this->char_list[i - 1].second;
				}
			}
		}

		// if �ɂЂ�������Ȃ������ꍇ�͈�Ԓl�̑傫�����̂Ɍ���
		return this->char_list.back().second;
	}

	// CharVideoPlayer�̃R���X�g���N�^
	// arg:
	//      char_style_path: �X�^�C���̃p�X
	//      characters:�A�X�L�[�A�[�g�Ɏg�p���镶����
	//      ratio: �A�X�L�[�A�[�g�̃T�C�Y�䗦
	//      speed: �Đ����x
	//      draw_method: �`�����
	CharVideoPlayer::CharVideoPlayer(string char_style_path, string characters, double ratio, double speed, int draw_method) : CharImageViewer(char_style_path, characters, ratio)
	{
		// ���x�̐ݒ�
		if (speed <= 0)
		{
			throw runtime_error("speed must be greater than 0.");
		}
		this->speed = speed;
		this->status = cm::play;
		this->draw_method = draw_method;
	}

	// CharVideoPlayer�̃f�X�g���N�^
	CharVideoPlayer::~CharVideoPlayer()
	{
	}


	// 1�t���[��������̎��Ԃ��v�Z
	// arg:
	//      video: ����
	// return:
	//      1�t���[��������̎���
	int CharVideoPlayer::calcurateDuration(cv::VideoCapture& video)
	{
		//�t���[����
		int frame_size = (int)video.get(cv::CAP_PROP_FRAME_COUNT);
		// fps
		double fps = video.get(cv::CAP_PROP_FPS);
		// 1�t���[���̎���
		return (int)(1000.0 / fps / this->speed);
	}


	// ������Đ�
	// arg:
	//      video: �Đ����铮��
	//      ms_per_frame: 1�t���[��������̎���(ms). 0�����Ŏ����ݒ�D(Default: -1)
	// return:
	//      void
	void CharVideoPlayer::show(cv::VideoCapture& video, int ms_per_frame)
	{
		cv::Mat frame;
		//�t���[����
		int frame_size = (int)video.get(cv::CAP_PROP_FRAME_COUNT);
		// chrono �Ō��ݎ������擾
		chrono::steady_clock::time_point target_time;

		chrono::milliseconds duration_time;
		if (ms_per_frame < 0)
		{
			duration_time = chrono::milliseconds(this->calcurateDuration(video));
		}
		else
		{
			duration_time = chrono::milliseconds(ms_per_frame);
		}

		// ��ʂ̃N���A
		switch (this->draw_method)
		{
			case cm::override :
			case cm::clear:
				// �G�X�P�[�v�V�[�P���X�ɂ���ʂ̃N���A
				cout << clist::control::Control_ClearScreen << clist::control::Control_MoveToHead << ends;
				break;
			case cm::clear_old:
				// cls �R�}���h�ɂ���ʂ̃N���A
				system("cls");
				break;
			default:
				throw runtime_error("draw_method is invalid.");
		}

		// ����̍Đ�
		for (int cfn = 0; cfn < frame_size; cfn++)
		{
			// pause�Ȃ�ꎞ��~
			while (this->status == cm::pause);
			// quit�Ȃ�I��
			if (this->status == cm::quit)
			{
				break;
			}

			target_time = chrono::steady_clock::now();

			// �t���[���̎擾
			video.read(frame);
			// �`�F�b�N (�擾�ł��Ȃ�������I��)
			if (frame.empty())
			{
				throw runtime_error("frame is empty.");
			}

			// �J�[�\����擪�Ɉړ� -> ��ʂ̃N���A�ɓ���
			// ���̎�@���� �e�B�A�����O ����������\������
			// �����ʃN���A����� ����� ����������\������
			// system("cls");

			// ��ʂ̃N���A
			switch (this->draw_method)
			{
				case cm::override :
					// �G�X�P�[�v�V�[�P���X�ɂ��J�[�\���̈ړ� (��ʂ̃N���A�����Ȃ�)
					cout << clist::control::Control_MoveToHead << ends;
					break;
				case cm::clear:
					// �G�X�P�[�v�V�[�P���X�ɂ���ʂ̃N���A�ƃJ�[�\���̈ړ�
					cout << clist::control::Control_ClearScreen << clist::control::Control_MoveToHead << ends;
					break;
				case cm::clear_old:
					// cls �R�}���h�ɂ���ʂ̃N���A
					system("cls");
					break;
			}

			// �t���[���̍Đ�
			CharImageViewer::show(frame);

			// ���̃t���[���܂őҋ@
			this_thread::sleep_until(target_time + duration_time);
		}

		// �L�[���͂ɂ��I���ł͂Ȃ��ꍇ�Cstatus �� end �ɕύX
		if (this->status != cm::quit)
		{
			this->status = cm::end;
		}
	}


	// Web�J�����̉f�����Đ�
	// arg:
	//      video: �Đ����铮��
	//      ms_per_frame: 1�t���[��������̎���(ms). 0�����Ŏ����ݒ�D(Default: -1)
	// return:
	//      void
	void CharVideoPlayer::show(int webcam_id)
	{
		cv::VideoCapture cam(webcam_id);
		cv::Mat frame;
		int duration_time = 1;

		if (!cam.isOpened())
		{
			throw runtime_error("failed to cennect to web camera.");
		}

		// ��ʂ̃N���A
		switch (this->draw_method)
		{
			case cm::override :
			case cm::clear:
				// �G�X�P�[�v�V�[�P���X�ɂ���ʂ̃N���A
				cout << clist::control::Control_ClearScreen << clist::control::Control_MoveToHead << ends;
				break;
			case cm::clear_old:
				// cls �R�}���h�ɂ���ʂ̃N���A
				system("cls");
				break;
			default:
				throw runtime_error("draw_method is invalid.");
		}

		int x = 0;
		// ����̍Đ�
		while (true)
		{
			// pause�Ȃ�ꎞ��~
			while (this->status == cm::pause);

			// quit�Ȃ�I��
			if (this->status == cm::quit)
			{
				break;
			}

			// �t���[���̎擾
			cam.read(frame);
			// �`�F�b�N (�擾�ł��Ȃ�������I��)
			if (frame.empty())
			{
				throw runtime_error("frame is empty.");
			}

			// �J�[�\����擪�Ɉړ� -> ��ʂ̃N���A�ɓ���
			// ���̎�@���� �e�B�A�����O ����������\������
			// �����ʃN���A����� ����� ����������\������
			// system("cls");

			// ��ʂ̃N���A
			switch (this->draw_method)
			{
				case cm::override :
					// �G�X�P�[�v�V�[�P���X�ɂ��J�[�\���̈ړ� (��ʂ̃N���A�����Ȃ�)
					cout << clist::control::Control_MoveToHead << ends;
					break;
				case cm::clear:
					// �G�X�P�[�v�V�[�P���X�ɂ���ʂ̃N���A�ƃJ�[�\���̈ړ�
					cout << clist::control::Control_ClearScreen << clist::control::Control_MoveToHead << ends;
					break;
				case cm::clear_old:
					// cls �R�}���h�ɂ���ʂ̃N���A
					system("cls");
					break;
			}

			// �t���[���̍Đ�
			CharImageViewer::show(frame);

			// ��u�ҋ@
			this_thread::sleep_for(chrono::milliseconds(1));
			//cv::waitKey(duration_time);
		}

		cam.release();
	}
}