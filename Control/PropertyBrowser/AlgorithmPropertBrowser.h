#ifndef ALGORITHMPROPERTBROWSER_H
#define ALGORITHMPROPERTBROWSER_H

#include <AbstractPropertyBrowser.h>

class AlgorithmPropertBrowser: public AbstractPropertyBrowser {
    Q_OBJECT
    using BaseTypeIndexSeq = std::integer_sequence< size_t,
                                                    STRING_INDEX, INT_INDEX, INT_INDEX,
                                                    STRING_INDEX, INT_INDEX, INT_INDEX,
                                                    STRING_INDEX >;

public:
    AlgorithmPropertBrowser( const QString &json_name, QWidget *parent = nullptr );

    // AbstractPropertyBrowser interface
public slots:
    virtual void setTableFromJson() noexcept override;
    virtual void setJsonFromTable() noexcept override;

protected:
    QJsonObject                 rect_up_obj, rect_down_obj;
    virtual void                initTableProperty() noexcept override;
    virtual bool                needReWriteJson() noexcept override;
    constexpr inline static int property_count = 7;
};

#endif// ALGORITHMPROPERTBROWSER_H
