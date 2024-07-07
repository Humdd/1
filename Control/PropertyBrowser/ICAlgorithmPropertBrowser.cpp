#include "ICAlgorithmPropertBrowser.h"

//读取json文件的构造函数
ICAlgorithmPropertBrowser::ICAlgorithmPropertBrowser( const QString &json_name, QWidget *parent )
    : AbstractPropertyBrowser( parent ) {
    Q_ASSERT( static_cast< decltype( this ) >( this ) );//将 this 指针转换为指向ICAlgorithmPropertBrowser对象的指针
    static_cast< decltype( this ) >( this )->init( json_name );
}

//ic相机参数初始化
void ICAlgorithmPropertBrowser::initTableProperty() noexcept {
    AbstractPropertyBrowser::initTableProperty();
    QVector< QtProperty * > property_ptr_array = {
        //chip
        getStringProperty( u8"芯片区域坐标", getJsonValue< QString >( 0 ) ),
        getIntProperty( u8"芯片二值化阈值", getJsonValue< int >( 1 ), 0 ),
        getIntProperty( u8"芯片黑柱平均宽度", getJsonValue< int >( 2 ), 0 ),
        //logo
        getStringProperty( u8"logo区域坐标", getJsonValue< QString >( 3 ) ),
        getIntProperty( u8"binary_threshold", getJsonValue< int >( 4 ), 0, 255 ),
        getDoubleProperty( u8"diffRatio", getJsonValue< double >( 5 ), 0, 1 ),
        getIntProperty( u8"wndsz", getJsonValue< int >( 6 ), 0 ),
        getIntProperty( u8"pad_logo", getJsonValue< int >( 7 ), 0 ),
        //circle
        getStringProperty( u8"圆矩形区域坐标", getJsonValue< QString >( 8 ) ),
        getIntProperty( u8"cnt_BGA_circle", getJsonValue< int >( 9 ), 0 ),
        getIntProperty( u8"minDist", getJsonValue< int >( 10 ), 0 ),
        getIntProperty( u8"cannyThresh", getJsonValue< int >( 11 ), 0 ),
        getIntProperty( u8"precision", getJsonValue< int >( 12 ), 0 ),
        getIntProperty( u8"minRadius_bga", getJsonValue< int >( 13 ), 0 ),
        getIntProperty( u8"maxRadius_bga", getJsonValue< int >( 14 ), 0 ),
        getIntProperty( u8"minRadius", getJsonValue< int >( 15 ), 0 ),
        getIntProperty( u8"maxRadius", getJsonValue< int >( 16 ), 0 ),
        getIntProperty( u8"pad_circle", getJsonValue< int >( 17 ), 0 )
    };
    addSubProperty( u8"芯片", { property_ptr_array.cbegin(), property_ptr_array.cbegin() + 3 } );
    addSubProperty( u8"logo", { property_ptr_array.cbegin() + 3, property_ptr_array.cbegin() + 8 } );
    addSubProperty( u8"圆点", { property_ptr_array.cbegin() + 8, property_ptr_array.cbegin() + 18 } );
    //    addProperty( property_ptr_array[ 6 ] );
    Q_ASSERT( property_tuple_array.size() == property_ptr_array.size() );
    for( int index = 0; index < property_tuple_array.size(); ++index ) {
        std::get< 2 >( property_tuple_array[ index ] ) = property_ptr_array[ index ];
        property_ptr_array[ index ]->setEnabled( false );
    }
}

//包括initJsonProperty
bool ICAlgorithmPropertBrowser::needReWriteJson() noexcept {
    AbstractPropertyBrowser::needReWriteJson();
    bool needWrite = false;
    if( !obj.contains( "rect_chip" ) ) {
        obj.insert( "rect_chip", rect_chip_obj = QJsonObject() );
        needWrite = true;
    }
    else {
        rect_chip_obj = obj[ "rect_chip" ].toObject();
    }
    if( !obj.contains( "rect_logo" ) ) {
        obj.insert( "rect_logo", rect_logo_obj = QJsonObject() );
        needWrite = true;
    }
    else {
        rect_logo_obj = obj[ "rect_logo" ].toObject();
    }
    if( !obj.contains( "rect_circle" ) ) {
        obj.insert( "rect_circle", rect_circle_obj = QJsonObject() );
        needWrite = true;
    }
    else {
        rect_circle_obj = obj[ "rect_circle" ].toObject();
    }
    //rect_chip
    bool isRectPosNeedWrite = false;
    if( !rect_chip_obj.contains( "Pos" ) ) {
        rect_chip_obj.insert( "Pos", "" );
        isRectPosNeedWrite = true;
    }
    if( !rect_chip_obj.contains( "binary_threshold" ) ) {
        rect_chip_obj.insert( "binary_threshold", 70 );
        isRectPosNeedWrite = true;
    }
    if( !rect_chip_obj.contains( "mean_height" ) ) {
        rect_chip_obj.insert( "mean_height", 40 );
        isRectPosNeedWrite = true;
    }
    if( isRectPosNeedWrite ) {
        obj.insert( "rect_chip", rect_chip_obj );
    }
    //rect_logo
    bool isAlgorithmParamsNeedWrite = false;
    if( !rect_logo_obj.contains( "Pos" ) ) {
        rect_logo_obj.insert( "Pos", "" );
        isAlgorithmParamsNeedWrite = true;
    }
    if( !rect_logo_obj.contains( "binary_threshold" ) ) {
        rect_logo_obj.insert( "binary_threshold", 120 );
        isAlgorithmParamsNeedWrite = true;
    }
    if( !rect_logo_obj.contains( "wndsz" ) ) {
        rect_logo_obj.insert( "wndsz", 8 );
        isAlgorithmParamsNeedWrite = true;
    }
    if( !rect_logo_obj.contains( "diffRatio" ) ) {
        rect_logo_obj.insert( "diffRatio", 0.98 );
        isAlgorithmParamsNeedWrite = true;
    }
    if( !rect_logo_obj.contains( "pad_logo" ) ) {
        rect_logo_obj.insert( "pad_logo", 60 );
        isAlgorithmParamsNeedWrite = true;
    }
    if( isAlgorithmParamsNeedWrite ) {
        obj.insert( "rect_logo", rect_logo_obj );
    }
    //rect_circle
    isAlgorithmParamsNeedWrite = false;
    if( !rect_circle_obj.contains( "Pos" ) ) {
        rect_circle_obj.insert( "Pos", "" );
        isAlgorithmParamsNeedWrite = true;
    }
    if( !rect_circle_obj.contains( "minRadius" ) ) {
        rect_circle_obj.insert( "minRadius", 40 );
        isAlgorithmParamsNeedWrite = true;
    }
    if( !rect_circle_obj.contains( "maxRadius" ) ) {
        rect_circle_obj.insert( "maxRadius", 70 );
        isAlgorithmParamsNeedWrite = true;
    }
    if( !rect_circle_obj.contains( "minRadius_bga" ) ) {
        rect_circle_obj.insert( "minRadius_bga", 12 );
        isAlgorithmParamsNeedWrite = true;
    }
    if( !rect_circle_obj.contains( "maxRadius_bga" ) ) {
        rect_circle_obj.insert( "maxRadius_bga", 17 );
        isAlgorithmParamsNeedWrite = true;
    }
    if( !rect_circle_obj.contains( "cnt_BGA_circle" ) ) {
        rect_circle_obj.insert( "cnt_BGA_circle", 30 );
        isAlgorithmParamsNeedWrite = true;
    }
    if( !rect_circle_obj.contains( "minDist" ) ) {
        rect_circle_obj.insert( "minDist", 60 );
        isAlgorithmParamsNeedWrite = true;
    }
    if( !rect_circle_obj.contains( "cannyThresh" ) ) {
        rect_circle_obj.insert( "cannyThresh", 120 );
        isAlgorithmParamsNeedWrite = true;
    }
    if( !rect_circle_obj.contains( "precision" ) ) {
        rect_circle_obj.insert( "precision", 17 );
        isAlgorithmParamsNeedWrite = true;
    }
    if( !rect_circle_obj.contains( "pad_circle" ) ) {
        rect_circle_obj.insert( "pad_circle", 60 );
        isAlgorithmParamsNeedWrite = true;
    }
    if( isAlgorithmParamsNeedWrite ) {
        obj.insert( "rect_circle", rect_circle_obj );
    }
    //    printVar( obj );
    if( property_tuple_array.empty() ) {
        property_tuple_array.reserve( property_count );
        property_tuple_array.push_back( { "Pos", rect_chip_obj, nullptr } );
        property_tuple_array.push_back( { "binary_threshold", rect_chip_obj, nullptr } );
        property_tuple_array.push_back( { "mean_height", rect_chip_obj, nullptr } );
        //
        property_tuple_array.push_back( { "Pos", rect_logo_obj, nullptr } );
        property_tuple_array.push_back( { "binary_threshold", rect_logo_obj, nullptr } );
        property_tuple_array.push_back( { "diffRatio", rect_logo_obj, nullptr } );
        property_tuple_array.push_back( { "wndsz", rect_logo_obj, nullptr } );
        property_tuple_array.push_back( { "pad_logo", rect_logo_obj, nullptr } );
        //
        property_tuple_array.push_back( { "Pos", rect_circle_obj, nullptr } );
        property_tuple_array.push_back( { "cnt_BGA_circle", rect_circle_obj, nullptr } );
        property_tuple_array.push_back( { "minDist", rect_circle_obj, nullptr } );
        property_tuple_array.push_back( { "cannyThresh", rect_circle_obj, nullptr } );
        property_tuple_array.push_back( { "precision", rect_circle_obj, nullptr } );
        property_tuple_array.push_back( { "minRadius_bga", rect_circle_obj, nullptr } );
        property_tuple_array.push_back( { "maxRadius_bga", rect_circle_obj, nullptr } );
        property_tuple_array.push_back( { "minRadius", rect_circle_obj, nullptr } );
        property_tuple_array.push_back( { "maxRadius", rect_circle_obj, nullptr } );
        property_tuple_array.push_back( { "pad_circle", rect_circle_obj, nullptr } );
        Q_ASSERT( property_tuple_array.size() == property_count );
    }
    return needWrite;
}

//将json对象中的某个属性值转换为对应的C++数据类型，并设置到ICAlgorithmPropertBrowser中管理的QtProperty中
void ICAlgorithmPropertBrowser::setTableFromJson() noexcept {
    //    initJsonProperty();
    if( !readObjFromJson() || obj.empty() ) return;
    rect_chip_obj   = obj[ "rect_chip" ].toObject();
    rect_logo_obj   = obj[ "rect_logo" ].toObject();
    rect_circle_obj = obj[ "rect_circle" ].toObject();
    //    printVar( obj );
    //make_index_sequence生成一个指定大小的索引序列类型
    setTableFromJson_Help( std::make_index_sequence< property_count >(), BaseTypeIndexSeq() );
}

void ICAlgorithmPropertBrowser::setJsonFromTable() noexcept {
    setJsonFromTable_Help( std::make_index_sequence< property_count >(), BaseTypeIndexSeq() );
    obj.insert( "rect_chip", rect_chip_obj );
    obj.insert( "rect_logo", rect_logo_obj );
    obj.insert( "rect_circle", rect_circle_obj );
    AbstractPropertyBrowser::setJsonFromTable();
}
