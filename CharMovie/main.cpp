/*
cmovie.exe --help
cmovie.exe --version
cmovie.exe --sample [--character <chars>] [--ouput <chars-file>]
cmovie.exe --analyze [--character <chars>] [--threshold <0-255>] [--ouput <chars-file>] <image>
cmovie.exe --info [--image] <image|video>
cmovie.exe --image [--character <chars>] [--ratio <gt.0>] [--style <style-file>] <image>
cmovie.exe [--character <chars>] [--ratio <gt.0>] [--speed <gt.0>] [--draw <0-2>] [--style <style-file>] <video>
*/

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include "char_list.hpp"
#include "char_movie.hpp"
#include "optarg/optarg.hpp"
#include <opencv2/opencv.hpp>
#include <cstdlib>

using namespace std;

// �I�v�V�������ʗpID
enum
{
	opt_help = 1001,
	opt_version,
	opt_sample,
	opt_char,
	opt_analyze,
	opt_threshold,
	opt_output,
	opt_ratio,
	opt_speed,
	opt_image,
	opt_draw,
	opt_style,
	opt_info,
	opt_webcam,
};

// �v���g�^�C�v�錾
string getEnvVariable(const char* name);
int    printSample(unsigned int width, string src, bool do_output, string output_path);
int    analyzeSample(string characters, string img_path, int threshold, bool do_output, string output_path);
void   printVideoInfo(const cv::VideoCapture& video);
int    printMediaInfo(string media_path, bool is_image);
int    printImage(string image_path, string cstyle_path, string characters, double ratio);
int    printVideo(string video_path, string cstyle_path, string characters, double ratio, double speed, int draw_method);
void   inputKey(const keymap km, volatile int& status);
void   printImageInfo(const cv::Mat& img);
void   printVideoInfo(const cv::VideoCapture& video);
int    printWebcam(int webcam_id, string cstyle_path, string characters, double ratio, int draw_method);
int    printWebcamImage(int webcam_id, string cstyle_path, string characters, double ratio);

int main(int argc, char* argv[])
{
	// info
	const char* version = "1.0.0";
	const char* header = "\
Specific Usage of Arguments:\n\
  ... --help\n\
  ... --version\n\
  ... --sample [--character <chars>] [--ouput <chars-file>]\n\
  ... --analyze [--character <chars>] [--threshold <0-255>] [--ouput <chars-file>] <image>\n\
  ... --info [--image] <image|video>\n\
  ... --image [--character <chars>] [--ratio <gt.0>] [--style <style-file>] <image>\n\
  ... [--character <chars>] [--ratio <gt.0>] [--speed <gt.0>] [--draw <0-2>] [--style <style-file>] <video>\n\
  ... --webcam [gt.0] [--character <chars>] [--ratio <gt.0>] [--draw <0-2>] [style <style-file>]\n\
  ... --webcam [gt.0] [--image] [--character <chars>] [--ratio <gt.0>] [style <style-file>]\n\
\n\
KeyMap:\n\
  'q'      Quit video.\n\
  SPACE    Pause/Resume video.";
	const char* copyright = "Copyright (c) 2021, Grape Juice.";

	// style
	const char* dusage = "[OPTION]... <image|video>";
	//struct docstyle dstyle = { .indent = "  ", .separator = " , ", .margin = "    ", .doc_width = 80 };
	struct docstyle dstyle = { "  ", " , ", "    ", 80 };
	// sample�\���ɂ������I�v�V����
	bool do_sample = false;
	int sample_width = 20;
	string sample_chs = " 1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!?\'\"`/\\,.#$%&()[]{}<>-=~^|@:;+*_";

	// ���(analyze)�Ɋւ��I�v�V����
	bool do_analyze = false;
	int threshold = 127;

	// ���\���Ɋւ��I�v�V����
	bool do_info = false;

	// �摜 or ����̕\���Ɋւ��I�v�V����
	bool is_image = false;
	bool is_old = false;
	double ratio = 1.0;
	double speed = 1.0;
	int draw_method = cm::override;
	string cstyle_path = getEnvVariable("USERPROFILE") + "\\cmovie_character_style_template.txt";

	// Web�J�����Ɋւ��I�v�V����
	bool is_webcam = false;
	int webcam_id = 0;

	// �t�@�C���o�͂Ɋւ���I�v�V����
	bool do_output = false;
	string output_path;

	// �I�v�V�����̐ݒ�
	const struct docoption opts[] = {
		{opt_help     , 'h', "help"     , no_argument      , "Show this help."},
		{opt_version  , 'v', "version"  , no_argument      , "Show version."},
		{opt_sample   , 's', "sample"   , no_argument      , "Displays sample characters for style."},
		{opt_char     , 'c', "character", required_argument, "Set source characters. Default: a-zA-Z0-9 and all symbols."},
		{opt_analyze  , 'a', "analyze"  , no_argument      , "Analyze the captured sample characters."},
		{opt_threshold, 't', "threshold", required_argument, "Set the threshold value. It is used when \'analyze\' option enabled. Default: 127"},
		{opt_output   , 'o', "output"   , required_argument, "Set the output file path. It is used when \'analyze\' option enabled. Default: stdout"},
		{opt_ratio    , 'r', "ratio"    , required_argument, "Set the ratio value. Default: 1.0\nNote: If value is less than the minimum, it\'ll be corrected to the minimum."},
		{opt_speed    , 'p', "speed"    , required_argument, "Set the speed value. Default: 1.0, 0.25-2.0 is recommended.\nIf the value is too large, the processing may not be able to keep up and the actual magnification may be smaller than the specified value."},
		{opt_image    , 'i', "image"    , no_argument      , "Use an image instead of a video."},
		{opt_draw     , 'd', "draw"     , required_argument, "Set Drawing method.\n  0: Override. (Default)\n  1: Clear->Write.\n  2: \'cls\'->Write.\nNote 1: Use 2 if your terminal doesn\'t support clearing the screen with escape sequences.\nNote 2: When set to 0, tearing may occur; when set to 1 or 2, flickering will occur in most cases."},
		{opt_style    , 'S', "style"    , required_argument, "Set character styles file path corresponding to colors. Default: \'%USERPROFILE%/cmovie_character_style_template.txt\'."},
		{opt_info     , 'I', "info"     , no_argument      , "Show image/video information."},
		{opt_webcam   , 'w', "webcam"   , required_argument, "Draw the image from the web camera. The camera number is required as an argument; if there is only one webcam, specify 0."},
		DOCOPT_END
	};

	// ��͌��ʂ��i�[����ϐ�
	const int     fosize = 10;
	struct optarg findopts[fosize];
	// ���o���ꂽ�I�v�V�����̐����i�[����ϐ�
	int folen;

	// �I�v�V�����̉��
	folen = getopt_flex(argc, argv, opts, findopts, fosize);

	// ���o���ꂽ�I�v�V�����̏���
	for (int i = 0; i < folen; i++)
	{
		switch (findopts[i].opt)
		{
		case opt_help:
			printHelp(opts, argv[0], dusage, dstyle, header, copyright);
			return 0;

		case opt_version:
			printVersion(argv[0], version, copyright);
			return 0;

		case opt_sample:
			do_sample = true;
			break;

		case opt_char:
			sample_chs = string(findopts[i].arg);
			break;

		case opt_analyze:
			do_analyze = true;
			break;

		case opt_threshold:
			threshold = atoi(findopts[i].arg);
			break;

		case opt_output:
			do_output = true;
			output_path = string(findopts[i].arg);
			break;

		case opt_ratio:
			ratio = atof(findopts[i].arg);
			break;

		case opt_speed:
			speed = atof(findopts[i].arg);
			break;

		case opt_image:
			is_image = true;
			break;

		case opt_draw:
			draw_method = atoi(findopts[i].arg);
			break;

		case opt_style:
			cstyle_path = string(findopts[i].arg);
			break;

		case opt_info:
			do_info = true;
			break;

		case opt_webcam:
			is_webcam = true;
			webcam_id = atoi(findopts[i].arg);
			break;
		}
	}

	// ��O����
	if  (folen == -1 || // �I�v�V������͂Ɏ��s
		(do_sample && argc - optind != 0) || // -s �I�v�V�������L���̎� : �I�v�V�����ȊO�̈����� 0 �łȂ�
		(is_webcam && argc - optind != 0) || // -w �I�v�V�������L���̎� : �I�v�V�����ȊO�̈����� 0 �łȂ�
		(!do_sample && !is_webcam && argc - optind != 1))  // ����ȊO�̂Ƃ� : �I�v�V�����ȊO�̈����� 1 �łȂ�
	{
		cout << "Error: Invalid arguments. Please see the help using \'-h\' or \'--help\'." << endl;
		return -1;
	}

	try
	{
		// �T���v���\��
		if (do_sample)
		{
			return printSample(sample_width, sample_chs, do_output, output_path);
		}
		// ���
		else if (do_analyze)
		{
			return analyzeSample(sample_chs, string(argv[optind]), threshold, do_output, output_path);
		}
		// ���\��
		else if (do_info)
		{
			return printMediaInfo(string(argv[optind]), is_image);
		}
		// webcam �摜�̕\��
		else if (is_image && is_webcam)
		{
			return printWebcamImage(webcam_id, cstyle_path, sample_chs, ratio);
		}
		// webcam �f���̕\��
		else if (is_webcam)
		{
			return printWebcam(webcam_id, cstyle_path, sample_chs, ratio, draw_method);
		}
		// �摜�̕\��
		else if (is_image)
		{
			return printImage(argv[optind], cstyle_path, sample_chs, ratio);
		}
		// ����̕\��
		else
		{
			return printVideo(argv[optind], cstyle_path, sample_chs, ratio, speed, draw_method);
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return -1;
	}

	return 0;
}

// ���ϐ����擾����
// arg:
//      name: ���ϐ���
// return:
//      ���ϐ��̒l
string getEnvVariable(const char* name)
{
	const char* tmp = getenv(name);
	string env_var(tmp ? tmp : "");
	if (env_var.empty()) {
		throw runtime_error("Environment variable " + string(name) + " not set");
	}
	return env_var;
}

// �T���v���f�[�^��\������
// arg:
//      width:       �T���v��������̕� (=1�s�ɕ\������v�f�̐�)
//      chs:         �T���v��������
//      do_output:   �o�̓t�@�C�����w�肵���ꍇ�� true
//      output_path: �o�̓t�@�C���̃p�X
int printSample(unsigned int width, string src, bool do_output, string output_path)
{
	// print list
	cout << "Character Sample List :" << src << endl;

	if (clist::charList(width, src))
	{
		cerr << "Error: Character list contains duplicate character." << endl;
		return -1;
	}
	else
	{
		cout << "Take a screenshot of the above character list." << endl;
		cout << "  Note 1: Take it that includes all the character in their entirety and does not include the outside of the white frame." << endl;
		cout << "  Note 2: Sometimes the white frame may not be partially displayed. In this case, please execute the command again." << endl;

		// �t�@�C���ւ̏o��
		if (do_output)
		{
			// src �� ouput_path �ɏo��
			ofstream ofs(output_path);
			if (!ofs)
			{
				cerr << "Error: Failed to open the output file." << endl;
				return -1;
			}

			// �o��
			ofs << src;

			ofs.close();

			// log
			cout << endl;
			cout << "Sample source written to " << output_path << endl;
		}
	}

	return 0;
}

// �T���v���f�[�^�̉摜(�L���v�`����������)����͂���
// arg:
//      characters:  ��͑Ώۂ̉摜�Ɏg�p���ꂽ������
//      img_path:    �摜�t�@�C���̃p�X
//      threshold:   2�l���̍ۂ�臒l
//      do_output:   �o�̓t�@�C�����w�肵���ꍇ�� true
//      output_path: �o�̓t�@�C���̃p�X
// return:
//      0: ����I�� / -1: �G���[
int analyzeSample(string characters, string img_path, int threshold, bool do_output, string output_path)
{
	if (threshold < 0 || threshold > 255)
	{
		cerr << "Invalid threshold value. It must be between 0 and 255." << endl;
		return -1;
	}

	// �C���X�^���X��
	CharAnalyze analyzer(characters, img_path, threshold);

	// ���
	if (analyzer.analyze())
	{
		cerr << "Error: Failed to analyze the character list." << endl;
		return -1;
	}

	// ��͌��ʂ�\��
	analyzer.printCharList();

	// do_output �� true �Ȃ�t�@�C���o��
	if (do_output)
	{
		// �o��
		if (analyzer.exportCharList(output_path))
		{
			cerr << "Error: Failed to export the character list." << endl;
			return -1;
		}

		// log
		cout << "Analyzed character list written to " << output_path << endl;
	}

	return 0;
}

// �摜/����̏���\������
// arg:
//      media_path:  �摜/����t�@�C���̃p�X
//      is_image:    �摜�̏ꍇ�� true, ����̏ꍇ�� false
// return:
//      0: ����I�� / -1: �G���[
int printMediaInfo(string media_path, bool is_image)
{
	if (is_image)
	{
		// �摜���擾
		cv::Mat img = cv::imread(media_path);
		if (img.empty())
		{
			cerr << "Error: Failed to read the image." << endl;
			return -1;
		}
		// �摜����\��
		printImageInfo(img);

		img.release();
	}
	// ����̏ꍇ
	else
	{
		// ������擾
		cv::VideoCapture video(media_path);
		if (!video.isOpened())
		{
			cerr << "Error: Failed to open the video." << endl;
			return -1;
		}

		// �������\��
		printVideoInfo(video);

		video.release();
	}

	return 0;
}

// �摜���A�X�L�[�A�[�g�ŏo�͂���
// arg:
//      image_path:  �摜�t�@�C���̃p�X
//      cstyle_path: �X�^�C���̃p�X
//      characters:  �A�X�L�[�A�[�g�Ɏg�p���镶����
//      ratio:       �A�X�L�[�A�[�g�̃T�C�Y�䗦
// return:
//      0: ����I�� / -1: �G���[
int printImage(string image_path, string cstyle_path, string characters, double ratio)
{
	// cstyle_path ���󂩃`�F�b�N
	if (cstyle_path.empty())
	{
		cerr << "Error: Invalid style." << endl;
		return -1;
	}

	// characters ���󂩃`�F�b�N
	if (characters.empty())
	{
		cerr << "Error: Invalid characters." << endl;
		return -1;
	}

	// ratio��0�ȉ����`�F�b�N
	if (ratio <= 0)
	{
		cerr << "Invalid ratio value. It must be greater than 0." << endl;
		return -1;
	}

	// �摜���擾
	cv::Mat img = cv::imread(image_path);
	if (img.empty())
	{
		cerr << "Error: Failed to read the image." << endl;
		return -1;
	}

	// �摜�\���p�N���X�̐ݒ�
	cm::CharImageViewer imgvwr(cstyle_path, characters, ratio);

	// �摜��\��
	imgvwr.show(img);

	img.release();

	return 0;
}


// ������A�X�L�[�A�[�g�ŏo�͂���
// arg:
//      video_path:  ����t�@�C���̃p�X
//      cstyle_path: �X�^�C���̃p�X
//      characters:  �A�X�L�[�A�[�g�Ɏg�p���镶����
//      ratio:       �A�X�L�[�A�[�g�̃T�C�Y�䗦
//      speed:       ����Đ����x
//      draw_method: �`�����
// return:
//      0: ����I�� / -1: �G���[
int printVideo(string video_path, string cstyle_path, string characters, double ratio, double speed, int draw_method)
{
	// ��O����

	// cstyle_path ���󂩃`�F�b�N
	if (cstyle_path.empty())
	{
		cerr << "Error: Invalid style." << endl;
		return -1;
	}

	// characters ���󂩃`�F�b�N
	if (characters.empty())
	{
		cerr << "Error: Invalid characters." << endl;
		return -1;
	}

	// ratio��0�ȉ����`�F�b�N
	if (ratio <= 0)
	{
		cerr << "Invalid ratio value. It must be greater than 0." << endl;
		return -1;
	}

	// speed��0�ȉ����`�F�b�N
	if (speed <= 0)
	{
		cerr << "Invalid speed value. It must be greater than 0." << endl;
		return -1;
	}

	// draw_method��0~2�͈̔͊O���`�F�b�N
	if (draw_method < 0 || draw_method > 2)
	{
		cerr << "Invalid draw style. It must be between 0 and 2." << endl;
		return -1;
	}

	// ����Đ����̃L�[�̊��蓖��
	const keymap keys = {
		{cm::play, ' '},
		{cm::pause, ' '},
		{cm::quit, 'q'},
	};

	// ������擾
	cv::VideoCapture video(video_path);
	if (!video.isOpened())
	{
		cerr << "Error: Failed to open the video." << endl;
		return -1;
	}

	// ����\���p�̃N���X�̐ݒ�
	cm::CharVideoPlayer vidplayer(cstyle_path, characters, ratio, speed, draw_method);

	// �X���b�h���쐬
	thread vid_thread(inputKey, keys, ref(vidplayer.status));

	// �����\��
	vidplayer.show(video);

	// ���悪�Ō�܂ōĐ����ꂽ�ꍇ
	if (vidplayer.status == cm::end)
	{
		cout << endl;
		cout << "Video ended." << endl;
		cout << "Press any key to quit." << endl;
	}

	// �X���b�h�̏I����ҋ@
	vid_thread.join();

	video.release();

	return 0;
}

// ����Đ����̃L�[���͂��󂯕t����
// arg:
//      km: �L�[���蓖��
//      status: ����Đ����
// return:
//      void
void inputKey(const keymap km, volatile int& status)
{
	int key;

	// �L�[���͂ɂ���� status �� quit �ɂȂ�܂ŌJ��Ԃ�
	while (status != cm::quit)
	{
		// �L�[����
		// key = getch();
		key = _getch();

		// CharVideoPlayer���� status �� quit �ɂȂ����ꍇ�͑����ɏI��
		if (status == cm::end)
		{
			return;
		}

		// ����
		for (auto& k : km)
		{
			// ���悪�I�����Ă��Ȃ��āC��Ԃ��J�ڂ����ꍇ
			if (status != cm::end && status != k.first && key == k.second)
			{
				// �X�V
				status = k.first;
				break;
			}
		}
	}
}

// �摜�̏���\������
// arg:
//      img: �摜
// return:
//      void
void printImageInfo(const cv::Mat& img)
{
	// �摜�̊T�v��\��
	int i_w = img.cols;  //�c�̑傫��
	int i_h = img.rows;  //���̑傫��
	size_t i_len = img.total(); //��f��
	// �t�H�[�}�b�g���擾
	int i_fmt = img.type();

	// �o��
	cout << "Image Information:" << endl;
	cout << "  Size       : " << i_w << "x" << i_h << " px" << endl;
	cout << "  Pixel Size : " << i_len << " pixels" << endl;
	switch (i_fmt) // �t�H�[�}�b�g
	{
	case CV_8UC1:
		cout << "  Format     : 8-bit unsigned integer (1 channel)" << endl;
		break;
	case CV_8UC3:
		cout << "  Format     : 8-bit unsigned integer (3 channels)" << endl;
		break;
	case CV_8UC4:
		cout << "  Format     : 8-bit unsigned integer (4 channels)" << endl;
		break;
	case CV_16UC1:
		cout << "  Format     : 16-bit unsigned integer (1 channel)" << endl;
		break;
	case CV_16UC3:
		cout << "  Format     : 16-bit unsigned integer (3 channels)" << endl;
		break;
	case CV_16UC4:
		cout << "  Format     : 16-bit unsigned integer (4 channels)" << endl;
		break;
	case CV_32SC1:
		cout << "  Format     : 32-bit signed integer (1 channel)" << endl;
		break;
	case CV_32SC3:
		cout << "  Format     : 32-bit signed integer (3 channels)" << endl;
		break;
	case CV_32SC4:
		cout << "  Format     : 32-bit signed integer (4 channels)" << endl;
		break;
	case CV_32FC1:
		cout << "  Format     : 32-bit floating point (1 channel)" << endl;
		break;
	case CV_32FC3:
		cout << "  Format     : 32-bit floating point (3 channels)" << endl;
		break;
	case CV_32FC4:
		cout << "  Format     : 32-bit floating point (4 channels)" << endl;
		break;
	case CV_64FC1:
		cout << "  Format     : 64-bit floating point (1 channel)" << endl;
		break;
	case CV_64FC3:
		cout << "  Format     : 64-bit floating point (3 channels)" << endl;
		break;
	case CV_64FC4:
		cout << "  Format     : 64-bit floating point (4 channels)" << endl;
		break;
	default:
		cout << "  Format     : Unknown" << endl;
		break;
	}
}

// ����̏���\������
// arg:
//      video: ����
// return:
//      void
void printVideoInfo(const cv::VideoCapture& video)
{
	// ����̊T�v��\��
	int v_w = (int)video.get(cv::CAP_PROP_FRAME_WIDTH);  //�c�̑傫��
	int v_h = (int)video.get(cv::CAP_PROP_FRAME_HEIGHT); //���̑傫��
	double v_fps = video.get(cv::CAP_PROP_FPS);          //�t���[�����[�g
	int v_len = (int)video.get(cv::CAP_PROP_FRAME_COUNT);  //�t���[����
	double v_sec = (double)v_len / v_fps;                        //����̕b��

	cout << "Video Information:" << endl;
	cout << "  Size       : " << v_w << "x" << v_h << " px" << endl;
	cout << "  Frame Rate : " << v_fps << " fps" << endl;
	cout << "  Frame Size : " << v_len << " frames" << endl;
	cout << "  Time       : " << (int)v_sec / 3600 << " h " << ((int)v_sec % 3600) / 60 << " m " << (int)v_sec % 60 << " s" << endl;
}

// Web�J�����̉f����\������
// arg:
//      webcam_id:  Web�J������ID
//      cstyle_path: �X�^�C���̃p�X
//      characters:  �A�X�L�[�A�[�g�Ɏg�p���镶����
//      ratio:       �A�X�L�[�A�[�g�̃T�C�Y�䗦
//      draw_method: �`�����
// return:
//      0: ����I�� / -1: �G���[
int printWebcam(int webcam_id, string cstyle_path, string characters, double ratio, int draw_method)
{
	// ��O����

	// cstyle_path ���󂩃`�F�b�N
	if (cstyle_path.empty())
	{
		cerr << "Error: Invalid style." << endl;
		return -1;
	}

	// characters ���󂩃`�F�b�N
	if (characters.empty())
	{
		cerr << "Error: Invalid characters." << endl;
		return -1;
	}

	// ratio��0�ȉ����`�F�b�N
	if (ratio <= 0)
	{
		cerr << "Invalid ratio value. It must be greater than 0." << endl;
		return -1;
	}

	// draw_method��0~2�͈̔͊O���`�F�b�N
	if (draw_method < 0 || draw_method > 2)
	{
		cerr << "Invalid draw style. It must be between 0 and 2." << endl;
		return -1;
	}

	// Web�J�����Đ����̃L�[�̊��蓖��
	const keymap keys = {
		{cm::play, ' '},
		{cm::pause, ' '},
		{cm::quit, 'q'},
	};

	// �\���p�̃N���X�̐ݒ�
	cm::CharVideoPlayer vidplayer(cstyle_path, characters, ratio, 1.0, draw_method);

	// �X���b�h���쐬
	thread vid_thread(inputKey, keys, ref(vidplayer.status));

	// �����\��
	vidplayer.show(webcam_id);

	// �X���b�h�̏I����ҋ@
	vid_thread.join();

	return 0;
}

// Web�J�����̉摜��\������
// arg:
//      webcam_id:  Web�J������ID
//      cstyle_path: �X�^�C���̃p�X
//      characters:  �A�X�L�[�A�[�g�Ɏg�p���镶����
//      ratio:       �A�X�L�[�A�[�g�̃T�C�Y�䗦
// return:
//      0: ����I�� / -1: �G���[
int printWebcamImage(int webcam_id, string cstyle_path, string characters, double ratio)
{
	// ��O����

	// cstyle_path ���󂩃`�F�b�N
	if (cstyle_path.empty())
	{
		cerr << "Error: Invalid style." << endl;
		return -1;
	}

	// characters ���󂩃`�F�b�N
	if (characters.empty())
	{
		cerr << "Error: Invalid characters." << endl;
		return -1;
	}

	// ratio��0�ȉ����`�F�b�N
	if (ratio <= 0)
	{
		cerr << "Invalid ratio value. It must be greater than 0." << endl;
		return -1;
	}

	// �摜�\���p�N���X�̐ݒ�
	cm::CharImageViewer imgvwr(cstyle_path, characters, ratio);

	// �摜��\��
	imgvwr.show(webcam_id);

	return 0;
}
