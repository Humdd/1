#include "AbstractPropertyBrowser.h"
#define SET_FACTORY_FOR_MANAGER( INDEX ) setFactoryForManager( GET_MANAGER( INDEX ), GET_FACTORY( INDEX ) );
//setFactoryForManager作用是将一个属性管理器与其对应的属性工厂进行关联，使得属性管理器能够正确地创建、显示、编辑属性。

AbstractPropertyBrowser::AbstractPropertyBrowser( QWidget *parent ) noexcept
    : QtTreePropertyBrowser( parent ) {
    manager_array = {
        new QtIntPropertyManager( this ),
        new QtDoublePropertyManager( this ),
        new QtStringPropertyManager( this ),
        new QtBoolPropertyManager( this ),
        new QtVariantPropertyManager( this )
    };
    factory_array = {
        new QtSpinBoxFactory( this ),
        new QtDoubleSpinBoxFactory( this ),
        new QtLineEditFactory( this ),
        new QtCheckBoxFactory( this ),
        new QtVariantEditorFactory( this )
    };
    //将每个 QtAbstractPropertyManager 子类对象与对应的 QtAbstractEditorFactoryBase 子类对象关联起来。
    //目的是在属性浏览器中显示不同类型的属性时，使用不同类型的编辑器，以便更好地编辑属性值。
    SET_FACTORY_FOR_MANAGER( 0 )
    SET_FACTORY_FOR_MANAGER( 1 )
    SET_FACTORY_FOR_MANAGER( 2 )
    SET_FACTORY_FOR_MANAGER( 3 )
    SET_FACTORY_FOR_MANAGER( 4 )
}

void AbstractPropertyBrowser::init( const QString &_json_name ) noexcept {
    this->json_name = _json_name;
    initJsonProperty();
    initTableProperty();
}

bool AbstractPropertyBrowser::readObjFromJson() noexcept {
    json_helper.open( json_name, QFile::ReadOnly );
    if( !json_helper.file.isOpen() ) return false;
    obj = json_helper.readObjFromJson();
    json_helper.file.close();
    return true;
}

//
QtProperty *
    AbstractPropertyBrowser::getIntProperty( const QString &name, const int value, const int min, const int max, const int single_step ) {
    auto int_manager = GET_MANAGER( 0 );
    auto dst         = int_manager->addProperty( name );
    int_manager->setRange( dst, min, max );
    int_manager->setValue( dst, value );
    int_manager->setSingleStep( dst, single_step );
    return dst;
}

QtProperty *AbstractPropertyBrowser::getDoubleProperty( const QString &name, const double value, const double min, const double max, const double single_step ) {
    auto double_manager = GET_MANAGER( 1 );
    auto dst            = double_manager->addProperty( name );
    double_manager->setRange( dst, min, max );
    double_manager->setValue( dst, value );
    double_manager->setSingleStep( dst, single_step );
    return dst;
}


QtProperty *AbstractPropertyBrowser::getStringProperty( const QString &name, const QString &text ) {
    auto string_manager = GET_MANAGER( 2 );
    Q_ASSERT( string_manager );
    auto dst = string_manager->addProperty( name );
    string_manager->setValue( dst, text );
    return dst;
}

void AbstractPropertyBrowser::addSubProperty( const QString &name, const QList< QtProperty * > &property_list ) noexcept {
    auto var_manager = GET_MANAGER( 4 );
    auto dst         = var_manager->addProperty( var_manager->groupTypeId(), name );
    for( auto elem: qAsConst( property_list ) ) {
        dst->addSubProperty( elem );
    }
    addProperty( dst );
}

bool AbstractPropertyBrowser::needReWriteJson() {
    Q_ASSERT( json_name.isEmpty() == false );
    if( !QFile( json_name ).exists() ) {
        obj = QJsonObject();
        return false;
    }
    json_helper.open( json_name, QFile::ReadOnly );

    if( !json_helper.file.isOpen() ) return false;

    obj = json_helper.readObjFromJson();
    json_helper.close();

    if( obj.empty() ) json_helper.file.remove();

    return false;
}

void AbstractPropertyBrowser::setJsonFromTable() {
    Q_ASSERT( json_name.isEmpty() == false );
    json_helper.open( json_name, QFile::WriteOnly );
    json_helper.writeObjToJson( obj );
    json_helper.file.close();
    needReWrite = false;
    emit propertys_reset_signal();
}

void AbstractPropertyBrowser::initJsonProperty() noexcept {
    Q_ASSERT( json_name.isEmpty() == false );

    if( !needReWriteJson() ) return;
    Q_ASSERT( !obj.empty() );
    DirManager::detectAndCreaDir( QFileInfo( json_name ).path() );

    json_helper.open( json_name, QFile::WriteOnly );
    Q_ASSERT( json_helper.file.isOpen() == true );
    json_helper.writeObjToJson( obj );
    json_helper.file.close();
}

void AbstractPropertyBrowser::initTableProperty() {
    Q_ASSERT( QFile( json_name ).exists() );
    clear();
}
