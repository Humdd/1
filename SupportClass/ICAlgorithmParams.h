#ifndef ICALGORITHMPARAMS_H
#define ICALGORITHMPARAMS_H
#include "SingletonTemplate.h"
#include <QVector>
#include "core/mat.hpp"
#include "core/types.hpp"

struct ICAlgorithmParams: public SingletonTemplate< ICAlgorithmParams > {
    enum {
        EMPTY,
        SET_ORIGIN_IMG,
        SET_EXECUTE_IMG,
        ALGORITHM_FINISHED
    };
    QString paramRectStatus() noexcept;
    void    resetOriginImage( cv::Mat origin_gray_image, cv::Mat origin_bgr_image ) noexcept;
    bool    isStarted() noexcept;
    void    resetParams() noexcept;
    bool    isParamsValid() noexcept;
    void    algorithmFinished() noexcept;
    void    getParamsFromJson() noexcept;
    void    storeParamsIntoJson() noexcept;
    void    resetOriginGrayImage( QImage img ) noexcept;
    void    resetOriginGrayImage( cv::Mat img ) noexcept;
    cv::Mat excute_gray_image, excute_bgr_image;//复制的图像，用以执行算法，显示图像
    int *   rect_binary_threshold_ptr = nullptr;
    //存放小圆圈矩形的数组
    QVector< cv::Rect * > rect_circle_array;
    // cv::Rect *: rect_ptr int : binary_threshold int : mean_height int : type
    std::tuple<> chip_params;
    //diffRatio, wndsz, pad_logo
    std::tuple< double, int, int > logo_params;
    //cnt_BGA_circle, minDist, cannyThresh, precision, minRadius_bga, maxRadius_bga, minRadius, maxRadius, pad_circle
    std::tuple< int, int, int, int, int, int, int, int, int > circle_params;
    using ParamsTupleType = std::tuple< cv::Rect *, int, int, int >;//三个int分别是：二值化阈值 平均黑柱宽度 item类型
                                                                    //    using ParamsTupleType = std::tuple< cv::Rect *, int, int, int >; //三个int分别是：二值化阈值 平均黑柱宽度 item类型
    std::array< ParamsTupleType, 3 > rect_tuple_array;
    int                              status                = EMPTY;
    inline static double             invalid_point_ratio   = 0.8; //每一列的黑点数量要低于rect_mean_height的ratio才算invalid
    inline static double             invalid_col_ratio     = 0.1; //整个矩形ROI，invalid的列数比例超过多少即算NOT_PREESED_WELL
    inline static double             max_white_point_ratio = 0.95;//整个矩形ROI 白点数量比例超过ratio就算NOT_PREESED
    inline static double             max_black_point_ratio = 1.4; //整个矩形ROI 黑点数量比例超过ratio就算NORMAL
    //    inline static FilmStripWindow *window                = nullptr;

private:
    cv::Mat origin_gray_image, origin_bgr_image;//原始灰度/彩色图
    Q_DECLARE_SINGLETON( ICAlgorithmParams )
    ICAlgorithmParams() noexcept;
};

inline auto &params_ic = ICAlgorithmParams::getInstance();
#endif// ICALGORITHMPARAMS_H
