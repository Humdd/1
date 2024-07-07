//#include "PropertyBrowserTemplate.h"

//PropertyBrowserTemplate::PropertyBrowserTemplate( const QString &json_name , QWidget *parent )
//        : AbstractPropertyBrowser( parent ) {
//    assert( qobject_cast< decltype( this ) >( this ) );
//    qobject_cast< decltype( this ) >( this )->init( json_name ); //这样强行调用子类方法
//}

//const QVector< QString > PropertyBrowserTemplate::getJsonPropertyNameArray() {
//    return {
//            "min_area_ratio" , "min_area_width" , "min_area_height" , "valid_point_count_ratio"
//            , "valid_height_range_min" , "valid_height_range_max" , "lean_diff" , "height_diff"
//            , "result_image_store_path" , "total_rows" , "total_cols" , "min_area_x_ratio" , "min_area_y_ratio"
//    };
//}

//const QVector< const char * > PropertyBrowserTemplate::getTablePropertyNameArray() {
//    return {
//            "小区域位置比例" , "小区域宽度" , "小区域高度" , "有效点数量比例" , "有效点最小高度" , "有效点最大高度" , "倾斜阈值" , "高度阈值" , "结果图像保存路径" , "总行数"
//            , "总列数" , "小区域水平位置比例" , "小区域垂直位置比例"
//    };
//}

//bool PropertyBrowserTemplate::needReWriteJson() {
//    AbstractPropertyBrowser::needReWriteJson();
//    bool needWrite = false;

//    if( !obj.contains( getJsonPropertyName( 0 ) ) ) {
//        obj.insert( getJsonPropertyName( 0 ) , 0.5 );
//        needWrite = true;
//    }

//    if( !obj.contains( getJsonPropertyName( 1 ) ) ) {
//        obj.insert( getJsonPropertyName( 1 ) , 6 );
//        needWrite = true;
//    }

//    if( !obj.contains( getJsonPropertyName( 2 ) ) ) {
//        obj.insert( getJsonPropertyName( 2 ) , 4 );
//        needWrite = true;
//    }

//    if( !obj.contains( getJsonPropertyName( 3 ) ) ) {
//        obj.insert( getJsonPropertyName( 3 ) , 0.5 );
//        needWrite = true;
//    }

//    if( !obj.contains( getJsonPropertyName( 4 ) ) ) {
//        obj.insert( getJsonPropertyName( 4 ) , -10 );
//        needWrite = true;
//    }

//    if( !obj.contains( getJsonPropertyName( 5 ) ) ) {
//        obj.insert( getJsonPropertyName( 5 ) , -1 );
//        needWrite = true;
//    }

//    if( !obj.contains( getJsonPropertyName( 6 ) ) ) {
//        obj.insert( getJsonPropertyName( 6 ) , 0.3 );
//        needWrite = true;
//    }

//    if( !obj.contains( getJsonPropertyName( 7 ) ) ) {
//        obj.insert( getJsonPropertyName( 7 ) , 0.4 );
//        needWrite = true;
//    }

//    if( !obj.contains( getJsonPropertyName( 8 ) ) ) {
//        obj.insert( getJsonPropertyName( 8 ) , "D:/bin/3DImage.bmp" );
//        needWrite = true;
//    }

//    if( !obj.contains( getJsonPropertyName( 9 ) ) ) {
//        obj.insert( getJsonPropertyName( 9 ) , 11 );
//        needWrite = true;
//    }

//    if( !obj.contains( getJsonPropertyName( 10 ) ) ) {
//        obj.insert( getJsonPropertyName( 10 ) , 16 );
//        needWrite = true;
//    }

//    if( !obj.contains( getJsonPropertyName( 11 ) ) || !obj.contains( getJsonPropertyName( 12 ) ) ) {
//        obj.insert( getJsonPropertyName( 11 ) , obj[ getJsonPropertyName( 0 ) ].toDouble() );
//        obj.insert( getJsonPropertyName( 12 ) , obj[ getJsonPropertyName( 0 ) ].toDouble() );
//        needWrite = true;
//    }

//    return needWrite;
//}

//void PropertyBrowserTemplate::initTableProperty() {
//    AbstractPropertyBrowser::initTableProperty();
//    auto prop_8 = var_manager->addProperty( QVariant::String , getTablePropertyName( 8 ) );
//    prop_8->setValue( obj[ "result_image_store_path" ].toString() );
//    //
//    auto prop_1 = getIntProperty( getTablePropertyName( 1 ) , obj[ "min_area_width" ].toInt() , 0 ) ,
//            prop_2 = getIntProperty( getTablePropertyName( 2 ) , obj[ "min_area_height" ].toInt() , 0 ) ,
//            prop_9 = getIntProperty( getTablePropertyName( 9 ) , obj[ "total_rows" ].toInt() , 0 ) ,
//            prop_10 = getIntProperty( getTablePropertyName( 10 ) , obj[ "total_cols" ].toInt() , 0 );
//    //
//    auto prop_0 = getDoubleProperty( getTablePropertyName( 0 ) , obj[ "min_area_ratio" ].toDouble() , 0 , 1 ) ,
//            prop_3 = getDoubleProperty( getTablePropertyName( 3 ) , obj[ "valid_point_count_ratio" ].toDouble() , 0
//                                        , 1 ) ,
//            prop_4 = getDoubleProperty( getTablePropertyName( 4 ) , obj[ "valid_height_range_min" ].toDouble()
//                                        , -DBL_MAX , -5 ) ,
//            prop_5 = getDoubleProperty( getTablePropertyName( 5 ) , obj[ "valid_height_range_max" ].toDouble() ) ,
//            prop_6 = getDoubleProperty( getTablePropertyName( 6 ) , obj[ "lean_diff" ].toDouble() , 0 ) ,
//            prop_7 = getDoubleProperty( getTablePropertyName( 7 ) , obj[ "height_diff" ].toDouble() , 0 ) ,
//            prop_11 = getDoubleProperty( getTablePropertyName( 11 ) , obj[ "min_area_x_ratio" ].toDouble() , 0 , 1 ) ,
//            prop_12 = getDoubleProperty( getTablePropertyName( 12 ) , obj[ "min_area_y_ratio" ].toDouble() , 0 , 1 );
//    //
//    addSubProperty( QString::fromLocal8Bit( "小区域信息" ) , {prop_0 , prop_1 , prop_2 , prop_11 , prop_12} );
//    addSubProperty( QString::fromLocal8Bit( "有效点信息" ) , {prop_3 , prop_4 , prop_5} );
//    addSubProperty( QString::fromLocal8Bit( "阈值" ) , {prop_6 , prop_7} );
//    addProperty( prop_8 );
//    addProperty( prop_9 );
//    addProperty( prop_10 );
//    property_array << prop_0 << prop_1 << prop_2 << prop_3 << prop_4
//                   << prop_5 << prop_6 << prop_7 << prop_8 << prop_9
//                   << prop_10 << prop_11 << prop_12;
//}

//void PropertyBrowserTemplate::setTableFromJson() {
//    initJsonProperty();
//    if( obj.empty() ) return;
//    //    qDebug() << obj;
//    flushTable< double >( obj , 0 );
//    flushTable< int >( obj , 1 );
//    flushTable< int >( obj , 2 );
//    flushTable< double >( obj , 3 );
//    flushTable< double >( obj , 4 );
//    flushTable< double >( obj , 5 );
//    flushTable< double >( obj , 6 );
//    flushTable< double >( obj , 7 );
//    flushTable< QString >( obj , 8 );
//    flushTable< int >( obj , 9 );
//    flushTable< int >( obj , 10 );
//    flushTable< double >( obj , 11 );
//    flushTable< double >( obj , 12 );
//}

//void PropertyBrowserTemplate::setJsonFromTable() {
//    flushJson< double >( obj , 0 );
//    flushJson< int >( obj , 1 );
//    flushJson< int >( obj , 2 );
//    flushJson< double >( obj , 3 );
//    flushJson< double >( obj , 4 );
//    flushJson< double >( obj , 5 );
//    flushJson< double >( obj , 6 );
//    flushJson< double >( obj , 7 );
//    flushJson< QString >( obj , 8 );
//    flushJson< int >( obj , 9 );
//    flushJson< int >( obj , 10 );
//    flushJson< double >( obj , 11 );
//    flushJson< double >( obj , 12 );
//    //    qDebug() << obj;
//    AbstractPropertyBrowser::setJsonFromTable();
//}
