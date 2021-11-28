#include "char_movie.hpp"

// CharAnalyze のコンストラクタ
// args:
//      text: 解析する画像で使用されている文字列
//      file_path: 画像のファイルパス
//      threshold: 解析する画像の二値化の閾値
CharAnalyze::CharAnalyze(string text, string file_path, unsigned char threshold)
{
	this->text = text;
	this->image = cv::imread(file_path);
	// エラー処理
	if (this->image.empty())
	{
		throw runtime_error("Image is empty");
	}
	this->threshold = threshold;
}

// CharAnalyze のデストラクタ
CharAnalyze::~CharAnalyze()
{
	this->image.release();
	this->work_image.release();
}

// 画像の解析を行う
// return:
//      0: 正常終了 / -1: エラー
int CharAnalyze::analyze()
{
	int x, y;

	// グレースケール化 & 二値化
	cv::cvtColor(this->image, this->work_image, cv::COLOR_BGR2GRAY);
	// 2値化
	cv::threshold(this->work_image, this->work_image, this->threshold, 255, cv::THRESH_BINARY);

	// 基準点を探す
	this->locateBasePosition(x, y);
	// 領域の幅と高さを取得
	this->calItemSize(x, y);
	// 枠の幅と高さを取得
	this->calFrameSize(x, y);

	// 文字と濃度の関係を計算して登録
	return this->setCharList(x, y);
}

// 基準点を探す
// args:
//      x: 基準点のx座標
//      y: 基準点のy座標
// return:
//      0: 正常終了 / -1: エラー
int CharAnalyze::locateBasePosition(int& x, int& y)
{
	int base_x, base_y;

	// 基準点を計算
	// (y, x) = (0,0), (0,1), (1,0), (0,2), (1,1), (2,0), ...
	base_x = 0;
	base_y = 0;
	while (this->work_image.at<uchar>(base_y, base_x) == 255)
	{
		if (base_x == this->work_image.cols - 1 && base_y == this->work_image.rows - 1)
		{
			// 見つからなかった
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

	// このとき，base_x, base_y のピクセルが色が異なる．＝基準点
	x = base_x;
	y = base_y;

	return 0;
}

// 文字領域の大きさを計算
// args:
//      x: 基準点のx座標
//      y: 基準点のy座標
// return:
//      0: 正常終了 / -1: エラー
int CharAnalyze::calItemSize(int x, int y)
{
	// 幅を計算
	// 幅では文字が領域からはみ出すことがあるので，領域の一番上のピクセルと使用して計算
	int cnt = 0;
	while (this->work_image.at<uchar>(y, x + cnt) != 255)
	{
		cnt++;
		if (x + cnt >= this->work_image.cols)
		{
			// 列数を超えた
			return -1;
		}
	}
	this->item_width = cnt; // cnt が幅

	// 高さを計算
	// 高さでは領域内に書かれた文字の影響で
	cnt = 1;
	while (1)
	{
		int i;
		for (i = 0; i < this->work_image.cols; i++)
		{
			if (y + cnt >= this->work_image.rows)
			{
				// 行数が越えた
				return -1;
			}
			if (this->work_image.at<uchar>(y + cnt, i) != 255)
			{
				break;
			}
		}
		// 一行すべてが white のとき
		if (i == this->work_image.cols)
		{
			break;
		}

		// 次の行へ
		cnt++;
	}
	this->item_height = cnt; // cnt が高さ

	return 0;
}

// 枠の大きさを計算
// args:
//      x: 基準点のx座標
//      y: 基準点のy座標
// return:
//      void
void CharAnalyze::calFrameSize(int x, int y)
{
	//枠の大きさを取得
	int fx = x + this->item_width, fy = y + this->item_height;
	while (this->work_image.at<uchar>(y, fx) == 255)
	{
		fx++;
		if (fx >= this->work_image.cols)
		{
			// もう後ろにない = 列数は 1
			break;
		}
	}
	while (this->work_image.at<uchar>(fy, x) == 255)
	{
		fy++;
		if (fy >= this->work_image.rows)
		{
			// もう後ろにない = 行数は 1
			break;
		}
	}
	// 枠の幅
	this->frame_width = fx >= this->work_image.cols ? this->work_image.cols : fx - x - this->item_width;
	// 枠の高さ
	this->frame_height = fy >= this->work_image.rows ? this->work_image.cols : fy - y - this->item_height;
}

// 文字と濃度の関係を計算して登録
// args:
//      x: 基準点のx座標
//      y: 基準点のy座標
// return:
//      0: 正常終了 / -1: エラー
int CharAnalyze::setCharList(int x, int y)
{
	int c_index = 0;

	for (int tmpy = y; tmpy < this->work_image.rows; tmpy += this->item_height + this->frame_height)
	{
		for (int tmpx = x; tmpx < this->work_image.cols; tmpx += this->item_width + this->frame_width)
		{
			// ピクセルの色をチェック
			if (this->work_image.at<uchar>(tmpy, tmpx) == 255)
			{
				// 何もしない．基本的にはここでfor文が終了する．
				continue;
			}

			// トリミング範囲
			// (tmpx, tmpy), w: item_width, h: item_height

			// 平均濃度のチェック
			double con_ratio = CharAnalyze::averageConcentration(this->work_image, tmpx, tmpy, this->item_width, this->item_height) / 255.0;

			// 登録
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

// 範囲内の画素の平均濃度を計算
// args:
//      image: 画像
//      x: x座標
//      y: y座標
//      w: 幅
//      h: 高さ
// return:
//      平均濃度
double CharAnalyze::averageConcentration(const cv::Mat& image, int x, int y, int w, int h)
{
	// 値のチェック
	if (x < 0 || x >= image.cols || y < 0 || y >= image.rows || w <= 0 || w > image.cols || h <= 0 || h > image.rows)
	{
		throw runtime_error("Invalid position");
	}

	long line_sum;
	double sum = 0;

	// 各行の平均の和を求める
	for (int i = y; i < y + h; i++)
	{
		// 初期化
		line_sum = 0;
		// 一行の値の和を求める
		for (int j = x; j < x + w; j++)
		{
			line_sum += image.at<uchar>(i, j);
		}
		// 平均を求める
		sum += (double)line_sum / w;
	}

	// 平均を求める
	return sum / h;
}

// 文字と濃度の関係をファイルに出力する
// args:
//      path: 出力先のパス
// return:
//      0: 正常終了 / -1: エラー
int CharAnalyze::exportCharList(string path)
{
	// ファイルを開く
	fstream ofs(path, ios::out);
	if (!ofs.is_open())
	{
		return -1;
	}

	// 寸法を出力
	ofs << this->item_width << " " << this->item_height << endl;

	// 書式を指定
	ofs << fixed << std::setprecision(6);

	// リストを出力
	for (cmap::iterator it = this->char_list.begin(); it != this->char_list.end(); it++)
	{
		ofs << it->second << it->first << endl;
	}

	// 書式をデフォルトに戻す
	ofs << defaultfloat;

	ofs.close();

	return 0;
}

// 文字と濃度の関係を画面に出力する
// args:
//      none
// return:
//      void
void CharAnalyze::printCharList()
{
	// 寸法を出力
	cout << this->item_width << " " << this->item_height << endl;

	cout << fixed << std::setprecision(6);

	// リストを出力
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
