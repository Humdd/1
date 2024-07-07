#ifndef ICALGORITHMPROPERTBROWSER_H
#define ICALGORITHMPROPERTBROWSER_H

#include <AbstractPropertyBrowser.h>

class ICAlgorithmPropertBrowser: public AbstractPropertyBrowser {
    Q_OBJECT
    using BaseTypeIndexSeq = std::integer_sequence< size_t,
                                                    STRING_INDEX, INT_INDEX, INT_INDEX,                                                                             //3
                                                    STRING_INDEX, INT_INDEX, DOUBLE_INDEX, INT_INDEX, INT_INDEX,                                                //5
                                                    STRING_INDEX, INT_INDEX, INT_INDEX, INT_INDEX, INT_INDEX, INT_INDEX, INT_INDEX, INT_INDEX, INT_INDEX, INT_INDEX>;//10

public:
    ICAlgorithmPropertBrowser( const QString &json_name, QWidget *parent = nullptr );

    // AbstractPropertyBrowser interface
public slots:
    virtual void setTableFromJson() noexcept override;
    virtual void setJsonFromTable() noexcept override;

protected:
    QJsonObject                 rect_chip_obj, rect_logo_obj, rect_circle_obj;
    virtual void                initTableProperty() noexcept override;
    virtual bool                needReWriteJson() noexcept override;
    constexpr inline static int property_count = 18;
};

#endif// ICALGORITHMPROPERTBROWSER_H
