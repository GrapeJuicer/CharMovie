# CharMovie 1.0.0

CharMovieはターミナル上で使用する動画/画像/Webカメラの簡易ビューアです．

English edi is [here](https://github.com/GrapeJuicer/CharMovie/blob/master/README.md).

## 概要

CharMovie は，画像や動画,Webカメラの映像をアスキーアートによって表示するツールです．
本ツールでは，次の機能をサポートしています．

- 再生速度の設定
- 比率(倍率)の設定
- 描画に使用する文字の設定
- スタイルの設定
- 描画方式の設定 (画面のクリア方式)
- 動画再生中のキー操作
  - Q キー : 動画の終了
  - SPACE キー : 動画の一時停止/再生

また，画像や動画の描画に使用するスタイルの作成も可能です．

注1: スタイルの作成には，エスケープシーケンスに対応したターミナル (例: Windows Terminal) を使用する必要があります．
注2: ターミナルの画面をスクリーンショットする機能 (例: Snipping Tools) を有している必要があります．

## 使用方法

### ヘルプの表示
```
$ cmovie.exe -h
$ cmovie.exe --help
```
### バージョンの表示
```
$ cmovie.exe -v
$ cmovie.exe --version
```
### 動画の表示
```
$ cmovie.exe <video>
$ cmovie.exe [-c <chars>] [-r <gt.0>] [-p <gt.0>] [-d <0-2>] [-S <style-file>] <video>
$ cmovie.exe [--character <chars>] [--ratio <gt.0>] [--speed <gt.0>] [--draw <0-2>] [--style <style-file>] <video>
```
使用例
```
$ cmovie.exe -c "+*#" -r 0.5 -p 2 myvideo.mp4
>>> -c "+*#" ... 次の文字のみを使用: '+', '*', '#'
>>> -r 0.5   ... 1文字あたり画素数を0.5倍に変更．
>>> -p 2     ... 再生速度を2倍に変更．
```
### 画像の表示
```
$ cmovie.exe -i <image>
$ cmovie.exe -i [-c <chars>] [-r <gt.0>] [-S <style-file>] <image>
$ cmovie.exe --image [--character <chars>] [--ratio <gt.0>] [--style <style-file>] <image>
```
### 動画/画像の詳細情報の表示
```
$ cmovie.exe -I <video>               # 動画
$ cmovie.exe --info <video>
$ cmovie.exe -i -I <image>            # 画像
$ cmovie.exe --image --info <image>
```
### スタイル作成用サンプル文字の表示
```
$ cmovie.exe -s [-c <chars>] [-o <chars-file>]
$ cmovie.exe --sample [--character <chars>] [--ouput <chars-file>]
```

### スタイル作成用サンプル文字の解析
```
$ cmovie.exe -a [-c <chars>] [-t <0-255>] [-o <chars-file>] <image>
$ cmovie.exe --analyze [--character <chars>] [--threshold <0-255>] [--ouput <chars-file>] <image>
```

## スタイルの作成方法

### 全ての文字を使用してスタイルを作成
```
$ cmovie.exe -s                               # --character を使用しない場合は全ての文字が対象となる
{表示されたサンプルをキャプチャして mycap.png として保存}
$ cmovie.exe -a -t 40 -o mystyle.txt mycap.png  # 必要に応じて --threshold で2値化の閾値を決定するのが望ましい
$ cmovie.exe -i -S mystyle.txt image.png      # mystyle.txt のスタイルを使用して image.png を表示
$ cmovie.exe -S mystyle.txt video.mp4         # mystyle.txt のスタイルを使用して video.mp4 を表示
```

### 指定した文字を使用してスタイルを作成
```
$ cmovie.exe -sc ABCDEFG -o mysample.txt             # ABCDEFGのみのサンプルを表示
{表示されたサンプルをキャプチャして mycap.png として保存}
$ type mysample.txt | clip                           # mysample.txtの内容(ABCDEFG)をクリップボードにコピー
$ cmovie.exe -a -c ABCDEFG -o mystyle.txt mycap.png  # -t 利用推奨．ABCDEFGの部分はCtrl+Vでペーストすると効率的
$ cmovie.exe -i -S mystyle.txt image.png             # mystyle.txt のスタイルを使用して image.png を表示
$ cmovie.exe -S mystyle.txt video.mp4                # mystyle.txt のスタイルを使用して video.mp4 を表示
```

## オプション一覧

| short  | long            | description                                                                                                                                                        |
| :----- | :-------------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| -h     | --help          | ヘルプを表示します．                                                                                                                                                |
| -v     | --version       | バージョンを表示します．                                                                                                                                           |
| -s     | --sample        | スタイル作成用のサンプル文字を表示します．                                                                                                                         |
| -c ARG | --character ARG | 使用する文字列を設定します．デフォルト：すべての英数字および記号                                                                                                   |
| -a     | --analyze       | キャプチャしたサンプル文字列を解析します．                                                                                                                         |
| -t ARG | --threshold ARG | 閾値を設定します．(0~255)                                                                                                                                          |
| -o ARG | --output ARG    | 出力ファイルのパスを設定します．                                                                                                                                   |
| -r ARG | --ratio ARG     | 動画/画像と文字の比率を設定します．(>0) デフォルト：1.0<br>注：値が最小値より小さい場合は、最小値が使用されます．                                                |
| -p ARG | --speed ARG     | 再生速度を設定します．値は 0.25~2.0 が推奨されます．デフォルト : 1.0<br>値が大きすぎると処理が追いつかず、実際の倍率が指定した値よりも小さくなることがあります． |
| -i     | --image         | 画像を使用します．                                                                                                                                                 |
| -d ARG | --draw ARG      | 描画方式を設定します．( 0: 上書き (デフォルト), 1: 画面のクリア後に描画, 2: `cls`コマンド実行後に描画 )<br>注1：エスケープシーケンス非対応のターミナルを使用している場合は，2を使用してください．<br>注2：0に設定するとティアリングが発生する場合があり，1または2に設定するとほとんどの場合ちらつきが発生します． |
| -S ARG | --style ARG     | スタイルのファイルパスを設定します．<br>デフォルト : `%USERPROFILE%/cmovie_character_style_template.txt`                                                               |
| -I     | --info          | 動画または画像の詳細情報を表示します。                                                                                                                             |
| -w ARG | --webcam ARG    | Webカメラの映像を描画します．引数にカメラの番号を必要とします．<br>0を指定するとデフォルトのWebカメラが指定されます．Webカメラが1つしかない場合はこちらを使用してください． |

以下のようにオプションを指定することも可能です.
- `-o ARG` ---> `-oARG`
- `--option ARG`  ---> `--option=ARG`

## インストール方法

1. [release](https://github.com/GrapeJuicer/CharMovie/releases)から`CharMovie-x.x.x.zip`をダウンロードする．
2. `CharMovie-x.x.x.zip`を解凍する．
3. `CharMovie-x.x.x\cmovie_character_style_template.txt`をユーザディレクトリ(`%USERPROFILE%`)に移動する．
4. (環境変数にパスを指定する．)

## バージョン情報

| バージョン | ハイライト     |
| :--------- | :------------- |
| 1.0.0      | 初回リリース． |

# 問い合わせ
| 項目     | 内容                                          |
| :------- | :-------------------------------------------- |
| Name     | GrapeJuice                                    |
| Email    | Dev.Grape@outlook.jp                          |
| Twitter  | [@G__HaC](https://twitter.com/G__HaC)         |
| GitHub   | [GrapeJuicer](https://github.com/GrapeJuicer) |
