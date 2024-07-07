//
// Created by TonyMa on 2022/8/15.
//
#ifndef CHIPTAGGINGTOOL_QTSINK_H
#define CHIPTAGGINGTOOL_QTSINK_H
#include "spdlog/details/null_mutex.h" // 用于单线程的互斥量，其实什么也没做
#include "spdlog/sinks/base_sink.h"
#include "spdlog/fmt/fmt.h"            // 用于格式化输出的
#include <mutex>                       // 标准的互斥量库
#include <QString>
#include <QObject>
#include <QColor>

//#define QT_USER_NEED //

class MsgSender
    : public QObject {
    Q_OBJECT
public:
    MsgSender() = default;
    void emitSignal( const std::string msg, const int level ) const noexcept;

signals:
    void send_message( const QString msg, const QColor color ) const;

private:
    static const QColor &getColor( const int level ) noexcept;
};

template<class Mutex>  // 模板类，用于sink的不同的线程情况，单线程就使用一个假的互斥量，多线程就添加真正的互斥量
class QtSink
    :public spdlog::sinks::base_sink<Mutex> {
public:
    MsgSender msg_sender;
    explicit QtSink() = default;
    virtual ~QtSink() override = default;

protected:
    void sink_it_( const spdlog::details::log_msg &msg ) noexcept override{
        using memory_buf_t = fmt::basic_memory_buffer<char, 250>;
        memory_buf_t formatted;  // 内存中的数据
        spdlog::sinks::base_sink<Mutex>::formatter_->format( msg, formatted );
        // 上面 这句大致就是从msg的原始数据获得经过变换之后的数据，这个语句是去处理原始语句中的一些通配符之类的东西
        msg_sender.emitSignal( formatted.data(), msg.level );
    }
    virtual void flush_() noexcept override {}

};

using QtSink_mt = QtSink<std::mutex>;
using QtSink_st = QtSink<spdlog::details::null_mutex>;
#endif //CHIPTAGGINGTOOL_QTSINK_H
