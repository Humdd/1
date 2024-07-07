# BdjDetectionSystem

#### 介绍

编带机检测系统。目前已经可以检测以下芯片缺陷状态

##### 缺陷状态

0：无芯片

1：正常

6：旋转180°

8：锡球朝上

#### 算法流程

```cpp
cv::resize(img, img, cv::Size(0, 0), 0.5, 0.5);
cv::GaussianBlur(img, img, cv::Size(5, 5), 1.5, 1.5);
cv::GaussianBlur(img, img, cv::Size(3, 3), 1.5, 1.5);

cv::Mat edges;
cv::Canny(img, edges, 125, 255);

```

图像缩放50%

gaussian，5x5，3x3

canny检测

划分2x2区域

统计有效点(白点)的个数，计算与模板的比值，加权求和，最小值即为该类型。

#### 模板来源

0_1  cam1-20220729-200611-977.bmp      0_2  cam1-20220729-200636-090.bmp

1_1、1_2   cam1-20220729-200357-729.bmp

6_1、6_2   cam1-20220729-200611-977.bmp

8_1、8_2   cam1-20220729-200636-090.bmp


#### 安装教程

1.  xxxx
2.  xxxx
3.  xxxx

#### 使用说明

1.  xxxx
2.  xxxx
3.  xxxx

#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request


#### 特技

1.  使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2.  Gitee 官方博客 [blog.gitee.com](https://blog.gitee.com)
3.  你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解 Gitee 上的优秀开源项目
4.  [GVP](https://gitee.com/gvp) 全称是 Gitee 最有价值开源项目，是综合评定出的优秀开源项目
5.  Gitee 官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6.  Gitee 封面人物是一档用来展示 Gitee 会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
