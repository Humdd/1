#ifndef ABSTRACTTREEPROPERTYBROSWER_H
#define ABSTRACTTREEPROPERTYBROSWER_H
#include "stable.h"

// std::tuple_element_t用于获取元组类型中的某一个元素的类型。
//这里将manager_array[ INDEX ] 强转为std::tuple_element_t< INDEX, ManagerTypeList >
#define GET_MANAGER( INDEX ) static_cast< std::tuple_element_t< INDEX, ManagerTypeList > >( manager_array[ INDEX ] )
#define GET_FACTORY( INDEX ) static_cast< std::tuple_element_t< INDEX, FactoryTypeList > >( factory_array[ INDEX ] )

class AbstractPropertyBrowser: public QtTreePropertyBrowser {
    Q_OBJECT
    using BaseTypeList    = std::tuple< int, double, QString, bool, QVariant >;
    using ManagerTypeList = std::tuple< QtIntPropertyManager *, QtDoublePropertyManager *, QtStringPropertyManager *, QtBoolPropertyManager *, QtVariantPropertyManager * >;
    using FactoryTypeList = std::tuple< QtSpinBoxFactory *, QtDoubleSpinBoxFactory *, QtLineEditFactory *, QtCheckBoxFactory *, QtVariantEditorFactory * >;
    using BaseTupleType   = std::tuple< QString, QJsonObject &, QtProperty * >;

public:
    enum {
        INT_INDEX,
        DOUBLE_INDEX,
        STRING_INDEX,
        BOOL_INDEX,
        VARIANT_INDEX
    };
    QString json_name;

    explicit AbstractPropertyBrowser( QWidget *parent = nullptr ) noexcept;

    void initJsonProperty() noexcept;

    template< typename T >
    void flushTableValue( const int property_index, const T &var ) noexcept {
        using remove_cvf_T   = remove_cvf_t< T >;
        constexpr auto Index = getBaseTypeIndex< remove_cvf_T >();
        static_assert( std::is_same_v< remove_cvf_T, std::tuple_element_t< Index, BaseTypeList > >,
                       "The T type is no same as std::tuple_element_t< Index, BaseTypeList >" );
        auto manager = GET_MANAGER( Index );

        Q_ASSERT( manager );
        Q_ASSERT( property_index < property_tuple_array.size() && property_index >= 0 );
        auto &[ json_property_name, _, property_ptr ] = property_tuple_array[ property_index ];
        manager->setValue( property_ptr, var );
        needReWrite = true;
    }

    template< typename T >
    void flushJsonValue( const int property_index, const T &var ) noexcept {
        using remove_cvf_T   = remove_cvf_t< T >;
        constexpr auto Index = getBaseTypeIndex< remove_cvf_T >();
        static_assert( std::is_same_v< remove_cvf_T, std::tuple_element_t< Index, BaseTypeList > >,
                       "The T type is no same as std::tuple_element_t< Index, BaseTypeList >" );
        Q_ASSERT( property_index < property_tuple_array.size() && property_index >= 0 );
        auto &[ json_property_name, json_obj, _ ] = property_tuple_array[ property_index ];
        json_obj.insert( json_property_name, var );
    }

    template< typename T >
    T getJsonValue( const int property_index ) noexcept {
        using remove_cvf_T = remove_cvf_t< T >;
        Q_ASSERT( property_index < property_tuple_array.size() && property_index >= 0 );
        auto &[ json_property_name, json_obj, _ ] = property_tuple_array[ property_index ];
        Q_ASSERT( json_obj[ json_property_name ].toVariant().canConvert< remove_cvf_T >() );
        return json_obj[ json_property_name ].toVariant().value< remove_cvf_T >();
    }

    bool isJsonExist() const noexcept {
        return QFile::exists( json_name );
    }

    void checkAndCreateJson() noexcept {
        if( !QFile::exists( json_name ) ) {
            initJsonProperty();
            setTableFromJson();
        }
    }

    void needResetJson() noexcept {
        needReWrite = true;
    }

    void tryResetJsonFromTable() noexcept {
        if( !needReWrite )
            return;
        setJsonFromTable();
    }

    bool readObjFromJson() noexcept;

signals:
    void propertys_reset_signal();

public slots:

    virtual void setTableFromJson() = 0;

    virtual void setJsonFromTable() = 0;

protected:
    void init( const QString &_json_name ) noexcept;

    virtual void initTableProperty() = 0;

    template< size_t Index >
    void flushTableFromJson( const int property_index ) noexcept {
        auto manager = GET_MANAGER( Index );
        Q_ASSERT( manager );
        Q_ASSERT( property_index < property_tuple_array.size() && property_index >= 0 );
        auto &[ json_property_name, json_obj, property_ptr ] = property_tuple_array[ property_index ];
        manager->setValue(
            property_ptr,
            json_obj[ json_property_name ].toVariant().value< std::tuple_element_t< Index, BaseTypeList > >() );
    }
    //json_obj[ json_property_name ]是访问json对象中的一个属性，然后通过toVariant()将其转换为Qt的Variant类型。
    //接着，使用value< std::tuple_element_t< Index, BaseTypeList > >()将Variant类型转换为C++数据类型，其中Index是当前属性在BaseTypeList中的下标。

    template< size_t Index >
    void flushJsonFromTable( const int property_tuple_index ) noexcept {
        auto manager = GET_MANAGER( Index );
        Q_ASSERT( manager );
        Q_ASSERT( property_tuple_index < property_tuple_array.size() && property_tuple_index >= 0 );
        auto &[ json_property_name, json_obj, property_ptr ] = property_tuple_array[ property_tuple_index ];
        json_obj.insert( json_property_name, manager->value( property_ptr ) );
    }

    QtProperty *
        getIntProperty(
            const QString &name, const int value,
            const int min = INT_MIN, const int max = INT_MAX, const int single_step = 1 );

    QtProperty *getDoubleProperty(
        const QString &name, const double value,
        const double min = -DBL_MAX, const double max = DBL_MAX, const double single_step = 0.01 );

    QtProperty *getStringProperty( const QString &name, const QString &text );

    void addSubProperty( const QString &name, const QList< QtProperty * > &property_list ) noexcept;

    virtual bool needReWriteJson() = 0;
    //
    bool needReWrite = false;

    JsonHelper json_helper;

    QJsonObject obj;

    QVector< BaseTupleType > property_tuple_array;

    QVarLengthArray< QtAbstractPropertyManager *, 5 > manager_array;

    QVarLengthArray< QtAbstractEditorFactoryBase *, 5 > factory_array;

    template< size_t... Index1, size_t... Index2 >
    inline void setTableFromJson_Help( std::index_sequence< Index1... >, std::index_sequence< Index2... > ) noexcept {
        static_cast< void >( std::initializer_list< int >{ ( flushTableFromJson< Index2 >( Index1 ), 0 )... } );
    }//flushTableFromJson<Index2>(Index1) 的方式，将 Index1 和 Index2 中的每一个索引对应起来，然后调用 flushTableFromJson() 函数进行操作。同时，为了保证代码的语法正确性，使用了一个 std::initializer_list 的方式来进行抑制警告。
//initializer_list有三个参数，所以后面加0补位

    template< size_t... Index1, size_t... Index2 >
    inline void setJsonFromTable_Help( std::index_sequence< Index1... >, std::index_sequence< Index2... > ) noexcept {
        static_cast< void >( std::initializer_list< int >{ ( flushJsonFromTable< Index2 >( Index1 ), 0 )... } );
    }

    template< typename _T >
    constexpr inline static size_t getBaseTypeIndex() noexcept {
        using T = remove_cvf_t< _T >;
        if constexpr( std::is_same_v< T, int > )
            return 0;
        else if constexpr( std::is_same_v< T, double > )
            return 1;
        else if constexpr( std::is_same_v< T, QString > )
            return 2;
        else if constexpr( std::is_same_v< T, bool > )
            return 3;
        else if constexpr( std::is_same_v< T, QVariant > )
            return 4;
    }
};

using AbstractPropertyBrowserPtr = AbstractPropertyBrowser *;

Q_DECLARE_METATYPE( AbstractPropertyBrowserPtr )

#endif// ABSTRACTTREEPROPERTYBROSWER_H
