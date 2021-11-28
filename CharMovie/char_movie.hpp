#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <fstream>
#include <cmath>
#include <thread>
#include <iomanip>
#include <map>
#include <conio.h> // _getch()
#include <iostream>

using namespace std;

typedef vector<pair<double, char>> cmap;
typedef vector<pair<int, int>> keymap;

class CharAnalyze
{
private:
	string text;
	cv::Mat image;
	int threshold;
	cmap char_list;

	cv::Mat work_image; // ��Ɨp
	int item_width;
	int item_height;
	int frame_width;
	int frame_height;

	int locateBasePosition(int& x, int& y);
	int calItemSize(int x, int y);
	void calFrameSize(int x, int y);
	int setCharList(int x, int y);

public:
	CharAnalyze(string text, string file_path, unsigned char threshold = 127);
	~CharAnalyze();
	int analyze();
	static double averageConcentration(const cv::Mat& image, int x, int y, int w, int h);
	int exportCharList(string path);
	void printCharList();
};

namespace cm
{
	class CharImageViewer
	{
	private:
		unsigned int width;
		unsigned int height;
		double ratio;
		cmap char_list; // ����, ����

		void loadCharMap(string char_style_path, string characters);

		char selectChar(double con_ratio);

	public:
		CharImageViewer(string char_style_path, string characters, double ratio = 1.0);
		~CharImageViewer();
		void show(const cv::Mat& image);
		void show(int webcam_id);
	};

	// �`�����
	enum
	{
		override = 0,
		clear,
		clear_old,
	};

	// ����X�e�[�^�X
	enum
	{
		play = 0, // �Đ���
		pause,    // �ꎞ��~
		quit,     // �L�[���͂ɂ��I��
		end,      // ����I��
	};

	class CharVideoPlayer : public CharImageViewer
	{
	private:
		double speed;
		int draw_method;

	public:
		volatile int status;
		CharVideoPlayer(string char_style_path, string characters, double ratio = 1.0, double speed = 1.0, int draw_method = cm::override);
		~CharVideoPlayer();
		int calcurateDuration(cv::VideoCapture& video);
		void show(cv::VideoCapture& video, int ms_per_frame = -1);
		void show(int webcam_id);
	};
}
