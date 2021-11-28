# CharMovie 1.0.0

CharMovie is a video/image/webcamera viewer used in terminal.

日本語版は[こちら](https://github.com/GrapeJuicer/CharMovie/blob/master/README_ja.md).

## Summary

CharMovie is a tool to display image/video/web-camera with ASCII art.
It supports following options.

- Setting the playback speed
- Setting the text-to-image ratio
- Setting the characters to be used for drawing
- Setting the style
- Setting the drawing method (terminal's display clear method)
- Key operations during video playback
  - Q key : Quit
  - SPACE key : Pause/Resume

It also supports to create styles to be used for drawing.
Note 1: To create styles, You need to use a terminal that supports escape sequences (e.g. Windows Terminal).
Note 2: And you also needs tools to take a screenshot of the terminal screen (e.g. Snipping Tools).

## Usage

### Show help
```
$ cmovie.exe -h
$ cmovie.exe --help
```
### Show version
```
$ cmovie.exe -v
$ cmovie.exe --version
```
### Playback video
```
$ cmovie.exe <video>
$ cmovie.exe [-c <chars>] [-r <gt.0>] [-p <gt.0>] [-d <0-2>] [-S <style-file>] <video>
$ cmovie.exe [--character <chars>] [--ratio <gt.0>] [--speed <gt.0>] [--draw <0-2>] [--style <style-file>] <video>
```
e.g.
```
$ cmovie.exe -c "+*#" -r 0.5 -p 2 myvideo.mp4
>>> -c "+*#" ... Use following characters: '+', '*', '#'
>>> -r 0.5   ... Increase the number of pixels assigned per character by 0.5 (=Decrease)
>>> -p 2     ... Change playback speed to 2x.
```
### Show image
```
$ cmovie.exe -i <image>
$ cmovie.exe -i [-c <chars>] [-r <gt.0>] [-S <style-file>] <image>
$ cmovie.exe --image [--character <chars>] [--ratio <gt.0>] [--style <style-file>] <image>
```
### Show information about the video/image
```
$ cmovie.exe -I <video>               # video
$ cmovie.exe --info <video>
$ cmovie.exe -i -I <image>            # image
$ cmovie.exe --image --info <image>
```
### Show sample characters for style creation
```
$ cmovie.exe -s [-c <chars>] [-o <chars-file>]
$ cmovie.exe --sample [--character <chars>] [--ouput <chars-file>]
```
### Analyze sample characters for style creation
```
$ cmovie.exe -a [-c <chars>] [-t <0-255>] [-o <chars-file>] <image>
$ cmovie.exe --analyze [--character <chars>] [--threshold <0-255>] [--ouput <chars-file>] <image>
```

## How to create styles

### Create a style with all characters
```
$ cmovie.exe -s                                 # If '--character' option is not used, all characters will be targeted.
{Save sample's screen shot as mycap.png}
$ cmovie.exe -a -t 40 -o mystyle.txt mycap.png  # If necessary, it is desirable to use '--threshold' option to determine the threshold for binarization.
$ cmovie.exe -i -S mystyle.txt image.png        # Show image.png using style 'mystyle.txt'
$ cmovie.exe -S mystyle.txt video.mp4           # Playback video.mp4 using style 'mystyle.txt'
```

### Create a style with the specified characters
```
$ cmovie.exe -sc ABCDEFG -o mysample.txt             # Show sample for ABCDEFG only (use --output option if you wanna be more efficient)
{Save sample's screen shot as mycap.png}
$ type mysample.txt | clip                           # Copy the contents of mysample.txt(ABCDEFG) to the clipboard.
$ cmovie.exe -a -c ABCDEFG -o mystyle.txt mycap.png  # '--threshold' option is recommended．Use Ctrl+V to paste the ABCDEFG.
$ cmovie.exe -i -S mystyle.txt image.png             # Show image.png using style 'mystyle.txt'
$ cmovie.exe -S mystyle.txt video.mp4                # Playback video.mp4 using style 'mystyle.txt'
```

## Options list

| short  | long | description |
| :-- | :-- | :-- |
|  -h     | --help            | Show this help. |
|  -v     | --version         | Show version. |
|  -s     | --sample          | Displays sample characters for style. |
|  -c ARG | --character ARG   | Set source characters. Default: a-zA-Z0-9 and all symbols. |
|  -a     | --analyze         | Analyze the captured sample characters. |
|  -t ARG | --threshold ARG   | Set the threshold value. It is used when 'analyze' option enabled. Default: 127 |
|  -o ARG | --output ARG      | Set the output file path. It is used when 'analyze' option enabled. Default: stdout |
|  -r ARG | --ratio ARG       | Set the ratio value. Default: 1.0<br>Note: If value is less than the minimum, it'll be corrected to the minimum. |
|  -p ARG | --speed ARG       | Set the speed value. Default: 1.0, 0.25-2.0 is recommended.<br>If the value is too large, the processing may not be able to keep up and the actual magnification may be smaller than the specified value. |
|  -i     | --image           | Use an image instead of a video. |
|  -d ARG | --draw ARG        | Set Drawing method.<br>  0: Override. (Default)<br>  1: Clear->Write.<br>  2: 'cls'->Write.<br>Note 1: Use 2 if your terminal doesn't support clearing the screen with escape sequences.<br>Note 2: When set to 0, tearing may occur; when set to 1 or 2, flickering will occur in most cases. |
|  -S ARG | --style ARG       | Set character styles file path corresponding to colors.<br>Default: '%USERPROFILE%/cmovie_character_style_template.txt'. |
|  -I     | --info            | Show image/video information. |
|  -w ARG | --webcam ARG      | Draw the image from the web camera. The camera number is required as an argument;<br>if there is only one webcam, specify 0. |

It is also possible to specify the options in the following:
- `-o ARG` ---> `-oARG`
- `--option ARG`  ---> `--option=ARG`


## Installation

1. Download `CharMovie-x.x.x.zip` from [release](https://github.com/GrapeJuicer/CharMovie/releases).
2. Unzip `CharMovie-x.x.x.zip`.
3. Move `CharMovie-x.x.x\cmovie_character_style_template.txt` to your home directory (`%USERPROFILE%`).
4. (Add the path to the env)

## Versions

| Version    | Highlight      |
| :--------- | :------------- |
| 1.0.0      | First release. |

# Contact
| Contents | About                                         |
| :------- | :-------------------------------------------- |
| Name     | GrapeJuice                                    |
| Email    | Dev.Grape@outlook.jp                          |
| Twitter  | [@G__HaC](https://twitter.com/G__HaC)         |
| GitHub   | [GrapeJuicer](https://github.com/GrapeJuicer) |
