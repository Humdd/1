#include "AlgorithmPropertBrowser.h"

//读取json文件的构造函数
AlgorithmPropertBrowser::AlgorithmPropertBrowser( const QString &json_name, QWidget *parent )
    : AbstractPropertyBrowser( parent ) {
    Q_ASSERT( static_cast< decltype( this ) >( this ) );
    //decltype(this)是一个返回当前对象类型的表达式
    static_cast< decltype( this ) >( this )->init( json_name );//这样强行调用子类方法
}

void AlgorithmPropertBrowser::initTableProperty() noexcept {
    AbstractPropertyBrowser::initTableProperty();
    QVector< QtProperty * > property_ptr_array = {
        getStringProperty( u8"上矩形区域坐标", getJsonValue< QString >( 0 ) ),
        getIntProperty( u8"上矩形二值化阈值", getJsonValue< int >( 1 ), 0 ),
        getIntProperty( u8"上矩形黑柱平均宽度", getJsonValue< int >( 2 ), 0 ),
        getStringProperty( u8"下矩形区域坐标", getJsonValue< QString >( 3 ) ),
        getIntProperty( u8"下矩形二值化阈值", getJsonValue< int >( 4 ), 0 ),
        getIntProperty( u8"下矩形黑柱平均宽度", getJsonValue< int >( 5 ), 0 ),
        getStringProperty( u8"圆矩形区域坐标", getJsonValue< QString >( 6 ) ),
    };
    addSubProperty( u8"上矩形区域", { property_ptr_array.cbegin(), property_ptr_array.cbegin() + 3 } );
    addSubProperty( u8"下矩形区域", { property_ptr_array.cbegin() + 3, property_ptr_array.cbegin() + 6 } );
    addProperty( property_ptr_array[ 6 ] );
    Q_ASSERT( property_tuple_array.size() == property_ptr_array.size() );
    for( int index = 0; index < property_tuple_array.size(); ++index ) {
        std::get< 2 >( property_tuple_array[ index ] ) = property_ptr_array[ index ];
        property_ptr_array[ index ]->setEnabled( false );
    }
}

//包括initJsonProperty
bool AlgorithmPropertBrowser::needReWriteJson() noexcept {
    AbstractPropertyBrowser::needReWriteJson();
    bool needWrite = false;
    if( !obj.contains( "rect_up" ) ) {
        obj.insert( "rect_up", rect_up_obj = QJsonObject() );
        needWrite = true;
    }
    else {
        rect_up_obj = obj[ "rect_up" ].toObject();
    }
    if( !obj.contains( "rect_down" ) ) {
        obj.insert( "rect_down", rect_down_obj = QJsonObject() );
        needWrite = true;
    }
    else {
        rect_down_obj = obj[ "rect_down" ].toObject();
    }
    //3
    bool isRectPosNeedWrite = false;
    if( !rect_up_obj.contains( "Pos" ) ) {
        rect_up_obj.insert( "Pos", "" );
        isRectPosNeedWrite = true;
    }
    if( !rect_up_obj.contains( "binary_threshold" ) ) {
        rect_up_obj.insert( "binary_threshold", 70 );
        isRectPosNeedWrite = true;
    }
    if( !rect_up_obj.contains( "mean_height" ) ) {
        rect_up_obj.insert( "mean_height", 40 );
        isRectPosNeedWrite = true;
    }
    if( isRectPosNeedWrite ) {
        obj.insert( "rect_up", rect_up_obj );
    }
    //4
    bool isAlgorithmParamsNeedWrite = false;
    if( !rect_down_obj.contains( "Pos" ) ) {
        rect_down_obj.insert( "Pos", "" );
        isAlgorithmParamsNeedWrite = true;
    }
    if( !rect_down_obj.contains( "binary_threshold" ) ) {
        rect_down_obj.insert( "binary_threshold", 70 );
        isAlgorithmParamsNeedWrite = true;
    }
    if( !rect_down_obj.contains( "mean_height" ) ) {
        rect_down_obj.insert( "mean_height", 40 );
        isAlgorithmParamsNeedWrite = true;
    }
    if( isAlgorithmParamsNeedWrite ) {
        obj.insert( "rect_down", rect_down_obj );
    }
    //
    if( !obj.contains( "circle_pos" ) ) {
        obj.insert( "circle_pos", "" );
    }
    //    printVar( obj );
    if( property_tuple_array.empty() ) {
        property_tuple_array.reserve( property_count );
        property_tuple_array.push_back( { "Pos", rect_up_obj, nullptr } );
        property_tuple_array.push_back( { "binary_threshold", rect_up_obj, nullptr } );
        property_tuple_array.push_back( { "mean_height", rect_up_obj, nullptr } );
        property_tuple_array.push_back( { "Pos", rect_down_obj, nullptr } );
        property_tuple_array.push_back( { "binary_threshold", rect_down_obj, nullptr } );
        property_tuple_array.push_back( { "mean_height", rect_down_obj, nullptr } );
        property_tuple_array.push_back( { "circle_pos", obj, nullptr } );
        Q_ASSERT( property_tuple_array.size() == property_count );
    }
    return needWrite;
}

void AlgorithmPropertBrowser::setTableFromJson() noexcept {
    //    initJsonProperty();
    if( !readObjFromJson() || obj.empty() ) return;
    rect_up_obj   = obj[ "rect_up" ].toObject();
    rect_down_obj = obj[ "rect_down" ].toObject();
    //    printVar( obj );
    setTableFromJson_Help( std::make_index_sequence< property_count >(), BaseTypeIndexSeq() );
}

void AlgorithmPropertBrowser::setJsonFromTable() noexcept {
    setJsonFromTable_Help( std::make_index_sequence< property_count >(), BaseTypeIndexSeq() );
    obj.insert( "rect_up", rect_up_obj );
    obj.insert( "rect_down", rect_down_obj );
    //    printVar( obj );
    AbstractPropertyBrowser::setJsonFromTable();
}
