#include "char_movie.hpp"
#include "char_list.hpp"

namespace cm
{
	// CharImageViewerのコンストラクタ
	// arg:
	//      char_style_path: スタイルのパス
	//      characters: アスキーアートに使用する文字列
	//      ratio: アスキーアートのサイズ比率
	CharImageViewer::CharImageViewer(string char_style_path, string characters, double ratio)
	{
		// 文字マップの読み込み
		// ここで width と height が設定される
		this->loadCharMap(char_style_path, characters);

		// ratioの設定 (最小値を下回った場合は最小値に修正)
		double min_val = 1.0 / min<int>(this->width, this->height);
		this->ratio = max<double>(ratio, min_val);
	}

	// CharImageViewerのデストラクタ
	CharImageViewer::~CharImageViewer()
	{
	}

	// スタイルの読み込み
	// arg:
	//      char_style_path: スタイルファイルのパス
	//      characters: アスキーアートに使用する文字列
	// return:
	//      void
	void CharImageViewer::loadCharMap(string char_style_path, string characters)
	{
		char c;
		double val;
		string line;
		// スタイルファイルの読み込み
		fstream fin(char_style_path, ios::in);
		if (!fin.is_open())
		{
			throw runtime_error("Can't open the file: " + char_style_path);
		}

		// 寸法を取得
		fin >> this->width >> this->height;
		// 読み捨てる
		getline(fin, line);
		// 各行を読み込む
		while (getline(fin, line))
		{
			//要素を分ける
			c = line[0];
			val = stod(line.substr(1));

			// c が characters に含まれているかチェック
			if (characters.find(c) != string::npos)
			{
				// 含まれていたら追加
				this->char_list.push_back({ val, c });
			}
		}

		// 割合が小さい順にソート
		sort(this->char_list.begin(), this->char_list.end());

		fin.close();
	}

	// アスキーアートを描画
	// arg:
	//      image: アスキーアートで描画する画像
	// return:
	//      void
	void CharImageViewer::show(const cv::Mat& image)
	{
		cv::Mat gray_image;
		int pxw, pxh;     // 1文字で表現する範囲のピクセル数
		int w, h;         // 修正後の幅と高さ
		double con_ratio; // 濃度の比率
		string chimg;

		// グレースケールに変換
		cv::cvtColor(image, gray_image, cv::COLOR_BGR2GRAY);

		// 1文字あたりのピクセル数を計算
		pxw = max<int>((int)(this->width * this->ratio), 1);
		pxh = max<int>((int)(this->height * this->ratio), 1);

		// 各範囲で繰り返し
		for (int i = 0; i < gray_image.rows; i += pxh)
		{
			for (int j = 0; j < gray_image.cols; j += pxw)
			{
				// トリミングするサイズを決定
				w = min<int>(pxw, gray_image.cols - j);
				h = min<int>(pxh, gray_image.rows - i);

				// 対象範囲 -> 左上座標: (j, i), width: w, height: h

				// 濃度の比率を計算
				// 0: 黒い, 255: 白い --> 白 1----0 黒
				con_ratio = CharAnalyze::averageConcentration(gray_image, j, i, w, h) / 255.0;

				// 文字を決定
				chimg += this->selectChar(con_ratio);
			}
			chimg += '\n';
		}

		// 表示
		cout << chimg << ends;

		gray_image.release();
	}

	// Webカメラの画像を描画
	void CharImageViewer::show(int webcam_id)
	{
		cv::Mat img;
		// webカメラに接続
		cv::VideoCapture cam(webcam_id);
		if (!cam.isOpened())
		{
			throw runtime_error("failed to cennect to web camera.");
		}

		// 1フレーム取得
		cam.read(img);

		// 表示
		this->show(img);

		img.release();
		cam.release();
	}


	// 文字を決定
	// arg:
	//      con_ratio: 濃度の比率
	// return:
	//      文字
	char CharImageViewer::selectChar(double con_ratio)
	{
		// 例外処理
		if (con_ratio < 0 || con_ratio > 1)
		{
			throw runtime_error("con_ratio must be between 0 and 1.");
		}

		// リストの情報
		static double range = this->char_list.back().first - this->char_list.front().first;
		static pair<double, char> min_val = this->char_list.front();
		static size_t lsize = this->char_list.size();

		// 相対的に値を決定
		double con_ratio_norm = (range * con_ratio) /*相対的な値*/ + min_val.first /*最小値*/;

		// 割合が小さい順に検索
		for (int i = 0; i < lsize; i++)
		{
			// リストの値が con_ratio_norm 以上になる -> i番目とi-1番目のどちらかが最も近い値
			if (this->char_list[i].first >= con_ratio_norm)
			{
				// 先頭の時
				if (i == 0)
				{
					// 1つ前がないので現在の値を決定
					return min_val.second;
				}
				else
				{
					// 1つ前の値と比較して，より近いほうに決定
					double diff1 = this->char_list[i].first - con_ratio_norm;
					double diff2 = con_ratio_norm - this->char_list[i - 1].first;
					return diff1 < diff2 ? this->char_list[i].second : this->char_list[i - 1].second;
				}
			}
		}

		// if にひっかからなかった場合は一番値の大きいものに決定
		return this->char_list.back().second;
	}

	// CharVideoPlayerのコンストラクタ
	// arg:
	//      char_style_path: スタイルのパス
	//      characters:アスキーアートに使用する文字列
	//      ratio: アスキーアートのサイズ比率
	//      speed: 再生速度
	//      draw_method: 描画方式
	CharVideoPlayer::CharVideoPlayer(string char_style_path, string characters, double ratio, double speed, int draw_method) : CharImageViewer(char_style_path, characters, ratio)
	{
		// 速度の設定
		if (speed <= 0)
		{
			throw runtime_error("speed must be greater than 0.");
		}
		this->speed = speed;
		this->status = cm::play;
		this->draw_method = draw_method;
	}

	// CharVideoPlayerのデストラクタ
	CharVideoPlayer::~CharVideoPlayer()
	{
	}


	// 1フレーム当たりの時間を計算
	// arg:
	//      video: 動画
	// return:
	//      1フレーム当たりの時間
	int CharVideoPlayer::calcurateDuration(cv::VideoCapture& video)
	{
		//フレーム数
		int frame_size = (int)video.get(cv::CAP_PROP_FRAME_COUNT);
		// fps
		double fps = video.get(cv::CAP_PROP_FPS);
		// 1フレームの時間
		return (int)(1000.0 / fps / this->speed);
	}


	// 動画を再生
	// arg:
	//      video: 再生する動画
	//      ms_per_frame: 1フレーム当たりの時間(ms). 0未満で自動設定．(Default: -1)
	// return:
	//      void
	void CharVideoPlayer::show(cv::VideoCapture& video, int ms_per_frame)
	{
		cv::Mat frame;
		//フレーム数
		int frame_size = (int)video.get(cv::CAP_PROP_FRAME_COUNT);
		// chrono で現在時刻を取得
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

		// 画面のクリア
		switch (this->draw_method)
		{
			case cm::override :
			case cm::clear:
				// エスケープシーケンスによる画面のクリア
				cout << clist::control::Control_ClearScreen << clist::control::Control_MoveToHead << ends;
				break;
			case cm::clear_old:
				// cls コマンドによる画面のクリア
				system("cls");
				break;
			default:
				throw runtime_error("draw_method is invalid.");
		}

		// 動画の再生
		for (int cfn = 0; cfn < frame_size; cfn++)
		{
			// pauseなら一時停止
			while (this->status == cm::pause);
			// quitなら終了
			if (this->status == cm::quit)
			{
				break;
			}

			target_time = chrono::steady_clock::now();

			// フレームの取得
			video.read(frame);
			// チェック (取得できなかったら終了)
			if (frame.empty())
			{
				throw runtime_error("frame is empty.");
			}

			// カーソルを先頭に移動 -> 画面のクリアに同等
			// この手法だと ティアリング が発生する可能性あり
			// 毎回画面クリアすると ちらつき が発生する可能性あり
			// system("cls");

			// 画面のクリア
			switch (this->draw_method)
			{
				case cm::override :
					// エスケープシーケンスによるカーソルの移動 (画面のクリアをしない)
					cout << clist::control::Control_MoveToHead << ends;
					break;
				case cm::clear:
					// エスケープシーケンスによる画面のクリアとカーソルの移動
					cout << clist::control::Control_ClearScreen << clist::control::Control_MoveToHead << ends;
					break;
				case cm::clear_old:
					// cls コマンドによる画面のクリア
					system("cls");
					break;
			}

			// フレームの再生
			CharImageViewer::show(frame);

			// 次のフレームまで待機
			this_thread::sleep_until(target_time + duration_time);
		}

		// キー入力による終了ではない場合，status を end に変更
		if (this->status != cm::quit)
		{
			this->status = cm::end;
		}
	}


	// Webカメラの映像を再生
	// arg:
	//      video: 再生する動画
	//      ms_per_frame: 1フレーム当たりの時間(ms). 0未満で自動設定．(Default: -1)
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

		// 画面のクリア
		switch (this->draw_method)
		{
			case cm::override :
			case cm::clear:
				// エスケープシーケンスによる画面のクリア
				cout << clist::control::Control_ClearScreen << clist::control::Control_MoveToHead << ends;
				break;
			case cm::clear_old:
				// cls コマンドによる画面のクリア
				system("cls");
				break;
			default:
				throw runtime_error("draw_method is invalid.");
		}

		int x = 0;
		// 動画の再生
		while (true)
		{
			// pauseなら一時停止
			while (this->status == cm::pause);

			// quitなら終了
			if (this->status == cm::quit)
			{
				break;
			}

			// フレームの取得
			cam.read(frame);
			// チェック (取得できなかったら終了)
			if (frame.empty())
			{
				throw runtime_error("frame is empty.");
			}

			// カーソルを先頭に移動 -> 画面のクリアに同等
			// この手法だと ティアリング が発生する可能性あり
			// 毎回画面クリアすると ちらつき が発生する可能性あり
			// system("cls");

			// 画面のクリア
			switch (this->draw_method)
			{
				case cm::override :
					// エスケープシーケンスによるカーソルの移動 (画面のクリアをしない)
					cout << clist::control::Control_MoveToHead << ends;
					break;
				case cm::clear:
					// エスケープシーケンスによる画面のクリアとカーソルの移動
					cout << clist::control::Control_ClearScreen << clist::control::Control_MoveToHead << ends;
					break;
				case cm::clear_old:
					// cls コマンドによる画面のクリア
					system("cls");
					break;
			}

			// フレームの再生
			CharImageViewer::show(frame);

			// 一瞬待機
			this_thread::sleep_for(chrono::milliseconds(1));
			//cv::waitKey(duration_time);
		}

		cam.release();
	}
}