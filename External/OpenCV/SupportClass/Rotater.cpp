#include "Rotater.h"
#include "imgproc.hpp"

double Rotater::getImageRotateAngle( const cv::Mat &image ) {
    cv::Mat dst_image, imgGray, imgCanny, resize_image;
    if( image.channels() != 1 )
        cv::cvtColor( image, imgGray, cv::COLOR_BGR2GRAY );
    else
        imgGray = image;
    resize( imgGray, resize_image, cv::Size( 1280, 720 ) );// reduce image size to speed up calculation
    Canny( resize_image, imgCanny, 100, 200 );             // use canny operator to detect contour
    std::vector< cv::Vec4i > lineAll;
    HoughLinesP( imgCanny, lineAll, 1, CV_PI / 180, 30, 50, 4 );
    //
    std::list< cv::Vec4i > linesList;
    for( const auto &elem: qAsConst( lineAll ) ) {
        linesList.push_back( elem );
    }
    std::vector< cv::Vec4i > lineFiltered;
    for( auto itorOuter = linesList.begin(); itorOuter != linesList.end(); )
        for( auto itorInner = linesList.begin(); itorInner != linesList.end(); ++itorInner ) {
            if( abs( angleOfLines( *itorOuter, *itorInner ) - 90 ) < 1 ) {
                // take out the current two perpendicular lines to reduce the size of linesList
                lineFiltered.push_back( *itorOuter );
                lineFiltered.push_back( *itorInner );
                itorInner = linesList.erase( itorInner );
                itorOuter = linesList.erase( itorOuter );
                break;
            }
            if( itorInner == --linesList.end() )
                if( linesList.size() > 2 ) {
                    itorOuter = linesList.erase(
                        itorOuter );// erase current element when there is no other line perpendicular to it.
                }
                else {
                    itorOuter = linesList.end();
                    break;
                }
            else
                ;
        }
    double angle = 0.0;// average tilt angle of image
    if( !lineFiltered.empty() ) {
        // find edge lines of image
        std::vector< cv::Vec4i > lineEdge;
        sort( lineFiltered.begin(), lineFiltered.end(), getMinMidX );// get the line at the far left of the image
        lineEdge.push_back( lineFiltered[ 0 ] );
        sort( lineFiltered.begin(), lineFiltered.end(), getMaxMidX );// get the line at the far right of the image
        lineEdge.push_back( lineFiltered[ 0 ] );
        sort( lineFiltered.begin(), lineFiltered.end(), getMinMidY );// get the line at the top of the image
        lineEdge.push_back( lineFiltered[ 0 ] );
        sort( lineFiltered.begin(), lineFiltered.end(), getMaxMidY );// get the line at the buttom of the image
        lineEdge.push_back( lineFiltered[ 0 ] );
        for( const auto &elem: qAsConst( lineEdge ) ) {// calcualte averge tilt angle of PCB edge lines
            angle += angleForCorrect( elem );
        }
        angle /= lineEdge.size();
    }
    return angle;
}

double Rotater::angleOfLines( const cv::Vec4i &line1, const cv::Vec4i &line2 ) {
    double moduleLine1 = sqrt( pow( line1[ 0 ] - line1[ 2 ], 2 ) + pow( line1[ 1 ] - line1[ 3 ], 2 ) );
    double moduleLine2 = sqrt( pow( line2[ 0 ] - line2[ 2 ], 2 ) + pow( line2[ 1 ] - line2[ 3 ], 2 ) );
    double dotProduct  = ( line1[ 2 ] - line1[ 0 ] ) * ( line2[ 2 ] - line2[ 0 ] ) +
                        ( line1[ 3 ] - line1[ 1 ] ) * ( line2[ 3 ] - line2[ 1 ] );
    return acos( dotProduct / moduleLine1 / moduleLine2 ) * 180 / CV_PI;
}

bool Rotater::getMinMidX( const cv::Vec4i &line1, const cv::Vec4i &line2 ) {
    return ( line1[ 0 ] + line1[ 2 ] ) < ( line2[ 0 ] + line2[ 2 ] );
}

bool Rotater::getMaxMidX( const cv::Vec4i &line1, const cv::Vec4i &line2 ) {
    return ( line1[ 0 ] + line1[ 2 ] ) > ( line2[ 0 ] + line2[ 2 ] );
}

bool Rotater::getMinMidY( const cv::Vec4i &line1, const cv::Vec4i &line2 ) {
    return ( line1[ 1 ] + line1[ 3 ] ) < ( line2[ 1 ] + line2[ 3 ] );
}

bool Rotater::getMaxMidY( const cv::Vec4i &line1, const cv::Vec4i &line2 ) {
    return ( line1[ 1 ] + line1[ 3 ] ) > ( line2[ 1 ] + line2[ 3 ] );
}

double Rotater::angleForCorrect( const cv::Vec4i &line ) {
    cv::Vec4i unitXVector( 0, 0, 1, 0 );
    double    angle = angleOfLines( unitXVector, line );// here angle belongs to [0, pi/2]
    // @attention: the increment direction of X and Y axis of OpenCV is different from usual rectangular coordinate system. The origin point is in the upper left corner of the image
    if( angle < 45 )
        // consider in the horizontal direction
        if( line[ 1 ] > line[ 3 ] ) {
            angle = -angle;
        }
        else
            ;
    else if( line[ 1 ] > line[ 3 ] ) {
        angle = 90 - angle;
    }
    else {
        angle = angle - 90;
    }
    return angle;
}
