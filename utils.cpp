#include "utils.h"
#include "header.h"
#include "ICAlgorithmParams.h"

Utils::Utils()
{
//    templ = cv::imread("./utils/templ/0004_1.bmp", cv::IMREAD_GRAYSCALE);
//    templ_bin = cv::imread("./utils/templ_bin/0004_1.bmp", cv::IMREAD_GRAYSCALE);
//    logoMask = cv::imread("./utils/logoMask/0004_1.bmp", cv::IMREAD_GRAYSCALE);
}

///图像的二维分割，即将图像分割成rows行、cols列的小格子—array[rows][cols]，
///保存在二维矢量vector中
std::vector<std::vector<cv::Mat> > Utils::partitionImage(cv::Mat &src, int rows, int cols)
{
    std::vector<std::vector<cv::Mat>> array(rows, std::vector<cv::Mat>(cols));

    int irows = src.rows, icols = src.cols;   //原图像的行数、列数
    int dr = irows / rows, dc = icols / cols; //分割后的小块图像的行数、列数
    ///分割行、列数与图像的行列数不能整除时，有效图像偏移delt_x、delt_y。
    /// 也就是将不能整除的余数像素丢弃，丢弃的方法就是，在图像上、下、左右
    /// 各删除余数一半的像素。比如行余数为4，则在图像顶部、底部分别删除2行像素。
    int delt_y = (irows % rows) / 2, delt_x = (icols % cols) / 2;
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            int x = j * dc + delt_x, y = i * dr + delt_y;
            array[i][j] = src(cv::Rect(x, y, dc, dr));
        }
    }
    return array;
}

cv::Mat Utils::img1;
cv::Mat Utils::img2;

void Utils::callback(int value, void*)
{
   threshold(img1, img2, value, 255, cv::THRESH_BINARY);
   cv::imshow("滑条", img2);
}

void Utils::binarizationWithSlider(cv::Mat &src)
{
    cv::Mat img = src.clone();
//    cvtColor(src, img, cv::COLOR_BGR2GRAY);
    cv::namedWindow("滑条");
    imshow("原图", img);
    img1 = img.clone();
    int value = 60;
    cv::createTrackbar("阈值", "滑条", &value, 255, callback);
    cv::waitKey(0);
}


cv::Scalar Utils::getAvg(cv::Mat &src)
{
    return cv::mean(src);
}

/**
 * @brief 统计二值图白点数量
 * @param bin 二值图
 * @return
 */
int Utils::getWhiteCnt(cv::Mat &bin)
{
    int total = 0;
//    for (int i = 0; i < src.rows; i++)
//    {
//        for (int j = 0; j < src.cols; j++)
//        {
//            if (src.at<uchar>(i, j) == 255)
//                ++total;
//        }
//    }
    total = cv::countNonZero(bin);
    return total;
}

int Utils::getBlackCnt(cv::Mat &src)
{
    int total = 0;
    for (int i = 0; i < src.rows; i++)
    {
        for (int j = 0; j < src.cols; j++)
        {
            if (src.at<uchar>(i, j) == 0)
                ++total;
        }
    }
    return total;
}

double Utils::getMinVal(cv::Mat &src)
{
    double minVal, maxVal;
    cv::minMaxIdx(src, &minVal, &maxVal);
    return minVal;
}

double Utils::getMaxVal(cv::Mat &src)
{
    double minVal, maxVal;
    cv::minMaxIdx(src, &minVal, &maxVal);
    return maxVal;
}

double Utils::getMean(cv::Mat &src)
{
    cv::Mat mean, stddev;
    cv::meanStdDev(src, mean, stddev);
    return mean.at<double>(0, 0);
}

double Utils::getStdDev(cv::Mat &src)
{
    cv::Mat mean, stddev;
    cv::meanStdDev(src, mean, stddev);
    return stddev.at<double>(0, 0);
}

double Utils::getRatio(int x, int y)
{
    if (x > y)
        std::swap(x, y);
    return double(y - x) / (y + 1);
}



std::vector<int> Utils::getBlockWhiteCnts(cv::Mat &src)
{
    int pad = src.cols * 0.1;
    int roi_width = src.cols * 0.35;
    int roi_height = src.rows * 0.35;
    cv::Rect roi1 = cv::Rect(pad, pad, roi_width, roi_height);
    cv::Rect roi2 = cv::Rect(src.cols - pad - roi_width, pad, roi_width, roi_height);
    cv::Rect roi3 = cv::Rect(pad, src.rows - pad - roi_height, roi_width, roi_height);
    cv::Rect roi4 = cv::Rect(src.cols - pad - roi_width, src.rows - pad - roi_height, roi_width, roi_height);

    cv::Mat block1 = src(roi1);
    cv::Mat block2 = src(roi2);
    cv::Mat block3 = src(roi3);
    cv::Mat block4 = src(roi4);
    int block1_cnt = getWhiteCnt(block1);
    int block2_cnt = getWhiteCnt(block2);
    int block3_cnt = getWhiteCnt(block3);
    int block4_cnt = getWhiteCnt(block4);

//    cv::Mat tmp;
//    cv::cvtColor(src, tmp, cv::COLOR_GRAY2BGR);
//    cv::rectangle(tmp, roi1, cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
//    cv::rectangle(tmp, roi2, cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
//    cv::rectangle(tmp, roi3, cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
//    cv::rectangle(tmp, roi4, cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
//    cv::imshow("block", tmp);
//    cv::waitKey(0);

//    int total = block1_cnt + block2_cnt + block3_cnt + block4_cnt;
//    qDebug() << "total_white_cnt:" << total;
//    qDebug() << "block1_cnt:" << block1_cnt << "\t"
//             << "block2_cnt:" << block2_cnt << "\t"
//             << "block3_cnt:" << block3_cnt << "\t"
//             << "block4_cnt:" << block4_cnt;
//    qDebug() << "ratio1:" << (double)block1_cnt / total << "\t"
//             << "ratio2:" << (double)block2_cnt / total << "\t"
//             << "ratio3:" << (double)block3_cnt / total << "\t"
//             << "ratio4:" << (double)block4_cnt / total;
//    std::vector<int> block_cnts{block1_cnt, block2_cnt, block3_cnt, block4_cnt};
//    return block_cnts;
    return {block1_cnt, block2_cnt, block3_cnt, block4_cnt};
}

std::vector<int> Utils::getBlockWhiteCntsTest(cv::Mat &src)
{
    int pad = src.cols * 0.1;
    int roi_width = src.cols * 0.35;
    int roi_height = src.rows * 0.35;
    cv::Rect roi1 = cv::Rect(pad, pad, roi_width, roi_height);
    cv::Rect roi2 = cv::Rect(src.cols - pad - roi_width, pad, roi_width, roi_height);
    cv::Rect roi3 = cv::Rect(pad, src.rows - pad - roi_height, roi_width, roi_height);
    cv::Rect roi4 = cv::Rect(src.cols - pad - roi_width, src.rows - pad - roi_height, roi_width, roi_height);

    cv::Mat block1 = src(roi1);
    cv::Mat block2 = src(roi2);
    cv::Mat block3 = src(roi3);
    cv::Mat block4 = src(roi4);
    int block1_cnt = getWhiteCnt(block1);
    int block2_cnt = getWhiteCnt(block2);
    int block3_cnt = getWhiteCnt(block3);
    int block4_cnt = getWhiteCnt(block4);

    cv::Mat tmp;
    cv::cvtColor(src, tmp, cv::COLOR_GRAY2BGR);
    cv::rectangle(tmp, roi1, cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
    cv::rectangle(tmp, roi2, cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
    cv::rectangle(tmp, roi3, cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
    cv::rectangle(tmp, roi4, cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
    cv::imshow("block", tmp);
    cv::waitKey(0);

    //    int total = block1_cnt + block2_cnt + block3_cnt + block4_cnt;
    //    qDebug() << "total_white_cnt:" << total;
    //    qDebug() << "block1_cnt:" << block1_cnt << "\t"
    //             << "block2_cnt:" << block2_cnt << "\t"
    //             << "block3_cnt:" << block3_cnt << "\t"
    //             << "block4_cnt:" << block4_cnt;
    //    qDebug() << "ratio1:" << (double)block1_cnt / total << "\t"
    //             << "ratio2:" << (double)block2_cnt / total << "\t"
    //             << "ratio3:" << (double)block3_cnt / total << "\t"
    //             << "ratio4:" << (double)block4_cnt / total;
    //    std::vector<int> block_cnts{block1_cnt, block2_cnt, block3_cnt, block4_cnt};
    //    return block_cnts;
    return {block1_cnt, block2_cnt, block3_cnt, block4_cnt};
}

std::vector<double> Utils::getRatioWithinVector(std::vector<int> &v)
{
    int total = std::accumulate(v.begin(), v.end(), 0);
    std::vector<double> res;
    for (auto x : v)
    {
        double ratio = double(x) / total;
        res.push_back(ratio);
    }
    return res;
}

std::vector<double> Utils::getRatioBetweenVector(std::vector<int> &v1, std::vector<int> &v2)
{
    std::vector<double> res;
    double ratio;
    for (int i = 0; i < v1.size(); i++)
    {
        if (std::abs(v1[i] - v2[i]) < 50)
            ratio = getRatio(0, 0);
        else
            ratio = getRatio(v1[i], v2[i]);
        res.push_back(ratio);
    }
    return res;
}

void Utils::printStatistics(cv::Mat &src)
{
    qDebug() << "mean:   \t" << getMean(src);
    qDebug() << "stddev: \t" << getStdDev(src);
    qDebug() << "whiteCnt:\t" << getWhiteCnt(src) << "\n";
}


void Utils::printVector(std::vector<int> &v)
{
    for (auto x : v)
    {
        std::cout << x << "\t";
    }
    std::cout << std::endl;
}

void Utils::printVector(std::vector<double> &v)
{
    for (auto x : v)
    {
        std::cout << x << "\t";
    }
    std::cout << std::endl;
}

/**
 * @brief 获取朝向类型
 * @param src 灰度图
 * @return 返回朝向类型， 0是无芯片，1是正常方向，6是旋转180°，8是锡球朝上
 */
int Utils::getType(cv::Mat &src)
{
    using namespace std;
    using namespace cv;
    cv::Mat img = src.clone();
    if (img.channels() == 3)
    {
        cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
    }
    cv::resize(img, img, cv::Size(0, 0), 0.5, 0.5);
    cv::GaussianBlur(img, img, cv::Size(5, 5), 1.5, 1.5);
    cv::GaussianBlur(img, img, cv::Size(3, 3), 1.5, 1.5);

    cv::Mat edges;
    cv::Canny(img, edges, 125, 255); // origin
    auto vBlockWhiteCnts = getBlockWhiteCnts(edges);
//    printVector(vBlockWhiteCnts);

    // 各种类型的白点先验值
    std::vector<int> prior_0 = {0, 0, 0, 0};
    std::vector<int> prior_1 = {1749, 1763, 922, 850};
    std::vector<int> prior_6 = {792, 873, 1684, 1758};
    std::vector<int> prior_8 = {1775, 1935, 1827, 1771};

    int type = -1;
    double minScore = DBL_MAX;

    // compare with prior white cnt
    std::vector<double> vRatioBetweenVector;
    double score; // 越小越好

    vRatioBetweenVector = getRatioBetweenVector(vBlockWhiteCnts, prior_0);
    score = std::accumulate(vRatioBetweenVector.begin(), vRatioBetweenVector.end(), 0.0);
//    printVector(vRatioBetweenVector);
//    std::cout << "score0: " << score << std::endl;
    Q_LOG4CPLUS_INFO(QString("score0:") + QString::number(score))
    if (score < minScore)
    {
        type = 0;
        minScore = score;
    }

    vRatioBetweenVector = getRatioBetweenVector(vBlockWhiteCnts, prior_1);
    score = std::accumulate(vRatioBetweenVector.begin(), vRatioBetweenVector.end(), 0.0);
//    printVector(vRatioBetweenVector);
//    std::cout << "score1: " << score << std::endl;
    Q_LOG4CPLUS_INFO(QString("score1:") + QString::number(score))
    if (score < minScore)
    {
        type = 1;
        minScore = score;
    }

    vRatioBetweenVector = getRatioBetweenVector(vBlockWhiteCnts, prior_6);
    score = std::accumulate(vRatioBetweenVector.begin(), vRatioBetweenVector.end(), 0.0);
//    printVector(vRatioBetweenVector);
//    std::cout << "score6: " << score << std::endl;
    Q_LOG4CPLUS_INFO(QString("score6:") + QString::number(score))
    if (score < minScore)
    {
        type = 6;
        minScore = score;
    }

    vRatioBetweenVector = getRatioBetweenVector(vBlockWhiteCnts, prior_8);
    score = std::accumulate(vRatioBetweenVector.begin(), vRatioBetweenVector.end(), 0.0);
//    printVector(vRatioBetweenVector);
//    std::cout << "score8: " << score << std::endl;
    Q_LOG4CPLUS_INFO(QString("score8:") + QString::number(score))
    if (score < minScore)
    {
        type = 8;
        minScore = score;
    }
    return type;
}

/**
 * @brief 获取缺陷类型
 * @param src  灰度图
 * @return 返回缺陷类型， 0是无芯片，1是正常
 */
int Utils::getTypeTest(cv::Mat &src)
{
    using namespace std;
    using namespace cv;
    cv::Mat img = src.clone();
    if (img.channels() == 3)
    {
        cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
    }
    cv::GaussianBlur(img, img, cv::Size(3, 3), 0, 0);
    cv::resize(img, img, cv::Size(0, 0), 0.5, 0.5);
//    cv::GaussianBlur(img, img, cv::Size(5, 5), 1.5, 1.5);
//    cv::GaussianBlur(img, img, cv::Size(3, 3), 1.5, 1.5);


    cv::Mat edges;
    cv::Canny(img, edges, 135, 255);
//    cv::Canny(img, edges, 125, 255); // origin
    imshow("src", img);
//    imshow("canny", edges);
    waitKey(0);
    auto vBlockWhiteCnts = getBlockWhiteCntsTest(edges);
    printVector(vBlockWhiteCnts);

    // 各种类型的白点先验值
    std::vector<int> prior_0 = {0, 0, 0, 0};
    std::vector<int> prior_1 = {1749, 1763, 922, 850};
    std::vector<int> prior_6 = {792, 873, 1684, 1758};
    std::vector<int> prior_8 = {1775, 1935, 1827, 1771};

    int type = -1;
    double minScore = DBL_MAX;

    // compare with prior white cnt
    std::vector<double> vRatioBetweenVector;
    double score; // 越小越好

    vRatioBetweenVector = getRatioBetweenVector(vBlockWhiteCnts, prior_0);
    score = std::accumulate(vRatioBetweenVector.begin(), vRatioBetweenVector.end(), 0.0);
    printVector(vRatioBetweenVector);
    std::cout << "score0: " << score << std::endl;
//    Q_LOG4CPLUS_INFO(QString("score0:") + QString::number(score))
    if (score < minScore)
    {
        type = 0;
        minScore = score;
    }

    vRatioBetweenVector = getRatioBetweenVector(vBlockWhiteCnts, prior_1);
    score = std::accumulate(vRatioBetweenVector.begin(), vRatioBetweenVector.end(), 0.0);
    printVector(vRatioBetweenVector);
    std::cout << "score1: " << score << std::endl;
//    Q_LOG4CPLUS_INFO(QString("score1:") + QString::number(score))
    if (score < minScore)
    {
        type = 1;
        minScore = score;
    }

    vRatioBetweenVector = getRatioBetweenVector(vBlockWhiteCnts, prior_6);
    score = std::accumulate(vRatioBetweenVector.begin(), vRatioBetweenVector.end(), 0.0);
    printVector(vRatioBetweenVector);
    std::cout << "score6: " << score << std::endl;
//    Q_LOG4CPLUS_INFO(QString("score6:") + QString::number(score))
    if (score < minScore)
    {
        type = 6;
        minScore = score;
    }

    vRatioBetweenVector = getRatioBetweenVector(vBlockWhiteCnts, prior_8);
    score = std::accumulate(vRatioBetweenVector.begin(), vRatioBetweenVector.end(), 0.0);
    printVector(vRatioBetweenVector);
    std::cout << "score8: " << score << std::endl;
//    Q_LOG4CPLUS_INFO(QString("score8:") + QString::number(score))
    if (score < minScore)
    {
        type = 8;
        minScore = score;
    }
    return type;
}

/**
 * @brief 模板匹配
 * @param img 待测chip
 * @param templ 模板
 * @return 匹配结果在待测chip的相对位置
 */
cv::Rect Utils::getMatchResultROI(cv::Mat &img, const cv::Mat &templ)
{
    //创建一个空画布用来绘制匹配过程
    cv::Mat dstImg;
    dstImg.create(img.dims, img.size, img.type());

    //匹配，最后一个参数为匹配方式，共有6种，详细请查阅函数介绍
//    clock_t start = clock();
    cv::matchTemplate(img, templ, dstImg, cv::TM_SQDIFF_NORMED);
//    clock_t end = clock();
//    std::cout << "elapsed_time: " << end - start << " ms" << std::endl;

    //获取最大或最小匹配系数
    //首先是从得到的 输出矩阵中得到 最大或最小值（平方差匹配方式是越小越好，所以在这种方式下，找到最小位置）
    //找矩阵的最小位置的函数是 minMaxLoc函数
    cv::Point minPoint;
    cv::Point maxPoint;
    double minVal = 0;
    double maxVal = 0;
    cv::minMaxLoc(dstImg, &minVal, &maxVal, &minPoint, &maxPoint);
    std::cout << "[cv::TM_SQDIFF_NORMED score]: " << minVal << std::endl;

    //绘制匹配结果
    cv::Rect rect(minPoint.x, minPoint.y, templ.cols, templ.rows);
    return rect;
}

/**
 * @brief 计算灰度图的直方图
 * @param img
 * @return
 */
cv::Mat Utils::calcHist(cv::Mat &img)
{
    cv::Mat gray;
    if (img.channels() == 3)
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    else
        gray = img;
    //设置提取直方图的相关变量
    cv::Mat hist;  //用于存放直方图计算结果
    const int channels[1] = { 0 };  //通道索引
    const int bins[1] = { 256 };  //直方图的维度，其实就是像素灰度值的最大值
    float inRanges[2] = { 0, 255 };
    const float* ranges[1] = { inRanges };  //像素灰度值范围
    cv::calcHist(&gray, 1, channels, cv::Mat(), hist, 1, bins, ranges);  //计算图像直方图
    //准备绘制直方图
    int hist_w = 512;
    int hist_h = 400;
    int width = 2;
    cv::Mat histImage = cv::Mat::zeros(hist_h, hist_w, CV_8UC3);
//    for (int i = 1; i <= hist.rows; i++)
//    {
//        cv::rectangle(histImage, cv::Point(width * (i - 1), hist_h - 1),
//                      cv::Point(width * i - 1, hist_h - cvRound(hist.at<float>(i - 1) / 15)),
//                      cv::Scalar(255, 255, 255), -1);
//    }
    //绘制直方图
    for (int i = 0; i < hist.rows; i++)
    {
        cv::rectangle(histImage, cv::Point(width * i, hist_h - 1),
                      cv::Point(width * i + 1, hist_h - cvRound(hist.at<float>(i) / 15)),
                      cv::Scalar(0, 255, 0), -1);
    }
//    cv::imshow("gray", gray);
//    int total = gray.rows * gray.cols;
//    double accumulative_ratio = 0;
//    for (int i = 0; i < hist.rows; i++)
//    {
//        double ratio = hist.at<float>(i) / total;
//        accumulative_ratio += ratio;
//        std::cout << "[pix]: " << i
//                  << "\t[cnt]: " << hist.at<float>(i)
//                  << "\t[rat]: " << ratio
//                  << "\t[acc_rat]: " << accumulative_ratio << std::endl;
//    }
//    cv::imshow("histImage", histImage);
//    cv::waitKey(0);
    return hist;
}

/**
 * @brief 获取差值图的二值化阈值
 * @param gau_grayDiff 差值图
 * @return
 */
int Utils::getDiffThresh(cv::Mat &gau_grayDiff)
{
    cv::Mat gau_grayDiff_hist = calcHist(gau_grayDiff);
    int total = gau_grayDiff.rows * gau_grayDiff.cols;
    double gau_acc_rat = 0;
    const auto &[ diffRatio, wndsz, pad_logo ] = params_ic.logo_params;
    for (int i = 0; i < gau_grayDiff_hist.rows; i++)
    {
        double gau_ratio = gau_grayDiff_hist.at<float>(i) / total;
        gau_acc_rat += gau_ratio;
        if (gau_acc_rat > diffRatio) //0.95
            return i;
    }
    return 0;
}

/**
 * @brief 检测logo是否正常
 * @param chip 芯片图片
 * @return bool 检测结果
 */
bool Utils::inspectLogo(cv::Mat &logo)
{
    cv::Mat templ; //模板
    cv::Mat templ_bin; //模板二值图
    cv::Mat logoMask;

    //模板匹配
    cv::Rect matchResult_roi = getMatchResultROI(logo, templ);
    cv::Mat matchResult = logo(matchResult_roi);
    //3x3高斯模糊，灰度图absdiff
    cv::Mat matchResult_gau, templ_gau, gau_grayDiff;
    cv::GaussianBlur(matchResult, matchResult_gau, cv::Size(3, 3), 1.5, 1.5);
    cv::GaussianBlur(templ, templ_gau, cv::Size(3, 3), 1.5, 1.5);
    cv::absdiff(matchResult_gau, templ_gau, gau_grayDiff);
    //匹配结果二值化，diff二值化
    cv::Mat matchResult_bin, gau_grayDiff_bin;
    int templ_thesh = 200;
    cv::threshold(matchResult, matchResult_bin, templ_thesh, 255, cv::THRESH_BINARY); //手动调整得到二值化阈值
    int diff_thresh = getDiffThresh(gau_grayDiff); //直方图统计，gau_grayDiff的gau_acc_rat大于0.95对应的像素值
    cv::threshold(gau_grayDiff, gau_grayDiff_bin, diff_thresh, 255, cv::THRESH_BINARY); //手动调整得到二值化阈值

    //mask按位与处理
    cv::Mat matchResult_mask, templ_mask, matchResult_bin_mask, templ_bin_mask, gau_grayDiff_bin_mask;
    cv::bitwise_and(matchResult, matchResult, matchResult_mask, logoMask); //匹配结果mask
    cv::bitwise_and(matchResult_bin, matchResult_bin, matchResult_bin_mask, logoMask); //匹配结果二值图mask
    cv::bitwise_and(templ, templ, templ_mask, logoMask); //模板mask
    cv::bitwise_and(templ_bin, templ_bin, templ_bin_mask, logoMask); //模板二值图mask
//    cv::bitwise_and(gau_grayDiff_bin, gau_grayDiff_bin, gau_grayDiff_bin_mask, logoMask); //高斯滤波、灰度图差值、二值图mask
    cv::bitwise_and(gau_grayDiff_bin, gau_grayDiff_bin, gau_grayDiff_bin_mask, templ_bin); //高斯滤波、灰度图差值、二值图mask

    //gau_grayDiff_bin_mask白转红，为叠加做准备
    cv::Mat gau_grayDiff_bin_bgr = cv::Mat::zeros(gau_grayDiff_bin.size(), CV_8UC3);
    for (int i = 0; i < gau_grayDiff_bin.rows; i++)
    {
        for (int j = 0; j < gau_grayDiff_bin.cols; j++)
        {
            if (gau_grayDiff_bin_mask.at<uchar>(i, j) == 255)
            {
                gau_grayDiff_bin_bgr.at<cv::Vec3b>(i, j)[0] = 0;
                gau_grayDiff_bin_bgr.at<cv::Vec3b>(i, j)[1] = 0;
                gau_grayDiff_bin_bgr.at<cv::Vec3b>(i, j)[2] = 255;
            }
        }
    }

    //GRAY2BGR，为叠加做准备，方便显示带有颜色的图片
    cv::Mat matchResult_bin_mask_bgr, templ_bin_mask_bgr, gau_grayDiff_bin_mask_bgr;
    cv::cvtColor(matchResult_bin_mask, matchResult_bin_mask_bgr, cv::COLOR_GRAY2BGR);
    cv::cvtColor(templ_bin_mask, templ_bin_mask_bgr, cv::COLOR_GRAY2BGR);
    cv::cvtColor(gau_grayDiff_bin_mask, gau_grayDiff_bin_mask_bgr, cv::COLOR_GRAY2BGR);

    //叠加
    cv::Mat match_result_mask_bgr_add, templ_mask_bgr_add;
    cv::add(matchResult_bin_mask_bgr, gau_grayDiff_bin_bgr, match_result_mask_bgr_add);
    cv::add(templ_bin_mask_bgr, gau_grayDiff_bin_bgr, templ_mask_bgr_add);

    //利用窗口对图像进行遍历，窗口满元素则标红
    cv::Size wndSize(6, 6);
    bool isAbnormal = false;
    for (int i = 0; i < gau_grayDiff_bin_mask.rows - wndSize.height; i++)
    {
        for (int j = 0; j < gau_grayDiff_bin_mask.cols - wndSize.width; j++)
        {
            cv::Rect wnd_roi(cv::Point(j, i), wndSize);
            cv::Mat wnd = gau_grayDiff_bin_mask(wnd_roi);
            if (cv::countNonZero(wnd) == wndSize.width * wndSize.height)
            {
                isAbnormal = true;
                gau_grayDiff_bin_mask_bgr(wnd_roi) = cv::Scalar(0, 0, 255);
            }
        }
    }

    return isAbnormal;
    //显示图片
//    cv::imshow("gau_grayDiff", gau_grayDiff);
//    cv::imshow("gau_grayDiff_bin", gau_grayDiff_bin);
//    cv::imshow("matchResult", matchResult);
//    cv::imshow("matchResult_mask_bgr_add", match_result_mask_bgr_add);
//    cv::imshow("templ", templ);
//    cv::imshow("templ_mask_bgr_add", templ_mask_bgr_add);
//    cv::imshow("gau_grayDiff_bin_mask_bgr", gau_grayDiff_bin_mask_bgr);
//    QString savePath = QString("./imgs/gau_grayDiff_bin_mask/%1").arg(fileName);
//    cv::imwrite("./inspect_result/anomaly.bmp", gau_grayDiff_bin_mask);
    //    cv::waitKey(0);
}

//正在使用这个函数，注意img是模板匹配后的结果，mask是logo区域
bool Utils::inspectLogo(cv::Mat &img, cv::Mat &templ, cv::Mat &templ_bin, cv::Mat &mask, int thresh, int wnd_sz)
{
    cv::Mat matchResult = img.clone();
    //3x3高斯模糊，灰度图absdiff
    cv::Mat matchResult_gau, templ_gau, gau_grayDiff;
    cv::GaussianBlur(matchResult, matchResult_gau, cv::Size(3, 3), 1.5, 1.5);
    cv::GaussianBlur(templ, templ_gau, cv::Size(3, 3), 1.5, 1.5);
    cv::absdiff(matchResult_gau, templ_gau, gau_grayDiff);

    //匹配结果二值化，absdiff二值化
    cv::Mat matchResult_bin, gau_grayDiff_bin;
    cv::threshold(matchResult, matchResult_bin, thresh, 255, cv::THRESH_BINARY); //标注阶段手动调整得到二值化阈值
    int thresh_diff = getDiffThresh(gau_grayDiff); //直方图统计，gau_grayDiff的gau_acc_rat大于0.95对应的像素值
    cv::threshold(gau_grayDiff, gau_grayDiff_bin, thresh_diff, 255, cv::THRESH_BINARY);

    //mask按位与处理，排除无关区域
    cv::Mat matchResult_mask, matchResult_bin_mask, templ_mask, templ_bin_mask, gau_grayDiff_bin_mask;
    cv::bitwise_and(matchResult, matchResult, matchResult_mask, mask); //匹配结果mask
    cv::bitwise_and(matchResult_bin, matchResult_bin, matchResult_bin_mask, mask); //匹配结果二值图mask
    cv::bitwise_and(templ, templ, templ_mask, mask); //模板mask
    cv::bitwise_and(templ_bin, templ_bin, templ_bin_mask, mask); //模板二值图mask
    cv::bitwise_and(gau_grayDiff_bin, gau_grayDiff_bin, gau_grayDiff_bin_mask, mask); //高斯滤波、灰度图差值、二值图mask

    //gau_grayDiff_bin_mask白转红，为叠加做准备
    cv::Mat gau_grayDiff_bin_mask_red = cv::Mat::zeros(gau_grayDiff_bin_mask.size(), CV_8UC3);
    for (int i = 0; i < gau_grayDiff_bin_mask.rows; i++)
    {
        for (int j = 0; j < gau_grayDiff_bin_mask.cols; j++)
        {
            if (gau_grayDiff_bin_mask.at<uchar>(i, j) == 255)
            {
                gau_grayDiff_bin_mask_red.at<cv::Vec3b>(i, j)[0] = 0;
                gau_grayDiff_bin_mask_red.at<cv::Vec3b>(i, j)[1] = 0;
                gau_grayDiff_bin_mask_red.at<cv::Vec3b>(i, j)[2] = 255;
            }
        }
    }


    //GRAY2BGR，为叠加做准备，方便显示带有颜色的图片
    cv::Mat matchResult_bin_mask_bgr, templ_bin_mask_bgr, gau_grayDiff_bin_mask_bgr;
    cv::cvtColor(matchResult_bin_mask, matchResult_bin_mask_bgr, cv::COLOR_GRAY2BGR);
    cv::cvtColor(templ_bin_mask, templ_bin_mask_bgr, cv::COLOR_GRAY2BGR);
    cv::cvtColor(gau_grayDiff_bin_mask, gau_grayDiff_bin_mask_bgr, cv::COLOR_GRAY2BGR);

    //叠加红色，是为了显示差异，简单的add好像不行，原本白的地方加红已经无法变成红色
    cv::Mat matchResult_bin_mask_bgr_add, templ_mask_bgr_add;
    matchResult_bin_mask_bgr_add = matchResult_bin_mask_bgr.clone();
//    cv::add(matchResult_bin_mask_bgr, gau_grayDiff_bin_mask_bgr, matchResult_mask_bgr_add);
//    cv::add(templ_bin_mask_bgr, gau_grayDiff_bin_mask_bgr, templ_mask_bgr_add);
//    cv::add(matchResult_bin_mask_bgr, gau_grayDiff_bin_mask_red, matchResult_mask_bgr_add);
    cv::add(templ_bin_mask_bgr, gau_grayDiff_bin_mask_red, templ_mask_bgr_add);

    //利用窗口对图像进行遍历，窗口满元素则标红
    cv::Size wndSize(wnd_sz, wnd_sz);
    bool isAbnormal = false;
    for (int i = 0; i < gau_grayDiff_bin_mask.rows - wndSize.height; i++)
    {
        for (int j = 0; j < gau_grayDiff_bin_mask.cols - wndSize.width; j++)
        {
            cv::Rect wnd_roi(cv::Point(j, i), wndSize);
            cv::Mat wnd = gau_grayDiff_bin_mask(wnd_roi);
            if (cv::countNonZero(wnd) == wndSize.width * wndSize.height)
            {
                isAbnormal = true;
                gau_grayDiff_bin_mask_bgr(wnd_roi) = cv::Scalar(0, 0, 255);
                matchResult_bin_mask_bgr_add(wnd_roi) = cv::Scalar(0, 0, 255);
            }
        }
    }
    //显示图片
    //    cv::imshow("gau_grayDiff", gau_grayDiff);
    //    cv::imshow("gau_grayDiff_bin", gau_grayDiff_bin);
    //    cv::imshow("matchResult", matchResult);
#ifdef _DEBUG
    //    QString savePath = QString("./imgs/gau_grayDiff_bin_mask/%1").arg(fileName);
    //    cv::imwrite("./inspect_result/anomaly.bmp", gau_grayDiff_bin_mask);
#endif
//    cv::imshow("matchResult", matchResult); //待匹配图片
//    cv::imshow("templ", templ);
//    cv::imshow("gau_grayDiff", gau_grayDiff);
//    cv::imshow("gau_grayDiff_bin", gau_grayDiff_bin);

//    //bin
//    cv::imshow("matchResult_bin", matchResult_bin); //待匹配图片
//    cv::imshow("templ_bin", templ_bin);

//    //mask
//    cv::imshow("matchResult_mask", matchResult_mask);
//    cv::imshow("matchResult_bin_mask", matchResult_bin_mask);
//    cv::imshow("templ_mask", templ_mask);
//    cv::imshow("templ_bin_mask", templ_bin_mask);
//    cv::imshow("gau_grayDiff_bin_mask", gau_grayDiff_bin_mask);

//    //add
//    cv::imshow("gau_grayDiff_bin_mask_red", gau_grayDiff_bin_mask_red);
//    cv::imshow("matchResult_bin_mask_bgr_add", matchResult_bin_mask_bgr_add); //直接在匹配结果上显示差异
//    cv::imshow("templ_mask_bgr_add", templ_mask_bgr_add);
//    cv::imshow("gau_grayDiff_bin_mask_bgr", gau_grayDiff_bin_mask_bgr); //大于窗口的会标红
//    cv::waitKey(0);
    return isAbnormal;
}


bool Utils::ocv(cv::Mat &img, cv::Mat &templ, cv::Mat &templ_bin, cv::Mat &mask)
{
    cv::Mat matchResult = img.clone();
    //3x3高斯模糊，灰度图absdiff
    cv::Mat matchResult_gau, templ_gau, gau_grayDiff;
    cv::GaussianBlur(matchResult, matchResult_gau, cv::Size(3, 3), 1.5, 1.5);
    cv::GaussianBlur(templ, templ_gau, cv::Size(3, 3), 1.5, 1.5);
    cv::absdiff(matchResult_gau, templ_gau, gau_grayDiff);
    //匹配结果二值化，diff二值化
    cv::Mat matchResult_bin, gau_grayDiff_bin;
    int templ_thesh = 200;
    cv::threshold(matchResult, matchResult_bin, templ_thesh, 255, cv::THRESH_BINARY); //手动调整得到二值化阈值
    int diff_thresh = getDiffThresh(gau_grayDiff); //直方图统计，gau_grayDiff的gau_acc_rat大于0.95对应的像素值
    cv::threshold(gau_grayDiff, gau_grayDiff_bin, diff_thresh, 255, cv::THRESH_BINARY); //手动调整得到二值化阈值

    //mask按位与处理
    cv::Mat matchResult_mask, templ_mask, matchResult_bin_mask, templ_bin_mask, gau_grayDiff_bin_mask;
    cv::bitwise_and(matchResult, matchResult, matchResult_mask, mask); //匹配结果mask
    cv::bitwise_and(matchResult_bin, matchResult_bin, matchResult_bin_mask, mask); //匹配结果二值图mask
    cv::bitwise_and(templ, templ, templ_mask, mask); //模板mask
    cv::bitwise_and(templ_bin, templ_bin, templ_bin_mask, mask); //模板二值图mask
//    cv::bitwise_and(gau_grayDiff_bin, gau_grayDiff_bin, gau_grayDiff_bin_mask, logoMask); //高斯滤波、灰度图差值、二值图mask
    cv::bitwise_and(gau_grayDiff_bin, gau_grayDiff_bin, gau_grayDiff_bin_mask, mask); //高斯滤波、灰度图差值、二值图mask

    //gau_grayDiff_bin_mask白转红，为叠加做准备
    cv::Mat gau_grayDiff_bin_bgr = cv::Mat::zeros(gau_grayDiff_bin.size(), CV_8UC3);
    for (int i = 0; i < gau_grayDiff_bin.rows; i++)
    {
        for (int j = 0; j < gau_grayDiff_bin.cols; j++)
        {
            if (gau_grayDiff_bin_mask.at<uchar>(i, j) == 255)
            {
                gau_grayDiff_bin_bgr.at<cv::Vec3b>(i, j)[0] = 0;
                gau_grayDiff_bin_bgr.at<cv::Vec3b>(i, j)[1] = 0;
                gau_grayDiff_bin_bgr.at<cv::Vec3b>(i, j)[2] = 255;
            }
        }
    }

    //GRAY2BGR，为叠加做准备，方便显示带有颜色的图片
    cv::Mat matchResult_bin_mask_bgr, templ_bin_mask_bgr, gau_grayDiff_bin_mask_bgr;
    cv::cvtColor(matchResult_bin_mask, matchResult_bin_mask_bgr, cv::COLOR_GRAY2BGR);
    cv::cvtColor(templ_bin_mask, templ_bin_mask_bgr, cv::COLOR_GRAY2BGR);
    cv::cvtColor(gau_grayDiff_bin_mask, gau_grayDiff_bin_mask_bgr, cv::COLOR_GRAY2BGR);

    //叠加
    cv::Mat match_result_mask_bgr_add, templ_mask_bgr_add;
    cv::add(matchResult_bin_mask_bgr, gau_grayDiff_bin_bgr, match_result_mask_bgr_add);
    cv::add(templ_bin_mask_bgr, gau_grayDiff_bin_bgr, templ_mask_bgr_add);

    //利用窗口对图像进行遍历，窗口满元素则标红
    cv::Size wndSize(4, 4);
    bool isAbnormal = false;
    for (int i = 0; i < gau_grayDiff_bin_mask.rows - wndSize.height; i++)
    {
        for (int j = 0; j < gau_grayDiff_bin_mask.cols - wndSize.width; j++)
        {
            cv::Rect wnd_roi(cv::Point(j, i), wndSize);
            cv::Mat wnd = gau_grayDiff_bin_mask(wnd_roi);
            if (cv::countNonZero(wnd) == wndSize.width * wndSize.height)
            {
                isAbnormal = true;
                gau_grayDiff_bin_mask_bgr(wnd_roi) = cv::Scalar(0, 0, 255);
            }
        }
    }
    //显示图片
//    cv::imshow("gau_grayDiff", gau_grayDiff);
//    cv::imshow("gau_grayDiff_bin", gau_grayDiff_bin);
//    cv::imshow("matchResult", matchResult);
#ifdef _DEBUG
    cv::imshow("matchResult_mask_bgr_add", match_result_mask_bgr_add); //直接在匹配结果上显示差异
//    cv::imshow("templ", templ);
//    cv::imshow("templ_mask_bgr_add", templ_mask_bgr_add);
    cv::imshow("gau_grayDiff_bin_mask_bgr", gau_grayDiff_bin_mask_bgr); //大于窗口的会标红
//    QString savePath = QString("./imgs/gau_grayDiff_bin_mask/%1").arg(fileName);
//    cv::imwrite("./inspect_result/anomaly.bmp", gau_grayDiff_bin_mask);
    cv::waitKey(0);
#endif
    return isAbnormal;
}
