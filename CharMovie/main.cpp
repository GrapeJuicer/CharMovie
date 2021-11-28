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

// オプション識別用ID
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

// プロトタイプ宣言
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
	// sample表示にかかわるオプション
	bool do_sample = false;
	int sample_width = 20;
	string sample_chs = " 1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!?\'\"`/\\,.#$%&()[]{}<>-=~^|@:;+*_";

	// 解析(analyze)に関わるオプション
	bool do_analyze = false;
	int threshold = 127;

	// 情報表示に関わるオプション
	bool do_info = false;

	// 画像 or 動画の表示に関わるオプション
	bool is_image = false;
	bool is_old = false;
	double ratio = 1.0;
	double speed = 1.0;
	int draw_method = cm::override;
	string cstyle_path = getEnvVariable("USERPROFILE") + "\\cmovie_character_style_template.txt";

	// Webカメラに関わるオプション
	bool is_webcam = false;
	int webcam_id = 0;

	// ファイル出力に関するオプション
	bool do_output = false;
	string output_path;

	// オプションの設定
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

	// 解析結果を格納する変数
	const int     fosize = 10;
	struct optarg findopts[fosize];
	// 検出されたオプションの数を格納する変数
	int folen;

	// オプションの解析
	folen = getopt_flex(argc, argv, opts, findopts, fosize);

	// 検出されたオプションの処理
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

	// 例外処理
	if  (folen == -1 || // オプション解析に失敗
		(do_sample && argc - optind != 0) || // -s オプションが有効の時 : オプション以外の引数が 0 でない
		(is_webcam && argc - optind != 0) || // -w オプションが有効の時 : オプション以外の引数が 0 でない
		(!do_sample && !is_webcam && argc - optind != 1))  // それ以外のとき : オプション以外の引数が 1 でない
	{
		cout << "Error: Invalid arguments. Please see the help using \'-h\' or \'--help\'." << endl;
		return -1;
	}

	try
	{
		// サンプル表示
		if (do_sample)
		{
			return printSample(sample_width, sample_chs, do_output, output_path);
		}
		// 解析
		else if (do_analyze)
		{
			return analyzeSample(sample_chs, string(argv[optind]), threshold, do_output, output_path);
		}
		// 情報表示
		else if (do_info)
		{
			return printMediaInfo(string(argv[optind]), is_image);
		}
		// webcam 画像の表示
		else if (is_image && is_webcam)
		{
			return printWebcamImage(webcam_id, cstyle_path, sample_chs, ratio);
		}
		// webcam 映像の表示
		else if (is_webcam)
		{
			return printWebcam(webcam_id, cstyle_path, sample_chs, ratio, draw_method);
		}
		// 画像の表示
		else if (is_image)
		{
			return printImage(argv[optind], cstyle_path, sample_chs, ratio);
		}
		// 動画の表示
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

// 環境変数を取得する
// arg:
//      name: 環境変数名
// return:
//      環境変数の値
string getEnvVariable(const char* name)
{
	const char* tmp = getenv(name);
	string env_var(tmp ? tmp : "");
	if (env_var.empty()) {
		throw runtime_error("Environment variable " + string(name) + " not set");
	}
	return env_var;
}

// サンプルデータを表示する
// arg:
//      width:       サンプル文字列の幅 (=1行に表示する要素の数)
//      chs:         サンプル文字列
//      do_output:   出力ファイルを指定した場合は true
//      output_path: 出力ファイルのパス
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

		// ファイルへの出力
		if (do_output)
		{
			// src を ouput_path に出力
			ofstream ofs(output_path);
			if (!ofs)
			{
				cerr << "Error: Failed to open the output file." << endl;
				return -1;
			}

			// 出力
			ofs << src;

			ofs.close();

			// log
			cout << endl;
			cout << "Sample source written to " << output_path << endl;
		}
	}

	return 0;
}

// サンプルデータの画像(キャプチャしたもの)を解析する
// arg:
//      characters:  解析対象の画像に使用された文字列
//      img_path:    画像ファイルのパス
//      threshold:   2値化の際の閾値
//      do_output:   出力ファイルを指定した場合は true
//      output_path: 出力ファイルのパス
// return:
//      0: 正常終了 / -1: エラー
int analyzeSample(string characters, string img_path, int threshold, bool do_output, string output_path)
{
	if (threshold < 0 || threshold > 255)
	{
		cerr << "Invalid threshold value. It must be between 0 and 255." << endl;
		return -1;
	}

	// インスタンス化
	CharAnalyze analyzer(characters, img_path, threshold);

	// 解析
	if (analyzer.analyze())
	{
		cerr << "Error: Failed to analyze the character list." << endl;
		return -1;
	}

	// 解析結果を表示
	analyzer.printCharList();

	// do_output が true ならファイル出力
	if (do_output)
	{
		// 出力
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

// 画像/動画の情報を表示する
// arg:
//      media_path:  画像/動画ファイルのパス
//      is_image:    画像の場合は true, 動画の場合は false
// return:
//      0: 正常終了 / -1: エラー
int printMediaInfo(string media_path, bool is_image)
{
	if (is_image)
	{
		// 画像を取得
		cv::Mat img = cv::imread(media_path);
		if (img.empty())
		{
			cerr << "Error: Failed to read the image." << endl;
			return -1;
		}
		// 画像情報を表示
		printImageInfo(img);

		img.release();
	}
	// 動画の場合
	else
	{
		// 動画を取得
		cv::VideoCapture video(media_path);
		if (!video.isOpened())
		{
			cerr << "Error: Failed to open the video." << endl;
			return -1;
		}

		// 動画情報を表示
		printVideoInfo(video);

		video.release();
	}

	return 0;
}

// 画像をアスキーアートで出力する
// arg:
//      image_path:  画像ファイルのパス
//      cstyle_path: スタイルのパス
//      characters:  アスキーアートに使用する文字列
//      ratio:       アスキーアートのサイズ比率
// return:
//      0: 正常終了 / -1: エラー
int printImage(string image_path, string cstyle_path, string characters, double ratio)
{
	// cstyle_path が空かチェック
	if (cstyle_path.empty())
	{
		cerr << "Error: Invalid style." << endl;
		return -1;
	}

	// characters が空かチェック
	if (characters.empty())
	{
		cerr << "Error: Invalid characters." << endl;
		return -1;
	}

	// ratioが0以下かチェック
	if (ratio <= 0)
	{
		cerr << "Invalid ratio value. It must be greater than 0." << endl;
		return -1;
	}

	// 画像を取得
	cv::Mat img = cv::imread(image_path);
	if (img.empty())
	{
		cerr << "Error: Failed to read the image." << endl;
		return -1;
	}

	// 画像表示用クラスの設定
	cm::CharImageViewer imgvwr(cstyle_path, characters, ratio);

	// 画像を表示
	imgvwr.show(img);

	img.release();

	return 0;
}


// 動画をアスキーアートで出力する
// arg:
//      video_path:  動画ファイルのパス
//      cstyle_path: スタイルのパス
//      characters:  アスキーアートに使用する文字列
//      ratio:       アスキーアートのサイズ比率
//      speed:       動画再生速度
//      draw_method: 描画方式
// return:
//      0: 正常終了 / -1: エラー
int printVideo(string video_path, string cstyle_path, string characters, double ratio, double speed, int draw_method)
{
	// 例外処理

	// cstyle_path が空かチェック
	if (cstyle_path.empty())
	{
		cerr << "Error: Invalid style." << endl;
		return -1;
	}

	// characters が空かチェック
	if (characters.empty())
	{
		cerr << "Error: Invalid characters." << endl;
		return -1;
	}

	// ratioが0以下かチェック
	if (ratio <= 0)
	{
		cerr << "Invalid ratio value. It must be greater than 0." << endl;
		return -1;
	}

	// speedが0以下かチェック
	if (speed <= 0)
	{
		cerr << "Invalid speed value. It must be greater than 0." << endl;
		return -1;
	}

	// draw_methodが0~2の範囲外かチェック
	if (draw_method < 0 || draw_method > 2)
	{
		cerr << "Invalid draw style. It must be between 0 and 2." << endl;
		return -1;
	}

	// 動画再生中のキーの割り当て
	const keymap keys = {
		{cm::play, ' '},
		{cm::pause, ' '},
		{cm::quit, 'q'},
	};

	// 動画を取得
	cv::VideoCapture video(video_path);
	if (!video.isOpened())
	{
		cerr << "Error: Failed to open the video." << endl;
		return -1;
	}

	// 動画表示用のクラスの設定
	cm::CharVideoPlayer vidplayer(cstyle_path, characters, ratio, speed, draw_method);

	// スレッドを作成
	thread vid_thread(inputKey, keys, ref(vidplayer.status));

	// 動画を表示
	vidplayer.show(video);

	// 動画が最後まで再生された場合
	if (vidplayer.status == cm::end)
	{
		cout << endl;
		cout << "Video ended." << endl;
		cout << "Press any key to quit." << endl;
	}

	// スレッドの終了を待機
	vid_thread.join();

	video.release();

	return 0;
}

// 動画再生中のキー入力を受け付ける
// arg:
//      km: キー割り当て
//      status: 動画再生状態
// return:
//      void
void inputKey(const keymap km, volatile int& status)
{
	int key;

	// キー入力によって status が quit になるまで繰り返し
	while (status != cm::quit)
	{
		// キー入力
		// key = getch();
		key = _getch();

		// CharVideoPlayer側で status が quit になった場合は即座に終了
		if (status == cm::end)
		{
			return;
		}

		// 検索
		for (auto& k : km)
		{
			// 動画が終了していなくて，状態が遷移した場合
			if (status != cm::end && status != k.first && key == k.second)
			{
				// 更新
				status = k.first;
				break;
			}
		}
	}
}

// 画像の情報を表示する
// arg:
//      img: 画像
// return:
//      void
void printImageInfo(const cv::Mat& img)
{
	// 画像の概要を表示
	int i_w = img.cols;  //縦の大きさ
	int i_h = img.rows;  //横の大きさ
	size_t i_len = img.total(); //画素数
	// フォーマットを取得
	int i_fmt = img.type();

	// 出力
	cout << "Image Information:" << endl;
	cout << "  Size       : " << i_w << "x" << i_h << " px" << endl;
	cout << "  Pixel Size : " << i_len << " pixels" << endl;
	switch (i_fmt) // フォーマット
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

// 動画の情報を表示する
// arg:
//      video: 動画
// return:
//      void
void printVideoInfo(const cv::VideoCapture& video)
{
	// 動画の概要を表示
	int v_w = (int)video.get(cv::CAP_PROP_FRAME_WIDTH);  //縦の大きさ
	int v_h = (int)video.get(cv::CAP_PROP_FRAME_HEIGHT); //横の大きさ
	double v_fps = video.get(cv::CAP_PROP_FPS);          //フレームレート
	int v_len = (int)video.get(cv::CAP_PROP_FRAME_COUNT);  //フレーム数
	double v_sec = (double)v_len / v_fps;                        //動画の秒数

	cout << "Video Information:" << endl;
	cout << "  Size       : " << v_w << "x" << v_h << " px" << endl;
	cout << "  Frame Rate : " << v_fps << " fps" << endl;
	cout << "  Frame Size : " << v_len << " frames" << endl;
	cout << "  Time       : " << (int)v_sec / 3600 << " h " << ((int)v_sec % 3600) / 60 << " m " << (int)v_sec % 60 << " s" << endl;
}

// Webカメラの映像を表示する
// arg:
//      webcam_id:  WebカメラのID
//      cstyle_path: スタイルのパス
//      characters:  アスキーアートに使用する文字列
//      ratio:       アスキーアートのサイズ比率
//      draw_method: 描画方式
// return:
//      0: 正常終了 / -1: エラー
int printWebcam(int webcam_id, string cstyle_path, string characters, double ratio, int draw_method)
{
	// 例外処理

	// cstyle_path が空かチェック
	if (cstyle_path.empty())
	{
		cerr << "Error: Invalid style." << endl;
		return -1;
	}

	// characters が空かチェック
	if (characters.empty())
	{
		cerr << "Error: Invalid characters." << endl;
		return -1;
	}

	// ratioが0以下かチェック
	if (ratio <= 0)
	{
		cerr << "Invalid ratio value. It must be greater than 0." << endl;
		return -1;
	}

	// draw_methodが0~2の範囲外かチェック
	if (draw_method < 0 || draw_method > 2)
	{
		cerr << "Invalid draw style. It must be between 0 and 2." << endl;
		return -1;
	}

	// Webカメラ再生中のキーの割り当て
	const keymap keys = {
		{cm::play, ' '},
		{cm::pause, ' '},
		{cm::quit, 'q'},
	};

	// 表示用のクラスの設定
	cm::CharVideoPlayer vidplayer(cstyle_path, characters, ratio, 1.0, draw_method);

	// スレッドを作成
	thread vid_thread(inputKey, keys, ref(vidplayer.status));

	// 動画を表示
	vidplayer.show(webcam_id);

	// スレッドの終了を待機
	vid_thread.join();

	return 0;
}

// Webカメラの画像を表示する
// arg:
//      webcam_id:  WebカメラのID
//      cstyle_path: スタイルのパス
//      characters:  アスキーアートに使用する文字列
//      ratio:       アスキーアートのサイズ比率
// return:
//      0: 正常終了 / -1: エラー
int printWebcamImage(int webcam_id, string cstyle_path, string characters, double ratio)
{
	// 例外処理

	// cstyle_path が空かチェック
	if (cstyle_path.empty())
	{
		cerr << "Error: Invalid style." << endl;
		return -1;
	}

	// characters が空かチェック
	if (characters.empty())
	{
		cerr << "Error: Invalid characters." << endl;
		return -1;
	}

	// ratioが0以下かチェック
	if (ratio <= 0)
	{
		cerr << "Invalid ratio value. It must be greater than 0." << endl;
		return -1;
	}

	// 画像表示用クラスの設定
	cm::CharImageViewer imgvwr(cstyle_path, characters, ratio);

	// 画像を表示
	imgvwr.show(webcam_id);

	return 0;
}
