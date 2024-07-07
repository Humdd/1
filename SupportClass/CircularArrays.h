#ifndef CIRCULARARRAYS_H
#define CIRCULARARRAYS_H
#include <QVector>

template< typename T >
struct CircularArrays {
    template< typename T2 >
    void append( T2 &&elem ) noexcept {
        array.append( std::forward< T2 >( elem ) );
    }
    template< typename... Args >
    void emplace( Args &&...args ) noexcept {
        array.append( T( std::forward< Args >( args )... ) );
    }
    void clear() noexcept {
        array.clear();
        current = -1;
    }
    int size() noexcept {
        return array.size();
    }
    bool empty() noexcept {
        return array.empty();
    }
    T &first() noexcept {
        return array[ ( current = 0 ) ];
    }
    T &getNext() noexcept {
        Q_ASSERT( current != -1 );
        return array[ ( current = ( current + 1 ) % array.size() ) ];
    }

private:
    int          current = -1;
    QVector< T > array;
};

#endif// CIRCULARARRAYS_H
